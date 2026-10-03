// SPDX-License-Identifier: MS-PL
/**
 * @file StudioStartupDocument.cpp
 * @brief Opening the project and the scene a Studio was started with.
 */

#include "CNA/Studio/StudioStartupDocument.hpp"

#include "CNA/Studio/StudioContext.hpp"

namespace CNA::Studio
{
    StudioStartupDocument openStudioStartupDocument(StudioContext& context,
                                                    const std::string& projectPath,
                                                    const std::string& scenePath)
    {
        StudioStartupDocument result;

        std::string problem;

        if (projectPath.empty())
        {
            // A scene with a camera in it, not an empty document. A scene with no camera renders
            // nothing, and nothing is what a broken editor also renders.
            context.newScene("Untitled");
        }
        else if (context.openProject(projectPath, &problem))
        {
            result.projectOpened = true;
        }
        else
        {
            // The reason, not the fact (`plan.md` STUDIO-31008). This sentence is for the places a
            // log sink does not reach -- standard error, and the status bar -- which is exactly
            // where saying only "could not open" leaves a user with nothing to act on. The context
            // has worked out which line of the file is wrong; carrying it here costs one string.
            result.error = "Could not open '" + projectPath + "'."
                         + (problem.empty() ? std::string{} : " " + problem);
            return result;
        }

        if (scenePath.empty()) { return result; }

        if (context.openScene(scenePath, &problem)) { result.sceneOpened = true; }
        else
        {
            result.error = "Could not open scene '" + scenePath + "'."
                         + (problem.empty() ? std::string{} : " " + problem);
        }

        return result;
    }
}
