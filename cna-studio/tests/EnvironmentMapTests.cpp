// SPDX-License-Identifier: MS-PL
/**
 * @file EnvironmentMapTests.cpp
 * @brief What an environment map is, and what its settings will cost (`plan.md` STUDIO-10010).
 *
 * Two things that go wrong here, and neither of them is arithmetic.
 *
 * The first is a number a user typed that the editor quietly changed. Every size and sample count
 * on an environment map is multiplied into CPU work — six faces, times a texel count, times a
 * sample count, and for irradiance the sample count is *per axis* so it enters squared. The bounds
 * that keep an import from taking a quarter of an hour are necessary and are exactly the kind of
 * thing that gets applied silently, so every clamp here is asserted to produce a note.
 *
 * The second is a file that round-trips to something other than itself. A `.cnaenv` holds one
 * reference and nine numbers, which is small enough that a missing key looks like a default and a
 * default looks like a choice; the round-trip cases are what tell those apart.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Assets/EnvironmentMapDocument.hpp"
#include "CNA/Studio/Assets/EnvironmentMapImport.hpp"
#include "CNA/Studio/Core/Json.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

using namespace CNA::Studio;

namespace
{
    /** @brief True when any note mentions @p fragment, which is how a clamp announces itself. */
    bool notesMention(const StudioEnvironmentMapPlan& plan, std::string_view fragment)
    {
        return std::any_of(plan.notes.begin(), plan.notes.end(),
                           [fragment](const std::string& note) {
                               return note.find(fragment) != std::string::npos;
                           });
    }

    /** @brief A panorama of the shape every HDR sky ships in: twice as wide as it is tall. */
    StudioEnvironmentMapSource panorama(int width)
    {
        return StudioEnvironmentMapSource{width, width / 2};
    }
}

/**
 * @brief A face size of zero is a ratio, not a missing answer.
 *
 * A quarter of the panorama's width is what CNA's own documentation calls the usual choice, and it
 * is the default here precisely because it is a *ratio*: swapping a 2K sky for an 8K one should
 * give a sharper cube without the user editing a second field. A fixed default would silently
 * throw the larger one's detail away.
 */
CNA_STUDIO_TEST(AnUnsetFaceSizeFollowsThePanoramaRatherThanAFixedNumber)
{
    const StudioEnvironmentMapImportSettings defaults;
    CNA_STUDIO_EXPECT_EQ(defaults.faceSize, 0);

    CNA_STUDIO_EXPECT_EQ(studioPlanEnvironmentMapImport(defaults, panorama(2048)).faceSize, 512);
    CNA_STUDIO_EXPECT_EQ(studioPlanEnvironmentMapImport(defaults, panorama(8192)).faceSize, 2048);

    // And it is still bounded. A 32K panorama would derive an 8192-pixel face, which is four cube
    // maps' worth of CPU convolution before anything has been drawn.
    const StudioEnvironmentMapPlan huge = studioPlanEnvironmentMapImport(defaults, panorama(32768));
    CNA_STUDIO_EXPECT_EQ(huge.faceSize, kMaximumEnvironmentFaceSize);

    // A number the user typed is used as typed, which is the whole point of being able to type one.
    StudioEnvironmentMapImportSettings chosen;
    chosen.faceSize = 256;
    const StudioEnvironmentMapPlan explicitSize =
        studioPlanEnvironmentMapImport(chosen, panorama(8192));
    CNA_STUDIO_EXPECT_EQ(explicitSize.faceSize, 256);
    CNA_STUDIO_EXPECT(!notesMention(explicitSize, "Face Size"));

    // An unmeasured panorama cannot derive anything, and says so rather than picking a number and
    // leaving the user to find out which one.
    const StudioEnvironmentMapPlan unmeasured =
        studioPlanEnvironmentMapImport(defaults, StudioEnvironmentMapSource{});
    CNA_STUDIO_EXPECT_EQ(unmeasured.faceSize, 0);
    CNA_STUDIO_EXPECT(notesMention(unmeasured, "has not been measured"));

    // But the rest of the plan is still answerable, which is the difference between an inspector
    // that shows most of the answer and one that shows none of it.
    CNA_STUDIO_EXPECT_EQ(unmeasured.irradianceSize, defaults.irradianceSize);
    CNA_STUDIO_EXPECT(unmeasured.estimatedSamples > 0);
}

