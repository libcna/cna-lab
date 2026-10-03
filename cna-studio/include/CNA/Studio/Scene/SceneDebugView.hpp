// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Scene/SceneDebugView.hpp
 * @brief Viewport debug views: unlit, lighting only, and the material channels
 *        (`plan.md` STUDIO-11011).
 *
 * **These are not shading modes and they are not a second set of them.** `StudioViewportShading`
 * answers "solid, edges, or both"; a debug view answers "what colour is the surface". They compose:
 * a user can look at roughness in shaded-wireframe, and asking them to give up their wireframe to
 * see a channel would be a choice with no reason behind it.
 *
 * **Everything here is expressed as a material and a lighting environment**, which is what makes
 * the whole feature CNA-free. The batch a debug view produces is an ordinary `SceneModelBatch`:
 * the renderer draws it exactly as it draws any other, and there is no debug branch in the model
 * pass at all. Unlit is not a flag the effect is told about — it is an environment with a white
 * ambient and no directional lights, which is what `BasicEffect` and `PbrEffect` both already
 * compute to a flat albedo. A channel view is a material whose base colour *is* the channel.
 *
 * **What that bargain cannot buy, and the plan should not pretend it can.** A true normal buffer
 * — every pixel coloured by its interpolated surface normal — needs a shader that writes one, and
 * CNA exposes `BasicEffect` and `PbrEffect` and no seam for an effect of one's own (gap G-12 in
 * `docs/CNA-GAPS.md`). Studio is not entitled to invent one, and faking it by uploading a second
 * vertex buffer per model, coloured per vertex, would double the GPU memory of every mesh in the
 * project for a view nobody leaves on.
 *
 * So the normal view here is the other honest answer: the normals themselves, drawn as segments,
 * each one coloured by the direction it points. It answers the questions a normal buffer is opened
 * for — are these normals inverted, is this seam split, did the importer's Y mirror survive — and
 * it is a picture of the data rather than a picture of a shader nobody has.
 */

#include "CNA/Studio/Core/MeshData.hpp"
#include "CNA/Studio/Core/StudioMath.hpp"
#include "CNA/Studio/Scene/SceneLighting.hpp"

namespace CNA::Studio
{
    /** @brief What the 3D view colours a surface by. */
    enum class StudioDebugView
    {
        /** @brief The scene as authored. The default, and the only one a user works in. */
        None,

        /** @brief Base colour with the lights ignored: the albedo, as the artist set it. */
        Unlit,

        /**
         * @brief The lights alone, over a white surface.
         *
         * The complement of Unlit, and the pair is the point: a model that looks wrong is either
         * wrong in its texture or wrong in its lighting, and these two separate the question
         * without needing a second scene to compare against.
         */
        LightingOnly,

        /** @brief Metalness as grey, from black (dielectric) to white (metal). */
        Metallic,

        /** @brief Roughness as grey, from black (mirror) to white (fully diffuse). */
        Roughness,

        /** @brief The surface normals, drawn as coloured segments. See this file's header. */
        Normals,

        Count
    };

    /** @brief The name of @p view, for a menu row or the viewport overlay. */
    [[nodiscard]] const char* studioDebugViewName(StudioDebugView view);

    /**
     * @brief True when @p view keeps the scene's own lights.
     *
     * Only `None` and `LightingOnly` do. A channel drawn with the lights on would be the channel
     * multiplied by whatever happens to be shining on it, which is a picture of the lighting
     * wearing the channel's name.
     */
    [[nodiscard]] bool studioDebugViewIsLit(StudioDebugView view);

    /** @brief True when @p view asks the wireframe for the normal segments. */
    [[nodiscard]] bool studioDebugViewDrawsNormals(StudioDebugView view);

    /**
     * @brief The material @p view draws @p source as.
     *
     * Returns @p source unchanged for `None`, so a caller can apply this unconditionally.
     *
     * Textures are dropped by every view except `Unlit`, which keeps the base-colour map because
     * the map *is* the albedo. A roughness view that sampled the base-colour texture would be
     * showing a number multiplied by a picture.
     */
    [[nodiscard]] MeshMaterial studioDebugMaterial(StudioDebugView view, const MeshMaterial& source);

    /**
     * @brief The lighting environment @p view draws in.
     *
     * Returns @p source unchanged when `studioDebugViewIsLit`. Otherwise a white ambient with no
     * directional lights, which both effects compute to the base colour exactly — see this file's
     * header for why that is the whole of "unlit" here.
     */
    [[nodiscard]] EffectLighting studioDebugLighting(StudioDebugView view,
                                                     const EffectLighting& source);

    /**
     * @brief The colour a normal segment pointing @p normal is drawn in.
     *
     * The tangent-space convention every normal map is written in — `n * 0.5 + 0.5` — so a
     * surface facing +X is red, +Y green, +Z blue, and a user who has ever opened a normal map
     * reads it without being told. @p normal is taken in world space and normalised here; a zero
     * vector is drawn mid-grey rather than treated as an error, because a mesh with a degenerate
     * normal is exactly what somebody opens this view to find.
     */
    [[nodiscard]] StudioColor studioNormalColor(const StudioVector3& normal);
}
