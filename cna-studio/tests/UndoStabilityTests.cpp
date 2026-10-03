// SPDX-License-Identifier: MS-PL
/**
 * @file UndoStabilityTests.cpp
 * @brief Undo puts the document back exactly, for every editing path (`plan.md` STUDIO-31004).
 *
 * `tests/CommandTests.cpp` checks each command against the fields it was expected to touch: the
 * entity is back, the name is back, the property is back. That is the right test for "does this
 * command do its job", and it is structurally blind to the failure this row is about — a command
 * that restores what it *knew* it changed and leaves behind something it did not know it changed.
 * Editor-only state, an entity's position in the list, a component's order, a parent link that a
 * second entity happened to carry. Each one is invisible until a user notices their scene is not
 * the scene they had, and by then the undo history is gone.
 *
 * So the property here is stronger and blunter: **undo restores the document byte for byte.**
 *
 *     serialise → execute → serialise → undo → serialise → redo → serialise → undo → serialise
 *
 * with the first, third and fifth equal, and the second and fourth equal. Byte equality is only
 * meaningful because `STUDIO-02037` made the writer deterministic; before that this test could not
 * have been written.
 *
 * Every row also asserts the command **changed something**, so a factory that quietly built an
 * invalid command — the commonest way a case like this goes vacuous — fails rather than passing
 * six ways.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Core/Json.hpp"
#include "CNA/Studio/ProjectCommands.hpp"
#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneCommands.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    const char* const kAlpha = "0cf45f27-2ecd-44a6-8c45-cd8d2122179f";
    const char* const kBeta = "5c1d8e34-0f2b-4a77-9c61-2b8e5f0a41d3";
    const char* const kGamma = "11111111-2222-3333-4444-555555555555";

    /** @brief A scene with a hierarchy, components, editor state and an asset reference. */
    SceneDocument makeScene(const ComponentRegistry& registry)
    {
        SceneDocument scene;
        scene.setName("Level01");

        const auto entity = [&registry](const char* id, const char* name, const char* parent) {
            StudioEntity made{Uuid::parse(id), name};
            if (parent != nullptr) { made.setParentId(Uuid::parse(parent)); }

            StudioComponent transform{BuiltinComponentIds::kTransform};
            transform.applyDefaults(*registry.find(BuiltinComponentIds::kTransform));
            transform.setProperty("position", PropertyValue{StudioVector3{1.5f, -2.0f, 3.25f}});
            made.addComponent(std::move(transform));

            StudioComponent sprite{BuiltinComponentIds::kSpriteRenderer};
            sprite.applyDefaults(*registry.find(BuiltinComponentIds::kSpriteRenderer));
            made.addComponent(std::move(sprite));

            // Editor-only state, which is the half a command that restores "the fields it knows
            // about" is most likely to drop.
            made.setStudioState("expanded", PropertyValue{true});
            made.setStudioState("colour", PropertyValue{std::string{"#ff8800"}});
            return made;
        };

        scene.addEntity(entity(kAlpha, "Alpha", nullptr));
        scene.addEntity(entity(kBeta, "Beta", kAlpha));
        scene.addEntity(entity(kGamma, "Gamma", nullptr));
        return scene;
    }

    /** @brief One editing path, built fresh for each run so the scene is never carried between. */
    struct EditingPath
    {
        const char* name;
        std::function<std::unique_ptr<StudioCommand>(SceneDocument&, ComponentRegistry&)> make;
    };
}

/**
 * @brief **Every scene-editing command undoes to the same bytes and redoes to the same bytes.**
 *
 * The table is the point. A property checked once on one command is a property that holds for one
 * command; checked over every path into the document, it is a property of the editor.
 */
