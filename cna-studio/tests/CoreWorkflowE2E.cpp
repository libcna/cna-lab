// SPDX-License-Identifier: MS-PL
/**
 * @file CoreWorkflowE2E.cpp
 * @brief The whole Core workflow, once, against a real CNA build (`plan.md` CORE-10).
 *
 * ### Why this exists at all
 *
 * "Complete" has to mean something a test can check, or maintenance mode is an assertion rather
 * than a state. `plan.md` defines CNA Studio as eleven steps a developer takes, and the suite
 * covers every one of them — thoroughly, in isolation, mostly without CNA. What nothing covered is
 * the *sequence*: create a project, put an asset in it, edit a scene, save it, reopen it, build it,
 * play it, kill the player, and get the work back. Every step of that had a test and the walk did
 * not, which is how a product passes its whole suite and cannot be used.
 *
 * ### Why it is its own executable
 *
 * Because it compiles CNA. The unit suite is a hundred and eighty thousand assertions that run in
 * a minute and a half, and a case that takes five would make it something people stop running. So
 * this is a separate binary, one CTest case, labelled `slow`, holding the same resource lock as
 * the template builds — for the same reason they do: each compiles CNA into its own tree, and
 * running two at once is how a CI job runs out of memory.
 *
 * ### What it does *not* do
 *
 * It does not re-test what the unit suite already tests. Every assertion here is about the step
 * having *happened* and the next one being able to start from it; the details of each — what an
 * importer does with a PNG, what an undo entry contains, how a build command is quoted — belong to
 * the cases that own them and are not repeated.
 */

#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/Core/StudioCommand.hpp"
#include "CNA/Studio/Project/BuildDiagnostics.hpp"
#include "CNA/Studio/Project/BuildRunner.hpp"
#include "CNA/Studio/Project/LanguageAdapter.hpp"
#include "CNA/Studio/Project/ProjectCreation.hpp"
#include "CNA/Studio/Project/ProjectTemplate.hpp"
#include "CNA/Studio/Project/TargetProfile.hpp"
#include "CNA/Studio/Project/RecoveryStore.hpp"
#include "CNA/Studio/RuntimeBridge/PlayerProcess.hpp"
#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneCommands.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/StudioContext.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace CNA::Studio;

namespace
{
    int failures = 0;

    /** @brief Reports a step that did not work, and lets the walk stop where it stopped. */
    bool step(bool ok, const std::string& what)
    {
        if (ok)
        {
            std::cout << "core-e2e: " << what << "\n" << std::flush;
            return true;
        }
        std::cerr << "core-e2e: FAILED at: " << what << "\n" << std::flush;
        ++failures;
        return false;
    }

    /** @brief A 2x2 PNG, so the import is of a real file rather than of a name. */
    void writeTinyPng(const std::filesystem::path& path)
    {
        // Written byte by byte rather than copied from the repository, so this file has no fixture
        // to keep in step with it. The bytes are a 2x2 opaque RGBA image with one IDAT chunk.
        static const unsigned char kPng[] = {
            0x89, 'P',  'N',  'G',  0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 'I',  'H',
            'D',  'R',  0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x08, 0x06, 0x00, 0x00,
            0x00, 0x72, 0xB6, 0x0D, 0x24, 0x00, 0x00, 0x00, 0x16, 'I',  'D',  'A',  'T',  0x78,
            0x9C, 0x63, 0xFC, 0xCF, 0xC0, 0xF0, 0x9F, 0x81, 0x81, 0x81, 0x11, 0x03, 0x18, 0x00,
            0x1D, 0xC1, 0x03, 0xFD, 0x2B, 0x8C, 0x8D, 0xF1, 0x00, 0x00, 0x00, 0x00, 'I',  'E',
            'N',  'D',  0xAE, 0x42, 0x60, 0x82};

        std::ofstream stream{path, std::ios::binary};
        stream.write(reinterpret_cast<const char*>(kPng), static_cast<std::streamsize>(sizeof(kPng)));
    }
}

