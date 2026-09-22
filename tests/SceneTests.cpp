// SPDX-License-Identifier: MS-PL
/**
 * @file SceneTests.cpp
 * @brief Tests for the scene document, its invariants, its serialisation and its undo behaviour.
 */

#include "TestHarness.hpp"
#include <unordered_map>
#include <chrono>
#include "CNA/Studio/ShellPanels/StudioOutlinerPanel.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <memory>

#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/EntityArchetypes.hpp"
#include "CNA/Studio/Assets/MaterialCapabilities.hpp"
#include "CNA/Studio/Scene/SceneShadows.hpp"
#include "CNA/Studio/Scene/SceneSky.hpp"
#include "CNA/Studio/Scene/StudioCamera3D.hpp"
#include "CNA/Studio/Scene/SceneLighting.hpp"
#include "CNA/Studio/Scene/SceneModels.hpp"
#include "CNA/Studio/Scene/SceneSprites3D.hpp"
#include "CNA/Studio/Scene/EntityJson.hpp"
#include "CNA/Studio/Scene/SceneLock.hpp"
#include "CNA/Studio/Scene/SceneWireframe.hpp"
#include "CNA/Studio/Scene/TransformGizmos3D.hpp"
#include "CNA/Studio/Scene/SceneCommands.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/Scene/PrefabCommands.hpp"
#include "CNA/Studio/Scene/PrefabDocument.hpp"
#include "CNA/Studio/Scene/SceneValidation.hpp"
#include "CNA/Studio/Scene/SpriteAnimation.hpp"
#include "CNA/Studio/Scene/Tilemap.hpp"
#include "CNA/Studio/Project/Project.hpp"

using namespace CNA::Studio;

namespace
{
    /** @brief Returns a registry holding the built-in component descriptors. */
    ComponentRegistry makeRegistry()
    {
        ComponentRegistry registry;
        registerBuiltinComponents(registry);
        return registry;
    }

    /** @brief Builds an entity with a Transform positioned at (@p x, @p y, 0). */
    StudioEntity makeEntity(const ComponentRegistry& registry, std::string name, float x, float y)
    {
        StudioEntity entity{Uuid::generate(), std::move(name)};
        StudioComponent transform{BuiltinComponentIds::kTransform};
        transform.applyDefaults(*registry.find(BuiltinComponentIds::kTransform));
        transform.setProperty("position", PropertyValue{StudioVector3{x, y, 0.0f}});
        entity.addComponent(std::move(transform));
        return entity;
    }

    /**
     * @brief `makeEntity` plus a sprite, so the entity has geometry the wireframe can box.
     *
     * Needed once `STUDIO-11009` gave a transform-only entity a marker badge instead of a box: a
     * case about *boxes* has to be built out of entities that actually have something to box.
     */
    StudioEntity makeSpriteEntity(const ComponentRegistry& registry, std::string name, float x,
                                  float y)
    {
        StudioEntity entity = makeEntity(registry, std::move(name), x, y);
        StudioComponent renderer{BuiltinComponentIds::kSpriteRenderer};
        renderer.applyDefaults(*registry.find(BuiltinComponentIds::kSpriteRenderer));
        entity.addComponent(std::move(renderer));
        return entity;
    }

    /** @brief The `GizmoAxis3D` for ring or arm @p index, which the library keeps to itself. */
    GizmoAxis3D axis3DForTest(std::size_t index)
    {
        if (index == 0) { return GizmoAxis3D::X; }
        if (index == 1) { return GizmoAxis3D::Y; }
        return GizmoAxis3D::Z;
    }

    /** @brief Returns a unique scratch directory for a test that touches the filesystem. */
    std::filesystem::path makeScratchDirectory(const std::string& name)
    {
        const std::filesystem::path directory =
            std::filesystem::temp_directory_path() / ("cna-studio-tests-" + name + "-" + Uuid::generate().toString());
        std::filesystem::create_directories(directory);
        return directory;
    }
}

/**
 * The six standard views point the camera along the axis they are named for (`plan.md` STUDIO-11004).
 *
 * Named for where the camera *is*, which is the opposite of how the view direction reads: the Front
 * view looks backwards along -Z from in front of the subject. Asserted on the eye rather than on
 * the yaw, because the yaw is the implementation and where the camera ends up is the promise.
 */
CNA_STUDIO_TEST(EachStandardViewPutsTheCameraOnTheAxisItIsNamedFor)
{
    StudioCamera3D camera;
    camera.setPivot(StudioVector3{10.0f, 20.0f, 30.0f});
    camera.setDistance(100.0f);

    struct Expectation
    {
        StudioStandardView view;
        StudioVector3 offsetFromPivot;
    };

    // The editor's world is Y-down, so "above" the pivot is -Y and the Top view's eye sits there.
    const std::vector<Expectation> expectations{
        {StudioStandardView::Front, StudioVector3{0.0f, 0.0f, 100.0f}},
        {StudioStandardView::Back, StudioVector3{0.0f, 0.0f, -100.0f}},
        {StudioStandardView::Right, StudioVector3{100.0f, 0.0f, 0.0f}},
        {StudioStandardView::Left, StudioVector3{-100.0f, 0.0f, 0.0f}},
        {StudioStandardView::Top, StudioVector3{0.0f, -100.0f, 0.0f}},
        {StudioStandardView::Bottom, StudioVector3{0.0f, 100.0f, 0.0f}},
    };

    for (const Expectation& expectation : expectations)
    {
        studioApplyStandardView(camera, expectation.view);

        const StudioVector3 eye = camera.getEye();
        const StudioVector3 offset = subtract(eye, camera.getPivot());

        if (std::abs(offset.x - expectation.offsetFromPivot.x) > 0.05f
            || std::abs(offset.y - expectation.offsetFromPivot.y) > 0.05f
            || std::abs(offset.z - expectation.offsetFromPivot.z) > 0.05f)
        {
            CnaStudioTest::reportFailure(
                __FILE__, __LINE__,
                std::string{"the "} + toString(expectation.view) + " view put the eye at an offset of ("
                    + std::to_string(offset.x) + ", " + std::to_string(offset.y) + ", "
                    + std::to_string(offset.z) + ") from the pivot.");
        }

        // Orientation only. The pivot and the distance are what the user framed, and a standard
        // view that also moved them would be a navigation wearing the name of a rotation.
        CNA_STUDIO_EXPECT(std::abs(camera.getPivot().x - 10.0f) < 1e-4f);
        CNA_STUDIO_EXPECT(std::abs(camera.getPivot().z - 30.0f) < 1e-4f);
        CNA_STUDIO_EXPECT(std::abs(camera.getDistance() - 100.0f) < 1e-3f);

        // And the camera really is looking at the pivot from there, which the eye alone does not
        // say: an eye in the right place looking the wrong way is a view of nothing.
        const StudioVector3 forward = camera.getForward();
        const StudioVector3 toPivot = normalize(subtract(camera.getPivot(), eye));
        CNA_STUDIO_EXPECT(dot(forward, toPivot) > 0.999f);
    }
}

CNA_STUDIO_TEST(AStandardViewIsTheSamePictureWhereverTheUserWasOrbiting)
{
    // The point of a standard view is that pressing it twice from different places gives one
    // picture. Top and Bottom are where that could go wrong, because the other four fix the yaw
    // anyway and these two could have left it wherever the orbit had wandered.
    StudioCamera3D first;
    first.setYaw(2.1f);
    first.setPitch(-0.8f);
    studioApplyStandardView(first, StudioStandardView::Top);

    StudioCamera3D second;
    second.setYaw(-0.4f);
    second.setPitch(1.2f);
    studioApplyStandardView(second, StudioStandardView::Top);

    CNA_STUDIO_EXPECT(std::abs(first.getYaw() - second.getYaw()) < 1e-5f);
    CNA_STUDIO_EXPECT(std::abs(first.getPitch() - second.getPitch()) < 1e-5f);
}

CNA_STUDIO_TEST(LookingStraightDownIsAnOrdinaryPointRatherThanASingularity)
{
    // The camera's basis used to be `normalize(cross(forward, Y))`, which is the zero vector when
    // forward *is* Y -- so the pitch was clamped one degree short of vertical to keep it away from
    // there, and a Top view was one degree short of a top view. The basis is now written so the
    // pole is an ordinary point (`plan.md` STUDIO-11004), and these are the assertions that say so.
    StudioCamera3D camera;
    camera.setPivot(StudioVector3{1.0f, 2.0f, 3.0f});
    camera.setDistance(50.0f);
    studioApplyStandardView(camera, StudioStandardView::Top);

    CNA_STUDIO_EXPECT(std::abs(camera.getPitch() - StudioCamera3D::kMaxPitchRadians) < 1e-6f);

    const StudioVector3 right = camera.getRight();
    const StudioVector3 up = camera.getUp();
    const StudioVector3 forward = camera.getForward();

    // Finite, unit-length and mutually perpendicular -- none of which a NaN basis manages.
    for (const StudioVector3& axis : {right, up, forward})
    {
        CNA_STUDIO_EXPECT(std::isfinite(axis.x) && std::isfinite(axis.y) && std::isfinite(axis.z));
        CNA_STUDIO_EXPECT(std::abs(length(axis) - 1.0f) < 1e-4f);
    }
    CNA_STUDIO_EXPECT(std::abs(dot(right, up)) < 1e-4f);
    CNA_STUDIO_EXPECT(std::abs(dot(right, forward)) < 1e-4f);
    CNA_STUDIO_EXPECT(std::abs(dot(up, forward)) < 1e-4f);

    // The view matrix is what actually degenerated, and a projection through it has to land
    // somewhere real: the pivot is dead centre when the camera is pointed at it.
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const std::optional<StudioVector2> centre = camera.worldToScreen(camera.getPivot());
    CNA_STUDIO_EXPECT(centre.has_value());
    if (centre)
    {
        CNA_STUDIO_EXPECT(std::abs(centre->x - 400.0f) < 1.0f);
        CNA_STUDIO_EXPECT(std::abs(centre->y - 300.0f) < 1.0f);
    }

    // And the right vector agrees with the limit approached from just below the pole, which is
    // what makes the pole continuous rather than merely defined.
    StudioCamera3D nearly;
    nearly.setPitch(StudioCamera3D::kMaxPitchRadians - 0.0005f);
    CNA_STUDIO_EXPECT(dot(nearly.getRight(), camera.getRight()) > 0.9999f);
}

CNA_STUDIO_TEST(BuiltinComponentsAreRegistered)
{
    const ComponentRegistry registry = makeRegistry();
    CNA_STUDIO_EXPECT(registry.contains(BuiltinComponentIds::kTransform));
    CNA_STUDIO_EXPECT(registry.contains(BuiltinComponentIds::kSpriteRenderer));
    CNA_STUDIO_EXPECT(registry.contains(BuiltinComponentIds::kCamera));
    CNA_STUDIO_EXPECT(registry.contains(BuiltinComponentIds::kAudioSource));

    const ComponentDescriptor* transform = registry.find(BuiltinComponentIds::kTransform);
    CNA_STUDIO_EXPECT(transform->required);
    CNA_STUDIO_EXPECT(transform->unique);

    // Scale must default to 1, not 0: a zero-scaled entity is invisible, and "my sprite does not
    // appear" is the least debuggable possible first experience.
    const PropertyDescriptor* scale = transform->findProperty("scale");
    CNA_STUDIO_EXPECT(scale != nullptr);
    CNA_STUDIO_EXPECT_EQ(scale->defaultValue.get<StudioVector3>().x, 1.0f);
}

CNA_STUDIO_TEST(SceneAddsAndFindsEntities)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid id = scene.addEntity(makeEntity(registry, "Player", 10.0f, 20.0f));
    CNA_STUDIO_EXPECT(id.isValid());
    CNA_STUDIO_EXPECT_EQ(scene.getEntityCount(), std::size_t{1});

    const StudioEntity* entity = scene.findEntity(id);
    CNA_STUDIO_EXPECT(entity != nullptr);
    CNA_STUDIO_EXPECT_EQ(entity->getName(), std::string{"Player"});
    CNA_STUDIO_EXPECT(scene.findEntity(Uuid::generate()) == nullptr);
}

CNA_STUDIO_TEST(SceneRejectsDuplicateEntityIds)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Player", 0.0f, 0.0f);
    const Uuid id = scene.addEntity(entity);
    CNA_STUDIO_EXPECT(id.isValid());
    CNA_STUDIO_EXPECT(!scene.addEntity(entity).isValid());
    CNA_STUDIO_EXPECT_EQ(scene.getEntityCount(), std::size_t{1});
}

CNA_STUDIO_TEST(SceneMaintainsHierarchy)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid parent = scene.addEntity(makeEntity(registry, "Parent", 0.0f, 0.0f));
    const Uuid child = scene.addEntity(makeEntity(registry, "Child", 0.0f, 0.0f));

    CNA_STUDIO_EXPECT_EQ(scene.getRootEntities().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(scene.reparentEntity(child, parent));
    CNA_STUDIO_EXPECT_EQ(scene.getRootEntities().size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(scene.getChildren(parent).size(), std::size_t{1});
    CNA_STUDIO_EXPECT(scene.isAncestorOf(parent, child));
    CNA_STUDIO_EXPECT(!scene.isAncestorOf(child, parent));
}

CNA_STUDIO_TEST(SceneRejectsParentCycles)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid a = scene.addEntity(makeEntity(registry, "A", 0.0f, 0.0f));
    const Uuid b = scene.addEntity(makeEntity(registry, "B", 0.0f, 0.0f));
    const Uuid c = scene.addEntity(makeEntity(registry, "C", 0.0f, 0.0f));

    CNA_STUDIO_EXPECT(scene.reparentEntity(b, a));
    CNA_STUDIO_EXPECT(scene.reparentEntity(c, b));

    // A graph with a cycle has no roots and makes every hierarchy walk infinite, so these must
    // be refused outright rather than detected later.
    CNA_STUDIO_EXPECT(!scene.reparentEntity(a, c));
    CNA_STUDIO_EXPECT(!scene.reparentEntity(a, a));
    CNA_STUDIO_EXPECT_EQ(scene.getRootEntities().size(), std::size_t{1});
}

CNA_STUDIO_TEST(SceneDeletesSubtreesRecursively)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid root = scene.addEntity(makeEntity(registry, "Root", 0.0f, 0.0f));
    const Uuid middle = scene.addEntity(makeEntity(registry, "Middle", 0.0f, 0.0f));
    const Uuid leaf = scene.addEntity(makeEntity(registry, "Leaf", 0.0f, 0.0f));
    scene.reparentEntity(middle, root);
    scene.reparentEntity(leaf, middle);

    const std::vector<StudioEntity> removed = scene.removeEntityRecursive(root);
    CNA_STUDIO_EXPECT_EQ(removed.size(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(scene.getEntityCount(), std::size_t{0});

    // Parents first, so an undo can re-add without ever pointing at a missing parent.
    CNA_STUDIO_EXPECT(removed.front().getId() == root);
}

CNA_STUDIO_TEST(SceneRoundTripsThroughJson)
{
    const ComponentRegistry registry = makeRegistry();

    SceneDocument original;
    original.setName("Level01");
    const Uuid parent = original.addEntity(makeEntity(registry, "Parent", 100.0f, 220.0f));

    StudioEntity sprite = makeEntity(registry, "Player", 5.0f, 6.0f);
    StudioComponent renderer{BuiltinComponentIds::kSpriteRenderer};
    renderer.applyDefaults(*registry.find(BuiltinComponentIds::kSpriteRenderer));
    const Uuid textureId = Uuid::generate();
    renderer.setProperty("texture", PropertyValue{PropertyValue::AssetReference{textureId}});
    renderer.setProperty("tint", PropertyValue{StudioColor{255, 0, 0, 128}});
    renderer.setProperty("layerDepth", PropertyValue{0.25f});
    sprite.addComponent(std::move(renderer));
    sprite.setStudioState("expanded", PropertyValue{true});
    const Uuid spriteId = original.addEntity(std::move(sprite));
    original.reparentEntity(spriteId, parent);

    SceneDocument restored;
    const SceneLoadResult result = restored.loadFromJson(original.toJson(), registry);
    CNA_STUDIO_EXPECT(result.succeeded);
    CNA_STUDIO_EXPECT_EQ(result.warnings.size(), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(restored.getName(), std::string{"Level01"});
    CNA_STUDIO_EXPECT(restored.getSceneId() == original.getSceneId());
    CNA_STUDIO_EXPECT_EQ(restored.getEntityCount(), std::size_t{2});

    const StudioEntity* restoredSprite = restored.findEntity(spriteId);
    CNA_STUDIO_EXPECT(restoredSprite != nullptr);
    CNA_STUDIO_EXPECT(restoredSprite->getParentId() == parent);

    const StudioComponent* restoredRenderer = restoredSprite->findComponent(BuiltinComponentIds::kSpriteRenderer);
    CNA_STUDIO_EXPECT(restoredRenderer != nullptr);
    CNA_STUDIO_EXPECT(restoredRenderer->getProperty("texture").get<PropertyValue::AssetReference>().id == textureId);
    CNA_STUDIO_EXPECT(restoredRenderer->getProperty("tint").get<StudioColor>() == (StudioColor{255, 0, 0, 128}));
    CNA_STUDIO_EXPECT_EQ(restoredRenderer->getProperty("layerDepth").get<float>(), 0.25f);
    CNA_STUDIO_EXPECT(restoredSprite->getStudioState().count("expanded") > 0);
}

CNA_STUDIO_TEST(SceneRoundTripsThroughAFile)
{
    const ComponentRegistry registry = makeRegistry();
    const std::filesystem::path directory = makeScratchDirectory("scene");
    const std::string path = (directory / "Level01.cnascene").generic_string();

    SceneDocument original;
    original.setName("Level01");
    const Uuid id = original.addEntity(makeEntity(registry, "Player", 1.0f, 2.0f));

    std::string errorMessage;
    CNA_STUDIO_EXPECT(original.saveToFile(path, &errorMessage));
    CNA_STUDIO_EXPECT_EQ(errorMessage, std::string{});

    SceneDocument restored;
    const SceneLoadResult result = restored.loadFromFile(path, registry);
    CNA_STUDIO_EXPECT(result.succeeded);
    CNA_STUDIO_EXPECT(restored.findEntity(id) != nullptr);

    std::filesystem::remove_all(directory);
}

CNA_STUDIO_TEST(SceneRejectsAFutureFormatVersion)
{
    const ComponentRegistry registry = makeRegistry();

    JsonValue json = JsonValue::makeObject();
    json.set("formatVersion", JsonValue{SceneDocument::kFormatVersion + 1});
    json.set("name", JsonValue{"FromTheFuture"});

    SceneDocument scene;
    const SceneLoadResult result = scene.loadFromJson(json, registry);
    CNA_STUDIO_EXPECT(!result.succeeded);
    CNA_STUDIO_EXPECT(result.errorMessage.find("newer") != std::string::npos);
}

CNA_STUDIO_TEST(ScenePreservesUnknownComponentTypes)
{
    // A scene using a component from a plugin that failed to load must still round-trip. Dropping
    // the data would turn "the plugin is missing" into "the plugin's data is gone".
    ComponentRegistry registry = makeRegistry();

    JsonValue componentJson = JsonValue::makeObject();
    componentJson.set("spawnCount", JsonValue{7});
    componentJson.set("label", JsonValue{"wave-1"});

    JsonValue componentsJson = JsonValue::makeObject();
    componentsJson.set("Mc3.SpawnPoint", std::move(componentJson));

    JsonValue entityJson = JsonValue::makeObject();
    entityJson.set("id", JsonValue{Uuid::generate().toString()});
    entityJson.set("name", JsonValue{"Spawner"});
    entityJson.set("components", std::move(componentsJson));

    JsonValue entitiesJson = JsonValue::makeArray();
    entitiesJson.append(std::move(entityJson));

    JsonValue sceneJson = JsonValue::makeObject();
    sceneJson.set("formatVersion", JsonValue{SceneDocument::kFormatVersion});
    sceneJson.set("sceneId", JsonValue{Uuid::generate().toString()});
    sceneJson.set("name", JsonValue{"WithPlugin"});
    sceneJson.set("entities", std::move(entitiesJson));

    SceneDocument scene;
    const SceneLoadResult result = scene.loadFromJson(sceneJson, registry);
    CNA_STUDIO_EXPECT(result.succeeded);
    CNA_STUDIO_EXPECT_EQ(result.warnings.size(), std::size_t{1});

    const StudioEntity& entity = scene.getEntities().front();
    const StudioComponent* component = entity.findComponent("Mc3.SpawnPoint");
    CNA_STUDIO_EXPECT(component != nullptr);
    CNA_STUDIO_EXPECT_EQ(component->getProperty("label").get<std::string>(), std::string{"wave-1"});

    // And it must survive a save/load cycle unchanged.
    SceneDocument reloaded;
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(scene.toJson(), registry).succeeded);
    CNA_STUDIO_EXPECT(reloaded.getEntities().front().findComponent("Mc3.SpawnPoint") != nullptr);
}

CNA_STUDIO_TEST(SceneRepairsDanglingParentReferences)
{
    ComponentRegistry registry = makeRegistry();

    JsonValue entityJson = JsonValue::makeObject();
    entityJson.set("id", JsonValue{Uuid::generate().toString()});
    entityJson.set("name", JsonValue{"Orphan"});
    entityJson.set("parent", JsonValue{Uuid::generate().toString()});
    entityJson.set("components", JsonValue::makeObject());

    JsonValue entitiesJson = JsonValue::makeArray();
    entitiesJson.append(std::move(entityJson));

    JsonValue sceneJson = JsonValue::makeObject();
    sceneJson.set("formatVersion", JsonValue{SceneDocument::kFormatVersion});
    sceneJson.set("sceneId", JsonValue{Uuid::generate().toString()});
    sceneJson.set("entities", std::move(entitiesJson));

    SceneDocument scene;
    const SceneLoadResult result = scene.loadFromJson(sceneJson, registry);

    // A scene broken by a bad merge is exactly the scene a user needs the editor to open.
    CNA_STUDIO_EXPECT(result.succeeded);
    CNA_STUDIO_EXPECT_EQ(result.warnings.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(scene.getRootEntities().size(), std::size_t{1});
}

// --------------------------------------------------------------------------------------------
// Scene validation (plan.md ED-310)
//
// Every rule describes a state the editor allows the user to reach, so each test asserts both
// halves: that the offending scene is reported, and that the nearest legitimate scene is not.
// A validator that cries wolf is turned off, and then it catches nothing at all.
// --------------------------------------------------------------------------------------------

namespace
{
    /** @brief Adds a component of @p typeId with its declared defaults filled in. */
    StudioComponent& addComponentWithDefaults(StudioEntity& entity,
                                              const ComponentRegistry& registry,
                                              const std::string& typeId)
    {
        StudioComponent component{typeId};
        if (const ComponentDescriptor* descriptor = registry.find(typeId))
        {
            component.applyDefaults(*descriptor);
        }
        return entity.addComponent(std::move(component));
    }

    /** @brief Returns how many issues carry @p ruleId. */
    std::size_t countRule(const std::vector<SceneIssue>& issues, const std::string& ruleId)
    {
        return static_cast<std::size_t>(
            std::count_if(issues.begin(), issues.end(),
                          [&](const SceneIssue& issue) { return issue.ruleId == ruleId; }));
    }
}

CNA_STUDIO_TEST(AnEmptySceneReportsNothing)
{
    const ComponentRegistry registry = makeRegistry();
    const SceneDocument scene;

    CNA_STUDIO_EXPECT(validateScene(scene, registry).empty());
}

CNA_STUDIO_TEST(TwoPrimaryCamerasAreAnError)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity first = makeEntity(registry, "Main Camera", 0.0f, 0.0f);
    addComponentWithDefaults(first, registry, BuiltinComponentIds::kCamera);
    scene.addEntity(std::move(first));

    StudioEntity second = makeEntity(registry, "Cutscene Camera", 0.0f, 0.0f);
    addComponentWithDefaults(second, registry, BuiltinComponentIds::kCamera);
    scene.addEntity(std::move(second));

    const std::vector<SceneIssue> issues = validateScene(scene, registry);

    // One issue per offending camera, so that either row selects a real entity.
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "duplicate-primary-camera"), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(countIssues(issues, SceneIssue::Severity::Error), std::size_t{2});

    // And the ordinary case -- one camera, marked primary -- says nothing at all.
    SceneDocument single;
    StudioEntity only = makeEntity(registry, "Main Camera", 0.0f, 0.0f);
    addComponentWithDefaults(only, registry, BuiltinComponentIds::kCamera);
    single.addEntity(std::move(only));

    CNA_STUDIO_EXPECT(validateScene(single, registry).empty());
}

CNA_STUDIO_TEST(SwitchingACameraOffResolvesThePrimaryConflict)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity first = makeEntity(registry, "Main Camera", 0.0f, 0.0f);
    addComponentWithDefaults(first, registry, BuiltinComponentIds::kCamera);
    scene.addEntity(std::move(first));

    StudioEntity second = makeEntity(registry, "Cutscene Camera", 0.0f, 0.0f);
    addComponentWithDefaults(second, registry, BuiltinComponentIds::kCamera);
    // Switching the entity off is how a person swaps cameras, so it has to count as a resolution.
    second.setEnabled(false);
    scene.addEntity(std::move(second));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(scene, registry), "duplicate-primary-camera"),
                         std::size_t{0});
}

CNA_STUDIO_TEST(ACameraUnderADisabledParentDoesNotCompete)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity primary = makeEntity(registry, "Main Camera", 0.0f, 0.0f);
    addComponentWithDefaults(primary, registry, BuiltinComponentIds::kCamera);
    scene.addEntity(std::move(primary));

    StudioEntity group = makeEntity(registry, "Cutscene", 0.0f, 0.0f);
    group.setEnabled(false);
    const Uuid groupId = scene.addEntity(std::move(group));

    StudioEntity child = makeEntity(registry, "Cutscene Camera", 0.0f, 0.0f);
    addComponentWithDefaults(child, registry, BuiltinComponentIds::kCamera);
    const Uuid childId = scene.addEntity(std::move(child));
    CNA_STUDIO_EXPECT(scene.reparentEntity(childId, groupId));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(scene, registry), "duplicate-primary-camera"),
                         std::size_t{0});
}

CNA_STUDIO_TEST(ASceneWhoseCamerasAreAllSecondaryIsReported)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Camera", 0.0f, 0.0f);
    StudioComponent& camera = addComponentWithDefaults(entity, registry, BuiltinComponentIds::kCamera);
    camera.setProperty("isPrimary", PropertyValue{false});
    scene.addEntity(std::move(entity));

    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "no-primary-camera"), std::size_t{1});

    // Scene-wide: there is no offending entity, and blaming one would send the user to the
    // wrong row.
    for (const SceneIssue& issue : issues)
    {
        if (issue.ruleId == "no-primary-camera") { CNA_STUDIO_EXPECT(!issue.entityId.isValid()); }
    }

    // A scene with no cameras at all is not reported: it is a fragment, not a broken level.
    SceneDocument empty;
    empty.addEntity(makeEntity(registry, "Group", 0.0f, 0.0f));
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(empty, registry), "no-primary-camera"), std::size_t{0});
}

CNA_STUDIO_TEST(InvertedCameraPlanesAreAnError)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Camera", 0.0f, 0.0f);
    StudioComponent& camera = addComponentWithDefaults(entity, registry, BuiltinComponentIds::kCamera);
    camera.setProperty("nearPlane", PropertyValue{1000.0f});
    camera.setProperty("farPlane", PropertyValue{0.1f});
    scene.addEntity(std::move(entity));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(scene, registry), "camera-planes-inverted"),
                         std::size_t{1});
}

CNA_STUDIO_TEST(AZeroScaleIsReportedOnAnyAxis)
{
    const ComponentRegistry registry = makeRegistry();

    for (int axis = 0; axis < 3; ++axis)
    {
        SceneDocument scene;
        StudioEntity entity = makeEntity(registry, "Player", 0.0f, 0.0f);
        addComponentWithDefaults(entity, registry, BuiltinComponentIds::kSpriteRenderer);

        StudioVector3 scale{1.0f, 1.0f, 1.0f};
        (axis == 0 ? scale.x : axis == 1 ? scale.y : scale.z) = 0.0f;
        entity.findComponent(BuiltinComponentIds::kTransform)->setProperty("scale", PropertyValue{scale});

        scene.addEntity(std::move(entity));

        CNA_STUDIO_EXPECT_EQ(countRule(validateScene(scene, registry), "zero-scale"), std::size_t{1});
    }

    // A negative scale is a mirror, not a mistake, and must not be reported.
    SceneDocument mirrored;
    StudioEntity entity = makeEntity(registry, "Player", 0.0f, 0.0f);
    addComponentWithDefaults(entity, registry, BuiltinComponentIds::kSpriteRenderer);
    entity.findComponent(BuiltinComponentIds::kTransform)
        ->setProperty("scale", PropertyValue{StudioVector3{-1.0f, 1.0f, 1.0f}});
    mirrored.addEntity(std::move(entity));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(mirrored, registry), "zero-scale"), std::size_t{0});
}

CNA_STUDIO_TEST(AnEntityThatDoesNothingIsReportedButAGroupIsNot)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    // A transform and nothing else, with no children: it occupies a row in the hierarchy and
    // does not exist as far as the game is concerned.
    scene.addEntity(makeEntity(registry, "Leftover", 0.0f, 0.0f));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(scene, registry), "empty-entity"), std::size_t{1});

    // The same entity with a child is a group, which is the ordinary way to move a set of things
    // together, and must stay silent.
    SceneDocument grouped;
    const Uuid parent = grouped.addEntity(makeEntity(registry, "Enemies", 0.0f, 0.0f));

    StudioEntity child = makeEntity(registry, "Enemy", 0.0f, 0.0f);
    addComponentWithDefaults(child, registry, BuiltinComponentIds::kSpriteRenderer);
    const Uuid childId = grouped.addEntity(std::move(child));
    CNA_STUDIO_EXPECT(grouped.reparentEntity(childId, parent));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(grouped, registry), "empty-entity"), std::size_t{0});
}

CNA_STUDIO_TEST(ASpriteWithNoTextureIsReported)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Player", 0.0f, 0.0f);
    addComponentWithDefaults(entity, registry, BuiltinComponentIds::kSpriteRenderer);
    scene.addEntity(std::move(entity));

    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "sprite-without-texture"), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(countIssues(issues, SceneIssue::Severity::Error), std::size_t{0});

    // Once it points at something, the rule stops firing -- whether that id resolves is the
    // missing-reference report's question, not this one's.
    SceneDocument textured;
    StudioEntity sprite = makeEntity(registry, "Player", 0.0f, 0.0f);
    StudioComponent& renderer =
        addComponentWithDefaults(sprite, registry, BuiltinComponentIds::kSpriteRenderer);
    renderer.setProperty("texture", PropertyValue{PropertyValue::AssetReference{Uuid::generate()}});
    textured.addEntity(std::move(sprite));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(textured, registry), "sprite-without-texture"),
                         std::size_t{0});
}

CNA_STUDIO_TEST(ATilemapWithNoTileSizeIsReported)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Level", 0.0f, 0.0f);
    StudioComponent& tilemap = addComponentWithDefaults(entity, registry, BuiltinComponentIds::kTilemap);
    tilemap.setProperty(TilemapKeys::kTileWidth, PropertyValue{std::int64_t{0}});
    scene.addEntity(std::move(entity));

    // A tile size of zero draws nothing and swallows every brush stroke, neither of which says
    // why. That is the whole reason this rule exists.
    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "tilemap-without-tile-size"), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(countIssues(issues, SceneIssue::Severity::Error), std::size_t{0});

    // A tilemap with a real tile size is not reported, whether or not it has any tiles in it yet.
    SceneDocument sized;
    StudioEntity ok = makeEntity(registry, "Level", 0.0f, 0.0f);
    addComponentWithDefaults(ok, registry, BuiltinComponentIds::kTilemap);
    sized.addEntity(std::move(ok));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(sized, registry), "tilemap-without-tile-size"),
                         std::size_t{0});
}

CNA_STUDIO_TEST(AnAnimationWithNoSheetOrNoFramesIsReported)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Hero", 0.0f, 0.0f);
    addComponentWithDefaults(entity, registry, BuiltinComponentIds::kSpriteRenderer);
    addComponentWithDefaults(entity, registry, BuiltinComponentIds::kSpriteAnimation);
    scene.addEntity(std::move(entity));

    // Both halves fire on a freshly added component, and each is worth its own line: a sheet with
    // no frames plays nothing, and frames with no sheet leave the sprite drawing a placeholder --
    // which looks exactly like a broken asset reference and is a different problem.
    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "animation-without-sheet"), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "animation-without-frames"), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(countIssues(issues, SceneIssue::Severity::Error), std::size_t{0});

    SceneDocument ready;
    StudioEntity animated = makeEntity(registry, "Hero", 0.0f, 0.0f);
    addComponentWithDefaults(animated, registry, BuiltinComponentIds::kSpriteRenderer);
    StudioComponent& animation =
        addComponentWithDefaults(animated, registry, BuiltinComponentIds::kSpriteAnimation);
    animation.setProperty(SpriteAnimationKeys::kSheet,
                          PropertyValue{PropertyValue::AssetReference{Uuid::generate()}});

    PropertyValue::ListValue frames;
    frames.items.push_back(PropertyValue{std::int64_t{0}});
    animation.setProperty(SpriteAnimationKeys::kFrames, PropertyValue{frames});
    ready.addEntity(std::move(animated));

    const std::vector<SceneIssue> readyIssues = validateScene(ready, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(readyIssues, "animation-without-sheet"), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(countRule(readyIssues, "animation-without-frames"), std::size_t{0});
}

CNA_STUDIO_TEST(AnUnregisteredComponentTypeIsReportedRatherThanIgnored)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Spawner", 0.0f, 0.0f);
    entity.addComponent(StudioComponent{"Mc3.SpawnPoint"});
    scene.addEntity(std::move(entity));

    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "unknown-component-type"), std::size_t{1});

    // And it counts as doing something: the editor cannot see what it does, which is not the
    // same as knowing that it does nothing.
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "empty-entity"), std::size_t{0});
}

CNA_STUDIO_TEST(AnEntityWithoutATransformIsAnError)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    // Reachable from a hand-edited file, or from a build older than the Transform requirement.
    scene.addEntity(StudioEntity{Uuid::generate(), "Ghost"});

    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "missing-required-component"), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(countIssues(issues, SceneIssue::Severity::Error), std::size_t{1});
}

CNA_STUDIO_TEST(ASecondUniqueComponentIsAnError)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Player", 0.0f, 0.0f);
    addComponentWithDefaults(entity, registry, BuiltinComponentIds::kTransform);
    scene.addEntity(std::move(entity));

    const std::vector<SceneIssue> issues = validateScene(scene, registry);

    // Reported once for the pair, not once per instance: the user has one problem to fix.
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "duplicate-component"), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(issues.front().componentTypeId, std::string{BuiltinComponentIds::kTransform});
}

CNA_STUDIO_TEST(IssuesComeBackInDocumentOrder)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    scene.addEntity(makeEntity(registry, "First", 0.0f, 0.0f));
    scene.addEntity(makeEntity(registry, "Second", 0.0f, 0.0f));
    scene.addEntity(makeEntity(registry, "Third", 0.0f, 0.0f));

    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(issues.size(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(issues[0].entityName, std::string{"First"});
    CNA_STUDIO_EXPECT_EQ(issues[1].entityName, std::string{"Second"});
    CNA_STUDIO_EXPECT_EQ(issues[2].entityName, std::string{"Third"});

    // Pure: the same document must produce the same report, or it cannot be diffed or asserted on.
    CNA_STUDIO_EXPECT(validateScene(scene, registry).size() == issues.size());
}

CNA_STUDIO_TEST(AnEntityLeftOnARenamedLayerIsReportedNotRewritten)
{
    ComponentRegistry registry = makeRegistry();
    applyProjectLayers(registry, {"Background", "Default"});

    SceneDocument scene;
    StudioEntity entity = makeEntity(registry, "Backdrop", 0.0f, 0.0f);
    addComponentWithDefaults(entity, registry, BuiltinComponentIds::kSpriteRenderer);
    StudioComponent& layer = addComponentWithDefaults(entity, registry, BuiltinComponentIds::kLayer);
    layer.setProperty("layer", PropertyValue{PropertyValue::EnumValue{"Background"}});
    const Uuid entityId = scene.addEntity(std::move(entity));

    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(scene, registry), "unknown-enum-value"), std::size_t{0});

    // Rename the layer out from under it, which is exactly what editing the project's layer list
    // does. The stored value is kept: which of the remaining layers the user meant is their
    // decision, not the editor's.
    applyProjectLayers(registry, {"Backdrop", "Default"});

    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "unknown-enum-value"), std::size_t{1});

    const StudioComponent* stored = scene.findEntity(entityId)->findComponent(BuiltinComponentIds::kLayer);
    CNA_STUDIO_EXPECT_EQ(stored->getProperty("layer").get<PropertyValue::EnumValue>().name,
                         std::string{"Background"});

    // And it is a warning, not an error: the scene still runs and nothing was lost.
    CNA_STUDIO_EXPECT_EQ(countIssues(issues, SceneIssue::Severity::Error), std::size_t{0});
}

