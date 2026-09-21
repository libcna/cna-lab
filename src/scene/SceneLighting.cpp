// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Scene/SceneLighting.hpp"

#include <algorithm>
#include <cmath>

#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/Scene/SceneTransform.hpp"

namespace CNA::Studio
{
    namespace
    {
        /** @brief Returns the light kind @p name spells, defaulting to Directional. */
        SceneLightKind parseLightKind(const std::string& name)
        {
            if (name == "Point") { return SceneLightKind::Point; }
            if (name == "Spot") { return SceneLightKind::Spot; }

            // Directional for anything unrecognised, rather than skipping the light. A scene
            // written by a newer Studio with a fourth kind in it should still light something:
            // the wrong kind of light is a visible, correctable state, and no light at all reads
            // as the component being ignored.
            return SceneLightKind::Directional;
        }

        /** @brief Returns @p color scaled by @p intensity and @p falloff, in 0..1 components. */
        StudioVector3 toLinearScaled(const StudioColor& color, float intensity, float falloff)
        {
            const float factor = std::max(0.0f, intensity) * std::max(0.0f, falloff) / 255.0f;
            return StudioVector3{static_cast<float>(color.r) * factor,
                                 static_cast<float>(color.g) * factor,
                                 static_cast<float>(color.b) * factor};
        }

        /** @brief Returns how much of @p light reaches @p targetWorld, in 0..1. */
        float falloffAt(const SceneLight& light, const StudioVector3& targetWorld)
        {
            // A directional light is the sun: it does not get further away.
            if (light.kind == SceneLightKind::Directional) { return 1.0f; }

            const float range = std::max(light.range, 1e-4f);
            const StudioVector3 offset = subtract(targetWorld, light.position);
            const float distance = std::sqrt(dot(offset, offset));
            if (distance >= range) { return 0.0f; }

            // Smooth to zero at the range rather than inverse-square clipped at it. Inverse-square
            // is the physical answer and the wrong one here: it never reaches zero, so a lamp with
            // a range would still tint everything outside it, and the range control in the
            // inspector would do nothing a user could see. This falls off and then stops.
            const float t = 1.0f - distance / range;
            return t * t;
        }

        /** @brief Returns the direction @p light illuminates @p targetWorld from. */
        StudioVector3 directionAt(const SceneLight& light, const StudioVector3& targetWorld)
        {
            if (light.kind == SceneLightKind::Directional) { return light.direction; }

            // A point or spot light aimed at the thing being drawn -- which is what makes the
            // approximation work at all, and also its limit: the whole object is lit as though it
            // sat at the point this was resolved against.
            const StudioVector3 offset = subtract(targetWorld, light.position);
            const float length = std::sqrt(dot(offset, offset));
            if (length < 1e-4f) { return light.direction; }

            return scale(offset, 1.0f / length);
        }

        /** @brief Returns the perceptual weight of a colour, for ranking lights against each other. */
        float brightnessOf(const StudioVector3& color)
        {
            // Rec. 601 luma. Any monotonic weighting would order lights the same way most of the
            // time; this one at least orders a green light above a blue one of the same numbers,
            // which matches what somebody looking at the viewport would call brighter.
            return 0.299f * color.x + 0.587f * color.y + 0.114f * color.z;
        }
    }

    const char* toString(SceneLightKind kind)
    {
        switch (kind)
        {
            case SceneLightKind::Point: return "Point";
            case SceneLightKind::Spot: return "Spot";
            default: return "Directional";
        }
    }

    bool sceneLightUsesDirection(SceneLightKind kind)
    {
        return kind == SceneLightKind::Directional || kind == SceneLightKind::Spot;
    }

    bool sceneLightUsesPosition(SceneLightKind kind)
    {
        return kind == SceneLightKind::Point || kind == SceneLightKind::Spot;
    }

    bool sceneLightUsesRange(SceneLightKind kind) { return sceneLightUsesPosition(kind); }

