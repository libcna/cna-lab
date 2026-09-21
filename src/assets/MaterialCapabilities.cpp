// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Assets/MaterialCapabilities.hpp"

namespace CNA::Studio
{
    std::vector<StudioMaterialCapabilityIssue> studioMaterialCapabilityIssues(
        std::string_view effectName, const MaterialDocument& material)
    {
        std::vector<StudioMaterialCapabilityIssue> issues;

        // `PbrEffect` takes every slot this document has, including the occlusion map and glTF's
        // alpha coverage, so there is nothing to report. Anything else -- an effect from a later
        // CNA, "none" where the device could not make one, or the empty name a headless preview
        // gives -- reports nothing either, deliberately: see the header.
        if (effectName != "BasicEffect") { return issues; }

        // The order the editor lists the slots in, so a user reading the warnings and a user
        // reading the rows are reading the same material top to bottom.
        if (material.normalTexture.isValid())
        {
            issues.push_back({"Normal map",
                              "BasicEffect has no normal mapping. The surface is drawn with the "
                              "geometry's own normals, which is flatter rather than wrong."});
        }
        if (material.metallicRoughnessTexture.isValid())
        {
            issues.push_back({"Metallic-roughness map",
                              "BasicEffect does not sample it. The metallic and roughness "
                              "*factors* below still shape the highlight, so the material varies "
                              "as a whole and not across its surface."});
        }
        if (material.emissiveTexture.isValid())
        {
            issues.push_back({"Emissive map",
                              "BasicEffect does not sample it. The emissive colour still applies "
                              "to the whole surface."});
        }
        if (material.occlusionTexture.isValid())
        {
            issues.push_back({"Occlusion map",
                              "BasicEffect does not sample it. Nothing stands in for it; the "
                              "creases this map darkens are lit like the rest of the surface."});
        }

        // Not the blend mode: that is the device's blend state rather than the effect's, so a
        // `Blend` material draws correctly on both. `Mask` is the one that needs a shader.
        if (material.alphaMode == MeshAlphaMode::Mask)
        {
            issues.push_back({"Mask alpha",
                              "BasicEffect has no alpha test, so the cut-out is not made and the "
                              "material draws solid. Blend is the mode that works on this build."});
        }

        return issues;
    }

    std::vector<StudioMaterialCapabilityIssue> studioLightCapabilityIssues(
        std::string_view effectName, std::string_view lightKind)
    {
        std::vector<StudioMaterialCapabilityIssue> issues;

        // `PbrEffect` implements CNA's punctual-light extension, so it draws both kinds properly.
        // Anything unrecognised is silent for the reason the header gives.
        if (effectName != "BasicEffect") { return issues; }

        if (lightKind == "Point")
        {
            issues.push_back({"Point light",
                              "BasicEffect has only directional lights, so this one is aimed at "
                              "each object as it is drawn. It behaves like a point light between "
                              "objects and cannot fall off across a large one."});
        }
        else if (lightKind == "Spot")
        {
            issues.push_back({"Spot light",
                              "BasicEffect has only directional lights, so this one is drawn as a "
                              "point light: its position, direction and range are used and its "
                              "cone is not."});
        }

        return issues;
    }

    std::vector<StudioMaterialCapabilityIssue> studioEnvironmentCapabilityIssues(
        std::string_view effectName, bool lightsTheScene)
    {
        std::vector<StudioMaterialCapabilityIssue> issues;

        // A sky that is only drawn needs no effect support: `Skybox` carries its own shader.
        if (!lightsTheScene) { return issues; }

        // `PbrEffect` takes an `ImageBasedLightEXT`. Anything unrecognised is silent, for the
        // reason the header gives about the other two.
        if (effectName != "BasicEffect") { return issues; }

        issues.push_back({"Image-based lighting",
                          "BasicEffect cannot sample an environment, so the sky is drawn behind "
                          "the scene and lights nothing in it. Only PbrEffect takes an image-based "
                          "light, and this build draws through BasicEffect."});
        return issues;
    }
}
