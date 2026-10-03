// SPDX-License-Identifier: MS-PL
/**
 * @file SceneSelectionOverlay.cpp
 * @brief Where the selection marks go (`plan.md` CORE-03).
 */

#include "CNA/Studio/Scene/SceneSelectionOverlay.hpp"

#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/Scene/TransformGizmos3D.hpp"

namespace CNA::Studio
{
    StudioSelectionOverlay2D studioSelectionOverlay2D(const SceneDocument& scene,
                                                      const std::vector<Uuid>& selection,
                                                      const SpriteSizeProvider& sizeProvider,
                                                      StudioPivotMode pivotMode)
    {
        StudioSelectionOverlay2D overlay;
        if (selection.empty()) { return overlay; }

        bool anyBounds = false;
        WorldBounds2D combined;

        for (const Uuid& entityId : selection)
        {
            const std::optional<WorldBounds2D> bounds =
                computeEntityBounds2D(scene, entityId, sizeProvider);
            if (!bounds) { continue; }

            overlay.outlines.push_back(*bounds);
            combined = anyBounds ? WorldBounds2D::combine(combined, *bounds) : *bounds;
            anyBounds = true;
        }

        // Only for a real multi-selection, and only when more than one of them actually has
        // bounds. Two selected entities of which one is a camera would otherwise get a combined
        // box sitting exactly on the other one's outline -- two rectangles at the same place in
        // two colours, which is a picture of nothing.
        if (anyBounds && overlay.outlines.size() > 1) { overlay.combined = combined; }

        // Asked of the same function the gizmo asks, rather than recomputed here. A mark that
        // said the rotation would happen somewhere the rotation does not happen is worse than no
        // mark: it is a wrong answer to the question the user asked by looking.
        overlay.pivot = computeSelectionPivot(scene, selection, pivotMode);
        return overlay;
    }

    StudioSelectionOverlay3D studioSelectionOverlay3D(const SceneDocument& scene,
                                                      const std::vector<Uuid>& selection,
                                                      const SpriteSizeProvider& sizeProvider,
                                                      StudioPivotMode pivotMode)
    {
        StudioSelectionOverlay3D overlay;
        if (selection.empty()) { return overlay; }

        std::size_t measured = 0;
        WorldBounds3D combined = WorldBounds3D::makeEmpty();

        for (const Uuid& entityId : selection)
        {
            const std::optional<WorldBounds3D> bounds =
                computeEntityBounds3D(scene, entityId, sizeProvider);
            if (!bounds) { continue; }

            combined = WorldBounds3D::combine(combined, *bounds);
            ++measured;
        }

        if (measured > 1) { overlay.combined = combined; }

        overlay.pivot = computeSelectionPivot3D(scene, selection, pivotMode);
        return overlay;
    }
}