CNA_STUDIO_TEST(TagsAreAListOnTheirOwnComponent)
{
    const ComponentRegistry registry = makeRegistry();

    // Its own component rather than a field on StudioEntity: a tag is a game concept and the
    // entity type is deliberately not one (D-04).
    const ComponentDescriptor* descriptor = registry.find(BuiltinComponentIds::kTags);
    CNA_STUDIO_EXPECT(descriptor != nullptr);

    const PropertyDescriptor* tags = descriptor->findProperty("tags");
    CNA_STUDIO_EXPECT(tags != nullptr);
    CNA_STUDIO_EXPECT(tags->type == PropertyType::List);
    CNA_STUDIO_EXPECT(tags->elementType == PropertyType::String);

    SceneDocument scene;
    StudioEntity entity = makeEntity(registry, "Enemy", 0.0f, 0.0f);
    StudioComponent& component = addComponentWithDefaults(entity, registry, BuiltinComponentIds::kTags);

    PropertyValue::ListValue list;
    list.items.emplace_back(std::string{"enemy"});
    list.items.emplace_back(std::string{"spawns-loot"});
    component.setProperty("tags", PropertyValue{list});
    scene.addEntity(std::move(entity));

    SceneDocument reloaded;
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(scene.toJson(), registry).succeeded);
    CNA_STUDIO_EXPECT(reloaded.getEntities().front().findComponent(BuiltinComponentIds::kTags)
                          ->getProperty("tags") == PropertyValue{list});

    // An entity carrying only a transform and a tag list still does nothing, and says so.
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(reloaded, registry), "empty-entity"), std::size_t{0});
}

// --------------------------------------------------------------------------------------------
// Prefabs (plan.md ED-300)
// --------------------------------------------------------------------------------------------

namespace
{
    /** @brief Builds a two-entity scene: "Enemy" with a sprite, and a child "Weapon". */
    struct PrefabScene
    {
        ComponentRegistry registry = makeRegistry();
        SceneDocument scene;
        Uuid rootId;
        Uuid childId;

        PrefabScene()
        {
            StudioEntity root = makeEntity(registry, "Enemy", 10.0f, 20.0f);
            addComponentWithDefaults(root, registry, BuiltinComponentIds::kSpriteRenderer);
            rootId = scene.addEntity(std::move(root));

            StudioEntity child = makeEntity(registry, "Weapon", 5.0f, 0.0f);
            addComponentWithDefaults(child, registry, BuiltinComponentIds::kSpriteRenderer);
            childId = scene.addEntity(std::move(child));
            scene.reparentEntity(childId, rootId);
        }

        [[nodiscard]] PrefabDocument capture(const std::string& name = "Enemy") const
        {
            PrefabDocument prefab;
            prefab.captureFromScene(scene, rootId, name);
            return prefab;
        }
    };
}

CNA_STUDIO_TEST(APrefabCapturesASubtreeAndRoundTripsThroughAFile)
{
    const PrefabScene fixture;
    const PrefabDocument prefab = fixture.capture();

    CNA_STUDIO_EXPECT_EQ(prefab.getEntities().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(prefab.getRootId() == fixture.rootId);
    CNA_STUDIO_EXPECT(prefab.getPrefabId().isValid());

    // The root has no parent inside the prefab: keeping the scene's would make the file describe a
    // hierarchy that only exists where it was captured from.
    CNA_STUDIO_EXPECT(!prefab.getEntities().front().getParentId().isValid());
    CNA_STUDIO_EXPECT(prefab.getEntities().back().getParentId() == fixture.rootId);

    PrefabDocument reloaded;
    const PrefabLoadResult result = reloaded.loadFromJson(prefab.toJson(), fixture.registry);
    CNA_STUDIO_EXPECT(result.succeeded);
    CNA_STUDIO_EXPECT(result.warnings.empty());
    CNA_STUDIO_EXPECT_EQ(Json::write(reloaded.toJson()), Json::write(prefab.toJson()));

    // Capturing something that is not in the scene fails rather than producing an empty prefab.
    PrefabDocument missing;
    CNA_STUDIO_EXPECT(!missing.captureFromScene(fixture.scene, Uuid::generate(), "Nothing"));
}

CNA_STUDIO_TEST(APrefabRefusesAnEmptyFileAndRepairsABrokenOne)
{
    const ComponentRegistry registry = makeRegistry();

    JsonValue json = JsonValue::makeObject();
    json.set("formatVersion", JsonValue{PrefabDocument::kFormatVersion});
    json.set("prefabId", JsonValue{Uuid::generate().toString()});
    json.set("entities", JsonValue::makeArray());

    // A prefab with no entities instantiates to nothing, and the user would have no way to tell
    // that from an instantiation that silently failed.
    PrefabDocument empty;
    CNA_STUDIO_EXPECT(!empty.loadFromJson(json, registry).succeeded);

    // A child whose parent is not in the file is attached to the root rather than dropped: a file
    // broken by a bad merge is exactly the one somebody needs the editor to open.
    const Uuid rootId = Uuid::generate();

    JsonValue root = JsonValue::makeObject();
    root.set("id", JsonValue{rootId.toString()});
    root.set("name", JsonValue{"Root"});
    root.set("components", JsonValue::makeObject());

    JsonValue orphan = JsonValue::makeObject();
    orphan.set("id", JsonValue{Uuid::generate().toString()});
    orphan.set("name", JsonValue{"Orphan"});
    orphan.set("parent", JsonValue{Uuid::generate().toString()});
    orphan.set("components", JsonValue::makeObject());

    JsonValue entities = JsonValue::makeArray();
    entities.append(std::move(root));
    entities.append(std::move(orphan));
    json.set("entities", std::move(entities));

    PrefabDocument repaired;
    const PrefabLoadResult result = repaired.loadFromJson(json, registry);
    CNA_STUDIO_EXPECT(result.succeeded);
    CNA_STUDIO_EXPECT_EQ(result.warnings.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(repaired.getEntities().back().getParentId() == rootId);

    // A file from a newer build is refused by the same gate every other format uses.
    json.set("formatVersion", JsonValue{PrefabDocument::kFormatVersion + 1});
    PrefabDocument future;
    CNA_STUDIO_EXPECT(!future.loadFromJson(json, registry).succeeded);
    CNA_STUDIO_EXPECT_EQ(getPrefabFormatMigrator().getMigrationCount(), std::size_t{0});
}

CNA_STUDIO_TEST(InstantiatingAPrefabGivesFreshIdsAndKeepsTheLink)
{
    PrefabScene fixture;
    const PrefabDocument prefab = fixture.capture();
    const Uuid assetId = Uuid::generate();

    SceneDocument target;
    auto command = std::make_unique<InstantiatePrefabCommand>(target, prefab, assetId, Uuid{});
    CNA_STUDIO_EXPECT(command->isValid());

    const Uuid instanceRoot = command->getRootId();
    command->execute();

    CNA_STUDIO_EXPECT_EQ(target.getEntityCount(), std::size_t{2});
    CNA_STUDIO_EXPECT(getPrefabAssetOf(target, instanceRoot) == assetId);

    // Fresh ids: two instances of one prefab are two different entities, and reusing the prefab's
    // would make the second instantiation collide with the first.
    CNA_STUDIO_EXPECT(instanceRoot != fixture.rootId);

    auto second = std::make_unique<InstantiatePrefabCommand>(target, prefab, assetId, Uuid{});
    second->execute();
    CNA_STUDIO_EXPECT_EQ(target.getEntityCount(), std::size_t{4});
    CNA_STUDIO_EXPECT(second->getRootId() != instanceRoot);

    // Every entity carries its link, so selecting a child still answers questions about the
    // instance it belongs to.
    const std::vector<Uuid> children = target.getChildren(instanceRoot);
    CNA_STUDIO_EXPECT_EQ(children.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(findInstanceRoot(target, children.front()) == instanceRoot);

    // A freshly made instance has changed nothing.
    CNA_STUDIO_EXPECT(findPrefabOverrides(target, instanceRoot, prefab, fixture.registry).empty());

    second->undo();
    command->undo();
    CNA_STUDIO_EXPECT_EQ(target.getEntityCount(), std::size_t{0});

    // An empty prefab, or an unknown parent, is refused rather than half-applied.
    const PrefabDocument nothing;
    CNA_STUDIO_EXPECT(!InstantiatePrefabCommand(target, nothing, assetId, Uuid{}).isValid());
    CNA_STUDIO_EXPECT(!InstantiatePrefabCommand(target, prefab, assetId, Uuid::generate()).isValid());
}

CNA_STUDIO_TEST(OverridesAreFoundByComparingRatherThanByRecording)
{
    PrefabScene fixture;
    const PrefabDocument prefab = fixture.capture();

    SceneDocument target;
    InstantiatePrefabCommand instantiate{target, prefab, Uuid::generate(), Uuid{}};
    instantiate.execute();
    const Uuid instanceRoot = instantiate.getRootId();

    // Change a property, rename an entity, add one and delete one -- four different ways an
    // instance can diverge, all of them ordinary things a user does.
    target.findEntityForEdit(instanceRoot)
        ->findComponent(BuiltinComponentIds::kTransform)
        ->setProperty("position", PropertyValue{StudioVector3{999.0f, 0.0f, 0.0f}});
    target.findEntityForEdit(instanceRoot)->setName("Boss");

    StudioEntity extra = makeEntity(fixture.registry, "Shield", 0.0f, 0.0f);
    const Uuid extraId = target.addEntity(std::move(extra));
    target.reparentEntity(extraId, instanceRoot);

    const std::vector<PrefabOverride> overrides = findPrefabOverrides(target, instanceRoot, prefab, fixture.registry);

    const auto countKind = [&](PrefabOverride::Kind kind) {
        return static_cast<std::size_t>(
            std::count_if(overrides.begin(), overrides.end(),
                          [kind](const PrefabOverride& entry) { return entry.kind == kind; }));
    };

    CNA_STUDIO_EXPECT_EQ(countKind(PrefabOverride::Kind::AddedEntity), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(countKind(PrefabOverride::Kind::RemovedEntity), std::size_t{0});
    CNA_STUDIO_EXPECT(countKind(PrefabOverride::Kind::Property) >= std::size_t{2});

    bool sawRename = false;
    bool sawPosition = false;
    for (const PrefabOverride& entry : overrides)
    {
        if (entry.propertyName == "name") { sawRename = true; }
        if (entry.propertyName == "position") { sawPosition = true; }
    }
    CNA_STUDIO_EXPECT(sawRename);
    CNA_STUDIO_EXPECT(sawPosition);

    // Deleting one of the prefab's *own* entities is a removal, not a silent match. Found by its
    // link rather than by position: the children are ordered by name, and "Shield" sorts before
    // "Weapon", so an index here would delete the entity the test just added.
    Uuid weaponId;
    for (const Uuid& childId : target.getChildren(instanceRoot))
    {
        if (target.findEntity(childId)->getName() == "Weapon") { weaponId = childId; }
    }
    CNA_STUDIO_EXPECT(weaponId.isValid());
    target.removeEntityRecursive(weaponId);
    const std::vector<PrefabOverride> afterDelete = findPrefabOverrides(target, instanceRoot, prefab, fixture.registry);
    CNA_STUDIO_EXPECT(std::any_of(afterDelete.begin(), afterDelete.end(),
                                  [](const PrefabOverride& entry)
                                  { return entry.kind == PrefabOverride::Kind::RemovedEntity; }));
}

CNA_STUDIO_TEST(RevertingAnInstancePutsItBackExactlyAndUndoesInOnePress)
{
    PrefabScene fixture;
    const PrefabDocument prefab = fixture.capture();

    SceneDocument target;
    InstantiatePrefabCommand instantiate{target, prefab, Uuid::generate(), Uuid{}};
    instantiate.execute();
    const Uuid instanceRoot = instantiate.getRootId();

    target.findEntityForEdit(instanceRoot)->setName("Boss");
    target.findEntityForEdit(instanceRoot)
        ->findComponent(BuiltinComponentIds::kTransform)
        ->setProperty("position", PropertyValue{StudioVector3{999.0f, 0.0f, 0.0f}});

    StudioEntity extra = makeEntity(fixture.registry, "Shield", 0.0f, 0.0f);
    const Uuid extraId = target.addEntity(std::move(extra));
    target.reparentEntity(extraId, instanceRoot);

    auto revert = std::make_unique<RevertPrefabInstanceCommand>(target, instanceRoot, prefab);
    CNA_STUDIO_EXPECT(revert->isValid());
    revert->execute();

    CNA_STUDIO_EXPECT(findPrefabOverrides(target, instanceRoot, prefab, fixture.registry).empty());
    CNA_STUDIO_EXPECT_EQ(target.getEntityCount(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(target.findEntity(instanceRoot)->getName(), std::string{"Enemy"});

    // The added entity is gone. That is deliberate: a revert that left some of the user's changes
    // in place would not be a revert, and the user who wanted a partial one has undo.
    CNA_STUDIO_EXPECT(target.findEntity(extraId) == nullptr);

    // The root keeps its id, because the selection and every reference into the instance point at
    // it -- reverting must not break them.
    CNA_STUDIO_EXPECT(target.findEntity(instanceRoot) != nullptr);
    CNA_STUDIO_EXPECT(getPrefabAssetOf(target, instanceRoot).isValid());

    revert->undo();
    CNA_STUDIO_EXPECT_EQ(target.getEntityCount(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(target.findEntity(instanceRoot)->getName(), std::string{"Boss"});
    CNA_STUDIO_EXPECT(target.findEntity(extraId) != nullptr);

    // Reverting an instance that has not diverged is refused: an undo entry that undoes to the
    // state it is already in reads to the user as a broken Ctrl+Z.
    revert->execute();
    CNA_STUDIO_EXPECT(!RevertPrefabInstanceCommand(target, instanceRoot, prefab).isValid());

    // As is reverting something that is not an instance at all.
    const Uuid plain = target.addEntity(makeEntity(fixture.registry, "Plain", 0.0f, 0.0f));
    CNA_STUDIO_EXPECT(!RevertPrefabInstanceCommand(target, plain, prefab).isValid());
}

// --------------------------------------------------------------------------------------------
// Tilemaps (plan.md ED-301)
// --------------------------------------------------------------------------------------------

namespace
{
    /** @brief Builds a scene with one 4x3 tilemap entity at the origin, 32px tiles. */
    struct TilemapScene
    {
        ComponentRegistry registry = makeRegistry();
        SceneDocument scene;
        Uuid entityId;

        TilemapScene()
        {
            StudioEntity entity = makeEntity(registry, "Ground", 0.0f, 0.0f);
            StudioComponent& tilemap = addComponentWithDefaults(entity, registry, BuiltinComponentIds::kTilemap);
            tilemap.setProperty(TilemapKeys::kColumns, PropertyValue{std::int64_t{4}});
            tilemap.setProperty(TilemapKeys::kRows, PropertyValue{std::int64_t{3}});
            entityId = scene.addEntity(std::move(entity));
        }

        [[nodiscard]] TilemapGrid grid()
        {
            return readTilemapGrid(*scene.findEntity(entityId)->findComponent(BuiltinComponentIds::kTilemap),
                                   registry.find(BuiltinComponentIds::kTilemap));
        }
    };
}

CNA_STUDIO_TEST(ATilemapGridReadsPadsAndResizesByCoordinate)
{
    TilemapScene fixture;

    // A tilemap that has never been painted reads as a full grid of empty cells, not as nothing:
    // the paint tool needs somewhere to put the first tile.
    TilemapGrid grid = fixture.grid();
    CNA_STUDIO_EXPECT_EQ(grid.columns, 4);
    CNA_STUDIO_EXPECT_EQ(grid.rows, 3);
    CNA_STUDIO_EXPECT_EQ(grid.tiles.size(), std::size_t{12});
    CNA_STUDIO_EXPECT_EQ(grid.at(0, 0), kEmptyTile);

    // Out of range answers empty rather than failing: the cursor leaves the map constantly.
    CNA_STUDIO_EXPECT_EQ(grid.at(-1, 0), kEmptyTile);
    CNA_STUDIO_EXPECT_EQ(grid.at(4, 0), kEmptyTile);
    grid.set(9, 9, 5);
    CNA_STUDIO_EXPECT_EQ(grid.tiles.size(), std::size_t{12});

    grid.set(0, 0, 7);
    grid.set(3, 2, 8);
    CNA_STUDIO_EXPECT_EQ(grid.at(0, 0), std::int64_t{7});
    CNA_STUDIO_EXPECT_EQ(grid.at(3, 2), std::int64_t{8});

    // Resizing keeps tiles where they were on screen. Copying a flat list without remapping shifts
    // every row sideways, which turns "one column wider" into "scramble the level".
    const TilemapGrid wider = resizeTilemapGrid(grid, 6, 3);
    CNA_STUDIO_EXPECT_EQ(wider.at(0, 0), std::int64_t{7});
    CNA_STUDIO_EXPECT_EQ(wider.at(3, 2), std::int64_t{8});
    CNA_STUDIO_EXPECT_EQ(wider.at(5, 2), kEmptyTile);

    // And shrinking drops what no longer fits rather than wrapping it somewhere unexpected.
    const TilemapGrid smaller = resizeTilemapGrid(grid, 2, 2);
    CNA_STUDIO_EXPECT_EQ(smaller.at(0, 0), std::int64_t{7});
    CNA_STUDIO_EXPECT_EQ(smaller.tiles.size(), std::size_t{4});

    // A stored list of the wrong length is padded, not rejected: a hand-edited scene one row short
    // should open and be fixable.
    StudioComponent* component =
        fixture.scene.findEntityForEdit(fixture.entityId)->findComponent(BuiltinComponentIds::kTilemap);
    PropertyValue::ListValue truncated;
    truncated.items.emplace_back(std::int64_t{3});
    component->setProperty(TilemapKeys::kTiles, PropertyValue{truncated});

    const TilemapGrid padded = fixture.grid();
    CNA_STUDIO_EXPECT_EQ(padded.tiles.size(), std::size_t{12});
    CNA_STUDIO_EXPECT_EQ(padded.at(0, 0), std::int64_t{3});
    CNA_STUDIO_EXPECT_EQ(padded.at(1, 0), kEmptyTile);
}

CNA_STUDIO_TEST(WorldPointsMapToTilesIncludingOutsideTheMap)
{
    WorldTransform transform;
    transform.position = StudioVector3{100.0f, 200.0f, 0.0f};

    CNA_STUDIO_EXPECT_EQ(worldToTile(transform, 32, 32, StudioVector2{100.0f, 200.0f}).x, 0);
    CNA_STUDIO_EXPECT_EQ(worldToTile(transform, 32, 32, StudioVector2{131.0f, 200.0f}).x, 0);
    CNA_STUDIO_EXPECT_EQ(worldToTile(transform, 32, 32, StudioVector2{132.0f, 200.0f}).x, 1);

    // Grows downward, matching how SpriteBatch addresses the screen and how tile editors number
    // their rows.
    CNA_STUDIO_EXPECT_EQ(worldToTile(transform, 32, 32, StudioVector2{100.0f, 233.0f}).y, 1);

    // Floored rather than truncated: truncation folds -0.5 and +0.5 onto the same cell, so a click
    // just left of the origin would land on the first column instead of outside the map.
    CNA_STUDIO_EXPECT_EQ(worldToTile(transform, 32, 32, StudioVector2{99.0f, 200.0f}).x, -1);

    // Scale is honoured, because zooming a map by scaling its entity is ordinary.
    transform.scale = StudioVector3{2.0f, 2.0f, 1.0f};
    CNA_STUDIO_EXPECT_EQ(worldToTile(transform, 32, 32, StudioVector2{163.0f, 200.0f}).x, 0);
    CNA_STUDIO_EXPECT_EQ(worldToTile(transform, 32, 32, StudioVector2{165.0f, 200.0f}).x, 1);

    // A zero tile size would divide by zero, and a hand-edited scene can hold one.
    CNA_STUDIO_EXPECT_EQ(worldToTile(transform, 0, 32, StudioVector2{100.0f, 200.0f}).x, -1);
}

CNA_STUDIO_TEST(APaintStrokeIsOneUndoEntryAndTwoStrokesAreTwo)
{
    TilemapScene fixture;
    CommandHistory history;

    const auto paint = [&](int x, int y, std::int64_t tile, std::uint64_t stroke, bool first) {
        auto command = std::make_unique<PaintTilesCommand>(fixture.scene, fixture.registry,
                                                            fixture.entityId, stroke);
        if (!command->paint(x, y, tile)) { return; }
        history.execute(std::move(command),
                        first ? MergePolicy::NewEntry : MergePolicy::MergeWithPrevious);
    };

    // One drag across three cells.
    paint(0, 0, 5, 1, true);
    paint(1, 0, 5, 1, false);
    paint(2, 0, 5, 1, false);

    CNA_STUDIO_EXPECT_EQ(history.getCount(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(1, 0), std::int64_t{5});

    // A second drag, on a different stroke, must not fold into the first -- one Ctrl+Z would then
    // lose both, and the merge key of entity + property alone cannot tell them apart.
    paint(0, 1, 6, 2, true);
    paint(1, 1, 6, 2, false);
    CNA_STUDIO_EXPECT_EQ(history.getCount(), std::size_t{2});

    history.undo();
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(0, 1), kEmptyTile);
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(0, 0), std::int64_t{5});

    history.undo();
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(0, 0), kEmptyTile);
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(2, 0), kEmptyTile);

    history.redo();
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(2, 0), std::int64_t{5});
}

CNA_STUDIO_TEST(PaintingIgnoresNoOpCellsAndRemembersTheValueTheStrokeStartedFrom)
{
    TilemapScene fixture;

    PaintTilesCommand first{fixture.scene, fixture.registry, fixture.entityId, 1};
    CNA_STUDIO_EXPECT(first.paint(0, 0, 3));
    first.execute();
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(0, 0), std::int64_t{3});

    // Painting a cell the value it already holds records nothing: an undo entry that changes
    // nothing is one the user cannot see the effect of.
    PaintTilesCommand same{fixture.scene, fixture.registry, fixture.entityId, 2};
    CNA_STUDIO_EXPECT(!same.paint(0, 0, 3));
    CNA_STUDIO_EXPECT(!same.isValid());

    // Nor does a cell outside the grid.
    CNA_STUDIO_EXPECT(!same.paint(99, 99, 4));

    // Crossing the same cell twice within one stroke keeps the value it had *before* the stroke,
    // so one Ctrl+Z goes back to before the drag rather than to the middle of it.
    PaintTilesCommand wobble{fixture.scene, fixture.registry, fixture.entityId, 3};
    CNA_STUDIO_EXPECT(wobble.paint(0, 0, 7));
    CNA_STUDIO_EXPECT(wobble.paint(0, 0, 8));
    CNA_STUDIO_EXPECT_EQ(wobble.getCells().size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(wobble.getCells().front().oldTile, std::int64_t{3});

    wobble.execute();
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(0, 0), std::int64_t{8});
    wobble.undo();
    CNA_STUDIO_EXPECT_EQ(fixture.grid().at(0, 0), std::int64_t{3});
}

CNA_STUDIO_TEST(ATilemapRoundTripsThroughASceneFile)
{
    TilemapScene fixture;

    PaintTilesCommand stroke{fixture.scene, fixture.registry, fixture.entityId, 1};
    stroke.paint(0, 0, 1);
    stroke.paint(3, 2, 2);
    stroke.execute();

    SceneDocument reloaded;
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(fixture.scene.toJson(), fixture.registry).succeeded);

    const StudioComponent* tilemap =
        reloaded.getEntities().front().findComponent(BuiltinComponentIds::kTilemap);
    CNA_STUDIO_EXPECT(tilemap != nullptr);

    const TilemapGrid grid =
        readTilemapGrid(*tilemap, fixture.registry.find(BuiltinComponentIds::kTilemap));
    CNA_STUDIO_EXPECT_EQ(grid.at(0, 0), std::int64_t{1});
    CNA_STUDIO_EXPECT_EQ(grid.at(3, 2), std::int64_t{2});
    CNA_STUDIO_EXPECT_EQ(grid.at(1, 1), kEmptyTile);

    // No new serialised structure: the grid is an ordinary List property, so a scene holding a
    // tilemap is readable by anything that could already read a scene.
    CNA_STUDIO_EXPECT(fixture.scene.toJson()["entities"]
                          .getElements()
                          .front()["components"]["CNA.Tilemap"]["tiles"]
                          .isArray());
}

// --------------------------------------------------------------------------------------------
// Sprite animation (plan.md ED-303)
// --------------------------------------------------------------------------------------------

namespace
{
    /** @brief A four-frame clip at 10 fps, frames 0..3 of an 8-wide sheet of 32px cells. */
    SpriteAnimationClip makeClip(bool loop = true)
    {
        SpriteAnimationClip clip;
        clip.frames = {0, 1, 2, 3};
        clip.frameWidth = 32;
        clip.frameHeight = 32;
        clip.sheetColumns = 8;
        clip.framesPerSecond = 10.0f;
        clip.loop = loop;
        return clip;
    }
}

CNA_STUDIO_TEST(AClipTurnsAFrameIndexIntoASourceRectangle)
{
    SpriteAnimationClip clip = makeClip();
    clip.frames = {0, 7, 8, 15};

    CNA_STUDIO_EXPECT_EQ(clip.getFrameRectangle(0).x, 0);
    CNA_STUDIO_EXPECT_EQ(clip.getFrameRectangle(0).y, 0);

    // The last cell of the first row, then the first of the second: the wrap is what makes an
    // index cheaper to author than a rectangle.
    CNA_STUDIO_EXPECT_EQ(clip.getFrameRectangle(1).x, 224);
    CNA_STUDIO_EXPECT_EQ(clip.getFrameRectangle(1).y, 0);
    CNA_STUDIO_EXPECT_EQ(clip.getFrameRectangle(2).x, 0);
    CNA_STUDIO_EXPECT_EQ(clip.getFrameRectangle(2).y, 32);
    CNA_STUDIO_EXPECT_EQ(clip.getFrameRectangle(3).y, 32);

    // Out of range draws nothing rather than clamping: a caller that has lost track of where it is
    // should show that, not silently show frame zero.
    CNA_STUDIO_EXPECT(clip.getFrameRectangle(99).isEmpty());

    CNA_STUDIO_EXPECT_EQ(clip.getFrameCount(), std::size_t{4});
    CNA_STUDIO_EXPECT_EQ(clip.getDuration(), 0.4f);
}

CNA_STUDIO_TEST(PlaybackAdvancesOnTheClockItIsGivenAndCatchesUp)
{
    const SpriteAnimationClip clip = makeClip();
    AnimationPlayback playback;

    // Not playing: the clock passes and nothing moves.
    CNA_STUDIO_EXPECT(!playback.advance(clip, 1.0f));
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{0});

    playback.playing = true;
    CNA_STUDIO_EXPECT(!playback.advance(clip, 0.05f));
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{0});

    CNA_STUDIO_EXPECT(playback.advance(clip, 0.05f));
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{1});

    // A long stall covers the frames it spans rather than crawling back one at a time: at 10 fps a
    // quarter-second gap is two and a half frames.
    CNA_STUDIO_EXPECT(playback.advance(clip, 0.25f));
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{3});

    // Looping wraps and keeps playing.
    CNA_STUDIO_EXPECT(playback.advance(clip, 0.1f));
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{0});
    CNA_STUDIO_EXPECT(playback.playing);
}

CNA_STUDIO_TEST(ANonLoopingClipHoldsItsLastFrameAndStops)
{
    const SpriteAnimationClip clip = makeClip(false);
    AnimationPlayback playback;
    playback.playing = true;

    playback.advance(clip, 1.0f);

    // Held, not wrapped, and stopped -- a non-looping clip that quietly restarted would be
    // indistinguishable from a looping one.
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{3});
    CNA_STUDIO_EXPECT(!playback.playing);
}

CNA_STUDIO_TEST(SteppingWrapsBothWaysAndStopsPlayback)
{
    const SpriteAnimationClip clip = makeClip();
    AnimationPlayback playback;
    playback.playing = true;

    playback.step(clip, 1);
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{1});

    // A step is a deliberate look at one frame, so it stops playback rather than fighting it.
    CNA_STUDIO_EXPECT(!playback.playing);

    playback.step(clip, -1);
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{0});
    playback.step(clip, -1);
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{3});

    // A frame list shortened while the preview was past its new end is clamped, not left dangling.
    SpriteAnimationClip shorter = clip;
    shorter.frames = {0, 1};
    playback.clampTo(shorter);
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{1});

    SpriteAnimationClip empty = clip;
    empty.frames.clear();
    playback.clampTo(empty);
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{0});
}

CNA_STUDIO_TEST(AnAnimationClipRoundTripsThroughAComponent)
{
    const ComponentRegistry registry = makeRegistry();

    StudioEntity entity = makeEntity(registry, "Hero", 0.0f, 0.0f);
    StudioComponent& component =
        addComponentWithDefaults(entity, registry, BuiltinComponentIds::kSpriteAnimation);

    PropertyValue::ListValue frames;
    frames.items.emplace_back(std::int64_t{4});
    frames.items.emplace_back(std::int64_t{5});
    component.setProperty(SpriteAnimationKeys::kFrames, PropertyValue{frames});
    component.setProperty(SpriteAnimationKeys::kFramesPerSecond, PropertyValue{24.0f});

    SceneDocument scene;
    scene.addEntity(std::move(entity));

    SceneDocument reloaded;
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(scene.toJson(), registry).succeeded);

    const StudioComponent* readBack =
        reloaded.getEntities().front().findComponent(BuiltinComponentIds::kSpriteAnimation);
    CNA_STUDIO_EXPECT(readBack != nullptr);

    const SpriteAnimationClip clip =
        readSpriteAnimationClip(*readBack, registry.find(BuiltinComponentIds::kSpriteAnimation));
    CNA_STUDIO_EXPECT_EQ(clip.frames.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(clip.frames.front(), std::int64_t{4});
    CNA_STUDIO_EXPECT_EQ(clip.framesPerSecond, 24.0f);
    CNA_STUDIO_EXPECT(clip.loop);
}

CNA_STUDIO_TEST(PerFrameDurationsAreOptionalAndIgnoredWhenTheyDoNotFit)
{
    SpriteAnimationClip clip = makeClip();

    // No durations: the uniform rate applies, which is what every scene written before this
    // existed relies on.
    CNA_STUDIO_EXPECT(!clip.hasFrameDurations());
    CNA_STUDIO_EXPECT_EQ(clip.getFrameDuration(0), 0.1f);
    CNA_STUDIO_EXPECT_EQ(clip.getDuration(), 0.4f);

    // A list of the wrong length is ignored rather than partly applied: half a clip at one rate
    // and half at another is a state nobody asked for.
    clip.frameDurations = {0.5f, 0.5f};
    CNA_STUDIO_EXPECT(!clip.hasFrameDurations());
    CNA_STUDIO_EXPECT_EQ(clip.getFrameDuration(0), 0.1f);

    clip.frameDurations = {0.5f, 0.1f, 0.1f, 0.1f};
    CNA_STUDIO_EXPECT(clip.hasFrameDurations());
    CNA_STUDIO_EXPECT_EQ(clip.getFrameDuration(0), 0.5f);
    CNA_STUDIO_EXPECT_EQ(clip.getFrameDuration(1), 0.1f);
    // Summed rather than multiplied, so the comparison has to tolerate the last bit.
    CNA_STUDIO_EXPECT(std::fabs(clip.getDuration() - 0.8f) < 1e-5f);

    // A zero entry falls back to the uniform rate rather than becoming a frame of no length, which
    // would spin the playback loop forever looking for the next one.
    clip.frameDurations[2] = 0.0f;
    CNA_STUDIO_EXPECT_EQ(clip.getFrameDuration(2), 0.1f);
}

CNA_STUDIO_TEST(PlaybackHoldsEachFrameForItsOwnDuration)
{
    SpriteAnimationClip clip = makeClip();
    clip.frameDurations = {0.5f, 0.1f, 0.1f, 0.1f};

    AnimationPlayback playback;
    playback.playing = true;

    // The long first frame is held through what would have been four uniform frames.
    CNA_STUDIO_EXPECT(!playback.advance(clip, 0.4f));
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{0});

    CNA_STUDIO_EXPECT(playback.advance(clip, 0.1f));
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{1});

    // Then the short ones go by three times as fast.
    CNA_STUDIO_EXPECT(playback.advance(clip, 0.2f));
    CNA_STUDIO_EXPECT_EQ(playback.position, std::size_t{3});

    // A rate of zero with no durations cannot play, and says so by not moving rather than by
    // dividing by it.
    SpriteAnimationClip stopped = makeClip();
    stopped.framesPerSecond = 0.0f;
    AnimationPlayback stuck;
    stuck.playing = true;
    CNA_STUDIO_EXPECT(!stuck.advance(stopped, 10.0f));
    CNA_STUDIO_EXPECT_EQ(stuck.position, std::size_t{0});
}

CNA_STUDIO_TEST(AClipWithoutDurationsSerialisesExactlyAsItDidBefore)
{
    const ComponentRegistry registry = makeRegistry();

    StudioEntity entity = makeEntity(registry, "Hero", 0.0f, 0.0f);
    StudioComponent& component =
        addComponentWithDefaults(entity, registry, BuiltinComponentIds::kSpriteAnimation);

    PropertyValue::ListValue frames;
    frames.items.emplace_back(std::int64_t{0});
    frames.items.emplace_back(std::int64_t{1});
    component.setProperty(SpriteAnimationKeys::kFrames, PropertyValue{frames});

    SceneDocument scene;
    scene.addEntity(std::move(entity));

    SceneDocument reloaded;
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(scene.toJson(), registry).succeeded);

    const SpriteAnimationClip clip = readSpriteAnimationClip(
        *reloaded.getEntities().front().findComponent(BuiltinComponentIds::kSpriteAnimation),
        registry.find(BuiltinComponentIds::kSpriteAnimation));

    // The default is an empty list, which round-trips as an empty list and changes nothing about
    // how the clip plays.
    CNA_STUDIO_EXPECT(!clip.hasFrameDurations());
    CNA_STUDIO_EXPECT_EQ(clip.getFrameDuration(0), clip.getFrameDuration(1));
}

CNA_STUDIO_TEST(TwoEnabledAudioListenersAreAnError)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity first = makeEntity(registry, "Player", 0.0f, 0.0f);
    addComponentWithDefaults(first, registry, BuiltinComponentIds::kAudioListener);
    scene.addEntity(std::move(first));

    // One listener is the ordinary case and says nothing.
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(scene, registry), "duplicate-audio-listener"),
                         std::size_t{0});

    StudioEntity second = makeEntity(registry, "Camera Rig", 0.0f, 0.0f);
    addComponentWithDefaults(second, registry, BuiltinComponentIds::kAudioListener);
    scene.addEntity(std::move(second));

    // XNA mixes 3D audio relative to one listener. Two is not louder or wider -- it is a choice
    // the runtime makes for the user, exactly as invisible as two primary cameras.
    const std::vector<SceneIssue> issues = validateScene(scene, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "duplicate-audio-listener"), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(countIssues(issues, SceneIssue::Severity::Error), std::size_t{2});

    // Switching one off resolves it, the same way it resolves a second primary camera.
    scene.findEntityForEdit(scene.getEntities().back().getId())->setEnabled(false);
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(scene, registry), "duplicate-audio-listener"),
                         std::size_t{0});
}

namespace
{
    /** @brief Returns true when @p a and @p b agree to within @p tolerance. */
    bool cameraNearlyEqual(float a, float b, float tolerance = 0.01f) { return std::abs(a - b) <= tolerance; }

    /** @brief Fails unless @p actual matches @p expected on every axis. */
    void expectVectorEquals(const StudioVector3& actual, const StudioVector3& expected)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(actual.x, expected.x));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(actual.y, expected.y));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(actual.z, expected.z));
    }

    /** @brief A camera over a 1600x900 viewport, looking at the origin from ten units away. */
    StudioCamera3D makeCamera()
    {
        StudioCamera3D camera;
        camera.setViewportSize(StudioVector2{1600.0f, 900.0f});
        return camera;
    }
}

