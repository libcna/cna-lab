// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Project/BuildDiagnostics.hpp
 * @brief Turning a build log back into the errors that are in it.
 *
 * `plan.md` CORE-01 (`STUDIO-17010`, `STUDIO-17011`).
 *
 * A build that fails with nothing but *Build failed* is worse than no build button: it has taken
 * the one operation a developer performs most and made it less informative than the terminal they
 * would otherwise have used. Core step 9 is that a compiler error arrives as a row naming a file,
 * a line and a message, and that activating the row puts a cursor there.
 *
 * ### What is parsed, and what is deliberately not
 *
 * Three shapes, because between them they are every compiler a CNA project is built with:
 *
 * | Producer | Shape |
 * |----------|-------|
 * | GCC, Clang | `path:line:column: error: message` — the column is optional |
 * | MSVC | `path(line,column): error C2065: message` — the column is optional |
 * | CMake | `CMake Error at path:line (command):` with the message on the following lines |
 *
 * Nothing here tries to be a compiler front end. A note that belongs to the error above it is
 * attached to it rather than promoted to a row of its own, because a list where one mistake
 * produces nine rows is a list nobody reads; anything else in the log stays in the log, which
 * `CORE-01` requires to be reachable whole and unparsed whatever this file made of it.
 *
 * ### Why the log and not the pipe
 *
 * `BuildProcess` writes its children's output to a file, so a build that failed an hour ago is
 * still explainable and the file can be opened in whatever the developer reads logs with. Parsing
 * that file rather than intercepting the stream keeps those properties and makes this function
 * pure: a string in, rows out, testable against a recorded log with no compiler present.
 */

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace CNA::Studio
{
    /** @brief How serious a build diagnostic is. */
    enum class BuildDiagnosticSeverity
    {
        /** @brief The build did not produce its output. */
        Error,
        /** @brief The build produced its output and said something about it. */
        Warning,
        /** @brief Context for the diagnostic above it, e.g. a template instantiation. */
        Note
    };

    /** @brief Returns the stable lower-case name of a severity. */
    [[nodiscard]] std::string_view buildDiagnosticSeverityName(BuildDiagnosticSeverity severity);

    /** @brief One thing a compiler said about one place in the source. */
    struct BuildDiagnostic
    {
        BuildDiagnosticSeverity severity = BuildDiagnosticSeverity::Error;

        /**
         * @brief The file, exactly as the compiler wrote it.
         *
         * Absolute for most toolchains and relative to the build directory for some, which is why
         * it is not normalised here: `studioExternalEditorCommand` resolves it against the
         * project's source directory and the project root, and it needs what the compiler said.
         */
        std::string file;

        /** @brief 1-based line, or zero when the producer named none. */
        int line = 0;

        /** @brief 1-based column, or zero. */
        int column = 0;

        /** @brief What was said, without the file, line or severity. */
        std::string message;

        /**
         * @brief Notes that belong to this diagnostic, in the order the compiler emitted them.
         *
         * Attached rather than listed separately: one mistake in a template can produce nine
         * notes, and a list where the error is the tenth row is a list a user scrolls past.
         */
        std::vector<BuildDiagnostic> notes;

        /** @brief One line naming the place and the message, for a row. */
        [[nodiscard]] std::string toRowText() const;
    };

    /** @brief What a build log turned out to contain. */
    struct BuildDiagnostics
    {
        /** @brief Errors and warnings, in the order the compilers produced them. */
        std::vector<BuildDiagnostic> entries;

        /** @brief How many entries are errors. */
        [[nodiscard]] std::size_t errorCount() const;

        /** @brief How many entries are warnings. */
        [[nodiscard]] std::size_t warningCount() const;

        /** @brief Whether anything was found. */
        [[nodiscard]] bool empty() const { return entries.empty(); }
    };

    /**
     * @brief Reads every diagnostic a build log contains.
     *
     * @param log The log's text, however many lines of it the caller has.
     * @return What was found. An empty result means the parser recognised nothing, which is not
     *         the same as a build that produced nothing -- the unparsed log is still the record.
     */
    [[nodiscard]] BuildDiagnostics studioParseBuildDiagnostics(std::string_view log);
}
