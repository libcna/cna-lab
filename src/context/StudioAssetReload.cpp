// SPDX-License-Identifier: MS-PL
/**
 * @file StudioAssetReload.cpp
 * @brief Noticing that an asset changed outside Studio, and letting go of what went stale.
 */

#include "CNA/Studio/StudioAssetReload.hpp"

#include "CNA/Studio/Assets/AssetImporters.hpp"
#include "CNA/Studio/Assets/AssetWatcher.hpp"
#include "CNA/Studio/StudioContext.hpp"

#include <string>

namespace CNA::Studio
{
    namespace
    {
        /** @brief The asset's source path, or its id when the record has gone with it. */
        std::string nameOf(const StudioContext& context, const Uuid& assetId)
        {
            const AssetRecord* record = context.getAssets().find(assetId);
            return record != nullptr ? record->sourcePath : assetId.toString();
        }
    }

    StudioAssetReloadResult studioPollAssetChanges(AssetWatcher& watcher, StudioContext& context,
                                                   const StudioAssetReloadSinks& sinks,
                                                   double deltaSeconds)
    {
        StudioAssetReloadResult result;

        const AssetWatchResult changes = watcher.poll(context.getAssets(), deltaSeconds);
        if (!changes.hasChanges()) { return result; }

        // **One message for a batch, not one per asset** (`plan.md` STUDIO-16004). The player
        // rescans the whole asset directory on *every* reload it is told about -- it has to, or
        // the record it looks up still carries the size and timestamp from before the change. So
        // fifty files touched at once, which is what a texture export or a `git checkout` looks
        // like, was fifty full scans inside the running game.
        //
        // The player already understands a nil id as "everything", and says so: *"A nil id means
        // 'everything', which is what a project-wide change is best reported as rather than as one
        // message per asset."* Studio was the half not doing it.
        const std::size_t reloadable = changes.changed.size() + changes.restored.size();
        const bool reloadEverythingAtOnce = reloadable > 1;

        for (const Uuid& assetId : changes.changed)
        {
            // Dropping the cached texture is what makes the change visible. Without it the editor
            // would report the edit and go on drawing the art from before it. A mesh is the same
            // bargain in the 3D view: `MeshCache` holds an imported model until told otherwise, so
            // an edited .gltf would keep drawing the shape it had when the project was opened.
            if (sinks.invalidateRendered) { sinks.invalidateRendered(assetId); }
            context.getMeshes().invalidate(assetId);
            if (!reloadEverythingAtOnce && sinks.reloadInPlayer) { sinks.reloadInPlayer(assetId); }

            context.log(LogSeverity::Info,
                        "Reloaded '" + nameOf(context, assetId) + "' after an external change.");
        }

        for (const Uuid& assetId : changes.restored)
        {
            if (sinks.invalidateRendered) { sinks.invalidateRendered(assetId); }
            context.getMeshes().invalidate(assetId);
            if (!reloadEverythingAtOnce && sinks.reloadInPlayer) { sinks.reloadInPlayer(assetId); }

            context.log(LogSeverity::Info, "'" + nameOf(context, assetId) + "' is back.");
        }

        // The single message, sent after the caches are dropped so the editor and the player are
        // not briefly disagreeing about what is current. Still nothing for a *removal*: a file
        // that has gone is not a file the running game should be told to re-read.
        if (reloadEverythingAtOnce && sinks.reloadInPlayer) { sinks.reloadInPlayer(Uuid{}); }

        for (const Uuid& assetId : changes.removed)
        {
            // Forgotten rather than kept: a model whose file has gone should stop being drawn, and
            // a cache that held the last good copy would show a mesh that is no longer there.
            context.getMeshes().invalidate(assetId);

            context.log(LogSeverity::Warning,
                        "'" + nameOf(context, assetId)
                            + "' has gone missing. Anything referencing it is listed in Missing "
                              "References.");
        }

        // The pixel size of a texture that just changed is no longer the one on record -- so the
        // assets that moved are *reported* as needing a reimport rather than re-read here. Two
        // things were wrong with reading them here: it was the whole project rather than the files
        // that changed, and it was on the frame, where a glTF parse does not belong
        // (`plan.md` STUDIO-10011).
        result.needsReimport.reserve(changes.changed.size() + changes.restored.size());
        result.needsReimport.insert(result.needsReimport.end(), changes.changed.begin(),
                                    changes.changed.end());
        result.needsReimport.insert(result.needsReimport.end(), changes.restored.begin(),
                                    changes.restored.end());

        result.changed = changes.changed.size();
        result.restored = changes.restored.size();
        result.removed = changes.removed.size();
        return result;
    }
}
