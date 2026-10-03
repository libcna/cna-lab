// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Scene/GameCamera.hpp
 * @brief What the *game* sees: the view a scene's own camera describes.
 *
 * The editor's camera is a tool -- the user pans and zooms it and it belongs to nobody but them.
 * The game's camera is data: an entity with a `CNA.Camera` component, its position in the scene and
 * its orthographic size in world units. `cna-player` needs the second one, and so does anything
 * that ever asks "what will this look like when it runs".
 *
 * CNA-free and pure, like everything else in this module, so the answer can be checked in CI
 * against a document rather than against a screenshot.
 */

#include "CNA/Studio/Core/StudioMath.hpp"
#include "CNA/Studio/Core/Uuid.hpp"
#include "CNA/Studio/Scene/StudioCamera2D.hpp"
#include "CNA/Studio/Scene/StudioCamera3D.hpp"

namespace CNA::Studio
{
    class SceneDocument;

    /** @brief The view a scene's primary camera describes, ready to render with. */
    struct GameView
    {
        /** @brief Positioned and zoomed so the camera's orthographic size fills the height. */
        StudioCamera2D camera;

        /**
         * @brief The same view as a 3D camera, filled in when @ref perspective is set.
         *
         * `plan.md` STUDIO-20007. `CNA.Camera` has carried a `projection` since Phase 1 and the
         * game view read every other property and ignored that one, so it drew a 3D scene through
         * a *2D sprite pass*: no models, no lighting, nothing but the sprites and the clear
         * colour. "What will a player see" answered with a picture of almost none of the scene.
         *
         * Both cameras are filled on every call rather than one of them. The 2D one is what a
         * camera *preview* inset draws through whatever the projection says, and a `GameView` with
         * a default-constructed camera in the field a caller happened to read is the kind of empty
         * that looks like a placed camera at the origin.
         */
        StudioCamera3D camera3D;

        /**
         * @brief Which of the two cameras describes this view.
         *
         * Read from the camera component's `projection`. False for a scene with no camera at all,
         * because the fallback is the 2D origin-at-1:1 view this has always used -- a scene
         * without a camera is not a scene whose projection anybody has stated.
         */
        bool perspective = false;

        /** @brief The colour to clear to, from the camera's own `clearColor`. */
        StudioColor clearColor{100, 149, 237, 255};

        /** @brief The camera entity this came from, or a nil id when the scene has none. */
        Uuid cameraId;

        /** @brief True when a real camera was found rather than the fallback used. */
        [[nodiscard]] bool hasCamera() const { return cameraId.isValid(); }
    };

    /**
     * @brief Returns the view @p scene's primary camera describes at @p viewportSize pixels.
     *
     * `orthographicSize` is the visible **height** in world units -- the descriptor says so and the
     * inspector shows it -- so the zoom is the pixel height divided by it. Width follows from the
     * viewport's aspect, which is what makes a window resize show more of the world rather than
     * stretching what was already there.
     *
     * **A perspective camera looks along its entity's own forward axis, which is +Z rotated by its
     * rotation.** That is the one convention this codebase has for "which way is an entity
     * facing": `SceneLighting` established it for `CNA.Light` and writes down why -- the axis that
     * points into a Y-down, XY-plane world. A camera answering a different one would mean Studio
     * had two answers to the same question about the same transform. It is deliberately *not* the
     * editor orbit camera's zero pose, which is user state rather than an entity's rotation and is
     * free to start wherever suits an orbit.
     *
     * Falls back to a camera centred on the origin at 1:1 when the scene has no enabled camera. A
     * player that refused to draw a scene without one would be unable to show the very scene a user
     * is trying to fix; drawing it from the origin is wrong in a way they can see and act on.
     *
     * Where several cameras claim to be primary, the first in document order wins -- and validation
     * already reports the duplicate (`duplicate-primary-camera`), so the editor has said so before
     * the player has to choose.
     */
    [[nodiscard]] GameView computeGameView(const SceneDocument& scene, const StudioVector2& viewportSize);

    /**
     * @brief Returns the view @p cameraId describes, whether or not it is the scene's primary one.
     *
     * `plan.md` STUDIO-11012. What the editor's camera preview needs: a user who selects the second
     * of three cameras wants to see through *that* one, and "primary" is a property of the scene
     * rather than a question about the camera in front of them.
     *
     * Falls back to the same origin-at-1:1 view when @p cameraId is not an entity, or is an entity
     * with no `CNA.Camera` on it -- the preview then shows a view that is wrong in a way the user
     * can see, which is what `computeGameView` does with a scene that has no camera and for the
     * same reason.
     *
     * A *disabled* camera is still answered for, deliberately, and this is where the two functions
     * differ on purpose. `computeGameView` skips disabled entities because a disabled camera is not
     * in the game. A preview is a question about the entity the user has selected, and refusing to
     * show one because it is switched off would leave them aiming it blind.
     */
    [[nodiscard]] GameView computeGameViewFor(const SceneDocument& scene, const Uuid& cameraId,
                                              const StudioVector2& viewportSize);
}