CNA_STUDIO_TEST(TheEyeIsDerivedFromThePivotDistanceAndAngles)
{
    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{5.0f, 1.0f, -2.0f});
    camera.setDistance(20.0f);
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);

    // Yaw zero, pitch zero: looking down -Z, so the eye is twenty units along +Z from the pivot.
    const StudioVector3 eye = camera.getEye();
    CNA_STUDIO_EXPECT(cameraNearlyEqual(eye.x, 5.0f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(eye.y, 1.0f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(eye.z, 18.0f));

    // Positive pitch looks down on screen, which is what dragging downwards on an orbit has to do
    // -- and in this camera's Y-down world, downward on screen is towards +Y, so the eye sits at
    // the *smaller* Y. Both of these read backwards against XNA's 3D convention and are correct
    // against the one the document and the 2D viewport actually use.
    camera.setPitch(0.5f);
    CNA_STUDIO_EXPECT(camera.getForward().y > 0.0f);
    CNA_STUDIO_EXPECT(camera.getEye().y < camera.getPivot().y);

    // The basis stays orthonormal whatever the angles, or every projection built on it shears.
    const StudioVector3 right = camera.getRight();
    const StudioVector3 up = camera.getUp();
    const StudioVector3 forward = camera.getForward();
    CNA_STUDIO_EXPECT(cameraNearlyEqual(length(right), 1.0f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(length(up), 1.0f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(dot(right, up), 0.0f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(dot(right, forward), 0.0f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(dot(up, forward), 0.0f));
}

CNA_STUDIO_TEST(PitchIsClampedJustShortOfVertical)
{
    StudioCamera3D camera = makeCamera();

    // Straight down is where the up vector and the view direction become parallel and the view
    // matrix stops being defined. Clamping short of it is cheaper than handling it everywhere.
    camera.orbit(0.0f, 100.0f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getPitch(), StudioCamera3D::kMaxPitchRadians));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(length(camera.getRight()), 1.0f));

    camera.orbit(0.0f, -100.0f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getPitch(), -StudioCamera3D::kMaxPitchRadians));

    // Yaw wraps rather than clamping: it has no ends, and letting it grow costs precision.
    camera.setYaw(0.0f);
    camera.orbit(100.0f, 0.0f);
    CNA_STUDIO_EXPECT(std::abs(camera.getYaw()) <= 3.1416f);
}

CNA_STUDIO_TEST(ThePivotProjectsToTheCentreOfTheViewport)
{
    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{3.0f, -4.0f, 12.0f});
    camera.orbit(0.7f, -0.2f);

    const std::optional<StudioVector2> centre = camera.worldToScreen(camera.getPivot());
    CNA_STUDIO_EXPECT(centre.has_value());
    CNA_STUDIO_EXPECT(cameraNearlyEqual(centre->x, 800.0f, 0.5f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(centre->y, 450.0f, 0.5f));

    // Behind the eye there is no answer. Returning one would send a line across the screen the
    // moment a vertex passed the camera, which is the classic wireframe artefact.
    const StudioVector3 behind = add(camera.getEye(), scale(camera.getForward(), -5.0f));
    CNA_STUDIO_EXPECT(!camera.worldToScreen(behind).has_value());

    // Up on screen is up in the world: a point above the pivot lands above the centre, where
    // screen Y is smaller. The Y flip between clip space and the panel is easy to lose.
    const std::optional<StudioVector2> above =
        camera.worldToScreen(add(camera.getPivot(), scale(camera.getUp(), 1.0f)));
    CNA_STUDIO_EXPECT(above.has_value());
    CNA_STUDIO_EXPECT(above->y < centre->y);
}

CNA_STUDIO_TEST(ARayThroughAPixelComesBackToThatPixel)
{
    for (const CameraProjection projection : {CameraProjection::Perspective, CameraProjection::Orthographic})
    {
        StudioCamera3D camera = makeCamera();
        camera.setProjection(projection);
        camera.setPivot(StudioVector3{2.0f, 3.0f, -1.0f});
        camera.orbit(0.5f, 0.3f);

        const StudioVector2 pixel{1180.0f, 260.0f};
        const WorldRay ray = camera.screenToRay(pixel);

        CNA_STUDIO_EXPECT(cameraNearlyEqual(length(ray.direction), 1.0f));

        // Round trip: a point along the ray must project back to the pixel it came from. This is
        // the property picking depends on, and it is the one an inverted projection gets wrong.
        const std::optional<StudioVector2> back = camera.worldToScreen(ray.at(25.0f));
        CNA_STUDIO_EXPECT(back.has_value());
        CNA_STUDIO_EXPECT(cameraNearlyEqual(back->x, pixel.x, 0.5f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(back->y, pixel.y, 0.5f));
    }

    // An orthographic ray starts wherever the pixel is, not at the eye -- which is why the ray is
    // unprojected at both depth limits rather than fired from the camera position.
    StudioCamera3D orthographic = makeCamera();
    orthographic.setProjection(CameraProjection::Orthographic);
    const WorldRay corner = orthographic.screenToRay(StudioVector2{0.0f, 0.0f});
    const WorldRay middle = orthographic.screenToRay(StudioVector2{800.0f, 450.0f});
    CNA_STUDIO_EXPECT(length(subtract(corner.origin, middle.origin)) > 1.0f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(dot(corner.direction, middle.direction), 1.0f));
}

CNA_STUDIO_TEST(OrbitTurnsAboutThePivotAndFlyingCarriesItAlong)
{
    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{1.0f, 2.0f, 3.0f});
    camera.setDistance(15.0f);

    const StudioVector3 pivotBefore = camera.getPivot();
    const StudioVector3 eyeBefore = camera.getEye();

    camera.orbit(0.9f, 0.1f);
    expectVectorEquals(camera.getPivot(), pivotBefore);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getDistance(), 15.0f));
    CNA_STUDIO_EXPECT(length(subtract(camera.getEye(), eyeBefore)) > 1.0f);

    // Looking is the mirror image: the eye stays put and the pivot swings round in front of it,
    // so that a following orbit turns about what the user is now looking at.
    const StudioVector3 eyeBeforeLook = camera.getEye();
    camera.look(0.4f, -0.2f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getEye().x, eyeBeforeLook.x));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getEye().y, eyeBeforeLook.y));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getEye().z, eyeBeforeLook.z));
    CNA_STUDIO_EXPECT(length(subtract(camera.getPivot(), pivotBefore)) > 0.5f);

    // Flying forward closes the gap to whatever is ahead without changing the orbit radius.
    const StudioVector3 forward = camera.getForward();
    const StudioVector3 eyeBeforeMove = camera.getEye();
    camera.moveLocal(StudioVector3{0.0f, 0.0f, 4.0f});
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getDistance(), 15.0f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(length(subtract(camera.getEye(), eyeBeforeMove)), 4.0f));
    CNA_STUDIO_EXPECT(dot(subtract(camera.getEye(), eyeBeforeMove), forward) > 0.0f);
}

CNA_STUDIO_TEST(PanningKeepsThePivotUnderTheCursor)
{
    for (const CameraProjection projection : {CameraProjection::Perspective, CameraProjection::Orthographic})
    {
        StudioCamera3D camera = makeCamera();
        camera.setProjection(projection);
        camera.orbit(0.6f, 0.35f);

        const StudioVector3 pivotBefore = camera.getPivot();
        const float yawBefore = camera.getYaw();
        const float pitchBefore = camera.getPitch();
        camera.panByScreenDelta(StudioVector2{120.0f, -45.0f});

        // A drag moves the world with the cursor: the point that was at the centre is now exactly
        // as far from it as the cursor travelled. Anything else is drift, which is the whole
        // reason the delta is taken in pixels rather than converted by the caller.
        const std::optional<StudioVector2> moved = camera.worldToScreen(pivotBefore);
        CNA_STUDIO_EXPECT(moved.has_value());
        CNA_STUDIO_EXPECT(cameraNearlyEqual(moved->x, 800.0f + 120.0f, 0.5f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(moved->y, 450.0f - 45.0f, 0.5f));

        // Panning slides the camera; it never turns it.
        CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getYaw(), yawBefore));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getPitch(), pitchBefore));
    }
}

CNA_STUDIO_TEST(DollyingScalesTheDistanceAndStopsAtTheLimits)
{
    StudioCamera3D camera = makeCamera();
    camera.setDistance(10.0f);

    camera.dolly(0.5f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getDistance(), 5.0f));

    // Multiplying rather than subtracting is what makes one wheel notch feel the same close up
    // and far away -- a fixed step crawls across a level and lands inside a model.
    camera.dolly(2.0f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getDistance(), 10.0f));

    for (int step = 0; step < 200; ++step) { camera.dolly(0.5f); }
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getDistance(), StudioCamera3D::kMinDistance, 0.001f));

    // A factor of zero or less would put the eye on the pivot or behind it; it is refused rather
    // than clamped, because there is no sensible interpretation of a negative zoom.
    camera.setDistance(10.0f);
    camera.dolly(0.0f);
    camera.dolly(-1.0f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(camera.getDistance(), 10.0f));
}

CNA_STUDIO_TEST(FramingFitsTheWholeBoxOnScreen)
{
    for (const CameraProjection projection : {CameraProjection::Perspective, CameraProjection::Orthographic})
    {
        StudioCamera3D camera = makeCamera();
        camera.setProjection(projection);
        camera.orbit(0.8f, 0.4f);

        const WorldBounds3D bounds{StudioVector3{-30.0f, 10.0f, -5.0f}, StudioVector3{50.0f, 40.0f, 25.0f}};
        camera.frame(bounds);

        expectVectorEquals(camera.getPivot(), bounds.getCenter());

        // Every corner has to land inside the panel, not merely the centre: framing that fits the
        // middle of a box and clips its ends is the failure this is here to catch.
        for (int corner = 0; corner < 8; ++corner)
        {
            const StudioVector3 point{(corner & 1) != 0 ? bounds.max.x : bounds.min.x,
                                      (corner & 2) != 0 ? bounds.max.y : bounds.min.y,
                                      (corner & 4) != 0 ? bounds.max.z : bounds.min.z};
            const std::optional<StudioVector2> screen = camera.worldToScreen(point);
            CNA_STUDIO_EXPECT(screen.has_value());
            if (!screen) { continue; }
            CNA_STUDIO_EXPECT(screen->x >= 0.0f && screen->x <= 1600.0f);
            CNA_STUDIO_EXPECT(screen->y >= 0.0f && screen->y <= 900.0f);
        }
    }

    // An empty box is not a request to look at nothing: it is a caller with no selection, and the
    // camera it already has is a better answer than an arbitrary one.
    StudioCamera3D unchanged = makeCamera();
    unchanged.setPivot(StudioVector3{7.0f, 7.0f, 7.0f});
    unchanged.frame(WorldBounds3D::makeEmpty());
    expectVectorEquals(unchanged.getPivot(), StudioVector3{7.0f, 7.0f, 7.0f});
}

CNA_STUDIO_TEST(SwitchingProjectionKeepsTheSubjectTheSameSize)
{
    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{0.0f, 0.0f, 0.0f});
    camera.setDistance(30.0f);
    camera.orbit(0.0f, 0.0f);

    // The orthographic height is the perspective extent *at the pivot*, so a toggle is a change of
    // projection rather than a jump cut -- which is the point of having the toggle at all.
    const StudioVector3 sample = add(camera.getPivot(), scale(camera.getUp(), 4.0f));

    const std::optional<StudioVector2> inPerspective = camera.worldToScreen(sample);
    camera.setProjection(CameraProjection::Orthographic);
    const std::optional<StudioVector2> inOrthographic = camera.worldToScreen(sample);

    CNA_STUDIO_EXPECT(inPerspective.has_value() && inOrthographic.has_value());
    CNA_STUDIO_EXPECT(cameraNearlyEqual(inPerspective->y, inOrthographic->y, 1.0f));

    // An orthographic camera can see what is beside it, so its near plane sits behind the eye:
    // clipping at the eye would hide everything the user just dollied towards.
    const StudioVector3 besideTheEye = add(camera.getEye(), scale(camera.getRight(), 3.0f));
    CNA_STUDIO_EXPECT(camera.worldToScreen(besideTheEye).has_value());
}

CNA_STUDIO_TEST(SceneBoundsCoverEntitiesThatDrawNothing)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid spriteId = scene.addEntity(makeEntity(registry, "Sprite", 100.0f, 0.0f));
    StudioComponent renderer{BuiltinComponentIds::kSpriteRenderer};
    renderer.applyDefaults(*registry.find(BuiltinComponentIds::kSpriteRenderer));
    scene.findEntityForEdit(spriteId)->addComponent(std::move(renderer));

    // A bare Transform -- a camera, a light, an empty parent. The 2D viewport leaves these out of
    // framing because it draws them as fixed-size icons, but in a 3D view they are often the only
    // thing a scene contains, and "nothing to look at" would be the wrong answer.
    const Uuid emptyId = scene.addEntity(makeEntity(registry, "Spawn Point", -400.0f, 0.0f));

    const SpriteSizeProvider noSizes = [](const Uuid&) { return StudioVector2{32.0f, 32.0f}; };

    const std::optional<WorldBounds3D> empty = computeEntityBounds3D(scene, emptyId, noSizes);
    CNA_STUDIO_EXPECT(empty.has_value());
    CNA_STUDIO_EXPECT(empty->contains(StudioVector3{-400.0f, 0.0f, 0.0f}));

    const std::optional<WorldBounds3D> whole = computeSceneBounds3D(scene, noSizes);
    CNA_STUDIO_EXPECT(whole.has_value());
    CNA_STUDIO_EXPECT(whole->min.x <= -400.0f);
    CNA_STUDIO_EXPECT(whole->max.x >= 100.0f);

    // An unknown entity has no bounds, rather than bounds at the origin that would drag every
    // union towards it.
    CNA_STUDIO_EXPECT(!computeEntityBounds3D(scene, Uuid::generate(), noSizes).has_value());
}

CNA_STUDIO_TEST(ASegmentCrossingTheNearPlaneIsShortenedRatherThanDropped)
{
    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(10.0f);

    // The eye is at (0, 0, 10) looking down -Z. A line running from well behind the camera to well
    // in front of it is the grid line under the user's feet: dropping it whole leaves a wedge of
    // missing floor exactly where they are looking.
    const std::optional<std::pair<StudioVector2, StudioVector2>> crossing =
        projectSegment(camera, StudioVector3{0.0f, -5.0f, 40.0f}, StudioVector3{0.0f, -5.0f, -40.0f});
    CNA_STUDIO_EXPECT(crossing.has_value());

    // Entirely behind the eye there is nothing to draw, and the projected coordinates of such a
    // segment are mirrored through the origin -- plausible numbers describing the wrong line.
    CNA_STUDIO_EXPECT(!projectSegment(camera, StudioVector3{0.0f, 0.0f, 20.0f},
                                      StudioVector3{0.0f, 0.0f, 15.0f})
                           .has_value());

    // Entirely in front, nothing is clipped and both ends survive as they are.
    const std::optional<std::pair<StudioVector2, StudioVector2>> ahead =
        projectSegment(camera, StudioVector3{-5.0f, 0.0f, -10.0f}, StudioVector3{5.0f, 0.0f, -10.0f});
    CNA_STUDIO_EXPECT(ahead.has_value());
    if (ahead) { CNA_STUDIO_EXPECT(ahead->first.x < ahead->second.x); }
}

CNA_STUDIO_TEST(TheSceneGridIsCentredOnThePivotAndMarksTheAxes)
{
    StudioCamera3D camera = makeCamera();
    camera.setPitch(0.6f);

    WireframeOptions options;
    options.gridSpacing = 10.0f;
    options.gridHalfExtent = 5;

    const std::vector<WireSegment> atOrigin = buildSceneGrid(camera, options);

    // Eleven lines each way, minus whatever the near plane took. The axes must be among them, or
    // the view has no landmark at all: an orbiting camera with no origin marker is disorienting
    // in a way no amount of grid is.
    CNA_STUDIO_EXPECT(!atOrigin.empty());
    const auto hasColor = [&atOrigin](const StudioColor& color) {
        return std::any_of(atOrigin.begin(), atOrigin.end(),
                           [&color](const WireSegment& segment) { return segment.color == color; });
    };
    CNA_STUDIO_EXPECT(hasColor(WireColors::kAxisX));
    CNA_STUDIO_EXPECT(hasColor(WireColors::kAxisY));

    // Flying away carries the grid: snapped to the spacing, so the lines do not shimmer, but
    // centred on the pivot, so a level laid out far from the origin still has a floor.
    camera.setPivot(StudioVector3{1000.0f, 1000.0f, 0.0f});
    const std::vector<WireSegment> farAway = buildSceneGrid(camera, options);
    CNA_STUDIO_EXPECT(!farAway.empty());

    // No spacing given means one is chosen from the camera's distance, exactly as the 2D grid
    // chooses one from its zoom -- and it must be a round number a user can read coordinates off.
    WireframeOptions automatic;
    automatic.gridHalfExtent = 4;
    camera.setPivot(StudioVector3{});
    CNA_STUDIO_EXPECT(!buildSceneGrid(camera, automatic).empty());
}

/**
 * The grid dissolves at its rim instead of stopping (`plan.md` STUDIO-11005).
 *
 * It used to draw forty-nine lines each way at full strength and then nothing, which puts a bright
 * square edge across the middle of a scene -- and, in any view that is not straight down, a solid
 * aliased band where the far lines converge. The fade is what turns the square into a disc.
 */
CNA_STUDIO_TEST(TheGridFadesOutTowardsItsRimRatherThanStopping)
{
    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});

    WireframeOptions options;
    options.gridSpacing = 10.0f;
    options.gridHalfExtent = 8;

    const std::vector<WireSegment> faded = buildSceneGrid(camera, options);
    CNA_STUDIO_EXPECT(!faded.empty());

    // Every line of one colour family, gathered by alpha: the grid's own dim grey, so the axis and
    // major lines -- which have colours of their own -- do not muddle the comparison.
    std::uint8_t strongest = 0;
    std::uint8_t weakest = 255;
    for (const WireSegment& segment : faded)
    {
        if (segment.color.r != WireColors::kGrid.r || segment.color.g != WireColors::kGrid.g
            || segment.color.b != WireColors::kGrid.b)
        {
            continue;
        }
        strongest = std::max(strongest, segment.color.a);
        weakest = std::min(weakest, segment.color.a);
    }

    // Something is at full strength near the middle and something is nearly gone further out,
    // which is the whole claim.
    CNA_STUDIO_EXPECT_EQ(strongest, WireColors::kGrid.a);
    CNA_STUDIO_EXPECT(weakest < WireColors::kGrid.a / 2);

    // Nothing is emitted at zero strength: a fully transparent segment is work the renderer does
    // to draw nothing, and the grid is the one thing in the scene there are hundreds of.
    for (const WireSegment& segment : faded) { CNA_STUDIO_EXPECT(segment.color.a > 0); }

    // The discriminating assertion, and the one it is easy to write a test that misses: a *single
    // line* has to vary in strength along its length. The X axis is the case that proves it,
    // because it passes through the grid's centre -- so a fade measured only from a line to the
    // centre, rather than from each piece of it, leaves this one line at full strength end to end.
    std::uint8_t axisStrongest = 0;
    std::uint8_t axisWeakest = 255;
    std::size_t axisPieces = 0;
    for (const WireSegment& segment : faded)
    {
        if (segment.color.r != WireColors::kAxisX.r || segment.color.g != WireColors::kAxisX.g
            || segment.color.b != WireColors::kAxisX.b)
        {
            continue;
        }
        ++axisPieces;
        axisStrongest = std::max(axisStrongest, segment.color.a);
        axisWeakest = std::min(axisWeakest, segment.color.a);
    }

    CNA_STUDIO_EXPECT(axisPieces > 1);
    CNA_STUDIO_EXPECT_EQ(axisStrongest, WireColors::kAxisX.a);
    if (axisWeakest >= axisStrongest)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
                                     "the X axis line is one strength from end to end, so the grid "
                                     "fades between its lines rather than along them.");
    }

    // Turning the fade off gives the old grid back exactly -- one strength everywhere -- which is
    // what makes this an option rather than a new opinion baked in.
    WireframeOptions solid = options;
    solid.gridFadeStart = 0.0f;
    solid.gridFadeSteps = 1;

    const std::vector<WireSegment> unfaded = buildSceneGrid(camera, solid);
    CNA_STUDIO_EXPECT(!unfaded.empty());
    for (const WireSegment& segment : unfaded)
    {
        if (segment.color.r == WireColors::kGrid.r && segment.color.g == WireColors::kGrid.g
            && segment.color.b == WireColors::kGrid.b)
        {
            CNA_STUDIO_EXPECT_EQ(segment.color.a, WireColors::kGrid.a);
        }
    }
}

CNA_STUDIO_TEST(TheGridsSegmentCountIsWhatTheSubdivisionSaysItIs)
{
    // The fade costs segments, because a `WireSegment` carries one colour and a line that fades
    // along its length has to be more than one. Counted rather than assumed: this is the one thing
    // in a 3D frame there are hundreds of, and a subdivision quietly multiplying it is the kind of
    // cost that shows up as a frame rate rather than as a number anybody looked at.
    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});

    WireframeOptions single;
    single.gridSpacing = 10.0f;
    single.gridHalfExtent = 6;
    single.gridFadeStart = 0.0f;
    single.gridFadeSteps = 1;

    const std::size_t lines = buildSceneGrid(camera, single).size();
    CNA_STUDIO_EXPECT(lines > 0);

    WireframeOptions divided = single;
    divided.gridFadeSteps = 6;

    // Six pieces a line, so six times the segments -- no more, because nothing else changed, and
    // no fewer, because the fade is off and so nothing is dropped for being transparent.
    CNA_STUDIO_EXPECT_EQ(buildSceneGrid(camera, divided).size(), lines * 6u);

    // With the fade on, the corners of the square fall outside the disc and are dropped, so the
    // count comes *down* from the full multiple rather than up.
    WireframeOptions round = divided;
    round.gridFadeStart = 0.45f;
    const std::size_t rounded = buildSceneGrid(camera, round).size();
    CNA_STUDIO_EXPECT(rounded > lines);
    CNA_STUDIO_EXPECT(rounded < lines * 6u);
}

namespace
{
    /** @brief A closed cube of twelve triangles, centred on the origin with edge length @p size. */
    MeshData makeCubeMesh(float size)
    {
        const float h = size * 0.5f;
        const StudioVector3 corners[8] = {
            {-h, -h, -h}, {h, -h, -h}, {h, h, -h}, {-h, h, -h},
            {-h, -h, h},  {h, -h, h},  {h, h, h},  {-h, h, h},
        };

        // Wound counter-clockwise seen from outside, which is what the importer guarantees and what
        // the silhouette's facing test reads.
        const std::uint32_t faces[12][3] = {
            {0, 2, 1}, {0, 3, 2},  // -Z
            {4, 5, 6}, {4, 6, 7},  // +Z
            {0, 1, 5}, {0, 5, 4},  // -Y
            {3, 7, 6}, {3, 6, 2},  // +Y
            {0, 4, 7}, {0, 7, 3},  // -X
            {1, 2, 6}, {1, 6, 5},  // +X
        };

        MeshPart part;
        part.name = "Cube";
        for (const StudioVector3& corner : corners)
        {
            MeshVertex vertex;
            vertex.position = corner;
            part.vertices.push_back(vertex);
        }
        for (const auto& face : faces)
        {
            part.indices.push_back(face[0]);
            part.indices.push_back(face[1]);
            part.indices.push_back(face[2]);
        }

        MeshData mesh;
        mesh.parts.push_back(std::move(part));
        recomputeMeshBounds(mesh);
        return mesh;
    }

    /** @brief An entity a `MeshProvider` will answer for: a transform and a model reference. */
    Uuid addModelEntity(SceneDocument& scene, const Uuid& modelId)
    {
        StudioEntity prop{Uuid::generate(), "Crate"};
        prop.addComponent(StudioComponent{BuiltinComponentIds::kTransform});

        StudioComponent renderer{BuiltinComponentIds::kModelRenderer};
        renderer.setProperty("model", PropertyValue{PropertyValue::AssetReference{modelId}});
        prop.addComponent(std::move(renderer));

        return scene.addEntity(std::move(prop));
    }
}

/**
 * A band in 3D is the extent of eight projected corners, not of two (`plan.md` STUDIO-12009).
 *
 * A box in the world is not a box on the screen: under any view that is not straight down an axis,
 * projecting `min` and `max` alone misses the extent by however much the box is turned. That is the
 * same mistake `transformBounds3D` exists to avoid, one projection further along, and the case that
 * catches it is a camera looking at a corner.
 */
/**
 * The 3D picker honours a lock too (`plan.md` STUDIO-13005).
 *
 * Both 3D paths, because they are separate loops over the scene and a rule applied to one of them
 * is a lock that holds for a click and not for a band -- which is the easier of the two ways to
 * move something somebody locked precisely so it would stop moving.
 */
CNA_STUDIO_TEST(TheThreeDimensionalPickerLeavesLockedEntitiesAlone)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid near = scene.addEntity(makeSpriteEntity(registry, "Near", 0.0f, 0.0f));
    const Uuid far = scene.addEntity(makeSpriteEntity(registry, "Far", 400.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setDistance(700.0f);

    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{60.0f, 60.0f}; };

    const std::optional<WorldBounds3D> bounds = computeEntityBounds3D(scene, near, sizes);
    CNA_STUDIO_EXPECT(bounds.has_value());
    if (!bounds) { return; }
    const std::optional<StudioVector2> at = camera.worldToScreen(bounds->getCenter());
    CNA_STUDIO_EXPECT(at.has_value());
    if (!at) { return; }

    CNA_STUDIO_EXPECT(pickEntityAt3D(scene, camera, *at, sizes) == near);

    scene.findEntityForEdit(near)->setStudioState(kStudioLockedKey, PropertyValue{true});
    CNA_STUDIO_EXPECT(!pickEntityAt3D(scene, camera, *at, sizes).isValid());

    // The band is a second loop and needs the rule of its own.
    const std::vector<Uuid> banded = pickEntitiesIn3D(
        scene, camera, StudioVector2{0.0f, 0.0f}, StudioVector2{1600.0f, 900.0f}, sizes);
    CNA_STUDIO_EXPECT_EQ(banded.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(banded.front() == far);
}

/**
 * A lock lives in the studio state and is a command like everything else (`plan.md` STUDIO-13005).
 */
CNA_STUDIO_TEST(LockingAnEntityIsUndoableAndLeavesNothingBehindWhenUndone)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid entityId = scene.addEntity(makeSpriteEntity(registry, "Crate", 0.0f, 0.0f));
    CNA_STUDIO_EXPECT(!isEntityLocked(scene, entityId));

    SetEntityLockedCommand lock{scene, entityId, true};
    CNA_STUDIO_EXPECT(lock.isValid());
    lock.execute();
    CNA_STUDIO_EXPECT(isEntityLocked(scene, entityId));
    CNA_STUDIO_EXPECT(isEntityLockedItself(*scene.findEntity(entityId)));

    // Undo puts the key back the way it was, which for an entity nobody had touched means *gone*.
    // Writing `"locked": false` instead would leave the flag on every entity the user ever clicked
    // and make the scene file's diff unreadable.
    lock.undo();
    CNA_STUDIO_EXPECT(!isEntityLocked(scene, entityId));
    CNA_STUDIO_EXPECT(scene.findEntity(entityId)->getStudioState().find(kStudioLockedKey)
                      == scene.findEntity(entityId)->getStudioState().end());

    // A command that changes nothing is refused rather than executed: an undo stack with no-ops in
    // it makes Ctrl+Z appear to do nothing, and the user cannot tell how many more to press.
    CNA_STUDIO_EXPECT(!SetEntityLockedCommand(scene, entityId, false).isValid());
    lock.execute();
    CNA_STUDIO_EXPECT(!SetEntityLockedCommand(scene, entityId, true).isValid());
    CNA_STUDIO_EXPECT(SetEntityLockedCommand(scene, entityId, false).isValid());

    // An entity the scene does not have is not something to lock.
    CNA_STUDIO_EXPECT(!SetEntityLockedCommand(scene, Uuid::generate(), true).isValid());
}

/**
 * And it survives a round trip through the scene file (`plan.md` STUDIO-13005).
 *
 * Free, because the studio state already round-trips -- which is the reason the flag lives there
 * and not beside `enabled`. Asserted anyway: "free" is a claim about a mechanism somewhere else,
 * and a lock that quietly dropped on save would be the kind of loss a user finds a week later.
 */
CNA_STUDIO_TEST(ALockSurvivesBeingSavedAndLoaded)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid entityId = scene.addEntity(makeSpriteEntity(registry, "Crate", 0.0f, 0.0f));
    scene.findEntityForEdit(entityId)->setStudioState(kStudioLockedKey, PropertyValue{true});

    std::vector<std::string> warnings;
    const StudioEntity read =
        entityFromJson(entityToJson(*scene.findEntity(entityId)), registry, warnings);

    CNA_STUDIO_EXPECT(warnings.empty());
    CNA_STUDIO_EXPECT(isEntityLockedItself(read));
}

CNA_STUDIO_TEST(ABandInThreeDimensionsSweepsWhatTheCameraCanSee)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid left = scene.addEntity(makeSpriteEntity(registry, "Left", -150.0f, 0.0f));
    const Uuid middle = scene.addEntity(makeSpriteEntity(registry, "Middle", 0.0f, 0.0f));
    const Uuid right = scene.addEntity(makeSpriteEntity(registry, "Right", 150.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setDistance(700.0f);

    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{60.0f, 60.0f}; };

    const auto screenOf = [&](const Uuid& id) {
        const std::optional<WorldBounds3D> bounds = computeEntityBounds3D(scene, id, sizes);
        return camera.worldToScreen(bounds->getCenter());
    };

    const std::optional<StudioVector2> leftAt = screenOf(left);
    const std::optional<StudioVector2> rightAt = screenOf(right);
    CNA_STUDIO_EXPECT(leftAt.has_value() && rightAt.has_value());
    if (!leftAt || !rightAt) { return; }

    // The whole panel: all three.
    const std::vector<Uuid> everything = pickEntitiesIn3D(
        scene, camera, StudioVector2{0.0f, 0.0f}, StudioVector2{1600.0f, 900.0f}, sizes);
    CNA_STUDIO_EXPECT_EQ(everything.size(), std::size_t{3});

    // A band around the middle one alone, taken from where it is actually drawn.
    const std::optional<StudioVector2> middleAt = screenOf(middle);
    CNA_STUDIO_EXPECT(middleAt.has_value());
    if (!middleAt) { return; }

    const std::vector<Uuid> one = pickEntitiesIn3D(
        scene, camera, StudioVector2{middleAt->x - 10.0f, middleAt->y - 10.0f},
        StudioVector2{middleAt->x + 10.0f, middleAt->y + 10.0f}, sizes);
    CNA_STUDIO_EXPECT_EQ(one.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(one.front() == middle);

    // Empty space takes nothing, and a disabled entity is passed over.
    CNA_STUDIO_EXPECT(pickEntitiesIn3D(scene, camera, StudioVector2{2.0f, 2.0f},
                                       StudioVector2{6.0f, 6.0f}, sizes)
                          .empty());

    scene.findEntityForEdit(middle)->setEnabled(false);
    CNA_STUDIO_EXPECT_EQ(pickEntitiesIn3D(scene, camera, StudioVector2{0.0f, 0.0f},
                                          StudioVector2{1600.0f, 900.0f}, sizes)
                             .size(),
                         std::size_t{2});
    scene.findEntityForEdit(middle)->setEnabled(true);

    // Now the case about eight corners. A model turned to face a corner of the view projects wider
    // than its `min` and `max` do, so a band placed just outside those two must still catch it.
    SceneDocument boxed;
    const Uuid modelId = Uuid::generate();
    const Uuid crate = addModelEntity(boxed, modelId);
    const MeshData cube = makeCubeMesh(100.0f);
    const MeshProvider meshes = [&](const Uuid& which) -> const MeshData* {
        return which == modelId ? &cube : nullptr;
    };

    StudioCamera3D corner = makeCamera();
    corner.setPivot(StudioVector3{});
    corner.setDistance(400.0f);
    corner.setYaw(0.7853982f);    // 45 degrees
    corner.setPitch(0.6154797f);  // atan(1 / sqrt(2)): the isometric corner

    const std::optional<WorldBounds3D> bounds = computeEntityBounds3D(boxed, crate, sizes, meshes);
    CNA_STUDIO_EXPECT(bounds.has_value());
    if (!bounds) { return; }

    const std::optional<StudioVector2> low = corner.worldToScreen(bounds->min);
    const std::optional<StudioVector2> high = corner.worldToScreen(bounds->max);
    CNA_STUDIO_EXPECT(low.has_value() && high.has_value());
    if (!low || !high) { return; }

    // A one-pixel band just outside the further of those two projected corners, on the side the
    // cube's other corners reach. An implementation that projected only `min` and `max` would put
    // the cube's screen extent inside that pair and find nothing here.
    const float beyond = std::max(low->x, high->x) + 8.0f;
    const std::optional<StudioVector2> center = corner.worldToScreen(bounds->getCenter());
    CNA_STUDIO_EXPECT(center.has_value());
    if (!center) { return; }

    const std::vector<Uuid> caught =
        pickEntitiesIn3D(boxed, corner, StudioVector2{beyond, center->y - 1.0f},
                         StudioVector2{beyond + 1.0f, center->y + 1.0f}, sizes, meshes);
    CNA_STUDIO_EXPECT_EQ(caught.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(caught.front() == crate);
}

/**
 * An imported model is measured by its mesh, and was measured by an eight-unit box (STUDIO-11008).
 *
 * The defect this fixes is not subtle once it is named: `computeEntityBounds3D` had no way to ask
 * for a model's geometry, so a model fell through to the box every icon-drawn entity gets -- eight
 * units around its origin. The model was *drawn* at whatever size its mesh is. So a click landed on
 * a large model only near its middle, and Focus Selected framed the eight-unit box and put the
 * camera inside the thing it was asked to look at.
 */
CNA_STUDIO_TEST(AModelIsMeasuredByItsMeshRatherThanByTheBoxAnIconGets)
{
    SceneDocument scene;
    const Uuid modelId = Uuid::generate();
    const Uuid id = addModelEntity(scene, modelId);

    const MeshData cube = makeCubeMesh(100.0f);
    const MeshProvider meshes = [&](const Uuid& which) -> const MeshData* {
        return which == modelId ? &cube : nullptr;
    };
    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{0.0f, 0.0f}; };

    // Without a provider: the fallback box, eight units each way. Unchanged, deliberately -- it is
    // also the honest answer for a model whose mesh has not finished importing.
    const std::optional<WorldBounds3D> unmeasured = computeEntityBounds3D(scene, id, sizes);
    CNA_STUDIO_EXPECT(unmeasured.has_value());
    if (unmeasured)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(unmeasured->min.x, -8.0f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(unmeasured->max.x, 8.0f));
    }

    // With one: the mesh, which is a hundred units across and therefore more than six times what
    // the fallback claimed in each direction.
    const std::optional<WorldBounds3D> measured = computeEntityBounds3D(scene, id, sizes, meshes);
    CNA_STUDIO_EXPECT(measured.has_value());
    if (measured)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(measured->min.x, -50.0f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(measured->max.y, 50.0f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(measured->max.z, 50.0f));
    }

    // The scene's bounds and the hierarchy's follow from the same answer, which is what makes Focus
    // Selected and the first switch into 3D frame the model rather than a speck at its origin.
    const std::optional<WorldBounds3D> whole = computeSceneBounds3D(scene, sizes, meshes);
    CNA_STUDIO_EXPECT(whole.has_value());
    if (whole) { CNA_STUDIO_EXPECT(cameraNearlyEqual(whole->max.x, 50.0f)); }

    // A click at the model's edge. Along -Z towards the cube's +Z face, offset to x = 40 -- inside
    // the mesh, well outside the eight-unit box. The ray is built directly rather than through the
    // camera so the case is about the bounds and not about a projection.
    const WorldRay edge{StudioVector3{40.0f, 0.0f, 200.0f}, StudioVector3{0.0f, 0.0f, -1.0f}};
    CNA_STUDIO_EXPECT(intersectRayWithBounds(edge, *measured).has_value());
    CNA_STUDIO_EXPECT(!intersectRayWithBounds(edge, *unmeasured).has_value());
}

