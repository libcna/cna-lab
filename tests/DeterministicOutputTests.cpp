// SPDX-License-Identifier: MS-PL
/**
 * @file DeterministicOutputTests.cpp
 * @brief Saving the same document twice produces the same bytes (`plan.md` STUDIO-02037).
 *
 * These files live in git. A save that reordered its members, or leaked a timestamp, or spelled a
 * float differently on a second run, would put a diff in front of a user who changed nothing — and
 * a project whose every save is noisy is one where a *real* change cannot be found by review. The
 * merge is worse than the review: two people who each opened and saved a scene produce two
 * conflicting rewrites of a file neither of them edited.
 *
 * Two properties, and only the second is interesting:
 *
 * 1. **Writing the same document twice is identical.** A low bar, and the one a writer passes by
 *    accident.
 * 2. **A round trip is identical.** Write, read back, write again. This is the one that catches an
 *    unordered container: a `std::unordered_map` behind object members produces one order in the
 *    process that built the document and another in the process that loaded it, and the difference
 *    shows up as every member moving on a colleague's machine and not on yours.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/Assets/EnvironmentMapDocument.hpp"
#include "CNA/Studio/Assets/MaterialDocument.hpp"
#include "CNA/Studio/Core/Json.hpp"
#include "CNA/Studio/Project/Project.hpp"
#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/PrefabDocument.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    /** @brief A scene with enough in it that an ordering bug has somewhere to show. */
    SceneDocument makeBusyScene(const ComponentRegistry& registry)
    {
        SceneDocument scene;
        scene.setName("Level01");

        // Fixed ids rather than generated ones: a case about *byte* equality cannot have a random
        // number in the bytes, and a `Uuid::generate()` here would make the failure look
        // intermittent when it is not.
        static const char* const kIds[] = {
            "0cf45f27-2ecd-44a6-8c45-cd8d2122179f", "5c1d8e34-0f2b-4a77-9c61-2b8e5f0a41d3",
            "11111111-2222-3333-4444-555555555555", "22222222-3333-4444-5555-666666666666",
            "33333333-4444-5555-6666-777777777777"};

        for (std::size_t i = 0; i < 5; ++i)
        {
            StudioEntity entity{Uuid::parse(kIds[i]), "Entity" + std::to_string(i)};

            StudioComponent transform{BuiltinComponentIds::kTransform};
            transform.applyDefaults(*registry.find(BuiltinComponentIds::kTransform));
            transform.setProperty("position",
                                  PropertyValue{StudioVector3{static_cast<float>(i) * 0.6f,
                                                              -1.25f, 100.0f}});
            entity.addComponent(std::move(transform));

            StudioComponent sprite{BuiltinComponentIds::kSpriteRenderer};
            sprite.applyDefaults(*registry.find(BuiltinComponentIds::kSpriteRenderer));
            entity.addComponent(std::move(sprite));

            // Editor-only state, which travels in its own section and is a second place an
            // ordering bug could live.
            entity.setStudioState("locked", PropertyValue{i % 2 == 0});
            entity.setStudioState("colour", PropertyValue{std::string{"#ff8800"}});

            scene.addEntity(std::move(entity));
        }

        // A parent link, so the order the entities are written in is not the order they were made.
        scene.addEntity([&registry] {
            StudioEntity child{Uuid::parse("44444444-5555-6666-7777-888888888888"), "Child"};
            child.setParentId(Uuid::parse("0cf45f27-2ecd-44a6-8c45-cd8d2122179f"));
            StudioComponent transform{BuiltinComponentIds::kTransform};
            transform.applyDefaults(*registry.find(BuiltinComponentIds::kTransform));
            child.addComponent(std::move(transform));
            return child;
        }());

        return scene;
    }

    /** @brief Whether @p text holds this year's number, which is how a timestamp usually leaks. */
    [[nodiscard]] bool mentionsThisYear(const std::string& text)
    {
        const std::time_t now = std::time(nullptr);
        const std::tm* parts = std::gmtime(&now);
        if (parts == nullptr) { return false; }
        return text.find(std::to_string(1900 + parts->tm_year)) != std::string::npos;
    }
}

/**
 * @brief A scene written, read back and written again is the same bytes both times.
 *
 * The round trip is the part that matters. Writing twice from one in-memory document passes even
 * when object members are held in a hash map, because the map is the same map; loading it into a
 * *fresh* document is what asks whether the order came from the file or from the allocator.
 */
