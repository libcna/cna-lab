// SPDX-License-Identifier: MS-PL
/**
 * @file CNA/Studio/ShellPanels/StudioBuildPanel.hpp
 * @brief The Build panel on the Studio UI.
 *
 * `plan.md` STUDIO-07010.
 *
 * ### It is not a straight port
 *
 * The ImGui Build panel offers two axes — a platform triple and a graphics backend — because that
 * is all the prototype's model had. Studio's model is the six-axis `StudioTargetProfile`
 * (`STUDIO-02040`): operating system, architecture, CNA platform, CNA renderer, configuration and
 * the optional subsystems, with validation that knows which combinations exist. That model has had
 * no user interface at all since it landed, and a project could only change its targets by editing
 * the `.cnaproject` by hand. This is that interface.
 *
 * ### A profile is a project's, not a panel's
 *
 * The legacy panel kept the chosen platform and backend in its own members, so they were forgotten
 * when the panel was closed and were never saved. Here the profile list *is* the project's, so
 * what the Build button does and what the project ships are one thing rather than two that agree
 * until they do not.
 *
 * **And the panel does not write it** (`STUDIO-17002`). The first version of this file edited the
 * project in place and raised a `profileChanged` flag that nothing read — so a target edit reached
 * the model, skipped the undo stack, never reached the file, and was gone when Studio closed. This
 * header promised "the profile the next save writes"; there was no next save, because nothing
 * marked the project as having changed. Now the panel *reports* a @ref StudioTargetProfileEdit and
 * the binder runs a `SetTargetProfilesCommand`, which is where the undo entry and the write-through
 * live.
 */

#pragma once

#include "CNA/Studio/Project/BuildRunner.hpp"
#include "CNA/Studio/Project/LanguageAdapter.hpp"
#include "CNA/Studio/Project/TargetProfile.hpp"
#include "CNA/Studio/UiCore/StudioFrame.hpp"
#include "CNA/Studio/UiCore/UiRect.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace CNA::Studio
{
    class StudioContext;

    /**
     * @brief A change to the project's build targets that the user asked for (`STUDIO-17002`).
     *
     * **Reported, not applied.** The panel used to write straight into the open `Project` and set
     * a `profileChanged` flag that nothing read, so every target edit lived in memory, never
     * reached the undo stack, and was lost when Studio closed. Panels report and the binder acts,
     * and here the binder's act is a `SetTargetProfilesCommand`.
     *
     * The whole list travels rather than a diff, because adding and removing a target move the
     * selection as well as the list, and the two have to change together or an undo leaves the
     * selection pointing at a target that is not there.
     */
    struct StudioTargetProfileEdit
    {
        /** @brief The list the edit produced. */
        std::vector<StudioTargetProfile> profiles;

        /** @brief Which of them is active afterwards. */
        std::size_t activeIndex = 0;

        /** @brief What to call this in the History panel, e.g. `"Add target"`. */
        std::string description;
    };

    /** @brief What the Build panel did this frame. */
    struct StudioBuildPanelResult
    {
        /**
         * @brief The target-profile change the user asked for, if any. Input pass only.
         *
         * Empty on a frame where nothing was edited, which is almost every frame.
         */
        std::optional<StudioTargetProfileEdit> profileEdit;

        /** @brief The user asked for a build. Input pass only. */
        bool buildRequested = false;

        /** @brief The user asked to stop the running build. Input pass only. */
        bool cancelRequested = false;

        /** @brief How tall the content is, so the panel can be scrolled. */
        float contentHeight = 0.0f;
    };

    /**
     * @brief The Build panel: what this project ships, and the button that builds it.
     *
     * Holds no copy of the profile. Everything it shows is read from the project when it is drawn,
     * and everything it changes is written back to the project, so a profile edited here is the
     * profile the Play button uses and the profile the next save writes.
     */
    class StudioBuildPanel
    {
    public:
        /**
         * @brief Constructs the panel.
         * @param context The editor context, for the project and the log.
         * @param build The build process this panel starts and reports on.
         */
        StudioBuildPanel(StudioContext& context, BuildProcess& build);

        /**
         * @brief Describes the panel into a frame.
         * @param frame The frame.
         * @param body Where the panel's content goes.
         * @return What the user asked for.
         */
        StudioBuildPanelResult draw(StudioFrame& frame, const UiRect& body);

        /**
         * @brief The build this panel would start, from the project's active profile.
         *
         * Planned by the project's language adapter, not here. The panel shows the commands and
         * presses the button; which commands those are is the language's answer, and a panel that
         * knew it would be a panel that had to be edited for every language Studio grows.
         *
         * @return The planned job, or one with no steps when there is no adapter or nothing to run.
         */
        [[nodiscard]] StudioBuildJob planBuild() const;

        /** @brief How many lines of the build log the panel shows. */
        static constexpr std::size_t kLogTailLines = 12;

    private:
        /**
         * @brief Draws one labelled row and returns its content rectangle.
         *
         * Every row in the panel is "a label on the left, a control on the right", and having one
         * place decide the split is what keeps the controls in a column rather than stepping in
         * and out as the labels change length.
         */
        [[nodiscard]] static UiRect labelledRow(StudioFrame& frame, UiRect& cursor,
                                                std::string_view label);

        StudioContext& context_;
        BuildProcess& build_;

        /**
         * @brief Resolved once: probing a toolchain walks the PATH, which is not a per-frame cost.
         *
         * Held rather than asked for each frame, and the field is the adapter's *report* rather
         * than a path, so the reason a toolchain is unusable survives to the row that shows it.
         */
        StudioToolchainReport toolchain_;
        bool toolchainProbed_ = false;
    };
}
