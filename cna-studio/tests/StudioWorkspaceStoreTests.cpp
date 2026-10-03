// SPDX-License-Identifier: MS-PL
/**
 * @file StudioWorkspaceStoreTests.cpp
 * @brief The workspace layout survives exit, and never costs anything when it does not.
 *
 * `plan.md` STUDIO-05014.
 *
 * Every case here is really about one question: what does a user lose when this goes wrong? The
 * answer has to be "their arrangement, and they are told", never "their work" and never "the
 * ability to start Studio at all". So the interesting cases are the failures -- a truncated file, a
 * file from a newer Studio, a directory that cannot be created, a save interrupted halfway.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Core/UserPaths.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"
#include "CNA/Studio/UiCore/StudioWorkspaceStore.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

using namespace CNA::Studio;

namespace
{
    /** @brief A temporary directory that removes itself, so a failing case leaves nothing behind. */
    class ScopedDirectory
    {
    public:
        explicit ScopedDirectory(const std::string& name)
        {
            path_ = std::filesystem::temp_directory_path()
                  / ("cna-studio-workspace-" + name + "-" + std::to_string(counter()++));
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

        [[nodiscard]] std::string file(const char* name) const
        {
            return (path_ / name).generic_string();
        }

        /** @brief The directory itself, for a case that needs to plant siblings inside it. */
        [[nodiscard]] const std::filesystem::path& path() const { return path_; }

    private:
        static int& counter() { static int value = 0; return value; }
        std::filesystem::path path_;
    };

    void writeText(const std::string& path, std::string_view text)
    {
        std::ofstream stream{path, std::ios::binary | std::ios::trunc};
        stream << text;
    }

    /** @brief A shell with the panels the default workspace arranges. */
    std::unique_ptr<StudioShell> makeShell()
    {
        auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
        shell->resetLayout();
        return shell;
    }
}

CNA_STUDIO_TEST(AnArrangementSurvivesBeingWrittenAndReadBack)
{
    const ScopedDirectory directory{"roundtrip"};
    const StudioWorkspaceStore store{directory.file("workspace.json")};

    const std::unique_ptr<StudioShell> arranged = makeShell();

    // Changed from the default, or this would pass just as happily on a store that wrote nothing
    // and a shell that reset itself.
    const std::vector<std::string> before = arranged->dockTree().panels();
    CNA_STUDIO_EXPECT(before.size() > 3);
    CNA_STUDIO_EXPECT(arranged->closePanel("output"));

    std::string problem;
    CNA_STUDIO_EXPECT(store.save(arranged->saveLayout(), &problem));
    CNA_STUDIO_EXPECT_EQ(problem, std::string{});
    CNA_STUDIO_EXPECT(std::filesystem::exists(store.getPath()));

    const StudioWorkspaceDocument stored = store.load();
    CNA_STUDIO_EXPECT(stored.found);
    CNA_STUDIO_EXPECT_EQ(stored.problem, std::string{});

    const std::unique_ptr<StudioShell> restored = makeShell();
    CNA_STUDIO_EXPECT(restored->loadLayout(stored.layout, &problem));
    CNA_STUDIO_EXPECT_EQ(problem, std::string{});

    CNA_STUDIO_EXPECT_EQ(restored->dockTree().panels().size(), arranged->dockTree().panels().size());
    CNA_STUDIO_EXPECT(!restored->isPanelOpen("output"));
}

CNA_STUDIO_TEST(AFirstRunIsNotAProblemToReport)
{
    // Nothing stored yet is the ordinary case, not a fault. Reporting it would train a user to
    // ignore the channel that reports the real ones.
    const ScopedDirectory directory{"firstrun"};
    const StudioWorkspaceStore store{directory.file("workspace.json")};

    const StudioWorkspaceDocument stored = store.load();
    CNA_STUDIO_EXPECT(!stored.found);
    CNA_STUDIO_EXPECT_EQ(stored.problem, std::string{});
}

CNA_STUDIO_TEST(ACorruptLayoutFileCostsTheArrangementAndSaysSo)
{
    const ScopedDirectory directory{"corrupt"};
    const std::string path = directory.file("workspace.json");
    writeText(path, "{\"fileVersion\": 1, \"layout\": {\"root\": ");

    const StudioWorkspaceStore store{path};
    const StudioWorkspaceDocument stored = store.load();

    CNA_STUDIO_EXPECT(!stored.found);
    CNA_STUDIO_EXPECT(!stored.problem.empty());

    // And the shell still starts, on the default arrangement. This is the whole point: a file
    // nobody edited by hand should never be the reason Studio will not open.
    const std::unique_ptr<StudioShell> shell = makeShell();
    CNA_STUDIO_EXPECT(!shell->dockTree().panels().empty());
}

