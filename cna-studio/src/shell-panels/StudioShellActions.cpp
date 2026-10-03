// SPDX-License-Identifier: MS-PL
/**
 * @file StudioShellActions.cpp
 * @brief Binds the shell's actions to the editor.
 */

#include "CNA/Studio/ShellPanels/StudioShellActions.hpp"

#include "CNA/Studio/Core/StudioCommand.hpp"
#include "CNA/Studio/ShellPanels/StudioContentBrowser.hpp"
#include "CNA/Studio/Scene/SceneCommands.hpp"
#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/EntityArchetypes.hpp"
#include "CNA/Studio/Scene/TransformGizmos3D.hpp"
#include "CNA/Studio/Scene/SceneTransform.hpp"
#include "CNA/Studio/Scene/TransformGizmos.hpp"
#include "CNA/Studio/Project/Project.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/Ui/StudioLog.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"

#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <utility>

namespace CNA::Studio
{
    int bindStudioShellActions(StudioShell& shell, StudioContext& context, StudioLog& log)
    {
        StudioActionRegistry& actions = shell.actions();
        int bound = 0;

        // Copied, amended and put back rather than mutated in place: the registry hands out const
        // references so nothing can quietly rewrite a command another part of the shell is
        // pointing at. An id the registry does not carry is skipped -- the menus are built from
        // the registry, so an action invented here would be one no menu shows.
        const auto bind = [&](const char* id, std::function<bool()> enabled,
                              std::function<void()> run) {
            const StudioAction* existing = actions.find(id);
            if (existing == nullptr) { return; }

            StudioAction amended = *existing;
            amended.isEnabled = std::move(enabled);
            amended.run = std::move(run);
            if (actions.add(std::move(amended))) { ++bound; }
        };

        bind("studio.edit.undo",
             [&context] { return context.getHistory().canUndo(); },
             [&context, &log] {
                 // The description is read before the undo, because afterwards it names whatever
                 // is now on top of the stack -- a different entry, and a message that would tell
                 // the user the wrong thing about what just happened.
                 const std::string what = context.getHistory().getUndoDescription();
                 if (context.getHistory().undo())
                 {
                     log.append(LogSeverity::Info, "Undid " + what + ".");

                     // A reversal is a document change like any other: a running game that saw the
                     // edit but not its undo would be showing a state that exists nowhere any more.
                     if (const StudioCommand* entry =
                             context.getHistory().getCommandAt(context.getHistory().getCursor()))
                     {
                         context.announceCommand(*entry);
                     }
                 }
             });

        bind("studio.edit.redo",
             [&context] { return context.getHistory().canRedo(); },
             [&context, &log] {
                 const std::string what = context.getHistory().getRedoDescription();
                 const std::size_t beforeRedo = context.getHistory().getCursor();
                 if (context.getHistory().redo())
                 {
                     log.append(LogSeverity::Info, "Redid " + what + ".");
                     if (const StudioCommand* entry = context.getHistory().getCommandAt(beforeRedo))
                     {
                         context.announceCommand(*entry);
                     }
                 }
             });

        bind("studio.file.save",
             [&context] { return context.hasProject(); },
             [&context, &log] {
                 if (context.saveScene())
                 {
                     log.append(LogSeverity::Info, "Saved " + context.getScene().getName() + ".");
                 }
                 else
                 {
                     log.append(LogSeverity::Error, "Could not save the scene.");
                 }
             });

        // New Scene, which the prototype has on Ctrl+N and the native shell did not have at all
        // (docs/MIGRATION-INVENTORY.md). Refused with unsaved changes rather than discarding them:
        // a key people press all day must not be able to throw work away without asking.
        bind("studio.file.newScene",
             [&context] { return context.hasProject(); },
             [&context, &log] {
                 if (context.getHistory().isDirty())
                 {
                     log.append(LogSeverity::Warning,
                                "Save the scene before starting a new one, or undo your changes.");
                     return;
                 }
                 context.newScene();
                 log.append(LogSeverity::Info, "Started a new scene.");
             });

        bind("studio.edit.delete",
             [&context] {
                 return !context.getSelection().empty()
                     || context.getAssets().find(context.getSelectedAsset()) != nullptr;
             },
             [&context, &log] {
                 // One Delete, acting on whatever the inspector is showing (STUDIO-09009). The two
                 // selections are mutually exclusive by construction -- selecting an asset clears
                 // the entity selection and vice versa -- so there is never a question of which
                 // this means, and a second shortcut for assets would have been one the user has
                 // to know the difference between.
                 if (context.getSelection().empty())
                 {
                     const StudioContentOperation deleted =
                         studioContentDelete(context, context.getSelectedAsset());
                     if (!deleted.message.empty())
                     {
                         log.append(deleted.applied ? LogSeverity::Info : LogSeverity::Error,
                                    deleted.applied ? deleted.message + "."
                                                    : "Could not delete: " + deleted.message + ".");
                     }
                     return;
                 }

                 // **Every** selected entity, not the last one (STUDIO-07047). Deleting one of a
                 // selection of five and clearing the selection is the shape of bug a user reports
                 // as "Delete only sometimes works", because which one survived depended on the
                 // order they clicked.
                 //
                 // Roots only: a delete takes the whole subtree with it, so a selected descendant
                 // of a selected entity is already accounted for, and asking to delete it
                 // separately would push a command that finds nothing. The same helper the
                 // multi-selection gizmo uses.
                 const std::vector<Uuid> doomed =
                     findSelectionRoots(context.getScene(), context.getSelection());
                 if (doomed.empty()) { return; }

                 const StudioEntity* first = context.getScene().findEntity(doomed.front());
                 const std::string name = first != nullptr ? first->getName() : "entity";

                 // One entry for the whole action. Through the history, like every other edit:
                 // deleting is the single operation a user most needs to be able to take back.
                 auto batch = std::make_unique<CompositeCommand>(
                     "Delete " + std::to_string(doomed.size())
                     + (doomed.size() == 1 ? " entity" : " entities"));
                 for (const Uuid& entityId : doomed)
                 {
                     if (context.getScene().findEntity(entityId) == nullptr) { continue; }
                     batch->add(
                         std::make_unique<DeleteEntityCommand>(context.getScene(), entityId));
                 }
                 if (batch->isEmpty()) { return; }

                 context.execute(std::move(batch));
                 context.pruneSelection();
                 log.append(LogSeverity::Info,
                            doomed.size() == 1
                                ? "Deleted '" + name + "'."
                                : "Deleted " + std::to_string(doomed.size()) + " entities.");
             });

        // Creating an entity (`plan.md` STUDIO-13013). One binding per archetype rather than one
        // per kind hand-written: which components a kind gets is the archetype table's answer, and
        // a second answer here is how two ways of making the same thing come to differ.
        for (const StudioEntityArchetype& archetype : studioEntityArchetypes())
        {
            const std::string id = "studio.entity.create." + archetype.id;
            bind(id.c_str(),
                 // A scene is always open -- `newScene` builds one at startup -- so this is
                 // enabled whenever the shell is. There is deliberately no "needs a project"
                 // condition: a user trying out the editor before creating a project can still
                 // build a scene, and refusing them would be refusing the first thing they try.
                 [] { return true; },
                 // The archetype is *copied* into the handler rather than referenced. The table
                 // it comes from has static storage duration, so a reference would in fact
                 // outlive the loop -- but a binding whose correctness rests on that is one that
                 // breaks silently the day the table stops being static, and a copy of six small
                 // structs buys the question away.
                 [&context, &log, archetype] {
                     auto command = std::make_unique<CreateEntityCommand>(
                         context.getScene(),
                         studioMakeArchetypeEntity(archetype, context.getComponentRegistry()));

                     // Read before the move, and used after: the id is assigned at construction
                     // precisely so the caller can select what it just made.
                     const Uuid created = command->getEntityId();
                     const std::string name = archetype.name;
                     context.execute(std::move(command));

                     // Selected, like Duplicate and like the asset drop, because the next thing a
                     // user does is almost always move or rename what they just added.
                     context.select(created);
                     log.append(LogSeverity::Info,
                                "Created '" + name + "' at the origin.  Undo with Ctrl+Z.");
                 });
        }

        // Parenting from the viewport (`plan.md` STUDIO-12010). The *last* selected entity is the
        // parent, which is the rule every editor with this command uses: a user builds the group by
        // clicking the pieces and finishes on the thing they all belong to.
        bind("studio.entity.attach",
             [&context] { return context.getSelection().size() > 1; },
             [&context, &log] {
                 const std::vector<Uuid>& selection = context.getSelection();
                 if (selection.size() < 2) { return; }

                 const Uuid parent = selection.back();
                 const StudioEntity* parentEntity = context.getScene().findEntity(parent);
                 if (parentEntity == nullptr) { return; }

                 auto batch = std::make_unique<CompositeCommand>("Attach to '"
                                                                 + parentEntity->getName() + "'");
                 std::size_t refused = 0;

                 for (std::size_t index = 0; index + 1 < selection.size(); ++index)
                 {
                     const Uuid& child = selection[index];
                     if (context.getScene().findEntity(child) == nullptr) { continue; }

                     // Already there is not an edit, and an ancestor of the target cannot become
                     // its child -- the document refuses the cycle and would leave an undo entry
                     // that undoes nothing, which is the history that makes a user stop trusting
                     // Ctrl+Z.
                     if (context.getScene().findEntity(child)->getParentId() == parent) { continue; }
                     if (context.getScene().isAncestorOf(child, parent))
                     {
                         ++refused;
                         continue;
                     }

                     batch->add(std::make_unique<ReparentEntityCommand>(context.getScene(), child,
                                                                        parent));
                 }

                 if (refused > 0)
                 {
                     log.append(LogSeverity::Warning,
                                "Skipped " + std::to_string(refused)
                                    + (refused == 1 ? " entity that is" : " entities that are")
                                    + " above '" + parentEntity->getName()
                                    + "': an entity cannot be its own descendant.");
                 }

                 if (batch->isEmpty()) { return; }

                 const std::size_t moved = batch->getCount();
                 context.execute(std::move(batch));
                 log.append(LogSeverity::Info,
                            "Attached " + std::to_string(moved)
                                + (moved == 1 ? " entity to '" : " entities to '")
                                + parentEntity->getName() + "'.");
             });

        bind("studio.entity.detach",
             [&context] {
                 const std::vector<Uuid>& selection = context.getSelection();
                 return std::any_of(selection.begin(), selection.end(), [&context](const Uuid& id) {
                     const StudioEntity* entity = context.getScene().findEntity(id);
                     return entity != nullptr && entity->getParentId().isValid();
                 });
             },
             [&context, &log] {
                 auto batch = std::make_unique<CompositeCommand>("Detach");

                 for (const Uuid& id : context.getSelection())
                 {
                     const StudioEntity* entity = context.getScene().findEntity(id);
                     if (entity == nullptr || !entity->getParentId().isValid()) { continue; }

                     // The nil parent makes it a root, and the command keeps it where it is: a
                     // detach that moved the object would be one a user has to undo and redo by
                     // hand every time they reorganise a hierarchy.
                     batch->add(
                         std::make_unique<ReparentEntityCommand>(context.getScene(), id, Uuid{}));
                 }

                 if (batch->isEmpty()) { return; }

                 const std::size_t moved = batch->getCount();
                 context.execute(std::move(batch));
                 log.append(LogSeverity::Info,
                            "Detached " + std::to_string(moved)
                                + (moved == 1 ? " entity." : " entities."));
             });

        // Grouping (`plan.md` STUDIO-13008). Distinct from Attach above: Attach puts the selection
        // under one of *itself*, which needs something to be the parent; Group makes the parent,
        // which is what a user wants when the parent does not exist yet.
        //
        // A group is an ordinary entity with a transform and nothing else. No new document concept:
        // a folder that had to be stripped on export would be a Studio-only idea living in a CNA
        // file, and Studio produces CNA games rather than CNA Studio games.
        bind("studio.entity.group",
             [&context] { return !context.getSelection().empty(); },
             [&context, &log] {
                 // Roots only. A `ReparentEntityCommand` on a descendant of something already
                 // moving would pull it out of the thing it is travelling with, and the parent
                 // takes its subtree with it anyway.
                 const std::vector<Uuid> roots =
                     findSelectionRoots(context.getScene(), context.getSelection());
                 if (roots.empty()) { return; }

                 // The group goes where the selection is, so the pivot a user then drags is in the
                 // middle of what they grouped rather than at the world origin -- which is where a
                 // group with no transform of its own would put it, usually off screen.
                 const std::optional<StudioVector3> pivot =
                     computeSelectionPivot3D(context.getScene(), roots);

                 // Under the common parent when the selection shares one, and at the scene root
                 // otherwise. Grouping three siblings should leave the group where they were; there
                 // is no sensible common answer for entities from different branches, and the root
                 // is the one place that is not a guess.
                 Uuid shared;
                 bool first = true;
                 for (const Uuid& id : roots)
                 {
                     const StudioEntity* entity = context.getScene().findEntity(id);
                     if (entity == nullptr) { continue; }
                     if (first) { shared = entity->getParentId(); first = false; }
                     else if (entity->getParentId() != shared) { shared = Uuid{}; break; }
                 }

                 StudioEntity group{Uuid::generate(), "Group"};
                 group.setParentId(shared);

                 StudioComponent transform{BuiltinComponentIds::kTransform};
                 if (const ComponentDescriptor* descriptor =
                         context.getComponentRegistry().find(BuiltinComponentIds::kTransform))
                 {
                     transform.applyDefaults(*descriptor);
                 }
                 if (pivot)
                 {
                     // The pivot is a *world* point and the transform is relative to the parent, so
                     // it is converted rather than assigned. Assigning it would put the group at
                     // the right place only while its parent sat at the origin.
                     WorldTransform world;
                     world.position = *pivot;

                     WorldTransform parentWorld;
                     if (shared.isValid())
                     {
                         if (const std::optional<WorldTransform> found =
                                 computeWorldTransform(context.getScene(), shared))
                         {
                             parentWorld = *found;
                         }
                     }
                     transform.setProperty(
                         "position",
                         PropertyValue{localTransformUnder(world, parentWorld).position});
                 }
                 group.addComponent(std::move(transform));

                 const Uuid groupId = group.getId();

                 // One entry for the whole action: the user pressed Ctrl+G once. The create comes
                 // first so the reparents have somewhere to go -- each reads its entity's world
                 // transform when it is *built* and the new parent's when it *runs*, so the
                 // children stay exactly where they are.
                 auto batch = std::make_unique<CompositeCommand>(
                     "Group " + std::to_string(roots.size())
                     + (roots.size() == 1 ? " entity" : " entities"));
                 batch->add(std::make_unique<CreateEntityCommand>(context.getScene(),
                                                                  std::move(group)));
                 for (const Uuid& id : roots)
                 {
                     batch->add(
                         std::make_unique<ReparentEntityCommand>(context.getScene(), id, groupId));
                 }

                 context.execute(std::move(batch));

                 // Selecting the group is what makes "group, then move it" work without a trip back
                 // to the World Outliner -- the same bargain Duplicate strikes.
                 context.setSelection({groupId});
                 log.append(LogSeverity::Info,
                            "Grouped " + std::to_string(roots.size())
                                + (roots.size() == 1 ? " entity." : " entities."));
             });

        bind("studio.entity.ungroup",
             [&context] {
                 const std::vector<Uuid>& selection = context.getSelection();
                 return std::any_of(selection.begin(), selection.end(), [&context](const Uuid& id) {
                     return !context.getScene().getChildren(id).empty();
                 });
             },
             [&context, &log] {
                 const std::vector<Uuid> selection = context.getSelection();

                 auto batch = std::make_unique<CompositeCommand>("Ungroup");
                 std::vector<Uuid> freed;
                 std::size_t groups = 0;

                 for (const Uuid& id : selection)
                 {
                     const StudioEntity* group = context.getScene().findEntity(id);
                     if (group == nullptr) { continue; }

                     const std::vector<Uuid> children = context.getScene().getChildren(id);
                     if (children.empty()) { continue; }

                     ++groups;
                     const Uuid destination = group->getParentId();
                     for (const Uuid& child : children)
                     {
                         freed.push_back(child);
                         batch->add(std::make_unique<ReparentEntityCommand>(context.getScene(),
                                                                            child, destination));
                     }

                     // The group goes after its children have left, so the delete takes an empty
                     // entity rather than the subtree -- a `DeleteEntityCommand` built before them
                     // would have captured the children too and put them back on undo, twice.
                     batch->add(std::make_unique<DeleteEntityCommand>(context.getScene(), id));
                 }

                 if (batch->isEmpty()) { return; }

                 context.execute(std::move(batch));

                 // What came out, rather than the groups that are now gone: a selection naming
                 // entities the scene no longer has is one the inspector cannot show.
                 context.setSelection(std::move(freed));
                 log.append(LogSeverity::Info,
                            "Ungrouped " + std::to_string(groups)
                                + (groups == 1 ? " group." : " groups."));
             });

        bind("studio.file.saveAll",
             [&context] { return context.hasProject(); },
             [&context, &log] {
                 // The scene *and* the project. "Save All" that saved one of the two would be the
                 // command a user reaches for precisely when they cannot afford it to be partial.
                 const bool scene = context.saveScene();
                 std::string problem;
                 const bool project = context.getProject().saveToFile({}, &problem);

                 if (scene && project)
                 {
                     log.append(LogSeverity::Info, "Saved the scene and the project.");
                     return;
                 }
                 log.append(LogSeverity::Error,
                            std::string{"Save All did not complete: "}
                                + (scene ? "" : "the scene would not save. ")
                                + (project ? "" : "the project would not save. " + problem));
             });

        bind("studio.edit.duplicate",
             [&context] {
                 return !context.getSelection().empty()
                     || context.getAssets().find(context.getSelectedAsset()) != nullptr;
             },
             [&context, &log] {
                 // The asset the inspector is showing, when that is what is selected -- the same
                 // bargain Delete strikes above, and for the same reason.
                 if (context.getSelection().empty())
                 {
                     const StudioContentOperation copied =
                         studioContentDuplicate(context, context.getSelectedAsset());
                     if (!copied.message.empty())
                     {
                         log.append(copied.applied ? LogSeverity::Info : LogSeverity::Error,
                                    copied.applied ? copied.message + "."
                                                   : "Could not duplicate: " + copied.message + ".");
                     }
                     return;
                 }

                 // Roots only, and the same helper Delete uses (`plan.md` STUDIO-13006). A
                 // `DuplicateEntityCommand` copies an entity *and its whole subtree*, so
                 // duplicating a rig and one of its selected bones produced a copy of the rig with
                 // a copy of the bone inside it **and** a second loose bone standing beside it.
                 // Delete has taken the roots only since `STUDIO-07047`, for exactly this reason;
                 // this iterated the raw selection.
                 //
                 // Snapshotted for a second reason too: the copies are selected as they are made,
                 // and iterating the live selection would duplicate the copies as well.
                 const std::vector<Uuid> sources =
                     findSelectionRoots(context.getScene(), context.getSelection());
                 if (sources.empty()) { return; }

                 // One entry for the whole action, for the reason Delete has one: duplicating five
                 // entities is one press of Ctrl+D, so undoing it is one press of Ctrl+Z.
                 auto batch = std::make_unique<CompositeCommand>(
                     "Duplicate " + std::to_string(sources.size())
                     + (sources.size() == 1 ? " entity" : " entities"));

                 std::vector<Uuid> copies;
                 for (const Uuid& sourceId : sources)
                 {
                     auto command =
                         std::make_unique<DuplicateEntityCommand>(context.getScene(), sourceId);
                     if (!command->isValid()) { continue; }
                     copies.push_back(command->getEntityId());
                     batch->add(std::move(command));
                 }
                 if (batch->isEmpty()) { return; }

                 // Asked *before* executing, because after it the description names a copy that did
                 // not exist when the question was asked.
                 const std::string what = batch->getDescription();
                 context.execute(std::move(batch));

                 // Selecting the copies is what makes "duplicate, then drag it somewhere" work
                 // without a trip back to the World Outliner (STUDIO-07047).
                 context.setSelection(std::move(copies));
                 log.append(LogSeverity::Info, what + ".");
             });

        return bound;
    }
}
