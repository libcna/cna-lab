// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/ProjectCommands.hpp
 * @brief Undoable changes to the open `.cnaproject`.
 *
 * The project is a document like the scene and the asset database, so a change to it goes through
 * a command (ANALYSIS.md decision D-06). An editor where some edits undo and others quietly do not
 * is worse than one where nothing does.
 *
 * These live above both `cna-studio-project` and `cna-studio-scene` rather than inside either:
 * changing the layer list has to update the component registry as well as the project, and neither
 * of those modules may depend on the other.
 */

#include <string>
#include <vector>

#include "CNA/Studio/Core/ComponentDescriptor.hpp"
#include "CNA/Studio/Core/StudioCommand.hpp"
#include "CNA/Studio/Project/Project.hpp"

namespace CNA::Studio
{
    /**
     * @brief Replaces the project's render layers.
     *
     * Also re-registers `CNA.Layer` so the inspector offers the new names, and writes the project
     * file -- the same shape as `SetImporterSettingCommand`, which persists a sidecar rather than
     * waiting for a save the user has no reason to expect.
     *
     * Entities already on a layer that has been renamed away are **not** rewritten. Which of the
     * remaining layers they belonged to is the user's decision, not the command's; scene validation
     * reports them instead (`unknown-enum-value`).
     */
    class SetProjectLayersCommand final : public StudioCommand
    {
    public:
        SetProjectLayersCommand(Project& project, ComponentRegistry& registry, std::vector<std::string> layers);

        /** @brief Returns false when the new list is empty or identical to the current one. */
        [[nodiscard]] bool isValid() const { return valid_; }

        void execute() override;
        void undo() override;
        [[nodiscard]] std::string getDescription() const override;

        /** @brief Returns whether the last apply() managed to write the project file. */
        [[nodiscard]] bool wasSavedToDisk() const { return savedToDisk_; }

    private:
        void apply(const std::vector<std::string>& layers);

        Project* project_;
        ComponentRegistry* registry_;
        std::vector<std::string> newLayers_;
        std::vector<std::string> oldLayers_;
        bool valid_ = false;
        bool savedToDisk_ = false;
    };

    /**
     * @brief Replaces the project's build targets and which of them is active (STUDIO-17002).
     *
     * **The list and the active index are one command, not two.** Adding a target selects it,
     * removing one moves the selection, and a user who undid an "Add target" and found the
     * selection still pointing past the end of the list would be looking at a bug. The two facts
     * change together, so they undo together.
     *
     * Written through on every apply, like the two commands above and for the reason they give:
     * the crash-recovery snapshot holds the *scene*, not the project, so a project change that
     * lived only in memory is one a crash loses entirely — and before this command existed the
     * Build panel wrote target profiles straight into the open `Project` and nothing ever saved
     * them. The panel's own header promised "the profile the next save writes"; there was no next
     * save, because nothing marked the project as having changed.
     *
     * The description is the caller's, because the caller knows which gesture this was. "Set
     * renderer" and "Add target" are the same operation on the list and two different things in
     * the History panel, and a command that named itself could only say the first.
     */
    class SetTargetProfilesCommand final : public StudioCommand
    {
    public:
        /**
         * @param project The open project.
         * @param profiles The list the edit produced. Empty is refused: a project that declares no
         *                 build target cannot be built, and there would be no row to add one from.
         * @param activeIndex Which profile is active afterwards. Clamped into the list.
         * @param description What to call this in the History panel.
         */
        SetTargetProfilesCommand(Project& project, std::vector<StudioTargetProfile> profiles,
                                 std::size_t activeIndex, std::string description);

        /** @brief Returns false when the list is empty or nothing about it actually changed. */
        [[nodiscard]] bool isValid() const { return valid_; }

        void execute() override;
        void undo() override;
        [[nodiscard]] std::string getDescription() const override { return description_; }

        /** @brief Returns whether the last apply() managed to write the project file. */
        [[nodiscard]] bool wasSavedToDisk() const { return savedToDisk_; }

    private:
        void apply(const std::vector<StudioTargetProfile>& profiles, std::size_t activeIndex);

        Project* project_;
        std::vector<StudioTargetProfile> newProfiles_;
        std::vector<StudioTargetProfile> oldProfiles_;
        std::size_t newActive_ = 0;
        std::size_t oldActive_ = 0;
        std::string description_;
        bool valid_ = false;
        bool savedToDisk_ = false;
    };

    /**
     * @brief Sets the project's snap step, undoably.
     *
     * Beside SetProjectLayersCommand rather than in the project module, for the same reason that
     * one is here: a project edit has to reach the undo stack like every other document change
     * (D-06), and the project module cannot depend on the command history.
     *
     * Simpler than its neighbour in one respect -- a step needs no registry, since nothing but the
     * viewport reads it, and no descriptor has to be re-registered when it changes.
     */
    class SetProjectGridSnapCommand final : public StudioCommand
    {
    public:
        SetProjectGridSnapCommand(Project& project, float step);

        /** @brief Returns false when the step is negative or already the current one. */
        [[nodiscard]] bool isValid() const { return valid_; }

        void execute() override;
        void undo() override;
        [[nodiscard]] std::string getDescription() const override;

        /** @brief Returns whether the last apply() managed to write the project file. */
        [[nodiscard]] bool wasSavedToDisk() const { return savedToDisk_; }

    private:
        void apply(float step);

        Project* project_;
        float newStep_ = 0.0f;
        float oldStep_ = 0.0f;
        bool valid_ = false;
        bool savedToDisk_ = false;
    };
}