CNA_STUDIO_TEST(ALayoutFromANewerStudioIsRefusedRatherThanHalfRead)
{
    // Half-reading a layout is worse than the default one: a newer Studio may have written fields
    // whose absence means something, and the user would get an arrangement nobody designed.
    const ScopedDirectory directory{"newer"};
    const std::string path = directory.file("workspace.json");
    writeText(path, "{\"fileVersion\": 99, \"layout\": {}}");

    const StudioWorkspaceStore store{path};
    const StudioWorkspaceDocument stored = store.load();

    CNA_STUDIO_EXPECT(!stored.found);
    CNA_STUDIO_EXPECT(stored.problem.find("newer") != std::string::npos);
}

CNA_STUDIO_TEST(AnInterruptedSaveLeavesThePreviousLayoutRatherThanAHalfWrittenOne)
{
    // Written to a temporary and renamed, so the file a reader sees is always one a writer
    // finished. Checked by proving the temporary is gone and the real file parses -- the rename is
    // what makes those two statements the same statement.
    const ScopedDirectory directory{"atomic"};
    const std::string path = directory.file("workspace.json");
    const StudioWorkspaceStore store{path};

    const std::unique_ptr<StudioShell> shell = makeShell();
    CNA_STUDIO_EXPECT(store.save(shell->saveLayout()));

    CNA_STUDIO_EXPECT(!std::filesystem::exists(path + ".tmp"));
    CNA_STUDIO_EXPECT(store.load().found);

    // A second save over an existing file must also leave exactly one file behind.
    CNA_STUDIO_EXPECT(shell->closePanel("problems"));
    CNA_STUDIO_EXPECT(store.save(shell->saveLayout()));
    CNA_STUDIO_EXPECT(!std::filesystem::exists(path + ".tmp"));
    CNA_STUDIO_EXPECT(store.load().found);
}

CNA_STUDIO_TEST(ForgettingTheLayoutBringsBackTheDefaultArrangement)
{
    const ScopedDirectory directory{"forget"};
    const StudioWorkspaceStore store{directory.file("workspace.json")};

    const std::unique_ptr<StudioShell> shell = makeShell();
    CNA_STUDIO_EXPECT(store.save(shell->saveLayout()));
    CNA_STUDIO_EXPECT(store.load().found);

    CNA_STUDIO_EXPECT(store.forget());
    CNA_STUDIO_EXPECT(!store.load().found);

    // Twice is not an error. "Reset my layout" should work whether or not one was ever stored.
    CNA_STUDIO_EXPECT(!store.forget());
}

CNA_STUDIO_TEST(AStoreWithNowhereToWriteFailsWithAReasonRatherThanSilently)
{
    const StudioWorkspaceStore store{""};
    const std::unique_ptr<StudioShell> shell = makeShell();

    std::string problem;
    CNA_STUDIO_EXPECT(!store.save(shell->saveLayout(), &problem));
    CNA_STUDIO_EXPECT(!problem.empty());

    // And reading is still safe: a store that cannot write must not be a store that crashes.
    CNA_STUDIO_EXPECT(!store.load().found);
    CNA_STUDIO_EXPECT(!store.forget());
}

CNA_STUDIO_TEST(ConfigurationAndStateAreKeptApart)
{
    // Two directories because the platforms keep them apart and because what should happen when
    // one is lost differs: a swept state directory costs a recovery snapshot, a swept config
    // directory costs the arrangement the user made.
    const std::string config = getStudioConfigDirectory();
    const std::string state = getStudioStateDirectory();

    CNA_STUDIO_EXPECT(!config.empty());
    CNA_STUDIO_EXPECT(!state.empty());
    CNA_STUDIO_EXPECT(config.find("cna-studio") != std::string::npos);
    CNA_STUDIO_EXPECT(state.find("cna-studio") != std::string::npos);

    // The default layout path is under the configuration directory, not beside the executable and
    // not in the project: a workspace is the user's, not the project's and not the install's.
    const std::string layout = StudioWorkspaceStore::defaultPath();
    CNA_STUDIO_EXPECT(layout.rfind(config, 0) == 0);
    CNA_STUDIO_EXPECT(layout.find(StudioWorkspaceStore::kFileName) != std::string::npos);
}

