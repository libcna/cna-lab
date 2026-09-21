// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Assets/EnvironmentMapImport.hpp"

#include <algorithm>
#include <string>

#include "CNA/Studio/Core/Json.hpp"

namespace CNA::Studio
{
    namespace
    {
        /** @brief Four bytes a texel, which is what `SurfaceFormat::Color` is. */
        constexpr std::uint64_t kBytesPerTexel = 4;

        /**
         * @brief Clamps @p value into @p low .. @p high, and says so when it had to.
         *
         * Every clamp in this file produces a note, because a silent one is the failure mode the
         * whole file exists to prevent: a user types 4096, sees the field keep saying 4096, and
         * gets a 2048 cube with nothing on screen to explain the difference.
         */
        int clampWithNote(int value, int low, int high, const char* what,
                          std::vector<std::string>& notes)
        {
            if (value < low)
            {
                notes.push_back(std::string{what} + " was raised to " + std::to_string(low)
                                + ", the smallest this editor generates.");
                return low;
            }
            if (value > high)
            {
                notes.push_back(std::string{what} + " was reduced to " + std::to_string(high)
                                + ", the largest this editor generates on the CPU.");
                return high;
            }
            return value;
        }

        /** @brief Texels in a cube of @p size, i.e. six faces of it. */
        std::uint64_t cubeTexels(int size)
        {
            const std::uint64_t edge = static_cast<std::uint64_t>(std::max(0, size));
            return 6 * edge * edge;
        }

        /** @brief Texels in a cube mip chain of @p mipCount levels from @p baseSize. */
        std::uint64_t cubeChainTexels(int baseSize, int mipCount)
        {
            std::uint64_t total = 0;
            for (int mip = 0; mip < mipCount; ++mip)
            {
                total += cubeTexels(std::max(1, baseSize >> mip));
            }
            return total;
        }
    }

    StudioEnvironmentMapImportSettings StudioEnvironmentMapImportSettings::fromJson(
        const JsonValue& json)
    {
        StudioEnvironmentMapImportSettings settings;

        // Absent means "the user never chose", which is the declared default rather than a zero --
        // and here a zero is not even neutral: it would ask for a one-texel irradiance cube and a
        // prefiltered chain with no mips in it.
        const auto readInt = [&json](const char* name, int& field) {
            const JsonValue& value = json[name];
            if (!value.isNull()) { field = value.asInt(field); }
        };

        readInt("faceSize", settings.faceSize);
        readInt("irradianceSize", settings.irradianceSize);
        readInt("irradianceSamples", settings.irradianceSamples);
        readInt("specularBaseSize", settings.specularBaseSize);
        readInt("specularMipCount", settings.specularMipCount);
        readInt("specularSamples", settings.specularSamples);
        readInt("brdfLutSize", settings.brdfLutSize);
        readInt("brdfLutSamples", settings.brdfLutSamples);

        const JsonValue& brdf = json["generateBrdfLut"];
        if (!brdf.isNull()) { settings.generateBrdfLut = brdf.asBoolean(settings.generateBrdfLut); }

        return settings;
    }

    JsonValue StudioEnvironmentMapImportSettings::toJson() const
    {
        JsonValue json = JsonValue::makeObject();
        json.set("faceSize", JsonValue{static_cast<double>(faceSize)});
        json.set("irradianceSize", JsonValue{static_cast<double>(irradianceSize)});
        json.set("irradianceSamples", JsonValue{static_cast<double>(irradianceSamples)});
        json.set("specularBaseSize", JsonValue{static_cast<double>(specularBaseSize)});
        json.set("specularMipCount", JsonValue{static_cast<double>(specularMipCount)});
        json.set("specularSamples", JsonValue{static_cast<double>(specularSamples)});
        json.set("generateBrdfLut", JsonValue{generateBrdfLut});
        json.set("brdfLutSize", JsonValue{static_cast<double>(brdfLutSize)});
        json.set("brdfLutSamples", JsonValue{static_cast<double>(brdfLutSamples)});
        return json;
    }