/** @brief Every bound the editor applies is a bound the user is told about. */
CNA_STUDIO_TEST(EveryClampedSettingSaysSoRatherThanChangingQuietly)
{
    StudioEnvironmentMapImportSettings absurd;
    absurd.faceSize = 99999;
    absurd.irradianceSize = 4096;
    absurd.irradianceSamples = 10000;
    absurd.specularBaseSize = 99999;
    absurd.specularSamples = 10000;
    absurd.brdfLutSamples = 10000;

    const StudioEnvironmentMapPlan plan = studioPlanEnvironmentMapImport(absurd, panorama(4096));

    CNA_STUDIO_EXPECT_EQ(plan.faceSize, kMaximumEnvironmentFaceSize);
    CNA_STUDIO_EXPECT_EQ(plan.irradianceSize, kMaximumEnvironmentIrradianceSize);
    CNA_STUDIO_EXPECT_EQ(plan.specularBaseSize, kMaximumEnvironmentSpecularSize);

    for (const char* field : {"Face Size", "Irradiance Size", "Irradiance Samples",
                              "Specular Size", "Specular Samples", "BRDF Table Samples"})
    {
        if (!notesMention(plan, field))
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"'"} + field
                + "' was clamped and nothing said so, which is a number the user typed, kept on "
                  "screen, and not used.");
        }
    }

    // The other direction too. Zero samples is not "cheap", it is an output with nothing in it.
    StudioEnvironmentMapImportSettings empty;
    empty.irradianceSamples = 0;
    empty.specularSamples = -4;
    const StudioEnvironmentMapPlan raised = studioPlanEnvironmentMapImport(empty, panorama(1024));
    CNA_STUDIO_EXPECT(notesMention(raised, "Irradiance Samples"));
    CNA_STUDIO_EXPECT(notesMention(raised, "Specular Samples"));

    // And a plan nobody has pushed is exactly what was asked for, which is what makes the notes
    // worth reading at all: a list that is never empty is a list nobody reads.
    const StudioEnvironmentMapPlan ordinary =
        studioPlanEnvironmentMapImport(StudioEnvironmentMapImportSettings{}, panorama(2048));
    CNA_STUDIO_EXPECT(ordinary.isExactlyAsAsked());
}

/**
 * @brief A mip chain cannot run past one texel, and a roughness cannot read a mip that is not there.
 *
 * `EnvironmentProcessor::mipForRoughness(roughness, mipCount)` spreads roughness across the levels
 * a cube claims to have. A `mipCount` that lies is not a wasted level — it is every roughness
 * reading the wrong level, which looks like reflections that sharpen as a surface gets rougher.
 */
CNA_STUDIO_TEST(ASpecularChainIsShortenedRatherThanClaimingLevelsItCannotHave)
{
    StudioEnvironmentMapImportSettings settings;
    settings.specularBaseSize = 16;
    settings.specularMipCount = 12;

    const StudioEnvironmentMapPlan plan = studioPlanEnvironmentMapImport(settings, panorama(1024));

    // 16, 8, 4, 2, 1 -- five levels and no more.
    CNA_STUDIO_EXPECT_EQ(plan.specularMipCount, 5);
    CNA_STUDIO_EXPECT(notesMention(plan, "Specular Mips"));

    // A chain that fits is left alone.
    settings.specularBaseSize = 128;
    settings.specularMipCount = 5;
    const StudioEnvironmentMapPlan fits = studioPlanEnvironmentMapImport(settings, panorama(1024));
    CNA_STUDIO_EXPECT_EQ(fits.specularMipCount, 5);
    CNA_STUDIO_EXPECT(!notesMention(fits, "Specular Mips"));
}

/**
 * @brief The cost figure is what tells a user their irradiance change cost four times as much.
 *
 * Irradiance samples are **per axis**, so the cost is the square — the single most surprising
 * number in the whole dialogue and the reason the figure is shown at all.
 */