CNA_STUDIO_TEST(ASceneSurvivesARoundTripByteForByte)
{
    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    SceneDocument scene = makeBusyScene(registry);
    const std::string first = Json::write(scene.toJson(), true);

    // The same document, written again.
    CNA_STUDIO_EXPECT_EQ(Json::write(scene.toJson(), true), first);

    // And after a round trip, which is the real question.
    SceneDocument reloaded;
    const JsonParseResult parsed = Json::parse(first);
    CNA_STUDIO_EXPECT(parsed.succeeded);
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(parsed.value, registry).succeeded);

    const std::string second = Json::write(reloaded.toJson(), true);
    CNA_STUDIO_EXPECT_EQ(second, first);

    // Twice more, because a first round trip can normalise something and a second reveal that the
    // normalisation was not idempotent.
    SceneDocument third;
    CNA_STUDIO_EXPECT(third.loadFromJson(Json::parse(second).value, registry).succeeded);
    CNA_STUDIO_EXPECT_EQ(Json::write(third.toJson(), true), first);

    // No timestamp leaked into it. A save that recorded *when* it happened would make every file
    // differ from itself on the next save, which is the loudest form of this defect.
    CNA_STUDIO_EXPECT(!mentionsThisYear(first));

    // And the numbers are the numbers that were authored: `0.6`, not `0.600000024`
    // (`STUDIO-02042`). A writer that spelled a float by its bit pattern would be deterministic and
    // still unreadable, so this is checked here as well as there.
    CNA_STUDIO_EXPECT(first.find("0.600000") == std::string::npos);
}

/** @brief The same for every other authored document, since each has its own writer. */
CNA_STUDIO_TEST(EveryAuthoredDocumentSurvivesARoundTripByteForByte)
{
    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    // --- .cnaprefab -----------------------------------------------------------------------------
    {
        PrefabDocument prefab;
        prefab.setName("Enemy");
        prefab.setPrefabId(Uuid::parse("3c1e9a44-2ecd-44a6-8c45-cd8d2122179f"));

        // A root and two children *parented to it*. A prefab is one reusable subtree by
        // definition of the format, so three parentless entities would be a malformed document --
        // which the loader repairs by adopting the strays, and says so. A determinism case has to
        // start from a document the format allows, or it measures the repair rather than the
        // writer. (Found by this case failing, which is the right way round.)
        std::vector<StudioEntity> entities;
        const std::string rootId = "55555550-1111-2222-3333-444444444444";
        for (int i = 0; i < 3; ++i)
        {
            StudioEntity entity{
                Uuid::parse(std::string{"5555555"} + std::to_string(i)
                            + "-1111-2222-3333-444444444444"),
                "Part" + std::to_string(i)};
            if (i > 0) { entity.setParentId(Uuid::parse(rootId)); }
            StudioComponent transform{BuiltinComponentIds::kTransform};
            transform.applyDefaults(*registry.find(BuiltinComponentIds::kTransform));
            entity.addComponent(std::move(transform));
            entities.push_back(std::move(entity));
        }
        prefab.setEntities(std::move(entities));

        const std::string first = Json::write(prefab.toJson(), true);
        PrefabDocument reloaded;
        CNA_STUDIO_EXPECT(reloaded.loadFromJson(Json::parse(first).value, registry).succeeded);
        CNA_STUDIO_EXPECT_EQ(Json::write(reloaded.toJson(), true), first);
        CNA_STUDIO_EXPECT(!mentionsThisYear(first));
    }

    // --- .cnaproject ----------------------------------------------------------------------------
    {
        Project project;
        const std::string first = Json::write(project.toJson(), true);

        Project reloaded;
        CNA_STUDIO_EXPECT(reloaded.loadFromJson(Json::parse(first).value).succeeded);
        CNA_STUDIO_EXPECT_EQ(Json::write(reloaded.toJson(), true), first);
        CNA_STUDIO_EXPECT(!mentionsThisYear(first));
    }

    // --- .cnamaterial ---------------------------------------------------------------------------
    {
        MaterialDocument material;
        material.name = "Brick";
        material.roughness = 0.6f;
        material.metallic = 0.25f;
        material.diffuseTexture = Uuid::parse("66666666-1111-2222-3333-444444444444");
        material.overridden.insert("roughness");
        material.overridden.insert("metallic");
        material.overridden.insert("diffuseTexture");

        const std::string first = Json::write(material.toJson(), true);

        MaterialDocument reloaded;
        CNA_STUDIO_EXPECT(reloaded.loadFromJson(Json::parse(first).value));
        CNA_STUDIO_EXPECT_EQ(Json::write(reloaded.toJson(), true), first);
        CNA_STUDIO_EXPECT(!mentionsThisYear(first));
    }

    // --- .cnaenv --------------------------------------------------------------------------------
    {
        EnvironmentMapDocument environment;
        environment.name = "Overcast";
        environment.panorama = Uuid::parse("77777777-1111-2222-3333-444444444444");

        const std::string first = Json::write(environment.toJson(), true);

        EnvironmentMapDocument reloaded;
        CNA_STUDIO_EXPECT(reloaded.loadFromJson(Json::parse(first).value));
        CNA_STUDIO_EXPECT_EQ(Json::write(reloaded.toJson(), true), first);
        CNA_STUDIO_EXPECT(!mentionsThisYear(first));
    }
}

