// SPDX-License-Identifier: MS-PL
/**
 * @file PartialFileTests.cpp
 * @brief What Studio does with the file an interrupted save left behind (`plan.md` STUDIO-31010).
 *
 * `STUDIO-31003` made every authored write atomic, so a save that is interrupted leaves the
 * previous document whole. It does not make the interruption invisible: the temporary beside the
 * target survives the crash, and a sidecar or a document written by an *older* build -- or by
 * anything else that truncated in place -- is still out there half-written. These cases are the
 * reading half of the row.
 *
 * Three properties, and the third is the one that was broken:
 *
 * 1. **A leftover temporary is not an asset.** The scan skips it and says so, rather than importing
 *    a half-written file under a name the user did not choose.
 * 2. **Every authored loader refuses a partial file** rather than reading what it can and calling
 *    the result a document. A scene half-loaded is worse than a scene not loaded: the user saves
 *    over the original with it.
 * 3. **A malformed sidecar does not change an asset's identity.** Scenes reference assets by id
 *    (D-08), so a regenerated id breaks every reference to that asset across the project -- and the
 *    scan was regenerating one whenever the sidecar's JSON would not parse, which is precisely what
 *    a half-written sidecar is.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/Assets/EnvironmentMapDocument.hpp"
#include "CNA/Studio/Assets/MaterialDocument.hpp"
#include "CNA/Studio/Core/StudioFileWrite.hpp"
#include "CNA/Studio/Project/Project.hpp"
#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/PrefabDocument.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

using namespace CNA::Studio;

namespace
{
    /** @brief A directory that cleans itself up. */
    class ScopedDirectory
    {
    public:
        explicit ScopedDirectory(const std::string& name)
        {
            path_ = std::filesystem::temp_directory_path()
                  / ("cna-studio-partial-" + name + "-" + std::to_string(counter()++));
            std::error_code code;
            std::filesystem::remove_all(path_, code);
            std::filesystem::create_directories(path_, code);
        }

        ~ScopedDirectory()
        {
            std::error_code code;
            std::filesystem::remove_all(path_, code);
        }

        ScopedDirectory(const ScopedDirectory&) = delete;
        ScopedDirectory& operator=(const ScopedDirectory&) = delete;

        [[nodiscard]] const std::filesystem::path& root() const { return path_; }

        [[nodiscard]] std::string at(const std::string& relative) const
        {
            return (path_ / relative).generic_string();
        }

        /** @brief Writes @p text verbatim, creating the folders above it. */
        void write(const std::string& relative, const std::string& text) const
        {
            const std::filesystem::path file = path_ / relative;
            std::error_code code;
            std::filesystem::create_directories(file.parent_path(), code);
            std::ofstream stream{file, std::ios::binary | std::ios::trunc};
            stream.write(text.data(), static_cast<std::streamsize>(text.size()));
        }

    private:
        static int& counter() { static int value = 0; return value; }
        std::filesystem::path path_;
    };

    /** @brief True where any warning contains @p fragment. */
    [[nodiscard]] bool said(const std::vector<std::string>& warnings, const std::string& fragment)
    {
        for (const std::string& warning : warnings)
        {
            if (warning.find(fragment) != std::string::npos) { return true; }
        }
        return false;
    }
}

/**
 * @brief A leftover temporary is recognised by shape, and a user's own file is not.
 *
 * The scanner has to tell "a save that was interrupted" from "a file the user named oddly", and
 * the only evidence on disk is the name. `studioWriteFileAtomically` builds one shape -- the
 * suffix followed by a ticket number -- so that is the shape recognised, rather than the suffix
 * appearing anywhere in the name.
 */
