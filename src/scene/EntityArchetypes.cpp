// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Scene/EntityArchetypes.hpp"

#include "CNA/Studio/Scene/BuiltinComponents.hpp"

#include <algorithm>

namespace CNA::Studio
{
    const std::vector<StudioEntityArchetype>& studioEntityArchetypes()
    {
        // Empty first, because it is the one that is always the right answer -- a user who wants
        // something this list does not offer builds it from an empty and the Add Component menu,
        // and that path must not be the one they have to hunt for.
        //
        // Then the four kinds the built-in registry describes and the editor already draws: a
        // camera has a frustum in the viewport, a light has an aim and a badge, and the two
        // renderers have an asset slot the Content Browser can fill. `CNA.Tilemap`,
        // `CNA.SpriteAnimation`, `CNA.Tags`, `CNA.Layer` and `CNA.AudioListener` are components a
        // user adds *to* something rather than kinds of thing, so they are not rows here.
        static const std::vector<StudioEntityArchetype> kArchetypes = {
            {"empty", "Entity", {}, {}},
            {"camera", "Camera", {BuiltinComponentIds::kCamera}, {}},
            // Named "Directional Light" rather than "Light": `CNA.Light`'s `kind` defaults to
            // Directional, and a row called Light that always made one kind would be a row whose
            // name is a promise the other two kinds break (`plan.md` STUDIO-20001).
            {"light.directional", "Directional Light", {BuiltinComponentIds::kLight}, {}},
            // The other two kinds of the same component (`plan.md` STUDIO-20002, STUDIO-20003).
            // A preset rather than a component of their own, because `CNA.Light` is what the
            // runtime reads and three components mapping onto one would be three ways to write
            // the same scene file.
            {"light.point", "Point Light", {BuiltinComponentIds::kLight},
             {{BuiltinComponentIds::kLight, "kind",
               PropertyValue{PropertyValue::EnumValue{"Point"}}}}},
            {"light.spot", "Spot Light", {BuiltinComponentIds::kLight},
             {{BuiltinComponentIds::kLight, "kind",
               PropertyValue{PropertyValue::EnumValue{"Spot"}}}}},
            {"sprite", "Sprite", {BuiltinComponentIds::kSpriteRenderer}, {}},
            {"model", "Model", {BuiltinComponentIds::kModelRenderer}, {}},
            {"audio", "Audio Source", {BuiltinComponentIds::kAudioSource}, {}},
        };
        return kArchetypes;
    }

    const StudioEntityArchetype* studioFindEntityArchetype(std::string_view id)
    {
        const std::vector<StudioEntityArchetype>& all = studioEntityArchetypes();
        const auto found = std::find_if(all.begin(), all.end(),
            [id](const StudioEntityArchetype& archetype) { return archetype.id == id; });
        return found == all.end() ? nullptr : &*found;
    }

    StudioEntity studioMakeArchetypeEntity(const StudioEntityArchetype& archetype,
                                           const ComponentRegistry& registry)
    {
        StudioEntity entity{Uuid::generate(), archetype.name};

        const auto add = [&](std::string_view typeId) {
            const ComponentDescriptor* descriptor = registry.find(typeId);
            if (descriptor == nullptr) { return; }

            StudioComponent component{std::string{typeId}};
            component.applyDefaults(*descriptor);
            entity.addComponent(std::move(component));
        };

        // The transform first and always: an entity with no transform has no position, cannot be
        // picked in the viewport and cannot be a parent that means anything.
        add(BuiltinComponentIds::kTransform);
        for (const std::string& typeId : archetype.components) { add(typeId); }

        // Then the handful of values that distinguish this kind from its neighbours, applied over
        // the defaults rather than instead of them: a preset says what makes a point light a point
        // light and leaves its colour, intensity and range to the descriptor.
        for (const StudioEntityArchetype::Preset& preset : archetype.presets)
        {
            StudioComponent* target = entity.findComponent(preset.component);
            if (target == nullptr) { continue; }
            target->setProperty(preset.property, preset.value);
        }

        return entity;
    }
}
