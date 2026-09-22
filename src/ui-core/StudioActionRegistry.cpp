// SPDX-License-Identifier: MS-PL
/**
 * @file StudioActionRegistry.cpp
 * @brief Command storage, dispatch, shortcut binding, and Studio's core command set.
 */

#include "CNA/Studio/UiCore/StudioActionRegistry.hpp"

#include <algorithm>

namespace CNA::Studio
{
    namespace
    {
        /** @brief Builds a modifier set from the flags a shortcut needs. */
        UiKeyModifiers mods(bool control = false, bool shift = false, bool alt = false)
        {
            UiKeyModifiers m;
            m.control = control;
            m.shift = shift;
            m.alt = alt;
            return m;
        }

        /** @brief Builds a shortcut. */
        StudioShortcut chord(UiKey key, UiKeyModifiers modifiers = {})
        {
            StudioShortcut shortcut;
            shortcut.key = key;
            shortcut.modifiers = modifiers;
            return shortcut;
        }

        /** @brief The display text for a key, for menu shortcut hints. */
        std::string_view keyNameImpl(UiKey key)
        {
            switch (key)
            {
                case UiKey::Tab:        return "Tab";
                case UiKey::LeftArrow:  return "Left";
                case UiKey::RightArrow: return "Right";
                case UiKey::UpArrow:    return "Up";
                case UiKey::DownArrow:  return "Down";
                case UiKey::PageUp:     return "PageUp";
                case UiKey::PageDown:   return "PageDown";
                case UiKey::Home:       return "Home";
                case UiKey::End:        return "End";
                case UiKey::Insert:     return "Insert";
                case UiKey::Delete:     return "Delete";
                case UiKey::Backspace:  return "Backspace";
                case UiKey::Space:      return "Space";
                case UiKey::Enter:      return "Enter";
                case UiKey::Escape:     return "Escape";
                case UiKey::A: return "A";  case UiKey::C: return "C";
                case UiKey::V: return "V";  case UiKey::X: return "X";
                case UiKey::Y: return "Y";  case UiKey::Z: return "Z";
                case UiKey::D: return "D";  case UiKey::F: return "F";
                case UiKey::N: return "N";  case UiKey::O: return "O";
                case UiKey::B: return "B";
                case UiKey::G: return "G";
                case UiKey::Q: return "Q";
                case UiKey::S: return "S";  case UiKey::W: return "W";
                case UiKey::E: return "E";  case UiKey::R: return "R";
                case UiKey::F1: return "F1"; case UiKey::F2: return "F2";
                case UiKey::F5: return "F5";
                case UiKey::F12: return "F12";
                case UiKey::Digit2: return "2";
                case UiKey::Digit3: return "3";
                case UiKey::Digit4: return "4";
                case UiKey::None:
                case UiKey::Count: break;
            }
            return "";
        }
    } // namespace

    std::string_view studioActionCategoryName(StudioActionCategory category)
    {
        switch (category)
        {
            case StudioActionCategory::File:    return "File";
            case StudioActionCategory::Edit:    return "Edit";
            case StudioActionCategory::Entity:  return "Entity";
            case StudioActionCategory::View:    return "View";
            case StudioActionCategory::Project: return "Project";
            case StudioActionCategory::Build:   return "Build";
            case StudioActionCategory::Play:    return "Play";
            case StudioActionCategory::Tools:   return "Tools";
            case StudioActionCategory::Window:  return "Window";
            case StudioActionCategory::Help:    return "Help";
        }
        return "";
    }

    std::string_view studioKeyName(UiKey key) { return keyNameImpl(key); }

    bool parseStudioKey(std::string_view name, UiKey& out)
    {
        if (name.empty()) { return false; }

        // Over the enum rather than a second table: a name list written twice is a list that
        // disagrees with itself the first time a key is added, and the disagreement is a shortcut
        // that stops loading.
        for (int value = 1; value < static_cast<int>(UiKey::Count); ++value)
        {
            const auto key = static_cast<UiKey>(value);
            if (!keyNameImpl(key).empty() && keyNameImpl(key) == name) { out = key; return true; }
        }
        return false;
    }