/**
 * A rotated model's box is re-bounded about its corners rather than about two of them.
 *
 * The mistake worth catching: transforming only `min` and `max` and calling the result a box. Under
 * any rotation those two corners no longer span the shape, so the box comes out too small and
 * off-centre -- and it is *invisible* until something is rotated, because an unrotated box
 * transforms correctly either way.
 */
CNA_STUDIO_TEST(ARotatedModelsBoxContainsTheWholeModel)
{
    SceneDocument scene;
    const Uuid modelId = Uuid::generate();
    const Uuid id = addModelEntity(scene, modelId);

    const MeshData cube = makeCubeMesh(100.0f);
    const MeshProvider meshes = [&](const Uuid& which) -> const MeshData* {
        return which == modelId ? &cube : nullptr;
    };
    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{0.0f, 0.0f}; };

    // An eighth of a turn about Z: the worst case for an axis-aligned bound of a square, which
    // grows from 100 across to 100 * sqrt(2).
    constexpr float kEighthTurn = 0.39269908f;  // pi / 8 -- half the angle, as a quaternion takes it
    StudioComponent* transform =
        scene.findEntityForEdit(id)->findComponent(BuiltinComponentIds::kTransform);
    CNA_STUDIO_EXPECT(transform != nullptr);
    if (transform == nullptr) { return; }
    transform->setProperty("rotation", PropertyValue{StudioQuaternion{0.0f, 0.0f,
                                                                      std::sin(kEighthTurn),
                                                                      std::cos(kEighthTurn)}});

    const std::optional<WorldBounds3D> rotated = computeEntityBounds3D(scene, id, sizes, meshes);
    CNA_STUDIO_EXPECT(rotated.has_value());
    if (!rotated) { return; }

    constexpr float kHalfDiagonal = 70.71068f;  // 50 * sqrt(2)
    CNA_STUDIO_EXPECT(cameraNearlyEqual(rotated->max.x, kHalfDiagonal, 0.05f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(rotated->min.x, -kHalfDiagonal, 0.05f));

    // Z is the rotation axis, so it does not grow. Asserted because a re-bound that grew every
    // axis equally would pass the two above and still be wrong.
    CNA_STUDIO_EXPECT(cameraNearlyEqual(rotated->max.z, 50.0f, 0.05f));

    // Every corner of the mesh, placed by the same transform, is inside. The property rather than
    // the eight numbers: that is what a bounding box promises.
    const StudioMatrix matrix = toWorldMatrix(*computeWorldTransform(scene, id));
    for (const MeshVertex& vertex : cube.parts.front().vertices)
    {
        const StudioVector3 world = transformPosition(matrix, vertex.position);
        CNA_STUDIO_EXPECT(world.x >= rotated->min.x - 0.05f && world.x <= rotated->max.x + 0.05f);
        CNA_STUDIO_EXPECT(world.y >= rotated->min.y - 0.05f && world.y <= rotated->max.y + 0.05f);
        CNA_STUDIO_EXPECT(world.z >= rotated->min.z - 0.05f && world.z <= rotated->max.z + 0.05f);
    }
}

/**
 * The bounds overlay draws the volume the editor measures with (STUDIO-11008).
 *
 * A model is drawn as its mesh and a badge is drawn at a fixed pixel size, so in both cases the box
 * that a click is tested against and that Focus frames is invisible. The overlay is that box. An
 * entity already drawn as exactly that box gets no second copy of it, which is the part of the rule
 * that is worth a test rather than a comment.
 */
CNA_STUDIO_TEST(TheBoundsOverlayDrawsWhatTheEditorMeasuresWith)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid modelId = Uuid::generate();
    const Uuid model = addModelEntity(scene, modelId);
    const Uuid sprite = scene.addEntity(makeSpriteEntity(registry, "Backdrop", 300.0f, 0.0f));

    const MeshData cube = makeCubeMesh(40.0f);

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{150.0f, 0.0f, 0.0f});
    camera.setDistance(900.0f);

    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{64.0f, 64.0f}; };

    WireframeOptions options;
    options.drawGrid = false;
    options.meshProvider = [&](const Uuid& which) -> const MeshData* {
        return which == modelId ? &cube : nullptr;
    };

    const auto countBounds = [](const WireframeResult& result) {
        return static_cast<std::size_t>(
            std::count_if(result.segments.begin(), result.segments.end(),
                          [](const WireSegment& segment) {
                              return segment.color.r == WireColors::kBounds.r
                                  && segment.color.g == WireColors::kBounds.g
                                  && segment.color.b == WireColors::kBounds.b;
                          }));
    };

    // Off by default, and off is off: not one segment wears the overlay colour.
    const WireframeResult none = buildSceneWireframe(scene, camera, {}, sizes, options);
    CNA_STUDIO_EXPECT_EQ(countBounds(none), std::size_t{0});

    // All: the model gets a box, because a mesh is not a box. The sprite gets none, because the box
    // it is already drawn as *is* its bounds -- twelve edges, not twenty-four.
    options.boundsOverlay = BoundsDisplay::All;
    const WireframeResult all = buildSceneWireframe(scene, camera, {}, sizes, options);
    CNA_STUDIO_EXPECT_EQ(countBounds(all), std::size_t{12});

    // Selected: the same box, and only when the model is the thing selected.
    options.boundsOverlay = BoundsDisplay::Selected;
    const WireframeResult selectedSprite = buildSceneWireframe(scene, camera, {sprite}, sizes, options);
    CNA_STUDIO_EXPECT_EQ(countBounds(selectedSprite), std::size_t{0});

    const WireframeResult selectedModel = buildSceneWireframe(scene, camera, {model}, sizes, options);
    CNA_STUDIO_EXPECT_EQ(countBounds(selectedModel), std::size_t{12});

    // The overlay adds to what was there rather than replacing it: the model is still drawn as a
    // model. Compared against the same scene with the overlay off, so this cannot pass by the mesh
    // having quietly stopped being drawn.
    options.boundsOverlay = BoundsDisplay::None;
    const WireframeResult plain = buildSceneWireframe(scene, camera, {model}, sizes, options);
    CNA_STUDIO_EXPECT_EQ(selectedModel.segments.size(), plain.segments.size() + 12);
}

/**
 * The bounding sphere is the one `BoundingSphere::CreateFromBoundingBox` builds (STUDIO-11008).
 *
 * CNA's collision, like XNA's, is `BoundingBox` and `BoundingSphere` and the tests on them -- there
 * is no collider component, and Phase 26 owns integrating a physics system rather than Studio
 * inventing one. So the collision volume a CNA game actually tests *is* the bounding sphere, and
 * the thing a user cannot guess from the box is how much bigger it is: around anything long and
 * thin the sphere reaches the far corner and swallows the empty space beside the object.
 */
CNA_STUDIO_TEST(TheBoundingSphereIsTheOneACollisionTestWouldUse)
{
    SceneDocument scene;
    const Uuid modelId = Uuid::generate();
    addModelEntity(scene, modelId);

    // Long in X, thin in Y and Z: the shape whose sphere is nothing like its box.
    MeshData plank = makeCubeMesh(200.0f);
    for (MeshVertex& vertex : plank.parts.front().vertices)
    {
        vertex.position.y *= 0.05f;
        vertex.position.z *= 0.05f;
    }
    recomputeMeshBounds(plank);

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setDistance(1200.0f);

    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{0.0f, 0.0f}; };

    WireframeOptions options;
    options.drawGrid = false;
    options.drawMeshEdges = false;
    options.meshProvider = [&](const Uuid& which) -> const MeshData* {
        return which == modelId ? &plank : nullptr;
    };
    options.boundsOverlay = BoundsDisplay::All;

    const WireframeResult boxOnly = buildSceneWireframe(scene, camera, {}, sizes, options);

    options.drawBoundingSpheres = true;
    const WireframeResult withSphere = buildSceneWireframe(scene, camera, {}, sizes, options);

    // Three rings of twenty-four segments each. Three rather than the light gizmo's one: a light's
    // ring is a boundary being aimed and two of three would collapse edge-on into lines through
    // its badge, while a sphere's collapsed rings are its silhouette, which is the truth about it.
    CNA_STUDIO_EXPECT_EQ(withSphere.segments.size(), boxOnly.segments.size() + 72);

    // And it is *much* bigger than the box in the thin direction. The camera looks down -Z with the
    // plank lying across the screen, so the box is a sliver ten units tall and the sphere is a
    // circle reaching the plank's far corner, a hundred units away. Measured over the overlay's own
    // segments, because the claim is about the two volumes and not about the badge drawn with them.
    const auto overlaySpan = [](const WireframeResult& result) {
        float low = std::numeric_limits<float>::max();
        float high = -std::numeric_limits<float>::max();
        for (const WireSegment& segment : result.segments)
        {
            if (segment.color.r != WireColors::kBounds.r
                || segment.color.g != WireColors::kBounds.g
                || segment.color.b != WireColors::kBounds.b)
            {
                continue;
            }
            low = std::min({low, segment.from.y, segment.to.y});
            high = std::max({high, segment.from.y, segment.to.y});
        }
        return high - low;
    };

    // Eight times is a floor rather than the figure: the true ratio here is nearer twenty, and the
    // margin is what stops the plausible wrong answer -- a sphere sized from the box's *smallest*
    // dimension, a tenth of the right radius -- from passing.
    CNA_STUDIO_EXPECT(overlaySpan(boxOnly) > 0.0f);
    CNA_STUDIO_EXPECT(overlaySpan(withSphere) > overlaySpan(boxOnly) * 8.0f);
}

/**
 * A selected model is outlined rather than filled in with the selection colour (STUDIO-11007).
 *
 * The selection used to recolour *every* edge of the mesh, which on anything denser than a crate
 * does not read as a selection at all -- it reads as the object turning into a solid block of the
 * selection colour. The outline is the silhouette: edges where the two triangles sharing them face
 * opposite ways, plus the rim of an open shell.
 */
CNA_STUDIO_TEST(ASelectedModelIsOutlinedRatherThanFilledWithTheSelectionColour)
{
    SceneDocument scene;
    const Uuid modelId = Uuid::generate();
    const Uuid id = addModelEntity(scene, modelId);

    const MeshData cube = makeCubeMesh(10.0f);

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setDistance(60.0f);

    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{0.0f, 0.0f}; };

    WireframeOptions options;
    options.drawGrid = false;
    options.meshProvider = [&](const Uuid& which) -> const MeshData* {
        return which == modelId ? &cube : nullptr;
    };

    // The shaded mode: no mesh edges at all. An unselected cube is its box and nothing more.
    options.drawMeshEdges = false;
    const WireframeResult unselected = buildSceneWireframe(scene, camera, {}, sizes, options);

    const auto countSelected = [](const WireframeResult& result) {
        return static_cast<std::size_t>(
            std::count_if(result.segments.begin(), result.segments.end(),
                          [](const WireSegment& segment) {
                              return segment.color == WireColors::kSelected;
                          }));
    };

    const WireframeResult selected = buildSceneWireframe(scene, camera, {id}, sizes, options);

    // Nothing wears the selection colour until something is selected.
    CNA_STUDIO_EXPECT_EQ(countSelected(unselected), std::size_t{0});
    CNA_STUDIO_EXPECT(countSelected(selected) > 0);

    // Selecting *replaces* the bounds box with the outline rather than adding to it, so the count
    // goes down. That is the right trade and it is worth pinning: a box and an outline together
    // would be two marks for one selection, and the outline traces the object where the box only
    // says roughly where it is.
    CNA_STUDIO_EXPECT(selected.segments.size() < unselected.segments.size());

    // A cube square on to a face shows four of its twelve edges on the silhouette, and from a
    // corner six. Either way it is fewer than the twelve a full wireframe draws: an outline is a
    // subset of the edges, and one that grows with the shape rather than with the triangle count.
    const std::size_t outlineEdges = countSelected(selected);
    CNA_STUDIO_EXPECT(outlineEdges >= 4);
    CNA_STUDIO_EXPECT(outlineEdges < 12);

    // And turning the outline off leaves the shaded mode with no mesh marking at all, which is what
    // makes this an option rather than a behaviour nobody can decline.
    WireframeOptions noOutline = options;
    noOutline.drawSelectionOutline = false;
    const WireframeResult plain = buildSceneWireframe(scene, camera, {id}, sizes, noOutline);
    CNA_STUDIO_EXPECT_EQ(plain.segments.size(), unselected.segments.size());
}

CNA_STUDIO_TEST(TheOutlineFollowsTheCameraRatherThanBeingFixedToTheMesh)
{
    // A silhouette is a function of where you are looking from -- that is what separates it from a
    // list of edges somebody marked once. Orbiting to a corner shows more of a cube's edges than
    // facing it square on, and the outline has to change with it.
    SceneDocument scene;
    const Uuid modelId = Uuid::generate();
    const Uuid id = addModelEntity(scene, modelId);

    const MeshData cube = makeCubeMesh(10.0f);

    WireframeOptions options;
    options.drawGrid = false;
    options.drawMeshEdges = false;
    options.meshProvider = [&](const Uuid& which) -> const MeshData* {
        return which == modelId ? &cube : nullptr;
    };
    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{0.0f, 0.0f}; };

    StudioCamera3D square = makeCamera();
    square.setPivot(StudioVector3{});
    square.setDistance(60.0f);

    StudioCamera3D corner = square;
    corner.setYaw(0.785398f);   // 45 degrees
    corner.setPitch(0.615479f); // atan(1/sqrt(2)), which looks down a body diagonal

    const std::size_t squareOn =
        buildSceneWireframe(scene, square, {id}, sizes, options).segments.size();
    const std::size_t fromCorner =
        buildSceneWireframe(scene, corner, {id}, sizes, options).segments.size();

    if (squareOn == fromCorner)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
                                     "the outline is the same from square on and from a corner, so "
                                     "it is a fixed set of edges rather than a silhouette.");
    }

    // Square on to a face, exactly four edges of a cube are on the silhouette: the square you see.
    CNA_STUDIO_EXPECT(squareOn >= 4);
    CNA_STUDIO_EXPECT(fromCorner > squareOn);
}

CNA_STUDIO_TEST(TheWireframeBoxesEveryEntityAndMarksTheSelection)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid firstId = scene.addEntity(makeSpriteEntity(registry, "First", 0.0f, 0.0f));
    const Uuid secondId = scene.addEntity(makeSpriteEntity(registry, "Second", 60.0f, 0.0f));
    const Uuid disabledId = scene.addEntity(makeSpriteEntity(registry, "Disabled", -60.0f, 0.0f));
    scene.findEntityForEdit(disabledId)->setEnabled(false);

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setDistance(400.0f);
    camera.setPitch(0.5f);

    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{32.0f, 32.0f}; };

    WireframeOptions options;
    options.drawGrid = false;

    const WireframeResult result = buildSceneWireframe(scene, camera, {secondId}, sizes, options);

    // Two enabled entities, twelve edges each. A disabled entity is not drawn, for the same reason
    // it gets no icon in the 2D viewport: it is not part of the running game.
    CNA_STUDIO_EXPECT_EQ(result.entitiesDrawn, std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(result.segments.size(), std::size_t{24});
    CNA_STUDIO_EXPECT(!result.truncated);

    const std::size_t selected =
        static_cast<std::size_t>(std::count_if(result.segments.begin(), result.segments.end(),
                                               [](const WireSegment& segment) {
                                                   return segment.color == WireColors::kSelected;
                                               }));
    CNA_STUDIO_EXPECT_EQ(selected, std::size_t{12});
    static_cast<void>(firstId);

    // The ceiling is announced rather than reached quietly, or a half-drawn scene reads as a scene
    // with half its entities missing.
    WireframeOptions tiny = options;
    tiny.maxSegments = 5;
    CNA_STUDIO_EXPECT(buildSceneWireframe(scene, camera, {}, sizes, tiny).truncated);
}

CNA_STUDIO_TEST(PickingInThreeDimensionsTakesTheNearestBox)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    // Two entities one behind the other along the view direction. The near one has to win --
    // "nearest" rather than the 2D viewport's "topmost", because depth is a real quantity here
    // and layer order is not.
    const Uuid nearId = scene.addEntity(makeEntity(registry, "Near", 0.0f, 0.0f));
    scene.findEntityForEdit(nearId)->findComponent(BuiltinComponentIds::kTransform)
        ->setProperty("position", PropertyValue{StudioVector3{0.0f, 0.0f, 40.0f}});

    const Uuid farId = scene.addEntity(makeEntity(registry, "Far", 0.0f, 0.0f));
    scene.findEntityForEdit(farId)->findComponent(BuiltinComponentIds::kTransform)
        ->setProperty("position", PropertyValue{StudioVector3{0.0f, 0.0f, -40.0f}});

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(200.0f);

    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{32.0f, 32.0f}; };
    const StudioVector2 centre{800.0f, 450.0f};

    CNA_STUDIO_EXPECT(pickEntityAt3D(scene, camera, centre, sizes) == nearId);

    // Turned around, the other one is nearest. Nothing about the entities changed.
    camera.setYaw(3.14159265f);
    CNA_STUDIO_EXPECT(pickEntityAt3D(scene, camera, centre, sizes) == farId);

    // A ray into empty space hits nothing, and says so with the nil Uuid rather than the last
    // entity it happened to test.
    CNA_STUDIO_EXPECT(!pickEntityAt3D(scene, camera, StudioVector2{5.0f, 5.0f}, sizes).isValid());
}

CNA_STUDIO_TEST(TheSlabTestHandlesFlatBoxesAndRaysStartingInside)
{
    const WorldBounds3D box{StudioVector3{-1.0f, -1.0f, -1.0f}, StudioVector3{1.0f, 1.0f, 1.0f}};

    const WorldRay towards{StudioVector3{0.0f, 0.0f, 10.0f}, StudioVector3{0.0f, 0.0f, -1.0f}};
    const std::optional<float> hit = intersectRayWithBounds(towards, box);
    CNA_STUDIO_EXPECT(hit.has_value());
    CNA_STUDIO_EXPECT(cameraNearlyEqual(*hit, 9.0f));

    // Behind the ray is a miss, not a hit at a negative distance -- which would let a click select
    // whatever happened to be behind the camera.
    const WorldRay away{StudioVector3{0.0f, 0.0f, 10.0f}, StudioVector3{0.0f, 0.0f, 1.0f}};
    CNA_STUDIO_EXPECT(!intersectRayWithBounds(away, box).has_value());

    // Starting inside is a hit at zero distance, not a miss.
    const WorldRay inside{StudioVector3{}, StudioVector3{1.0f, 0.0f, 0.0f}};
    const std::optional<float> fromInside = intersectRayWithBounds(inside, box);
    CNA_STUDIO_EXPECT(fromInside.has_value());
    CNA_STUDIO_EXPECT(cameraNearlyEqual(*fromInside, 0.0f));

    // A sprite is a box with no thickness, so the axis-parallel case is the common one rather
    // than the exotic one: get it wrong and no sprite can be clicked from the side.
    const WorldBounds3D flat{StudioVector3{-1.0f, -1.0f, 0.0f}, StudioVector3{1.0f, 1.0f, 0.0f}};
    CNA_STUDIO_EXPECT(intersectRayWithBounds(towards, flat).has_value());

    const WorldRay parallel{StudioVector3{0.0f, 0.0f, 5.0f}, StudioVector3{1.0f, 0.0f, 0.0f}};
    CNA_STUDIO_EXPECT(!intersectRayWithBounds(parallel, flat).has_value());
}

CNA_STUDIO_TEST(TheThreeDimensionalGizmoIsSizedInPixelsAndGrabbedInPixels)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.6f);
    camera.setPitch(0.4f);
    camera.setDistance(100.0f);

    const std::optional<TranslateGizmo3DLayout> near =
        computeTranslateGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(near.has_value());
    if (!near) { return; }

    // Twice as far away, the arms are twice as long in world units -- which is what keeps them the
    // same length on screen, the property that makes them grabbable at any distance.
    camera.setDistance(200.0f);
    const std::optional<TranslateGizmo3DLayout> far =
        computeTranslateGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(far.has_value());
    if (!far) { return; }

    CNA_STUDIO_EXPECT(cameraNearlyEqual(far->armLength / near->armLength, 2.0f, 0.05f));

    const float nearScreenLength =
        length(subtract(StudioVector3{near->screenTips[0].x, near->screenTips[0].y, 0.0f},
                        StudioVector3{near->screenOrigin.x, near->screenOrigin.y, 0.0f}));
    const float farScreenLength =
        length(subtract(StudioVector3{far->screenTips[0].x, far->screenTips[0].y, 0.0f},
                        StudioVector3{far->screenOrigin.x, far->screenOrigin.y, 0.0f}));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(nearScreenLength, farScreenLength, 2.0f));

    // Grabbing is decided on the screen: on an arm is a grab, well away from all three is not.
    const StudioVector2 alongX{(far->screenOrigin.x + far->screenTips[0].x) * 0.5f,
                               (far->screenOrigin.y + far->screenTips[0].y) * 0.5f};
    CNA_STUDIO_EXPECT(hitTestTranslateGizmo3D(*far, alongX) == GizmoAxis3D::X);
    CNA_STUDIO_EXPECT(hitTestTranslateGizmo3D(*far, StudioVector2{5.0f, 5.0f}) == GizmoAxis3D::None);

    // An entity behind the eye has no manipulator at all, rather than one at a plausible-looking
    // screen position the user would be able to grab.
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(100.0f);
    camera.setPivot(StudioVector3{0.0f, 0.0f, -200.0f});  // Eye at z = -100, looking further away.
    CNA_STUDIO_EXPECT(!computeTranslateGizmo3DLayout(scene, camera, entityId).has_value());
}

/**
 * The translate gizmo has plane handles, and did not (`plan.md` STUDIO-12001).
 *
 * Three bare lines was the whole of it, so sliding an object across a floor took two drags along
 * two arms and landed wherever the second one stopped. The 2D gizmo has had its `Both` centre
 * handle since ED-401 for exactly this; in three dimensions there are three planes to choose, and
 * which one the user wants is the question they answer by reaching for a square.
 */
CNA_STUDIO_TEST(TheTranslateGizmoHasAPlaneHandleForEachPairOfArms)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.6f);
    camera.setPitch(0.4f);
    camera.setDistance(100.0f);

    const std::optional<TranslateGizmo3DLayout> layout =
        computeTranslateGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    // Three squares, each with the normal of the arm it leaves out: XY's normal is Z.
    for (std::size_t index = 0; index < 3; ++index)
    {
        CNA_STUDIO_EXPECT(layout->planes[index].visible);
        const StudioVector3 expected = layout->axes[(index + 2) % 3];
        CNA_STUDIO_EXPECT(cameraNearlyEqual(layout->planes[index].normal.x, expected.x, 0.001f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(layout->planes[index].normal.y, expected.y, 0.001f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(layout->planes[index].normal.z, expected.z, 0.001f));
    }

    // The middle of the XY square is a grab on that plane.
    const auto centreOf = [](const GizmoPlaneHandle3D& plane) {
        StudioVector2 total;
        for (const StudioVector2& corner : plane.screenCorners)
        {
            total.x += corner.x * 0.25f;
            total.y += corner.y * 0.25f;
        }
        return total;
    };

    CNA_STUDIO_EXPECT(hitTestTranslateGizmo3D(*layout, centreOf(layout->planes[0]))
                      == GizmoAxis3D::XY);
    CNA_STUDIO_EXPECT(hitTestTranslateGizmo3D(*layout, centreOf(layout->planes[1]))
                      == GizmoAxis3D::YZ);
    CNA_STUDIO_EXPECT(hitTestTranslateGizmo3D(*layout, centreOf(layout->planes[2]))
                      == GizmoAxis3D::ZX);

    // **The arms still win where they overlap.** The squares start a quarter of the way out, so a
    // press along an arm's own length is still the arm's -- which is what stops the planes making
    // an axis drag unreachable, and is the failure this would have if the two were tested the other
    // way round.
    const StudioVector2 alongX{(layout->screenOrigin.x + layout->screenTips[0].x) * 0.5f,
                               (layout->screenOrigin.y + layout->screenTips[0].y) * 0.5f};
    CNA_STUDIO_EXPECT(hitTestTranslateGizmo3D(*layout, alongX) == GizmoAxis3D::X);

    // The origin itself is not in any square: they are offset, so the point where the three arms
    // meet still belongs to whichever arm is nearest rather than to a plane.
    CNA_STUDIO_EXPECT(!isGizmoPlane(hitTestTranslateGizmo3D(*layout, layout->screenOrigin)));

    // Well away from everything is still nothing at all.
    CNA_STUDIO_EXPECT(hitTestTranslateGizmo3D(*layout, StudioVector2{5.0f, 5.0f})
                      == GizmoAxis3D::None);

    // Drawn as well as grabbed, and where they are drawn is where they are grabbed: three arms of
    // one segment and three squares of four.
    const std::vector<WireSegment> segments = buildTranslateGizmo3DSegments(*layout);
    CNA_STUDIO_EXPECT_EQ(segments.size(), std::size_t{15});
}

/**
 * A plane drag moves on two axes at once and leaves the third exactly alone (STUDIO-12001).
 *
 * The third axis is the assertion that matters. A plane drag solved as "wherever the ray happens to
 * be" moves the entity off the plane as soon as the cursor leaves the square, which is the failure
 * that makes a plane handle worse than two axis drags rather than better.
 */
CNA_STUDIO_TEST(APlaneDragMovesOnTwoAxesAndLeavesTheThirdAlone)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.6f);
    camera.setPitch(0.4f);
    camera.setDistance(100.0f);

    const std::optional<TranslateGizmo3DLayout> layout =
        computeTranslateGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    StudioVector2 grab;
    for (const StudioVector2& corner : layout->planes[0].screenCorners)
    {
        grab.x += corner.x * 0.25f;
        grab.y += corner.y * 0.25f;
    }

    TranslateGizmo3DDrag drag;
    CNA_STUDIO_EXPECT(drag.begin(scene, camera, *layout, entityId, grab));
    CNA_STUDIO_EXPECT(drag.getAxis() == GizmoAxis3D::XY);

    // Not moved yet is not an edit, exactly as an axis drag promises.
    CNA_STUDIO_EXPECT(!drag.update(scene, camera, grab, GizmoSnap{}).has_value());

    const std::optional<StudioVector3> moved =
        drag.update(scene, camera, StudioVector2{grab.x + 50.0f, grab.y + 30.0f}, GizmoSnap{});
    CNA_STUDIO_EXPECT(moved.has_value());
    if (!moved) { return; }

    // Z untouched, X and Y both changed: the plane's own two axes, and nothing else.
    CNA_STUDIO_EXPECT(cameraNearlyEqual(moved->z, 0.0f, 0.001f));
    CNA_STUDIO_EXPECT(std::fabs(moved->x) > 0.5f);
    CNA_STUDIO_EXPECT(std::fabs(moved->y) > 0.5f);

    // Snapping rounds both in-plane axes and still leaves the third where it was.
    GizmoSnap snap;
    snap.translate = 10.0f;
    const std::optional<StudioVector3> snapped =
        drag.update(scene, camera, StudioVector2{grab.x + 50.0f, grab.y + 30.0f}, snap);
    CNA_STUDIO_EXPECT(snapped.has_value());
    if (snapped)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(std::fmod(snapped->x, 10.0f), 0.0f, 0.001f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(std::fmod(snapped->y, 10.0f), 0.0f, 0.001f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(snapped->z, 0.0f, 0.001f));
    }

    // A plane seen exactly edge-on has no answer, and the drag refuses rather than inventing one --
    // the same refusal `closestPointOnAxis` makes for an arm pointing at the camera.
    const WorldRay acrossZ{StudioVector3{0.0f, 0.0f, 100.0f}, StudioVector3{1.0f, 0.0f, 0.0f}};
    CNA_STUDIO_EXPECT(
        !intersectRayWithPlane(acrossZ, StudioVector3{}, StudioVector3{0.0f, 0.0f, 1.0f})
             .has_value());

    // A ray pointed away from the plane meets it behind the eye, which is not an answer either: an
    // entity dragged to that point would jump to a mirror image of where the cursor is.
    const WorldRay away{StudioVector3{0.0f, 0.0f, 100.0f}, StudioVector3{0.0f, 0.0f, 1.0f}};
    CNA_STUDIO_EXPECT(
        !intersectRayWithPlane(away, StudioVector3{}, StudioVector3{0.0f, 0.0f, 1.0f}).has_value());

    // And the ordinary case is where the geometry says: straight down at (30, 40) meets the XY
    // plane at exactly that point.
    const WorldRay downZ{StudioVector3{30.0f, 40.0f, 100.0f}, StudioVector3{0.0f, 0.0f, -1.0f}};
    const std::optional<StudioVector3> hit =
        intersectRayWithPlane(downZ, StudioVector3{}, StudioVector3{0.0f, 0.0f, 1.0f});
    CNA_STUDIO_EXPECT(hit.has_value());
    if (hit)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(hit->x, 30.0f, 0.001f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(hit->y, 40.0f, 0.001f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(hit->z, 0.0f, 0.001f));
    }
}

