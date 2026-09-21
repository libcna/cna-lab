// SPDX-License-Identifier: MS-PL
/**
 * @file StudioShellActionTests.cpp
 * @brief One action object, reached three ways: menu, toolbar and keyboard.
 *
 * `plan.md` STUDIO-06001, STUDIO-06008.
 *
 * The registry exists so those three cannot drift apart, and that promise is only worth anything if
 * something checks it. A menu item that saves while Ctrl+S does nothing, or a toolbar button that
 * stays bright while the menu row greys out, are the two ways an editor loses a user's trust
 * quietly — neither crashes, and both are obvious the moment somebody tries the other route.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/ShellPanels/StudioShellActions.hpp"
#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/EntityArchetypes.hpp"
#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/ShellPanels/StudioShellPanels.hpp"
#include "CNA/Studio/Scene/SceneCommands.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/Ui/StudioLog.hpp"
#include "CNA/Studio/Scene/StudioCamera3D.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include "CNA/Studio/Scene/SceneTransform.hpp"
#include <cmath>
#include <string>
#include <utility>
#include <vector>

using namespace CNA::Studio;

namespace
{
    UiInputState at(float x, float y, bool leftDown = false)
    {
        UiInputState input;
        input.displayWidth = 1280.0f;
        input.displayHeight = 720.0f;
        input.mouseX = x;
        input.mouseY = y;
        input.mouseInWindow = true;
        input.setMouseDown(UiMouseButton::Left, leftDown);
        return input;
    }

    /** @brief A shell bound to a context holding one renamed entity, so Undo has work to do. */
    struct Fixture
    {
        StudioContext context;
        StudioLog log;
        std::unique_ptr<StudioShell> shell = std::make_unique<StudioShell>(StudioTheme::dark());
        Uuid entity;

        Fixture()
        {
            StudioEntity subject{Uuid::generate(), "Player"};
            entity = subject.getId();
            context.getScene().addEntity(std::move(subject));
            context.select(entity);

            shell->resetLayout();
            CNA_STUDIO_EXPECT(bindStudioShellActions(*shell, context, log) >= 4);
            shell->renderFrame(at(-1.0f, -1.0f));
        }

        [[nodiscard]] std::string name() const
        {
            const StudioEntity* found = context.getScene().findEntity(entity);
            return found != nullptr ? found->getName() : std::string{};
        }

        void rename(const std::string& to)
        {
            context.execute(std::make_unique<RenameEntityCommand>(context.getScene(), entity, to));
        }
    };
}

CNA_STUDIO_TEST(UndoIsDisabledUntilThereIsSomethingToUndo)
{
    // A control that looks available and refuses is indistinguishable from one that is broken.
    // Asking the predicate at the moment the answer is needed is what makes this exact, rather
    // than correct until somebody forgets to refresh a cached boolean.
    Fixture fixture;

    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.edit.undo"));
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.edit.redo"));

    fixture.rename("Hero");
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.edit.undo"));
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.edit.redo"));

    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT_EQ(fixture.name(), std::string{"Player"});
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.edit.undo"));
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.edit.redo"));

    fixture.shell->invoke("studio.edit.redo");
    CNA_STUDIO_EXPECT_EQ(fixture.name(), std::string{"Hero"});
}

CNA_STUDIO_TEST(TheKeyboardAndTheMenuReachTheSameAction)
{
    // Not "both work" -- the same object. A shortcut wired to its own copy of the handler is a
    // shortcut that keeps working after the menu's stops, which is how the two come to disagree.
    Fixture fixture;
    fixture.rename("Hero");

    UiInputState undo = at(600.0f, 400.0f);
    undo.setKeyDown(UiKey::Z, true);
    undo.modifiers.control = true;

    fixture.shell->renderFrame(undo);
    CNA_STUDIO_EXPECT_EQ(fixture.name(), std::string{"Player"});

    // And what ran is recorded under the action's id, whichever route invoked it.
    bool sawUndo = false;
    for (const std::string& invoked : fixture.shell->invokedActions())
    {
        if (invoked == "studio.edit.undo") { sawUndo = true; }
    }
    CNA_STUDIO_EXPECT(sawUndo);
}

CNA_STUDIO_TEST(DeletingAnEntityIsUndoable)
{
    // The single operation a user most needs to be able to take back.
    Fixture fixture;
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.edit.delete"));

    fixture.shell->invoke("studio.edit.delete");
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(fixture.entity) == nullptr);
    CNA_STUDIO_EXPECT(fixture.context.getSelection().empty());

    // With nothing selected, Delete greys out rather than staying bright and doing nothing.
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.edit.delete"));

    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(fixture.entity) != nullptr);
}

/**
 * Delete takes every selected entity and one undo entry (`plan.md` STUDIO-13006, `STUDIO-07047`).
 *
 * The single-entity case above has been covered since the action existed; the multi-selection rule
 * it was written for had not. Deleting one of a selection of five and clearing the selection is the
 * shape of bug a user reports as "Delete only sometimes works", because which one survived depended
 * on the order they clicked.
 */
