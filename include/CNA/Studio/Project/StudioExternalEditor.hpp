// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Project/StudioExternalEditor.hpp
 * @brief Handing a project, or one source file at one line, back to the developer's own IDE.
 *
 * `plan.md` CORE-02 (`STUDIO-15010`).
 *
 * This is the clearest statement of what CNA Studio is. Studio is a visual companion for CNA
 * development, not a C++ IDE and not a replacement for CLion: `docs/ADR-001-SCOPE-REDUCTION.md`
 * says so, and this file is where that sentence stops being a claim. A developer must be able to
 * leave at any point without losing anything, and leaving means *this gesture* — the project open
 * in the tool they actually write code in.
 *
 * It also carries the other half of `CORE-01`. A compiler error that names a file and a line is
 * only useful if activating it puts a cursor there; otherwise the Problems panel is a prettier way
 * of reading a build log.
 *
 * ### Planning is separated from launching, deliberately
 *
 * @ref studioExternalEditorCommand computes what *would* be run and refuses when it cannot,
 * without starting anything. That is what makes this testable on a machine with no editor
 * installed and no desktop session — which is every machine this project's tests run on. A wrongly
 * quoted path or a line flag in the wrong position would otherwise be found by a user, once, in
 * the middle of debugging something else.
 *
 * ### A refusal happens at the gesture, not after it
 *
 * An unset editor, an editor that is not on the PATH, a file that is not in the project: each is
 * refused *before* anything is launched, and the refusal says what to configure rather than that
 * something went wrong. `CORE-02`'s acceptance requires exactly this, because the failure it
 * replaces — a menu item that appears to work and silently does nothing — is the one that costs a
 * user the most time.
 *
 * ### Why there is a table of editors
 *
 * Every editor spells *open at line N* differently, and there is no convention to fall back on.
 * `code -g file:line`, `clion --line N file`, `vim +N file`, `kate -l N file` — the same intent,
 * four incompatible spellings. The table below holds the ones a CNA developer is actually likely
 * to have configured; anything not in it opens the file without the line, and says so, which is
 * strictly better than passing a flag the editor will treat as a filename and refusing to open at
 * all. It is a lookup table and not an extension point: adding a row is a one-line change, and a
 * registration mechanism for it would be infrastructure for a problem nobody has.
 */

#include <string>
#include <vector>

namespace CNA::Studio
{
    struct StudioLanguageDescriptor;

    /** @brief What Studio is being asked to open, and in what. */
    struct StudioExternalEditorRequest
    {
        /**
         * @brief The editor, from `StudioPreferences::externalEditor`. Empty is refused.
         *
         * A program name to be found on the `PATH` (`clion`, `code`, `vim`) or an absolute path.
         * Deliberately not defaulted to "whatever the system opens a `.cpp` with": on Linux that
         * is as likely to be a text viewer as an IDE, and a developer who has not said which
         * editor they use is better served by being asked than by being surprised.
         */
        std::string editor;

        /** @brief Absolute path of the project root. Required. */
        std::string projectRoot;

        /**
         * @brief What to open, or empty to open the project itself.
         *
         * Either absolute, or relative to the project root or to its language's source directory —
         * which is how a compiler names a file, and how the Problems panel will hand one over.
         */
        std::string file;

        /**
         * @brief 1-based line to place the cursor on. Zero for none.
         *
         * Honoured only for an editor this file knows the syntax of; otherwise dropped, and
         * @ref StudioExternalEditorCommand::opensAtTheLine says so rather than pretending.
         */
        int line = 0;
    };

    /** @brief What would be run, or why nothing will be. */
    struct StudioExternalEditorCommand
    {
        /** @brief The executable and its arguments. Empty when @ref refusal explains why. */
        std::vector<std::string> argv;

        /**
         * @brief Absolute path of the thing that will be opened: a file, or the project directory.
         *
         * Reported separately from @ref argv so a caller can say *what* it opened without
         * re-deriving it, and so a test can check the resolution without parsing a command line.
         */
        std::string resolvedPath;

        /**
         * @brief Why this will not run, in a sentence naming what to configure. Empty when it will.
         *
         * Phrased for a status bar or a log line, so it starts with the problem rather than with
         * the word "Error".
         */
        std::string refusal;

        /**
         * @brief Whether the cursor will land on @ref StudioExternalEditorRequest::line.
         *
         * False when no line was asked for, and false when the configured editor's syntax for one
         * is not known here. A caller that told the user "opening at line 42" on the strength of a
         * command that drops the line would be lying about something easy to check.
         */
        bool opensAtTheLine = false;

        /** @brief Whether there is anything to run. */
        [[nodiscard]] bool isValid() const { return !argv.empty(); }
    };

    /**
     * @brief Plans the command for @p request without running it.
     *
     * Resolution order for @ref StudioExternalEditorRequest::file, which is the order that makes a
     * compiler's own path work unchanged: absolute as given; then under the language's source
     * directory; then under the project root. The first that exists wins, and a file that exists
     * nowhere is refused rather than passed to an editor that would offer to create it.
     *
     * @param request What to open, and in what.
     * @param language The project's language, for its source directory. Optional: a null one means
     *        the project root is the only place a relative path is looked for.
     * @return The command, or one carrying a refusal and no argv.
     */
    [[nodiscard]] StudioExternalEditorCommand studioExternalEditorCommand(
        const StudioExternalEditorRequest& request,
        const StudioLanguageDescriptor* language = nullptr);

    /**
     * @brief Plans and runs the command for @p request, and does not wait for it.
     *
     * Detached, for the same reason the file manager is: an IDE is a long-lived application the
     * developer is about to work in for hours, and an editor that owned it as a child would be one
     * that closes their IDE when Studio exits.
     *
     * @param request What to open, and in what.
     * @param language The project's language, for its source directory. Optional.
     * @param errorMessage Set to the refusal, or to the launch failure, when the answer is false.
     * @return False when the command was refused or could not be started.
     */
    bool studioOpenInExternalEditor(const StudioExternalEditorRequest& request,
                                    const StudioLanguageDescriptor* language = nullptr,
                                    std::string* errorMessage = nullptr);

    /**
     * @brief The absolute path of @p program if it can be launched, or empty.
     *
     * An absolute or relative path is checked as given; a bare name is looked for on the `PATH`.
     * Exposed because the refusal for an editor that is not installed has to be produced *before*
     * launching, and a caller that wants to grey a menu row out needs the same answer.
     */
    [[nodiscard]] std::string studioFindExecutable(const std::string& program);
}
