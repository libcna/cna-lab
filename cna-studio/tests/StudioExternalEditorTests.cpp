// SPDX-License-Identifier: MS-PL
/**
 * @file StudioExternalEditorTests.cpp
 * @brief Handing the project, or one file at one line, back to the developer's IDE.
 *
 * `plan.md` CORE-02 (`STUDIO-15010`).
 *
 * Every case here checks the *planned* command rather than a launched one. That is not a
 * compromise: it is the only way this can be tested at all on the machines this suite runs on,
 * which have no IDE installed and no desktop session, and it is the layer every defect this
 * feature can have lives at. A line flag in the wrong position, a path resolved against the wrong
 * root, a refusal that does not say what to configure -- all of them are visible in the argv, and
 * none of them is visible in "a process started".
 *
 * The one thing a planning test cannot check is that `fork`/`exec` works, and that is shared with
 * `studioRevealInFileManager`, which has been launching processes since `STUDIO-09011`.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Project/LanguageAdapter.hpp"
#include "CNA/Studio/Project/Cpp/CppLanguage.hpp"
#include "CNA/Studio/Project/StudioExternalEditor.hpp"
#include "CNA/Studio/UiCore/StudioActionRegistry.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    /**
     * @brief A project tree with a `Source/` directory, and fake editors on the `PATH`.
     *
     * The editors are empty executable files. Nothing runs them -- every case stops at the planned
     * command -- but `studioFindExecutable` insists on a file it could execute, which is the check
     * that produces the "could not be found" refusal, so they have to be real enough to pass it.
     */
    class Fixture
    {
    public:
        Fixture()
        {
            root_ = std::filesystem::temp_directory_path()
                  / ("cna-studio-externaleditor-" + std::to_string(counter()++));
            std::error_code code;
            std::filesystem::remove_all(root_, code);

            project_ = root_ / "Game";
            bin_ = root_ / "bin";
            std::filesystem::create_directories(project_ / "Source", code);
            std::filesystem::create_directories(project_ / "Assets", code);
            std::filesystem::create_directories(bin_, code);

            write(project_ / "Source" / "Main.cpp", "int main() { return 0; }\n");

            // The same name in both places, so a case can prove which one wins rather than
            // assuming the only candidate that exists is the right one.
            write(project_ / "Source" / "Shared.hpp", "// the source one\n");
            write(project_ / "Shared.hpp", "// the root one\n");

            previousPath_ = readPath();
            setPath(bin_.string() + separator() + previousPath_);
        }

        ~Fixture()
        {
            setPath(previousPath_);
            std::error_code code;
            std::filesystem::remove_all(root_, code);
        }

        Fixture(const Fixture&) = delete;
        Fixture& operator=(const Fixture&) = delete;

        /** @brief Creates an executable called @p name on the fixture's PATH. */
        std::string editor(const std::string& name)
        {
            const std::filesystem::path path = bin_ / name;
            write(path, "#!/bin/sh\nexit 0\n");
            std::error_code code;
            std::filesystem::permissions(path,
                                         std::filesystem::perms::owner_all
                                             | std::filesystem::perms::group_exec
                                             | std::filesystem::perms::others_exec,
                                         std::filesystem::perm_options::add, code);
            return name;
        }

        [[nodiscard]] std::string projectRoot() const { return project_.generic_string(); }
        [[nodiscard]] std::string path(const char* relative) const
        {
            return (project_ / relative).generic_string();
        }
        [[nodiscard]] const std::filesystem::path& binaries() const { return bin_; }

    private:
        static void write(const std::filesystem::path& path, const std::string& text)
        {
            std::ofstream stream{path, std::ios::binary};
            stream << text;
        }

        static char separator()
        {
#if defined(_WIN32)
            return ';';
#else
            return ':';
#endif
        }

        static std::string readPath()
        {
            const char* value = std::getenv("PATH");
            return value != nullptr ? std::string{value} : std::string{};
        }

        static void setPath(const std::string& value)
        {
#if defined(_WIN32)
            _putenv_s("PATH", value.c_str());
#else
            ::setenv("PATH", value.c_str(), 1);
#endif
        }

        static int& counter() { static int value = 0; return value; }

        std::filesystem::path root_;
        std::filesystem::path project_;
        std::filesystem::path bin_;
        std::string previousPath_;
    };

    /** @brief The C++ descriptor, which is what a real request carries. */
    const StudioLanguageDescriptor& cppDescriptor()
    {
        static const StudioLanguageRegistry registry = studioBuiltInLanguages();
        static const StudioLanguageDescriptor descriptor =
            registry.find(kCppLanguageId)->descriptor();
        return descriptor;
    }

    /** @brief Joins an argv, so a failure prints the command rather than a size mismatch. */
    std::string joined(const std::vector<std::string>& argv)
    {
        std::string text;
        for (const std::string& value : argv)
        {
            if (!text.empty()) { text += ' '; }
            text += value;
        }
        return text;
    }
}

