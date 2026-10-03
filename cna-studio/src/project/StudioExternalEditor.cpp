// SPDX-License-Identifier: MS-PL
/**
 * @file StudioExternalEditor.cpp
 * @brief Plans and launches the hand-off to the developer's own IDE (`plan.md` CORE-02).
 */

#include "CNA/Studio/Project/StudioExternalEditor.hpp"

#include "CNA/Studio/Project/LanguageAdapter.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#    include <windows.h>
#else
#    include <cerrno>
#    include <cstring>
#    include <sys/wait.h>
#    include <unistd.h>
#endif

namespace CNA::Studio
{
    namespace
    {
#if defined(_WIN32)
        constexpr char kPathSeparator = ';';
#else
        constexpr char kPathSeparator = ':';
#endif

        /** @brief How an editor is told to place the cursor on a line. */
        enum class LineSyntax
        {
            /** @brief `editor +N file` — vim, nvim, gvim, emacs, nano, gedit. */
            PlusLineThenFile,
            /** @brief `editor -g file:N` — VS Code and its forks, which need the `-g` to mean it. */
            GotoFileColonLine,
            /** @brief `editor file:N` — Sublime Text. */
            FileColonLine,
            /** @brief `editor --line N file` — the JetBrains family, CLion among them. */
            DashDashLineThenFile,
            /** @brief `editor -l N file` — kate, kwrite. */
            DashLThenFile,
        };

        /**
         * @brief The editors whose line syntax is known.
         *
         * Matched on the executable's stem, so `/usr/bin/code`, `code` and `C:\...\code.exe` are
         * the same editor. Lower-cased first, because Windows paths are not case sensitive and a
         * developer who typed `Code` should not get a worse result than one who typed `code`.
         *
         * Only editors a CNA developer plausibly has. This is a table, not a registry: `CORE-02`
         * is two hours of work and a discovery protocol for text editors is not in it.
         */
        struct KnownEditor
        {
            const char* stem;
            LineSyntax syntax;
        };

        constexpr KnownEditor kKnownEditors[] = {
            // JetBrains. CLion is the one `plan.md` names, and `idea`, `rider` and the rest take
            // the same switch because they are the same launcher script.
            {"clion", LineSyntax::DashDashLineThenFile},
            {"idea", LineSyntax::DashDashLineThenFile},
            {"rider", LineSyntax::DashDashLineThenFile},
            {"pycharm", LineSyntax::DashDashLineThenFile},
            {"webstorm", LineSyntax::DashDashLineThenFile},
            {"fleet", LineSyntax::DashDashLineThenFile},

            // VS Code and the forks that kept its command line.
            {"code", LineSyntax::GotoFileColonLine},
            {"code-insiders", LineSyntax::GotoFileColonLine},
            {"codium", LineSyntax::GotoFileColonLine},
            {"vscodium", LineSyntax::GotoFileColonLine},
            {"cursor", LineSyntax::GotoFileColonLine},
            {"windsurf", LineSyntax::GotoFileColonLine},

            {"subl", LineSyntax::FileColonLine},
            {"sublime_text", LineSyntax::FileColonLine},

            {"vim", LineSyntax::PlusLineThenFile},
            {"nvim", LineSyntax::PlusLineThenFile},
            {"gvim", LineSyntax::PlusLineThenFile},
            {"vi", LineSyntax::PlusLineThenFile},
            {"emacs", LineSyntax::PlusLineThenFile},
            {"emacsclient", LineSyntax::PlusLineThenFile},
            {"nano", LineSyntax::PlusLineThenFile},
            {"gedit", LineSyntax::PlusLineThenFile},

            {"kate", LineSyntax::DashLThenFile},
            {"kwrite", LineSyntax::DashLThenFile},
        };

        /** @brief Lower-cases ASCII, which is all an executable name needs. */
        std::string lowered(std::string text)
        {
            std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return text;
        }

        /** @brief The line syntax for @p executable, or nothing when it is not a known editor. */
        const KnownEditor* knownEditorFor(const std::string& executable)
        {
            const std::string stem = lowered(std::filesystem::path{executable}.stem().string());
            for (const KnownEditor& candidate : kKnownEditors)
            {
                if (stem == candidate.stem) { return &candidate; }
            }
            return nullptr;
        }