/**
 * @brief The override set is written in a stable order, whatever order it was filled in.
 *
 * `MaterialDocument::overridden` is the one place where the *set* of keys decides what is written
 * (`STUDIO-19005`), and a set iterated in insertion order would make two materials with the same
 * values produce different files depending on which parameter the user touched first.
 */
CNA_STUDIO_TEST(AMaterialsOverrideSetIsWrittenInAStableOrder)
{
    MaterialDocument forwards;
    forwards.name = "Brick";
    for (const char* key : {"alpha", "metallic", "roughness", "diffuseColor"})
    {
        forwards.overridden.insert(key);
    }

    MaterialDocument backwards;
    backwards.name = "Brick";
    for (const char* key : {"diffuseColor", "roughness", "metallic", "alpha"})
    {
        backwards.overridden.insert(key);
    }

    CNA_STUDIO_EXPECT_EQ(Json::write(backwards.toJson(), true),
                         Json::write(forwards.toJson(), true));
}

/**
 * @brief An asset sidecar is written the same way twice, and the one time it carries is the source's.
 *
 * `sourceModifiedTime` looks like a leaked timestamp and is not one: it is a fact about the file
 * beside it, so it is the same on every machine that has the same file and changes only when the
 * asset does. That is the distinction this case holds — a sidecar recording *when it was written*
 * would make every scan rewrite every sidecar, and turn opening a project into a commit.
 *
 * Driven through the real write-and-scan path rather than the serialiser, because that is what the
 * editor does and because the serialiser is not public.
 */
CNA_STUDIO_TEST(AnAssetSidecarIsStableAndItsOnlyTimeIsTheSourcesOwn)
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "cna-studio-determinism-sidecar";
    std::error_code code;
    std::filesystem::remove_all(root, code);
    std::filesystem::create_directories(root / "Assets", code);

    {
        std::ofstream stream{root / "Assets" / "Crate.png", std::ios::binary};
        stream << "not really a png";
    }

    const auto readSidecar = [&root] {
        std::ifstream stream{root / "Assets" / "Crate.png.cnaasset", std::ios::binary};
        return std::string{std::istreambuf_iterator<char>{stream},
                           std::istreambuf_iterator<char>{}};
    };

    AssetDatabase assets;
    assets.setProjectRoot(root.generic_string());
    CNA_STUDIO_EXPECT(assets.scan("Assets").succeeded);

    const AssetRecord* crate = assets.findByPath("Assets/Crate.png");
    CNA_STUDIO_EXPECT(crate != nullptr);
    if (crate == nullptr) { std::filesystem::remove_all(root, code); return; }

    const Uuid id = crate->id;
    CNA_STUDIO_EXPECT(assets.writeSidecar(id));
    const std::string first = readSidecar();
    CNA_STUDIO_EXPECT(!first.empty());

    // Written again from the same record: identical.
    CNA_STUDIO_EXPECT(assets.writeSidecar(id));
    CNA_STUDIO_EXPECT_EQ(readSidecar(), first);

    // And after a scan that read it back, which is the round trip. A scan that rewrote the file
    // differently would put a diff in front of everyone who opened the project.
    AssetDatabase second;
    second.setProjectRoot(root.generic_string());
    CNA_STUDIO_EXPECT(second.scan("Assets").succeeded);
    CNA_STUDIO_EXPECT(second.findByPath("Assets/Crate.png") != nullptr);
    CNA_STUDIO_EXPECT(second.writeSidecar(id));
    CNA_STUDIO_EXPECT_EQ(readSidecar(), first);

    // The id survived, which is what makes the bytes the same bytes rather than merely the same
    // shape.
    CNA_STUDIO_EXPECT(first.find(id.toString()) != std::string::npos);

    // No time of *writing* in it. The source's own modification time is there and is a fact about
    // the file, not about the save.
    CNA_STUDIO_EXPECT(!mentionsThisYear(first));

    std::filesystem::remove_all(root, code);
}