    std::vector<SceneLight> collectSceneLights(const SceneDocument& scene)
    {
        std::vector<SceneLight> lights;

        for (const StudioEntity& entity : scene.getEntities())
        {
            if (!entity.isEnabled()) { continue; }

            const StudioComponent* component = entity.findComponent(BuiltinComponentIds::kLight);
            if (component == nullptr) { continue; }

            const std::optional<WorldTransform> transform =
                computeWorldTransform(scene, entity.getId());
            if (!transform.has_value()) { continue; }

            SceneLight light;
            light.entityId = entity.getId();
            light.kind = parseLightKind(
                component->getProperty("kind").get<PropertyValue::EnumValue>().name);
            light.position = transform->position;
            light.direction =
                normalize(rotate(transform->rotation, StudioVector3{0.0f, 0.0f, 1.0f}));
            light.color = component->getProperty("color").get<StudioColor>();
            light.intensity = component->getProperty("intensity").get<float>();
            light.range = component->getProperty("range").get<float>();

            // Authored in degrees and carried in radians (`plan.md` STUDIO-20003). Clamped so the
            // inner cone cannot exceed the outer one: a hand-edited scene where it does would
            // divide by a negative band and light the *outside* of the cone, which looks like a
            // renderer fault rather than a bad number.
            constexpr float kDegreesToRadians = 3.14159265358979323846f / 180.0f;
            light.outerAngle = component->getProperty("outerAngle").get<float>(35.0f)
                               * kDegreesToRadians;
            light.innerAngle = std::min(component->getProperty("innerAngle").get<float>(25.0f)
                                            * kDegreesToRadians,
                                        light.outerAngle);

            lights.push_back(light);
        }

        return lights;
    }

    EffectLighting computeEffectLighting(const std::vector<SceneLight>& lights,
                                         const StudioVector3& targetWorld)
    {
        EffectLighting lighting;

        struct Candidate
        {
            EffectDirectionalLight light;
            float brightness = 0.0f;
        };

        std::vector<Candidate> candidates;
        candidates.reserve(lights.size());

        // The one punctual slot, filled by the brightest point or spot light here (`plan.md`
        // STUDIO-20003). Chosen before the directional reduction rather than after, so the light
        // that goes to CNA's punctual extension is not also approximated into a directional slot
        // -- the same lamp counted twice would be a lamp at twice its brightness.
        const SceneLight* punctual = nullptr;
        float punctualBrightness = 0.0f;

        for (const SceneLight& light : lights)
        {
            if (!sceneLightUsesPosition(light.kind)) { continue; }

            const float falloff = falloffAt(light, targetWorld);
            if (falloff <= 0.0f) { continue; }

            const float brightness =
                brightnessOf(toLinearScaled(light.color, light.intensity, falloff));
            if (brightness <= punctualBrightness) { continue; }

            punctual = &light;
            punctualBrightness = brightness;
        }

        for (const SceneLight& light : lights)
        {
            // The punctual one is sent whole, below, rather than flattened into a direction.
            if (&light == punctual) { continue; }

            const float falloff = falloffAt(light, targetWorld);
            if (falloff <= 0.0f) { continue; }

            const StudioVector3 color = toLinearScaled(light.color, light.intensity, falloff);
            const float brightness = brightnessOf(color);
            if (brightness <= 0.0f) { continue; }

            EffectDirectionalLight effectLight;
            effectLight.direction = directionAt(light, targetWorld);
            effectLight.diffuseColor = color;
            effectLight.specularColor = color;

            candidates.push_back(Candidate{effectLight, brightness});
        }

        if (punctual != nullptr)
        {
            lighting.hasPunctual = true;
            lighting.punctual.kind = punctual->kind;
            lighting.punctual.position = punctual->position;
            lighting.punctual.direction = punctual->direction;
            lighting.punctual.range = punctual->range;
            lighting.punctual.innerAngle = punctual->innerAngle;
            lighting.punctual.outerAngle = punctual->outerAngle;

            // Intensity is folded in and the *falloff is not*: the effect measures distance per
            // pixel from the position above, which is the whole reason this is worth sending
            // rather than approximating.
            lighting.punctual.diffuseColor =
                toLinearScaled(punctual->color, punctual->intensity, 1.0f);
        }

        // Nothing reaches this point in the world. Say "use the default" rather than "use these
        // zero lights": the two are the same arithmetic and completely different on screen, and
        // the one a user can act on is the model they can see.
        //
        // A punctual light counts as something reaching it, even with no directional candidate --
        // a scene lit by one lamp is a lit scene, and falling back to XNA's rig there would wash
        // out exactly the case the punctual slot exists for.
        if (candidates.empty() && !lighting.hasPunctual) { return lighting; }

        // Brightest first, and only where they differ -- `std::stable_sort` so that two lights of
        // equal brightness keep document order and the picture does not change from frame to frame
        // for reasons nothing in the scene explains.
        std::stable_sort(candidates.begin(), candidates.end(),
                         [](const Candidate& a, const Candidate& b)
                         { return a.brightness > b.brightness; });

        lighting.useDefaultLighting = false;
        lighting.lightCount = std::min(candidates.size(), lighting.lights.size());
        for (std::size_t i = 0; i < lighting.lightCount; ++i)
        {
            lighting.lights[i] = candidates[i].light;
        }

        return lighting;
    }
}