CNA_STUDIO_TEST(AnUnsetExternalEditorIsRefusedAtTheGestureAndSaysWhatToConfigure)
{
    // The case every new installation hits: `StudioPreferences::externalEditor` has existed since
    // the prototype, defaults to empty, and had no caller at all before CORE-02. A menu row that
    // silently did nothing here would be the worst outcome, because it is indistinguishable from
    // an IDE that opened behind Studio's window.
    Fixture fixture;

    StudioExternalEditorRequest request;
    request.projectRoot = fixture.projectRoot();

    const StudioExternalEditorCommand command =
        studioExternalEditorCommand(request, &cppDescriptor());

    CNA_STUDIO_EXPECT(!command.isValid());
    CNA_STUDIO_EXPECT(command.argv.empty());

    // Not merely non-empty: it has to name the setting and where it lives, or the user is told
    // that something is wrong and not what to do about it.
    CNA_STUDIO_EXPECT(command.refusal.find("Preferences") != std::string::npos);
    CNA_STUDIO_EXPECT(command.refusal.find("Tools") != std::string::npos);

    // And the launching half refuses with the same sentence rather than a second one.
    std::string problem;
    CNA_STUDIO_EXPECT(!studioOpenInExternalEditor(request, &cppDescriptor(), &problem));
    CNA_STUDIO_EXPECT_EQ(problem, command.refusal);
}

CNA_STUDIO_TEST(AnEditorThatCannotBeLaunchedIsRefusedBeforeAnythingIsStarted)
{
    Fixture fixture;

    StudioExternalEditorRequest request;
    request.editor = "an-editor-nobody-has";
    request.projectRoot = fixture.projectRoot();

    const StudioExternalEditorCommand command =
        studioExternalEditorCommand(request, &cppDescriptor());

    CNA_STUDIO_EXPECT(!command.isValid());
    CNA_STUDIO_EXPECT(command.refusal.find("an-editor-nobody-has") != std::string::npos);
    CNA_STUDIO_EXPECT(command.refusal.find("Preferences") != std::string::npos);

    // A path that exists and is not executable is the other half of the same refusal, and the one
    // a user reaches by pointing the preference at a README instead of at a binary.
    StudioExternalEditorRequest byPath = request;
    byPath.editor = fixture.path("Source/Main.cpp");
    CNA_STUDIO_EXPECT(!studioExternalEditorCommand(byPath, &cppDescriptor()).isValid());
}

