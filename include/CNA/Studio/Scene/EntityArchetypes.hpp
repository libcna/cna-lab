// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Scene/EntityArchetypes.hpp
 * @brief The kinds of entity the editor can make, and the one function that makes them.
 *
 * `plan.md` STUDIO-13013. Studio could rename, duplicate, delete, group, reparent, hide and lock
 * an entity, and could not **create** one. Every entity in every scene arrived from a file, from
 * a prefab, from an asset dropped into the viewport, or from the single camera `newScene` builds.
 * `CreateEntityCommand` had existed since Phase 2 with two callers -- the asset drop and Group --
 * and no menu, no button and no shortcut reached it.
 *
 * ### Why a table rather than one action per kind
 *
 * An archetype is data: an id, a name, and which component type ids go on it. Adding a kind is
 * a row, not a function, which is what keeps `STUDIO-20002`'s point light from being a second
 * code path that drifts from this one. The alternative -- an action apiece, each building an
 * entity by hand -- is how the *defaults* come to differ between two ways of making the same
 * thing.
 *
 * ### The component list, and what is deliberately not in it
 *
 * Every archetype gets a `CNA.Transform`, including the empty one: an entity with no transform
 * has no position, cannot be selected in the viewport, and cannot be a parent that means
 * anything. Beyond that each names only what it *is*. Nothing here sets a property: a component's
 * defaults come from its descriptor, and an archetype that set them a second time would be a
 * second answer to "what is a new Light" -- the disagreement `ED-300` is about, in a different
 * costume.
 *
 * That is also why there is no point or spot light row yet. Both would need `kind` preset, which
 * is a mechanism this file does not have and should not grow until something needs it
 * (`STUDIO-20002`, `STUDIO-20003`).
 *
 * ### The label is not here
 *
 * An archetype carries the entity's *name* and nothing else a person reads. The menu row's label
 * and its one-line description belong to the action, and the action registry already holds both
 * for every command in the editor. Repeating them here would be the same string in two modules,
 * free to disagree -- and the disagreement would be a menu that says one thing and makes another.
 * `EveryEntityArchetypeHasAMenuRowAndEveryRowAnArchetype` ties the two lists by id instead.
 *
 * ### Names are not made unique
 *
 * Two entities called "Light" are two entities called "Light", and that is the editor's existing
 * behaviour: Duplicate appends " Copy" whether or not one exists already, because an entity is
 * its id and never its name (`ANALYSIS.md` D-08). A numbering scheme here would be the only place
 * in Studio that thought otherwise.
 */

#include <string>
#include <string_view>
#include <vector>

#include "CNA/Studio/Core/ComponentDescriptor.hpp"
#include "CNA/Studio/Core/PropertyValue.hpp"
#include "CNA/Studio/Scene/StudioEntity.hpp"

namespace CNA::Studio
{
    /** @brief One kind of entity the Entity menu offers to create. */
    struct StudioEntityArchetype
    {
        /**
         * @brief The stable id, which is also the action's suffix:
         *        `studio.entity.create.light.directional`.
         *
         * Stable because a shortcut a user binds is bound to an action id, and renaming one would
         * silently unbind it.
         */
        std::string id;

        /** @brief What the new entity is called. */
        std::string name;

        /**
         * @brief Component type ids beyond `CNA.Transform`, which every archetype gets.
         *
         * Empty for the empty one, which is the whole of what makes it empty.
         */
        std::vector<std::string> components;

        /** @brief One property set away from its descriptor's default. */
        struct Preset
        {
            /** @brief The component type id that carries it. */
            std::string component;

            /** @brief The property's @ref PropertyDescriptor::name. */
            std::string property;

            /** @brief What to set it to. */
            PropertyValue value;
        };

        /**
         * @brief Properties this kind sets for itself (`plan.md` STUDIO-20002).
         *
         * Empty for every archetype whose defaults already describe it, which is most of them --
         * a preset repeating a descriptor's default would be a second answer to the same question
         * and free to drift from it the day the default moves.
         *
         * The point and spot lights are what forced this: all three light kinds are one component
         * distinguished by an enumeration, so "a point light" cannot be expressed as a set of
         * components at all. `EveryArchetypePresetNamesAPropertyThatExistsAndFits` checks each one
         * against the descriptor it claims, because a preset naming a component the archetype does
         * not include, or a property of the wrong type, silently does nothing.
         */
        std::vector<Preset> presets;
    };

    /** @brief Every kind the editor can create, in the order the menu lists them. */
    [[nodiscard]] const std::vector<StudioEntityArchetype>& studioEntityArchetypes();

    /** @brief The archetype with @p id, or null. */
    [[nodiscard]] const StudioEntityArchetype* studioFindEntityArchetype(std::string_view id);

    /**
     * @brief Builds the entity @p archetype describes, with every component at its defaults.
     *
     * A component type the registry does not carry is skipped rather than added empty: a
     * component with no descriptor has no properties, draws as an empty section in the Inspector
     * and means nothing to the runtime, so adding one would be worse than the build this build
     * can honestly do.
     *
     * @return The entity, with a fresh id. Nothing is added to any document -- that is
     *         `CreateEntityCommand`'s job, because creation belongs in the undo history.
     */
    [[nodiscard]] StudioEntity studioMakeArchetypeEntity(const StudioEntityArchetype& archetype,
                                                         const ComponentRegistry& registry);
}