CNA_STUDIO_TEST(DeleteTakesTheWholeSelectionAndTheSubtreesUnderIt)
{
    Fixture fixture;

    StudioEntity boneEntity{Uuid::generate(), "Bone"};
    boneEntity.setParentId(fixture.entity);
    const Uuid bone = boneEntity.getId();
    fixture.context.getScene().addEntity(std::move(boneEntity));

    StudioEntity otherEntity{Uuid::generate(), "Prop"};
    const Uuid other = otherEntity.getId();
    fixture.context.getScene().addEntity(std::move(otherEntity));

    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), std::size_t{3});

    // The rig, the bone inside it, and an unrelated root. The bone is already accounted for by its
    // parent, so asking to delete it separately would push a command that finds nothing.
    fixture.context.setSelection({fixture.entity, bone, other});
    fixture.shell->invoke("studio.edit.delete");

    CNA_STUDIO_EXPECT(fixture.context.getScene().getEntities().empty());
    CNA_STUDIO_EXPECT(fixture.context.getSelection().empty());

    // One press of Delete is one press of Ctrl+Z, and everything comes back -- including the bone,
    // which came out through its parent rather than on its own.
    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getChildren(fixture.entity).size(),
                         std::size_t{1});
}

/**
 * Duplicate takes the selection's roots, not every id in it (`plan.md` STUDIO-13006).
 *
 * `DuplicateEntityCommand` copies an entity *and its whole subtree*, so a selection holding both a
 * rig and one of its bones was duplicated as: a copy of the rig, with a copy of the bone inside it,
 * **and** a second loose copy of the bone beside the rig. Delete has taken the roots only since
 * `STUDIO-07047` and for exactly this reason; Duplicate iterated the raw selection.
 */
CNA_STUDIO_TEST(DuplicatingAParentAndItsChildCopiesTheParentOnce)
{
    Fixture fixture;

    StudioEntity boneEntity{Uuid::generate(), "Bone"};
    boneEntity.setParentId(fixture.entity);
    const Uuid bone = boneEntity.getId();
    fixture.context.getScene().addEntity(std::move(boneEntity));

    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), std::size_t{2});

    // Both selected, which is what a Ctrl-click or a shift-range through the Outliner produces.
    fixture.context.setSelection({fixture.entity, bone});
    fixture.shell->invoke("studio.edit.duplicate");

    // Two more entities, not three: the copy of the rig and the copy of the bone inside it. A
    // third would be the loose bone standing beside the rig it does not belong to.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), std::size_t{4});

    // And the copy that was made is a root with one child, not two roots.
    const std::vector<Uuid>& copies = fixture.context.getSelection();
    CNA_STUDIO_EXPECT_EQ(copies.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getChildren(copies.front()).size(),
                         std::size_t{1});

    // One entry for the whole action, as Delete has: the user pressed Ctrl+D once.
    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), std::size_t{2});
}

CNA_STUDIO_TEST(EveryActionTheShellInvokesEitherRunsOrIsRefusedOutLoud)
{
    // The failure this guards against is a menu with rows that quietly do nothing: an id a menu
    // names and the registry does not carry, or one carried with no handler. Both are invisible
    // from the outside and both are exactly what a half-finished migration produces.
    Fixture fixture;

    // Recursive, because a submenu is exactly where a dead row hides: it is one gesture further
    // from anybody who opens the menu to look.
    const auto check = [&](auto&& self, const std::string& where,
                           const std::vector<StudioMenuEntry>& entries) -> void {
        for (const StudioMenuEntry& entry : entries)
        {
            if (entry.isSeparator()) { continue; }
            if (entry.isSubmenu())
            {
                self(self, where + " \u203a " + entry.label, entry.rows);
                continue;
            }
            if (fixture.shell->actions().find(entry.id) == nullptr)
            {
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "the " + where + " menu names '" + entry.id
                    + "', which the action registry does not carry. The row would draw and do "
                      "nothing.");
            }
        }
    };

    for (const StudioMenuDefinition& menu : fixture.shell->menus())
    {
        check(check, menu.title, menu.entries);
    }

    // An unbound action refuses rather than pretending. Every menu row is therefore either
    // enabled and working, or greyed out -- never bright and inert.
    fixture.shell->invoke("studio.no.such.action");
    CNA_STUDIO_EXPECT(!fixture.shell->refusedActions().empty());
}

