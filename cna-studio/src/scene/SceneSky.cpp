// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Scene/SceneSky.hpp"

#include <algorithm>

namespace CNA::Studio
{
    namespace
    {
        /** @brief What `CNA.Light`'s cone angles use, restated where the other converter cannot see. */
        constexpr float kDegreesToRadians = 3.14159265358979323846f / 180.0f;
    }

    const char* toString(SceneSkyProblem problem)
    {
        switch (problem)
        {
            case SceneSkyProblem::None:                     return "none";
            case SceneSkyProblem::NoEnvironmentMap:         return "no-environment-map";
            case SceneSkyProblem::EnvironmentMapMissing:    return "environment-map-missing";
            case SceneSkyProblem::EnvironmentMapUnreadable: return "environment-map-unreadable";
            case SceneSkyProblem::NoPanorama:               return "no-panorama";
            case SceneSkyProblem::PanoramaMissing:          return "panorama-missing";
        }
        return "none";
    }

    std::string describeSceneSkyProblem(SceneSkyProblem problem)
    {
        switch (problem)
        {
            case SceneSkyProblem::None:
                return {};

            // Deliberately empty too. "This scene has no sky" is a sentence that would appear on
            // every 2D project and most 3D ones, and a message every scene shows is one nobody
            // reads. The caller that wants to say something about this state can; nothing here
            // insists on it.
            case SceneSkyProblem::NoEnvironmentMap:
                return {};

            case SceneSkyProblem::EnvironmentMapMissing:
                return "This scene's environment map is not in the project. It may have been "
                       "deleted or moved outside the asset folder.";

            case SceneSkyProblem::EnvironmentMapUnreadable:
                return "This scene's environment map cannot be read: it is not valid JSON, or it "
                       "was written by a newer Studio.";

            case SceneSkyProblem::NoPanorama:
                return "This scene's environment map has no panorama. Choose one in the Inspector "
                       "and the sky will appear.";

            case SceneSkyProblem::PanoramaMissing:
                return "The panorama this environment map names is not in the project, or is not a "
                       "texture.";
        }
        return {};
    }

    SceneSkyPlan planSceneSky(const SceneEnvironment& environment,
                              const SceneSkySourceProvider& provider)
    {
        SceneSkyPlan plan;

        // Carried whatever happens below, so a panel can show what the scene *asks for* even while
        // it is saying the chain is broken. An intensity that read as its default because the
        // panorama was missing would look like a setting that had been reset.
        plan.environmentMap = environment.environmentMap;
        plan.intensity = std::max(0.0f, environment.environmentIntensity);
        plan.yaw = environment.environmentYaw * kDegreesToRadians;

        if (!environment.environmentMap.isValid())
        {
            plan.problem = SceneSkyProblem::NoEnvironmentMap;
            return plan;
        }

        // A provider that is not callable is a caller with no database -- a headless test, or a
        // panel built without services. Answering "found nothing" is what that honestly is, and it
        // keeps every caller from having to guard the call site.
        const SceneSkySource source = provider ? provider(environment.environmentMap)
                                               : SceneSkySource{};

        if (!source.found)
        {
            plan.problem = SceneSkyProblem::EnvironmentMapMissing;
            return plan;
        }
        if (!source.readable)
        {
            plan.problem = SceneSkyProblem::EnvironmentMapUnreadable;
            return plan;
        }
        if (!source.panorama.isValid())
        {
            plan.problem = SceneSkyProblem::NoPanorama;
            return plan;
        }
        if (!source.panoramaIsTexture)
        {
            plan.problem = SceneSkyProblem::PanoramaMissing;
            return plan;
        }

        plan.panorama = source.panorama;
        plan.problem = SceneSkyProblem::None;

        // Read last, so that a scene with both switches off still reports a *resolved* chain. The
        // two are what the user asked for; the problem is what the editor cannot do. Reversing
        // that would warn somebody about a state they chose deliberately.
        plan.draws = environment.showSky;
        plan.lights = environment.lightFromEnvironment;
        return plan;
    }
}