CNA_STUDIO_TEST(TheEstimatedCostSquaresWithIrradianceSamplesAndScalesWithSpecularOnes)
{
    // Only the irradiance stage, so the arithmetic being asserted is not buried under the others.
    StudioEnvironmentMapImportSettings onlyIrradiance;
    onlyIrradiance.faceSize = kMinimumEnvironmentFaceSize;
    onlyIrradiance.specularBaseSize = kMinimumEnvironmentSpecularSize;
    onlyIrradiance.specularMipCount = 1;
    onlyIrradiance.specularSamples = 1;
    onlyIrradiance.generateBrdfLut = false;

    onlyIrradiance.irradianceSamples = 16;
    const std::uint64_t cheap =
        studioPlanEnvironmentMapImport(onlyIrradiance, panorama(1024)).estimatedSamples;

    onlyIrradiance.irradianceSamples = 32;
    const std::uint64_t dear =
        studioPlanEnvironmentMapImport(onlyIrradiance, panorama(1024)).estimatedSamples;

    // Four times the irradiance work for twice the number, plus the stages that did not change.
    const std::uint64_t fixed = cheap - 16ULL * 16ULL * 6 * 32 * 32;
    CNA_STUDIO_EXPECT_EQ(dear - fixed, (cheap - fixed) * 4);

    // Specular samples are per texel rather than per axis, so they scale linearly -- which is the
    // contrast that makes the irradiance square worth pointing at.
    StudioEnvironmentMapImportSettings onlySpecular;
    onlySpecular.faceSize = kMinimumEnvironmentFaceSize;
    onlySpecular.irradianceSize = kMinimumEnvironmentIrradianceSize;
    onlySpecular.irradianceSamples = 1;
    onlySpecular.generateBrdfLut = false;

    onlySpecular.specularSamples = 32;
    const std::uint64_t thin =
        studioPlanEnvironmentMapImport(onlySpecular, panorama(1024)).estimatedSamples;
    onlySpecular.specularSamples = 64;
    const std::uint64_t thick =
        studioPlanEnvironmentMapImport(onlySpecular, panorama(1024)).estimatedSamples;
    CNA_STUDIO_EXPECT(thick > thin);
    CNA_STUDIO_EXPECT(thick < thin * 3);

    // And past a scale nobody means to ask for, the plan says so out loud.
    StudioEnvironmentMapImportSettings expensive;
    expensive.irradianceSize = kMaximumEnvironmentIrradianceSize;
    expensive.irradianceSamples = kMaximumEnvironmentSampleCount;
    const StudioEnvironmentMapPlan loud =
        studioPlanEnvironmentMapImport(expensive, panorama(4096));
    CNA_STUDIO_EXPECT(loud.estimatedSamples >= kEnvironmentExpensiveSampleCount);
    CNA_STUDIO_EXPECT(notesMention(loud, "noticeable moment"));

    // The defaults are nowhere near it, which is what makes the warning mean something.
    CNA_STUDIO_EXPECT(
        studioPlanEnvironmentMapImport(StudioEnvironmentMapImportSettings{}, panorama(2048))
            .estimatedSamples < kEnvironmentExpensiveSampleCount);
}

/** @brief Turning the BRDF table off removes its memory and its cost, and nothing else. */
CNA_STUDIO_TEST(TheBrdfTableIsOptionalBecauseEveryEnvironmentMapGeneratesTheSameOne)
{
    StudioEnvironmentMapImportSettings with;
    const StudioEnvironmentMapPlan withTable = studioPlanEnvironmentMapImport(with, panorama(2048));

    StudioEnvironmentMapImportSettings without = with;
    without.generateBrdfLut = false;
    const StudioEnvironmentMapPlan withoutTable =
        studioPlanEnvironmentMapImport(without, panorama(2048));

    CNA_STUDIO_EXPECT_EQ(withTable.brdfLutSize, with.brdfLutSize);
    CNA_STUDIO_EXPECT_EQ(withoutTable.brdfLutSize, 0);
    CNA_STUDIO_EXPECT(withoutTable.estimatedBytes < withTable.estimatedBytes);
    CNA_STUDIO_EXPECT(withoutTable.estimatedSamples < withTable.estimatedSamples);

    // The cubes are untouched: the table depends on nothing but the BRDF, so switching it off is
    // not a quality setting.
    CNA_STUDIO_EXPECT_EQ(withoutTable.faceSize, withTable.faceSize);
    CNA_STUDIO_EXPECT_EQ(withoutTable.irradianceSize, withTable.irradianceSize);
    CNA_STUDIO_EXPECT_EQ(withoutTable.specularMipCount, withTable.specularMipCount);
}

