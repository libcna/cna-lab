// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Scene/SceneSelectionOverlay.hpp
 * @brief What the viewport draws to say *this is what you have selected*.
 *
 * `plan.md` CORE-03 (`STUDIO-35053`).
 *
 * ### What was already there, and what was not
 *
 * A selected entity has been outlined since the viewport was written — a box in the 2D renderer, a
 * thicker box in the selection colour in the 3D wireframe. What was missing is everything a
 * *multi-selection* needs. Eight selected crates produced eight identical boxes, with nothing
 * saying they were one selection rather than eight things that happen to be outlined, and nothing
 * at all marking the point a rotation would turn about — which is the one thing a user has to know
 * before they press R.
 *
 * So this adds two marks and no more: a combined bounds around everything selected, and a cross at
 * the pivot the gizmo is using.
 *
 * ### Why it is geometry rather than drawing
 *
 * The 2D viewport draws through CNA's sprite batch and the 3D one through a wire list, on a machine
 * with a GPU. Computing *where the marks go* here, in `cna-studio-scene`, is what makes the
 * interesting half — does the pivot follow the pivot mode, does the combined box cover every
 * selected entity, does a single selection get no combined box — checkable in the dependency-free
 * suite, on a machine with no CNA and no display.
 *
 * It is also what keeps the two viewports agreeing. A user who selects three entities in the 2D
 * view, switches to 3D and finds the pivot somewhere else has half a feature.
 */

#include <optional>
#include <vector>

#include "CNA/Studio/Core/Uuid.hpp"
#include "CNA/Studio/Scene/SceneTransform.hpp"
#include "CNA/Studio/Scene/StudioCamera3D.hpp"
#include "CNA/Studio/Scene/TransformGizmos.hpp"

namespace CNA::Studio
{
    class SceneDocument;

    /** @brief The marks the 2D viewport draws for a selection. */
    struct StudioSelectionOverlay2D
    {
        /** @brief One box per selected entity that has bounds, in world space. */
        std::vector<WorldBounds2D> outlines;

        /**
         * @brief A box round everything selected. Present only for a selection of more than one.
         *
         * Absent for a single entity because it would sit exactly on that entity's own outline,
         * which is two rectangles drawn at the same place in two colours — a picture of nothing.
         */
        std::optional<WorldBounds2D> combined;

        /**
         * @brief Where the gizmo turns and scales about, in world space.
         *
         * The same answer `computeSelectionPivot` gives the gizmo, asked here so the mark and the
         * manipulator cannot disagree. Absent when nothing selected has a transform.
         */
        std::optional<StudioVector2> pivot;

        /** @brief Whether there is anything to draw. */
        [[nodiscard]] bool isEmpty() const
        {
            return outlines.empty() && !combined.has_value() && !pivot.has_value();
        }
    };

    /** @brief The same marks for the 3D viewport, in world space. */
    struct StudioSelectionOverlay3D
    {
        /** @brief A box round everything selected. Present only for a selection of more than one. */
        std::optional<WorldBounds3D> combined;

        /** @brief Where the gizmo turns and scales about. */
        std::optional<StudioVector3> pivot;

        /** @brief Whether there is anything to draw. */
        [[nodiscard]] bool isEmpty() const
        {
            return !combined.has_value() && !pivot.has_value();
        }
    };

    /**
     * @brief Returns what the 2D viewport draws for @p selection.
     *
     * @param scene The scene.
     * @param selection The selected entities, in selection order — `Active` reads the last of them.
     * @param sizeProvider Supplies sprite dimensions, exactly as the picking path does.
     * @param pivotMode Which pivot the gizmo is using, so the mark lands where it does.
     */
    [[nodiscard]] StudioSelectionOverlay2D studioSelectionOverlay2D(
        const SceneDocument& scene, const std::vector<Uuid>& selection,
        const SpriteSizeProvider& sizeProvider,
        StudioPivotMode pivotMode = StudioPivotMode::Center);

    /**
     * @brief Returns what the 3D viewport draws for @p selection.
     *
     * No per-entity outlines: the wireframe already draws a box per selected entity, in the
     * selection colour and thicker, and a second identical box over it would be an overlay
     * arguing with itself.
     */
    [[nodiscard]] StudioSelectionOverlay3D studioSelectionOverlay3D(
        const SceneDocument& scene, const std::vector<Uuid>& selection,
        const SpriteSizeProvider& sizeProvider,
        StudioPivotMode pivotMode = StudioPivotMode::Center);
}