CNA_STUDIO_TEST(EachActionSaysWhatItDidRatherThanLeavingTheUserGuessing)
{
    // A user who pressed Ctrl+Z and saw nothing change needs to know whether nothing happened or
    // nothing was undoable -- and the description is read *before* the undo, because afterwards it
    // names whatever is now on top of the stack.
    Fixture fixture;
    fixture.rename("Hero");

    const std::size_t before = fixture.log.entries().size();
    fixture.shell->invoke("studio.edit.undo");

    CNA_STUDIO_EXPECT(fixture.log.entries().size() > before);
    CNA_STUDIO_EXPECT(fixture.log.toText().find("Undid") != std::string::npos);
}

CNA_STUDIO_TEST(EveryCommandThatIsStillUnimplementedIsNamedRatherThanDiscovered)
{
    // A command with no handler is drawn unavailable, which is right — but it means the *number*
    // of them is invisible from the UI, and a half-migrated menu can quietly stay half-migrated.
    // This is the list, and it has to be edited deliberately: binding one fails this test until
    // its name is removed, and adding an unbound command fails it until somebody writes down why.
    //
    // Each entry says what it is waiting for. None of them is waiting on nothing.
    // `studio.file.newProject` and `studio.file.openProject` came off this list with
    // STUDIO-08001: both are the Project Hub with a different tab in front. Neither is waiting on
    // a file picker any more -- New Project never needed one, and Open Project shows the recent
    // list, which is where a user's own projects are. A file dialog is a better Open Project and
    // is not the difference between a row that works and a row that is greyed out for ever.
    const std::vector<std::pair<std::string, std::string>> pending = {
        {"studio.view.toggleGrid", "a grid option on the viewport, which the renderer does not take"},
    };

    Fixture fixture;
    StudioCamera2D camera;
    StudioCamera3D camera3D;
    StudioShellPanels panels{*fixture.shell, fixture.context, fixture.log};
    panels.setViewportServices(camera, camera3D, {});

    std::vector<std::string> unimplemented;
    for (const StudioAction& action : fixture.shell->actions().commands())
    {
        // The per-panel show/hide commands are generated, not declared, and are always bound.
        if (action.id.rfind(std::string{kStudioPanelActionPrefix}, 0) == 0) { continue; }
        if (action.id.rfind(std::string{kStudioClosePanelActionPrefix}, 0) == 0) { continue; }
        if (!action.run) { unimplemented.push_back(action.id); }
    }

    for (const std::string& id : unimplemented)
    {
        const bool named = std::any_of(pending.begin(), pending.end(),
                                       [&](const auto& entry) { return entry.first == id; });
        if (!named)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "'" + id + "' has no handler and is not on the pending list. Either bind it, or "
                "add it with the reason it cannot be bound yet.");
        }
    }

    for (const auto& [id, reason] : pending)
    {
        const bool still = std::find(unimplemented.begin(), unimplemented.end(), id)
                        != unimplemented.end();
        if (!still)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "'" + id + "' is bound now, so remove it from the pending list (it was waiting on "
                + reason + ").");
        }
    }

    // And every one of them is drawn unavailable, which is what stops a user discovering the gap
    // by clicking something that appears to work.
    for (const auto& [id, reason] : pending)
    {
        (void)reason;
        CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled(id));
    }
}

CNA_STUDIO_TEST(EveryPanelWithoutContentIsNamedRatherThanBeingAnEmptyRectangle)
{
    // A panel with no content is a grey rectangle with a tab on it, and that is indistinguishable
    // from a panel whose content failed to draw. The same discipline as the unimplemented
    // commands: the list is here, with a reason, and it fails in both directions.
    //
    // **The list is empty**, and that is the state to keep it in. `material` was the last entry
    // and `STUDIO-07046` closed it: it had been registered since the shell existed and drawn
    // nothing, so raising that tab put a user in front of a blank rectangle. It shows the same
    // editor the Details panel does over the selected `.cnamaterial`, which is also what Phase 19
    // will grow a preview and texture slots into.
    const std::vector<std::pair<std::string, std::string>> pending = {};

    Fixture fixture;
    StudioCamera2D camera;
    StudioCamera3D camera3D;
    StudioShellPanels panels{*fixture.shell, fixture.context, fixture.log};
    panels.setViewportServices(camera, camera3D, {});

    std::vector<std::string> empty;
    for (const StudioPanelDescriptor& descriptor : fixture.shell->registeredPanels())
    {
        if (!fixture.shell->hasPanelContent(descriptor.id)) { empty.push_back(descriptor.id); }
    }

    for (const std::string& id : empty)
    {
        const bool named = std::any_of(pending.begin(), pending.end(),
                                       [&](const auto& entry) { return entry.first == id; });
        if (!named)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "the '" + id + "' panel draws nothing and is not on the pending list. Either give "
                "it content, or add it with the reason it has none yet.");
        }
    }

    for (const auto& [id, reason] : pending)
    {
        const bool still = std::find(empty.begin(), empty.end(), id) != empty.end();
        if (!still)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "the '" + id + "' panel has content now, so remove it from the pending list (it "
                "was waiting on " + reason + ").");
        }
    }
}