CNA_STUDIO_TEST(ALeftoverTemporaryIsRecognisedByItsShapeAndNotByASubstring)
{
    CNA_STUDIO_EXPECT(studioIsWriteTemporaryName("Level.cnascene.cnatmp1"));
    CNA_STUDIO_EXPECT(studioIsWriteTemporaryName("Level.cnascene.cnatmp417"));
    CNA_STUDIO_EXPECT(studioIsWriteTemporaryName("Crate.png.cnaasset.cnatmp9"));

    // No ticket, so not a name this writer produces.
    CNA_STUDIO_EXPECT(!studioIsWriteTemporaryName("Level.cnascene.cnatmp"));
    CNA_STUDIO_EXPECT(!studioIsWriteTemporaryName("Level.cnascene"));

    // A document the user deliberately called this is theirs. A scanner that hid it would be
    // deciding what their files mean from a substring.
    CNA_STUDIO_EXPECT(!studioIsWriteTemporaryName("notes.cnatmp-ideas.txt"));
    CNA_STUDIO_EXPECT(!studioIsWriteTemporaryName("cnatmp42.png"));

    // And the shape the writer actually produces round-trips, rather than being asserted from the
    // suffix constant by hand -- a writer that changed its naming and a test that did not would
    // agree with each other and with nothing on disk.
    const ScopedDirectory directory{"shape"};
    CNA_STUDIO_EXPECT(studioWriteFileAtomically(directory.at("Kept.cnascene"), "{}").succeeded);

    std::filesystem::path probe = directory.root() / "Kept.cnascene";
    probe += std::string{kStudioWriteTemporarySuffix} + "1";
    CNA_STUDIO_EXPECT(studioIsWriteTemporaryName(probe.filename().generic_string()));
}

/**
 * @brief The scan skips a half-written file and says so, rather than importing it.
 *
 * An interrupted save leaves `Crate.png.cnatmp7` beside `Crate.png`. Imported, it becomes a second
 * asset with its own id and its own sidecar, referenced by nothing, that the user cannot identify
 * and will not know is safe to delete. **Reported rather than deleted**, per `STUDIO-31011`: it may
 * be the only copy of the bytes if the original save was the one that failed.
 */
CNA_STUDIO_TEST(AnInterruptedSaveLeavesAFileTheScanSkipsAndReports)
{
    const ScopedDirectory directory{"scan"};
    directory.write("Assets/Crate.png", "not really a png");
    directory.write("Assets/Crate.png.cnatmp7", "half of a png");
    directory.write("Assets/Level.cnascene.cnatmp12", "{\"formatVersion\"");
    directory.write("Assets/notes.cnatmp-ideas.txt", "mine");

    AssetDatabase assets;
    assets.setProjectRoot(directory.root().generic_string());
    const AssetScanResult scanned = assets.scan("Assets");
    CNA_STUDIO_EXPECT(scanned.succeeded);

    CNA_STUDIO_EXPECT(assets.findByPath("Assets/Crate.png") != nullptr);
    CNA_STUDIO_EXPECT(assets.findByPath("Assets/Crate.png.cnatmp7") == nullptr);
    CNA_STUDIO_EXPECT(assets.findByPath("Assets/Level.cnascene.cnatmp12") == nullptr);

    // Said, not swallowed. The user has to be able to find out why a file they can see in their
    // file manager is not in the Content Browser.
    CNA_STUDIO_EXPECT(said(scanned.warnings, "Crate.png.cnatmp7"));
    CNA_STUDIO_EXPECT(said(scanned.warnings, "half-written"));

    // Left where it is, because it may be the only copy of the bytes.
    CNA_STUDIO_EXPECT(std::filesystem::exists(directory.root() / "Assets" / "Crate.png.cnatmp7"));

    // And the file that only looks like one is imported like any other.
    CNA_STUDIO_EXPECT(assets.findByPath("Assets/notes.cnatmp-ideas.txt") != nullptr);
}

/**
 * @brief Every authored loader refuses a partial file rather than half-reading it.
 *
 * The failure this forbids is not a crash. It is a scene that loads with three of its ten entities,
 * looks like a scene, and is saved back over the original -- at which point the interruption has
 * cost the user the seven entities that the atomic write had preserved.
 *
 * Four shapes, because they fail at different places: truncated mid-token, empty, whitespace only,
 * and bytes that are not text at all.
 */
