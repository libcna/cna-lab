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
}