/**
 * @brief One Delete and one Ctrl+D, acting on whatever the inspector is showing (STUDIO-09009).
 *
 * The two selections are mutually exclusive by construction — selecting an asset clears the entity
 * selection and vice versa — so a single shortcut can mean both without ambiguity. A second pair of
 * shortcuts for assets would have been a pair the user has to know the difference between, and the
 * difference would depend on which panel they last clicked.
 */
CNA_STUDIO_TEST(DeleteAndDuplicateActOnTheSelectedAssetWhenThatIsWhatIsSelected)
{
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cna-studio-assetshortcuts";
    std::error_code code;
    std::filesystem::remove_all(directory, code);
    std::filesystem::create_directories(directory / "Assets", code);
    {
        std::ofstream stream{directory / "Assets" / "Crate.png", std::ios::binary};
        stream << "pixels";
    }

    Fixture fixture;
    fixture.context.getAssets().setProjectRoot(directory.generic_string());
    CNA_STUDIO_EXPECT(fixture.context.getAssets().scan("Assets").succeeded);

    const Uuid assetId = fixture.context.getAssets().findByPath("Assets/Crate.png")->id;

    // Selecting the asset clears the entity selection, so what follows is unambiguous.
    fixture.context.selectAsset(assetId);
    CNA_STUDIO_EXPECT(fixture.context.getSelection().empty());

    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.edit.duplicate"));
    fixture.shell->invoke("studio.edit.duplicate");

    const Uuid copyId = fixture.context.getSelectedAsset();
    CNA_STUDIO_EXPECT(copyId != assetId);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getAssets().find(copyId)->sourcePath,
                         std::string{"Assets/Crate 2.png"});

    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.edit.delete"));
    fixture.shell->invoke("studio.edit.delete");
    CNA_STUDIO_EXPECT(fixture.context.getAssets().find(copyId) == nullptr);

    // Undoable like every other edit, and back under the id a scene would still be referencing.
    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT(fixture.context.getAssets().find(copyId) != nullptr);

    // With neither an entity nor an asset selected, both grey out rather than staying bright and
    // doing nothing.
    fixture.context.selectAsset(Uuid{});
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.edit.delete"));
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.edit.duplicate"));

    std::filesystem::remove_all(directory, code);
}

/**
 * @brief The whole of STUDIO-09008 end to end: a dropped asset becomes an entity, undoably.
 *
 * Through `StudioShellPanels`, because that is where the decision lives — the viewport and the
 * hierarchy both report a drop and one place turns it into a command, so a `.gltf` is a
 * `ModelRenderer` wherever it lands.
 */
CNA_STUDIO_TEST(APlacedAssetBecomesAnEntityThatUsesItAndUndoesInOneStep)
{
    Fixture fixture;
    registerBuiltinComponents(fixture.context.getComponentRegistry());

    StudioLog log;
    StudioShellPanels panels{*fixture.shell, fixture.context, log};

    const Uuid model = Uuid::generate();
    {
        AssetRecord record;
        record.id = model;
        record.sourcePath = "Assets/Models/crate.gltf";
        record.type = AssetType::Model;
        CNA_STUDIO_EXPECT(fixture.context.getAssets().add(std::move(record)));
    }

    const std::size_t before = fixture.context.getHistory().getCount();
    const std::size_t entitiesBefore = fixture.context.getScene().getEntityCount();

    CNA_STUDIO_EXPECT(panels.placeDroppedAsset(model, StudioVector3{7.0f, 8.0f, 9.0f}, Uuid{}));

    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntityCount(), entitiesBefore + 1);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before + 1);

    // Selected, so the next thing the user does happens to what they just placed -- which is
    // almost always moving it.
    const Uuid placed = fixture.context.getPrimarySelection();
    CNA_STUDIO_EXPECT(placed.isValid());

    const StudioEntity* entity = fixture.context.getScene().findEntity(placed);
    CNA_STUDIO_EXPECT(entity != nullptr);
    if (entity != nullptr)
    {
        CNA_STUDIO_EXPECT_EQ(entity->getName(), std::string{"crate"});

        const StudioComponent* renderer =
            entity->findComponent(BuiltinComponentIds::kModelRenderer);
        CNA_STUDIO_EXPECT(renderer != nullptr);
        if (renderer != nullptr)
        {
            CNA_STUDIO_EXPECT_EQ(
                renderer->getProperty("model").get<PropertyValue::AssetReference>().id.toString(),
                model.toString());
        }

        const StudioComponent* transform =
            entity->findComponent(BuiltinComponentIds::kTransform);
        CNA_STUDIO_EXPECT(transform != nullptr);
        if (transform != nullptr)
        {
            CNA_STUDIO_EXPECT_EQ(transform->getProperty("position").get<StudioVector3>().x, 7.0f);
        }
    }

    // One entry for the whole placement.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntityCount(), entitiesBefore);

    // An asset with no place in a scene is refused rather than producing an empty entity.
    const Uuid effect = Uuid::generate();
    {
        AssetRecord record;
        record.id = effect;
        record.sourcePath = "Assets/blur.fx";
        record.type = AssetType::Effect;
        CNA_STUDIO_EXPECT(fixture.context.getAssets().add(std::move(record)));
    }
    CNA_STUDIO_EXPECT(!panels.placeDroppedAsset(effect, StudioVector3{}, Uuid{}));
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntityCount(), entitiesBefore);

    // And it said why, rather than being a gesture that did nothing.
    bool explained = false;
    for (const StudioLogEntry& entry : log.entries())
    {
        if (entry.message.find("Effect") != std::string::npos) { explained = true; }
    }
    CNA_STUDIO_EXPECT(explained);
}