CNA_STUDIO_TEST(EveryAuthoredDocumentRefusesAPartialFile)
{
    const ScopedDirectory directory{"loaders"};

    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    const std::vector<std::pair<std::string, std::string>> partials = {
        {"truncated",
         R"({"formatVersion":1,"entities":[{"id":"0cf45f27-2ecd-44a6-8c45-cd8d2122179f","na)"},
        {"empty", std::string{}},
        {"whitespace", std::string{"   \n\t "}},
        {"binary", std::string{"\x00\x01\x02\x03", 4}}};

    for (const auto& [label, bytes] : partials)
    {
        {
            const std::string file = "s-" + label + ".cnascene";
            directory.write(file, bytes);
            SceneDocument scene;
            const SceneLoadResult loaded = scene.loadFromFile(directory.at(file), registry);
            CNA_STUDIO_EXPECT(!loaded.succeeded);
            // Named, so the user can tell a truncated file from one this build is too old to read.
            CNA_STUDIO_EXPECT(!loaded.errorMessage.empty());
            // And nothing was half-built: a document that refused is a document unchanged.
            CNA_STUDIO_EXPECT(scene.getEntities().empty());
        }
        {
            const std::string file = "p-" + label + ".cnaprefab";
            directory.write(file, bytes);
            PrefabDocument prefab;
            const PrefabLoadResult loaded = prefab.loadFromFile(directory.at(file), registry);
            CNA_STUDIO_EXPECT(!loaded.succeeded);
            CNA_STUDIO_EXPECT(!loaded.errorMessage.empty());
        }
        {
            const std::string file = "m-" + label + ".cnamaterial";
            directory.write(file, bytes);

            // Loaded over a document that already holds something, because that is the live case:
            // the editor has the material open and is reloading it from disk. A loader that
            // half-assigned would leave the open document part old and part new.
            MaterialDocument material;
            material.name = "Brick";
            CNA_STUDIO_EXPECT(loadMaterialFile(directory.at(file), material)
                              == MaterialLoadProblem::UnreadableFormat);
            CNA_STUDIO_EXPECT_EQ(material.name, std::string{"Brick"});
        }
        {
            const std::string file = "e-" + label + ".cnaenv";
            directory.write(file, bytes);
            EnvironmentMapDocument environment;
            environment.name = "Overcast";
            CNA_STUDIO_EXPECT(loadEnvironmentMapFile(directory.at(file), environment)
                              == EnvironmentMapLoadProblem::UnreadableFormat);
            CNA_STUDIO_EXPECT_EQ(environment.name, std::string{"Overcast"});
        }
        {
            const std::string file = "j-" + label + ".cnaproject";
            directory.write(file, bytes);
            Project project;
            const ProjectLoadResult loaded = project.loadFromFile(directory.at(file));
            CNA_STUDIO_EXPECT(!loaded.succeeded);
            CNA_STUDIO_EXPECT(!loaded.errorMessage.empty());
        }
    }
}

/**
 * @brief **A malformed sidecar does not change the asset's identity.**
 *
 * The defect this pins was written down three lines above the code that broke it. Where a sidecar
 * *migration* fails, the scan keeps the id and says why: *"Scenes reference assets by id (D-08), so
 * regenerating it would break every reference in the project -- a far worse outcome than an
 * importer setting reverting to its default."* Where the sidecar's **JSON** would not parse, it
 * assigned a new one.
 *
 * A half-written sidecar is exactly that case, and it is the case `STUDIO-31003` cannot prevent:
 * the sidecars written by every build before it truncated in place. `id` is the second key
 * `recordToJson` writes, so a file cut anywhere after the first forty bytes still holds it whole --
 * and recovering it costs nothing and saves every reference in the project.
 */
