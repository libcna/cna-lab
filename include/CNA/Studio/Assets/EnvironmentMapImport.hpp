// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Assets/EnvironmentMapImport.hpp
 * @brief What an environment map's processing settings will produce, worked out rather than stored.
 *
 * `plan.md` STUDIO-10010. The same division `TextureImport.hpp` draws, for the same reason: a
 * *setting* is what the user chose, a *fact* is what the file says, and "how big will the cube be,
 * what will it cost, and how long will it take" is neither — it is the two combined, and writing a
 * combination into a sidecar is how a project ends up with a stale number next to the setting that
 * used to produce it.
 *
 * ### What the processing is
 *
 * CNA's `CNA::Graphics::EnvironmentProcessor` turns one equirectangular panorama into the three
 * things an image-based light needs, and `ImageBasedLightEXT` names all three:
 *
 * - a **cube map**, because a panorama is longitude across and latitude down and a renderer samples
 *   a cube;
 * - an **irradiance cube**, each texel the cosine-weighted average of the hemisphere around it,
 *   which is what a matte surface facing that way receives;
 * - a **prefiltered specular cube**, one mip per roughness, which is the first half of the split-sum
 *   approximation. The second half is a **BRDF lookup table** that depends on nothing but the BRDF,
 *   so it is the same image in every project.
 *
 * ### Why the cost matters enough to predict
 *
 * **The conversion runs on the CPU**, and CNA says why: a render-to-cube version would need float
 * render targets, cube render targets and custom effects all present, and the processor has to work
 * on renderers that have none of them. That is the right trade for a load-time cost paid once — and
 * it means the numbers a user types here are multiplied into an amount of arithmetic that can run
 * from imperceptible to minutes. Irradiance is the sharp edge: its sample count is *per axis*, so
 * the cost is its square, and going from 32 to 64 is four times the work rather than twice.
 *
 * So this file predicts it, in samples rather than in seconds. Seconds would be a guess about a
 * machine this process is not running on; a sample count is arithmetic, is comparable between two
 * settings, and is the thing that actually doubled.
 *
 * ### Everything here is CNA-free
 *
 * `cna-studio-assets` is one of the modules that may not name a CNA type (`ANALYSIS.md` D-03), and
 * a build without CNA still has to tell a user what their settings do.
 *
 * The defaults below started as `EnvironmentProcessor`'s own, which is why they look familiar —
 * but they are **Studio's**, not a mirror that could fall out of step. Every call the viewport
 * makes passes these numbers explicitly, so CNA's own defaults are never reached and a divergence
 * would be a difference between two editors rather than a bug in one. That is deliberate: a copy
 * that has to agree with something is a copy that eventually does not, and the way to avoid it is
 * to stop depending on the agreement.
 */

#include <cstdint>
#include <string>
#include <vector>

namespace CNA::Studio
{
    class JsonValue;

    /**
     * @brief The bounds every size and sample count is clamped into, and why they are not wider.
     *
     * A panorama is user-supplied and so are these numbers, and the work is a product of them: six
     * faces times a texel count times a sample count, on the CPU, on the thread that is also
     * drawing the editor. The upper bounds are where a one-off import cost stops being one-off, and
     * the lower ones are where the output stops carrying the signal it is for.
     */
    inline constexpr int kMinimumEnvironmentFaceSize = 16;
    inline constexpr int kMaximumEnvironmentFaceSize = 2048;
    inline constexpr int kMinimumEnvironmentIrradianceSize = 8;

    /**
     * @brief 128, and the ceiling is low on purpose.
     *
     * Irradiance is a very low-frequency signal — CNA's own default is 32 a face and its header
     * says that is already more than the function has detail to fill. A bigger one is not a
     * sharper image, it is the same image computed more expensively.
     */
    inline constexpr int kMaximumEnvironmentIrradianceSize = 128;

    inline constexpr int kMinimumEnvironmentSampleCount = 1;

    /** @brief 256 a side for irradiance, whose cost is the *square* of this. */
    inline constexpr int kMaximumEnvironmentSampleCount = 256;

    inline constexpr int kMinimumEnvironmentSpecularSize = 16;
    inline constexpr int kMaximumEnvironmentSpecularSize = 1024;
    inline constexpr int kMaximumEnvironmentMipCount = 12;

    /**
     * @brief Above this, the plan says out loud that the import will be felt.
     *
     * Four hundred million texel-samples. Not a time — this process cannot know the machine — but
     * a scale: the defaults come to a few million, so a setting that reaches this is two orders of
     * magnitude past them and the user has almost certainly not meant it.
     */
    inline constexpr std::uint64_t kEnvironmentExpensiveSampleCount = 400'000'000ULL;