    StudioEnvironmentMapPlan studioPlanEnvironmentMapImport(
        const StudioEnvironmentMapImportSettings& settings,
        const StudioEnvironmentMapSource& source)
    {
        StudioEnvironmentMapPlan plan;

        // A panorama is longitude across and latitude down, so it wants to be twice as wide as it
        // is tall. Noted rather than refused, and CNA takes the same position for the same reason:
        // a slightly-off panorama is far more commonly a real sky somebody cropped than a mistake,
        // and refusing it helps nobody.
        if (source.isMeasured() && source.width != source.height * 2)
        {
            plan.notes.push_back(
                "The panorama is " + std::to_string(source.width) + " x "
                + std::to_string(source.height)
                + ", not twice as wide as it is tall. It is sampled as given, so the sky will be "
                  "stretched or squashed by the difference.");
        }

        // Zero means "a quarter of the panorama's width", which is a ratio rather than a number and
        // is why it keeps being right when the sky is replaced by a larger one.
        int faceSize = settings.faceSize;
        if (faceSize <= 0)
        {
            faceSize = source.isMeasured() ? source.width / 4 : 0;
            if (faceSize > 0)
            {
                faceSize = std::clamp(faceSize, kMinimumEnvironmentFaceSize,
                                      kMaximumEnvironmentFaceSize);
            }
        }
        else
        {
            faceSize = clampWithNote(faceSize, kMinimumEnvironmentFaceSize,
                                     kMaximumEnvironmentFaceSize, "Face Size", plan.notes);
        }

        plan.faceSize = faceSize;
        plan.irradianceSize =
            clampWithNote(settings.irradianceSize, kMinimumEnvironmentIrradianceSize,
                          kMaximumEnvironmentIrradianceSize, "Irradiance Size", plan.notes);
        plan.specularBaseSize =
            clampWithNote(settings.specularBaseSize, kMinimumEnvironmentSpecularSize,
                          kMaximumEnvironmentSpecularSize, "Specular Size", plan.notes);

        const int irradianceSamples =
            clampWithNote(settings.irradianceSamples, kMinimumEnvironmentSampleCount,
                          kMaximumEnvironmentSampleCount, "Irradiance Samples", plan.notes);
        const int specularSamples =
            clampWithNote(settings.specularSamples, kMinimumEnvironmentSampleCount,
                          kMaximumEnvironmentSampleCount, "Specular Samples", plan.notes);

        int mipCount = clampWithNote(settings.specularMipCount, 1, kMaximumEnvironmentMipCount,
                                     "Specular Mips", plan.notes);

        // A chain cannot run past one texel, and one that claims to is a chain whose last levels
        // are all the same 1x1 image computed several times over. Reduced rather than accepted,
        // because the mip a roughness reads is `mipForRoughness(roughness, mipCount)` -- so a
        // mipCount that lies spreads every roughness across levels that do not exist.
        int usableMips = 1;
        while (usableMips < mipCount && (plan.specularBaseSize >> usableMips) >= 1)
        {
            ++usableMips;
        }
        if (usableMips < mipCount)
        {
            plan.notes.push_back(
                "Specular Mips was reduced to " + std::to_string(usableMips) + ": a "
                + std::to_string(plan.specularBaseSize)
                + "-pixel face has no more levels above one texel, and a roughness cannot read a "
                  "mip that is not there.");
            mipCount = usableMips;
        }
        plan.specularMipCount = mipCount;

        plan.brdfLutSize =
            settings.generateBrdfLut
                ? clampWithNote(settings.brdfLutSize, kMinimumEnvironmentSpecularSize,
                                kMaximumEnvironmentSpecularSize, "BRDF Table Size", plan.notes)
                : 0;
        const int brdfSamples =
            settings.generateBrdfLut
                ? clampWithNote(settings.brdfLutSamples, kMinimumEnvironmentSampleCount,
                                kMaximumEnvironmentSampleCount, "BRDF Table Samples", plan.notes)
                : 0;

        const std::uint64_t brdfTexels = static_cast<std::uint64_t>(plan.brdfLutSize)
                                       * static_cast<std::uint64_t>(plan.brdfLutSize);

        plan.estimatedBytes =
            (cubeTexels(plan.faceSize) + cubeTexels(plan.irradianceSize)
             + cubeChainTexels(plan.specularBaseSize, plan.specularMipCount) + brdfTexels)
            * kBytesPerTexel;

        // The cost, stage by stage. The conversion visits each output texel once; the irradiance
        // convolution integrates a hemisphere per output texel, and its sample count is **per
        // axis**, so it appears here squared -- which is the single most surprising number in the
        // whole dialogue and the main reason this figure is shown at all.
        plan.estimatedSamples =
            cubeTexels(plan.faceSize)
            + cubeTexels(plan.irradianceSize) * static_cast<std::uint64_t>(irradianceSamples)
                  * static_cast<std::uint64_t>(irradianceSamples)
            + cubeChainTexels(plan.specularBaseSize, plan.specularMipCount)
                  * static_cast<std::uint64_t>(specularSamples)
            + brdfTexels * static_cast<std::uint64_t>(brdfSamples);

        if (plan.estimatedSamples >= kEnvironmentExpensiveSampleCount)
        {
            plan.notes.push_back(
                "This will take a noticeable moment: the processing runs on the CPU and these "
                "settings come to roughly "
                + std::to_string(plan.estimatedSamples / 1'000'000ULL)
                + " million samples. Irradiance Samples is the usual cause -- it is per axis, so "
                  "its cost is the square.");
        }

        // Last, because it is about the *source* rather than about a setting, and a user who has
        // not measured the file yet should see every answer that does not depend on it.
        if (!source.isMeasured())
        {
            plan.notes.push_back(
                "The panorama has not been measured yet, so the cube's face size cannot be "
                "derived from it. Set Face Size explicitly to decide it now.");
        }

        return plan;
    }
}
