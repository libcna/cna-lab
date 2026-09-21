// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Assets/MaterialPreview.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Studio
{
    namespace
    {
        /** @brief The key light, in the preview's own space. Normalised at the point of use. */
        constexpr float kLightX = -0.45f;
        constexpr float kLightY = 0.65f;
        constexpr float kLightZ = 0.61f;

        /** @brief A floor under the unlit side, so a sphere's dark half is a shape and not a hole. */
        constexpr float kAmbient = 0.14f;

        /**
         * @brief How much of the sphere the frame holds.
         *
         * Slightly more than the sphere, so it does not touch the edges: a preview cropped to its
         * own silhouette reads as a square of colour rather than as a ball.
         */
        constexpr float kExtent = 1.18f;

        [[nodiscard]] unsigned char toByte(float value)
        {
            return static_cast<unsigned char>(
                std::lround(std::clamp(value, 0.0f, 1.0f) * 255.0f));
        }
    }

    StudioThumbnail studioRenderMaterialPreview(const MaterialDocument& material,
                                                std::uint32_t edge)
    {
        StudioThumbnail thumbnail;
        if (edge == 0) { return thumbnail; }

        thumbnail.width = edge;
        thumbnail.height = edge;
        thumbnail.pixels.assign(static_cast<std::size_t>(edge) * edge * 4u, 0);

        // The Blinn-Phong pair the renderer would draw this with, derived here rather than
        // invented: a preview shaded by its own rules would be a picture of this function.
        const MeshMaterial shaded = material.toMeshMaterial();

        const float lightLength =
            std::sqrt(kLightX * kLightX + kLightY * kLightY + kLightZ * kLightZ);
        const float lx = kLightX / lightLength;
        const float ly = kLightY / lightLength;
        const float lz = kLightZ / lightLength;

        // Orthographic, looking down -Z, so the eye direction is constant and the half-vector is
        // too. Perspective on a sphere this size changes nothing a user could name.
        const float hx = lx;
        const float hy = ly;
        const float hz = lz + 1.0f;
        const float halfLength = std::sqrt(hx * hx + hy * hy + hz * hz);

        // Blinn-Phong's energy normalisation, capped. Without it a sharp highlight is a *dimmer*
        // picture than a broad one -- the same reflectance spread over fewer pixels -- which is
        // backwards, and on a dielectric, whose reflectance is 0.04, it leaves roughness worth
        // about ten levels out of 255. A preview that cannot show roughness is a preview of the
        // base colour.
        //
        // Softened and capped, and both are preview decisions stated as such. The exact factor
        // for a mirror-smooth material is around a hundred, which does not brighten the highlight
        // so much as saturate every channel across a quarter of the sphere -- and a sphere whose
        // highlight is white to the edges tells a user nothing about its colour, which is the
        // thing they came to the thumbnail for. The square root keeps the *ordering* of
        // roughness intact while leaving the hue readable, and the cap holds the sharpest
        // materials to a bright spot rather than a bleached one. A thumbnail is a picture rather
        // than a render, and this is the one place that difference is spent.
        const float normalisation =
            std::min(std::sqrt((shaded.specularPower + 8.0f) / 8.0f), 6.0f);

        // A `Blend` material is see-through and every other mode is solid: `Mask` cuts pixels out
        // rather than fading them, and the preview has no texture to cut against, so it draws as
        // the solid material a mask is everywhere its map keeps.
        const float surfaceAlpha =
            material.alphaMode == MeshAlphaMode::Blend ? std::clamp(material.alpha, 0.0f, 1.0f)
                                                       : 1.0f;

        for (std::uint32_t y = 0; y < edge; ++y)
        {
            for (std::uint32_t x = 0; x < edge; ++x)
            {
                // Pixel centres, so the sphere is symmetric about the middle of the image rather
                // than half a pixel off it.
                const float u =
                    ((static_cast<float>(x) + 0.5f) / static_cast<float>(edge) * 2.0f - 1.0f)
                    * kExtent;

                // Negated, because an image's rows run downwards and the light is above.
                const float v =
                    -(((static_cast<float>(y) + 0.5f) / static_cast<float>(edge) * 2.0f - 1.0f))
                    * kExtent;

                const float radial = u * u + v * v;

                // An early out rather than the silhouette itself: the coverage term below gives
                // every pixel beyond the sphere an alpha of zero anyway, so this changes the
                // picture not at all and the shading cost a great deal. Worth saying, because a
                // reader who took it for the silhouette would put the transparency at risk the
                // next time this loop is rearranged.
                if (radial > 1.0f) { continue; }

                const float nz = std::sqrt(std::max(0.0f, 1.0f - radial));
                const float diffuse = std::max(0.0f, u * lx + v * ly + nz * lz);

                const float specularDot =
                    std::max(0.0f, (u * hx + v * hy + nz * hz) / halfLength);
                const float specular =
                    diffuse > 0.0f ? std::pow(specularDot, shaded.specularPower) * normalisation
                                   : 0.0f;

                const auto channel = [&](float base, float spec, float emissive) {
                    return base * (kAmbient + diffuse) + spec * specular + emissive;
                };

                const std::size_t offset =
                    (static_cast<std::size_t>(y) * edge + x) * 4u;
                thumbnail.pixels[offset + 0] = toByte(channel(
                    material.diffuseColor.x, shaded.specularColor.x, material.emissiveColor.x));
                thumbnail.pixels[offset + 1] = toByte(channel(
                    material.diffuseColor.y, shaded.specularColor.y, material.emissiveColor.y));
                thumbnail.pixels[offset + 2] = toByte(channel(
                    material.diffuseColor.z, shaded.specularColor.z, material.emissiveColor.z));

                // Antialiased at the silhouette by the coverage of the last half-pixel, because a
                // hard-edged circle at 128 pixels is visibly a staircase and the fix is one line.
                const float distance = std::sqrt(radial);
                const float pixel = kExtent / static_cast<float>(edge) * 2.0f;
                const float coverage =
                    std::clamp((1.0f - distance) / std::max(pixel, 1e-6f), 0.0f, 1.0f);

                thumbnail.pixels[offset + 3] = toByte(surfaceAlpha * coverage);
            }
        }

        return thumbnail;
    }
}
