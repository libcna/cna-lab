// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Assets/MaterialDocument.hpp
 * @brief A `.cnamaterial`: a material somebody authored, rather than one a model brought with it
 *        (plan.md ED-403).
 *
 * A glTF file carries its own materials and `MeshData::materials` holds them. This is the other
 * kind: a material that is an *asset*, with an id, that a `ModelRenderer` can point at to override
 * what its model came with. `CNA.ModelRenderer` has declared exactly that reference since Phase 1
 * and nothing has ever been able to satisfy it -- the inspector offered a picker with nothing to
 * pick.
 *
 * **It is a new format at version 1, not a change to an existing one.** Nothing already written
 * gains or loses a field, so no `formatVersion` moves and no migration chain has anything to do --
 * the same shape `.cnarecovery` arrived in (ED-903).
 *
 * **The fields are `MeshMaterial`'s, deliberately.** A material asset that could express things an
 * imported material cannot would be a material the model pass has to handle twice, and the extra
 * expressiveness would be bounded by the same `BasicEffect`/`PbrEffect` pair regardless. So this
 * converts to a `MeshMaterial` and the pass stays one code path. What it adds is what an *asset*
 * needs and a mesh's own material does not: a name a person chose, and texture references by
 * `Uuid` rather than by a path relative to some model file.
 *
 * **It carries no id of its own**, which is worth stating because the first version did and it was
 * wrong. An asset's identity is the `Uuid` its `.cnaasset` sidecar holds -- that is what D-08 means
 * by identity being an id and never a path, and what every scene reference already uses. A second
 * id inside the file would be a second answer to "which material is this", free to disagree with
 * the first, and the disagreement would surface as a `ModelRenderer` pointing at a material the
 * database can find and the file denies being. The same reasoning ED-300 applies to prefab
 * overrides: one description of a fact, not two.
 *
 * That last difference is the one to keep in mind. A `MeshMaterial` carries texture *paths*,
 * because the glTF importer has no asset database and must not pretend to (`MeshData.hpp`). A
 * material asset is written by the editor, which does have one, so it carries ids -- and an id
 * survives the texture being renamed or moved, which a path does not (D-08).
 */

#include <set>
#include <string>
#include <vector>

#include "CNA/Studio/Core/FormatMigration.hpp"
#include "CNA/Studio/Core/Json.hpp"
#include "CNA/Studio/Core/MeshData.hpp"
#include "CNA/Studio/Core/Uuid.hpp"

namespace CNA::Studio
{
    /** @brief An authored material, as stored in a `.cnamaterial` file. */
    struct MaterialDocument
    {
        /** @brief The `formatVersion` this build writes, and the highest it can read. */
        static constexpr int kFormatVersion = 1;

        /** @brief What a person calls it. Defaults to the file's stem when one is created. */
        std::string name = "Material";

        /**
         * @brief The material this one inherits from, or nil for a material of its own
         *        (`plan.md` STUDIO-19005).
         *
         * An *instance* stores only the parameters it changes; everything else is whatever its
         * parent says, now and after the parent is edited. That is the whole point of one: a
         * project's forty crate variants follow the crate.
         */
        Uuid parent;

        /**
         * @brief Which fields this document states for itself, by their JSON key.
         *
         * Empty on a material with no parent, where every field is its own and the question does
         * not arise. On an instance it is exactly the set of keys *present in the file* — so a
         * parameter is overridden if and only if it is written, and inherited if and only if it is
         * absent.
         *
         * **This is not the stored override list `ED-300` rejects, and the difference is the whole
         * of why it is allowed here.** What that rule forbids is a list kept *beside* the values,
         * free to disagree with them — a parameter marked overridden whose value equals the
         * parent's, or the reverse, showing up later as a property that reverts to something the
         * user never chose. Here there is no second description to disagree with: a parameter that
         * is not overridden has **no value in this document at all**. The set is read from the
         * file's own keys on load and decides its own keys on save, so the two cannot drift.
         *
         * Prefabs answer the same question by comparison instead, and that is right for them: a
         * prefab instance must be indistinguishable from a hand-authored entity once it is in a
         * scene, so it has to hold every value. A material instance has the opposite requirement —
         * it must *follow* its parent — and comparison cannot express "inherited, and equal by
         * coincidence".
         */
        std::set<std::string> overridden;

        StudioVector3 diffuseColor{1.0f, 1.0f, 1.0f};
        StudioVector3 emissiveColor{0.0f, 0.0f, 0.0f};