int main(int argc, char** argv)
{
    // Told where things are rather than guessing: the CNA checkout, the directory to work in, and
    // where the player binaries were built. A test that searched for them could pass by finding
    // the wrong one.
    std::string cnaRoot;
    std::string workDirectory;
    std::string playerDirectory;
    std::string renderer = "software";
    std::string jobs;

    for (int i = 1; i < argc; ++i)
    {
        const std::string argument{argv[i]};
        const auto value = [&argument](std::string_view name) {
            return argument.rfind(name, 0) == 0 ? argument.substr(name.size()) : std::string{};
        };
        if (!value("--cna-root=").empty()) { cnaRoot = value("--cna-root="); }
        if (!value("--work-dir=").empty()) { workDirectory = value("--work-dir="); }
        if (!value("--player-dir=").empty()) { playerDirectory = value("--player-dir="); }
        if (!value("--renderer=").empty()) { renderer = value("--renderer="); }
        if (!value("--jobs=").empty()) { jobs = value("--jobs="); }
    }

    if (workDirectory.empty())
    {
        std::cerr << "core-e2e: --work-dir= is required\n";
        return 2;
    }

    std::error_code code;
    std::filesystem::remove_all(workDirectory, code);
    std::filesystem::create_directories(workDirectory, code);

    // --- 1. Create a project from a template -----------------------------------------------------
    //
    // Through `createStudioProject`, which is what the Project Hub's New button runs. A name with a
    // space in it, for the reason the template-build test uses one: a generator that composed a
    // target name without reducing it produces a tree that does not configure.
    StudioTemplateCatalogue templates;
    for (const std::string& path : studioTemplateSearchPaths(argv[0]))
    {
        (void)templates.addSearchPath(path);
    }
    const StudioLanguageRegistry languages = studioBuiltInLanguages();

    if (!step(templates.find("basic-sample") != nullptr,
              "0. found the basic-sample template to create from"))
    {
        return 1;
    }

    StudioNewProjectRequest request;
    request.name = "Core Walk";
    request.directory = (std::filesystem::path{workDirectory} / "Core Walk").generic_string();
    request.templateId = "basic-sample";

    const StudioNewProjectResult created = createStudioProject(request, templates, languages);
    if (!step(created.succeeded(), "1. created a project from the basic-sample template"))
    {
        for (const StudioNewProjectProblem& problem : created.problems)
        {
            std::cerr << "core-e2e:   " << problem.message << "\n";
        }
        return 1;
    }

    // --- 2. Open it ------------------------------------------------------------------------------
    StudioContext context;
    std::string problem;
    if (!step(context.openProject(created.projectFilePath, &problem),
              "2. opened it: " + created.projectFilePath))
    {
        std::cerr << "core-e2e:   " << problem << "\n";
        return 1;
    }

    const std::filesystem::path projectRoot{context.getProject().getRootPath()};

    // --- 3. Import an asset ----------------------------------------------------------------------
    //
    // A file put where a user would put it, and then the same scan the editor runs. The assertion
    // is that the database *found* it: an import that needs the panel to be open is not an import.
    const std::filesystem::path imported = projectRoot / "Assets" / "core-walk.png";
    std::filesystem::create_directories(imported.parent_path(), code);
    writeTinyPng(imported);

    const std::size_t before = context.getAssets().getAll().size();
    (void)context.getAssets().scan("Assets");
    const AssetRecord* record = context.getAssets().findByPath("Assets/core-walk.png");

    step(record != nullptr && context.getAssets().getAll().size() > before,
         "3. imported an asset and the database found it");

    // --- 4. Edit the scene, through the history --------------------------------------------------
    //
    // Through a command, because every mutation in Studio goes through one and a workflow test that
    // took the short cut would be testing a path no user can take.
    const std::size_t entitiesBefore = context.getScene().getEntityCount();

    StudioEntity added{Uuid::generate(), "Core Walk Marker"};
    StudioComponent transform{BuiltinComponentIds::kTransform};
    transform.applyDefaults(*context.getComponentRegistry().find(BuiltinComponentIds::kTransform));
    added.addComponent(std::move(transform));

    context.execute(std::make_unique<CreateEntityCommand>(context.getScene(), std::move(added)));

    step(context.getScene().getEntityCount() == entitiesBefore + 1
             && context.getHistory().canUndo(),
         "4. added an entity through the command history");

    // --- 5. Save ---------------------------------------------------------------------------------
    step(context.saveScene() && !context.getHistory().isDirty(), "5. saved the scene");

    // --- 6. Reopen, in a context that has never seen it ------------------------------------------
    //
    // A fresh context rather than a reload, because the question is whether what was written can be
    // read by a Studio that was not the one that wrote it.
    {
        StudioContext reopened;
        std::string reopenProblem;
        const bool opened = reopened.openProject(created.projectFilePath, &reopenProblem);

        bool found = false;
        for (const StudioEntity& entity : reopened.getScene().getEntities())
        {
            if (entity.getName() == "Core Walk Marker") { found = true; }
        }

        if (!step(opened && found, "6. reopened the project and the edit was there"))
        {
            std::cerr << "core-e2e:   " << reopenProblem << "\n";
        }
    }

    // --- 7. Build it, with its own CMake ---------------------------------------------------------
    //
    // The real thing: the commands the Build panel would run, run. This is the slow step and the
    // one that has never been part of a walk -- `STUDIO-08011` builds a project it creates, and
    // this builds the project the previous six steps have been editing.
    // The target this build of Studio can actually compile against, chosen the way a user chooses
    // one: by editing the project's active target profile. The default a new project gets is
    // `opengles3`, which needs sibling checkouts a CI machine may not have -- and a workflow test
    // that failed on a missing renderer would be reporting on the machine rather than on Studio.
    {
        std::vector<StudioTargetProfile> profiles = context.getProject().getTargetProfiles();
        if (profiles.empty()) { profiles.push_back(StudioTargetProfile::defaults()); }
        profiles.front().renderer = renderer;
        context.getProject().setTargetProfiles(profiles);
        step(context.getProject().getActiveTargetProfile().renderer == renderer,
             "7. targeting the " + renderer + " renderer");
    }

    const StudioLanguageAdapter* language = context.getLanguage();
    if (!step(language != nullptr, "7a. the project's language has an adapter")) { return 1; }

    const StudioToolchainReport toolchain = language->probeToolchain({});
    if (!step(toolchain.available, "7b. found a toolchain: " + toolchain.toolchainPath))
    {
        std::cerr << "core-e2e:   " << toolchain.problem << "\n";
        return 1;
    }

    StudioBuildJob job = language->planBuild(context.getProject(), toolchain);
    if (!step(!job.empty(), "7c. planned a build of " + std::to_string(job.steps.size()) + " steps"))
    {
        return 1;
    }

    // CNA is where it is, and the generated CMakeLists reads `CNA_ROOT`. Passed on the configure
    // step rather than exported into the environment, so what the build is told is visible here.
    if (!cnaRoot.empty()) { job.steps.front().arguments.push_back("-DCNA_ROOT=" + cnaRoot); }

    // The build step plans `--parallel` with no number, which means one job per core -- right for
    // a developer's machine and wrong for a CI runner, where four simultaneous compilations of
    // CNA's graphics-ext exhaust the memory and the kernel kills the compiler. The template-build
    // tests pass a count for the same reason, and an out-of-memory kill reads in the log as a
    // build failure with no diagnostic in it, which is the least useful failure there is.
    if (!jobs.empty() && job.steps.size() > 1) { job.steps.back().arguments.push_back(jobs); }

    BuildProcess build;
    std::string buildProblem;
    if (!step(build.start(job, &buildProblem), "7d. started the build"))
    {
        std::cerr << "core-e2e:   " << buildProblem << "\n";
        return 1;
    }

    // Polled rather than waited on, which is how the editor runs it: a build that blocked the
    // frame would be worse than one the user starts from a terminal.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::minutes(45);
    while (build.getState() == BuildState::Running
           && std::chrono::steady_clock::now() < deadline)
    {
        build.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (!step(build.getState() == BuildState::Succeeded,
              "7e. the build succeeded; log at " + build.getLogPath()))
    {
        // The log's diagnostics, through the parser `CORE-01` added -- which is also the thing a
        // user would be reading, so a failure here prints what the Build panel would show.
        std::string log;
        for (const std::string& line : build.readLogTail(2000)) { log += line + "\n"; }

        const BuildDiagnostics found = studioParseBuildDiagnostics(log);
        for (const BuildDiagnostic& entry : found.entries)
        {
            std::cerr << "core-e2e:   " << entry.toRowText() << "\n";
        }
        if (found.empty()) { std::cerr << log; }
        return 1;
    }

    // --- 8. Play it ------------------------------------------------------------------------------
    //
    // The real separate process, over the real bridge. What is asserted is the handshake: a player
    // that started and never connected is a play session that looks live and is not.
    const std::vector<PlayerBuild> builds = discoverPlayerBuilds(playerDirectory);
    if (!step(!builds.empty(), "8a. found a player build in " + playerDirectory)) { return 1; }

    PlayerProcess player;
    if (!step(player.start(builds.front(), created.projectFilePath), "8b. started the player"))
    {
        std::cerr << "core-e2e:   " << player.getError() << "\n";
        return 1;
    }

    bool connected = false;
    const auto playDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (!connected && std::chrono::steady_clock::now() < playDeadline)
    {
        for (const StudioMessage& message : player.poll())
        {
            if (message.type == StudioMessageType::Ready || message.type == StudioMessageType::Hello)
            {
                connected = true;
            }
        }
        if (!player.isRunning()) { break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    step(connected, "8c. the player connected and said hello");

    // --- 9. Kill it ------------------------------------------------------------------------------
    //
    // Stopped the way the Stop button stops it, and the assertion is that Studio *noticed*. A
    // session that is over and still reported as running is the state every stale-state defect in
    // play mode has looked like.
    player.stop();
    for (int attempt = 0; attempt < 200 && player.isRunning(); ++attempt)
    {
        (void)player.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    step(!player.isRunning(), "9. stopped the player and Studio noticed");

    // --- 10. Recover -----------------------------------------------------------------------------
    //
    // An edit that was never saved, a snapshot, and then a Studio that finds it by the project it
    // belongs to. That is the whole of what recovery promises: work survives a Studio that did not
    // get to exit.
    const std::filesystem::path recoveryDirectory =
        std::filesystem::path{workDirectory} / "recovery";

    RecoveryStore recovery{recoveryDirectory.generic_string()};

    StudioEntity unsaved{Uuid::generate(), "Unsaved After Crash"};
    context.execute(std::make_unique<CreateEntityCommand>(context.getScene(), std::move(unsaved)));

    RecoverySnapshot snapshot;
    snapshot.projectPath = created.projectFilePath;
    snapshot.scenePath = context.getScenePath();
    snapshot.sceneName = context.getScene().getName();
    snapshot.sceneId = context.getScene().getSceneId();
    snapshot.savedAtSeconds = 1;
    snapshot.scene = context.getScene().toJson();

    std::string recoveryProblem;
    step(recovery.write(snapshot, &recoveryProblem),
         "10a. autosaved a snapshot of unsaved work");

    const std::optional<RecoverySnapshot> offered =
        recovery.findForProject(created.projectFilePath);
    step(offered.has_value(), "10b. a fresh Studio finds the snapshot for this project");

    if (offered)
    {
        SceneDocument recovered;
        const SceneLoadResult read =
            recovered.loadFromJson(offered->scene, context.getComponentRegistry());

        bool hasUnsaved = false;
        for (const StudioEntity& entity : recovered.getEntities())
        {
            if (entity.getName() == "Unsaved After Crash") { hasUnsaved = true; }
        }
        step(read.succeeded && hasUnsaved, "10c. the snapshot holds the work that was never saved");
    }

    // --- 11. Hand off to an external editor ------------------------------------------------------
    //
    // The eleventh Core step, and the cheapest to walk: what it has to do is name a real file in
    // the project. The gesture itself is `CORE-02`'s and is tested there; what this adds is that
    // the file the language points at is in the project these ten steps built.
    const std::filesystem::path entryPoint =
        projectRoot / language->descriptor().sourceDirectory
        / language->descriptor().entryPointFile;
    step(std::filesystem::exists(entryPoint, code),
         "11. the project has the source file an IDE would be handed: "
             + entryPoint.generic_string());

    if (failures == 0)
    {
        std::cout << "core-e2e: the Core workflow walked end to end\n";
        return 0;
    }

    std::cerr << "core-e2e: " << failures << " step(s) of the Core workflow failed\n";
    return 1;
}
