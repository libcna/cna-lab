// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Assets/EnvironmentMapDocument.hpp"

#include <fstream>
#include <iterator>
#include <utility>

#include "CNA/Studio/Assets/AssetDatabase.hpp"

namespace CNA::Studio
{
    JsonValue EnvironmentMapDocument::toJson() const
    {
        JsonValue root = JsonValue::makeObject();
        root.set("formatVersion", JsonValue{kFormatVersion});
        root.set("name", JsonValue{name});

        // Written even when nil, and the empty string is what a nil id spells. An absent key would
        // be indistinguishable from a key this build does not know about, and the difference
        // matters to the next reader: "no sky chosen" and "a sky I cannot see" are different
        // states and only one of them is the user's doing.
        root.set("panorama", JsonValue{panorama.isValid() ? panorama.toString() : std::string{}});

        // Every setting, always, rather than only the ones that differ from the default. There is
        // no inheritance here for an omission to mean something else about -- `MaterialDocument`'s
        // key-set-as-override-set trick answers a question this document does not ask -- so the
        // full record is the one that keeps meaning the same thing when a default changes.
        root.set("processing", settings.toJson());
        return root;
    }

    bool EnvironmentMapDocument::loadFromJson(const JsonValue& json)
    {
        // The one hard failure, for the reason `MaterialDocument::loadFromJson` gives: quietly
        // rewriting somebody's file with less in it than they put there is worse than refusing.
        const int version = static_cast<int>(json["formatVersion"].asNumber(kFormatVersion));
        if (version > kFormatVersion) { return false; }

        const EnvironmentMapDocument defaults;

        name = json["name"].asString(defaults.name);
        panorama = Uuid::parse(json["panorama"].asString(""));
        settings = StudioEnvironmentMapImportSettings::fromJson(json["processing"]);
        return true;
    }

    EnvironmentMapLoadProblem loadEnvironmentMapFile(const std::string& absolutePath,
                                                     EnvironmentMapDocument& out)
    {
        std::ifstream stream{absolutePath};
        if (!stream) { return EnvironmentMapLoadProblem::Unreadable; }

        const std::string text{std::istreambuf_iterator<char>{stream},
                               std::istreambuf_iterator<char>{}};
        const JsonParseResult parsed = Json::parse(text);
        if (!parsed.succeeded) { return EnvironmentMapLoadProblem::UnreadableFormat; }

        // Loaded into a local first, so a document declaring a version this build cannot read
        // leaves the caller's own value untouched rather than half-overwritten.
        EnvironmentMapDocument loaded;
        if (!loaded.loadFromJson(parsed.value))
        {
            return EnvironmentMapLoadProblem::UnreadableFormat;
        }

        out = std::move(loaded);
        return EnvironmentMapLoadProblem::None;
    }

    EnvironmentMapLoadProblem loadEnvironmentMapDocument(const AssetDatabase& assets,
                                                         const Uuid& assetId,
                                                         EnvironmentMapDocument& out)
    {
        const AssetRecord* record = assets.find(assetId);
        if (record == nullptr || record->type != AssetType::EnvironmentMap)
        {
            return EnvironmentMapLoadProblem::NotAnEnvironmentMap;
        }

        return loadEnvironmentMapFile(assets.resolvePath(record->sourcePath), out);
    }
}