        /**
         * @brief Metallic-roughness, kept even when the build draws through `BasicEffect`.
         *
         * The same bargain the importer strikes: which effect draws is a property of the build
         * (gap G-05), so a material that stored only one description of itself would render as
         * something else entirely on the other one. `toMeshMaterial` derives the Blinn-Phong pair
         * from these rather than storing a second, driftable copy.
         */
        float metallic = 0.0f;
        float roughness = 1.0f;

        float alpha = 1.0f;

        /**
         * @brief How this material's alpha is read (`plan.md` STUDIO-19004).
         *
         * glTF's three modes, because that is where an imported material's answer comes from.
         * Additive at `formatVersion` 1 for the reason `occlusionTexture` is: an older material
         * has no mode written and reads as `Opaque`, which is glTF's default and what every
         * material drew as before this existed.
         */
        MeshAlphaMode alphaMode = MeshAlphaMode::Opaque;

        /** @brief The threshold a `Mask` material is cut at. Kept whatever the mode is. */
        float alphaCutoff = 0.5f;

        /** @brief Texture assets by id, never by path -- see this file's header. Nil for none. */
        Uuid diffuseTexture;
        Uuid normalTexture;
        Uuid metallicRoughnessTexture;
        Uuid emissiveTexture;

        /**
         * @brief The ambient-occlusion map (`plan.md` STUDIO-19002).
         *
         * Separate from `metallicRoughnessTexture` because `PbrEffect` takes both and glTF
         * permits both: the packed form carries occlusion in R, and a material that ships a
         * dedicated occlusion image has nowhere else to put it.
         *
         * **Additive at `formatVersion` 1 rather than a version bump.** Nothing already written
         * changes meaning, `loadFromJson` keeps its defaults for anything absent, and a bump would
         * make every material this build writes unreadable by the previous one for the sake of one
         * optional texture. The cost is stated rather than hidden: an older Studio opening a
         * material with an occlusion map ignores the field, and drops it if the user then saves.
         * That is the trade every additive field in this project makes.
         */
        Uuid occlusionTexture;

        /**
         * @brief Converts to the form the model pass already draws.
         *
         * The texture *paths* come back empty: this side speaks in ids and the caller is the one
         * holding the database that can resolve them. Filling in a path here would be inventing a
         * second way for a texture to be named, which is the mistake `MeshData.hpp` avoids from
         * the other direction.
         */
        [[nodiscard]] MeshMaterial toMeshMaterial() const;

        [[nodiscard]] JsonValue toJson() const;

        /**
         * @brief Reads @p json, upgrading it first and keeping defaults for anything absent.
         *
         * **The version gate and the upgrade are one piece of code** (`plan.md` STUDIO-31005), for
         * the reason `SceneDocument::loadFromJson` gives: refusing a file from the future and
         * upgrading one from the past are both answers to "what version is this?", and splitting
         * them is how a loader comes to refuse a file it could have read. This one had the gate
         * written by hand and no upgrade at all, which is exactly that split.
         *
         * Every absence *below* the version is still a default, because a material written by a
         * future editor with three more fields should load as the material it mostly is rather
         * than as nothing at all. The version itself is not an absence that can be defaulted: a
         * document with no `formatVersion` is refused rather than read as though it were current,
         * since reading it as current is a guess about a file whose shape is unknown.
         *
         * @param migrator The chain to run, or nullptr for `getMaterialFormatMigrator()`. Injected
         *                 only by tests, which is how a chain with a step in it can be proven to
         *                 run before the fields are read while every real chain is empty.
         * @param warnings Appended to when a step runs, so an upgrade is reported rather than
         *                 silent (`STUDIO-31011`). Optional; nullptr discards them.
         * @return False when the document is not an object, declares no usable `formatVersion`, or
         *         declares one this build cannot reach.
         */
        bool loadFromJson(const JsonValue& json, const FormatMigrator* migrator = nullptr,
                          std::vector<std::string>* warnings = nullptr);
    };

    /**
     * @brief Returns the migration chain that upgrades a `.cnamaterial`.
     *
     * Empty, like every other chain here: the format has only ever been at version 1. Registering
     * a step is not a licence to bump a version -- an additive field that older builds can ignore
     * costs nothing and needs no step, and `occlusionTexture` arriving at version 1 is the
     * precedent.
     */
    [[nodiscard]] const FormatMigrator& getMaterialFormatMigrator();

    class AssetDatabase;

    /** @brief Why a material asset could not be read. Empty when it was. */
    enum class MaterialLoadProblem
    {
        /** @brief Loaded. */
        None,
        /** @brief No such asset, or it is not a material. */
        NotAMaterial,
        /** @brief The file is not there, or cannot be opened. */
        Unreadable,
        /** @brief Not JSON, or a `formatVersion` this build cannot read. */
        UnreadableFormat,
    };

