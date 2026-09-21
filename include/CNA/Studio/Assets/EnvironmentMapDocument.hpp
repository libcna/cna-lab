// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Assets/EnvironmentMapDocument.hpp
 * @brief A `.cnaenv`: one panorama, and what to turn it into (`plan.md` STUDIO-10010).
 *
 * ### Why this is an asset of its own rather than a texture setting
 *
 * The obvious shape would have been a second importer on the panorama's own file — a `.png` marked
 * "this is a sky" — and it was rejected because **nothing in Studio can change an asset's
 * importer**. `AssetDatabase::defaultImporterFor` maps a type to one importer, the inspector reads
 * `AssetRecord::importerId` and never writes it, and there is no command that would. Building this
 * row on a gesture that does not exist would have meant inventing the gesture first, and a "change
 * importer" command is a bigger and more dangerous thing than a sky.
 *
 * The shape that *does* exist is the one `.cnamaterial` uses: an asset the editor authors, which
 * references an imported one by id. `MaterialDocument.hpp` states the reasoning and all of it
 * applies here — the reference is a `Uuid` rather than a path, so it survives the panorama being
 * renamed or moved (D-08), and the document carries no id of its own, because identity is the
 * `.cnaasset` sidecar's and a second copy would be free to disagree with it.
 *
 * It also separates two things that genuinely are separate. The panorama is an ordinary texture:
 * it is decoded, it has a size, it can be previewed, and a project may well use the same image as
 * a background elsewhere. What is *not* ordinary is the processing — a cube, an irradiance
 * convolution and a prefiltered specular chain, each with sizes and sample counts. Those belong to
 * the environment map, not to the image, and two skies at different qualities from one panorama is
 * an ordinary thing to want and impossible if the settings live on the file.
 *
 * ### What it does not decide
 *
 * How bright the sky lights the scene, whether it is drawn behind the geometry, and which scene
 * uses it are all *scene* questions, and they are `STUDIO-20005`'s. This document says what the
 * asset is; the scene says what is done with it.
 */

#include <string>

#include "CNA/Studio/Assets/EnvironmentMapImport.hpp"
#include "CNA/Studio/Core/Json.hpp"
#include "CNA/Studio/Core/Uuid.hpp"

namespace CNA::Studio
{
    /** @brief An authored environment map, as stored in a `.cnaenv` file. */
    struct EnvironmentMapDocument
    {
        /** @brief The `formatVersion` this build writes, and the highest it can read. */
        static constexpr int kFormatVersion = 1;

        /** @brief What a person calls it. Defaults to the file's stem when one is created. */
        std::string name = "Environment";

        /**
         * @brief The equirectangular panorama, by asset id. Nil until somebody picks one.
         *
         * Nil is a legal, ordinary state rather than a broken one: an environment map is created
         * before its sky is chosen, exactly as a material is created before its textures are —
         * and `studioPlanEnvironmentMapImport` still answers every question that does not depend
         * on the panorama, so the asset is inspectable from the moment it exists.
         */
        Uuid panorama;

        /** @brief What to generate from it, and how finely. @see EnvironmentMapImport.hpp */
        StudioEnvironmentMapImportSettings settings;

        [[nodiscard]] JsonValue toJson() const;

        /**
         * @brief Reads @p json, keeping defaults for anything absent.
         *
         * @return False when the document declares a `formatVersion` this build cannot read, which
         *         is the only hard failure — for the reason `MaterialDocument::loadFromJson` gives:
         *         an environment map written by a future editor with three more fields should load
         *         as the environment map it mostly is rather than as nothing at all.
         */
        bool loadFromJson(const JsonValue& json);
    };

    class AssetDatabase;

    /** @brief Why an environment map asset could not be read. @see MaterialLoadProblem */
    enum class EnvironmentMapLoadProblem
    {
        /** @brief Loaded. */
        None,
        /** @brief No such asset, or it is not an environment map. */
        NotAnEnvironmentMap,
        /** @brief The file is not there, or cannot be opened. */
        Unreadable,
        /** @brief Not JSON, or a `formatVersion` this build cannot read. */
        UnreadableFormat,
    };

    /**
     * @brief Reads the environment map at @p absolutePath.
     *
     * The half of @ref loadEnvironmentMapDocument that needs no database, split out for the same
     * reason `loadMaterialFile` is: a thumbnail worker runs off the frame, where the asset database
     * belongs to the main thread and must not be touched.
     *
     * @param out Filled in on success; untouched otherwise, so a caller's defaults survive.
     */
    [[nodiscard]] EnvironmentMapLoadProblem loadEnvironmentMapFile(const std::string& absolutePath,
                                                                   EnvironmentMapDocument& out);

    /**
     * @brief Reads the environment map asset @p assetId from the project.
     *
     * One reader rather than one per caller, for the reason `loadMaterialDocument` gives: the
     * viewport that processes the sky and the editor that writes the file must agree about what
     * the file says.
     *
     * @param out Filled in on success; untouched otherwise.
     * @return What happened, so a caller can tell a missing file from an unreadable one from one
     *         this build is too old for — three different messages, and only the last of them
     *         means "do not offer to overwrite it".
     */
    [[nodiscard]] EnvironmentMapLoadProblem loadEnvironmentMapDocument(
        const AssetDatabase& assets, const Uuid& assetId, EnvironmentMapDocument& out);
}
