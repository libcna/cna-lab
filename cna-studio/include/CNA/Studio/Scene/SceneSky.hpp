// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Scene/SceneSky.hpp
 * @brief Whether a scene has a sky, and what stops it (`plan.md` STUDIO-20005).
 *
 * A scene names an environment map; the environment map names a panorama; the panorama is a
 * texture. Three references, and **five distinct ways for the chain to end in no sky** — none of
 * which is an error, and every one of which looks identical in the viewport. A user who has set an
 * environment map and sees nothing deserves to be told which link is missing, not left to compare
 * screenshots.
 *
 * So the walk is a CNA-free function that names its answer, and the device layer is a call. That
 * is the same division `SceneShadows` uses and for the same reason: the decision has an answer that
 * can be wrong, so it is tested against a document rather than against an image.
 *
 * ### Why the provider is plain data rather than the document
 *
 * `cna-studio-scene` links `cna-studio-core` and nothing else, so it cannot name
 * `EnvironmentMapDocument` — that lives in `cna-studio-assets`, above it. Rather than invert the
 * dependency for one struct, the caller answers the three questions this function needs about an
 * asset id and hands back @ref SceneSkySource, which is four plain fields. The viewport holds the
 * database; this holds the rule.
 *
 * ### Drawing and lighting are two answers
 *
 * CNA offers them separately — `Skybox::draw` puts the cube on screen and
 * `PbrEffect::setImageBasedLightEXT` makes it light things — and they are separately worth wanting.
 * A backdrop that must not tint the scene is one; image-based lighting in a room whose windows show
 * no sky is the other. Two booleans on the scene, two on the plan, and the pass reads whichever it
 * is about.
 */

#include <functional>
#include <string>

#include "CNA/Studio/Core/Uuid.hpp"
#include "CNA/Studio/Scene/SceneEnvironment.hpp"

namespace CNA::Studio
{
    /**
     * @brief What the caller could find out about the environment map an id names.
     *
     * Plain data, deliberately: see this file's header. Every field is answerable by a database
     * lookup and a file read, and nothing here needs the document's own type.
     */
    struct SceneSkySource
    {
        /** @brief True when the id names an asset and that asset is an environment map. */
        bool found = false;

        /** @brief True when its `.cnaenv` parsed. False for a file from a newer Studio. */
        bool readable = false;

        /** @brief The panorama it names, or nil when nobody has chosen one. */
        Uuid panorama;

        /** @brief True when @ref panorama names an asset and that asset is a texture. */
        bool panoramaIsTexture = false;
    };

    /** @brief How the caller answers @ref SceneSkySource for one asset id. */
    using SceneSkySourceProvider = std::function<SceneSkySource(const Uuid&)>;

    /**
     * @brief Why a scene has no sky. Every value is an ordinary state rather than a failure.
     *
     * Named rather than reduced to a boolean because the five are *different problems with
     * different fixes*, and an editor that reports "no sky" for all of them has told the user only
     * what they can already see.
     */
    enum class SceneSkyProblem
    {
        /** @brief Nothing is wrong; the chain resolves. */
        None,

        /** @brief The scene names no environment map, which is most scenes and is not a fault. */
        NoEnvironmentMap,

        /** @brief The id names nothing, or names something that is not an environment map. */
        EnvironmentMapMissing,

        /** @brief The `.cnaenv` is there and would not parse -- a newer Studio, or bad JSON. */
        EnvironmentMapUnreadable,

        /** @brief The environment map exists and nobody has chosen a panorama for it. */
        NoPanorama,

        /** @brief It names a panorama and that asset is gone, or is not a texture. */
        PanoramaMissing,
    };

    /** @brief A sentence for the user, or an empty string for @ref SceneSkyProblem::None. */
    [[nodiscard]] std::string describeSceneSkyProblem(SceneSkyProblem problem);

    /** @brief The stable name of @p problem, for tests and for the validation rule id. */
    [[nodiscard]] const char* toString(SceneSkyProblem problem);

    /** @brief What the viewport should do about this scene's sky. */
    struct SceneSkyPlan
    {
        /** @brief True when the sky should be drawn behind the scene. */
        bool draws = false;

        /** @brief True when the environment should light the scene. */
        bool lights = false;

        /** @brief The panorama to process, or nil when there is nothing to process. */
        Uuid panorama;

        /** @brief The environment map asset the panorama came from, for caching by identity. */
        Uuid environmentMap;

        /** @brief Multiplies both the sky's brightness and the light it casts. */
        float intensity = 1.0f;

        /**
         * @brief How far the sky is turned about the world's Y axis, **in radians**.
         *
         * Radians here and degrees in the document, which is the same split `CNA.Light`'s cone
         * angles use: a person types degrees and every trigonometric consumer wants radians, so the
         * conversion happens once, where the document is read.
         */
        float yaw = 0.0f;

        /** @brief Which link of the chain is missing, or `None`. */
        SceneSkyProblem problem = SceneSkyProblem::NoEnvironmentMap;

        /** @brief True when there is a panorama to process and nothing stopping it. */
        [[nodiscard]] bool isUsable() const
        {
            return problem == SceneSkyProblem::None && panorama.isValid();
        }
    };

    /**
     * @brief Walks @p environment's environment map through @p provider and says what it found.
     *
     * @param provider How an environment map id becomes a @ref SceneSkySource. A provider that is
     *        not callable is treated as one that finds nothing, which is what a headless caller
     *        with no database looks like -- and is the honest answer rather than a crash.
     *
     * **Both switches off is still a resolved chain.** A user who has set an environment map and
     * unticked both Show Sky and Light From Sky has said what they want; the plan reports
     * `problem == None` with `draws` and `lights` false, and nothing warns at them about a state
     * they chose. What a problem means is that the editor cannot do what the scene asks.
     */
    [[nodiscard]] SceneSkyPlan planSceneSky(const SceneEnvironment& environment,
                                            const SceneSkySourceProvider& provider);
}
