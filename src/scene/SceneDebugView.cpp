// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Scene/SceneDebugView.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Studio
{
    namespace
    {
        /** @brief @p value clamped to 0..1 and written as a grey. */
        StudioVector3 grey(float value)
        {
            const float clamped = std::clamp(value, 0.0f, 1.0f);
            return StudioVector3{clamped, clamped, clamped};
        }

        /** @brief A material with nothing on it: no maps, no emission, no highlight. */
        MeshMaterial flat(const StudioVector3& color)
        {
            MeshMaterial material;
            material.diffuseColor = color;
            material.emissiveColor = StudioVector3{0.0f, 0.0f, 0.0f};
            material.specularColor = StudioVector3{0.0f, 0.0f, 0.0f};
            material.specularPower = 1.0f;
            material.alpha = 1.0f;
            return material;
        }
    }

    const char* studioDebugViewName(StudioDebugView view)
    {
        switch (view)
        {
            case StudioDebugView::None:         return "Default";
            case StudioDebugView::Unlit:        return "Unlit";
            case StudioDebugView::LightingOnly: return "Lighting Only";
            case StudioDebugView::Metallic:     return "Metallic";
            case StudioDebugView::Roughness:    return "Roughness";
            case StudioDebugView::Normals:      return "Normals";
            case StudioDebugView::Count:        break;
        }
        return "Default";
    }

    bool studioDebugViewIsLit(StudioDebugView view)
    {
        return view == StudioDebugView::None || view == StudioDebugView::LightingOnly;
    }

    bool studioDebugViewDrawsNormals(StudioDebugView view)
    {
        return view == StudioDebugView::Normals;
    }

    MeshMaterial studioDebugMaterial(StudioDebugView view, const MeshMaterial& source)
    {
        switch (view)
        {
            case StudioDebugView::None:
                return source;

            case StudioDebugView::Unlit:
            {
                // The base colour and its map, and nothing else. Emission is dropped because an
                // emissive surface is already unlit and would read as the brightest albedo in the
                // scene; the highlight is dropped because there is no light to make one.
                MeshMaterial material = flat(source.diffuseColor);
                material.diffuseTexturePath = source.diffuseTexturePath;
                material.alpha = source.alpha;
                return material;
            }

            case StudioDebugView::LightingOnly:
            {
                // White, so what is left on screen is the light. The highlight stays: a specular
                // response is light, and dropping it would hide the half of the lighting that is
                // hardest to get right.
                MeshMaterial material = flat(StudioVector3{1.0f, 1.0f, 1.0f});
                material.specularColor = source.specularColor;
                material.specularPower = source.specularPower;
                return material;
            }

            case StudioDebugView::Metallic:
                return flat(grey(source.metallic));

            case StudioDebugView::Roughness:
                return flat(grey(source.roughness));

            case StudioDebugView::Normals:
                // Dark, so the coloured segments over it are the brightest thing on screen. Not
                // black: a normal pointing away from the camera is drawn on the far side of the
                // surface, and against black there would be no surface to see it against.
                return flat(StudioVector3{0.12f, 0.12f, 0.14f});

            case StudioDebugView::Count:
                break;
        }
        return source;
    }

    EffectLighting studioDebugLighting(StudioDebugView view, const EffectLighting& source)
    {
        if (studioDebugViewIsLit(view)) { return source; }

        // White ambient, no directional lights, and `useDefaultLighting` off so the renderer does
        // not reach for XNA's three-point rig instead. Both effects compute ambient times base
        // colour, so this is the base colour exactly -- see this file's header.
        EffectLighting lighting;
        lighting.useDefaultLighting = false;
        lighting.ambientColor = StudioVector3{1.0f, 1.0f, 1.0f};
        lighting.lightCount = 0;
        return lighting;
    }

    StudioColor studioNormalColor(const StudioVector3& normal)
    {
        const float magnitude = length(normal);

        // A degenerate normal is mid-grey rather than a division by zero. It is also exactly what
        // `n * 0.5 + 0.5` gives for a zero vector, so the two answers agree and the branch is only
        // here to keep the normalise from producing one.
        const StudioVector3 unit = magnitude > 1e-6f ? normalize(normal)
                                                     : StudioVector3{0.0f, 0.0f, 0.0f};

        const auto channel = [](float component) {
            const float mapped = std::clamp(component * 0.5f + 0.5f, 0.0f, 1.0f);
            return static_cast<std::uint8_t>(std::lround(mapped * 255.0f));
        };

        return StudioColor{channel(unit.x), channel(unit.y), channel(unit.z), 255};
    }
}