/**
 * Attach makes the last-selected entity the parent, and Detach undoes the grouping (STUDIO-12010).
 *
 * The *last* selected is the parent, which is the rule every editor with this command uses: a user
 * builds the group by clicking the pieces and finishes on the thing they all belong to. Both
 * commands go through the history as one entry, because reorganising a hierarchy one entity at a
 * time on Ctrl+Z would put the scene through arrangements it was never actually in.
 */
CNA_STUDIO_TEST(AttachParentsToTheLastSelectedAndDetachTakesItBack)
{
    Fixture fixture;

    const auto add = [&](const std::string& name) {
        StudioEntity entity{Uuid::generate(), name};
        StudioComponent transform{BuiltinComponentIds::kTransform};
        transform.applyDefaults(*fixture.context.getComponentRegistry()
                                     .find(BuiltinComponentIds::kTransform));
        entity.addComponent(std::move(transform));
        const Uuid id = entity.getId();
        fixture.context.getScene().addEntity(std::move(entity));
        return id;
    };

    const Uuid crate = add("Crate");
    const Uuid barrel = add("Barrel");
    const Uuid rig = add("Rig");

    // One entity selected is not a grouping: there is nothing to attach it to.
    fixture.context.select(crate);
    fixture.shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.entity.attach"));

    // And nothing selected has a parent yet, so Detach has nothing to take back either.
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.entity.detach"));

    fixture.context.toggleSelection(barrel);
    fixture.context.toggleSelection(rig);
    fixture.shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.entity.attach"));

    fixture.shell->invoke("studio.entity.attach");

    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(crate)->getParentId() == rig);
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(barrel)->getParentId() == rig);
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(rig)->getParentId().isValid());

    // One entry for the whole grouping, and undoing it takes both children back at once.
    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(crate)->getParentId().isValid());
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(barrel)->getParentId().isValid());

    fixture.shell->invoke("studio.edit.redo");
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(crate)->getParentId() == rig);

    // Detach lights up now that something is parented, and puts them back as roots.
    fixture.context.select(crate);
    fixture.context.toggleSelection(barrel);
    fixture.shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.entity.detach"));

    fixture.shell->invoke("studio.entity.detach");
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(crate)->getParentId().isValid());
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(barrel)->getParentId().isValid());
}

/**
 * Attach refuses to make an entity its own descendant (`plan.md` STUDIO-12010).
 *
 * The document rejects the cycle anyway and leaves the scene untouched, so pushing the command
 * would be harmless -- and would put an undo entry on the stack that undoes nothing, which is the
 * kind of history that makes a user stop trusting Ctrl+Z. It is checked here for the same reason
 * the Outliner's drag checks it, and the user is told rather than left to wonder.
 */
/**
 * Group makes the parent; Attach needs one already (`plan.md` STUDIO-13008).
 *
 * The two are complementary and neither substitutes for the other: Attach puts the selection under
 * one of *itself*, which is what a user wants when the parent exists; Group makes it, which is what
 * they want when it does not. There was no way to do the second at all.
 *
 * A group is an ordinary entity carrying a transform and nothing else. No new document concept: a
 * folder that had to be stripped on export would be a Studio-only idea living in a CNA file, and
 * Studio produces CNA games rather than CNA Studio games.
 */