CNA_STUDIO_TEST(EveryEditingPathUndoesToTheDocumentItStartedFrom)
{
    static const EditingPath kPaths[] = {
        {"create an entity",
         [](SceneDocument& scene, ComponentRegistry&) {
             StudioEntity made{Uuid::parse("22222222-3333-4444-5555-666666666666"), "New"};
             return std::make_unique<CreateEntityCommand>(scene, std::move(made));
         }},
        {"delete an entity with children",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<DeleteEntityCommand>(scene, Uuid::parse(kAlpha));
         }},
        {"delete a leaf",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<DeleteEntityCommand>(scene, Uuid::parse(kGamma));
         }},
        {"duplicate an entity",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<DuplicateEntityCommand>(scene, Uuid::parse(kAlpha));
         }},
        {"rename an entity",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<RenameEntityCommand>(scene, Uuid::parse(kBeta), "Renamed");
         }},
        {"disable an entity",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<SetEntityEnabledCommand>(scene, Uuid::parse(kBeta), false);
         }},
        {"lock an entity",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<SetEntityLockedCommand>(scene, Uuid::parse(kBeta), true);
         }},
        {"reparent an entity",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<ReparentEntityCommand>(scene, Uuid::parse(kGamma),
                                                            Uuid::parse(kAlpha));
         }},
        {"reparent an entity to the root",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<ReparentEntityCommand>(scene, Uuid::parse(kBeta), Uuid{});
         }},
        {"set a property",
         [](SceneDocument& scene, ComponentRegistry&) {
             return std::make_unique<SetPropertyCommand>(
                 scene, Uuid::parse(kAlpha), std::string{BuiltinComponentIds::kTransform},
                 "position", PropertyValue{StudioVector3{9.0f, 8.0f, 7.0f}});
         }},
        {"add a component",
         [](SceneDocument& scene, ComponentRegistry& registry) {
             return std::make_unique<AddComponentCommand>(
                 scene, registry, Uuid::parse(kGamma),
                 std::string{BuiltinComponentIds::kCamera});
         }},
        {"remove a component",
         [](SceneDocument& scene, ComponentRegistry& registry) {
             return std::make_unique<RemoveComponentCommand>(
                 scene, registry, Uuid::parse(kBeta),
                 std::string{BuiltinComponentIds::kSpriteRenderer});
         }},
        {"transform several entities at once",
         [](SceneDocument& scene, ComponentRegistry&) {
             std::vector<EntityTransformEdit> edits;
             EntityTransformEdit first;
             first.entityId = Uuid::parse(kAlpha);
             first.position = StudioVector3{4.0f, 5.0f, 6.0f};
             edits.push_back(first);
             EntityTransformEdit second;
             second.entityId = Uuid::parse(kGamma);
             second.scale = StudioVector3{2.0f, 2.0f, 2.0f};
             edits.push_back(second);
             return std::make_unique<TransformEntitiesCommand>(scene, std::move(edits), "drag");
         }},
        {"change the scene environment",
         [](SceneDocument& scene, ComponentRegistry&) {
             SceneEnvironment environment = scene.getEnvironment();
             environment.fogEnabled = !environment.fogEnabled;
             environment.fogStart = 12.5f;
             return std::make_unique<SetSceneEnvironmentCommand>(scene, environment, "fog");
         }},
    };

    for (const EditingPath& path : kPaths)
    {
        ComponentRegistry registry;
        registerBuiltinComponents(registry);

        SceneDocument scene = makeScene(registry);
        const std::string before = Json::write(scene.toJson(), true);

        std::unique_ptr<StudioCommand> command = path.make(scene, registry);
        CNA_STUDIO_EXPECT(command != nullptr);
        if (command == nullptr) { continue; }

        command->execute();
        const std::string after = Json::write(scene.toJson(), true);

        // The command did something. A factory that quietly built an invalid command is the
        // commonest way a table like this goes vacuous, and it would pass every check below.
        if (after == before)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"'"} + path.name
                + "' changed nothing, so everything this case asserts about undoing it is "
                  "vacuous. Fix the command or fix the fixture.");
            continue;
        }

        command->undo();
        const std::string undone = Json::write(scene.toJson(), true);
        if (undone != before)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"undoing '"} + path.name
                + "' did not put the document back. Ctrl+Z that restores most of a change is worse "
                  "than one that restores none of it: the user believes they are where they were "
                  "(plan.md STUDIO-31004).");
            continue;
        }

        // Redo reproduces the edit exactly, which is the half a user reaches for after undoing one
        // step too far.
        command->execute();
        const std::string redone = Json::write(scene.toJson(), true);
        if (redone != after)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"redoing '"} + path.name + "' did not reproduce the edit.");
            continue;
        }

        // And a second cycle, because a command can restore correctly once and not twice -- one
        // that moved rather than copied its saved state has nothing left the second time.
        command->undo();
        CNA_STUDIO_EXPECT_EQ(Json::write(scene.toJson(), true), before);
        command->execute();
        CNA_STUDIO_EXPECT_EQ(Json::write(scene.toJson(), true), after);
        command->undo();
        CNA_STUDIO_EXPECT_EQ(Json::write(scene.toJson(), true), before);
    }
}