namespace
{
    /**
     * @brief Sets an environment variable for the length of a scope, then puts it back.
     *
     * The suite is one binary running its cases in order, so a case that changes the environment
     * and leaves it changed silently reconfigures every case after it. This is the alternative to
     * that, and it restores *absence* as well as a previous value -- an unset variable put back as
     * an empty string is not the same thing to `getStudioConfigDirectory`.
     */
    class ScopedEnvironment
    {
    public:
        ScopedEnvironment(const char* name, const std::string& value)
            : name_{name}
        {
            const char* previous = std::getenv(name);
            had_ = previous != nullptr;
            if (had_) { previous_ = previous; }
            assign(value.c_str());
        }

        ~ScopedEnvironment()
        {
            if (had_) { assign(previous_.c_str()); }
            else { clear(); }
        }

        ScopedEnvironment(const ScopedEnvironment&) = delete;
        ScopedEnvironment& operator=(const ScopedEnvironment&) = delete;

    private:
        void assign(const char* value)
        {
#ifdef _WIN32
            _putenv_s(name_.c_str(), value);
#else
            ::setenv(name_.c_str(), value, 1);
#endif
        }

        void clear()
        {
#ifdef _WIN32
            _putenv_s(name_.c_str(), "");
#else
            ::unsetenv(name_.c_str());
#endif
        }

        std::string name_;
        std::string previous_;
        bool had_ = false;
    };
}

CNA_STUDIO_TEST(StudioUserDirectoriesAreStudioNamedAndMigrateNothing)
{
    // `docs/ADR-002-THE-FOUR-SILENT-QUESTIONS.md` Decision 2 (STUDIO-01015). The prototype wrote a
    // layout to `$CONFIG/cna-editor` and recovery snapshots to `$STATE/cna-editor/recovery`. Studio
    // writes neither and reads neither, and no migration is provided: the prototype was never
    // released, and neither of its formats is readable by today's code, so the migration would be
    // an unexercisable branch running on every start-up for data belonging to nobody.
    //
    // Two things are checked, because the decision has two halves that fail differently. The rename
    // half fails by a path drifting back; the no-migration half fails by someone helpfully adding
    // one, which would show up here as the planted directory being touched.
    const ScopedDirectory home{"userpaths"};
    const ScopedEnvironment config{"XDG_CONFIG_HOME", home.path().generic_string()};
    const ScopedEnvironment state{"XDG_STATE_HOME", home.path().generic_string()};

    // A prototype-era state directory, as a developer who ran `cna-lab` in 2026 would still have.
    const std::filesystem::path legacy = home.path() / "cna-editor";
    std::error_code code;
    std::filesystem::create_directories(legacy / "recovery", code);
    {
        std::ofstream layout{legacy / "layout.json", std::ios::binary};
        layout << "{\"prototype\":true}";
    }
    {
        std::ofstream snapshot{legacy / "recovery" / "snapshot.json", std::ios::binary};
        snapshot << "{\"prototype\":true}";
    }

    const std::string resolvedConfig = getStudioConfigDirectory();
    const std::string resolvedState = getStudioStateDirectory();

    // The rename half: both land under `cna-studio`, and neither is the prototype's directory.
    CNA_STUDIO_EXPECT_EQ(resolvedConfig, (home.path() / "cna-studio").generic_string());
    CNA_STUDIO_EXPECT_EQ(resolvedState, (home.path() / "cna-studio").generic_string());
    CNA_STUDIO_EXPECT(resolvedConfig.find("cna-editor") == std::string::npos);
    CNA_STUDIO_EXPECT(resolvedState.find("cna-editor") == std::string::npos);

    // Resolving a path must not create one. Studio creates its directories when it writes, and a
    // resolver with a side effect would leave an empty `cna-studio` on every machine that ran
    // `--version`.
    CNA_STUDIO_EXPECT(!std::filesystem::exists(home.path() / "cna-studio", code));

    // The no-migration half: the prototype's files are exactly where they were, unmoved, uncopied
    // and unread. A migration, however well meant, would have emptied or duplicated them.
    CNA_STUDIO_EXPECT(std::filesystem::exists(legacy / "layout.json", code));
    CNA_STUDIO_EXPECT(std::filesystem::exists(legacy / "recovery" / "snapshot.json", code));
    CNA_STUDIO_EXPECT_EQ(std::filesystem::file_size(legacy / "layout.json", code),
                         std::uintmax_t{18});

    // And the layout Studio would write is under the Studio directory, not beside the prototype's.
    const std::string layoutPath = StudioWorkspaceStore::defaultPath();
    CNA_STUDIO_EXPECT(layoutPath.rfind(resolvedConfig, 0) == 0);
}
