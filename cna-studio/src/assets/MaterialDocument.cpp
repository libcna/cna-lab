// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Assets/MaterialDocument.hpp"

#include "CNA/Studio/Assets/AssetDatabase.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace CNA::Studio
{
    namespace
    {
        JsonValue vectorToJson(const StudioVector3& vector)
        {
            JsonValue array = JsonValue::makeArray();
            array.append(JsonValue{static_cast<double>(vector.x)});
            array.append(JsonValue{static_cast<double>(vector.y)});
            array.append(JsonValue{static_cast<double>(vector.z)});
            return array;
        }

        StudioVector3 vectorFromJson(const JsonValue& json, const StudioVector3& fallback)
        {
            if (!json.isArray() || json.getElements().size() < 3) { return fallback; }

            const std::vector<JsonValue>& elements = json.getElements();
            return StudioVector3{
                static_cast<float>(elements[0].asNumber(static_cast<double>(fallback.x))),
                static_cast<float>(elements[1].asNumber(static_cast<double>(fallback.y))),
                static_cast<float>(elements[2].asNumber(static_cast<double>(fallback.z)))};
        }

        /** @brief Writes an id, or omits it: a nil reference and an absent one mean the same thing. */
        void setTexture(JsonValue& object, const char* key, const Uuid& id)
        {
            if (!id.isValid()) { return; }
            object.set(key, JsonValue{id.toString()});
        }
    }

    MeshMaterial MaterialDocument::toMeshMaterial() const
    {
        MeshMaterial material;
        material.name = name;
        material.diffuseColor = diffuseColor;
        material.emissiveColor = emissiveColor;
        material.alpha = alpha;
        material.metallic = std::clamp(metallic, 0.0f, 1.0f);
        material.roughness = std::clamp(roughness, 0.0f, 1.0f);

        // Derived here rather than stored, exactly as the glTF importer derives them: a material
        // holding both descriptions independently is a material whose two halves can disagree, and
        // the disagreement only shows on whichever effect the user is not looking at.
        const float metallicFactor = material.metallic;
        material.specularColor =
            StudioVector3{metallicFactor * diffuseColor.x + (1.0f - metallicFactor) * 0.04f,
                          metallicFactor * diffuseColor.y + (1.0f - metallicFactor) * 0.04f,
                          metallicFactor * diffuseColor.z + (1.0f - metallicFactor) * 0.04f};

        const float clampedRoughness = std::clamp(roughness, 0.03f, 1.0f);
        material.specularPower =
            std::clamp(2.0f / (clampedRoughness * clampedRoughness) - 2.0f, 1.0f, 1024.0f);

        material.alphaMode = alphaMode;
        material.alphaCutoff = std::clamp(alphaCutoff, 0.0f, 1.0f);

        // The texture paths stay empty on purpose: this document speaks in ids, and resolving one
        // to a file is the caller's job because the caller is what holds the asset database.
        return material;
    }

    JsonValue MaterialDocument::toJson() const
    {
        JsonValue root = JsonValue::makeObject();
        root.set("formatVersion", JsonValue{kFormatVersion});
        root.set("name", JsonValue{name});

        // An instance writes its parent and *only* the parameters it states for itself; a
        // material of its own writes everything, exactly as it always did (`plan.md`
        // STUDIO-19005). The name is not a parameter -- every material has one of its own, and an
        // instance that inherited its name would be a second file with the first one's name on it.
        const bool isInstance = parent.isValid();
        if (isInstance) { root.set("parent", JsonValue{parent.toString()}); }

        const auto states = [&](const char* key) {
            return !isInstance || overridden.count(key) != 0;
        };

        if (states("diffuseColor")) { root.set("diffuseColor", vectorToJson(diffuseColor)); }
        if (states("emissiveColor")) { root.set("emissiveColor", vectorToJson(emissiveColor)); }
        if (states("metallic")) { root.set("metallic", JsonValue{static_cast<double>(metallic)}); }
        if (states("roughness")) { root.set("roughness", JsonValue{static_cast<double>(roughness)}); }
        if (states("alpha")) { root.set("alpha", JsonValue{static_cast<double>(alpha)}); }

        // Written only when it is not glTF's default, the same bargain an unset texture strikes:
        // a file carrying every field anybody ever looked at makes each material's diff noise. On
        // an instance the override decides instead, because there "the default" is the parent's
        // value and absence has to mean inherited rather than Opaque.
        if (isInstance ? states("alphaMode") : alphaMode != MeshAlphaMode::Opaque)
        {
            root.set("alphaMode", JsonValue{std::string{toString(alphaMode)}});
        }
        if (isInstance ? states("alphaCutoff") : alphaMode == MeshAlphaMode::Mask)
        {
            root.set("alphaCutoff", JsonValue{static_cast<double>(alphaCutoff)});
        }

        // A texture is absent when it is unset, and on an instance also when it is inherited. An
        // instance that overrides a slot *to nothing* -- "this variant has no normal map" -- is a
        // real thing to want, so the key is written as an empty string rather than omitted, which
        // is the one place a nil id and an absent one have to mean different things.
        const auto setSlot = [&](const char* key, const Uuid& id) {
            if (!states(key)) { return; }
            if (isInstance) { root.set(key, JsonValue{id.isValid() ? id.toString() : std::string{}}); }
            else { setTexture(root, key, id); }
        };

        setSlot("diffuseTexture", diffuseTexture);
        setSlot("normalTexture", normalTexture);
        setSlot("metallicRoughnessTexture", metallicRoughnessTexture);
        setSlot("emissiveTexture", emissiveTexture);
        setSlot("occlusionTexture", occlusionTexture);

        return root;
    }

    const FormatMigrator& getMaterialFormatMigrator()
    {
        static const FormatMigrator migrator{"material", MaterialDocument::kFormatVersion};
        return migrator;
    }

    bool MaterialDocument::loadFromJson(const JsonValue& json, const FormatMigrator* migrator,
                                        std::vector<std::string>* warnings)
    {
        if (!json.isObject()) { return false; }

        // The gate and the upgrade, in one piece of code (`plan.md` STUDIO-31005). This used to be
        // a hand-written `version > kFormatVersion` and nothing else, which is the half of the
        // answer that refuses: a `.cnamaterial` from an older format had no way to be read at all,
        // and the migrator that every other document type runs was never reached from here.
        //
        // The refusal it kept is still the right one and the migrator states it in the same words:
        // a file from a future editor may hold fields this build would silently drop on the next
        // save, and quietly rewriting somebody's material with less in it than they put there is
        // worse than refusing to open it.
        const FormatMigrator& chain = migrator != nullptr ? *migrator : getMaterialFormatMigrator();

        // Copied only when something has to change it, as `SceneDocument` does. A file already at
        // the current version is the overwhelmingly common case and costs nothing.
        JsonValue upgraded;
        const JsonValue* source = &json;

        if (json["formatVersion"].asInt(0) != chain.getCurrentVersion())
        {
            upgraded = json;

            const FormatMigrationResult migration = chain.migrate(upgraded);
            if (!migration.succeeded) { return false; }

            if (warnings != nullptr)
            {
                for (const std::string& step : migration.applied)
                {
                    warnings->push_back("upgraded from an older material format: " + step);
                }
            }

            source = &upgraded;
        }

        const JsonValue& document = *source;

        const MaterialDocument defaults;

        name = document["name"].asString(defaults.name);
        parent = Uuid::parse(document["parent"].asString(""));

        // The file's own keys *are* the override set (`plan.md` STUDIO-19005): a parameter is
        // stated here if and only if it is written here. Recorded for every material, not only for
        // an instance, so that giving a material a parent later does not silently turn every one
        // of its values into an inheritance.
        // Read from the *upgraded* document, not the original -- a migration that added a key
        // would otherwise leave it out of the override set, which is the one place where reading
        // the wrong one of the two would be silent rather than obvious.
        overridden.clear();
        for (const char* key : {"diffuseColor", "emissiveColor", "metallic", "roughness", "alpha",
                                "alphaMode", "alphaCutoff", "diffuseTexture", "normalTexture",
                                "metallicRoughnessTexture", "emissiveTexture", "occlusionTexture"})
        {
            if (document.contains(key)) { overridden.insert(key); }
        }

        diffuseColor = vectorFromJson(document["diffuseColor"], defaults.diffuseColor);
        emissiveColor = vectorFromJson(document["emissiveColor"], defaults.emissiveColor);
        metallic = static_cast<float>(document["metallic"].asNumber(defaults.metallic));
        roughness = static_cast<float>(document["roughness"].asNumber(defaults.roughness));
        alpha = static_cast<float>(document["alpha"].asNumber(defaults.alpha));
        alphaMode = parseMeshAlphaMode(document["alphaMode"].asString(toString(defaults.alphaMode)));
        alphaCutoff = static_cast<float>(document["alphaCutoff"].asNumber(defaults.alphaCutoff));

        diffuseTexture = Uuid::parse(document["diffuseTexture"].asString(""));
        normalTexture = Uuid::parse(document["normalTexture"].asString(""));
        metallicRoughnessTexture = Uuid::parse(document["metallicRoughnessTexture"].asString(""));
        emissiveTexture = Uuid::parse(document["emissiveTexture"].asString(""));
        occlusionTexture = Uuid::parse(document["occlusionTexture"].asString(""));

        return true;
    }

    MaterialLoadProblem loadMaterialFile(const std::string& absolutePath, MaterialDocument& out,
                                         const FormatMigrator* migrator,
                                         std::vector<std::string>* warnings)
    {
        std::ifstream stream{absolutePath};
        if (!stream) { return MaterialLoadProblem::Unreadable; }

        const std::string text{std::istreambuf_iterator<char>{stream},
                               std::istreambuf_iterator<char>{}};
        const JsonParseResult parsed = Json::parse(text);
        if (!parsed.succeeded) { return MaterialLoadProblem::UnreadableFormat; }

        // Loaded into a local first, so a document that declares a version this build cannot read
        // leaves the caller's own value untouched rather than half-overwritten.
        MaterialDocument loaded;
        if (!loaded.loadFromJson(parsed.value, migrator, warnings))
        {
            return MaterialLoadProblem::UnreadableFormat;
        }

        out = std::move(loaded);
        return MaterialLoadProblem::None;
    }

    bool studioMaterialChainWouldLoop(const AssetDatabase& assets, const Uuid& material,
                                      const Uuid& candidate)
    {
        if (!material.isValid() || !candidate.isValid()) { return false; }
        if (material == candidate) { return true; }

        // Walking *up* from the candidate: the loop this would close is the candidate finding its
        // way back to the material that is about to adopt it.
        std::set<std::string> visited;
        Uuid current = candidate;
        while (current.isValid())
        {
            if (current == material) { return true; }

            // A chain that already loops is not made worse by this link, and saying so is the
            // difference between a refusal the user can act on and one they cannot.
            if (!visited.insert(current.toString()).second) { return false; }

            MaterialDocument document;
            if (loadMaterialDocument(assets, current, document) != MaterialLoadProblem::None)
            {
                return false;
            }
            current = document.parent;
        }

        return false;
    }

    void applyMaterialOverrides(MaterialDocument& base, const MaterialDocument& instance)
    {
        const auto states = [&instance](const char* key) {
            return instance.overridden.count(key) != 0;
        };

        if (states("diffuseColor")) { base.diffuseColor = instance.diffuseColor; }
        if (states("emissiveColor")) { base.emissiveColor = instance.emissiveColor; }
        if (states("metallic")) { base.metallic = instance.metallic; }
        if (states("roughness")) { base.roughness = instance.roughness; }
        if (states("alpha")) { base.alpha = instance.alpha; }
        if (states("alphaMode")) { base.alphaMode = instance.alphaMode; }
        if (states("alphaCutoff")) { base.alphaCutoff = instance.alphaCutoff; }
        if (states("diffuseTexture")) { base.diffuseTexture = instance.diffuseTexture; }
        if (states("normalTexture")) { base.normalTexture = instance.normalTexture; }
        if (states("metallicRoughnessTexture"))
        {
            base.metallicRoughnessTexture = instance.metallicRoughnessTexture;
        }
        if (states("emissiveTexture")) { base.emissiveTexture = instance.emissiveTexture; }
        if (states("occlusionTexture")) { base.occlusionTexture = instance.occlusionTexture; }
    }

    MaterialResolveProblem resolveMaterialDocument(const AssetDatabase& assets,
                                                   const Uuid& assetId, MaterialDocument& out)
    {
        // Deep enough for any hierarchy a person would build and shallow enough that a cycle the
        // visited set somehow missed still ends. Both guards, because they fail differently: the
        // set catches a loop and this catches a chain that is merely absurd.
        constexpr int kMaximumDepth = 32;

        // Collected leaf-first and applied root-first, so a parameter belongs to the nearest
        // ancestor that states it.
        std::vector<MaterialDocument> chain;
        std::set<std::string> visited;

        Uuid current = assetId;
        MaterialResolveProblem problem = MaterialResolveProblem::None;

        while (current.isValid())
        {
            if (!visited.insert(current.toString()).second)
            {
                problem = MaterialResolveProblem::Cycle;
                break;
            }
            if (chain.size() >= static_cast<std::size_t>(kMaximumDepth))
            {
                problem = MaterialResolveProblem::TooDeep;
                break;
            }

            MaterialDocument document;
            if (loadMaterialDocument(assets, current, document) != MaterialLoadProblem::None)
            {
                // The leaf itself failing is the ordinary "not a material" answer; a *parent*
                // failing is a broken link, and both leave the caller with whatever was resolved.
                problem = MaterialResolveProblem::Unreadable;
                break;
            }

            current = document.parent;
            chain.push_back(std::move(document));
        }

        if (chain.empty()) { return problem; }

        // The root's values whole, then each descendant's stated parameters over them.
        MaterialDocument resolved = chain.back();
        for (auto level = chain.rbegin() + 1; level != chain.rend(); ++level)
        {
            applyMaterialOverrides(resolved, *level);
        }

        // The leaf's own identity, not the root's: this is still that material, drawn with what it
        // inherits filled in.
        resolved.name = chain.front().name;
        resolved.parent = chain.front().parent;
        resolved.overridden = chain.front().overridden;

        out = std::move(resolved);
        return problem;
    }

    MaterialLoadProblem loadMaterialDocument(const AssetDatabase& assets, const Uuid& assetId,
                                             MaterialDocument& out)
    {
        const AssetRecord* record = assets.find(assetId);
        if (record == nullptr || record->type != AssetType::Material)
        {
            return MaterialLoadProblem::NotAMaterial;
        }

        // The database's half is resolving the id to a file; reading it is the same code either
        // way, which is what this file's header means by one reader.
        return loadMaterialFile(assets.resolvePath(record->sourcePath), out);
    }
}