/**
 * @brief A composite undoes as one, and in the right order.
 *
 * `studio.entity.group` is a create followed by N reparents, and the order matters in both
 * directions: the create has to come first so the reparents have somewhere to go, and the undo has
 * to run backwards or it deletes the group while its children still point at it.
 */
CNA_STUDIO_TEST(ACompositeUndoesAsOneAndInReverse)
{
    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    SceneDocument scene = makeScene(registry);
    const std::string before = Json::write(scene.toJson(), true);

    const Uuid groupId = Uuid::parse("33333333-4444-5555-6666-777777777777");
    auto batch = std::make_unique<CompositeCommand>("Group 2 entities");
    batch->add(std::make_unique<CreateEntityCommand>(scene, StudioEntity{groupId, "Group"}));
    batch->add(std::make_unique<ReparentEntityCommand>(scene, Uuid::parse(kAlpha), groupId));
    batch->add(std::make_unique<ReparentEntityCommand>(scene, Uuid::parse(kGamma), groupId));

    batch->execute();
    const std::string after = Json::write(scene.toJson(), true);
    CNA_STUDIO_EXPECT(after != before);

    batch->undo();
    CNA_STUDIO_EXPECT_EQ(Json::write(scene.toJson(), true), before);

    batch->execute();
    CNA_STUDIO_EXPECT_EQ(Json::write(scene.toJson(), true), after);

    batch->undo();
    CNA_STUDIO_EXPECT_EQ(Json::write(scene.toJson(), true), before);
}

/**
 * @brief The same property for the project's own commands, over the project's bytes.
 *
 * The project is a document like the scene (D-06), and its three commands write through to disk on
 * every apply — so an undo that restored the model and not the file would leave the two disagreeing
 * until the next save picked one.
 */
CNA_STUDIO_TEST(EveryProjectEditUndoesToTheProjectItStartedFrom)
{
    struct ProjectPath
    {
        const char* name;
        std::function<std::unique_ptr<StudioCommand>(Project&, ComponentRegistry&)> make;
    };

    static const ProjectPath kPaths[] = {
        {"set the grid snap",
         [](Project& project, ComponentRegistry&) {
             return std::make_unique<SetProjectGridSnapCommand>(project, 0.25f);
         }},
        {"replace the layer list",
         [](Project& project, ComponentRegistry& registry) {
             return std::make_unique<SetProjectLayersCommand>(
                 project, registry, std::vector<std::string>{"Default", "Enemies", "UI"});
         }},
        {"add a build target",
         [](Project& project, ComponentRegistry&) {
             std::vector<StudioTargetProfile> profiles = project.getTargetProfiles();
             profiles.push_back(StudioTargetProfile::defaults());
             profiles.back().name = "Second";
             return std::make_unique<SetTargetProfilesCommand>(project, std::move(profiles), 1,
                                                               "Add target");
         }},
        {"select a different build target",
         [](Project& project, ComponentRegistry&) {
             std::vector<StudioTargetProfile> profiles = project.getTargetProfiles();
             profiles.push_back(StudioTargetProfile::defaults());
             profiles.back().name = "Second";
             project.setTargetProfiles(profiles);
             return std::make_unique<SetTargetProfilesCommand>(project, std::move(profiles), 1,
                                                               "Select target");
         }},
    };

    for (const ProjectPath& path : kPaths)
    {
        ComponentRegistry registry;
        registerBuiltinComponents(registry);

        Project project;
        std::unique_ptr<StudioCommand> command = path.make(project, registry);
        CNA_STUDIO_EXPECT(command != nullptr);
        if (command == nullptr) { continue; }

        // Captured *after* the factory, because two of these set the project up first -- the
        // command is the edit, not the arrangement it is made against.
        const std::string before = Json::write(project.toJson(), true);

        command->execute();
        const std::string after = Json::write(project.toJson(), true);
        if (after == before)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"'"} + path.name + "' changed nothing about the project.");
            continue;
        }

        command->undo();
        if (Json::write(project.toJson(), true) != before)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"undoing '"} + path.name + "' did not put the project back.");
            continue;
        }

        command->execute();
        CNA_STUDIO_EXPECT_EQ(Json::write(project.toJson(), true), after);
        command->undo();
        CNA_STUDIO_EXPECT_EQ(Json::write(project.toJson(), true), before);
    }
}