        /** @brief True when @p path is a file this process could plausibly execute. */
        bool isLaunchable(const std::filesystem::path& path)
        {
            std::error_code error;
            if (!std::filesystem::is_regular_file(path, error) || error) { return false; }
#if defined(_WIN32)
            return true;
#else
            return ::access(path.c_str(), X_OK) == 0;
#endif
        }
    }

    std::string studioFindExecutable(const std::string& program)
    {
        if (program.empty()) { return {}; }

        const std::filesystem::path given{program};

        // A name carrying a separator is a path the user meant literally. Searching the PATH for it
        // as well would turn a typo in an absolute path into a different program starting.
        if (given.has_parent_path())
        {
            std::error_code error;
            const std::filesystem::path absolute = std::filesystem::absolute(given, error);
            const std::filesystem::path candidate = error ? given : absolute;
            return isLaunchable(candidate) ? candidate.generic_string() : std::string{};
        }

        const char* path = std::getenv("PATH");
        if (path == nullptr) { return {}; }

#if defined(_WIN32)
        // A bare `code` on Windows is `code.exe`, or `code.cmd` for the editors that ship a shim.
        const std::vector<std::string> suffixes{"", ".exe", ".cmd", ".bat"};
#else
        const std::vector<std::string> suffixes{""};
#endif

        const std::string entries{path};
        std::size_t start = 0;
        while (start <= entries.size())
        {
            const std::size_t end = entries.find(kPathSeparator, start);
            const std::string directory =
                entries.substr(start, end == std::string::npos ? std::string::npos : end - start);

            if (!directory.empty())
            {
                for (const std::string& suffix : suffixes)
                {
                    const std::filesystem::path candidate =
                        std::filesystem::path{directory} / (program + suffix);
                    if (isLaunchable(candidate)) { return candidate.generic_string(); }
                }
            }

            if (end == std::string::npos) { break; }
            start = end + 1;
        }
        return {};
    }

    StudioExternalEditorCommand studioExternalEditorCommand(
        const StudioExternalEditorRequest& request, const StudioLanguageDescriptor* language)
    {
        StudioExternalEditorCommand command;

        const auto refuse = [&command](std::string reason) {
            command.argv.clear();
            command.refusal = std::move(reason);
            return command;
        };

        // The unset case first, and with the most specific message, because it is the one every
        // user hits once: the preference has existed since the prototype and had no caller until
        // CORE-02, so a fresh installation reaches this branch.
        if (request.editor.empty())
        {
            return refuse("No external editor is configured. Set one in Preferences, under Tools");
        }

        std::error_code error;
        if (request.projectRoot.empty()
            || !std::filesystem::is_directory(request.projectRoot, error) || error)
        {
            return refuse("There is no open project to hand over");
        }

        const std::string executable = studioFindExecutable(request.editor);
        if (executable.empty())
        {
            return refuse("The external editor '" + request.editor
                          + "' could not be found. Check the path in Preferences, under Tools");
        }

        const std::filesystem::path root{request.projectRoot};

        if (request.file.empty())
        {
            // Opening the project means opening its directory. Every editor in the table above
            // takes a directory and treats it as a project or a workspace, which is the gesture a
            // developer means by "continue this in CLion".
            command.resolvedPath = root.generic_string();
        }
        else
        {
            const std::filesystem::path asked{request.file};

            // The source directory before the project root, because that is where a compiler's
            // relative path is rooted and because a name that exists in both is far more likely to
            // be the source file than a sibling of the .cnaproject.
            std::vector<std::filesystem::path> candidates;
            if (asked.is_absolute()) { candidates.push_back(asked); }
            else
            {
                if (language != nullptr && !language->sourceDirectory.empty())
                {
                    candidates.push_back(root / language->sourceDirectory / asked);
                }
                candidates.push_back(root / asked);
            }

            for (const std::filesystem::path& candidate : candidates)
            {
                std::error_code exists;
                if (!std::filesystem::is_regular_file(candidate, exists) || exists) { continue; }

                const std::filesystem::path absolute =
                    std::filesystem::weakly_canonical(candidate, exists);
                command.resolvedPath =
                    (exists ? candidate : absolute).lexically_normal().generic_string();
                break;
            }

            if (command.resolvedPath.empty())
            {
                const std::string where =
                    (language != nullptr && !language->sourceDirectory.empty())
                        ? " under " + language->sourceDirectory + " or the project root"
                        : " under the project root";
                return refuse("'" + request.file + "' was not found" + where);
            }
        }

        command.argv.push_back(executable);

        const KnownEditor* known =
            request.line > 0 && !request.file.empty() ? knownEditorFor(executable) : nullptr;
        if (known == nullptr)
        {
            // No line asked for, no file to put one in, or an editor whose syntax is not known.
            // The file still opens; `opensAtTheLine` stays false so the caller does not claim more.
            command.argv.push_back(command.resolvedPath);
            return command;
        }

        const std::string line = std::to_string(request.line);
        switch (known->syntax)
        {
            case LineSyntax::PlusLineThenFile:
                command.argv.push_back("+" + line);
                command.argv.push_back(command.resolvedPath);
                break;
            case LineSyntax::GotoFileColonLine:
                // Without `-g`, `code file:12` opens a *new file literally called* `file:12`.
                command.argv.push_back("-g");
                command.argv.push_back(command.resolvedPath + ":" + line);
                break;
            case LineSyntax::FileColonLine:
                command.argv.push_back(command.resolvedPath + ":" + line);
                break;
            case LineSyntax::DashDashLineThenFile:
                command.argv.push_back("--line");
                command.argv.push_back(line);
                command.argv.push_back(command.resolvedPath);
                break;
            case LineSyntax::DashLThenFile:
                command.argv.push_back("-l");
                command.argv.push_back(line);
                command.argv.push_back(command.resolvedPath);
                break;
        }
        command.opensAtTheLine = true;
        return command;
    }