CNA_STUDIO_TEST(AThreeDimensionalDragFollowsTheCursorAlongTheGrabbedAxis)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(100.0f);

    const std::optional<TranslateGizmo3DLayout> layout =
        computeTranslateGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    // Looking down -Z with no pitch: world +X is screen right, so grabbing the X arm and dragging
    // right must move the entity along +X. A sign error here is the classic gizmo bug -- it works
    // and moves everything the wrong way.
    const StudioVector2 grab{(layout->screenOrigin.x + layout->screenTips[0].x) * 0.5f,
                             layout->screenOrigin.y};

    TranslateGizmo3DDrag drag;
    CNA_STUDIO_EXPECT(drag.begin(scene, camera, *layout, entityId, grab));
    CNA_STUDIO_EXPECT(drag.getAxis() == GizmoAxis3D::X);

    // Not moved yet: a drag that has not moved must push nothing, or one Ctrl+Z is spent undoing
    // a change the user cannot see.
    CNA_STUDIO_EXPECT(!drag.update(scene, camera, grab, GizmoSnap{}).has_value());

    const std::optional<StudioVector3> moved =
        drag.update(scene, camera, StudioVector2{grab.x + 60.0f, grab.y}, GizmoSnap{});
    CNA_STUDIO_EXPECT(moved.has_value());
    if (!moved) { return; }

    CNA_STUDIO_EXPECT(moved->x > 1.0f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(moved->y, 0.0f, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(moved->z, 0.0f, 0.001f));

    // Snapping rounds the result rather than the movement, so the entity lands on the grid rather
    // than a grid-sized distance from where it happened to start.
    GizmoSnap snap;
    snap.translate = 10.0f;
    const std::optional<StudioVector3> snapped =
        drag.update(scene, camera, StudioVector2{grab.x + 60.0f, grab.y}, snap);
    CNA_STUDIO_EXPECT(snapped.has_value());
    if (snapped) { CNA_STUDIO_EXPECT(cameraNearlyEqual(std::fmod(snapped->x, 10.0f), 0.0f, 0.001f)); }

    // An arm pointing straight at the camera has no answer -- a pixel of movement would otherwise
    // fling the entity across the level -- so the drag refuses rather than inventing one.
    const WorldRay downZ{StudioVector3{0.0f, 0.0f, 100.0f}, StudioVector3{0.0f, 0.0f, -1.0f}};
    CNA_STUDIO_EXPECT(
        !closestPointOnAxis(downZ, StudioVector3{}, StudioVector3{0.0f, 0.0f, 1.0f}).has_value());

    // And the ordinary case has the answer the geometry says: a ray straight down at x = 30 meets
    // the X axis thirty units along it.
    const WorldRay downwards{StudioVector3{30.0f, 50.0f, 0.0f}, StudioVector3{0.0f, -1.0f, 0.0f}};
    const std::optional<float> where =
        closestPointOnAxis(downwards, StudioVector3{}, StudioVector3{1.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(where.has_value());
    if (where) { CNA_STUDIO_EXPECT(cameraNearlyEqual(*where, 30.0f)); }
}

CNA_STUDIO_TEST(AThreeDimensionalDragOfAChildStoresTheParentRelativePosition)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid parentId = scene.addEntity(makeEntity(registry, "Rig", 0.0f, 0.0f));
    scene.findEntityForEdit(parentId)->findComponent(BuiltinComponentIds::kTransform)
        ->setProperty("rotation", PropertyValue{quaternionFromEulerDegrees(StudioVector3{0.0f, 90.0f, 0.0f})});

    const Uuid childId = scene.addEntity(makeEntity(registry, "Mount", 0.0f, 0.0f));
    CNA_STUDIO_EXPECT(scene.reparentEntity(childId, parentId));

    // A world delta along X, under a parent turned 90 degrees about Y, is a local delta along the
    // child's own Z. A gizmo that skipped this works perfectly on roots and drifts on every child.
    const StudioVector3 local = worldDeltaToLocal3D(scene, childId, StudioVector3{10.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(cameraNearlyEqual(std::abs(local.z), 10.0f, 0.01f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(local.x, 0.0f, 0.01f));

    // A root entity is its own frame, so the delta passes through untouched.
    CNA_STUDIO_EXPECT(worldDeltaToLocal3D(scene, parentId, StudioVector3{10.0f, 0.0f, 0.0f})
                      == (StudioVector3{10.0f, 0.0f, 0.0f}));
}

CNA_STUDIO_TEST(TheThreeDimensionalViewAgreesWithTheTwoDimensionalOneAboutWhichWayIsDown)
{
    // The property the whole Y-down decision exists for: an entity below another one in the 2D
    // viewport is below it in the 3D one too. Switching views moves the camera, not the scene.
    StudioCamera2D flat;
    flat.setViewportSize(StudioVector2{1600.0f, 900.0f});

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(500.0f);

    const StudioVector3 lower{0.0f, 200.0f, 0.0f};  // y grows downward, as SpriteBatch has it.
    const StudioVector3 higher{0.0f, -200.0f, 0.0f};

    CNA_STUDIO_EXPECT(flat.worldToScreen(StudioVector2{lower.x, lower.y}).y
                      > flat.worldToScreen(StudioVector2{higher.x, higher.y}).y);

    const std::optional<StudioVector2> lowerOnScreen = camera.worldToScreen(lower);
    const std::optional<StudioVector2> higherOnScreen = camera.worldToScreen(higher);
    CNA_STUDIO_EXPECT(lowerOnScreen.has_value() && higherOnScreen.has_value());
    if (!lowerOnScreen || !higherOnScreen) { return; }

    CNA_STUDIO_EXPECT(lowerOnScreen->y > higherOnScreen->y);

    // And so does X, which is the half a mirrored "up" vector would have broken: a 180-degree roll
    // fixes the vertical and puts world +X on the left, which is why the mirror is in the
    // projection instead.
    const std::optional<StudioVector2> right = camera.worldToScreen(StudioVector3{200.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(right.has_value());
    if (right) { CNA_STUDIO_EXPECT(right->x > 800.0f); }

    CNA_STUDIO_EXPECT(flat.worldToScreen(StudioVector2{200.0f, 0.0f}).x > 800.0f);

    // A ray still comes back to the pixel it came from: the mirror is in the view-projection both
    // directions go through, so picking cannot disagree with what is drawn.
    const StudioVector2 pixel{1100.0f, 300.0f};
    const std::optional<StudioVector2> back = camera.worldToScreen(camera.screenToRay(pixel).at(400.0f));
    CNA_STUDIO_EXPECT(back.has_value());
    if (back)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(back->x, pixel.x, 0.5f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(back->y, pixel.y, 0.5f));
    }
}

CNA_STUDIO_TEST(EntitiesThatDrawNothingGetABadgeRatherThanACube)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid cameraId = scene.addEntity(makeEntity(registry, "Main Camera", 0.0f, 0.0f));
    StudioComponent cameraComponent{BuiltinComponentIds::kCamera};
    cameraComponent.applyDefaults(*registry.find(BuiltinComponentIds::kCamera));
    scene.findEntityForEdit(cameraId)->addComponent(std::move(cameraComponent));

    const Uuid emptyId = scene.addEntity(makeEntity(registry, "Spawn Point", 200.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{100.0f, 0.0f, 0.0f});
    camera.setDistance(600.0f);

    const SpriteSizeProvider sizes = [](const Uuid&) { return StudioVector2{32.0f, 32.0f}; };

    WireframeOptions options;
    options.drawGrid = false;

    const WireframeResult result = buildSceneWireframe(scene, camera, {}, sizes, options);
    CNA_STUDIO_EXPECT_EQ(result.entitiesDrawn, std::size_t{2});

    // Both are badges, and different ones: the camera's silhouette against the marker's cross, so
    // the two cannot be confused for each other. The bare Transform was a box until `STUDIO-11009`
    // -- which meant a spawn point read as a tiny piece of geometry rather than as a place, and a
    // scene of ten of them was ten identical cubes.
    const std::size_t cameraSegments =
        buildIconBadge(StudioIconKind::Camera, StudioVector2{}, WireColors::kEntity).size();
    const std::size_t markerSegments =
        buildIconBadge(StudioIconKind::Empty, StudioVector2{}, WireColors::kEntity).size();
    CNA_STUDIO_EXPECT(cameraSegments > 0);
    CNA_STUDIO_EXPECT(markerSegments > 0);
    CNA_STUDIO_EXPECT(cameraSegments != markerSegments);
    CNA_STUDIO_EXPECT_EQ(result.segments.size(), cameraSegments + markerSegments);

    // Every icon kind draws something, and None draws nothing at all -- a badge for "this entity
    // has no icon" would be a badge on every entity in the scene.
    for (const StudioIconKind kind : {StudioIconKind::Camera, StudioIconKind::Light,
                                      StudioIconKind::AudioSource, StudioIconKind::Model,
                                      StudioIconKind::Empty})
    {
        CNA_STUDIO_EXPECT(!buildIconBadge(kind, StudioVector2{400.0f, 300.0f}, WireColors::kEntity).empty());
    }
    CNA_STUDIO_EXPECT(buildIconBadge(StudioIconKind::None, StudioVector2{}, WireColors::kEntity).empty());

    // The badge is a fixed size in pixels: it must not change when the camera moves away, or a
    // camera at the far end of a level becomes invisible and one nearby swallows the screen.
    const std::vector<WireSegment> near =
        buildIconBadge(StudioIconKind::Light, StudioVector2{100.0f, 100.0f}, WireColors::kEntity);
    const std::vector<WireSegment> far =
        buildIconBadge(StudioIconKind::Light, StudioVector2{900.0f, 700.0f}, WireColors::kEntity);
    CNA_STUDIO_EXPECT_EQ(near.size(), far.size());
    CNA_STUDIO_EXPECT(cameraNearlyEqual(near.front().to.x - near.front().from.x,
                                        far.front().to.x - far.front().from.x));

    static_cast<void>(emptyId);
}

/**
 * A rotate ring shows its front half only (`plan.md` STUDIO-12002).
 *
 * Three full circles drawn over each other are a tangle, and the half of a ring on the far side of
 * the object turns the opposite way on screen from the half in front -- so a press that lands on
 * the back reads as the gizmo working backwards. Hiding it makes the three rings read as a ball,
 * and makes the only grabbable half the one whose direction matches the drag.
 */
CNA_STUDIO_TEST(ARotateRingShowsAndIsGrabbedOnItsFrontHalfOnly)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.6f);
    camera.setPitch(0.4f);
    camera.setDistance(100.0f);

    const std::optional<RotateGizmo3DLayout> layout =
        computeRotateGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    const StudioVector3 towardsEye = normalize(subtract(camera.getEye(), layout->origin));

    std::size_t tested = 0;
    for (std::size_t index = 0; index < 3; ++index)
    {
        const std::vector<StudioVector2>& ring = layout->rings[index];
        if (ring.empty()) { continue; }

        // An arc, not a circle: a tilted ring keeps about half of its samples and is left open.
        CNA_STUDIO_EXPECT(ring.size() < static_cast<std::size_t>(kRotateGizmo3DSamples));
        CNA_STUDIO_EXPECT(ring.size() > 4);

        const StudioVector3 normal = layout->axes[index];

        // The nearest and furthest points *on this ring*: the in-plane component of the direction
        // to the eye, and its opposite. Computed from the normal alone rather than from the ring's
        // own basis, which is private to the layout and arbitrary anyway.
        const StudioVector3 inPlane =
            subtract(towardsEye, scale(normal, dot(towardsEye, normal)));
        if (length(inPlane) < 0.1f) { continue; }

        const StudioVector3 unit = normalize(inPlane);
        const std::optional<StudioVector2> nearest =
            camera.worldToScreen(add(layout->origin, scale(unit, layout->radius)));
        const std::optional<StudioVector2> furthest =
            camera.worldToScreen(add(layout->origin, scale(unit, -layout->radius)));
        CNA_STUDIO_EXPECT(nearest.has_value() && furthest.has_value());
        if (!nearest || !furthest) { continue; }

        // The front of the ring is this ring's to grab...
        CNA_STUDIO_EXPECT(hitTestRotateGizmo3D(*layout, *nearest) == axis3DForTest(index));

        // ...and the back of it is not. Not "is None", because a ring's far side can pass close to
        // another ring's near side -- what matters is that the hidden half is no longer a handle
        // for the ring it belongs to.
        CNA_STUDIO_EXPECT(hitTestRotateGizmo3D(*layout, *furthest) != axis3DForTest(index));
        ++tested;
    }

    CNA_STUDIO_EXPECT(tested > 0);

    // A ring seen face-on keeps the whole of itself. Every one of its samples is level with the
    // centre, so none of them is behind it, and cutting one arbitrarily in half would leave the one
    // ring the user can see best looking broken.
    StudioCamera3D straight = makeCamera();
    straight.setPivot(StudioVector3{});
    straight.setYaw(0.0f);
    straight.setPitch(0.0f);
    straight.setDistance(100.0f);

    const std::optional<RotateGizmo3DLayout> faceOn =
        computeRotateGizmo3DLayout(scene, straight, entityId);
    CNA_STUDIO_EXPECT(faceOn.has_value());
    if (!faceOn) { return; }

    // Looking down -Z: the Z ring lies across the view and the other two are edge-on, which the
    // layout already drops rather than drawing a line through the middle of the gizmo.
    CNA_STUDIO_EXPECT(faceOn->rings[0].empty());
    CNA_STUDIO_EXPECT(faceOn->rings[1].empty());
    CNA_STUDIO_EXPECT_EQ(faceOn->rings[2].size(),
                         static_cast<std::size_t>(kRotateGizmo3DSamples) + 1);

    // Closed: the last sample is the first again, so the circle has no gap in it.
    CNA_STUDIO_EXPECT(cameraNearlyEqual(faceOn->rings[2].front().x, faceOn->rings[2].back().x, 0.01f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(faceOn->rings[2].front().y, faceOn->rings[2].back().y, 0.01f));
}

CNA_STUDIO_TEST(TheThreeDimensionalRotateRingsAreGrabbedWhereTheyAreDrawn)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(300.0f);

    const std::optional<RotateGizmo3DLayout> layout =
        computeRotateGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    // The Z ring faces the camera, so it is drawn in full and is the one a click lands on.
    const std::vector<StudioVector2>& zRing = layout->rings[2];
    CNA_STUDIO_EXPECT(zRing.size() > 8);

    CNA_STUDIO_EXPECT(hitTestRotateGizmo3D(*layout, zRing.front()) != GizmoAxis3D::None);
    CNA_STUDIO_EXPECT(hitTestRotateGizmo3D(*layout, StudioVector2{800.0f, 450.0f}) == GizmoAxis3D::None);

    // What is drawn is what can be grabbed: the segments are the same polyline the hit-test walks,
    // so a ring the user can see is a ring the user can take hold of.
    const std::vector<WireSegment> segments = buildRotateGizmo3DSegments(*layout);
    CNA_STUDIO_EXPECT(!segments.empty());
    CNA_STUDIO_EXPECT(hitTestRotateGizmo3D(*layout, segments.front().from) != GizmoAxis3D::None);

    // Dragging around the ring turns the entity, and the turn is snapped rather than the angle --
    // snapping the absolute angle would straighten whatever was grabbed the moment it was grabbed.
    RotateGizmo3DDrag drag;
    CNA_STUDIO_EXPECT(drag.begin(scene, camera, *layout, entityId, zRing.front()));

    const StudioVector2 quarter = zRing[zRing.size() / 4];
    const std::optional<StudioQuaternion> turned = drag.update(scene, camera, quarter, GizmoSnap{});
    CNA_STUDIO_EXPECT(turned.has_value());
    if (!turned) { return; }

    CNA_STUDIO_EXPECT(cameraNearlyEqual(std::abs(zRotationOf(*turned)), 1.5707963f, 0.05f));

    GizmoSnap snap;
    snap.rotate = kDefaultRotationSnap;
    const std::optional<StudioQuaternion> snapped = drag.update(scene, camera, quarter, snap);
    CNA_STUDIO_EXPECT(snapped.has_value());
    if (snapped)
    {
        const float degrees = zRotationOf(*snapped) * 57.29578f;
        CNA_STUDIO_EXPECT(cameraNearlyEqual(std::fmod(std::abs(degrees), 15.0f), 0.0f, 0.05f));
    }

    // Not moved is no edit at all: an undo entry restoring a rotation the entity already had
    // costs the user a Ctrl+Z to reach a change they can see.
    CNA_STUDIO_EXPECT(!drag.update(scene, camera, zRing.front(), GizmoSnap{}).has_value());
}

CNA_STUDIO_TEST(TheGridIsDrawnOnWhicheverPlaneTheOptionsName)
{
    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(500.0f);

    WireframeOptions scenePlane;
    scenePlane.gridSpacing = 100.0f;
    scenePlane.gridHalfExtent = 8;

    WireframeOptions ground = scenePlane;
    ground.gridPlane = GridPlane::Ground;

    const std::vector<WireSegment> sceneLines = buildSceneGrid(camera, scenePlane);
    const std::vector<WireSegment> groundLines = buildSceneGrid(camera, ground);

    CNA_STUDIO_EXPECT(!sceneLines.empty());
    CNA_STUDIO_EXPECT(!groundLines.empty());

    const auto verticalSpan = [](const std::vector<WireSegment>& segments) {
        float lowest = segments.front().from.y;
        float highest = lowest;
        for (const WireSegment& segment : segments)
        {
            for (const float y : {segment.from.y, segment.to.y})
            {
                lowest = std::min(lowest, y);
                highest = std::max(highest, y);
            }
        }
        return highest - lowest;
    };

    // A camera with no pitch is *in* the ground plane, so it looks along it edge-on and sees one
    // line where the scene's own plane fills the view. That is the whole reason the choice exists
    // rather than a floor being drawn unconditionally.
    CNA_STUDIO_EXPECT(verticalSpan(sceneLines) > 200.0f);
    CNA_STUDIO_EXPECT(verticalSpan(groundLines) < 2.0f);

    const auto contains = [](const std::vector<WireSegment>& segments, const StudioColor& color) {
        for (const WireSegment& segment : segments)
        {
            if (segment.color == color) { return true; }
        }
        return false;
    };

    // Each plane names the axis running down the middle of it, so the origin is findable in both.
    CNA_STUDIO_EXPECT(contains(sceneLines, WireColors::kAxisY));
    CNA_STUDIO_EXPECT(!contains(sceneLines, WireColors::kAxisZ));
    CNA_STUDIO_EXPECT(contains(groundLines, WireColors::kAxisZ));
    CNA_STUDIO_EXPECT(contains(groundLines, WireColors::kAxisX));

    // Tipped over the floor, the two swap roles: the ground grid opens out and the scene's own
    // plane is the one seen edge-on.
    camera.setPitch(1.4f);
    CNA_STUDIO_EXPECT(verticalSpan(buildSceneGrid(camera, ground)) > 200.0f);
}

CNA_STUDIO_TEST(AThreeDimensionalSelectionTurnsAndGrowsAboutItsSharedPivot)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const Uuid leftId = scene.addEntity(makeEntity(registry, "Left", -100.0f, 0.0f));
    const Uuid rightId = scene.addEntity(makeEntity(registry, "Right", 100.0f, 0.0f));

    const std::optional<StudioVector3> pivot = computeSelectionPivot3D(scene, {leftId, rightId});
    CNA_STUDIO_EXPECT(pivot.has_value());
    if (!pivot) { return; }

    CNA_STUDIO_EXPECT(cameraNearlyEqual(pivot->x, 0.0f, 0.001f));

    MultiTransform3D drag;
    CNA_STUDIO_EXPECT(drag.begin(scene, {leftId, rightId}, *pivot));
    CNA_STUDIO_EXPECT_EQ(drag.getEntityCount(), std::size_t{2});

    // A quarter turn about world Z carries both members *around* the pivot as well as turning
    // them, which is the difference between turning an arrangement and spinning each of its parts
    // where it stands.
    const std::vector<EntityTransformEdit> turned =
        drag.rotate(scene, StudioVector3{0.0f, 0.0f, 1.0f}, 1.5707963f);
    CNA_STUDIO_EXPECT_EQ(turned.size(), std::size_t{2});

    for (const EntityTransformEdit& edit : turned)
    {
        CNA_STUDIO_EXPECT(edit.position.has_value());
        CNA_STUDIO_EXPECT(edit.rotation.has_value());
        if (!edit.position || !edit.rotation) { continue; }

        const float expectedY = edit.entityId == leftId ? -100.0f : 100.0f;
        CNA_STUDIO_EXPECT(cameraNearlyEqual(edit.position->x, 0.0f, 0.01f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(edit.position->y, expectedY, 0.01f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(zRotationOf(*edit.rotation), 1.5707963f, 0.01f));
    }

    // Scaling grows the distances from the pivot with the entities themselves. Without that half,
    // a group scaled up stays where it was and overlaps itself.
    const std::array<StudioVector3, 3> axes{StudioVector3{1.0f, 0.0f, 0.0f},
                                            StudioVector3{0.0f, 1.0f, 0.0f},
                                            StudioVector3{0.0f, 0.0f, 1.0f}};
    const std::vector<EntityTransformEdit> grown =
        drag.scale(scene, axes, StudioVector3{2.0f, 1.0f, 1.0f});
    CNA_STUDIO_EXPECT_EQ(grown.size(), std::size_t{2});

    for (const EntityTransformEdit& edit : grown)
    {
        CNA_STUDIO_EXPECT(edit.position.has_value());
        CNA_STUDIO_EXPECT(edit.scale.has_value());
        if (!edit.position || !edit.scale) { continue; }

        const float expectedX = edit.entityId == leftId ? -200.0f : 200.0f;
        CNA_STUDIO_EXPECT(cameraNearlyEqual(edit.position->x, expectedX, 0.01f));

        // Only the axis the gesture named: the other two are left exactly as they were.
        CNA_STUDIO_EXPECT(cameraNearlyEqual(edit.scale->x, 2.0f, 0.01f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(edit.scale->y, 1.0f, 0.001f));
    }

    // A child of a selected entity is carried by its parent, so taking it on its own account as
    // well would transform it twice. Only the selection's roots are captured.
    CNA_STUDIO_EXPECT(scene.reparentEntity(rightId, leftId));
    MultiTransform3D nested;
    CNA_STUDIO_EXPECT(nested.begin(scene, {leftId, rightId}, *pivot));
    CNA_STUDIO_EXPECT_EQ(nested.getEntityCount(), std::size_t{1});
}

CNA_STUDIO_TEST(TheThreeDimensionalScaleArmsShortenRatherThanVanishWhenSeenEdgeOn)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(300.0f);

    const std::optional<ScaleGizmo3DLayout> straightOn = computeScaleGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(straightOn.has_value());
    if (!straightOn) { return; }

    // Looking straight down the Z axis at an entity in the middle of the screen: X and Y lie across
    // the view, and Z runs exactly through the eye, where it projects onto its own origin and has
    // no direction to be dragged along at all. That is the one case an arm is dropped.
    CNA_STUDIO_EXPECT(straightOn->armVisible[0]);
    CNA_STUDIO_EXPECT(straightOn->armVisible[1]);
    CNA_STUDIO_EXPECT(!straightOn->armVisible[2]);

    // The pixels it vacated belong to the centre handle, so the middle of the gizmo still does
    // something -- and it is the thing a press in the middle of a gizmo means.
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*straightOn, straightOn->screenOrigin) == GizmoAxis3D::All);
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*straightOn, straightOn->screenHandles[0]) == GizmoAxis3D::X);
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*straightOn, StudioVector2{5.0f, 5.0f}) == GizmoAxis3D::None);

    // A degree of orbit and the arm is back: shortened to its floor and faded almost out, but on
    // screen and grabbable. This is where the rotate gizmo hides a ring instead -- a turn needs a
    // plane to measure an angle in, and a scale needs only a ratio, which the screen always has.
    camera.setYaw(0.02f);
    const std::optional<ScaleGizmo3DLayout> tilted = computeScaleGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(tilted.has_value());
    if (!tilted) { return; }

    CNA_STUDIO_EXPECT(tilted->armVisible[2]);

    const float pixels = std::hypot(tilted->screenHandles[2].x - tilted->screenOrigin.x,
                                    tilted->screenHandles[2].y - tilted->screenOrigin.y);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(pixels, kScaleGizmo3DMinimumArmPixels, 0.5f));
    CNA_STUDIO_EXPECT(pixels > tilted->centerExtent + tilted->handleExtent);
    CNA_STUDIO_EXPECT(tilted->armFade[2] < 0.2f);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(tilted->armFade[0], 1.0f, 0.001f));

    // What is drawn is what can be grabbed, the same invariant the rings hold: the handle sits on
    // the X arm's line at this angle, and the square still wins, because a square is what the eye
    // was aiming at.
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*tilted, tilted->screenHandles[2]) == GizmoAxis3D::Z);
    CNA_STUDIO_EXPECT(!buildScaleGizmo3DSegments(*tilted, GizmoAxis3D::Z).empty());
}

CNA_STUDIO_TEST(AThreeDimensionalScaleDragIsARatioOfScreenDistances)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.6f);
    camera.setPitch(0.4f);
    camera.setDistance(300.0f);

    const std::optional<ScaleGizmo3DLayout> layout = computeScaleGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    const StudioVector2 handle = layout->screenHandles[0];
    const StudioVector2 arm{handle.x - layout->screenOrigin.x, handle.y - layout->screenOrigin.y};

    ScaleGizmo3DDrag drag;
    CNA_STUDIO_EXPECT(drag.begin(scene, *layout, entityId, handle));
    CNA_STUDIO_EXPECT(drag.getAxis() == GizmoAxis3D::X);

    // Not moved is no edit at all.
    CNA_STUDIO_EXPECT(!drag.update(*layout, handle, GizmoSnap{}).has_value());

    // Twice as far out along the arm is twice the size -- a ratio, which is the only measure of a
    // unitless quantity that means the same thing at every camera distance.
    const StudioVector2 twiceOut{layout->screenOrigin.x + arm.x * 2.0f,
                                 layout->screenOrigin.y + arm.y * 2.0f};
    const std::optional<StudioVector3> doubled = drag.update(*layout, twiceOut, GizmoSnap{});
    CNA_STUDIO_EXPECT(doubled.has_value());
    if (!doubled) { return; }

    CNA_STUDIO_EXPECT(cameraNearlyEqual(doubled->x, 2.0f, 0.01f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(doubled->y, 1.0f, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(doubled->z, 1.0f, 0.001f));

    // Dragging back through the origin flips the entity, which is a legitimate edit XNA's own
    // negative scale supports -- but it never lands on zero, where the entity would be invisible
    // *and* unclickable, with nothing left to grab to get it back.
    const StudioVector2 behind{layout->screenOrigin.x - arm.x, layout->screenOrigin.y - arm.y};
    const std::optional<StudioVector3> flipped = drag.update(*layout, behind, GizmoSnap{});
    CNA_STUDIO_EXPECT(flipped.has_value());
    if (flipped) { CNA_STUDIO_EXPECT(flipped->x < 0.0f); }

    const std::optional<StudioVector3> atOrigin =
        drag.update(*layout, layout->screenOrigin, GizmoSnap{});
    CNA_STUDIO_EXPECT(atOrigin.has_value());
    if (atOrigin) { CNA_STUDIO_EXPECT(atOrigin->x != 0.0f); }

    // Snapped to tenths, and the *factor* is what is rounded: the shared quantity, so a selection
    // cannot end up at sizes that are round one at a time and not in proportion to each other.
    GizmoSnap snap;
    snap.scale = kDefaultScaleSnap;
    const StudioVector2 awkward{layout->screenOrigin.x + arm.x * 1.73f,
                                layout->screenOrigin.y + arm.y * 1.73f};
    const std::optional<StudioVector3> snapped = drag.update(*layout, awkward, snap);
    CNA_STUDIO_EXPECT(snapped.has_value());
    if (snapped) { CNA_STUDIO_EXPECT(cameraNearlyEqual(snapped->x, 1.7f, 0.001f)); }

    // The centre handle scales all three at once, which is the commonest scale there is.
    ScaleGizmo3DDrag uniform;
    const StudioVector2 nearCentre{layout->screenOrigin.x + 8.0f, layout->screenOrigin.y};
    CNA_STUDIO_EXPECT(uniform.begin(scene, *layout, entityId, nearCentre));
    CNA_STUDIO_EXPECT(uniform.getAxis() == GizmoAxis3D::All);

    const std::optional<StudioVector3> bigger =
        uniform.update(*layout, StudioVector2{layout->screenOrigin.x + 16.0f, layout->screenOrigin.y},
                       GizmoSnap{});
    CNA_STUDIO_EXPECT(bigger.has_value());
    if (bigger)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(bigger->x, 2.0f, 0.01f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(bigger->y, 2.0f, 0.01f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(bigger->z, 2.0f, 0.01f));
    }

    // A press exactly on the origin is not a drag: every factor is a division by how far out the
    // grab was, so a grab at the centre would scale by infinity. The press falls through instead.
    ScaleGizmo3DDrag atCentre;
    CNA_STUDIO_EXPECT(!atCentre.begin(scene, *layout, entityId, layout->screenOrigin));
}

/**
 * The scale gizmo has plane handles, and one factor is the point of them (`plan.md` STUDIO-12003).
 *
 * "Twice as wide and twice as deep, same height" is a single thing a user wants and had no handle
 * for. Two arm drags give two *independent* factors, which is a different operation and cannot be
 * made into this one without typing numbers -- the ratio each drag produces depends on where along
 * its own arm the cursor went, and getting two of them to agree by eye is not a thing anybody does.
 */
CNA_STUDIO_TEST(AScalePlaneHandleMultipliesTwoAxesByOneFactor)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.6f);
    camera.setPitch(0.4f);
    camera.setDistance(300.0f);

    const std::optional<ScaleGizmo3DLayout> layout =
        computeScaleGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    // Three squares, in the same places the translate gizmo puts them -- a user who has learnt
    // where the XY handle is under W finds it there under R as well.
    const std::optional<TranslateGizmo3DLayout> translate =
        computeTranslateGizmo3DLayout(scene, camera, entityId);
    CNA_STUDIO_EXPECT(translate.has_value());
    if (!translate) { return; }

    for (std::size_t index = 0; index < 3; ++index)
    {
        CNA_STUDIO_EXPECT(layout->planes[index].visible);
        CNA_STUDIO_EXPECT(cameraNearlyEqual(layout->planes[index].getScreenCenter().x,
                                            translate->planes[index].getScreenCenter().x, 0.01f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(layout->planes[index].getScreenCenter().y,
                                            translate->planes[index].getScreenCenter().y, 0.01f));
    }

    const StudioVector2 centre = layout->planes[0].getScreenCenter();
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*layout, centre) == GizmoAxis3D::XY);
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*layout, layout->planes[1].getScreenCenter())
                      == GizmoAxis3D::YZ);
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*layout, layout->planes[2].getScreenCenter())
                      == GizmoAxis3D::ZX);

    // The arms and the centre still win where they are: the squares are offset, and a plane that
    // swallowed an arm's own length would be a handle that took away three others.
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*layout, layout->screenOrigin) == GizmoAxis3D::All);
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*layout, layout->screenHandles[0]) == GizmoAxis3D::X);

    // And along an arm's line, away from its end square. This one is a guard rather than a gate:
    // the squares start a quarter of the way out on *both* of their axes, so on this gizmo -- whose
    // end handles are checked before anything else anyway -- no camera angle tried here projects a
    // square over an arm. It is asserted because the guard is cheap and the day a handle size
    // changes is the day it starts mattering.
    const StudioVector2 alongX{
        layout->screenOrigin.x + (layout->screenHandles[0].x - layout->screenOrigin.x) * 0.5f,
        layout->screenOrigin.y + (layout->screenHandles[0].y - layout->screenOrigin.y) * 0.5f};
    CNA_STUDIO_EXPECT(hitTestScaleGizmo3D(*layout, alongX) == GizmoAxis3D::X);

    ScaleGizmo3DDrag drag;
    CNA_STUDIO_EXPECT(drag.begin(scene, *layout, entityId, centre));
    CNA_STUDIO_EXPECT(drag.getAxis() == GizmoAxis3D::XY);

    // Not moved is no edit, as every other handle promises.
    CNA_STUDIO_EXPECT(!drag.update(*layout, centre, GizmoSnap{}).has_value());

    // Twice as far out along the diagonal is twice the size -- on **both** of the plane's axes, by
    // the same factor, and not at all on the third.
    const StudioVector2 twiceOut{layout->screenOrigin.x + (centre.x - layout->screenOrigin.x) * 2.0f,
                                 layout->screenOrigin.y + (centre.y - layout->screenOrigin.y) * 2.0f};
    const std::optional<StudioVector3> doubled = drag.update(*layout, twiceOut, GizmoSnap{});
    CNA_STUDIO_EXPECT(doubled.has_value());
    if (!doubled) { return; }

    CNA_STUDIO_EXPECT(cameraNearlyEqual(doubled->x, 2.0f, 0.01f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(doubled->y, 2.0f, 0.01f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(doubled->z, 1.0f, 0.001f));

    // The same factor on both, which is the whole claim: equal to each other, not merely both
    // bigger. An implementation that measured each axis separately would fail exactly here.
    CNA_STUDIO_EXPECT(cameraNearlyEqual(doubled->x, doubled->y, 0.0001f));

    // And the other two planes leave their own third axis alone.
    ScaleGizmo3DDrag yz;
    const StudioVector2 yzCentre = layout->planes[1].getScreenCenter();
    CNA_STUDIO_EXPECT(yz.begin(scene, *layout, entityId, yzCentre));
    const std::optional<StudioVector3> grown = yz.update(
        *layout, StudioVector2{layout->screenOrigin.x + (yzCentre.x - layout->screenOrigin.x) * 2.0f,
                               layout->screenOrigin.y + (yzCentre.y - layout->screenOrigin.y) * 2.0f},
        GizmoSnap{});
    CNA_STUDIO_EXPECT(grown.has_value());
    if (grown)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(grown->x, 1.0f, 0.001f));
        CNA_STUDIO_EXPECT(cameraNearlyEqual(grown->y, grown->z, 0.0001f));
        CNA_STUDIO_EXPECT(grown->y > 1.5f);
    }

    // Drawn as well as grabbed: three arms with their squares, the centre square, and three plane
    // outlines of four segments each.
    const std::vector<WireSegment> segments = buildScaleGizmo3DSegments(*layout);
    CNA_STUDIO_EXPECT_EQ(segments.size(), std::size_t{31});
}