CNA_STUDIO_TEST(AHalfWrittenSidecarKeepsTheAssetsIdentity)
{
    const ScopedDirectory directory{"sidecar"};
    directory.write("Assets/Crate.png", "not really a png");

    const Uuid identity = Uuid::parse("5c1d8e34-0f2b-4a77-9c61-2b8e5f0a41d3");
    CNA_STUDIO_EXPECT(identity.isValid());

    // A sidecar cut off partway through, which is what a save interrupted by a crash or a full
    // disk leaves behind: legal JSON up to the cut and nothing after it.
    directory.write("Assets/Crate.png.cnaasset",
                    "{\n  \"formatVersion\": 1,\n  \"id\": \"" + identity.toString()
                        + "\",\n  \"type\": \"Texture2D\",\n  \"importerSetti");

    AssetDatabase assets;
    assets.setProjectRoot(directory.root().generic_string());
    const AssetScanResult scanned = assets.scan("Assets");
    CNA_STUDIO_EXPECT(scanned.succeeded);

    const AssetRecord* crate = assets.findByPath("Assets/Crate.png");
    CNA_STUDIO_EXPECT(crate != nullptr);
    if (crate == nullptr) { return; }

    // The identity survived, so every scene, prefab and material pointing at it still resolves.
    CNA_STUDIO_EXPECT(crate->id == identity);

    // Not counted as new, because it is not new -- a scan that reported it as a fresh import would
    // be telling the user the opposite of what happened.
    CNA_STUDIO_EXPECT_EQ(scanned.newCount, std::size_t{0});

    // And it is a repair, so it is reported (STUDIO-31011) -- both halves: what was rescued and
    // what was lost.
    CNA_STUDIO_EXPECT(said(scanned.warnings, "malformed"));
    CNA_STUDIO_EXPECT(said(scanned.warnings, "id was recovered"));
    CNA_STUDIO_EXPECT(said(scanned.warnings, "settings were lost"));

    // The file is left on disk untouched, so a build that understands it still can and a user who
    // wants to look at it still may.
    CNA_STUDIO_EXPECT(std::filesystem::exists(directory.root() / "Assets" / "Crate.png.cnaasset"));
}

/**
 * @brief Where no id can be recovered, the loss is stated as a loss.
 *
 * The honest half of the row above. A sidecar cut before its `id`, or truncated mid-value, holds
 * nothing to preserve -- so a new id is the only option and the warning has to say that references
 * to the asset will not resolve, rather than reporting a routine assignment.
 */
CNA_STUDIO_TEST(ASidecarWithNoReadableIdSaysTheReferencesAreBroken)
{
    const ScopedDirectory directory{"sidecar-lost"};
    directory.write("Assets/Crate.png", "not really a png");

    // Cut before the id was written at all.
    directory.write("Assets/Crate.png.cnaasset", "{\n  \"formatVersion\": 1,\n  \"i");

    AssetDatabase assets;
    assets.setProjectRoot(directory.root().generic_string());
    const AssetScanResult scanned = assets.scan("Assets");
    CNA_STUDIO_EXPECT(scanned.succeeded);

    const AssetRecord* crate = assets.findByPath("Assets/Crate.png");
    CNA_STUDIO_EXPECT(crate != nullptr);
    if (crate == nullptr) { return; }

    CNA_STUDIO_EXPECT(crate->id.isValid());
    CNA_STUDIO_EXPECT_EQ(scanned.newCount, std::size_t{1});
    CNA_STUDIO_EXPECT(said(scanned.warnings, "no readable id"));
    CNA_STUDIO_EXPECT(said(scanned.warnings, "will not resolve"));

    // A value that is present but is not a Uuid is the same case: recovering a malformed id would
    // be inventing one, and a scene pointing at the real id would not find it either way.
    const ScopedDirectory garbled{"sidecar-garbled"};
    garbled.write("Assets/Crate.png", "not really a png");
    garbled.write("Assets/Crate.png.cnaasset", "{\n  \"id\": \"not-a-uuid\",\n  \"type\": \"Tex");

    AssetDatabase second;
    second.setProjectRoot(garbled.root().generic_string());
    const AssetScanResult again = second.scan("Assets");
    CNA_STUDIO_EXPECT(again.succeeded);
    CNA_STUDIO_EXPECT(said(again.warnings, "no readable id"));
}

/**
 * @brief Every authored loader says **where** a file is broken, not just that it is.
 *
 * `plan.md` STUDIO-31008: *"a tool that refuses to open a slightly broken file is one you cannot
 * use to fix a broken file."* `EveryAuthoredDocumentRefusesAPartialFile` above pins the refusal;
 * this pins that the refusal is usable. A message reading `"unexpected token at offset 48213"` is a
 * refusal with a number attached — the user has to go and count bytes to act on it, and the tool
 * that produced the number is the one that could have counted for them.
 */
