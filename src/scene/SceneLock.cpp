// SPDX-License-Identifier: MS-PL

#include "CNA/Studio/Scene/SceneLock.hpp"

#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/Scene/StudioEntity.hpp"

namespace CNA::Studio
{
    bool isEntityLockedItself(const StudioEntity& entity)
    {
        const auto found = entity.getStudioState().find(kStudioLockedKey);
        if (found == entity.getStudioState().end()) { return false; }

        // `get<bool>` rather than a checked read: a scene hand-edited to hold a string here should
        // come back unlocked rather than take the editor down, and "unlocked" is the safe way to be
        // wrong -- the user can see the entity and fix it, where a stuck lock is a thing they
        // cannot touch and cannot explain.
        return found->second.get<bool>(false);
    }

    bool isEntityLocked(const SceneDocument& scene, const Uuid& entityId)
    {
        // Upwards from the entity rather than downwards from the roots: the walk is one step per
        // ancestor and stops at the first lock, where marking a subtree would cost the whole scene
        // on every pick.
        //
        // The loop is bounded by the document's acyclicity invariant, which `SceneDocument`
        // enforces on every reparent -- there is no cycle for this to spin in.
        Uuid current = entityId;
        while (current.isValid())
        {
            const StudioEntity* entity = scene.findEntity(current);
            if (entity == nullptr) { return false; }
            if (isEntityLockedItself(*entity)) { return true; }
            current = entity->getParentId();
        }

        return false;
    }

    std::vector<Uuid> withoutLocked(const SceneDocument& scene, const std::vector<Uuid>& ids)
    {
        std::vector<Uuid> kept;
        kept.reserve(ids.size());
        for (const Uuid& id : ids)
        {
            if (!isEntityLocked(scene, id)) { kept.push_back(id); }
        }
        return kept;
    }
}