CNA_STUDIO_TEST(OpeningTheProjectHandsOverItsDirectory)
{
    // Core step 11. Every editor in the table takes a directory and treats it as a project or a
    // workspace, which is what "continue this in CLion" means.
    Fixture fixture;
    const std::string editor = fixture.editor("clion");

    StudioExternalEditorRequest request;
    request.editor = editor;
    request.projectRoot = fixture.projectRoot();

    const StudioExternalEditorCommand command =
        studioExternalEditorCommand(request, &cppDescriptor());

    CNA_STUDIO_EXPECT(command.isValid());
    CNA_STUDIO_EXPECT_EQ(command.resolvedPath, fixture.projectRoot());
    CNA_STUDIO_EXPECT_EQ(command.argv.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(command.argv[1], fixture.projectRoot());

    // No line, because a directory has none -- and the flag would be handed to the editor as a
    // filename if it were added anyway.
    CNA_STUDIO_EXPECT(!command.opensAtTheLine);
}

CNA_STUDIO_TEST(ARelativeSourceFileResolvesThroughTheLanguagesSourceDirectory)
{
    // The acceptance names this specifically: source locations resolve through
    // `StudioLanguageDescriptor::sourceDirectory`. It is also what makes a compiler's own relative
    // path work unchanged when CORE-01 hands one over.
    Fixture fixture;

    StudioExternalEditorRequest request;
    request.editor = fixture.editor("clion");
    request.projectRoot = fixture.projectRoot();
    request.file = "Main.cpp";

    const StudioExternalEditorCommand command =
        studioExternalEditorCommand(request, &cppDescriptor());

    CNA_STUDIO_EXPECT(command.isValid());
    CNA_STUDIO_EXPECT_EQ(command.resolvedPath, fixture.path("Source/Main.cpp"));
}

CNA_STUDIO_TEST(TheSourceDirectoryWinsOverTheProjectRootForTheSameName)
{
    // A name that exists in both is far more likely to be the source file than a sibling of the
    // .cnaproject, and a resolver that took whichever it looked at first would be one whose answer
    // depended on the order two `if`s happened to be written in.
    Fixture fixture;

    StudioExternalEditorRequest request;
    request.editor = fixture.editor("clion");
    request.projectRoot = fixture.projectRoot();
    request.file = "Shared.hpp";

    CNA_STUDIO_EXPECT_EQ(studioExternalEditorCommand(request, &cppDescriptor()).resolvedPath,
                         fixture.path("Source/Shared.hpp"));

    // With no language there is no source directory to prefer, and the root is the only answer --
    // which a `.cnaproject` naming a language this build does not implement will actually hit.
    CNA_STUDIO_EXPECT_EQ(studioExternalEditorCommand(request, nullptr).resolvedPath,
                         fixture.path("Shared.hpp"));
}

CNA_STUDIO_TEST(AnAbsolutePathIsOpenedAsGivenAndAMissingOneIsRefused)
{
    Fixture fixture;

    StudioExternalEditorRequest request;
    request.editor = fixture.editor("clion");
    request.projectRoot = fixture.projectRoot();
    request.file = fixture.path("Source/Main.cpp");

    CNA_STUDIO_EXPECT_EQ(studioExternalEditorCommand(request, &cppDescriptor()).resolvedPath,
                         fixture.path("Source/Main.cpp"));

    // A file that exists nowhere is refused rather than handed over: every editor in the table
    // would cheerfully open an empty buffer under that name, and the developer would spend a
    // minute wondering why their code had vanished.
    request.file = "NoSuchFile.cpp";
    const StudioExternalEditorCommand missing =
        studioExternalEditorCommand(request, &cppDescriptor());
    CNA_STUDIO_EXPECT(!missing.isValid());
    CNA_STUDIO_EXPECT(missing.refusal.find("NoSuchFile.cpp") != std::string::npos);
    CNA_STUDIO_EXPECT(missing.refusal.find("Source") != std::string::npos);
}

CNA_STUDIO_TEST(WithNoOpenProjectThereIsNothingToHandOver)
{
    Fixture fixture;

    StudioExternalEditorRequest request;
    request.editor = fixture.editor("clion");

    CNA_STUDIO_EXPECT(!studioExternalEditorCommand(request, &cppDescriptor()).isValid());

    request.projectRoot = fixture.path("NotADirectory");
    CNA_STUDIO_EXPECT(!studioExternalEditorCommand(request, &cppDescriptor()).isValid());
}

CNA_STUDIO_TEST(EachKnownEditorIsToldAboutTheLineInItsOwnSyntax)
{
    // The whole reason the table exists. These four spellings mean the same thing and no two of
    // them are interchangeable; passing `+42` to `code` opens a file called `+42`, and passing
    // `--line` to vim does the same. Each row here was read from that editor's own documentation,
    // and each is the kind of detail that is obvious once and never again.
    Fixture fixture;

    struct Expectation
    {
        const char* editor;
        std::vector<std::string> tail;
    };

    const std::string main = fixture.path("Source/Main.cpp");
    const std::vector<Expectation> expectations = {
        {"clion", {"--line", "42", main}},
        {"idea", {"--line", "42", main}},
        {"code", {"-g", main + ":42"}},
        {"codium", {"-g", main + ":42"}},
        {"subl", {main + ":42"}},
        {"vim", {"+42", main}},
        {"nvim", {"+42", main}},
        {"emacs", {"+42", main}},
        {"kate", {"-l", "42", main}},
    };

    for (const Expectation& expectation : expectations)
    {
        StudioExternalEditorRequest request;
        request.editor = fixture.editor(expectation.editor);
        request.projectRoot = fixture.projectRoot();
        request.file = "Main.cpp";
        request.line = 42;

        const StudioExternalEditorCommand command =
            studioExternalEditorCommand(request, &cppDescriptor());

        if (!command.isValid())
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{expectation.editor} + " was refused: " + command.refusal);
            continue;
        }

        std::vector<std::string> tail{command.argv.begin() + 1, command.argv.end()};
        if (tail != expectation.tail)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{expectation.editor} + " is asked for line 42 as '" + joined(tail)
                + "', expected '" + joined(expectation.tail) + "'.");
        }
        CNA_STUDIO_EXPECT(command.opensAtTheLine);
    }
}