CNA_STUDIO_TEST(ARefusedDocumentSaysWhichLineIsWrong)
{
    const ScopedDirectory directory{"where"};

    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    // A missing colon on line 4, which is the shape of a hand edit that went wrong.
    const std::string broken =
        "{\n"
        "  \"formatVersion\": 1,\n"
        "  \"sceneId\": \"0cf45f27-2ecd-44a6-8c45-cd8d2122179f\",\n"
        "  \"name\" \"Level01\",\n"
        "  \"entities\": []\n"
        "}\n";

    directory.write("Level01.cnascene", broken);
    SceneDocument scene;
    const SceneLoadResult sceneLoaded =
        scene.loadFromFile(directory.at("Level01.cnascene"), registry);
    CNA_STUDIO_EXPECT(!sceneLoaded.succeeded);
    CNA_STUDIO_EXPECT(sceneLoaded.errorMessage.find("line 4") != std::string::npos);
    CNA_STUDIO_EXPECT(sceneLoaded.errorMessage.find("\"name\"") != std::string::npos);

    // The path is still in it: a message naming a line but not a file is useless in a project with
    // forty scenes.
    CNA_STUDIO_EXPECT(sceneLoaded.errorMessage.find("Level01.cnascene") != std::string::npos);

    // And no offset, which is the thing it replaced rather than something it was added beside.
    CNA_STUDIO_EXPECT(sceneLoaded.errorMessage.find("offset") == std::string::npos);

    directory.write("Enemy.cnaprefab", broken);
    PrefabDocument prefab;
    const PrefabLoadResult prefabLoaded =
        prefab.loadFromFile(directory.at("Enemy.cnaprefab"), registry);
    CNA_STUDIO_EXPECT(!prefabLoaded.succeeded);
    CNA_STUDIO_EXPECT(prefabLoaded.errorMessage.find("line 4") != std::string::npos);

    directory.write("Game.cnaproject", broken);
    Project project;
    const ProjectLoadResult projectLoaded = project.loadFromFile(directory.at("Game.cnaproject"));
    CNA_STUDIO_EXPECT(!projectLoaded.succeeded);
    CNA_STUDIO_EXPECT(projectLoaded.errorMessage.find("line 4") != std::string::npos);
}

/** @brief And so does the sidecar warning, which is the one an ordinary scan produces. */
CNA_STUDIO_TEST(AMalformedSidecarsWarningSaysWhichLineIsWrong)
{
    const ScopedDirectory directory{"sidecar-where"};
    directory.write("Assets/Crate.png", "not really a png");
    directory.write("Assets/Crate.png.cnaasset",
                    "{\n"
                    "  \"formatVersion\": 1,\n"
                    "  \"id\": \"5c1d8e34-0f2b-4a77-9c61-2b8e5f0a41d3\",\n"
                    "  \"type\" \"Texture2D\"\n"
                    "}\n");

    AssetDatabase assets;
    assets.setProjectRoot(directory.root().generic_string());
    const AssetScanResult scanned = assets.scan("Assets");
    CNA_STUDIO_EXPECT(scanned.succeeded);

    CNA_STUDIO_EXPECT(said(scanned.warnings, "line 4"));
    CNA_STUDIO_EXPECT(said(scanned.warnings, "id was recovered"));
}

/**
 * @brief **Every automatic change to a user's document is reported.**
 *
 * `plan.md` STUDIO-31011. An inventory, not a sample. Studio repairs rather than refuses in a
 * dozen places, and each of those decisions is right on its own — a scene whose parent entity was
 * deleted by a bad merge should still open, or the user cannot fix it (`STUDIO-31008`). What makes
 * the set dangerous is that every one of them is written back on the next save. A repair the user
 * was not told about is a change to their file that they did not make, did not see, and will find
 * in a diff weeks later with no idea what caused it.
 *
 * So the rule is: **repair freely, report always.** This case is the list, and the point of
 * gathering it in one place is that a new repair added without a message fails here rather than
 * shipping — the reviewer of the thirteenth one has this to look at.
 */