CNA_STUDIO_TEST(AThreeDimensionalTurnIsAppliedInTheWorldRatherThanTheEntitysOwnFrame)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid entityId = scene.addEntity(makeEntity(registry, "Crate", 0.0f, 0.0f));

    // Already lying on its side. An unrotated entity cannot tell the two compositions apart --
    // they differ by exactly the rotation the entity already has -- which is why the gizmo passed
    // its first test while turning things about the wrong axes.
    scene.findEntityForEdit(entityId)
        ->findComponent(BuiltinComponentIds::kTransform)
        ->setProperty("rotation",
                      PropertyValue{quaternionFromEulerDegrees(StudioVector3{90.0f, 0.0f, 0.0f})});

    StudioCamera3D camera = makeCamera();
    camera.setPivot(StudioVector3{});
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    camera.setDistance(300.0f);

    const std::optional<RotateGizmo3DLayout> layout =
        computeRotateGizmo3DLayout(scene, camera, entityId, GizmoSpace::World);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    const std::vector<StudioVector2>& zRing = layout->rings[2];
    CNA_STUDIO_EXPECT(zRing.size() > 8);

    RotateGizmo3DDrag drag;
    CNA_STUDIO_EXPECT(drag.begin(scene, camera, *layout, entityId, zRing.front()));

    const std::optional<StudioQuaternion> turned =
        drag.update(scene, camera, zRing[zRing.size() / 4], GizmoSnap{});
    CNA_STUDIO_EXPECT(turned.has_value());
    if (!turned) { return; }

    // A turn about world Z leaves anything already pointing along world Z exactly where it is.
    // The entity's own Y axis is one such thing after a 90-degree tip about X, so this holds
    // whatever the drag's angle turned out to be -- and fails outright if the turn went in about
    // the entity's local axes instead, where the same drag would swing it a quarter turn away.
    const StudioVector3 localY = rotate(*turned, StudioVector3{0.0f, 1.0f, 0.0f});
    CNA_STUDIO_EXPECT(cameraNearlyEqual(localY.z, 1.0f, 0.02f));

    // And its own X axis, which started in the XY plane, stays in it.
    const StudioVector3 localX = rotate(*turned, StudioVector3{1.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(cameraNearlyEqual(localX.z, 0.0f, 0.02f));
}

namespace
{
    /** @brief Adds a `CNA.Light` to @p entity with the given kind, intensity and range. */
    void addLight(const ComponentRegistry& registry, StudioEntity& entity, const std::string& kind,
                  float intensity, float range, StudioColor color = StudioColor{255, 255, 255, 255})
    {
        StudioComponent light{BuiltinComponentIds::kLight};
        light.applyDefaults(*registry.find(BuiltinComponentIds::kLight));
        light.setProperty("kind", PropertyValue{PropertyValue::EnumValue{kind}});
        light.setProperty("intensity", PropertyValue{intensity});
        light.setProperty("range", PropertyValue{range});
        light.setProperty("color", PropertyValue{color});
        entity.addComponent(std::move(light));
    }
}

/**
 * @brief ED-402/ED-404: a scene with no light asks for the default rather than for darkness.
 *
 * The commonest scene there is -- one somebody has just dropped a model into -- has no light in
 * it, and a model rendered black in that scene is indistinguishable from a renderer that does not
 * work. So "no lights" is a distinct answer the renderer acts on, not zero lights applied.
 */
CNA_STUDIO_TEST(ASceneWithNoLightsAsksForTheDefaultLightingRatherThanForNone)
{
    const EffectLighting empty = computeEffectLighting({}, StudioVector3{0.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(empty.useDefaultLighting);
    CNA_STUDIO_EXPECT_EQ(empty.lightCount, std::size_t{0});

    // And so does a scene whose only lamp is out of range of what is being drawn: "unlit" and "too
    // far from the light" look identical to a user, and both are better answered by something
    // visible than by black.
    SceneLight distant;
    distant.kind = SceneLightKind::Point;
    distant.position = StudioVector3{1000.0f, 0.0f, 0.0f};
    distant.range = 5.0f;

    const EffectLighting outOfRange =
        computeEffectLighting({distant}, StudioVector3{0.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(outOfRange.useDefaultLighting);
}

/** @brief A light entity's rotation is what aims it, so turning it in the viewport turns the light. */
CNA_STUDIO_TEST(ADirectionalLightShinesAlongItsEntitysOwnForwardAxis)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Sun", 0.0f, 0.0f);
    addLight(registry, entity, "Directional", 1.0f, 10.0f);
    const Uuid id = scene.addEntity(std::move(entity));

    const std::vector<SceneLight> unrotated = collectSceneLights(scene);
    CNA_STUDIO_EXPECT_EQ(unrotated.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(unrotated[0].entityId == id);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(unrotated[0].direction.z, 1.0f, 0.001f));

    // Turned a half turn about Y, it must shine back the other way. Asserting the property rather
    // than a number: whatever convention the transform uses, a light spun 180 degrees cannot go on
    // pointing where it did.
    StudioEntity* stored = scene.findEntityForEdit(id);
    CNA_STUDIO_EXPECT(stored != nullptr);
    StudioComponent* transform = stored->findComponent(BuiltinComponentIds::kTransform);
    CNA_STUDIO_EXPECT(transform != nullptr);
    transform->setProperty("rotation",
                           PropertyValue{quaternionFromEulerDegrees(StudioVector3{0.0f, 180.0f, 0.0f})});

    const std::vector<SceneLight> turned = collectSceneLights(scene);
    CNA_STUDIO_EXPECT_EQ(turned.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(cameraNearlyEqual(turned[0].direction.z, -1.0f, 0.001f));
}

/**
 * @brief A disabled entity's light does not light anything.
 *
 * The same rule the rest of the viewport follows for what it draws. A user who disables an entity
 * has said "pretend this is not here", and a lamp that goes on shining is the editor disagreeing.
 */
CNA_STUDIO_TEST(ALightOnADisabledEntityIsNotCollected)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Lamp", 0.0f, 0.0f);
    addLight(registry, entity, "Point", 1.0f, 10.0f);
    const Uuid id = scene.addEntity(std::move(entity));

    CNA_STUDIO_EXPECT_EQ(collectSceneLights(scene).size(), std::size_t{1});

    scene.findEntityForEdit(id)->setEnabled(false);
    CNA_STUDIO_EXPECT(collectSceneLights(scene).empty());
}

/**
 * @brief `IEffectLights` holds three lights, so a fourth has to lose -- and the dimmest is the one.
 *
 * Brightest wins rather than nearest or first-in-document. Document order is not something a user
 * arranges deliberately, and dropping the sun because a dim lamp was added earlier is a picture
 * nobody could account for from what is on screen.
 */
CNA_STUDIO_TEST(WhereMoreThanThreeLightsApplyTheThreeBrightestWin)
{
    std::vector<SceneLight> lights;
    for (int i = 0; i < 4; ++i)
    {
        SceneLight light;
        light.kind = SceneLightKind::Directional;
        // The first is the dimmest, so a correct answer cannot also be "the first three".
        light.intensity = 0.1f + static_cast<float>(i);
        lights.push_back(light);
    }

    const EffectLighting lighting = computeEffectLighting(lights, StudioVector3{0.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(!lighting.useDefaultLighting);
    CNA_STUDIO_EXPECT_EQ(lighting.lightCount, std::size_t{3});

    // Brightest first, and the one left out is the dimmest.
    CNA_STUDIO_EXPECT(lighting.lights[0].diffuseColor.x > lighting.lights[1].diffuseColor.x);
    CNA_STUDIO_EXPECT(lighting.lights[1].diffuseColor.x > lighting.lights[2].diffuseColor.x);
    CNA_STUDIO_EXPECT(lighting.lights[2].diffuseColor.x > 1.0f);
}

/**
 * @brief A point light is sent as a point light, with its own position and range.
 *
 * `plan.md` STUDIO-20003, and this case **replaces** one that asserted the opposite. It used to
 * pin the workaround: `IEffectLights` has no point light, so Studio flattened one into a
 * directional light aimed at whatever was being drawn, dimmed by distance. That was the right
 * answer for an editor that only ever spoke `IEffectLights` — and the wrong belief about CNA,
 * which takes one real punctual light per draw through `PbrEffect::setPunctualLightEXT`. The old
 * assertions are kept below, applied to the light that *loses* the single slot, because the
 * approximation is still what every extra lamp gets.
 */
CNA_STUDIO_TEST(APointLightIsSentWholeAndTheRunnersUpAreStillApproximated)
{
    SceneLight lamp;
    lamp.kind = SceneLightKind::Point;
    lamp.position = StudioVector3{0.0f, 0.0f, 0.0f};
    lamp.range = 100.0f;
    lamp.intensity = 1.0f;

    const EffectLighting lit = computeEffectLighting({lamp}, StudioVector3{10.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(!lit.useDefaultLighting);

    // The light goes to the punctual slot whole: its own position and range, not a direction and
    // a pre-dimmed colour. That is what lets the effect shade per *pixel*, so one lamp beside a
    // large model lights the near end of it and not the far one.
    CNA_STUDIO_EXPECT(lit.hasPunctual);
    CNA_STUDIO_EXPECT(lit.punctual.kind == SceneLightKind::Point);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(lit.punctual.position.x, 0.0f, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(lit.punctual.range, 100.0f, 0.001f));

    // And it is *not* also a directional light: one lamp counted twice is one lamp at twice its
    // brightness, which is the defect this ordering exists to prevent.
    CNA_STUDIO_EXPECT_EQ(lit.lightCount, std::size_t{0});

    // The colour carries intensity and **not** falloff, because the effect measures the distance
    // itself. A pre-dimmed colour here would be the approximation wearing the new slot's clothes.
    const EffectLighting far = computeEffectLighting({lamp}, StudioVector3{90.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(far.hasPunctual);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(far.punctual.diffuseColor.x, lit.punctual.diffuseColor.x,
                                        0.001f));

    // Out of range it reaches nothing at all, and with nothing else in the scene that is a scene
    // with no light rather than a scene lit by a light that does nothing.
    const EffectLighting beyond =
        computeEffectLighting({lamp}, StudioVector3{101.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(!beyond.hasPunctual);
    CNA_STUDIO_EXPECT(beyond.useDefaultLighting);

    // Now the half that did not change. There is one punctual slot, so a second lamp is still
    // approximated as a directional light aimed at whatever is being drawn — and the property that
    // makes *that* worth having is the one the old case pinned: two objects on opposite sides of a
    // lamp are lit from opposite directions.
    SceneLight dim = lamp;
    dim.intensity = 0.25f;

    const EffectLighting left = computeEffectLighting({lamp, dim}, StudioVector3{-10.0f, 0.0f, 0.0f});
    const EffectLighting right = computeEffectLighting({lamp, dim}, StudioVector3{10.0f, 0.0f, 0.0f});

    CNA_STUDIO_EXPECT(left.hasPunctual);
    CNA_STUDIO_EXPECT_EQ(left.lightCount, std::size_t{1});
    CNA_STUDIO_EXPECT(cameraNearlyEqual(left.lights[0].direction.x, -1.0f, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(right.lights[0].direction.x, 1.0f, 0.001f));

    // And the brighter of the two is the one that got the slot, not whichever came first.
    CNA_STUDIO_EXPECT(left.punctual.diffuseColor.x > left.lights[0].diffuseColor.x);

    // The approximated one still dims with distance, since its falloff has nowhere else to go.
    const EffectLighting nearTwo = computeEffectLighting({lamp, dim}, StudioVector3{10.0f, 0.0f, 0.0f});
    const EffectLighting farTwo = computeEffectLighting({lamp, dim}, StudioVector3{90.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(nearTwo.lights[0].diffuseColor.x > farTwo.lights[0].diffuseColor.x);
}

/** @brief A spot light carries its cone, which is the whole of what makes it one. */
CNA_STUDIO_TEST(ASpotLightCarriesItsConeToTheEffect)
{
    SceneLight spot;
    spot.kind = SceneLightKind::Spot;
    spot.position = StudioVector3{0.0f, 0.0f, 0.0f};
    spot.direction = StudioVector3{0.0f, 0.0f, 1.0f};
    spot.range = 100.0f;
    spot.intensity = 1.0f;
    spot.innerAngle = 0.3f;
    spot.outerAngle = 0.6f;

    const EffectLighting lit = computeEffectLighting({spot}, StudioVector3{0.0f, 0.0f, 10.0f});
    CNA_STUDIO_EXPECT(lit.hasPunctual);
    CNA_STUDIO_EXPECT(lit.punctual.kind == SceneLightKind::Spot);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(lit.punctual.innerAngle, 0.3f, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(lit.punctual.outerAngle, 0.6f, 0.001f));

    // Its direction is its own axis, not one aimed at the target: that is exactly the difference
    // between a spot light and the point light it used to be flattened into.
    CNA_STUDIO_EXPECT(cameraNearlyEqual(lit.punctual.direction.z, 1.0f, 0.001f));

    // A directional light never reaches the punctual slot: it has no position for one.
    SceneLight sun;
    sun.kind = SceneLightKind::Directional;
    sun.intensity = 1.0f;
    const EffectLighting sunlit = computeEffectLighting({sun}, StudioVector3{0.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(!sunlit.hasPunctual);
    CNA_STUDIO_EXPECT_EQ(sunlit.lightCount, std::size_t{1});
}

namespace
{
    /** @brief A one-triangle mesh, enough for a batch to have something real to carry. */
    MeshData makeTinyMesh()
    {
        MeshPart part;
        part.vertices = {MeshVertex{StudioVector3{0.0f, 0.0f, 0.0f}, StudioVector3{0.0f, 0.0f, -1.0f}, {}},
                         MeshVertex{StudioVector3{1.0f, 0.0f, 0.0f}, StudioVector3{0.0f, 0.0f, -1.0f}, {}},
                         MeshVertex{StudioVector3{0.0f, 1.0f, 0.0f}, StudioVector3{0.0f, 0.0f, -1.0f}, {}}};
        part.indices = {0, 1, 2};

        MeshData mesh;
        mesh.parts.push_back(std::move(part));
        recomputeMeshBounds(mesh);
        return mesh;
    }

    /** @brief Adds a `CNA.ModelRenderer` naming @p modelId to @p entity. */
    void addModelRenderer(const ComponentRegistry& registry, StudioEntity& entity, const Uuid& modelId)
    {
        StudioComponent renderer{BuiltinComponentIds::kModelRenderer};
        renderer.applyDefaults(*registry.find(BuiltinComponentIds::kModelRenderer));
        renderer.setProperty("model", PropertyValue{PropertyValue::AssetReference{modelId}});
        entity.addComponent(std::move(renderer));
    }
}

/**
 * @brief ED-402: the batch carries the camera's own view-projection, mirror and all.
 *
 * The single most important property of this seam, and the cheapest to get wrong. The 3D camera's
 * view-projection already contains the Y mirror that converts XNA's Y-up 3D frame to this editor's
 * Y-down world; a model pass that built its own matrix, or applied the mirror a second time, would
 * draw models upside down relative to the grid and the gizmos -- or, worse, right way up but
 * somewhere a click could not reach them.
 */
CNA_STUDIO_TEST(TheModelBatchDrawsThroughTheSameViewProjectionAsEverythingElse)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Crate", 3.0f, 4.0f);
    addModelRenderer(registry, entity, modelId);
    scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    camera.orbit(35.0f, 25.0f);

    const MeshProvider provider = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };
    const SceneModelBatch batch = buildSceneModelBatch(scene, camera, provider);

    CNA_STUDIO_EXPECT_EQ(batch.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(batch.viewProjection == camera.getViewProjectionMatrix());
    CNA_STUDIO_EXPECT_EQ(batch.triangleCount, std::size_t{1});
    CNA_STUDIO_EXPECT(batch.draws[0].mesh == &mesh);
}

/**
 * @brief The world matrix in a draw maps model space exactly where the scene transform says.
 *
 * The property, not the sixteen numbers: a local point pushed through the matrix must land where
 * composing the entity's own scale, rotation and translation puts it. Written this way because the
 * failure it guards against is a *different composition order* -- rotate-then-scale instead of
 * scale-then-rotate -- which is invisible on an unrotated or unscaled entity and puts a rotated,
 * non-uniformly scaled one somewhere its own gizmo does not agree with.
 */
CNA_STUDIO_TEST(AModelDrawsWhereTheSceneTransformSaysItIs)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Crate", 3.0f, 4.0f);
    addModelRenderer(registry, entity, modelId);
    const Uuid id = scene.addEntity(std::move(entity));

    StudioComponent* transform = scene.findEntityForEdit(id)->findComponent(BuiltinComponentIds::kTransform);
    transform->setProperty("scale", PropertyValue{StudioVector3{2.0f, 3.0f, 1.0f}});
    transform->setProperty("rotation",
                           PropertyValue{quaternionFromEulerDegrees(StudioVector3{0.0f, 0.0f, 90.0f})});

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const MeshProvider provider = [&](const Uuid& queried) { return queried == modelId ? &mesh : nullptr; };
    const SceneModelBatch batch = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(batch.draws.size(), std::size_t{1});

    // (1, 0, 0) scaled by 2 on X, then turned a quarter turn about Z, then moved to (3, 4, 0).
    const std::optional<WorldTransform> world = computeWorldTransform(scene, id);
    CNA_STUDIO_EXPECT(world.has_value());

    const StudioVector3 local{1.0f, 0.0f, 0.0f};
    const StudioVector3 expected =
        add(rotate(world->rotation,
                   StudioVector3{local.x * world->scale.x, local.y * world->scale.y,
                                 local.z * world->scale.z}),
            world->position);
    const StudioVector3 actual = transformPosition(batch.draws[0].world, local);

    CNA_STUDIO_EXPECT(cameraNearlyEqual(actual.x, expected.x, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(actual.y, expected.y, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(actual.z, expected.z, 0.001f));
}

/**
 * @brief A model still importing is counted, not silently dropped.
 *
 * "Still importing" and "this entity has no model" look identical on screen and only one of them
 * is worth waiting for, so the batch keeps a count the viewport can report rather than leaving a
 * user to wonder where their crate went.
 */
CNA_STUDIO_TEST(AnEntityWhoseMeshHasNotArrivedIsCountedRatherThanDropped)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, entity, Uuid::generate());
    const Uuid id = scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const MeshProvider none = [](const Uuid&) -> const MeshData* { return nullptr; };
    const SceneModelBatch pending = buildSceneModelBatch(scene, camera, none);
    CNA_STUDIO_EXPECT(pending.draws.empty());
    CNA_STUDIO_EXPECT_EQ(pending.pendingMeshes, std::size_t{1});

    // And a disabled entity is not pending either -- it is simply not there, like everywhere else.
    scene.findEntityForEdit(id)->setEnabled(false);
    const SceneModelBatch disabled = buildSceneModelBatch(scene, camera, none);
    CNA_STUDIO_EXPECT_EQ(disabled.pendingMeshes, std::size_t{0});
}

/**
 * @brief The batch's split camera multiplies back to the product it is drawn through.
 *
 * `SceneModelBatch` carries the camera twice -- as one view-projection and as the view and
 * projection separately -- because CNA's `PbrEffect` inverts the view to recover the eye position
 * and an identity view is a camera claiming to sit at the origin. Two descriptions of one thing
 * that could drift is exactly the bug this seam was arranged to avoid, so the invariant is pinned
 * rather than trusted: the mirror lives in the projection, and the product is the matrix
 * everything else in the editor already positions against.
 */
CNA_STUDIO_TEST(TheModelBatchsSplitCameraMultipliesBackToItsProduct)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Crate", 1.0f, 2.0f);
    addModelRenderer(registry, entity, modelId);
    scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    camera.orbit(40.0f, -20.0f);

    const MeshProvider provider = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };
    const SceneModelBatch batch = buildSceneModelBatch(scene, camera, provider);

    const StudioMatrix product = multiply(batch.view, batch.projection);

    // Compared through a point rather than field by field: what has to agree is where a world
    // point lands, and a matrix comparison would fail on floating-point noise that moves nothing.
    const StudioVector3 probe{3.0f, -2.0f, 5.0f};
    const StudioVector3 throughProduct = transformPosition(product, probe);
    const StudioVector3 throughCombined = transformPosition(batch.viewProjection, probe);

    CNA_STUDIO_EXPECT(cameraNearlyEqual(throughProduct.x, throughCombined.x, 0.0005f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(throughProduct.y, throughCombined.y, 0.0005f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(throughProduct.z, throughCombined.z, 0.0005f));

    // And the mirror really is in the projection half, not left to the caller: a camera at rest
    // must send world +Y to screen -Y, which is the whole reason the 2D and 3D views agree about
    // which way is down.
    CNA_STUDIO_EXPECT(batch.projection.m22 < 0.0f);
}

namespace
{
    /** @brief Adds a `CNA.SpriteRenderer` naming @p textureId, with an optional origin. */
    void addSpriteRenderer(const ComponentRegistry& registry, StudioEntity& entity,
                           const Uuid& textureId, StudioVector2 origin = {})
    {
        StudioComponent sprite{BuiltinComponentIds::kSpriteRenderer};
        sprite.applyDefaults(*registry.find(BuiltinComponentIds::kSpriteRenderer));
        sprite.setProperty("texture", PropertyValue{PropertyValue::AssetReference{textureId}});
        sprite.setProperty("origin", PropertyValue{origin});
        entity.addComponent(std::move(sprite));
    }
}

/**
 * @brief A sprite's quad is the rectangle the picker already thinks the sprite is.
 *
 * The property worth pinning, because the alternative -- billboarding -- passes every other test
 * and fails this one. `computeEntityBounds3D`, the gizmos and the ray picker all treat a sprite as
 * a flat box in the scene's XY plane; a quad drawn anywhere else is a sprite drawn where it cannot
 * be clicked.
 */
CNA_STUDIO_TEST(ASpriteQuadIsTheRectangleThePickerAlreadyThinksItIs)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid textureId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Orb", 5.0f, 7.0f);
    addSpriteRenderer(registry, entity, textureId);
    const Uuid id = scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    camera.orbit(30.0f, 20.0f);

    const SpriteSizeProvider sizes = [&](const Uuid& queried)
    { return queried == textureId ? StudioVector2{32.0f, 16.0f} : StudioVector2{}; };

    const SceneSpriteBatch3D batch = buildSceneSpriteQuads(scene, camera, sizes, {}, {}, &registry);
    CNA_STUDIO_EXPECT_EQ(batch.quads.size(), std::size_t{1});

    const std::optional<WorldBounds3D> bounds = computeEntityBounds3D(scene, id, sizes);
    CNA_STUDIO_EXPECT(bounds.has_value());

    // Every corner lies inside what the picker calls this entity, and the quad spans the whole of
    // it -- so the two describe one rectangle rather than two that happen to overlap.
    float minX = batch.quads[0].corners[0].x, maxX = minX;
    float minY = batch.quads[0].corners[0].y, maxY = minY;
    for (const StudioVector3& corner : batch.quads[0].corners)
    {
        minX = std::min(minX, corner.x);
        maxX = std::max(maxX, corner.x);
        minY = std::min(minY, corner.y);
        maxY = std::max(maxY, corner.y);

        // And it is flat in the scene plane: a sprite that turned to face the camera would not be.
        CNA_STUDIO_EXPECT(cameraNearlyEqual(corner.z, 0.0f, 0.001f));
    }

    CNA_STUDIO_EXPECT(cameraNearlyEqual(minX, bounds->min.x, 0.01f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(maxX, bounds->max.x, 0.01f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(minY, bounds->min.y, 0.01f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(maxY, bounds->max.y, 0.01f));
}

/**
 * @brief Pitch and roll on the entity do not tilt the sprite, because `SpriteBatch` cannot.
 *
 * The 3D view is allowed to show more than the 2D one and not more than the *game*. A quad honouring
 * the full world rotation would look better and would be the editor drawing something the player
 * will render flat.
 */
CNA_STUDIO_TEST(ASpriteIgnoresEverythingButItsZRotation)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid textureId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Orb", 0.0f, 0.0f);
    addSpriteRenderer(registry, entity, textureId);
    const Uuid id = scene.addEntity(std::move(entity));

    scene.findEntityForEdit(id)
        ->findComponent(BuiltinComponentIds::kTransform)
        ->setProperty("rotation",
                      PropertyValue{quaternionFromEulerDegrees(StudioVector3{60.0f, 45.0f, 0.0f})});

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const SpriteSizeProvider sizes = [&](const Uuid&) { return StudioVector2{16.0f, 16.0f}; };
    const SceneSpriteBatch3D batch = buildSceneSpriteQuads(scene, camera, sizes, {}, {}, &registry);

    CNA_STUDIO_EXPECT_EQ(batch.quads.size(), std::size_t{1});
    for (const StudioVector3& corner : batch.quads[0].corners)
    {
        CNA_STUDIO_EXPECT(cameraNearlyEqual(corner.z, 0.0f, 0.001f));
    }
}

/**
 * @brief Sprites come back furthest first, because transparency does not commute.
 *
 * Two overlapping transparent quads blended in the wrong order give a different picture, and the
 * one that looks right is furthest first. This is the ordering the viewport relies on and does not
 * re-derive.
 */
CNA_STUDIO_TEST(SpriteQuadsArriveFurthestFromTheCameraFirst)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid textureId = Uuid::generate();

    // Added near-first, so a correct answer cannot also be "document order".
    StudioEntity near = makeEntity(registry, "Near", 0.0f, 0.0f);
    addSpriteRenderer(registry, near, textureId);
    scene.addEntity(std::move(near));

    StudioEntity far = makeEntity(registry, "Far", 400.0f, 0.0f);
    addSpriteRenderer(registry, far, textureId);
    scene.addEntity(std::move(far));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const SpriteSizeProvider sizes = [&](const Uuid&) { return StudioVector2{8.0f, 8.0f}; };
    const SceneSpriteBatch3D batch = buildSceneSpriteQuads(scene, camera, sizes, {}, {}, &registry);

    CNA_STUDIO_EXPECT_EQ(batch.quads.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(batch.quads[0].cameraDistance >= batch.quads[1].cameraDistance);
}

/** @brief A sprite whose texture cannot be sized is counted, never drawn at a guessed size. */
CNA_STUDIO_TEST(ASpriteWithNoKnownSizeIsCountedRatherThanGuessedAt)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity entity = makeEntity(registry, "Orb", 0.0f, 0.0f);
    addSpriteRenderer(registry, entity, Uuid::generate());
    scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const SpriteSizeProvider unknown = [](const Uuid&) { return StudioVector2{}; };
    const SceneSpriteBatch3D batch =
        buildSceneSpriteQuads(scene, camera, unknown, {}, {}, &registry);

    CNA_STUDIO_EXPECT(batch.quads.empty());
    CNA_STUDIO_EXPECT_EQ(batch.skipped, std::size_t{1});
}

/**
 * @brief ED-407: a scene's environment round-trips, and a scene that has none stays that way.
 *
 * The second half is the one worth a test. `environment` is an *additive* field, which means a
 * scene written before ED-407 must read as the defaults and be written back out **without** the
 * object -- a loader that read a default and then serialised it would turn every existing scene
 * into a modified file the first time it was opened, and a diff full of scenes nobody touched is
 * how a format change gets blamed on the wrong commit.
 */
CNA_STUDIO_TEST(ASceneWithoutAnEnvironmentDoesNotGrowOneByBeingOpened)
{
    const ComponentRegistry registry = makeRegistry();

    SceneDocument fresh;
    fresh.setName("Level01");
    fresh.addEntity(makeEntity(registry, "Player", 1.0f, 2.0f));

    // Nothing set: the serialised form must not mention the environment at all.
    const JsonValue bare = fresh.toJson();
    CNA_STUDIO_EXPECT(!bare["environment"].isObject());

    SceneDocument reloaded;
    reloaded.loadFromJson(bare, registry, nullptr);
    CNA_STUDIO_EXPECT(reloaded.getEnvironment().isDefault());
    CNA_STUDIO_EXPECT(!reloaded.toJson()["environment"].isObject());

    // And once it is set, every field survives the trip.
    SceneEnvironment foggy;
    foggy.ambientColor = StudioColor{40, 44, 60, 255};
    foggy.fogEnabled = true;
    foggy.fogColor = StudioColor{200, 190, 180, 255};
    foggy.fogStart = 250.0f;
    foggy.fogEnd = 900.0f;
    fresh.setEnvironment(foggy);

    SceneDocument roundTripped;
    roundTripped.loadFromJson(fresh.toJson(), registry, nullptr);

    const SceneEnvironment& back = roundTripped.getEnvironment();
    CNA_STUDIO_EXPECT(!back.isDefault());
    CNA_STUDIO_EXPECT(back.fogEnabled);
    CNA_STUDIO_EXPECT(back.ambientColor == foggy.ambientColor);
    CNA_STUDIO_EXPECT(back.fogColor == foggy.fogColor);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(back.fogStart, 250.0f, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(back.fogEnd, 900.0f, 0.001f));

    // The formatVersion is untouched: this is an additive field, not a new version of the format.
    CNA_STUDIO_EXPECT_EQ(fresh.toJson()["formatVersion"].asNumber(0.0),
                         bare["formatVersion"].asNumber(-1.0));
}

/** @brief The scene's ambient reaches the model batch, because a cave is dark for everything in it. */
CNA_STUDIO_TEST(TheScenesAmbientReachesEveryModelDrawnInIt)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, entity, modelId);
    scene.addEntity(std::move(entity));

    SceneEnvironment dim;
    dim.ambientColor = StudioColor{255, 0, 0, 255};
    scene.setEnvironment(dim);

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const MeshProvider provider = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };
    const SceneModelBatch batch = buildSceneModelBatch(scene, camera, provider);

    CNA_STUDIO_EXPECT_EQ(batch.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(cameraNearlyEqual(batch.draws[0].lighting.ambientColor.x, 1.0f, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(batch.draws[0].lighting.ambientColor.y, 0.0f, 0.001f));

    // And the fog settings ride on the batch rather than on each draw: fog is a property of the
    // level, so a per-draw copy would be the same numbers repeated with nothing able to differ.
    CNA_STUDIO_EXPECT(batch.environment.ambientColor == dim.ambientColor);
}

/**
 * @brief ED-410: a per-part material lands on the part it names and leaves the others alone.
 *
 * The behaviour that makes the list worth having, and the one an index-keyed list gets wrong
 * silently. It is checked here rather than by screenshot because which material reaches which part
 * is a lookup, not a colour -- and a lookup is exactly the kind of thing a picture is bad at
 * disproving.
 */
CNA_STUDIO_TEST(APerPartMaterialAppliesToItsOwnPartAndNoOther)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    // Two parts, so "only this one" has something to be distinguished from.
    MeshData mesh = makeTinyMesh();
    mesh.parts[0].name = "Body";
    MeshPart lid = mesh.parts[0];
    lid.name = "Lid";
    mesh.parts.push_back(std::move(lid));

    const Uuid modelId = Uuid::generate();
    const Uuid lidMaterialId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, entity, modelId);

    PropertyValue::StructureValue lidEntry;
    lidEntry.set("part", PropertyValue{std::string{"Lid"}});
    lidEntry.set("material", PropertyValue{PropertyValue::AssetReference{lidMaterialId}});

    PropertyValue::ListValue materials;
    materials.items.push_back(PropertyValue{lidEntry});

    StudioComponent* renderer = entity.findComponent(BuiltinComponentIds::kModelRenderer);
    renderer->setProperty("materials", PropertyValue{materials});
    scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const MeshProvider meshes = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };
    const MaterialProvider materialsProvider =
        [&](const Uuid& id) -> std::optional<MeshMaterial>
    {
        if (id != lidMaterialId) { return std::nullopt; }
        MeshMaterial material;
        material.name = "Lid Paint";
        return material;
    };

    const SceneModelBatch batch =
        buildSceneModelBatch(scene, camera, meshes, {}, materialsProvider);

    CNA_STUDIO_EXPECT_EQ(batch.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(batch.draws[0].partMaterials.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(batch.draws[0].partMaterials[0].first, std::string{"Lid"});
    CNA_STUDIO_EXPECT_EQ(batch.draws[0].partMaterials[0].second.name, std::string{"Lid Paint"});

    // The whole-model override is untouched: the two compose as "this model, except these parts".
    CNA_STUDIO_EXPECT(!batch.draws[0].materialOverride.has_value());
}

/**
 * @brief A per-part entry naming a part the model does not have is kept, not quietly dropped.
 *
 * The reason the list is keyed by name at all. An entry that matches nothing is *detectable* --
 * validation can say so, and a user can fix it -- whereas an index-keyed entry that shifted after a
 * reimport still matches a part, just the wrong one, and there is nothing anywhere to notice.
 * Dropping it here would throw away the very evidence that makes the choice worth making.
 */
CNA_STUDIO_TEST(APerPartMaterialNamingAMissingPartSurvivesSoItCanBeReported)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();
    const Uuid materialId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, entity, modelId);

    PropertyValue::StructureValue entry;
    entry.set("part", PropertyValue{std::string{"NoSuchPart"}});
    entry.set("material", PropertyValue{PropertyValue::AssetReference{materialId}});

    PropertyValue::ListValue materials;
    materials.items.push_back(PropertyValue{entry});
    entity.findComponent(BuiltinComponentIds::kModelRenderer)
        ->setProperty("materials", PropertyValue{materials});
    scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const MeshProvider meshes = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };
    const MaterialProvider materialsProvider = [&](const Uuid&) { return MeshMaterial{}; };

    const SceneModelBatch batch =
        buildSceneModelBatch(scene, camera, meshes, {}, materialsProvider);

    CNA_STUDIO_EXPECT_EQ(batch.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(batch.draws[0].partMaterials.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(batch.draws[0].partMaterials[0].first, std::string{"NoSuchPart"});
}

/**
 * @brief The rule that makes name-keying worth its cost: a part that is not there is reported.
 *
 * Without this the argument for keying by name over index is only half made. An index-keyed
 * override that shifted after a reimport still points at *a* part and there is nothing to notice;
 * a name that matches nothing is detectable, and this is what detects it.
 */
CNA_STUDIO_TEST(AMaterialAssignedToAPartTheModelDoesNotHaveIsReported)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    MeshData mesh = makeTinyMesh();
    mesh.parts[0].name = "Body";
    const Uuid modelId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, entity, modelId);

    PropertyValue::StructureValue entry;
    entry.set("part", PropertyValue{std::string{"Lid"}});
    entry.set("material", PropertyValue{PropertyValue::AssetReference{Uuid::generate()}});

    PropertyValue::ListValue materials;
    materials.items.push_back(PropertyValue{entry});
    entity.findComponent(BuiltinComponentIds::kModelRenderer)
        ->setProperty("materials", PropertyValue{materials});
    scene.addEntity(std::move(entity));

    const MeshProvider meshes = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };

    const std::vector<SceneIssue> issues = validateModelPartMaterials(scene, meshes);
    CNA_STUDIO_EXPECT_EQ(issues.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(issues[0].ruleId, std::string{"model-part-not-found"});
    CNA_STUDIO_EXPECT(issues[0].message.find("Lid") != std::string::npos);

    // A part that *is* there says nothing at all.
    mesh.parts[0].name = "Lid";
    CNA_STUDIO_EXPECT(validateModelPartMaterials(scene, meshes).empty());

    // And a model whose mesh has not been imported says nothing either: "not imported yet" is not
    // "wrong", and a rule that fired mid-scan would report every model in the project.
    const MeshProvider none = [](const Uuid&) -> const MeshData* { return nullptr; };
    CNA_STUDIO_EXPECT(validateModelPartMaterials(scene, none).empty());
}

// ------------------------------------------------------------------------------------------------
// Hierarchy lookup cost (STUDIO-30013)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(ChildrenByParentAgreesWithGetChildrenForEveryParent)
{
    // Two orderings of one hierarchy would show as an outliner whose rows moved when something
    // unrelated rebuilt them, which is the kind of defect nobody can reproduce on demand. The
    // grouped form exists for speed and must therefore be indistinguishable in every other way.
    SceneDocument scene;
    std::vector<Uuid> ids;
    Uuid parent;
    for (int i = 0; i < 60; ++i)
    {
        StudioEntity entity{Uuid::generate(), "Entity " + std::to_string((i * 7) % 60)};
        entity.setSortOrder(i % 5);
        if (i % 6 != 0) { entity.setParentId(parent); }
        const Uuid id = entity.getId();
        scene.addEntity(std::move(entity));
        ids.push_back(id);
        if (i % 6 == 0) { parent = id; }
    }

    const std::unordered_map<Uuid, std::vector<Uuid>> grouped = scene.getChildrenByParent();

    // The roots live under the nil Uuid, so a walk needs no special case for them.
    const auto roots = grouped.find(Uuid{});
    CNA_STUDIO_EXPECT(roots != grouped.end());
    if (roots != grouped.end())
    {
        CNA_STUDIO_EXPECT(roots->second == scene.getRootEntities());
    }

    std::size_t accountedFor = roots != grouped.end() ? roots->second.size() : 0;
    for (const Uuid& id : ids)
    {
        const std::vector<Uuid> expected = scene.getChildren(id);
        const auto found = grouped.find(id);
        if (expected.empty())
        {
            // A parent with no children is absent rather than present-and-empty, which is what
            // lets the map be sized by the hierarchy rather than by the entity count.
            CNA_STUDIO_EXPECT(found == grouped.end());
            continue;
        }
        CNA_STUDIO_EXPECT(found != grouped.end());
        if (found != grouped.end())
        {
            CNA_STUDIO_EXPECT(found->second == expected);
            accountedFor += found->second.size();
        }
    }

    // Every entity appears exactly once, under exactly one parent.
    CNA_STUDIO_EXPECT_EQ(accountedFor, scene.getEntityCount());
}

CNA_STUDIO_TEST(FlatteningTheOutlinerCostsLinearTimeInTheSceneRatherThanQuadratic)
{
    // The defect `--ui-benchmark` found: `getChildren` scans every entity, and the outliner's
    // flatten called it once per row. 250 entities cost 9 ms a frame, 500 cost 25 ms, 1 000 cost
    // 84 ms and 2 000 cost 309 ms -- four times the cost for twice the entities, and three frames
    // a second on a scene that is not large.
    //
    // Rows are virtualised, so the *drawing* was already flat: the benchmark reported the same 20
    // draw calls and 7 317 vertices at 5 entities and at 2 000. No capture, golden image or
    // draw-call assertion could have shown this, which is why the guard is a timing one.
    //
    // A ratio rather than an absolute, because an absolute is a number about this machine. Four
    // times the entities costs about four times as much when the walk is linear and about sixteen
    // when it is quadratic; the threshold sits between them with room for a loaded machine.
    const auto build = [](int count) {
        SceneDocument scene;
        Uuid parent;
        for (int i = 0; i < count; ++i)
        {
            StudioEntity entity{Uuid::generate(), "Entity " + std::to_string(i)};
            if (i % 8 != 0) { entity.setParentId(parent); }
            const Uuid id = entity.getId();
            scene.addEntity(std::move(entity));
            if (i % 8 == 0) { parent = id; }
        }
        return scene;
    };

    const auto timeRows = [](const SceneDocument& scene) {
        StudioTreeState state;
        state.expandAll();
        const std::vector<Uuid> selection;

        // Warmed, then the best of several: a scheduler preemption can only make a sample slower,
        // so the minimum is the closest thing to "what this costs" a shared machine can report.
        (void)studioOutlinerRows(scene, selection, state);

        double best = 1e18;
        for (int attempt = 0; attempt < 5; ++attempt)
        {
            const auto start = std::chrono::steady_clock::now();
            const std::vector<StudioTreeRow> rows = studioOutlinerRows(scene, selection, state);
            const auto finish = std::chrono::steady_clock::now();
            CNA_STUDIO_EXPECT(!rows.empty());
            best = std::min(best,
                            std::chrono::duration<double, std::micro>(finish - start).count());
        }
        return best;
    };

    const SceneDocument small = build(400);
    const SceneDocument large = build(1600);

    const double smallCost = timeRows(small);
    const double largeCost = timeRows(large);

    CNA_STUDIO_EXPECT(smallCost > 0.0);
    if (smallCost <= 0.0) { return; }

    const double ratio = largeCost / smallCost;
    if (ratio > 8.0)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "flattening 1600 entities cost " + std::to_string(ratio)
            + " times what 400 did. Four times the entities should cost about four times as much; "
              "sixteen means the walk asks the document for children once per node again. Derive "
              "the hierarchy once with SceneDocument::getChildrenByParent (plan.md STUDIO-30013).");
    }
    CNA_STUDIO_EXPECT(ratio <= 8.0);
}

// ------------------------------------------------------------------------------------------------
// Hierarchy index caching (STUDIO-30011)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(TheHierarchyIndexIsBuiltOnceAndGivenUpByEveryWayOfChangingTheScene)
{
    // The index used to be rebuilt on every call, deliberately, because the document could not
    // know when a parent changed and a silently stale hierarchy presents as entities *vanishing
    // from the outliner* -- a failure that looks like data loss rather than like a cache. Caching
    // it is only allowed if every way of changing the scene gives it up, so this enumerates them.
    SceneDocument scene;

    StudioEntity first{Uuid::generate(), "First"};
    const Uuid firstId = first.getId();
    scene.addEntity(std::move(first));

    StudioEntity second{Uuid::generate(), "Second"};
    const Uuid secondId = second.getId();
    scene.addEntity(std::move(second));

    // Built once and then reused, however many times it is asked for.
    const std::uint64_t afterFirstAsk = [&] {
        (void)scene.getChildrenByParent();
        return scene.getHierarchyRebuildCount();
    }();
    (void)scene.getChildrenByParent();
    (void)scene.getChildrenByParent();
    CNA_STUDIO_EXPECT_EQ(scene.getHierarchyRebuildCount(), afterFirstAsk);

    const auto rebuildsAfter = [&](auto&& change) {
        (void)scene.getChildrenByParent();
        const std::uint64_t before = scene.getHierarchyRebuildCount();
        change();
        (void)scene.getChildrenByParent();
        return scene.getHierarchyRebuildCount() > before;
    };

    // Adding.
    Uuid thirdId;
    CNA_STUDIO_EXPECT(rebuildsAfter([&] {
        StudioEntity third{Uuid::generate(), "Third"};
        thirdId = third.getId();
        scene.addEntity(std::move(third));
    }));

    // Reparenting through the document.
    CNA_STUDIO_EXPECT(rebuildsAfter([&] {
        CNA_STUDIO_EXPECT(scene.reparentEntity(secondId, firstId));
    }));
    CNA_STUDIO_EXPECT_EQ(scene.getChildrenByParent().at(firstId).size(), std::size_t{1});

    // And the hole this was left uncached for: a parent changed through the mutable entity the
    // document hands out, behind its back. Asking for that handle is what invalidates, because it
    // is the only moment the document can still see coming.
    CNA_STUDIO_EXPECT(rebuildsAfter([&] {
        StudioEntity* entity = scene.findEntityForEdit(thirdId);
        CNA_STUDIO_EXPECT(entity != nullptr);
        if (entity != nullptr) { entity->setParentId(firstId); }
    }));
    CNA_STUDIO_EXPECT_EQ(scene.getChildrenByParent().at(firstId).size(), std::size_t{2});

    // A rename through the same handle reorders a parent's children, which is the same index.
    CNA_STUDIO_EXPECT(rebuildsAfter([&] {
        StudioEntity* entity = scene.findEntityForEdit(thirdId);
        if (entity != nullptr) { entity->setName("AAA"); }
    }));
    CNA_STUDIO_EXPECT_EQ(scene.getChildrenByParent().at(firstId).front().toString(),
                         thirdId.toString());

    // Removing.
    CNA_STUDIO_EXPECT(rebuildsAfter([&] {
        CNA_STUDIO_EXPECT_EQ(scene.removeEntityRecursive(thirdId).size(), std::size_t{1});
    }));
    CNA_STUDIO_EXPECT_EQ(scene.getChildrenByParent().at(firstId).size(), std::size_t{1});

    // Loading, which replaces every entity at once.
    ComponentRegistry registry;
    registerBuiltinComponents(registry);
    const JsonValue saved = scene.toJson();

    CNA_STUDIO_EXPECT(rebuildsAfter([&] {
        CNA_STUDIO_EXPECT(scene.loadFromJson(saved, registry).succeeded);
    }));

    // Clearing.
    CNA_STUDIO_EXPECT(rebuildsAfter([&] { scene.clear(); }));
    CNA_STUDIO_EXPECT(scene.getChildrenByParent().empty());
}

CNA_STUDIO_TEST(ACachedHierarchyIsTheSameHierarchyAsARebuiltOne)
{
    // The cache must be indistinguishable from the pass it replaces, including the *order* within
    // each parent -- two orderings of one hierarchy show as an outliner whose rows move when
    // something unrelated rebuilt them, which nobody can reproduce on demand.
    SceneDocument scene;
    std::vector<Uuid> ids;
    Uuid parent;
    for (int i = 0; i < 80; ++i)
    {
        StudioEntity entity{Uuid::generate(), "Entity " + std::to_string((i * 11) % 80)};
        entity.setSortOrder(i % 4);
        if (i % 7 != 0) { entity.setParentId(parent); }
        const Uuid id = entity.getId();
        scene.addEntity(std::move(entity));
        ids.push_back(id);
        if (i % 7 == 0) { parent = id; }
    }

    const std::unordered_map<Uuid, std::vector<Uuid>> cached = scene.getChildrenByParent();

    // Every parent, against the scan the grouped form exists to replace.
    CNA_STUDIO_EXPECT(cached.at(Uuid{}) == scene.getRootEntities());
    for (const Uuid& id : ids)
    {
        const std::vector<Uuid> expected = scene.getChildren(id);
        const auto found = cached.find(id);
        if (expected.empty()) { CNA_STUDIO_EXPECT(found == cached.end()); }
        else
        {
            CNA_STUDIO_EXPECT(found != cached.end());
            if (found != cached.end()) { CNA_STUDIO_EXPECT(found->second == expected); }
        }
    }

    // And asking again gives the same answer rather than an accumulated one -- a cache rebuilt
    // into a container it forgot to clear would double every child list.
    scene.invalidateHierarchy();
    CNA_STUDIO_EXPECT(scene.getChildrenByParent() == cached);
}

CNA_STUDIO_TEST(SelectionRootsAnswerTheSameWayWhateverTheSelectionCosts)
{
    // `findSelectionRoots` decides what a multi-entity gizmo drag actually moves: an entity whose
    // ancestor is also selected must not be moved twice, once by itself and once by its parent.
    // The ancestor test was a linear scan of the whole selection, per step, per entity -- fine by
    // hand and quadratic for a select-all, which is one keystroke away. This is the behaviour that
    // had to survive making it a lookup (`plan.md` STUDIO-30026).
    SceneDocument scene;

    std::vector<Uuid> chain;
    Uuid parent;
    for (int i = 0; i < 12; ++i)
    {
        StudioEntity entity{Uuid::generate(), "Link " + std::to_string(i)};
        if (i > 0) { entity.setParentId(parent); }
        parent = entity.getId();
        chain.push_back(parent);
        scene.addEntity(std::move(entity));
    }

    // A second chain, so "selected somewhere else" is not mistaken for "selected above me".
    StudioEntity other{Uuid::generate(), "Elsewhere"};
    const Uuid elsewhere = other.getId();
    scene.addEntity(std::move(other));

    // The whole chain selected collapses to its one root.
    std::vector<Uuid> everything = chain;
    everything.push_back(elsewhere);
    const std::vector<Uuid> roots = findSelectionRoots(scene, everything);
    CNA_STUDIO_EXPECT_EQ(roots.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(roots.front() == chain.front());
    CNA_STUDIO_EXPECT(roots.back() == elsewhere);

    // At *any* depth, not just one level up: a gap in the selection does not make a second root,
    // because the entity below the gap still has a selected ancestor further up. This is the
    // property that makes the walk a walk rather than a parent check, and the one a lookup could
    // quietly have broken by stopping at the first step.
    const std::vector<Uuid> split{chain[0], chain[1], chain[5], chain[6]};
    const std::vector<Uuid> splitRoots = findSelectionRoots(scene, split);
    CNA_STUDIO_EXPECT_EQ(splitRoots.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(splitRoots.front() == chain[0]);

    // Two roots need two subtrees, which is what the second chain is for.
    const std::vector<Uuid> disjoint{chain[5], elsewhere};
    const std::vector<Uuid> disjointRoots = findSelectionRoots(scene, disjoint);
    CNA_STUDIO_EXPECT_EQ(disjointRoots.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(disjointRoots.front() == chain[5]);
    CNA_STUDIO_EXPECT(disjointRoots.back() == elsewhere);

    // A deep entity on its own is its own root, however much is above it unselected.
    const std::vector<Uuid> lone{chain.back()};
    CNA_STUDIO_EXPECT(findSelectionRoots(scene, lone) == lone);

    // And the order is the selection's, not the hierarchy's -- a drag that reordered what it moved
    // would apply the same delta in a different sequence, which matters once snapping is involved.
    const std::vector<Uuid> reversed{elsewhere, chain[6], chain[5]};
    const std::vector<Uuid> reversedRoots = findSelectionRoots(scene, reversed);
    CNA_STUDIO_EXPECT_EQ(reversedRoots.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(reversedRoots.front() == elsewhere);
    CNA_STUDIO_EXPECT(reversedRoots.back() == chain[5]);

    CNA_STUDIO_EXPECT(findSelectionRoots(scene, {}).empty());
}

// ------------------------------------------------------------------------------------------------
// Debug views (STUDIO-11011)
// ------------------------------------------------------------------------------------------------

/**
 * @brief Each view is the channel it names, and the pair Unlit/Lighting Only really is a pair.
 *
 * The decision function on its own, because that is the whole of what a debug view is: everything
 * downstream draws an ordinary `SceneModelBatch` and has no idea one of these is on.
 */
CNA_STUDIO_TEST(EachDebugViewDrawsTheChannelItNamesAndNothingElse)
{
    MeshMaterial source;
    source.diffuseColor = StudioVector3{0.8f, 0.2f, 0.1f};
    source.emissiveColor = StudioVector3{0.3f, 0.3f, 0.3f};
    source.specularColor = StudioVector3{0.5f, 0.5f, 0.5f};
    source.specularPower = 32.0f;
    source.metallic = 0.75f;
    source.roughness = 0.25f;
    source.diffuseTexturePath = "textures/crate.png";

    // None changes nothing, so a caller can apply this unconditionally rather than branching.
    const MeshMaterial none = studioDebugMaterial(StudioDebugView::None, source);
    CNA_STUDIO_EXPECT(none.diffuseColor.x == source.diffuseColor.x);
    CNA_STUDIO_EXPECT_EQ(none.diffuseTexturePath, source.diffuseTexturePath);
    CNA_STUDIO_EXPECT(none.emissiveColor.x == source.emissiveColor.x);

    // Unlit is the albedo: the base colour and its map, with the emission dropped. Keeping the
    // emission would make an emissive surface the brightest albedo in the scene, which is the one
    // reading this view exists to rule out.
    const MeshMaterial unlit = studioDebugMaterial(StudioDebugView::Unlit, source);
    CNA_STUDIO_EXPECT(unlit.diffuseColor.x == source.diffuseColor.x);
    CNA_STUDIO_EXPECT_EQ(unlit.diffuseTexturePath, source.diffuseTexturePath);
    CNA_STUDIO_EXPECT(unlit.emissiveColor.x == 0.0f);

    // Lighting Only is its complement: white, no map, so what is left on screen is the light.
    const MeshMaterial lighting = studioDebugMaterial(StudioDebugView::LightingOnly, source);
    CNA_STUDIO_EXPECT(lighting.diffuseColor.x == 1.0f);
    CNA_STUDIO_EXPECT(lighting.diffuseColor.y == 1.0f);
    CNA_STUDIO_EXPECT(lighting.diffuseTexturePath.empty());

    // The highlight survives, because a specular response is light and dropping it would hide the
    // half of the lighting that is hardest to get right.
    CNA_STUDIO_EXPECT(lighting.specularColor.x == source.specularColor.x);
    CNA_STUDIO_EXPECT(lighting.specularPower == source.specularPower);

    // The channels are grey, and they are each other's opposite on this material -- which is what
    // catches the one mistake worth catching here, the two reading the same field.
    const MeshMaterial metallic = studioDebugMaterial(StudioDebugView::Metallic, source);
    const MeshMaterial roughness = studioDebugMaterial(StudioDebugView::Roughness, source);
    CNA_STUDIO_EXPECT(std::fabs(metallic.diffuseColor.x - 0.75f) < 0.001f);
    CNA_STUDIO_EXPECT(metallic.diffuseColor.x == metallic.diffuseColor.z);
    CNA_STUDIO_EXPECT(std::fabs(roughness.diffuseColor.x - 0.25f) < 0.001f);
    CNA_STUDIO_EXPECT(roughness.diffuseColor.x == roughness.diffuseColor.z);

    // No maps on a channel view: a number multiplied by a picture is a picture.
    CNA_STUDIO_EXPECT(metallic.diffuseTexturePath.empty());
    CNA_STUDIO_EXPECT(roughness.diffuseTexturePath.empty());
}

/**
 * @brief Only the two lit views keep the scene's lights; the rest are drawn flat.
 *
 * Flat is not a flag the renderer is told about -- it is a white ambient with no directional
 * lights, which both effects already compute to the base colour exactly. That is what lets the
 * whole feature stay CNA-free, and it is worth a case of its own because the obvious shortcut,
 * leaving `useDefaultLighting` set, would put XNA's three-point rig back over the channel.
 */
CNA_STUDIO_TEST(ADebugViewKeepsTheLightsOnlyWhereTheyAreThePoint)
{
    EffectLighting scene;
    scene.useDefaultLighting = false;
    scene.ambientColor = StudioVector3{0.1f, 0.1f, 0.12f};
    scene.lightCount = 2;

    CNA_STUDIO_EXPECT(studioDebugViewIsLit(StudioDebugView::None));
    CNA_STUDIO_EXPECT(studioDebugViewIsLit(StudioDebugView::LightingOnly));
    CNA_STUDIO_EXPECT(!studioDebugViewIsLit(StudioDebugView::Unlit));
    CNA_STUDIO_EXPECT(!studioDebugViewIsLit(StudioDebugView::Metallic));
    CNA_STUDIO_EXPECT(!studioDebugViewIsLit(StudioDebugView::Roughness));
    CNA_STUDIO_EXPECT(!studioDebugViewIsLit(StudioDebugView::Normals));

    CNA_STUDIO_EXPECT_EQ(studioDebugLighting(StudioDebugView::None, scene).lightCount,
                         std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(studioDebugLighting(StudioDebugView::LightingOnly, scene).lightCount,
                         std::size_t{2});

    const EffectLighting flat = studioDebugLighting(StudioDebugView::Roughness, scene);
    CNA_STUDIO_EXPECT_EQ(flat.lightCount, std::size_t{0});
    CNA_STUDIO_EXPECT(flat.ambientColor.x == 1.0f);
    CNA_STUDIO_EXPECT(flat.ambientColor.y == 1.0f);
    CNA_STUDIO_EXPECT(flat.ambientColor.z == 1.0f);

    // The one that would otherwise slip through: XNA's own rig, put back over the channel by a
    // `useDefaultLighting` nobody cleared.
    CNA_STUDIO_EXPECT(!flat.useDefaultLighting);
}

/** @brief The normal colour is the convention every normal map is written in. */
CNA_STUDIO_TEST(ANormalSegmentIsColouredTheWayANormalMapIs)
{
    // +X is red, +Y green, +Z blue, and the negatives are the same channels at zero -- which is
    // what makes an inverted face obvious rather than merely different.
    CNA_STUDIO_EXPECT_EQ(int{studioNormalColor(StudioVector3{1.0f, 0.0f, 0.0f}).r}, 255);
    CNA_STUDIO_EXPECT_EQ(int{studioNormalColor(StudioVector3{-1.0f, 0.0f, 0.0f}).r}, 0);
    CNA_STUDIO_EXPECT_EQ(int{studioNormalColor(StudioVector3{0.0f, 1.0f, 0.0f}).g}, 255);
    CNA_STUDIO_EXPECT_EQ(int{studioNormalColor(StudioVector3{0.0f, 0.0f, 1.0f}).b}, 255);

    // Normalised first, so a normal of any length lands on the same colour as the unit one. A
    // reader that skipped this would colour an unnormalised mesh by its vertex scale.
    CNA_STUDIO_EXPECT_EQ(int{studioNormalColor(StudioVector3{7.0f, 0.0f, 0.0f}).r}, 255);

    // A degenerate normal is mid-grey rather than a division by zero, because a mesh with one is
    // exactly what somebody opens this view to find.
    const StudioColor degenerate = studioNormalColor(StudioVector3{0.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(degenerate.r > 120 && degenerate.r < 136);
}

/**
 * @brief A channel is resolved per part, so a model of several materials is right about each.
 *
 * The mistake this rules out is the cheap implementation: take the model's first material, show
 * its number everywhere. On a single-material crate that is correct, which is why it would ship.
 */
CNA_STUDIO_TEST(AChannelViewReadsEachPartsOwnMaterialRatherThanTheFirstOne)
{
    MeshData mesh = makeTinyMesh();
    mesh.parts[0].name = "Body";
    mesh.parts[0].materialIndex = 0;

    MeshPart second = mesh.parts[0];
    second.name = "Trim";
    second.materialIndex = 1;
    mesh.parts.push_back(std::move(second));

    MeshMaterial body;
    body.roughness = 0.2f;
    MeshMaterial trim;
    trim.roughness = 0.9f;
    mesh.materials = {body, trim};

    ModelDraw draw;
    draw.mesh = &mesh;
    draw.lighting.lightCount = 3;

    applyDebugViewToDraw(StudioDebugView::Roughness, draw);

    CNA_STUDIO_EXPECT_EQ(draw.partMaterials.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(draw.partMaterials[0].first, std::string{"Body"});
    CNA_STUDIO_EXPECT(std::fabs(draw.partMaterials[0].second.diffuseColor.x - 0.2f) < 0.001f);
    CNA_STUDIO_EXPECT_EQ(draw.partMaterials[1].first, std::string{"Trim"});
    CNA_STUDIO_EXPECT(std::fabs(draw.partMaterials[1].second.diffuseColor.x - 0.9f) < 0.001f);

    // And the lights go with it, in the same pass, so a channel is never multiplied by whatever
    // happens to be shining on it.
    CNA_STUDIO_EXPECT_EQ(draw.lighting.lightCount, std::size_t{0});
}

/**
 * @brief The channel reports the material that would have been drawn, override and all.
 *
 * An entity whose `ModelRenderer` overrides its model's material is looking at the override, and a
 * roughness view that read past it to the model's own would answer a question nobody asked.
 */
CNA_STUDIO_TEST(AChannelViewFollowsTheOverrideTheRendererWouldHaveDrawn)
{
    MeshData mesh = makeTinyMesh();
    mesh.parts[0].name = "Body";
    mesh.parts[0].materialIndex = 0;

    MeshMaterial own;
    own.metallic = 0.0f;
    mesh.materials = {own};

    MeshMaterial override;
    override.metallic = 1.0f;

    ModelDraw draw;
    draw.mesh = &mesh;
    draw.materialOverride = override;

    applyDebugViewToDraw(StudioDebugView::Metallic, draw);

    CNA_STUDIO_EXPECT_EQ(draw.partMaterials.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(draw.partMaterials[0].second.diffuseColor.x == 1.0f);

    // A per-part override is more specific still, and wins over the model-wide one -- the same
    // order the renderer resolves in.
    ModelDraw perPart;
    perPart.mesh = &mesh;
    perPart.materialOverride = override;
    MeshMaterial trim;
    trim.metallic = 0.5f;
    perPart.partMaterials.emplace_back("Body", trim);

    applyDebugViewToDraw(StudioDebugView::Metallic, perPart);
    CNA_STUDIO_EXPECT(std::fabs(perPart.partMaterials[0].second.diffuseColor.x - 0.5f) < 0.001f);
}

/** @brief The batch builder applies the view, so the whole feature is one argument to one call. */
CNA_STUDIO_TEST(TheModelBatchCarriesTheDebugViewItWasAskedFor)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    MeshData mesh = makeTinyMesh();
    mesh.parts[0].name = "Body";
    mesh.parts[0].materialIndex = 0;

    MeshMaterial material;
    material.diffuseColor = StudioVector3{0.9f, 0.1f, 0.1f};
    material.metallic = 1.0f;
    mesh.materials = {material};

    const Uuid modelId = Uuid::generate();
    StudioEntity entity = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, entity, modelId);
    scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider provider = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };

    // Default: untouched, so every existing caller keeps meaning what it meant.
    const SceneModelBatch plain = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(plain.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(plain.draws[0].partMaterials.empty());
    CNA_STUDIO_EXPECT(!plain.draws[0].materialOverride.has_value());

    const SceneModelBatch metallic =
        buildSceneModelBatch(scene, camera, provider, {}, {}, StudioDebugView::Metallic);
    CNA_STUDIO_EXPECT_EQ(metallic.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(metallic.draws[0].partMaterials.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(metallic.draws[0].partMaterials[0].second.diffuseColor.x == 1.0f);
    CNA_STUDIO_EXPECT_EQ(metallic.draws[0].lighting.lightCount, std::size_t{0});
}

/**
 * @brief STUDIO-19009: the material a user assigns replaces the model's own, on every part.
 *
 * The end of the chain STUDIO-10007, STUDIO-19001 and the typed slot make: a material can be
 * created, edited, picked, and — here — drawn. `CNA.ModelRenderer` has declared this reference
 * since Phase 1, and until ED-403 there was nothing to point it at; the per-part list has a case
 * of its own and the whole-model slot, which is the one an ordinary user fills, had none.
 *
 * "On every part" is the half worth pinning. One material is the only thing a single override can
 * mean for a model of several, and an implementation that applied it to the first part would look
 * correct on the crate everybody tests with.
 */
CNA_STUDIO_TEST(TheMaterialAnEntityNamesReplacesItsModelsOwnOnEveryPart)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    MeshData mesh = makeTinyMesh();
    mesh.parts[0].name = "Body";
    mesh.parts[0].materialIndex = 0;
    MeshPart lid = mesh.parts[0];
    lid.name = "Lid";
    lid.materialIndex = 1;
    mesh.parts.push_back(std::move(lid));

    MeshMaterial body;
    body.name = "Model Body";
    MeshMaterial trim;
    trim.name = "Model Lid";
    mesh.materials = {body, trim};

    const Uuid modelId = Uuid::generate();
    const Uuid materialId = Uuid::generate();

    StudioEntity entity = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, entity, modelId);
    entity.findComponent(BuiltinComponentIds::kModelRenderer)
        ->setProperty("material", PropertyValue{PropertyValue::AssetReference{materialId}});
    scene.addEntity(std::move(entity));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider meshes = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };

    const MaterialProvider materials = [&](const Uuid& id) -> std::optional<MeshMaterial> {
        if (id != materialId) { return std::nullopt; }
        MeshMaterial assigned;
        assigned.name = "Assigned";
        assigned.metallic = 1.0f;
        return assigned;
    };

    const SceneModelBatch batch = buildSceneModelBatch(scene, camera, meshes, {}, materials);
    CNA_STUDIO_EXPECT_EQ(batch.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(batch.draws[0].materialOverride.has_value());
    if (batch.draws[0].materialOverride.has_value())
    {
        CNA_STUDIO_EXPECT_EQ(batch.draws[0].materialOverride->name, std::string{"Assigned"});
    }

    // Model-wide, so nothing names a part: the renderer falls back to the override for every part
    // that the per-part list does not claim, and the list is empty here.
    CNA_STUDIO_EXPECT(batch.draws[0].partMaterials.empty());

    // A material that has not loaded draws with the model's own rather than not at all -- the same
    // answer `MeshProvider` returning nullptr gets from the mesh side. An entity that vanished
    // while its material was still being read would be the worse failure by far.
    const SceneModelBatch unresolved = buildSceneModelBatch(
        scene, camera, meshes, {},
        [](const Uuid&) -> std::optional<MeshMaterial> { return std::nullopt; });
    CNA_STUDIO_EXPECT_EQ(unresolved.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(!unresolved.draws[0].materialOverride.has_value());

    // And with no provider at all, which is the headless preview and every test that does not care
    // about materials.
    const SceneModelBatch none = buildSceneModelBatch(scene, camera, meshes);
    CNA_STUDIO_EXPECT_EQ(none.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(!none.draws[0].materialOverride.has_value());
}

// ------------------------------------------------------------------------------------------------
// Transparency (STUDIO-19004)
// ------------------------------------------------------------------------------------------------

/**
 * @brief Blended parts are drawn after the opaque ones, furthest from the eye first.
 *
 * Blending is not commutative with depth: a transparent pane drawn before what is behind it blends
 * against the background instead, and the result is a window with a hole in it. The order is the
 * whole of the fix and it is the part that can be wrong, so it is a CNA-free function with a case
 * of its own rather than a loop inside the renderer nothing can reach.
 */
CNA_STUDIO_TEST(BlendedDrawsFollowTheOpaqueOnesAndAreSortedBackToFront)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    MeshData opaqueMesh = makeTinyMesh();
    opaqueMesh.parts[0].name = "Body";
    opaqueMesh.parts[0].materialIndex = 0;
    MeshMaterial solid;
    solid.name = "Solid";
    opaqueMesh.materials = {solid};

    MeshData glassMesh = makeTinyMesh();
    glassMesh.parts[0].name = "Pane";
    glassMesh.parts[0].materialIndex = 0;
    MeshMaterial glass;
    glass.name = "Glass";
    glass.alphaMode = MeshAlphaMode::Blend;
    glassMesh.materials = {glass};

    const Uuid opaqueModel = Uuid::generate();
    const Uuid glassModel = Uuid::generate();

    // A wall, then a near pane, then a far one -- added in an order that is neither the answer nor
    // its reverse, so a stable sort that did nothing would be caught.
    StudioEntity wall = makeEntity(registry, "Wall", 0.0f, 0.0f);
    addModelRenderer(registry, wall, opaqueModel);
    scene.addEntity(std::move(wall));

    StudioEntity near = makeEntity(registry, "Near", 0.0f, 0.0f);
    addModelRenderer(registry, near, glassModel);
    const Uuid nearId = near.getId();
    scene.addEntity(std::move(near));

    StudioEntity far = makeEntity(registry, "Far", 0.0f, 0.0f);
    addModelRenderer(registry, far, glassModel);
    const Uuid farId = far.getId();
    scene.addEntity(std::move(far));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    // Along the camera's own forward axis, so "further" is unambiguous whichever way the default
    // camera happens to look: the near pane is put where the camera is and the far one well beyond.
    const StudioVector3 forward = camera.getForward();
    const auto place = [&scene](const Uuid& id, const StudioVector3& position) {
        StudioComponent* transform =
            scene.findEntityForEdit(id)->findComponent(BuiltinComponentIds::kTransform);
        transform->setProperty("position", PropertyValue{position});
    };
    place(nearId, scale(forward, 5.0f));
    place(farId, scale(forward, 50.0f));

    const MeshProvider meshes = [&](const Uuid& id) -> const MeshData* {
        if (id == opaqueModel) { return &opaqueMesh; }
        if (id == glassModel) { return &glassMesh; }
        return nullptr;
    };

    const SceneModelBatch batch = buildSceneModelBatch(scene, camera, meshes);
    CNA_STUDIO_EXPECT_EQ(batch.draws.size(), std::size_t{3});

    const SceneDrawOrder order = orderSceneModelDraws(batch);

    // The wall alone is opaque; the two panes are not, so they are in neither each other's pass
    // nor the wall's.
    CNA_STUDIO_EXPECT_EQ(order.opaque.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(batch.draws[order.opaque[0]].entityId.toString(),
                         batch.draws[0].entityId.toString());

    CNA_STUDIO_EXPECT_EQ(order.blended.size(), std::size_t{2});
    if (order.blended.size() == 2)
    {
        // Furthest first. Drawing the near pane first would blend it against the background and
        // then let the far one draw over it, which is a window with a hole in it.
        CNA_STUDIO_EXPECT_EQ(batch.draws[order.blended[0]].entityId.toString(), farId.toString());
        CNA_STUDIO_EXPECT_EQ(batch.draws[order.blended[1]].entityId.toString(), nearId.toString());
    }
}

/**
 * @brief A model with a pane in it is in both passes, and a masked part stays in the opaque one.
 *
 * The two cases a per-draw split would get wrong. A window frame with glass in it is one model and
 * both of its halves have to be drawn; a cut-out leaf is a hard edge that writes depth, so sorting
 * it would be paying for an ordering it does not need.
 */
CNA_STUDIO_TEST(AModelWithBothKindsOfPartIsDrawnInBothPassesAndMaskStaysOpaque)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    MeshData mesh = makeTinyMesh();
    mesh.parts[0].name = "Frame";
    mesh.parts[0].materialIndex = 0;

    MeshPart pane = mesh.parts[0];
    pane.name = "Pane";
    pane.materialIndex = 1;
    mesh.parts.push_back(std::move(pane));

    MeshMaterial frame;
    frame.name = "Frame";
    MeshMaterial glass;
    glass.name = "Glass";
    glass.alphaMode = MeshAlphaMode::Blend;
    mesh.materials = {frame, glass};

    const Uuid modelId = Uuid::generate();
    StudioEntity window = makeEntity(registry, "Window", 0.0f, 0.0f);
    addModelRenderer(registry, window, modelId);
    scene.addEntity(std::move(window));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider meshes = [&](const Uuid& id) { return id == modelId ? &mesh : nullptr; };

    const SceneModelBatch batch = buildSceneModelBatch(scene, camera, meshes);
    const SceneDrawOrder both = orderSceneModelDraws(batch);
    CNA_STUDIO_EXPECT_EQ(both.opaque.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(both.blended.size(), std::size_t{1});

    // A masked part is a cut-out rather than a fade, so it belongs with the solid geometry.
    mesh.materials[1].alphaMode = MeshAlphaMode::Mask;
    const SceneModelBatch masked = buildSceneModelBatch(scene, camera, meshes);
    const SceneDrawOrder cutout = orderSceneModelDraws(masked);
    CNA_STUDIO_EXPECT_EQ(cutout.opaque.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(cutout.blended.empty());

    // And the mode a material *asset* names wins over the model's own, because that is the
    // material the renderer would draw with (STUDIO-19009).
    mesh.materials[1].alphaMode = MeshAlphaMode::Blend;
    MeshMaterial assigned;
    assigned.alphaMode = MeshAlphaMode::Opaque;

    SceneModelBatch overridden = buildSceneModelBatch(scene, camera, meshes);
    overridden.draws[0].materialOverride = assigned;
    const SceneDrawOrder solid = orderSceneModelDraws(overridden);
    CNA_STUDIO_EXPECT_EQ(solid.opaque.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(solid.blended.empty());
}

/**
 * @brief A light the *editor* makes lights the models the editor draws.
 *
 * `plan.md` STUDIO-20001. Every other case here builds its `CNA.Light` by hand, setting each
 * property to what the test wants — which is exactly the shape that cannot tell whether the light
 * a *user* gets is a light at all. The archetype's defaults are what a new Directional Light
 * arrives with, and an archetype whose kind, colour or intensity were wrong would pass every one
 * of those cases and still light nothing.
 *
 * End to end on purpose: the archetype builds the entity, `buildSceneModelBatch` resolves what
 * each model is lit by, and the assertion is on the `EffectLighting` the renderer is handed.
 */
CNA_STUDIO_TEST(ADirectionalLightCreatedFromTheEditorLightsTheModelsInTheScene)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioEntity crate = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, crate, modelId);
    (void)scene.addEntity(std::move(crate));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider provider = [&](const Uuid& queried) {
        return queried == modelId ? &mesh : nullptr;
    };

    // Before: nothing lights the scene, so the renderer is told to use XNA's own default rather
    // than to draw the crate black.
    const SceneModelBatch unlit = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(unlit.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(unlit.draws[0].lighting.useDefaultLighting);

    // Now add exactly what the Entity menu adds (`plan.md` STUDIO-13013) -- no properties set
    // here, because the point of this case is what the defaults do.
    const StudioEntityArchetype* archetype = studioFindEntityArchetype("light.directional");
    CNA_STUDIO_EXPECT(archetype != nullptr);
    if (archetype == nullptr) { return; }

    const Uuid lightId = scene.addEntity(studioMakeArchetypeEntity(*archetype, registry));

    const SceneModelBatch lit = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(lit.draws.size(), std::size_t{1});

    const EffectLighting& lighting = lit.draws[0].lighting;
    CNA_STUDIO_EXPECT(!lighting.useDefaultLighting);
    CNA_STUDIO_EXPECT_EQ(lighting.lightCount, std::size_t{1});

    // White at full intensity, which is what makes a newly added light visibly do something. A
    // light that arrived black would satisfy every structural assertion above.
    const EffectDirectionalLight& first = lighting.lights[0];
    CNA_STUDIO_EXPECT(first.diffuseColor.x > 0.9f);
    CNA_STUDIO_EXPECT(first.diffuseColor.y > 0.9f);
    CNA_STUDIO_EXPECT(first.diffuseColor.z > 0.9f);

    // And it shines along the entity's own forward axis, so the rotate gizmo aims it -- which is
    // the whole of what "authoring" a directional light means. Unrotated, that is +Z.
    CNA_STUDIO_EXPECT(cameraNearlyEqual(first.direction.z, 1.0f, 0.001f));

    StudioComponent* transform =
        scene.findEntityForEdit(lightId)->findComponent(BuiltinComponentIds::kTransform);
    transform->setProperty(
        "rotation", PropertyValue{quaternionFromEulerDegrees(StudioVector3{0.0f, 90.0f, 0.0f})});

    const SceneModelBatch turned = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(turned.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(cameraNearlyEqual(turned.draws[0].lighting.lights[0].direction.z, 0.0f,
                                        0.001f));

    // Its *position* is nothing to a directional light, which is why Range is greyed out for one
    // (`STUDIO-20001`). Moving it a hundred units away changes what it lights by nothing at all.
    const StudioVector3 before = turned.draws[0].lighting.lights[0].direction;
    transform->setProperty("position", PropertyValue{StudioVector3{100.0f, 100.0f, 100.0f}});

    const SceneModelBatch moved = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(moved.draws.size(), std::size_t{1});
    const EffectLighting& after = moved.draws[0].lighting;
    CNA_STUDIO_EXPECT(!after.useDefaultLighting);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(after.lights[0].direction.x, before.x, 0.001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(after.lights[0].direction.z, before.z, 0.001f));
}

/**
 * @brief A point light arrives as one, lights from where it is, and stops at its range.
 *
 * `plan.md` STUDIO-20002. The three kinds are one component told apart by an enumeration, so
 * "a point light" cannot be said in components at all — which is what
 * `StudioEntityArchetype::presets` exists for. An archetype whose preset silently did nothing
 * would hand the user a *directional* light called "Point Light", and every structural assertion
 * about components would still pass.
 */
CNA_STUDIO_TEST(APointLightCreatedFromTheEditorLightsFromWhereItIsAndStopsAtItsRange)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioEntity crate = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, crate, modelId);
    (void)scene.addEntity(std::move(crate));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider provider = [&](const Uuid& queried) {
        return queried == modelId ? &mesh : nullptr;
    };

    const StudioEntityArchetype* archetype = studioFindEntityArchetype("light.point");
    CNA_STUDIO_EXPECT(archetype != nullptr);
    if (archetype == nullptr) { return; }

    StudioEntity lamp = studioMakeArchetypeEntity(*archetype, registry);

    // The preset landed: this is a Point in the document, not a Directional one wearing the name.
    const StudioComponent* asMade = lamp.findComponent(BuiltinComponentIds::kLight);
    CNA_STUDIO_EXPECT(asMade != nullptr);
    if (asMade == nullptr) { return; }
    CNA_STUDIO_EXPECT_EQ(asMade->getProperty("kind").get<PropertyValue::EnumValue>().name,
                         std::string{"Point"});

    // Its colour, intensity and range are still the descriptor's: a preset says what makes a point
    // light a point light and leaves the rest alone.
    CNA_STUDIO_EXPECT_EQ(asMade->getProperty("intensity").get<float>(), 1.0f);
    CNA_STUDIO_EXPECT(asMade->getProperty("range").get<float>() > 0.0f);

    const Uuid lampId = scene.addEntity(std::move(lamp));

    // Beside the crate and well inside its range: the crate is lit, and lit from where the lamp is
    // rather than along the lamp's own axis. That is the whole difference from a directional one.
    StudioComponent* transform =
        scene.findEntityForEdit(lampId)->findComponent(BuiltinComponentIds::kTransform);
    transform->setProperty("position", PropertyValue{StudioVector3{4.0f, 0.0f, 0.0f}});

    const SceneModelBatch near = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(near.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(!near.draws[0].lighting.useDefaultLighting);

    // Sent whole through the punctual slot (`plan.md` STUDIO-20003), which is where a point light
    // belongs: its own position and range, for the effect to measure per pixel. This case used to
    // assert the directional approximation, because that was all Studio spoke.
    CNA_STUDIO_EXPECT(near.draws[0].lighting.hasPunctual);
    CNA_STUDIO_EXPECT(near.draws[0].lighting.punctual.kind == SceneLightKind::Point);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(near.draws[0].lighting.punctual.position.x, 4.0f, 0.001f));

    // Move the lamp to the other side and the light moves with it. A directional light would not
    // have moved at all, which is what `STUDIO-20001`'s case asserts.
    transform->setProperty("position", PropertyValue{StudioVector3{-4.0f, 0.0f, 0.0f}});
    const SceneModelBatch across = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(across.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(across.draws[0].lighting.hasPunctual);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(across.draws[0].lighting.punctual.position.x, -4.0f,
                                        0.001f));

    // And beyond its range it lights nothing, so the scene falls back to the default rather than
    // drawing the crate black.
    const float range = asMade->getProperty("range").get<float>();
    transform->setProperty("position",
                           PropertyValue{StudioVector3{range * 10.0f, 0.0f, 0.0f}});
    const SceneModelBatch far = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(far.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(far.draws[0].lighting.useDefaultLighting);
}

/**
 * @brief The overlay draws an arrow only where direction means something, and a ring only where
 *        range does.
 *
 * `plan.md` STUDIO-20002. A point light shines equally in every direction, and it was drawn with
 * the same aim arrow a directional light gets: turning the entity swung a line about the viewport
 * and changed how the scene was lit by nothing at all. That is an indicator of a fact that does
 * not exist, and it is the same defect as the editable Range field `STUDIO-20001` took out of the
 * Inspector — one layer over.
 */
CNA_STUDIO_TEST(TheLightOverlayDrawsAnArrowOnlyWhereDirectionMeansSomething)
{
    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});

    const auto segmentsFor = [&camera](SceneLightKind kind, float range) {
        SceneLight light;
        light.kind = kind;
        light.position = StudioVector3{0.0f, 0.0f, 0.0f};
        light.direction = StudioVector3{0.0f, 0.0f, 1.0f};
        light.range = range;

        std::vector<WireSegment> segments;
        (void)appendLightVisualisation(segments, camera, light,
                                       StudioColor{255, 255, 255, 255}, 1024);
        return segments.size();
    };

    // A directional light: the arrow and nothing else. No ring, because it reaches everything and
    // a boundary the user could drag would be a boundary that means nothing.
    const std::size_t directional = segmentsFor(SceneLightKind::Directional, 10.0f);
    CNA_STUDIO_EXPECT(directional > 0);

    // A point light: the ring and no arrow. Fewer segments than a spot light, which has both --
    // asserted as a comparison rather than as a count, because the ring's sample rate is a
    // drawing decision and this case is about which parts are drawn at all.
    const std::size_t point = segmentsFor(SceneLightKind::Point, 10.0f);
    const std::size_t spot = segmentsFor(SceneLightKind::Spot, 10.0f);
    CNA_STUDIO_EXPECT(point > 0);
    CNA_STUDIO_EXPECT_EQ(spot, point + directional);

    // A point light with no range has nothing to draw at all: no ring, and no arrow either.
    CNA_STUDIO_EXPECT_EQ(segmentsFor(SceneLightKind::Point, 0.0f), std::size_t{0});

    // A directional light is unaffected by its range, which is the field the Inspector greys out.
    CNA_STUDIO_EXPECT_EQ(segmentsFor(SceneLightKind::Directional, 0.0f), directional);
}

/**
 * @brief The Inspector and the renderer agree about which fields each light kind uses.
 *
 * Two descriptions of one fact, which `ED-300` is about: `CNA.Light`'s `range` carries an
 * `appliesWhen` that decides whether the field is editable, and `sceneLightUsesRange` decides
 * whether it changes anything. A disagreement is a field a user can set and cannot see, or one
 * they cannot set and would need — and neither is visible from either side alone.
 */
CNA_STUDIO_TEST(TheLightInspectorAndTheLightOverlayAgreeAboutWhatEachKindUses)
{
    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    const ComponentDescriptor* light = registry.find(BuiltinComponentIds::kLight);
    CNA_STUDIO_EXPECT(light != nullptr);
    if (light == nullptr) { return; }

    const PropertyDescriptor* range = light->findProperty("range");
    CNA_STUDIO_EXPECT(range != nullptr);
    if (range == nullptr) { return; }

    CNA_STUDIO_EXPECT_EQ(range->appliesWhen.property, std::string{"kind"});

    for (const SceneLightKind kind :
         {SceneLightKind::Directional, SceneLightKind::Point, SceneLightKind::Spot})
    {
        const std::string name = toString(kind);
        const bool editable = std::find(range->appliesWhen.values.begin(),
                                        range->appliesWhen.values.end(), name)
                              != range->appliesWhen.values.end();
        if (editable != sceneLightUsesRange(kind))
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "the Inspector " + std::string{editable ? "lets" : "does not let"}
                + " a " + name + " light's range be set, and the renderer "
                + (sceneLightUsesRange(kind) ? "uses it" : "ignores it") + ".");
        }

        // Every kind the enumeration offers is one the descriptor offers, and the other way
        // round: a kind the user can choose and the renderer has never heard of would light
        // nothing, and one the renderer knows and the user cannot pick is unreachable.
        const bool offered = std::find(light->findProperty("kind")->enumOptions.begin(),
                                       light->findProperty("kind")->enumOptions.end(), name)
                             != light->findProperty("kind")->enumOptions.end();
        CNA_STUDIO_EXPECT(offered);
    }

    CNA_STUDIO_EXPECT_EQ(light->findProperty("kind")->enumOptions.size(), std::size_t{3});
}

/**
 * @brief A spot light's cone is drawn, and where it is not, the effect says so rather than the
 *        scene report.
 *
 * `plan.md` STUDIO-20003. This case **replaces** one that asserted `validateScene` reports every
 * spot light as undrawable. That rule was wrong twice over: CNA's `PbrEffect` takes a punctual
 * light with inner and outer cone angles, which Studio now sends, and even as a statement about
 * `BasicEffect` it had no business being in a report that cannot know which effect the build uses.
 */
CNA_STUDIO_TEST(ASpotLightsConeIsReportedByTheEffectRatherThanByTheSceneReport)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;

    StudioEntity lamp = makeEntity(registry, "Stage Spot", 0.0f, 0.0f);
    addLight(registry, lamp, "Spot", 1.0f, 10.0f);
    scene.addEntity(std::move(lamp));

    // Nothing from the scene report: a spot light is a legal, drawable thing now.
    CNA_STUDIO_EXPECT(validateScene(scene, registry).empty());

    // The effect answers instead, and only the one that cannot draw it. `PbrEffect` is silent for
    // every kind; `BasicEffect` explains what it does instead, per kind, because a point light
    // and a spot light lose different things.
    for (const char* kind : {"Directional", "Point", "Spot"})
    {
        CNA_STUDIO_EXPECT(studioLightCapabilityIssues("PbrEffect", kind).empty());
    }

    CNA_STUDIO_EXPECT(studioLightCapabilityIssues("BasicEffect", "Directional").empty());
    CNA_STUDIO_EXPECT_EQ(studioLightCapabilityIssues("BasicEffect", "Point").size(),
                         std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(studioLightCapabilityIssues("BasicEffect", "Spot").size(),
                         std::size_t{1});

    // Each one names the feature and says what happens instead: "some of this will not draw" is a
    // line a user cannot act on.
    const std::vector<StudioMaterialCapabilityIssue> spot =
        studioLightCapabilityIssues("BasicEffect", "Spot");
    CNA_STUDIO_EXPECT(!spot.empty());
    if (!spot.empty())
    {
        CNA_STUDIO_EXPECT(!spot.front().feature.empty());
        CNA_STUDIO_EXPECT(spot.front().detail.size() > spot.front().feature.size());
    }

    // An effect this build has never heard of is silent rather than wrong, which is the rule the
    // material report already follows.
    CNA_STUDIO_EXPECT(studioLightCapabilityIssues("SomeFutureEffect", "Spot").empty());
    CNA_STUDIO_EXPECT(studioLightCapabilityIssues("", "Spot").empty());
}

/**
 * @brief A fourth directional light can never reach anything, and that is worth saying.
 *
 * `plan.md` STUDIO-20008. `computeEffectLighting` keeps the three brightest lights *where the
 * object is*, which is the right answer for a level with twenty lamps spread across it — most
 * objects are near three or fewer, and a rule that fired on that would fire on every real level.
 * A directional light is different in kind: it has no position and applies everywhere, so a fourth
 * one is dropped for every object in the scene however the level is laid out.
 */
CNA_STUDIO_TEST(MoreDirectionalLightsThanTheEffectHasSlotsIsReported)
{
    const ComponentRegistry registry = makeRegistry();

    const auto sceneWith = [&registry](int count) {
        SceneDocument scene;
        for (int i = 0; i < count; ++i)
        {
            StudioEntity light = makeEntity(registry, "Sun " + std::to_string(i), 0.0f, 0.0f);
            addLight(registry, light, "Directional", 1.0f, 0.0f);
            scene.addEntity(std::move(light));
        }
        return scene;
    };

    // Three fit, so three say nothing. The rule is about what cannot be applied, not about how
    // many lights is tasteful.
    const SceneDocument three = sceneWith(3);
    CNA_STUDIO_EXPECT(validateScene(three, registry).empty());

    // Four do not. Reported on each of them, because which three win is decided by brightness at
    // each point -- no one of them is *the* extra one, and a report naming one arbitrarily would
    // send the user to delete a light that may be the one they wanted.
    const SceneDocument four = sceneWith(4);
    const std::vector<SceneIssue> issues = validateScene(four, registry);
    CNA_STUDIO_EXPECT_EQ(countRule(issues, "more-directional-lights-than-slots"), std::size_t{4});
    CNA_STUDIO_EXPECT_EQ(countIssues(issues, SceneIssue::Severity::Error), std::size_t{0});

    // Four *point* lights are fine: they are bounded by their ranges, so which three apply is a
    // question with a different answer in every part of the level.
    SceneDocument lamps;
    for (int i = 0; i < 4; ++i)
    {
        StudioEntity light = makeEntity(registry, "Lamp " + std::to_string(i),
                                        static_cast<float>(i) * 100.0f, 0.0f);
        addLight(registry, light, "Point", 1.0f, 10.0f);
        lamps.addEntity(std::move(light));
    }
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(lamps, registry),
                                   "more-directional-lights-than-slots"), std::size_t{0});
}

/** @brief A spot light created from the editor is a spot light, and uses all three of its fields. */
CNA_STUDIO_TEST(ASpotLightCreatedFromTheEditorUsesItsPositionDirectionAndRange)
{
    const ComponentRegistry registry = makeRegistry();

    const StudioEntityArchetype* archetype = studioFindEntityArchetype("light.spot");
    CNA_STUDIO_EXPECT(archetype != nullptr);
    if (archetype == nullptr) { return; }

    const StudioEntity made = studioMakeArchetypeEntity(*archetype, registry);
    const StudioComponent* light = made.findComponent(BuiltinComponentIds::kLight);
    CNA_STUDIO_EXPECT(light != nullptr);
    if (light == nullptr) { return; }
    CNA_STUDIO_EXPECT_EQ(light->getProperty("kind").get<PropertyValue::EnumValue>().name,
                         std::string{"Spot"});

    // The only kind that uses all three. That is what makes it a kind of its own in the editor
    // even on a renderer that draws its cone as no cone at all (CNA gap G-13).
    CNA_STUDIO_EXPECT(sceneLightUsesDirection(SceneLightKind::Spot));
    CNA_STUDIO_EXPECT(sceneLightUsesPosition(SceneLightKind::Spot));
    CNA_STUDIO_EXPECT(sceneLightUsesRange(SceneLightKind::Spot));

    // And the other two each use exactly two of the three, which is the whole of how they differ.
    CNA_STUDIO_EXPECT(sceneLightUsesDirection(SceneLightKind::Directional));
    CNA_STUDIO_EXPECT(!sceneLightUsesPosition(SceneLightKind::Directional));
    CNA_STUDIO_EXPECT(!sceneLightUsesRange(SceneLightKind::Directional));

    CNA_STUDIO_EXPECT(!sceneLightUsesDirection(SceneLightKind::Point));
    CNA_STUDIO_EXPECT(sceneLightUsesPosition(SceneLightKind::Point));
    CNA_STUDIO_EXPECT(sceneLightUsesRange(SceneLightKind::Point));
}

/**
 * @brief A scene's ambient survives the path a scene with no lights takes.
 *
 * `plan.md` STUDIO-20004. The ambient has reached every lit model since ED-407 and reached no
 * *unlit* one: a scene with no lights is drawn through XNA's `EnableDefaultLighting()`, which sets
 * that rig's own ambient and overwrites whatever Studio had put there. So a user who darkened a
 * scene they had not yet put a lamp in saw nothing happen — in exactly the scene the default rig
 * exists for, which is the one somebody has just dropped a model into.
 *
 * The flag is what the renderer reads, so the flag is what this asserts: the value alone was
 * already correct and already ignored.
 */
CNA_STUDIO_TEST(TheScenesAmbientIsAppliedEvenWhenNothingLightsTheScene)
{
    const ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioEntity crate = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, crate, modelId);
    (void)scene.addEntity(std::move(crate));

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider provider = [&](const Uuid& queried) {
        return queried == modelId ? &mesh : nullptr;
    };

    // An untouched scene keeps XNA's rig exactly, ambient included. That is the promise
    // `EnableDefaultLighting` is called for -- a CNA scene and an XNA one with no lights in them
    // look the same -- and overriding unconditionally would have broken it.
    const SceneModelBatch untouched = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(untouched.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(untouched.draws[0].lighting.useDefaultLighting);
    CNA_STUDIO_EXPECT(!untouched.draws[0].lighting.ambientOverridesDefault);

    // Now darken it, with no lights in the scene at all.
    SceneEnvironment cave;
    cave.ambientColor = StudioColor{0, 0, 0, 255};
    scene.setEnvironment(cave);

    const SceneModelBatch dark = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT_EQ(dark.draws.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(dark.draws[0].lighting.useDefaultLighting);
    CNA_STUDIO_EXPECT(dark.draws[0].lighting.ambientOverridesDefault);
    CNA_STUDIO_EXPECT_EQ(dark.draws[0].lighting.ambientColor.x, 0.0f);

    // A brighter-than-default ambient is just as much a statement as a darker one: the rule is
    // "the user said something", not "the user said something dark".
    SceneEnvironment noon;
    noon.ambientColor = StudioColor{200, 200, 190, 255};
    scene.setEnvironment(noon);

    const SceneModelBatch bright = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT(bright.draws[0].lighting.ambientOverridesDefault);
    CNA_STUDIO_EXPECT(bright.draws[0].lighting.ambientColor.x > 0.7f);

    // And putting a light in makes the flag beside the point: the lit path applies the ambient
    // unconditionally, which it always did.
    const StudioEntityArchetype* archetype = studioFindEntityArchetype("light.directional");
    CNA_STUDIO_EXPECT(archetype != nullptr);
    if (archetype == nullptr) { return; }
    (void)scene.addEntity(studioMakeArchetypeEntity(*archetype, registry));

    const SceneModelBatch lit = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT(!lit.draws[0].lighting.useDefaultLighting);
    CNA_STUDIO_EXPECT(lit.draws[0].lighting.ambientColor.x > 0.7f);

    // Setting it back to the default puts the scene back under XNA's own rig, rather than pinning
    // it to a value that merely equals the default. The two are the same picture and only one of
    // them follows the framework if the framework's rig ever changes.
    scene.setEnvironment(SceneEnvironment{});
    SceneDocument empty;
    StudioEntity lone = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, lone, modelId);
    (void)empty.addEntity(std::move(lone));
    empty.setEnvironment(SceneEnvironment{});
    const SceneModelBatch restored = buildSceneModelBatch(empty, camera, provider);
    CNA_STUDIO_EXPECT(!restored.draws[0].lighting.ambientOverridesDefault);
}

/**
 * @brief A scene lit only by lamps is told why its casters cast nothing.
 *
 * `plan.md` STUDIO-20006. This test used to assert that the editor drew no shadows at all, which
 * was true until the shadow pass landed. What replaced it is the limitation that survived: the
 * pass renders from a *directional* light, because `ShadowMap` fits an orthographic volume to the
 * scene. A point light's shadow is a cube and a spot's is a frustum; CNA ships `CubeShadowMap` and
 * `SpotShadowMap` for both and Studio does not drive them yet.
 *
 * Reported once for the scene rather than once per model, because the flags default to *on*: a
 * per-entity rule would fire on every model in every project forever, which is the shape of a rule
 * people configure their way out of and then stop reading.
 */
CNA_STUDIO_TEST(AShadowWantedFromALampIsReportedBecauseThePassNeedsASun)
{
    const ComponentRegistry registry = makeRegistry();
    static const char* const kRule = "shadows-need-a-directional-light";

    const auto sceneWith = [&registry](const char* lightKind, bool model, bool casts) {
        SceneDocument scene;
        if (lightKind != nullptr)
        {
            StudioEntity lamp = makeEntity(registry, "Lamp", 0.0f, 0.0f);
            addLight(registry, lamp, lightKind, 1.0f, 10.0f);
            scene.addEntity(std::move(lamp));
        }
        if (model)
        {
            StudioEntity crate = makeEntity(registry, "Crate", 0.0f, 0.0f);
            addModelRenderer(registry, crate, Uuid::generate());
            crate.findComponent(BuiltinComponentIds::kModelRenderer)
                ->setProperty("castShadows", PropertyValue{casts});
            scene.addEntity(std::move(crate));
        }
        return scene;
    };

    // A room lit by a lamp, with a model that says it casts: the user has asked for a shadow and
    // is not getting one, which is the whole of what this rule is for.
    const SceneDocument lamplit = sceneWith("Point", true, true);
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(lamplit, registry), kRule), std::size_t{1});

    // A spot light is the same answer for a different reason -- its shadow is a frustum rather
    // than a cube -- and the rule does not need to distinguish them to be right.
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(sceneWith("Spot", true, true), registry), kRule),
                         std::size_t{1});

    // Once, not once per model: the fact is about the light rig.
    SceneDocument crowded = sceneWith("Point", true, true);
    for (int i = 0; i < 4; ++i)
    {
        StudioEntity extra = makeEntity(registry, "Crate " + std::to_string(i), 0.0f, 0.0f);
        addModelRenderer(registry, extra, Uuid::generate());
        crowded.addEntity(std::move(extra));
    }
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(crowded, registry), kRule), std::size_t{1});

    // **And silent the moment a sun is added**, which is the assertion that would have failed
    // before the shadow pass existed and is the reason this rule could be narrowed rather than
    // deleted: the editor now draws the shadow it is describing.
    SceneDocument withSun = sceneWith("Point", true, true);
    StudioEntity sun = makeEntity(registry, "Sun", 0.0f, 0.0f);
    addLight(registry, sun, "Directional", 1.0f, 0.0f);
    withSun.addEntity(std::move(sun));
    CNA_STUDIO_EXPECT_EQ(countRule(validateScene(withSun, registry), kRule), std::size_t{0});

    // And only where it costs something. A scene with no light casts no shadow on any renderer,
    // a scene with no model has nothing to cast one, and a model that says it does not cast is a
    // user who has already answered the question.
    for (const SceneDocument& quiet : {sceneWith(nullptr, true, true), sceneWith("Point", false, true),
                                       sceneWith("Point", true, false)})
    {
        CNA_STUDIO_EXPECT_EQ(countRule(validateScene(quiet, registry), kRule), std::size_t{0});
    }

    // A warning rather than an error: the scene is legal, and the flags are carried through to a
    // game that may well drive CNA's cube and spot maps itself.
    CNA_STUDIO_EXPECT_EQ(countIssues(validateScene(lamplit, registry),
                                     SceneIssue::Severity::Error), std::size_t{0});
}

// ------------------------------------------------------------------------------------------------
// The shadow plan (STUDIO-20006)
// ------------------------------------------------------------------------------------------------

/**
 * @brief Cast Shadows and Receive Shadows finally decide something.
 *
 * `plan.md` STUDIO-20006. Both have been editable since Phase 1 and read by nothing: a user has
 * been able to turn a model's shadow off since the component existed and no picture has ever
 * changed. They now reach the draw, and `planSceneShadows` turns them into the two decisions a
 * shadow pass needs — what goes in the map, and what it is fitted to.
 */
CNA_STUDIO_TEST(TheShadowPlanFollowsTheCastAndReceiveFlags)
{
    const ComponentRegistry registry = makeRegistry();
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider provider = [&](const Uuid& queried) {
        return queried == modelId ? &mesh : nullptr;
    };

    SceneDocument scene;
    StudioEntity sun = makeEntity(registry, "Sun", 0.0f, 0.0f);
    addLight(registry, sun, "Directional", 1.0f, 0.0f);
    scene.addEntity(std::move(sun));

    StudioEntity crate = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, crate, modelId);
    const Uuid crateId = scene.addEntity(std::move(crate));

    // Read off the batch rather than by calling `planSceneShadows` beside it, so this covers the
    // wiring as well as the arithmetic: the viewport is handed a batch and nothing else, and a
    // plan that the builder forgot to attach is a scene that draws no shadows however right the
    // function computing it is.
    const auto plan = [&] { return buildSceneModelBatch(scene, camera, provider).shadows; };

    // The default is both on, which is what a scene written before this existed was promised.
    const SceneShadowPlan both = plan();
    CNA_STUDIO_EXPECT(both.enabled);
    CNA_STUDIO_EXPECT_EQ(both.casters, std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(both.receivers, std::size_t{1});
    CNA_STUDIO_EXPECT(!both.casterBounds.isEmpty());

    // Unticking Cast Shadows takes the model out of the map, and with nothing left to put in it
    // the pass is not worth running at all.
    StudioComponent* renderer =
        scene.findEntityForEdit(crateId)->findComponent(BuiltinComponentIds::kModelRenderer);
    renderer->setProperty("castShadows", PropertyValue{false});

    const SceneShadowPlan noCasters = plan();
    CNA_STUDIO_EXPECT(!noCasters.enabled);
    CNA_STUDIO_EXPECT_EQ(noCasters.casters, std::size_t{0});

    // It still *receives*, which is a different question: a model that is shadowed by others and
    // casts none of its own is an ordinary thing to want.
    CNA_STUDIO_EXPECT_EQ(noCasters.receivers, std::size_t{1});

    // And unticking Receive Shadows leaves the map being generated: what goes into it is not
    // decided by who reads it, and a user who unticks the last receiver should not silently
    // change what the casters do.
    renderer->setProperty("castShadows", PropertyValue{true});
    renderer->setProperty("receiveShadows", PropertyValue{false});

    const SceneShadowPlan noReceivers = plan();
    CNA_STUDIO_EXPECT(noReceivers.enabled);
    CNA_STUDIO_EXPECT_EQ(noReceivers.casters, std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(noReceivers.receivers, std::size_t{0});
}

/** @brief The map is generated from the brightest directional light, and from nothing else. */
CNA_STUDIO_TEST(TheShadowPlanPicksTheBrightestDirectionalLightAndIgnoresTheOthers)
{
    const ComponentRegistry registry = makeRegistry();
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider provider = [&](const Uuid& queried) {
        return queried == modelId ? &mesh : nullptr;
    };

    SceneDocument scene;
    StudioEntity crate = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, crate, modelId);
    scene.addEntity(std::move(crate));

    const auto plan = [&] { return buildSceneModelBatch(scene, camera, provider).shadows; };

    // A point light has its own shadow in CNA's punctual extension -- a cube of six faces -- and
    // is not what an orthographic map fitted to the scene describes. So it generates nothing here.
    StudioEntity lamp = makeEntity(registry, "Lamp", 0.0f, 0.0f);
    addLight(registry, lamp, "Point", 5.0f, 100.0f);
    scene.addEntity(std::move(lamp));
    CNA_STUDIO_EXPECT(!plan().enabled);

    // A dim sun does, and its direction is the one the map is rendered along.
    StudioEntity dim = makeEntity(registry, "Fill", 0.0f, 0.0f);
    addLight(registry, dim, "Directional", 0.2f, 0.0f, StudioColor{255, 0, 0, 255});
    scene.addEntity(std::move(dim));

    const SceneShadowPlan withFill = plan();
    CNA_STUDIO_EXPECT(withFill.enabled);
    CNA_STUDIO_EXPECT(withFill.color.x > withFill.color.y);

    // And a brighter one takes over, by the same rule the three effect slots use: brightness, not
    // document order, because document order is not something a user arranges deliberately.
    StudioEntity bright = makeEntity(registry, "Sun", 0.0f, 0.0f);
    addLight(registry, bright, "Directional", 1.0f, 0.0f, StudioColor{0, 255, 0, 255});
    scene.addEntity(std::move(bright));

    const SceneShadowPlan withSun = plan();
    CNA_STUDIO_EXPECT(withSun.enabled);
    CNA_STUDIO_EXPECT(withSun.color.y > withSun.color.x);

    // The direction is unit length, because the pass builds a view matrix from it.
    const float len = std::sqrt(withSun.direction.x * withSun.direction.x
                                + withSun.direction.y * withSun.direction.y
                                + withSun.direction.z * withSun.direction.z);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(len, 1.0f, 0.001f));

    // A scene with no light at all plans nothing, which is the 2D case and most of the 3D ones
    // before somebody adds a sun.
    SceneDocument bare;
    StudioEntity lone = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, lone, modelId);
    bare.addEntity(std::move(lone));
    CNA_STUDIO_EXPECT(!buildSceneModelBatch(bare, camera, provider).shadows.enabled);
}

// ------------------------------------------------------------------------------------------------
// The sky (STUDIO-20005)
// ------------------------------------------------------------------------------------------------

/**
 * @brief Five ways for a sky to be missing, and the editor says which one.
 *
 * `plan.md` STUDIO-20005. A scene names an environment map, the environment map names a panorama,
 * the panorama is a texture -- three references, and every break in the chain looks identical in
 * the viewport. A user who has set a sky and sees nothing deserves to be told which link is gone.
 */
CNA_STUDIO_TEST(EveryBrokenLinkInTheSkysChainIsNamedRatherThanReportedAsNoSky)
{
    SceneEnvironment environment;
    const Uuid map = Uuid::generate();

    // A scene that names none, which is most scenes and is not a fault.
    CNA_STUDIO_EXPECT(planSceneSky(environment, {}).problem == SceneSkyProblem::NoEnvironmentMap);
    CNA_STUDIO_EXPECT(describeSceneSkyProblem(SceneSkyProblem::NoEnvironmentMap).empty());

    environment.environmentMap = map;

    // No provider at all is a caller with no database -- a headless panel -- and answering
    // "found nothing" is what that honestly is rather than a crash or a false resolution.
    CNA_STUDIO_EXPECT(planSceneSky(environment, {}).problem
                      == SceneSkyProblem::EnvironmentMapMissing);

    const auto answering = [](SceneSkySource source) {
        return [source](const Uuid&) { return source; };
    };

    SceneSkySource source;
    CNA_STUDIO_EXPECT(planSceneSky(environment, answering(source)).problem
                      == SceneSkyProblem::EnvironmentMapMissing);

    source.found = true;
    CNA_STUDIO_EXPECT(planSceneSky(environment, answering(source)).problem
                      == SceneSkyProblem::EnvironmentMapUnreadable);

    source.readable = true;
    CNA_STUDIO_EXPECT(planSceneSky(environment, answering(source)).problem
                      == SceneSkyProblem::NoPanorama);

    source.panorama = Uuid::generate();
    CNA_STUDIO_EXPECT(planSceneSky(environment, answering(source)).problem
                      == SceneSkyProblem::PanoramaMissing);

    source.panoramaIsTexture = true;
    const SceneSkyPlan resolved = planSceneSky(environment, answering(source));
    CNA_STUDIO_EXPECT(resolved.problem == SceneSkyProblem::None);
    CNA_STUDIO_EXPECT(resolved.isUsable());
    CNA_STUDIO_EXPECT(resolved.panorama == source.panorama);
    CNA_STUDIO_EXPECT(resolved.environmentMap == map);

    // Every problem but the first has something to say, because every one of them is a state the
    // user did not choose. The first is silent on purpose: a sentence shown by every 2D project
    // is a sentence nobody reads.
    for (const SceneSkyProblem problem :
         {SceneSkyProblem::EnvironmentMapMissing, SceneSkyProblem::EnvironmentMapUnreadable,
          SceneSkyProblem::NoPanorama, SceneSkyProblem::PanoramaMissing})
    {
        if (describeSceneSkyProblem(problem).empty())
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"SceneSkyProblem::"} + toString(problem)
                + " has no sentence, so the viewport shows nothing and says nothing.");
        }
    }
}

