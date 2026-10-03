// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Scene/SceneLock.hpp
 * @brief Locking an entity against being picked or moved in the viewport.
 *
 * `plan.md` STUDIO-13005. A lock is an *authoring* concern and nothing else: the finished game has
 * no notion of a thing the designer told the editor to keep its hands off. So it lives in the
 * entity's studio state (ANALYSIS.md decision D-07), which the runtime scene compiler drops
 * wholesale, rather than as a field beside `enabled` — which is a real runtime property and the
 * reason the two are not the same kind of flag however alike the two eye-shaped buttons look.
 *
 * Absent means unlocked, so a scene written before this existed reads back exactly as it was and
 * an unlocked entity costs nothing to store.
 *
 * Deliberately CNA-free. What a lock *means* is a question about the document, and every path that
 * has to honour it — the 2D picker, the 3D picker, the band select, the gizmos — is a headless
 * function that must be able to ask without a window.
 */

#include <string_view>
#include <vector>

#include "CNA/Studio/Core/Uuid.hpp"

namespace CNA::Studio
{
    class SceneDocument;
    class StudioEntity;

    /** @brief The studio-state key holding the lock. */
    inline constexpr const char* kStudioLockedKey = "locked";

    /**
     * @brief Whether @p entity is locked in its own right.
     *
     * Not the question the viewport asks — that is @ref isEntityLocked, which also looks upwards —
     * but the one the outliner's toggle asks, because a row's button has to show and change the
     * entity's own state rather than an inherited one.
     */
    [[nodiscard]] bool isEntityLockedItself(const StudioEntity& entity);

    /**
     * @brief Whether @p entityId is locked, by itself or by anything above it.
     *
     * A lock inherits downwards, which is the whole reason to have one: a designer locks the
     * finished level geometry once, at the group, rather than forty times at the pieces. An editor
     * where locking a group left its children pickable would be one where the gesture does almost
     * nothing.
     */
    [[nodiscard]] bool isEntityLocked(const SceneDocument& scene, const Uuid& entityId);

    /**
     * @brief Drops every locked entity from @p ids, keeping the order of the rest.
     *
     * The shape every caller wants: a band select, a frame-selection or a transform starts with a
     * list and has to continue with the part of it the user is allowed to touch.
     */
    [[nodiscard]] std::vector<Uuid> withoutLocked(const SceneDocument& scene,
                                                  const std::vector<Uuid>& ids);
}