CNA_STUDIO_TEST(EveryAutomaticChangeToADocumentIsReported)
{
    const ScopedDirectory directory{"repairs"};

    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    const std::string keptId = "0cf45f27-2ecd-44a6-8c45-cd8d2122179f";

    // One scene carrying six repairs at once, because they have to coexist: a loader that reported
    // the first and stopped would pass six single-fault cases.
    const std::string scene = std::string{"{"}
        + R"("formatVersion":1,"name":"Level01","entities":[)"
        // 1. No scene id.  2. An entity with no id.
        + R"({"name":"NoId","components":{}},)"
        // 3. A parent that is not there.
        + R"({"id":")" + keptId + R"(","name":"Orphan","parent":"11111111-2222-3333-4444-555555555555","components":{}},)"
        // 4. A duplicate id.
        + R"({"id":")" + keptId + R"(","name":"Twin","components":{}},)"
        // 5. A component type nothing registered.
        + R"({"id":"22222222-2222-3333-4444-555555555555","name":"Plugin","components":{"Nobody.Loaded":{"x":1}}},)"
        // 6. A property whose value the declared type cannot hold.
        + R"({"id":"33333333-2222-3333-4444-555555555555","name":"Player",)"
        + R"("components":{"CNA.Transform":{"position":"over there"}}})"
        + R"(]})";

    directory.write("Level01.cnascene", scene);

    SceneDocument document;
    const SceneLoadResult loaded =
        document.loadFromFile(directory.at("Level01.cnascene"), registry);

    // It opens. Every one of these is a repair rather than a refusal, on purpose.
    CNA_STUDIO_EXPECT(loaded.succeeded);

    struct Repair
    {
        const char* what;
        const char* fragment;
    };

    // Each row is a change Studio made to what the file said. If one of these stops being
    // reported, the editor has started editing people's documents without telling them.
    static const Repair kRepairs[] = {
        {"a scene with no id gets one", "sceneId"},
        {"an entity with no id gets one", "no valid id"},
        {"an entity whose parent is missing becomes a root", "missing parent"},
        {"a duplicate entity id drops the later entity", "duplicate entity id"},
        {"a component nothing registered keeps its data", "unregistered component type"},
        {"a value the declared type cannot hold is replaced", "the default was used"},
    };

    for (const Repair& repair : kRepairs)
    {
        bool reported = false;
        for (const std::string& warning : loaded.warnings)
        {
            if (warning.find(repair.fragment) != std::string::npos) { reported = true; }
        }
        if (!reported)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"Studio changed the document -- "} + repair.what
                + " -- and said nothing. Every automatic change to a user's file is reported "
                  "(plan.md STUDIO-31011), because the change is written back on the next save "
                  "and a diff is not where somebody should find out.");
        }
    }

    // And the same for the asset database, whose repairs reach a different result type.
    directory.write("Assets/Crate.png", "not really a png");
    directory.write("Assets/Crate.png.cnaasset", "{\n  \"id\": \"" + keptId + "\",\n  \"type\": \"Tex");
    directory.write("Assets/Sprite.png", "not really a png");
    directory.write("Assets/Sprite.png.cnatmp4", "half of a png");

    AssetDatabase assets;
    assets.setProjectRoot(directory.root().generic_string());
    const AssetScanResult scanned = assets.scan("Assets");
    CNA_STUDIO_EXPECT(scanned.succeeded);

    static const Repair kScanRepairs[] = {
        {"a malformed sidecar's id is recovered", "id was recovered"},
        {"a half-written file is skipped", "half-written"},
    };

    for (const Repair& repair : kScanRepairs)
    {
        bool reported = false;
        for (const std::string& warning : scanned.warnings)
        {
            if (warning.find(repair.fragment) != std::string::npos) { reported = true; }
        }
        if (!reported)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"The asset scan changed what the project holds -- "} + repair.what
                + " -- and said nothing (plan.md STUDIO-31011).");
        }
    }
}