/**
 * @brief Drawing and lighting are two answers, and switching both off is not a problem.
 *
 * CNA separates them -- `Skybox::draw` puts the cube on screen and `setImageBasedLightEXT` makes it
 * light things -- and each is worth wanting alone. What a *problem* means is that the editor cannot
 * do what the scene asks, so a user who has deliberately unticked both must not be warned at.
 */
CNA_STUDIO_TEST(ASkyThatIsDeliberatelySwitchedOffIsNotAProblem)
{
    SceneEnvironment environment;
    environment.environmentMap = Uuid::generate();
    environment.environmentIntensity = 2.5f;
    environment.environmentYaw = 90.0f;

    SceneSkySource source;
    source.found = true;
    source.readable = true;
    source.panorama = Uuid::generate();
    source.panoramaIsTexture = true;
    const auto provider = [source](const Uuid&) { return source; };

    const SceneSkyPlan both = planSceneSky(environment, provider);
    CNA_STUDIO_EXPECT(both.draws);
    CNA_STUDIO_EXPECT(both.lights);

    // Degrees in the document, radians in the plan -- the same split `CNA.Light`'s cone angles
    // use, so every trigonometric consumer gets what it wants and the user types what they mean.
    CNA_STUDIO_EXPECT(cameraNearlyEqual(both.yaw, 1.5707963f, 0.0001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(both.intensity, 2.5f, 0.0001f));

    environment.showSky = false;
    const SceneSkyPlan litOnly = planSceneSky(environment, provider);
    CNA_STUDIO_EXPECT(!litOnly.draws);
    CNA_STUDIO_EXPECT(litOnly.lights);
    CNA_STUDIO_EXPECT(litOnly.problem == SceneSkyProblem::None);

    environment.lightFromEnvironment = false;
    const SceneSkyPlan neither = planSceneSky(environment, provider);
    CNA_STUDIO_EXPECT(!neither.draws);
    CNA_STUDIO_EXPECT(!neither.lights);
    CNA_STUDIO_EXPECT(neither.problem == SceneSkyProblem::None);

    // A negative intensity is nonsense rather than a dark sky, and is clamped rather than passed
    // to a shader that would multiply a colour by it.
    environment.environmentIntensity = -3.0f;
    CNA_STUDIO_EXPECT(cameraNearlyEqual(planSceneSky(environment, provider).intensity, 0.0f,
                                        0.0001f));
}

/** @brief The batch carries the sky, so the viewport is handed one answer rather than asked for it. */
CNA_STUDIO_TEST(TheModelBatchCarriesTheScenesSky)
{
    const ComponentRegistry registry = makeRegistry();
    const MeshData mesh = makeTinyMesh();
    const Uuid modelId = Uuid::generate();
    const Uuid map = Uuid::generate();

    StudioCamera3D camera;
    camera.setViewportSize(StudioVector2{800.0f, 600.0f});
    const MeshProvider provider = [&](const Uuid& queried) {
        return queried == modelId ? &mesh : nullptr;
    };

    SceneDocument scene;
    StudioEntity crate = makeEntity(registry, "Crate", 0.0f, 0.0f);
    addModelRenderer(registry, crate, modelId);
    scene.addEntity(std::move(crate));

    SceneEnvironment environment;
    environment.environmentMap = map;
    scene.setEnvironment(environment);

    SceneSkySource source;
    source.found = true;
    source.readable = true;
    source.panorama = Uuid::generate();
    source.panoramaIsTexture = true;

    const SceneModelBatch lit = buildSceneModelBatch(
        scene, camera, provider, {}, {}, StudioDebugView::None,
        [source](const Uuid&) { return source; });
    CNA_STUDIO_EXPECT(lit.sky.isUsable());
    CNA_STUDIO_EXPECT(lit.sky.panorama == source.panorama);

    // And a batch built with no provider still reports what the scene asks for, rather than
    // silently resolving: a host that forgot to pass one must not look like a scene with no sky.
    const SceneModelBatch blind = buildSceneModelBatch(scene, camera, provider);
    CNA_STUDIO_EXPECT(!blind.sky.isUsable());
    CNA_STUDIO_EXPECT(blind.sky.problem == SceneSkyProblem::EnvironmentMapMissing);
    CNA_STUDIO_EXPECT(blind.sky.environmentMap == map);
}

/** @brief The sky survives a save and a load, and a scene without one still writes nothing. */
CNA_STUDIO_TEST(TheScenesSkyRoundTripsAndAnAbsentOneWritesNothing)
{
    const ComponentRegistry registry = makeRegistry();

    SceneDocument plain;
    CNA_STUDIO_EXPECT(plain.getEnvironment().isDefault());

    // A scene that never touched its environment must come back byte for byte, which is what
    // keeps opening every existing scene once from rewriting every existing scene once.
    CNA_STUDIO_EXPECT(!plain.toJson().contains("environment"));

    SceneEnvironment environment;
    environment.environmentMap = Uuid::generate();
    environment.environmentIntensity = 0.5f;
    environment.environmentYaw = 145.0f;
    environment.showSky = false;
    environment.lightFromEnvironment = true;

    SceneDocument sky;
    sky.setEnvironment(environment);
    CNA_STUDIO_EXPECT(!sky.getEnvironment().isDefault());

    SceneDocument reloaded;
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(sky.toJson(), registry).succeeded);

    const SceneEnvironment& read = reloaded.getEnvironment();
    CNA_STUDIO_EXPECT(read.environmentMap == environment.environmentMap);
    CNA_STUDIO_EXPECT(cameraNearlyEqual(read.environmentIntensity, 0.5f, 0.0001f));
    CNA_STUDIO_EXPECT(cameraNearlyEqual(read.environmentYaw, 145.0f, 0.0001f));
    CNA_STUDIO_EXPECT(!read.showSky);
    CNA_STUDIO_EXPECT(read.lightFromEnvironment);
}

