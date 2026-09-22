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

    /**
     * @brief A place in the source a user asked to be taken to.
     *
     * `plan.md` CORE-01. Reported rather than opened, for the reason every other gesture in this
     * panel is: launching a process is the binder's business, and a panel that spawned an IDE
     * would be a panel that cannot be drawn in a test.
     */
    struct StudioSourceLocation
    {
        /** @brief The file, as the compiler named it. Empty means nothing was asked for. */
        std::string file;

        /** @brief 1-based line, or zero. */
        int line = 0;

        /** @brief Whether there is anything to open. */
        [[nodiscard]] bool isValid() const { return !file.empty(); }
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

        /**
         * @brief Which build they asked for, when @ref buildRequested.
         *
         * Two buttons rather than a button and a checkbox: they are pressed at different moments
         * and a modifier on the frequent one is a modifier people forget is set.
         */
        StudioBuildKind buildKind = StudioBuildKind::Incremental;

        /** @brief The user asked to stop the running build. Input pass only. */
        bool cancelRequested = false;

        /**
         * @brief A compiler error the user activated, for the external editor. Input pass only.
         *
         * `CORE-01`, resolved through `CORE-02`. A row that names a file and a line and does not
         * go there is a prettier way of reading a build log.
         */
        StudioSourceLocation openLocation;

        /** @brief The user asked to open the whole build log. Input pass only. */
        bool openLogRequested = false;

        /** @brief How many diagnostics the log's tail yielded, for a test and for the header. */
        std::size_t errorsShown = 0;

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
         * @param kind Incremental or clean.
         * @return The planned job, or one with no steps when there is no adapter or nothing to run.
         */
        [[nodiscard]] StudioBuildJob planBuild(
            StudioBuildKind kind = StudioBuildKind::Incremental) const;

        /** @brief How many lines of the build log the panel shows. */
        static constexpr std::size_t kLogTailLines = 12;

        /**
         * @brief How much of the log is read looking for diagnostics.
         *
         * More than the tail, because the first error is what a developer wants and a failing
         * build puts hundreds of lines after it -- CMake's summary, the linker's complaint, the
         * generator's "build stopped". Bounded, because a log can be megabytes and this runs on a
         * frame: the panel reads the end of the file, which is where a build that stopped left its
         * reason.
         */
        static constexpr std::size_t kDiagnosticScanLines = 2000;

        /** @brief How many diagnostic rows the panel lists at once. */
        static constexpr std::size_t kDiagnosticRows = 8;

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