    /**
     * @brief Reads the material at @p absolutePath.
     *
     * The half of `loadMaterialDocument` that does not need a database, split out for the
     * thumbnail worker (`plan.md` STUDIO-19007): that runs off the frame, where the asset
     * database and the document cache are the main thread's and must not be touched. Still one
     * reader -- `loadMaterialDocument` resolves an id to a path and then calls this.
     *
     * @param out Filled in on success; untouched otherwise, so a caller's defaults survive.
     * @param migrator The chain to run, or nullptr for `getMaterialFormatMigrator()`.
     * @param warnings Appended to when an upgrade runs. Optional.
     */
    [[nodiscard]] MaterialLoadProblem loadMaterialFile(const std::string& absolutePath,
                                                       MaterialDocument& out,
                                                       const FormatMigrator* migrator = nullptr,
                                                       std::vector<std::string>* warnings = nullptr);

    /**
     * @brief Reads the material asset @p assetId from the project.
     *
     * `plan.md` STUDIO-07046. One reader rather than one per caller: the model pass's material
     * provider and the editor that writes the file must agree about what the file says, and two
     * copies of "open it, parse it, load it, and decide what a failure means" is two chances to
     * disagree about a material that is half-written.
     *
     * @param assets The project's assets, for the record and the path.
     * @param assetId The material asset.
     * @param out Filled in on success; untouched otherwise, so a caller's defaults survive.
     * @return What happened, so the caller can say which of the three failures it was — a missing
     *         file, an unreadable one and one this build is too old for are three different
     *         messages, and only the last of them means "do not offer to overwrite it".
     */
    [[nodiscard]] MaterialLoadProblem loadMaterialDocument(const AssetDatabase& assets,
                                                           const Uuid& assetId,
                                                           MaterialDocument& out);

    /**
     * @brief Why a material chain could not be resolved. Empty when it was.
     *
     * `plan.md` STUDIO-19005. Separate from `MaterialLoadProblem` because a chain has a failure of
     * its own that a single file does not: it can come back to where it started.
     */
    enum class MaterialResolveProblem
    {
        /** @brief Resolved. */
        None,
        /** @brief The material itself, or one of its parents, could not be read. */
        Unreadable,
        /**
         * @brief A parent chain that comes back to a material it has already been through.
         *
         * Reported rather than followed: an editor that walked a cycle would hang, and one that
         * silently stopped would show a material whose parameters depend on where the walk began.
         * What is returned is the chain resolved as far as the repeat, so the user sees a material
         * rather than nothing while they fix it.
         */
        Cycle,
        /** @brief The chain is longer than any real material hierarchy. */
        TooDeep,
    };

    /**
     * @brief Whether making @p candidate the parent of @p material would close a loop.
     *
     * `plan.md` STUDIO-19005. Asked before the link is written rather than reported after: the
     * chain is resolvable now, and a user who has just chosen a parent has all the context they
     * will ever have for understanding why it was refused.
     *
     * True when @p candidate is @p material itself, or inherits from it however far up.
     */
    [[nodiscard]] bool studioMaterialChainWouldLoop(const AssetDatabase& assets,
                                                    const Uuid& material, const Uuid& candidate);

    /**
     * @brief Applies @p instance's stated parameters over @p base.
     *
     * `plan.md` STUDIO-19005. The one step of the inheritance walk, exposed because two callers
     * need it -- the walk over the asset database and the one the document cache does over its own
     * entries -- and two copies of "which fields does an override replace" is two chances to
     * forget one the day a field is added.
     *
     * Touches only the parameters: @p base keeps its own name, parent and override set, because
     * those belong to whichever document the caller is building.
     */
    void applyMaterialOverrides(MaterialDocument& base, const MaterialDocument& instance);

    /**
     * @brief The material @p assetId draws as, with every inherited parameter filled in.
     *
     * Walks to the root of the parent chain and applies each level's stated parameters on the way
     * back down, so a parameter belongs to the nearest ancestor that states it.
     *
     * The result's `overridden` set is the *leaf's* own, not the union: it answers "what does this
     * material state for itself", which is what the editor needs to draw an override marker, and
     * the union would answer nothing anybody asks. Its `parent` is the leaf's parent, so the
     * resolved document still says what it inherits from.
     *
     * @param out Filled in as far as the walk got, even on a failure -- a material with a broken
     *        parent is better shown wrong than not shown.
     */
    [[nodiscard]] MaterialResolveProblem resolveMaterialDocument(const AssetDatabase& assets,
                                                                 const Uuid& assetId,
                                                                 MaterialDocument& out);
}