CNA_STUDIO_TEST(GroupWrapsTheSelectionInANewParentWithoutMovingAnything)
{
    Fixture fixture;

    // Two siblings at known places, so "the group went to their middle" is checkable.
    const auto place = [&](const Uuid& id, float x) {
        StudioComponent transform{BuiltinComponentIds::kTransform};
        transform.applyDefaults(
            *fixture.context.getComponentRegistry().find(BuiltinComponentIds::kTransform));
        transform.setProperty("position", PropertyValue{StudioVector3{x, 0.0f, 0.0f}});
        fixture.context.getScene().findEntityForEdit(id)->addComponent(std::move(transform));
    };
    place(fixture.entity, 40.0f);

    StudioEntity otherEntity{Uuid::generate(), "Prop"};
    const Uuid other = otherEntity.getId();
    fixture.context.getScene().addEntity(std::move(otherEntity));
    place(other, 240.0f);

    const std::optional<WorldTransform> beforeA =
        computeWorldTransform(fixture.context.getScene(), fixture.entity);
    const std::optional<WorldTransform> beforeB =
        computeWorldTransform(fixture.context.getScene(), other);
    CNA_STUDIO_EXPECT(beforeA.has_value() && beforeB.has_value());
    if (!beforeA || !beforeB) { return; }

    fixture.context.setSelection({fixture.entity, other});
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.entity.group"));
    fixture.shell->invoke("studio.entity.group");

    // One new entity, and both originals are now under it.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{1});

    const Uuid group = fixture.context.getSelection().front();
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getChildren(group).size(), std::size_t{2});

    // Selecting the group is what makes "group, then move it" work without a trip back to the
    // World Outliner -- the same bargain Duplicate strikes.
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(fixture.entity)->getParentId() == group);
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(other)->getParentId() == group);

    // Nothing moved. A group that shifted what it grouped would be one a user has to put back by
    // hand every time they tidy a hierarchy, which is the opposite of what tidying is for.
    const std::optional<WorldTransform> afterA =
        computeWorldTransform(fixture.context.getScene(), fixture.entity);
    const std::optional<WorldTransform> afterB =
        computeWorldTransform(fixture.context.getScene(), other);
    CNA_STUDIO_EXPECT(afterA.has_value() && afterB.has_value());
    if (!afterA || !afterB) { return; }
    CNA_STUDIO_EXPECT(std::fabs(afterA->position.x - beforeA->position.x) < 0.001f);
    CNA_STUDIO_EXPECT(std::fabs(afterB->position.x - beforeB->position.x) < 0.001f);

    // And the group itself sits at their middle, so the pivot a user then drags is in the middle of
    // what they grouped rather than at the world origin -- usually off screen. The two entities are
    // at 40 and 240 rather than straddling the origin on purpose: with a pivot of zero, "the middle
    // of the selection" and "wherever the default put it" are the same number, and the first draft
    // of this case could not tell them apart.
    const std::optional<WorldTransform> groupWorld =
        computeWorldTransform(fixture.context.getScene(), group);
    CNA_STUDIO_EXPECT(groupWorld.has_value());
    if (!groupWorld) { return; }
    CNA_STUDIO_EXPECT(std::fabs(groupWorld->position.x - 140.0f) < 0.001f);

    // One entry for the whole action: the user pressed Ctrl+G once.
    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(fixture.entity)->getParentId().isValid());
}

/**
 * And Ungroup is its inverse (`plan.md` STUDIO-13008).
 */
CNA_STUDIO_TEST(UngroupLiftsTheChildrenOutAndRemovesTheGroup)
{
    Fixture fixture;

    StudioEntity childEntity{Uuid::generate(), "Bone"};
    childEntity.setParentId(fixture.entity);
    const Uuid child = childEntity.getId();
    fixture.context.getScene().addEntity(std::move(childEntity));

    StudioEntity secondEntity{Uuid::generate(), "Bone 2"};
    secondEntity.setParentId(fixture.entity);
    const Uuid second = secondEntity.getId();
    fixture.context.getScene().addEntity(std::move(secondEntity));

    // Nothing to ungroup while the selection holds only leaves: greyed out rather than bright and
    // doing nothing, because a control that looks available and refuses reads as broken.
    fixture.context.setSelection({child});
    CNA_STUDIO_EXPECT(!fixture.shell->actions().isEnabled("studio.entity.ungroup"));

    fixture.context.setSelection({fixture.entity});
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.entity.ungroup"));
    fixture.shell->invoke("studio.entity.ungroup");

    // The group is gone and its children are roots, which is where it was.
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(fixture.entity) == nullptr);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(child)->getParentId().isValid());
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(second)->getParentId().isValid());

    // What came out is selected, rather than the group that is now gone: a selection naming
    // entities the scene no longer has is one the inspector cannot show.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{2});

    // One entry, and the group comes back with its children under it.
    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(fixture.entity) != nullptr);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getChildren(fixture.entity).size(),
                         std::size_t{2});
}

CNA_STUDIO_TEST(AttachRefusesToMakeAnEntityItsOwnDescendant)
{
    Fixture fixture;

    const auto add = [&](const std::string& name) {
        StudioEntity entity{Uuid::generate(), name};
        StudioComponent transform{BuiltinComponentIds::kTransform};
        transform.applyDefaults(*fixture.context.getComponentRegistry()
                                     .find(BuiltinComponentIds::kTransform));
        entity.addComponent(std::move(transform));
        const Uuid id = entity.getId();
        fixture.context.getScene().addEntity(std::move(entity));
        return id;
    };

    const Uuid root = add("Root");
    const Uuid child = add("Child");
    fixture.context.getScene().reparentEntity(child, root);

    // Selecting the root last asks for the root to become a child of its own child.
    fixture.context.select(root);
    fixture.context.toggleSelection(child);
    fixture.shell->renderFrame(at(-1.0f, -1.0f));

    const std::size_t before = fixture.context.getHistory().getCursor();
    fixture.shell->invoke("studio.entity.attach");

    // Nothing moved and nothing was recorded: a refused action leaves no entry to undo.
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(child)->getParentId() == root);
    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(root)->getParentId().isValid());
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCursor(), before);
}