    bool parseStudioShortcut(std::string_view text, StudioShortcut& out)
    {
        if (text.empty()) { return false; }

        StudioShortcut parsed;
        std::size_t start = 0;
        while (true)
        {
            const std::size_t plus = text.find('+', start);
            const std::string_view part = text.substr(start, plus == std::string_view::npos
                                                                 ? std::string_view::npos
                                                                 : plus - start);
            if (plus == std::string_view::npos)
            {
                if (!parseStudioKey(part, parsed.key)) { return false; }
                break;
            }

            if (part == "Ctrl") { parsed.modifiers.control = true; }
            else if (part == "Alt") { parsed.modifiers.alt = true; }
            else if (part == "Shift") { parsed.modifiers.shift = true; }
            else if (part == "Super") { parsed.modifiers.super = true; }
            else { return false; }

            start = plus + 1;
        }

        out = parsed;
        return true;
    }

    std::string describeStudioShortcut(const StudioShortcut& shortcut)
    {
        if (!shortcut.isBound()) { return {}; }

        std::string text;
        // Conventional order, which is what every platform's menus use. Reordering the modifiers
        // makes a familiar chord read as an unfamiliar one.
        if (shortcut.modifiers.control) { text += "Ctrl+"; }
        if (shortcut.modifiers.alt) { text += "Alt+"; }
        if (shortcut.modifiers.shift) { text += "Shift+"; }
        if (shortcut.modifiers.super) { text += "Super+"; }
        text += studioKeyName(shortcut.key);
        return text;
    }

    bool StudioActionRegistry::add(StudioAction command)
    {
        const auto existing = std::find_if(commands_.begin(), commands_.end(),
            [&](const StudioAction& c) { return c.id == command.id; });

        if (existing != commands_.end())
        {
            *existing = std::move(command);
            return true;
        }
        commands_.push_back(std::move(command));
        return false;
    }

    bool StudioActionRegistry::remove(std::string_view id)
    {
        const auto found = std::find_if(commands_.begin(), commands_.end(),
            [&](const StudioAction& command) { return command.id == id; });
        if (found == commands_.end()) { return false; }
        commands_.erase(found);
        return true;
    }

    const StudioAction* StudioActionRegistry::find(std::string_view id) const
    {
        const auto found = std::find_if(commands_.begin(), commands_.end(),
            [&](const StudioAction& c) { return c.id == id; });
        return found == commands_.end() ? nullptr : &*found;
    }

    bool StudioActionRegistry::isEnabled(std::string_view id) const
    {
        const StudioAction* command = find(id);
        if (command == nullptr) { return false; }

        // A command with no handler is not available, whatever its predicate says. Otherwise every
        // half-migrated menu row draws as though it works, and a control that looks available and
        // then does nothing is indistinguishable from one that is broken -- which is worse than a
        // greyed-out row, because the user cannot tell whether to report it.
        if (!command->run) { return false; }

        // No predicate means always available. That is the common case, and requiring every
        // command to supply a trivial one would be noise that hides the ones that matter.
        return !command->isEnabled || command->isEnabled();
    }

    bool StudioActionRegistry::isChecked(std::string_view id) const
    {
        const StudioAction* command = find(id);
        if (command == nullptr || !command->checkable || !command->isChecked) { return false; }
        return command->isChecked();
    }

    StudioActionResult StudioActionRegistry::invoke(std::string_view id)
    {
        const StudioAction* command = find(id);
        if (command == nullptr) { return StudioActionResult::NotFound; }
        if (command->isEnabled && !command->isEnabled()) { return StudioActionResult::Disabled; }
        if (!command->run) { return StudioActionResult::NotImplemented; }

        // Copied before invoking. A handler may register commands -- a plugin loading, a tool
        // installing its own actions -- and that reallocates the vector out from under this
        // pointer. The crash would be intermittent and would look like anything but this.
        const std::function<void()> handler = command->run;
        handler();
        return StudioActionResult::Invoked;
    }

    const StudioAction* StudioActionRegistry::findByShortcut(const StudioShortcut& shortcut) const
    {
        if (!shortcut.isBound()) { return nullptr; }

        const auto found = std::find_if(commands_.begin(), commands_.end(),
            [&](const StudioAction& c) { return c.shortcut.isBound() && c.shortcut == shortcut; });
        return found == commands_.end() ? nullptr : &*found;
    }