/** @brief A panorama that is not twice as wide as it is tall is drawn, and mentioned. */
CNA_STUDIO_TEST(AnOddlyShapedPanoramaIsAcceptedAndReported)
{
    const StudioEnvironmentMapImportSettings settings;

    CNA_STUDIO_EXPECT(!notesMention(studioPlanEnvironmentMapImport(settings, panorama(2048)),
                                    "not twice as wide"));

    const StudioEnvironmentMapPlan square = studioPlanEnvironmentMapImport(
        settings, StudioEnvironmentMapSource{1024, 1024});
    CNA_STUDIO_EXPECT(notesMention(square, "not twice as wide"));

    // Accepted, not refused: a slightly-off panorama is far more commonly a real sky somebody
    // cropped than a mistake, and CNA takes the same position for the same reason.
    CNA_STUDIO_EXPECT(square.faceSize > 0);
    CNA_STUDIO_EXPECT(square.estimatedBytes > 0);
}

/** @brief A `.cnaenv` comes back as itself, and an absent field comes back as its default. */
CNA_STUDIO_TEST(AnEnvironmentMapDocumentRoundTripsThroughJson)
{
    EnvironmentMapDocument written;
    written.name = "Overcast Afternoon";
    written.panorama = Uuid::generate();
    written.settings.faceSize = 256;
    written.settings.irradianceSize = 64;
    written.settings.irradianceSamples = 48;
    written.settings.specularBaseSize = 256;
    written.settings.specularMipCount = 6;
    written.settings.specularSamples = 96;
    written.settings.generateBrdfLut = false;
    written.settings.brdfLutSize = 64;
    written.settings.brdfLutSamples = 96;

    const std::string text = Json::write(written.toJson(), true);
    const JsonParseResult parsed = Json::parse(text);
    CNA_STUDIO_EXPECT(parsed.succeeded);

    EnvironmentMapDocument read;
    CNA_STUDIO_EXPECT(read.loadFromJson(parsed.value));

    CNA_STUDIO_EXPECT_EQ(read.name, written.name);
    CNA_STUDIO_EXPECT(read.panorama == written.panorama);
    CNA_STUDIO_EXPECT_EQ(read.settings.faceSize, 256);
    CNA_STUDIO_EXPECT_EQ(read.settings.irradianceSize, 64);
    CNA_STUDIO_EXPECT_EQ(read.settings.irradianceSamples, 48);
    CNA_STUDIO_EXPECT_EQ(read.settings.specularBaseSize, 256);
    CNA_STUDIO_EXPECT_EQ(read.settings.specularMipCount, 6);
    CNA_STUDIO_EXPECT_EQ(read.settings.specularSamples, 96);
    CNA_STUDIO_EXPECT(!read.settings.generateBrdfLut);
    CNA_STUDIO_EXPECT_EQ(read.settings.brdfLutSize, 64);
    CNA_STUDIO_EXPECT_EQ(read.settings.brdfLutSamples, 96);

    // **An absent setting is the declared default, not a zero.** Reading a missing number as zero
    // would ask for a one-texel irradiance cube and a specular chain with no mips in it -- which
    // is the difference between an old file that keeps working and one that silently degrades.
    const JsonParseResult minimal = Json::parse(R"({"formatVersion":1,"name":"Sky"})");
    CNA_STUDIO_EXPECT(minimal.succeeded);

    EnvironmentMapDocument sparse;
    CNA_STUDIO_EXPECT(sparse.loadFromJson(minimal.value));

    const StudioEnvironmentMapImportSettings defaults;
    CNA_STUDIO_EXPECT_EQ(sparse.name, "Sky");
    CNA_STUDIO_EXPECT(!sparse.panorama.isValid());
    CNA_STUDIO_EXPECT_EQ(sparse.settings.irradianceSize, defaults.irradianceSize);
    CNA_STUDIO_EXPECT_EQ(sparse.settings.specularMipCount, defaults.specularMipCount);
    CNA_STUDIO_EXPECT_EQ(sparse.settings.generateBrdfLut, defaults.generateBrdfLut);

    // A version this build cannot read is the one hard failure, for the reason a material's is:
    // quietly rewriting somebody's file with less in it than they put there is worse than
    // refusing to open it.
    const JsonParseResult future = Json::parse(R"({"formatVersion":99})");
    CNA_STUDIO_EXPECT(future.succeeded);
    EnvironmentMapDocument tooNew;
    CNA_STUDIO_EXPECT(!tooNew.loadFromJson(future.value));
}