    /** @brief What the panorama file says about itself, as far as its header goes. */
    struct StudioEnvironmentMapSource
    {
        int width = 0;
        int height = 0;

        /** @brief False when nothing has measured the file, which is different from "0 x 0". */
        [[nodiscard]] bool isMeasured() const { return width > 0 && height > 0; }
    };

    /**
     * @brief What the user chose. Every field is a setting; none of them is ever a fact.
     *
     * The defaults match what a CNA program calling `EnvironmentProcessor` with no arguments would
     * get, so an environment map nobody has configured processes the way the API's own
     * documentation describes. See this file's header for why they are stated here rather than
     * inherited.
     */
    struct StudioEnvironmentMapImportSettings
    {
        /**
         * @brief Cube face edge in pixels, or **0 meaning "a quarter of the panorama's width"**.
         *
         * Zero is a real answer rather than a missing one: a quarter of the width is what CNA's own
         * documentation calls the usual choice and roughly preserves the panorama's detail, and it
         * is a *ratio*, so it keeps being right when somebody swaps a 2K sky for an 8K one. A fixed
         * number would silently throw away the second one's detail or upsample the first's.
         */
        int faceSize = 0;

        int irradianceSize = 32;

        /** @brief Samples per axis of the hemisphere sweep. **The cost is its square.** */
        int irradianceSamples = 32;

        int specularBaseSize = 128;
        int specularMipCount = 5;
        int specularSamples = 64;

        /**
         * @brief Whether to generate the split-sum BRDF table beside the cubes.
         *
         * On by default and worth being able to turn off: the table depends on nothing but the
         * BRDF, so every environment map in a project generates the same image. A project with
         * twelve skies pays for it twelve times unless something shares it, and until something
         * does, a user who knows that can switch it off here.
         */
        bool generateBrdfLut = true;

        int brdfLutSize = 128;
        int brdfLutSamples = 128;

        /**
         * @brief Reads the settings out of a `.cnaenv`'s `processing` object, defaulting each.
         *
         * A missing key is the declared default and never a zero, which here is not merely tidier:
         * zero asks for a one-texel irradiance cube and a specular chain with no mips in it, so an
         * older file read the careless way would not degrade, it would produce nothing.
         */
        [[nodiscard]] static StudioEnvironmentMapImportSettings fromJson(const JsonValue& json);

        /** @brief Writes the settings as the object `fromJson` reads. */
        [[nodiscard]] JsonValue toJson() const;
    };

    /** @brief What @ref studioPlanEnvironmentMapImport worked out. */
    struct StudioEnvironmentMapPlan
    {
        /** @brief The face edge actually used, with @ref StudioEnvironmentMapImportSettings::faceSize
         *         resolved and clamped. Zero when the panorama is unmeasured and none was given. */
        int faceSize = 0;

        int irradianceSize = 0;
        int specularBaseSize = 0;

        /** @brief Mips actually generated, reduced when the chain would run past one texel. */
        int specularMipCount = 0;

        int brdfLutSize = 0;

        /** @brief What the generated textures occupy, at four bytes a texel. */
        std::uint64_t estimatedBytes = 0;

        /**
         * @brief Texel-samples the processor will evaluate, which is the shape of the CPU cost.
         *
         * A proxy rather than a promise: the four stages do different work per sample, so this is
         * comparable between two settings rather than convertible into seconds. It is still the
         * number that tells a user their irradiance change cost them four times as much, which no
         * other figure on the screen does.
         */
        std::uint64_t estimatedSamples = 0;

        /**
         * @brief Why the plan is not exactly what was asked for. Empty when it is.
         *
         * Notes rather than errors, because none of these stops the processing: each is a setting
         * that had to be resolved to something else, and a user who cannot see *which* is left
         * with a sky that is quietly the wrong size.
         */
        std::vector<std::string> notes;

        [[nodiscard]] bool isExactlyAsAsked() const { return notes.empty(); }
    };

    /**
     * @brief Works out what @p settings applied to @p source actually produce.
     *
     * Pure: no filesystem, no graphics device, no state. That is what lets the inspector call it
     * every frame and a test call it a thousand times.
     *
     * An unmeasured @p source is not an error. It is what an asset looks like before anything has
     * read its header, and every field that does not depend on the panorama is still answerable —
     * which is the difference between an inspector that shows most of the answer and one that
     * shows none of it.
     */
    [[nodiscard]] StudioEnvironmentMapPlan studioPlanEnvironmentMapImport(
        const StudioEnvironmentMapImportSettings& settings,
        const StudioEnvironmentMapSource& source);
}