    StudioActionResult StudioActionRegistry::invokeShortcut(const StudioShortcut& shortcut)
    {
        const StudioAction* command = findByShortcut(shortcut);
        if (command == nullptr) { return StudioActionResult::NotFound; }
        return invoke(command->id);
    }

    bool StudioActionRegistry::rebind(std::string_view id, const StudioShortcut& shortcut,
                                       std::string* outConflictingCommandId)
    {
        const auto target = std::find_if(commands_.begin(), commands_.end(),
            [&](const StudioAction& c) { return c.id == id; });
        if (target == commands_.end()) { return false; }

        if (shortcut.isBound())
        {
            const StudioAction* holder = findByShortcut(shortcut);
            if (holder != nullptr && holder->id != target->id)
            {
                // Refused rather than shadowed. Two commands on one chord means one has stopped
                // working, and the user who bound the second has no way to find out which.
                if (outConflictingCommandId != nullptr) { *outConflictingCommandId = holder->id; }
                return false;
            }
        }
        target->shortcut = shortcut;
        return true;
    }

    std::vector<const StudioAction*> StudioActionRegistry::inCategory(
        StudioActionCategory category) const
    {
        std::vector<const StudioAction*> result;
        for (const StudioAction& command : commands_)
        {
            if (command.category == category) { result.push_back(&command); }
        }
        return result;
    }

