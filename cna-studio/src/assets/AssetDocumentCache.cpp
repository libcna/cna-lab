// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Assets/AssetDocumentCache.hpp"

#include <set>

#include "CNA/Studio/Core/ComponentDescriptor.hpp"

namespace CNA::Studio
{
    StudioAssetDocumentCache::Entry& StudioAssetDocumentCache::entryFor(const AssetRecord& record,
                                                                        bool& outNeedsLoad)
    {
        Entry& entry = entries_[record.id];

        // The stamp the last scan or watcher poll saw, compared without asking the filesystem
        // anything. That is what makes this free on the frames where nothing changed, and it is
        // why the watcher is what makes an external edit visible here as well as in the browser.
        //
        // The path too: an asset that moved is the same document, but an entry keyed only on the
        // stamp would survive a move to a file that happens to be the same size.
        outNeedsLoad = !entry.loaded || entry.size != record.sourceSize
                    || entry.modifiedTime != record.sourceModifiedTime
                    || entry.sourcePath != record.sourcePath;

        if (outNeedsLoad)
        {
            entry.size = record.sourceSize;
            entry.modifiedTime = record.sourceModifiedTime;
            entry.sourcePath = record.sourcePath;
            entry.material.reset();
            entry.prefab.reset();
        }
        return entry;
    }

    std::optional<MaterialDocument> StudioAssetDocumentCache::resolvedMaterial(
        const AssetDatabase& assets, const Uuid& id)
    {
        const MaterialDocument* leaf = material(assets, id);
        if (leaf == nullptr) { return std::nullopt; }

        // Nearly every material is its own, and that case costs one copy and no walk at all.
        if (!leaf->parent.isValid()) { return *leaf; }

        // Leaf-first, applied root-first, so a parameter belongs to the nearest ancestor that
        // states it. Both guards for the reason `resolveMaterialDocument` gives: the visited set
        // catches a loop and the depth catches a chain that is merely absurd.
        //
        // Pointers into the cache rather than copies, which is safe because `entries_` is an
        // `unordered_map` -- inserting a level does not move the ones already in it, and nothing
        // on this path erases. An eviction policy added to `material` would have to take copies
        // here instead.
        constexpr std::size_t kMaximumDepth = 32;
        std::vector<const MaterialDocument*> chain;
        std::set<std::string> visited;

        Uuid current = id;
        while (current.isValid() && chain.size() < kMaximumDepth)
        {
            if (!visited.insert(current.toString()).second) { break; }

            const MaterialDocument* level = material(assets, current);

            // A broken parent leaves the chain where it is rather than losing the leaf: a
            // material whose ancestor will not read is better drawn with what it has than not
            // drawn at all, and the Inspector reports the break.
            if (level == nullptr) { break; }

            current = level->parent;
            chain.push_back(level);
        }

        if (chain.empty()) { return *leaf; }

        MaterialDocument resolved = *chain.back();
        for (auto level = chain.rbegin() + 1; level != chain.rend(); ++level)
        {
            applyMaterialOverrides(resolved, **level);
        }

        // The leaf's own identity: this is still that material, drawn with what it inherits
        // filled in.
        resolved.name = leaf->name;
        resolved.parent = leaf->parent;
        resolved.overridden = leaf->overridden;
        return resolved;
    }

    const MaterialDocument* StudioAssetDocumentCache::material(const AssetDatabase& assets,
                                                               const Uuid& id)
    {
        const AssetRecord* record = assets.find(id);
        if (record == nullptr || record->type != AssetType::Material) { return nullptr; }

        bool needsLoad = false;
        Entry& entry = entryFor(*record, needsLoad);

        if (needsLoad)
        {
            auto document = std::make_unique<MaterialDocument>();
            ++fileReads_;

            // A failure is cached as a failure rather than left uncached: a broken material would
            // otherwise be reopened on every frame it is selected, which is the case this exists to
            // stop and the one most likely to be sitting on somebody's screen.
            entry.loaded = true;
            if (loadMaterialDocument(assets, id, *document) == MaterialLoadProblem::None)
            {
                entry.material = std::move(document);
            }
        }

        return entry.material.get();
    }

    const PrefabDocument* StudioAssetDocumentCache::prefab(const AssetDatabase& assets,
                                                           const Uuid& id,
                                                           const ComponentRegistry& registry)
    {
        const AssetRecord* record = assets.find(id);
        if (record == nullptr || record->type != AssetType::Prefab) { return nullptr; }

        bool needsLoad = false;
        Entry& entry = entryFor(*record, needsLoad);

        if (needsLoad)
        {
            auto document = std::make_unique<PrefabDocument>();
            ++fileReads_;

            entry.loaded = true;
            if (document->loadFromFile(assets.resolvePath(record->sourcePath), registry).succeeded)
            {
                entry.prefab = std::move(document);
            }
        }

        return entry.prefab.get();
    }

    void StudioAssetDocumentCache::invalidate(const Uuid& id)
    {
        if (id.isValid()) { entries_.erase(id); }
        else { entries_.clear(); }
    }
}