    bool studioOpenInExternalEditor(const StudioExternalEditorRequest& request,
                                    const StudioLanguageDescriptor* language,
                                    std::string* errorMessage)
    {
        const auto fail = [errorMessage](std::string reason) {
            if (errorMessage != nullptr) { *errorMessage = std::move(reason); }
            return false;
        };

        const StudioExternalEditorCommand command =
            studioExternalEditorCommand(request, language);
        if (!command.isValid()) { return fail(command.refusal); }

#if defined(_WIN32)
        std::string commandLine;
        for (const std::string& argument : command.argv)
        {
            if (!commandLine.empty()) { commandLine += ' '; }
            commandLine += argument.find(' ') == std::string::npos ? argument
                                                                   : "\"" + argument + "\"";
        }

        STARTUPINFOA startup{};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION process{};

        if (!CreateProcessA(nullptr, commandLine.data(), nullptr, nullptr, FALSE, 0, nullptr,
                            nullptr, &startup, &process))
        {
            return fail("Could not start " + command.argv.front() + " ("
                        + std::to_string(GetLastError()) + ")");
        }

        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return true;
#else
        std::vector<std::string> storage = command.argv;
        std::vector<char*> argv;
        argv.reserve(storage.size() + 1);
        for (std::string& value : storage) { argv.push_back(value.data()); }
        argv.push_back(nullptr);

        // Double-forked, so the IDE is reparented to init and outlives Studio -- the same reason
        // `studioRevealInFileManager` does it, and a stronger one: a developer's IDE closing
        // because they quit Studio would be the opposite of what this gesture is for.
        const pid_t first = ::fork();
        if (first < 0) { return fail("fork failed: " + std::string{std::strerror(errno)}); }

        if (first == 0)
        {
            const pid_t second = ::fork();
            if (second == 0)
            {
                ::execv(argv[0], argv.data());

                // _exit rather than exit: this is a copy of Studio, and running its atexit
                // handlers would flush its buffers twice and could corrupt files it had open.
                ::_exit(127);
            }
            ::_exit(second < 0 ? 127 : 0);
        }

        int status = 0;
        ::waitpid(first, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            return fail("Could not start " + command.argv.front());
        }
        return true;
#endif
    }
}