    void registerCoreStudioActions(StudioActionRegistry& registry)
    {
        const auto command = [&](const char* id, const char* label, const char* description,
                                 StudioActionCategory category, StudioShortcut shortcut,
                                 bool checkable = false) {
            StudioAction entry;
            entry.id = id;
            entry.label = label;
            entry.description = description;
            entry.category = category;
            entry.shortcut = shortcut;
            entry.checkable = checkable;
            registry.add(std::move(entry));
        };

        using C = StudioActionCategory;

        // The chords follow the prototype's, which is what existing users' hands already know
        // (docs/MIGRATION-INVENTORY.md). Ctrl+N is New *Scene* there and is the frequent one, so it
        // keeps the plain chord; New Project takes the Shift variant, as it does in most IDEs.
        command("studio.file.newScene", "New Scene",
                "Start an empty scene.", C::File, chord(UiKey::N, mods(true)));
        command("studio.file.newProject", "New Project...",
                "Create a new CNA game project.", C::File, chord(UiKey::N, mods(true, true)));
        // `plan.md` STUDIO-10007. The asset pipeline has recognised, loaded, edited and
        // dependency-tracked a `.cnamaterial` since ED-403, and nothing could make one: a user had
        // to write the file by hand before the editor would show them any of that.
        //
        // No shortcut. Creating an asset is a deliberate, infrequent act, and the chords left are
        // worth more to things people do every minute.
        command("studio.asset.newMaterial", "New Material",
                "Create a material asset in the current folder.", C::File, StudioShortcut{});
        // `plan.md` STUDIO-10010, and the same gap `STUDIO-10007` closed for materials: the
        // database recognises a `.cnaenv`, the Inspector edits it and the dependency scan follows
        // its panorama, and until this nothing could write the first one. No shortcut, for the
        // reason New Material has none.
        command("studio.asset.newEnvironmentMap", "New Environment Map",
                "Create an environment map asset in the current folder.", C::File,
                StudioShortcut{});
        // Ctrl+O, not Ctrl+D: Ctrl+D is Duplicate in the prototype and in every editor that has a
        // duplicate, and taking it for Open would silently repurpose a key people press all day.
        command("studio.file.openProject", "Open Project...",
                "Open an existing CNA game project.", C::File, chord(UiKey::O, mods(true)));
        command("studio.file.save", "Save",
                "Save the active document.", C::File, chord(UiKey::S, mods(true)));
        command("studio.file.saveAll", "Save All",
                "Save every document with unsaved changes.", C::File,
                chord(UiKey::S, mods(true, true)));
        // No chord, deliberately. These appear only after a crash, they are answered once, and one
        // of the two throws work away -- a key that does that is a key somebody hits by accident.
        command("studio.file.recoverScene", "Recover Unsaved Scene",
                "Restore the unsaved work a previous session left behind.", C::File,
                StudioShortcut{});
        command("studio.file.discardRecovered", "Discard Recovered Scene",
                "Throw away the unsaved work a previous session left behind.", C::File,
                StudioShortcut{});
        command("studio.file.quit", "Quit",
                "Close CNA Studio.", C::File, chord(UiKey::Q, mods(true)));

        command("studio.edit.undo", "Undo",
                "Undo the last change.", C::Edit, chord(UiKey::Z, mods(true)));
        command("studio.edit.redo", "Redo",
                "Redo the last undone change.", C::Edit, chord(UiKey::Y, mods(true)));
        // F2, as in the prototype and in every file manager and editor that renames in place.
        command("studio.edit.rename", "Rename",
                "Rename the selection in the World Outliner.", C::Edit, chord(UiKey::F2));
        command("studio.edit.duplicate", "Duplicate",
                "Duplicate the selection.", C::Edit, chord(UiKey::D, mods(true)));
        command("studio.edit.delete", "Delete",
                "Delete the selection.", C::Edit, chord(UiKey::Delete));

        // Creating an entity, which the editor could not do at all until `plan.md` STUDIO-13013:
        // every entity in every scene arrived from a file, a prefab, an asset dropped into the
        // viewport, or the one camera New Scene builds. `CreateEntityCommand` had existed since
        // Phase 2 and no menu, button or chord reached it.
        //
        // The ids end in the archetype id that `studioEntityArchetypes()` carries, and
        // `EveryEntityArchetypeHasAMenuRowAndEveryRowAnArchetype` holds the two lists together --
        // the *labels* live here, where every other command's label lives, and the components each
        // one gets live there, where the document facts live.
        //
        // No chords, including for the empty one. Unity and Godot both put Create Empty on
        // Ctrl+Shift+N and that chord is New Project here -- taking it would silently repurpose a
        // key somebody's hands already know, which is the thing the New Project comment above is
        // about. The shortcut editor binds these for anybody who wants them, and a chord per kind
        // would spend six of the chords left on six things a user does rarely.
        command("studio.entity.create.empty", "Create Empty",
                "Add an entity with a transform and nothing else.", C::Entity, StudioShortcut{});
        command("studio.entity.create.camera", "Create Camera",
                "Add a camera the game can render through.", C::Entity, StudioShortcut{});
        command("studio.entity.create.light.directional", "Create Directional Light",
                "Add a light with a direction and no position, like the sun.", C::Entity,
                StudioShortcut{});
        command("studio.entity.create.light.point", "Create Point Light",
                "Add a light that shines in every direction from where it is.", C::Entity,
                StudioShortcut{});
        // Offered although this build draws its cone as no cone at all: `CNA.Light` has carried
        // the kind since Phase 1, a scene may already hold one, and a game reading the loader's
        // components can implement a cone itself. What Studio owes the user is to *say* so, which
        // `validateScene`'s `spot-light-cone-not-rendered` does (`plan.md` STUDIO-20008) --
        // removing the kind would break scenes over a limitation of one renderer.
        command("studio.entity.create.light.spot", "Create Spot Light",
                "Add a light aimed in a cone. This build draws it as a point light.", C::Entity,
                StudioShortcut{});
        command("studio.entity.create.sprite", "Create Sprite",
                "Add a 2D sprite renderer with no texture yet.", C::Entity, StudioShortcut{});
        command("studio.entity.create.model", "Create Model",
                "Add a 3D model renderer with no mesh yet.", C::Entity, StudioShortcut{});
        command("studio.entity.create.audio", "Create Audio Source",
                "Add a sound the game can play from a place.", C::Entity, StudioShortcut{});

        // Parenting from the viewport (`plan.md` STUDIO-12010). Unbound, for the reason the six
        // standard views are: there is no chord every editor agrees on -- Unreal's Ctrl+P and
        // Shift+P are the nearest thing, and neither key is in this editor's vocabulary at all --
        // and inventing a scheme nobody knows is worse than a menu entry somebody can bind for
        // themselves, which the shortcut editor lets them do.
        command("studio.entity.attach", "Attach to Last Selected",
                "Make every other selected entity a child of the one selected last.", C::Entity,
                StudioShortcut{});
        command("studio.entity.detach", "Detach",
                "Make the selected entities roots, where they are.", C::Entity, StudioShortcut{});

        // Ctrl+G and Ctrl+Shift+G, as in every editor that groups anything (`plan.md`
        // STUDIO-13008). Distinct from Attach: Attach puts the selection under one of *itself*,
        // which needs a thing to be the parent; Group makes the parent, which is what a user wants
        // when the parent does not exist yet.
        command("studio.entity.group", "Group",
                "Put the selected entities under a new empty parent.", C::Entity,
                chord(UiKey::G, mods(true)));
        command("studio.entity.ungroup", "Ungroup",
                "Move the selected groups' children up and remove the groups.", C::Entity,
                chord(UiKey::G, mods(true, true)));

        command("studio.view.focusSelected", "Focus Selected",
                "Move the viewport camera to frame the selection.", C::View, chord(UiKey::F));
        command("studio.view.toggleGrid", "Show Grid",
                "Show or hide the viewport grid.", C::View, {}, /*checkable=*/true);
        // `STUDIO-07056`. Checkable rather than two commands, because it is one choice with two
        // answers and a pair would put both on the menu with one of them always wrong. Disabled in
        // the 2D view rather than hidden: a user who went looking for it should find it and see
        // why it is greyed out, which a missing row cannot tell them.
        command("studio.view.gridOnGroundPlane", "Grid on Ground Plane",
                "Draw the 3D grid on the ground plane (XZ) rather than the scene's own (XY).",
                C::View, {}, /*checkable=*/true);
        command("studio.view.translate", "Translate",
                "Switch the gizmo to translation.", C::View, chord(UiKey::W));
        command("studio.view.rotate", "Rotate",
                "Switch the gizmo to rotation.", C::View, chord(UiKey::E));
        // The two views, checkable and exclusive. The prototype binds 2 and 3 for these and so
        // does this: they are the keys anybody who has used a 3D editor reaches for, and a view
        // that can only be changed through a menu is one people stop changing.
        command("studio.view.2d", "2D View",
                "Show the scene in the orthographic 2D view.", C::View, chord(UiKey::Digit2),
                /*checkable=*/true);
        command("studio.view.3d", "3D View",
                "Show the scene in the 3D view.", C::View, chord(UiKey::Digit3),
                /*checkable=*/true);

        // The game view (`plan.md` STUDIO-11012). Digit4 because it carries on the row the other
        // two started and is what a user who has pressed 2 and 3 will try next -- the digits there
        // name a dimension and this one does not, which is a small inconsistency against a shortcut
        // nobody has to be told.
        command("studio.view.game", "Game View",
                "Show the scene through its own camera, as the game will.", C::View,
                chord(UiKey::Digit4), /*checkable=*/true);

        // The six axis-aligned views (`plan.md` STUDIO-11004). Unbound by default, which is a
        // decision rather than an omission: the keys a user's hands already know for these are the
        // numpad's 1, 3 and 7, which this build's key vocabulary does not carry, and the plain
        // digits next to them are already the 2D and 3D toggles above. Inventing a third scheme
        // nobody knows would be worse than a menu entry somebody can bind for themselves, which
        // the shortcut editor lets them do.
        command("studio.view.front", "Front View",
                "Look at the scene from the front, along -Z.", C::View, StudioShortcut{});
        command("studio.view.back", "Back View",
                "Look at the scene from behind, along +Z.", C::View, StudioShortcut{});
        command("studio.view.left", "Left View",
                "Look at the scene from the left, along +X.", C::View, StudioShortcut{});
        command("studio.view.right", "Right View",
                "Look at the scene from the right, along -X.", C::View, StudioShortcut{});
        command("studio.view.top", "Top View",
                "Look straight down at the scene.", C::View, StudioShortcut{});
        command("studio.view.bottom", "Bottom View",
                "Look straight up at the scene.", C::View, StudioShortcut{});

        // How the 3D view draws geometry (`plan.md` STUDIO-11010). Checkable and exclusive, like
        // the 2D/3D pair: a toolbar has to *show* which is on, because the difference between a
        // clean shaded picture and a hatched one is exactly what a user is looking at when they
        // reach for this.
        command("studio.view.shading.shaded", "Shaded",
                "Draw solid geometry without its edges.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.shading.wireframe", "Wireframe",
                "Draw only the edges: no solid meshes and no textured sprites.", C::View,
                StudioShortcut{}, /*checkable=*/true);
        command("studio.view.shading.shadedWireframe", "Shaded Wireframe",
                "Draw solid geometry with its edges over it.", C::View, StudioShortcut{},
                /*checkable=*/true);

        // What the 3D view colours a surface by (`plan.md` STUDIO-11011). A second exclusive
        // group rather than three more shading modes, because it answers a different question and
        // the two compose: roughness in shaded-wireframe is a reasonable thing to be looking at.
        // No shortcuts: a debug view is something a user turns on to answer a question and off
        // again, and a key that silently recoloured the scene would be a bug report.
        command("studio.view.debug.none", "Default",
                "Draw the scene as it is authored.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.debug.unlit", "Unlit",
                "Draw base colour with the lights ignored.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.debug.lighting", "Lighting Only",
                "Draw the lights over a white surface.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.debug.metallic", "Metallic",
                "Draw metalness as grey, black to white.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.debug.roughness", "Roughness",
                "Draw roughness as grey, black to white.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.debug.normals", "Normals",
                "Draw each surface normal, coloured by the way it points.", C::View,
                StudioShortcut{}, /*checkable=*/true);

        // The bounds overlay (`plan.md` STUDIO-11008). Three exclusive checkable entries for the
        // *which entities* question and one plain toggle for the sphere, because the two are
        // independent: a user asking "how big is the volume I am clicking" and one asking "how
        // much bigger is the sphere than the box" are asking different things, and folding the
        // sphere into a fourth mode would make one of them unreachable.
        command("studio.view.bounds.off", "No Bounds",
                "Draw no bounding volumes.", C::View, StudioShortcut{}, /*checkable=*/true);
        command("studio.view.bounds.selected", "Bounds: Selected",
                "Draw the bounding box of the selected entities.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.bounds.all", "Bounds: All",
                "Draw every entity's bounding box.", C::View, StudioShortcut{}, /*checkable=*/true);
        command("studio.view.bounds.spheres", "Bounding Spheres",
                "Also draw the sphere a BoundingSphere collision test would use.", C::View,
                StudioShortcut{}, /*checkable=*/true);

        // The tilemap tools. Checkable, because a toolbar has to *show* which one is armed: a
        // press means something different under each of them, and a user who cannot see which is
        // active finds out by editing their level.
        command("studio.view.tool.select", "Select Tool",
                "Pick entities and drag the gizmo.", C::View, StudioShortcut{}, /*checkable=*/true);
        command("studio.view.tool.paint", "Paint Tiles",
                "Set the tile under the cursor on the selected tilemap.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.tool.erase", "Erase Tiles",
                "Clear the tile under the cursor.", C::View, StudioShortcut{}, /*checkable=*/true);
        command("studio.view.tool.pick", "Pick Tile",
                "Take the tile under the cursor as the brush, then go back to painting.", C::View,
                StudioShortcut{}, /*checkable=*/true);
        command("studio.view.tool.fill", "Fill Tiles",
                "Fill the rectangle a drag encloses.", C::View, StudioShortcut{},
                /*checkable=*/true);

        // Snapping as a state rather than only as a held key (`plan.md` STUDIO-12007). Checkable,
        // because a viewport where a drag rounds and a viewport where it does not look identical
        // until the drag happens -- and a user who cannot see which they are in finds out by
        // placing something wrong.
        // Where a multi-selection turns and resizes about (`plan.md` STUDIO-12006). Exclusive and
        // checkable, like the shading modes: the two give different results from the same drag, and
        // a user who cannot see which is set finds out by turning a group the wrong way round.
        command("studio.view.pivot.center", "Pivot: Center",
                "Turn and resize a selection about the middle of it.", C::View, StudioShortcut{},
                /*checkable=*/true);
        command("studio.view.pivot.active", "Pivot: Active",
                "Turn and resize a selection about the entity selected last.", C::View,
                StudioShortcut{}, /*checkable=*/true);

        command("studio.view.snap", "Snap",
                "Round drags to the project's grid, angle and scale steps.", C::View,
                StudioShortcut{}, /*checkable=*/true);

        command("studio.view.toggleGizmoSpace", "Toggle Gizmo Space",
                "Switch the gizmo between world and local space.", C::View, chord(UiKey::X));
        command("studio.view.scale", "Scale",
                "Switch the gizmo to scaling.", C::View, chord(UiKey::R));

        command("studio.play.play", "Play",
                "Launch the game in a player process.", C::Play, chord(UiKey::F5));
        command("studio.play.stop", "Stop",
                "Stop the running player.", C::Play, chord(UiKey::F5, mods(false, true)));
        // Checkable rather than a button whose label flips between Pause and Resume. A menu row
        // that renames itself is one a user cannot find twice, and a toolbar has to *show* whether
        // the game is paused: the window is there either way, so nothing else says which.
        command("studio.play.pause", "Pause",
                "Pause the running game, or resume it.", C::Play,
                chord(UiKey::F5, mods(true)), /*checkable=*/true);
        command("studio.play.step", "Step One Frame",
                "Advance a paused game by a single frame.", C::Play, chord(UiKey::F5, mods(false, false, true)));
        command("studio.play.restart", "Restart",
                "Stop the game and start it again from the scene as it now stands.", C::Play,
                StudioShortcut{});
        // Beside Restart rather than instead of it, because they answer different questions
        // (`plan.md` STUDIO-16006). Restart begins the game again from the top; this hands the
        // running game the scene as it now stands and lets it carry on, which is what a level
        // designer wants when the thing they are tuning is thirty seconds in.
        command("studio.play.reloadScene", "Reload Scene In Game",
                "Send the saved scene to the running game without restarting it.", C::Play,
                chord(UiKey::F5, mods(true, true)));
        // F12, which is what every game and every launcher already uses for this. A shortcut a
        // user does not have to learn is worth more than one that is internally consistent.
        command("studio.play.capture", "Capture Frame",
                "Ask the running game to write the frame it is showing to a file.", C::Play,
                chord(UiKey::F12));

        command("studio.build.build", "Build",
                // Ctrl+B, not F2: F2 is Rename in the prototype (docs/MIGRATION-INVENTORY.md) and
                // in every file manager, and a shortcut that moved is one every existing user has
                // to relearn -- silently, because it still does something.
                "Build the project with its own CMake.", C::Build, chord(UiKey::B, mods(true)));
        command("studio.build.cancel", "Cancel Build",
                "Stop the build that is running.", C::Build, {});

        command("studio.build.package", "Package...",
                "Package a standalone build of the game.", C::Build, {});

        // `plan.md` CORE-02. The one gesture that says what CNA Studio is: Studio is a visual
        // companion, the C++ is written in a real IDE, and a developer must be able to cross that
        // line at any moment. The preference these read has existed since the prototype and had no
        // caller at all until now, so Studio was claiming an integration it did not have.
        command("studio.tools.openProjectInEditor", "Open Project in External Editor",
                "Open this project in the editor set in Preferences.", C::Tools, {});
        // Separate from the row above because they answer different questions. One hands over the
        // whole project; this one puts a cursor in the file a developer is going to edit first,
        // which is the file the project's language scaffolded.
        command("studio.tools.openSourceInEditor", "Open Main Source in External Editor",
                "Open this project's entry-point source file in the editor set in Preferences.",
                C::Tools, {});

        command("studio.window.resetLayout", "Reset Layout",
                "Restore the default panel arrangement.", C::Window, {});

        command("studio.window.saveLayoutAs", "Save Layout As...",
                "Save this arrangement under a name.", C::Window, {});

        command("studio.window.dockAll", "Dock All Windows",
                "Return every floating panel to the workspace.", C::Window, {});

        command("studio.help.about", "About CNA Studio",
                "Version, renderer and platform information.", C::Help, chord(UiKey::F1));

        // Toggles carry their checked state the same way they carry enablement -- through a
        // predicate pulled when needed, so it cannot go stale. The predicate itself is attached
        // when the viewport service that owns the grid setting lands (STUDIO-11005).
    }
} // namespace CNA::Studio