// ------------------------------------------------------------------------------------------------
// Creating an entity (STUDIO-13013)
// ------------------------------------------------------------------------------------------------

/**
 * @brief The editor can add an entity to a scene, which until now it could not.
 *
 * Studio could rename, duplicate, delete, group, ungroup, reparent, hide and lock an entity. Every
 * entity in every scene arrived from a file, from a prefab, from an asset dropped into the
 * viewport, or from the single camera `newScene` builds — and `CreateEntityCommand` had existed
 * since Phase 2 with no menu, no button and no chord reaching it. A user who wanted a light had to
 * write one into the `.cnascene` by hand.
 */
CNA_STUDIO_TEST(CreatingAnEntityAddsItSelectsItAndUndoesInOnePress)
{
    Fixture fixture;

    const std::size_t before = fixture.context.getScene().getEntities().size();
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.entity.create.light.directional"));

    fixture.shell->invoke("studio.entity.create.light.directional");

    const std::vector<StudioEntity>& entities = fixture.context.getScene().getEntities();
    CNA_STUDIO_EXPECT_EQ(entities.size(), before + 1);

    // Selected, like Duplicate and like the asset drop: the next thing a user does is move or
    // rename what they just added, and a selection left on the old entity sends the first of those
    // to the wrong place.
    const Uuid created = fixture.context.getPrimarySelection();
    CNA_STUDIO_EXPECT(created != fixture.entity);

    const StudioEntity* light = fixture.context.getScene().findEntity(created);
    CNA_STUDIO_EXPECT(light != nullptr);
    if (light == nullptr) { return; }

    CNA_STUDIO_EXPECT_EQ(light->getName(), std::string{"Directional Light"});
    CNA_STUDIO_EXPECT(light->findComponent(BuiltinComponentIds::kLight) != nullptr);

    // And a transform, which the archetype does not name and every archetype gets: an entity with
    // no transform has no position, cannot be picked in the viewport, and cannot be a parent that
    // means anything.
    CNA_STUDIO_EXPECT(light->findComponent(BuiltinComponentIds::kTransform) != nullptr);

    // A root, not a child of whatever happened to be selected. Parenting to the selection is a
    // surprise a user cannot see until they move the parent.
    CNA_STUDIO_EXPECT(!light->getParentId().isValid());

    // Through the history like every other mutation (`plan.md` STUDIO-13012), and as one entry: a
    // create that took two presses to undo would be a create a user stops trusting.
    CNA_STUDIO_EXPECT(fixture.shell->actions().isEnabled("studio.edit.undo"));
    fixture.shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), before);

    fixture.shell->invoke("studio.edit.redo");
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getEntities().size(), before + 1);
}

/** @brief An empty is empty, and every other archetype is its own component plus a transform. */
CNA_STUDIO_TEST(EveryArchetypeBuildsWhatItNamesAndNothingElse)
{
    StudioContext context;

    for (const StudioEntityArchetype& archetype : studioEntityArchetypes())
    {
        const StudioEntity entity =
            studioMakeArchetypeEntity(archetype, context.getComponentRegistry());

        CNA_STUDIO_EXPECT(entity.getId().isValid());
        CNA_STUDIO_EXPECT_EQ(entity.getName(), archetype.name);
        CNA_STUDIO_EXPECT(entity.findComponent(BuiltinComponentIds::kTransform) != nullptr);

        // Its own components and no others: the count is the transform plus what it names, so an
        // archetype that quietly gained a component would fail here rather than in a scene.
        CNA_STUDIO_EXPECT_EQ(entity.getComponents().size(), archetype.components.size() + 1);

        for (const std::string& typeId : archetype.components)
        {
            // Registered, not merely spelled. A component the registry does not carry has no
            // properties, draws as an empty section and means nothing to the runtime -- and a
            // typo in this table would produce exactly that, silently.
            CNA_STUDIO_EXPECT(context.getComponentRegistry().find(typeId) != nullptr);
            CNA_STUDIO_EXPECT(entity.findComponent(typeId) != nullptr);
        }
    }

    // The empty one is the row that is always the right answer, and it is genuinely empty.
    const StudioEntityArchetype* empty = studioFindEntityArchetype("empty");
    CNA_STUDIO_EXPECT(empty != nullptr);
    if (empty != nullptr) { CNA_STUDIO_EXPECT(empty->components.empty()); }

    CNA_STUDIO_EXPECT(studioFindEntityArchetype("no-such-archetype") == nullptr);
}

