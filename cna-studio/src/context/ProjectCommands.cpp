// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/ProjectCommands.hpp"

#include "CNA/Studio/Scene/BuiltinComponents.hpp"

namespace CNA::Studio
{
    SetProjectLayersCommand::SetProjectLayersCommand(Project& project,
                                                     ComponentRegistry& registry,
                                                     std::vector<std::string> layers)
        : project_(&project),
          registry_(&registry),
          newLayers_(std::move(layers)),
          oldLayers_(project.getLayers())
    {
        // An empty list would leave nothing for an entity to be on. An unchanged list would put an
        // entry in the undo history that undoes to the state it is already in, which reads to the
        // user as a broken Ctrl+Z.
        valid_ = !newLayers_.empty() && newLayers_ != oldLayers_;
    }

    void SetProjectLayersCommand::execute()
    {
        if (valid_) { apply(newLayers_); }
    }

    void SetProjectLayersCommand::undo()
    {
        if (valid_) { apply(oldLayers_); }
    }

    void SetProjectLayersCommand::apply(const std::vector<std::string>& layers)
    {
        project_->setLayers(layers);
        applyProjectLayers(*registry_, layers);

        // Written through, like an importer setting. A project change that lived only in memory
        // would be lost by a crash the recovery snapshot cannot help with -- that snapshot holds
        // the scene, not the project. A failed write is left to the caller to notice: refusing the
        // edit over it would leave the registry and the project disagreeing about what exists.
        savedToDisk_ = project_->saveToFile();
    }

    std::string SetProjectLayersCommand::getDescription() const
    {
        if (newLayers_.size() > oldLayers_.size()) { return "Add layer"; }
        if (newLayers_.size() < oldLayers_.size()) { return "Remove layer"; }
        return "Rename layer";
    }

    SetTargetProfilesCommand::SetTargetProfilesCommand(Project& project,
                                                       std::vector<StudioTargetProfile> profiles,
                                                       std::size_t activeIndex,
                                                       std::string description)
        : project_(&project),
          newProfiles_(std::move(profiles)),
          oldProfiles_(project.getTargetProfiles()),
          newActive_(activeIndex),
          oldActive_(project.getActiveTargetProfileIndex()),
          description_(std::move(description))
    {
        // Clamped rather than refused. An index past the end is what "remove the last target"
        // produces on the way, and the caller fixing it up would be the caller doing the same
        // arithmetic in four places.
        if (!newProfiles_.empty() && newActive_ >= newProfiles_.size())
        {
            newActive_ = newProfiles_.size() - 1;
        }

        // An empty list would leave a project that cannot be built and no row to add a target
        // from, which is `SetProjectLayersCommand`'s reasoning about an empty layer list. An
        // unchanged list *and* an unchanged selection would put an entry in the history that
        // undoes to the state it is already in, which reads to the user as a broken Ctrl+Z.
        valid_ = !newProfiles_.empty()
              && (newProfiles_ != oldProfiles_ || newActive_ != oldActive_);
    }

    void SetTargetProfilesCommand::execute()
    {
        if (valid_) { apply(newProfiles_, newActive_); }
    }

    void SetTargetProfilesCommand::undo()
    {
        if (valid_) { apply(oldProfiles_, oldActive_); }
    }

    void SetTargetProfilesCommand::apply(const std::vector<StudioTargetProfile>& profiles,
                                         std::size_t activeIndex)
    {
        // The list first, then the index: `setActiveTargetProfileIndex` validates against the list
        // it is given, so an index into the *new* list applied against the old one would be
        // refused whenever the list grew.
        project_->setTargetProfiles(profiles);
        project_->setActiveTargetProfileIndex(activeIndex);

        // Written through, like the two commands above and for the reason they give: the recovery
        // snapshot holds the scene, not the project, so a project change that lived only in memory
        // is a project change a crash loses entirely.
        savedToDisk_ = project_->saveToFile();
    }

    SetProjectGridSnapCommand::SetProjectGridSnapCommand(Project& project, float step)
        : project_(&project), newStep_(step), oldStep_(project.getGridSnap())
    {
        // An unchanged value would put an entry in the history that undoes to the state it is
        // already in, which reads to the user as a broken Ctrl+Z.
        valid_ = step >= 0.0f && step != oldStep_;
    }

    void SetProjectGridSnapCommand::execute()
    {
        if (valid_) { apply(newStep_); }
    }

    void SetProjectGridSnapCommand::undo()
    {
        if (valid_) { apply(oldStep_); }
    }

    void SetProjectGridSnapCommand::apply(float step)
    {
        project_->setGridSnap(step);

        // Written through, like the layer list beside it and for the same reason: the recovery
        // snapshot holds the scene, not the project, so a project change that lived only in
        // memory is a project change a crash loses entirely.
        savedToDisk_ = project_->saveToFile();
    }

    std::string SetProjectGridSnapCommand::getDescription() const
    {
        return newStep_ > 0.0f ? "Set grid snap" : "Use the visible grid for snapping";
    }
}