CNA_STUDIO_TEST(AnUnknownEditorStillOpensTheFileAndDoesNotClaimTheLine)
{
    // The honest failure. An editor whose syntax is not in the table gets the file and nothing
    // else: guessing a flag would turn "opened at the wrong line" into "refused to open at all",
    // and claiming the line anyway would make the caller's own message a lie.
    Fixture fixture;

    StudioExternalEditorRequest request;
    request.editor = fixture.editor("my-own-editor");
    request.projectRoot = fixture.projectRoot();
    request.file = "Main.cpp";
    request.line = 42;

    const StudioExternalEditorCommand command =
        studioExternalEditorCommand(request, &cppDescriptor());

    CNA_STUDIO_EXPECT(command.isValid());
    CNA_STUDIO_EXPECT_EQ(command.argv.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(command.argv[1], fixture.path("Source/Main.cpp"));
    CNA_STUDIO_EXPECT(!command.opensAtTheLine);
}

CNA_STUDIO_TEST(TheEditorNameIsMatchedByItsStemSoAFullPathWorksToo)
{
    // A developer who types `/usr/bin/clion` or `/opt/clion/bin/clion.sh` into the preference
    // deserves the same line handling as one who typed `clion`.
    Fixture fixture;
    fixture.editor("clion");

    StudioExternalEditorRequest request;
    request.editor = (fixture.binaries() / "clion").string();
    request.projectRoot = fixture.projectRoot();
    request.file = "Main.cpp";
    request.line = 7;

    const StudioExternalEditorCommand command =
        studioExternalEditorCommand(request, &cppDescriptor());

    CNA_STUDIO_EXPECT(command.isValid());
    CNA_STUDIO_EXPECT(command.opensAtTheLine);
    CNA_STUDIO_EXPECT_EQ(command.argv[1], std::string{"--line"});
}

CNA_STUDIO_TEST(ALineIsIgnoredWhereThereIsNoFileToPutItIn)
{
    Fixture fixture;

    StudioExternalEditorRequest request;
    request.editor = fixture.editor("clion");
    request.projectRoot = fixture.projectRoot();
    request.line = 42;

    const StudioExternalEditorCommand command =
        studioExternalEditorCommand(request, &cppDescriptor());

    CNA_STUDIO_EXPECT(command.isValid());
    CNA_STUDIO_EXPECT_EQ(command.argv.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(!command.opensAtTheLine);
}

CNA_STUDIO_TEST(TheCppAdapterNamesTheEntryPointItActuallyScaffolds)
{
    // `studio.tools.openSourceInEditor` opens whatever the language calls its entry point, and the
    // C++ adapter's answer has to be the file its own scaffolding writes -- `Source/Main.cpp`, as
    // `CppProjectExport` emits it. A descriptor naming a file the scaffolding does not create
    // would be a menu row that refuses on every project made by the Hub.
    Fixture fixture;
    CNA_STUDIO_EXPECT_EQ(cppDescriptor().entryPointFile, std::string{"Main.cpp"});

    StudioExternalEditorRequest request;
    request.editor = fixture.editor("clion");
    request.projectRoot = fixture.projectRoot();
    request.file = cppDescriptor().entryPointFile;

    CNA_STUDIO_EXPECT_EQ(studioExternalEditorCommand(request, &cppDescriptor()).resolvedPath,
                         fixture.path("Source/Main.cpp"));
}

CNA_STUDIO_TEST(TheHandOffCommandsAreRegisteredAndReachableFromTheToolsMenu)
{
    // An action the registry carries and no menu names is one a user cannot reach, whatever its
    // handler does -- the failure `STUDIO-07003` was closed on. The Tools menu had been registered
    // empty since the shell was written, so these are the first rows it has ever had.
    StudioActionRegistry registry;
    registerCoreStudioActions(registry);

    std::set<std::string> named;
    const auto collect = [&named](const auto& self,
                                  const std::vector<StudioMenuEntry>& entries) -> void {
        for (const StudioMenuEntry& entry : entries)
        {
            if (!entry.rows.empty()) { self(self, entry.rows); }
            if (entry.id.empty() || entry.isSeparator()) { continue; }
            named.insert(entry.id);
        }
    };
    for (const StudioMenuDefinition& menu : StudioShell::defaultMenus())
    {
        collect(collect, menu.entries);
    }

    for (const char* id : {"studio.tools.openProjectInEditor", "studio.tools.openSourceInEditor"})
    {
        const StudioAction* action = registry.find(id);
        if (action == nullptr)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{id} + " is not registered, so nothing can invoke it.");
            continue;
        }
        CNA_STUDIO_EXPECT(action->category == StudioActionCategory::Tools);
        CNA_STUDIO_EXPECT(!action->label.empty());
        if (named.count(id) == 0)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{id} + " is registered and no menu names it.");
        }
    }
}