/**
 * @brief Drawing a sky and being lit by one are different asks, and only the second needs PbrEffect.
 *
 * `plan.md` STUDIO-20005, and the one place in this family where the two effects are *not*
 * interchangeable. `setImageBasedLightEXT` is declared on `PbrEffect` and `SkinnedPbrEffect` and on
 * nothing else -- it is not on `IShadowReceiverEXT`, where the punctual light and the shadow map
 * live, so the seam that made those two work on both effects does not exist here. This build draws
 * through `BasicEffect` (`kPreferPbrEffect` is false, CNA gap `G-05`), which means a sky set up
 * today appears and lights nothing.
 */
CNA_STUDIO_TEST(ASkyOnABasicEffectBuildIsDrawnAndSaysItLightsNothing)
{
    // A sky that is only drawn asks for nothing `BasicEffect` cannot give.
    CNA_STUDIO_EXPECT(studioEnvironmentCapabilityIssues("BasicEffect", false).empty());
    CNA_STUDIO_EXPECT(studioEnvironmentCapabilityIssues("PbrEffect", false).empty());

    // A sky asked to light the scene does, and only on the effect that cannot.
    CNA_STUDIO_EXPECT(studioEnvironmentCapabilityIssues("PbrEffect", true).empty());

    const std::vector<StudioMaterialCapabilityIssue> issues =
        studioEnvironmentCapabilityIssues("BasicEffect", true);
    CNA_STUDIO_EXPECT_EQ(issues.size(), std::size_t{1});
    if (!issues.empty())
    {
        // The sentence has to say both halves -- that the sky *is* drawn, and that it lights
        // nothing -- because a user reading only the second would go looking for a sky that is
        // already on screen.
        CNA_STUDIO_EXPECT(issues[0].detail.find("drawn") != std::string::npos);
        CNA_STUDIO_EXPECT(issues[0].detail.find("lights nothing") != std::string::npos);
        CNA_STUDIO_EXPECT(issues[0].detail.find("PbrEffect") != std::string::npos);
    }

    // An effect nobody recognises is silent, for the reason the other two capability functions
    // are: a build drawing through something this editor has never heard of is not a build this
    // editor can make claims about.
    CNA_STUDIO_EXPECT(studioEnvironmentCapabilityIssues("SomeFutureEffect", true).empty());
}

/**
 * @brief **A value the file's shape cannot hold is replaced, and the replacement is reported.**
 *
 * `plan.md` STUDIO-31011. Every reader in Studio falls back rather than failing, and that is right:
 * a scene with one bad field should open so the user can fix it — refusing it would be
 * `STUDIO-31008`'s complaint about a tool you cannot use to repair a file. What is not right is
 * doing it quietly. The fallback is written back on the next save, so a value the user put in their
 * file is replaced by one they never chose, and the only evidence is a diff they were not looking
 * for.
 *
 * The message names the entity and the component as well as the property, because "position holds
 * text" in a scene with four hundred entities names the one thing the user already knows.
 */
CNA_STUDIO_TEST(APropertyTheFileCannotHoldIsReplacedAndSaidSo)
{
    const ComponentRegistry registry = makeRegistry();

    JsonValue transform = JsonValue::makeObject();
    transform.set("position", JsonValue{"over there"});

    JsonValue components = JsonValue::makeObject();
    components.set(std::string{BuiltinComponentIds::kTransform}, std::move(transform));

    JsonValue entity = JsonValue::makeObject();
    entity.set("id", JsonValue{Uuid::generate().toString()});
    entity.set("name", JsonValue{"Player"});
    entity.set("components", std::move(components));

    JsonValue entities = JsonValue::makeArray();
    entities.append(std::move(entity));

    JsonValue json = JsonValue::makeObject();
    json.set("formatVersion", JsonValue{SceneDocument::kFormatVersion});
    json.set("sceneId", JsonValue{Uuid::generate().toString()});
    json.set("name", JsonValue{"Level01"});
    json.set("entities", std::move(entities));

    SceneDocument scene;
    const SceneLoadResult result = scene.loadFromJson(json, registry);

    // It opens, because a scene you cannot open is a scene you cannot repair.
    CNA_STUDIO_EXPECT(result.succeeded);
    CNA_STUDIO_EXPECT_EQ(scene.getEntityCount(), std::size_t{1});

    bool said = false;
    for (const std::string& warning : result.warnings)
    {
        if (warning.find("Player") != std::string::npos
            && warning.find("position") != std::string::npos
            && warning.find("text") != std::string::npos)
        {
            said = true;
        }
    }
    CNA_STUDIO_EXPECT(said);

    // And it says the consequence, not only the fact: the next save writes the replacement, which
    // is the moment the user's value is actually gone.
    bool warnedAboutSaving = false;
    for (const std::string& warning : result.warnings)
    {
        if (warning.find("saving") != std::string::npos) { warnedAboutSaving = true; }
    }
    CNA_STUDIO_EXPECT(warnedAboutSaving);
}

/**
 * @brief And the nearest legitimate scene is not reported, which is the half that makes it usable.
 *
 * A warning that fires on ordinary documents is a warning users learn to scroll past, and then the
 * one that mattered scrolls past with it. Four shapes that look like faults and are not: an absent
 * property (which means "use the default" by contract), a longer array than a Vector3 needs (the
 * extra is ignored and nothing the type can hold was dropped), an asset reference naming an id
 * nothing has (well-formed text; *scene validation* reports the missing reference, with a better
 * message than a shape check could give), and a well-formed value.
 */
CNA_STUDIO_TEST(AnOrdinarySceneProducesNoReplacementWarnings)
{
    const ComponentRegistry registry = makeRegistry();

    const PropertyDescriptor* position =
        registry.find(BuiltinComponentIds::kTransform)->findProperty("position");
    CNA_STUDIO_EXPECT(position != nullptr);
    if (position == nullptr) { return; }

    // Absent: the default, by contract.
    CNA_STUDIO_EXPECT(studioJsonMatchesPropertyType(JsonValue{}, *position));

    // Exactly three, and more than three.
    JsonValue three = JsonValue::makeArray();
    three.append(JsonValue{1.0});
    three.append(JsonValue{2.0});
    three.append(JsonValue{3.0});
    CNA_STUDIO_EXPECT(studioJsonMatchesPropertyType(three, *position));

    JsonValue four = three;
    four.append(JsonValue{4.0});
    CNA_STUDIO_EXPECT(studioJsonMatchesPropertyType(four, *position));

    // Two is a loss, because the reader fills the third from a default and the document said
    // something shorter.
    JsonValue two = JsonValue::makeArray();
    two.append(JsonValue{1.0});
    two.append(JsonValue{2.0});
    CNA_STUDIO_EXPECT(!studioJsonMatchesPropertyType(two, *position));

    // A reference to nothing is well-formed text. The *missing* reference is scene validation's to
    // report, and it says something this could not.
    const PropertyDescriptor* texture =
        registry.find(BuiltinComponentIds::kSpriteRenderer)->findProperty("texture");
    CNA_STUDIO_EXPECT(texture != nullptr);
    if (texture != nullptr)
    {
        CNA_STUDIO_EXPECT(studioJsonMatchesPropertyType(JsonValue{Uuid::generate().toString()},
                                                        *texture));
        CNA_STUDIO_EXPECT(studioJsonMatchesPropertyType(JsonValue{""}, *texture));
        CNA_STUDIO_EXPECT(!studioJsonMatchesPropertyType(JsonValue{7.0}, *texture));
    }

    // A scene written by Studio itself reports nothing, which is the case that must stay quiet.
    SceneDocument written;
    written.addEntity(makeEntity(registry, "Player", 1.0f, 2.0f));

    SceneDocument reloaded;
    const SceneLoadResult result = reloaded.loadFromJson(written.toJson(), registry);
    CNA_STUDIO_EXPECT(result.succeeded);
    for (const std::string& warning : result.warnings)
    {
        CNA_STUDIO_EXPECT(warning.find("the default was used") == std::string::npos);
    }
}
