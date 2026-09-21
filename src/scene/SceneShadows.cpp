// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Scene/SceneShadows.hpp"

#include <cmath>

#include "CNA/Studio/Scene/SceneModels.hpp"

namespace CNA::Studio
{
    namespace
    {
        /** @brief The perceptual weight `computeEffectLighting` sorts its candidates by. */
        float brightnessOf(const StudioVector3& color)
        {
            return 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
        }
    }

    SceneShadowPlan planSceneShadows(const SceneModelBatch& batch,
                                     const std::vector<SceneLight>& lights)
    {
        SceneShadowPlan plan;

        // The brightest enabled directional light, by the same rule and for the same reason the
        // three effect slots are filled: a distant sun matters more than a dim fill, and document
        // order is not something a user arranges deliberately.
        const SceneLight* sun = nullptr;
        float best = 0.0f;

        for (const SceneLight& light : lights)
        {
            if (light.kind != SceneLightKind::Directional) { continue; }

            const StudioVector3 color{
                static_cast<float>(light.color.r) / 255.0f * light.intensity,
                static_cast<float>(light.color.g) / 255.0f * light.intensity,
                static_cast<float>(light.color.b) / 255.0f * light.intensity};

            const float brightness = brightnessOf(color);
            if (brightness <= best) { continue; }

            sun = &light;
            best = brightness;
            plan.color = color;
        }

        if (sun == nullptr) { return plan; }

        // The casters' bounds rather than the scene's, so a distant entity nobody can see does not
        // halve the shadow resolution of everything they can.
        for (const ModelDraw& draw : batch.draws)
        {
            if (draw.receivesShadow) { ++plan.receivers; }
            if (!draw.castsShadow || draw.mesh == nullptr) { continue; }

            ++plan.casters;
            plan.casterBounds = WorldBounds3D::combine(
                plan.casterBounds,
                transformBounds3D(WorldBounds3D{draw.mesh->boundsMin, draw.mesh->boundsMax},
                                  draw.world));
        }

        if (plan.casters == 0) { return plan; }

        plan.enabled = true;
        plan.direction = normalize(sun->direction);
        return plan;
    }
}