/**
 * @brief The archetype table and the Entity menu are one list held in two modules.
 *
 * The labels live in the action registry, where every other command's label lives; the components
 * live in the archetype table, where the document facts live. Neither can name a kind the other
 * does not: an archetype with no menu row is a kind nobody can make, and a menu row with no
 * archetype is a row that creates nothing.
 */
CNA_STUDIO_TEST(EveryEntityArchetypeHasAMenuRowAndEveryRowAnArchetype)
{
    StudioActionRegistry registry;
    registerCoreStudioActions(registry);

    static constexpr std::string_view kPrefix = "studio.entity.create.";

    for (const StudioEntityArchetype& archetype : studioEntityArchetypes())
    {
        const std::string id = std::string{kPrefix} + archetype.id;
        const StudioAction* action = registry.find(id);
        if (action == nullptr)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "archetype '" + archetype.id + "' has no '" + id + "' command.");
            continue;
        }

        // A label and a description, because the menu shows one and the shortcut editor the other.
        CNA_STUDIO_EXPECT(!action->label.empty());
        CNA_STUDIO_EXPECT(!action->description.empty());
    }

    for (const StudioAction& action : registry.commands())
    {
        if (action.id.rfind(kPrefix, 0) != 0) { continue; }
        const std::string archetypeId = action.id.substr(kPrefix.size());
        if (studioFindEntityArchetype(archetypeId) == nullptr)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "'" + action.id + "' names no archetype, so it would create nothing.");
        }
    }

    // A scan that matched nothing would agree with everything.
    CNA_STUDIO_EXPECT(studioEntityArchetypes().size() >= 5);
}

/**
 * @brief Every preset names a property that exists on a component the archetype includes.
 *
 * `plan.md` STUDIO-20002. A preset is three strings and a value, and every way of getting one
 * wrong is silent: naming a component the archetype does not carry, a property the descriptor
 * does not declare, or a value of the wrong type all produce an entity that looks right in the
 * Outliner and is not the kind the menu row promised. The Point Light is the case — get its
 * preset wrong and the user is handed a directional light called "Point Light".
 */
CNA_STUDIO_TEST(EveryArchetypePresetNamesAPropertyThatExistsAndFits)
{
    StudioContext context;
    const ComponentRegistry& registry = context.getComponentRegistry();

    std::size_t presetsChecked = 0;

    for (const StudioEntityArchetype& archetype : studioEntityArchetypes())
    {
        const StudioEntity built = studioMakeArchetypeEntity(archetype, registry);

        for (const StudioEntityArchetype::Preset& preset : archetype.presets)
        {
            ++presetsChecked;

            // On the entity this archetype actually builds, not merely in its component list:
            // a preset for a component the registry could not supply would be dropped in silence.
            if (built.findComponent(preset.component) == nullptr)
            {
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "archetype '" + archetype.id + "' presets '" + preset.component + "."
                    + preset.property + "' and builds no such component.");
                continue;
            }

            const ComponentDescriptor* owner = registry.find(preset.component);
            CNA_STUDIO_EXPECT(owner != nullptr);
            if (owner == nullptr) { continue; }

            const PropertyDescriptor* declared = owner->findProperty(preset.property);
            if (declared == nullptr)
            {
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "archetype '" + archetype.id + "' presets '" + preset.component + "."
                    + preset.property + "', which that component does not declare.");
                continue;
            }

            // The right *kind* of value: a float written into an enumeration reads back as
            // nothing, and the entity carries a property the runtime cannot use.
            if (preset.value.getType() != declared->type)
            {
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "archetype '" + archetype.id + "' presets '" + preset.property
                    + "' with the wrong kind of value.");
                continue;
            }

            // An enumeration preset has to name one of the options, or the user gets a kind
            // nothing in the editor or the runtime has heard of.
            if (declared->type == PropertyType::Enum)
            {
                const std::string wanted = preset.value.get<PropertyValue::EnumValue>().name;
                const bool offered = std::find(declared->enumOptions.begin(),
                                               declared->enumOptions.end(), wanted)
                                     != declared->enumOptions.end();
                if (!offered)
                {
                    CnaStudioTest::reportFailure(__FILE__, __LINE__,
                        "archetype '" + archetype.id + "' presets '" + preset.property + "' to '"
                        + wanted + "', which is not one of its values.");
                    continue;
                }
            }

            // And it landed. The whole mechanism is worth nothing if the entity comes back with
            // the descriptor's default on it.
            CNA_STUDIO_EXPECT(built.findComponent(preset.component)->getProperty(preset.property)
                              == preset.value);
        }
    }

    // A scan that found no preset would agree with everything. The Point Light is the one that
    // exists today; this number goes up as the mechanism is used.
    CNA_STUDIO_EXPECT(presetsChecked >= 1);
}
