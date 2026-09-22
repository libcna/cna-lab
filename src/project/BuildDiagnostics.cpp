// SPDX-License-Identifier: MS-PL
/**
 * @file BuildDiagnostics.cpp
 * @brief Parses compiler and CMake diagnostics out of a build log (`plan.md` CORE-01).
 */

#include "CNA/Studio/Project/BuildDiagnostics.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace CNA::Studio
{
    namespace
    {
        /** @brief Trims ASCII whitespace, including the carriage return a Windows log carries. */
        std::string_view trimmed(std::string_view text)
        {
            const auto space = [](char c) {
                return c == ' ' || c == '\t' || c == '\r' || c == '\n';
            };
            while (!text.empty() && space(text.front())) { text.remove_prefix(1); }
            while (!text.empty() && space(text.back())) { text.remove_suffix(1); }
            return text;
        }

        /** @brief Reads a run of digits, returning false when there is not one. */
        bool readNumber(std::string_view text, int& out)
        {
            if (text.empty()) { return false; }

            int value = 0;
            for (const char c : text)
            {
                if (std::isdigit(static_cast<unsigned char>(c)) == 0) { return false; }

                // Bounded rather than wrapped. A log with a twelve-digit line number is a corrupt
                // log, and a negative line silently handed to an editor is worse than none.
                if (value > 100000000) { return false; }
                value = value * 10 + (c - '0');
            }
            out = value;
            return true;
        }

        /** @brief Maps a severity word to a severity; false for a word that is neither. */
        bool readSeverity(std::string_view word, BuildDiagnosticSeverity& out)
        {
            std::string key;
            key.reserve(word.size());
            for (const char c : word)
            {
                key += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }

            if (key == "error" || key == "fatal error") { out = BuildDiagnosticSeverity::Error; return true; }
            if (key == "warning") { out = BuildDiagnosticSeverity::Warning; return true; }
            if (key == "note" || key == "remark") { out = BuildDiagnosticSeverity::Note; return true; }
            return false;
        }

        /**
         * @brief Splits `path:line:column: severity: message` — GCC and Clang.
         *
         * The colons make this fiddlier than it looks: a Windows path starts `C:\`, so the scan
         * cannot simply take the first colon, and it cannot take the last either because the
         * message contains colons of its own. It walks colons from the left, and accepts the first
         * split whose remainder parses as *number, optional number, severity word* -- which is the
         * only arrangement the format allows.
         */
        bool parseColumnStyle(std::string_view line, BuildDiagnostic& out)
        {
            for (std::size_t at = line.find(':'); at != std::string_view::npos;
                 at = line.find(':', at + 1))
            {
                // A drive letter, not a separator: `C:\src\main.cpp:12:5: error: ...`.
                if (at == 1 && line.size() > 2 && (line[2] == '\\' || line[2] == '/')) { continue; }
                if (at == 0) { continue; }

                std::string_view rest = line.substr(at + 1);

                const std::size_t afterLine = rest.find(':');
                if (afterLine == std::string_view::npos) { continue; }

                int lineNumber = 0;
                const std::string_view lineText = rest.substr(0, afterLine);
                if (!readNumber(lineText, lineNumber))
                {
                    // All digits and rejected means out of range, which is a corrupt log rather
                    // than a colon in a path. Carrying on would find the *next* colon and report a
                    // file called `src/Game.cpp:99999999999999` on line 1 -- a row that names a
                    // place no editor can open, which is worse than no row.
                    if (!lineText.empty()
                        && std::all_of(lineText.begin(), lineText.end(), [](char c) {
                               return std::isdigit(static_cast<unsigned char>(c)) != 0;
                           }))
                    {
                        return false;
                    }
                    continue;
                }

                rest = rest.substr(afterLine + 1);

                // The column is optional: GCC emits it, some producers do not.
                int columnNumber = 0;
                const std::size_t afterColumn = rest.find(':');
                if (afterColumn != std::string_view::npos
                    && readNumber(rest.substr(0, afterColumn), columnNumber))
                {
                    rest = rest.substr(afterColumn + 1);
                }

                const std::size_t afterSeverity = rest.find(':');
                if (afterSeverity == std::string_view::npos) { continue; }

                BuildDiagnosticSeverity severity = BuildDiagnosticSeverity::Error;
                if (!readSeverity(trimmed(rest.substr(0, afterSeverity)), severity)) { continue; }

                out.file = std::string{line.substr(0, at)};
                out.line = lineNumber;
                out.column = columnNumber;
                out.severity = severity;
                out.message = std::string{trimmed(rest.substr(afterSeverity + 1))};
                return true;
            }
            return false;
        }

        /** @brief Splits `path(line,column): severity Cnnnn: message` — MSVC. */
        bool parseParenStyle(std::string_view line, BuildDiagnostic& out)
        {
            const std::size_t open = line.find('(');
            if (open == std::string_view::npos || open == 0) { return false; }

            const std::size_t close = line.find(')', open);
            if (close == std::string_view::npos) { return false; }

            std::string_view inside = line.substr(open + 1, close - open - 1);
            const std::size_t comma = inside.find(',');

            int lineNumber = 0;
            int columnNumber = 0;
            if (comma == std::string_view::npos)
            {
                if (!readNumber(inside, lineNumber)) { return false; }
            }
            else
            {
                if (!readNumber(inside.substr(0, comma), lineNumber)) { return false; }
                (void)readNumber(inside.substr(comma + 1), columnNumber);
            }

            std::string_view rest = trimmed(line.substr(close + 1));
            if (rest.empty() || rest.front() != ':') { return false; }
            rest = trimmed(rest.substr(1));

            const std::size_t afterSeverity = rest.find(':');
            if (afterSeverity == std::string_view::npos) { return false; }

            // `error C2065` — the severity is the first word and the code is the second.
            std::string_view head = trimmed(rest.substr(0, afterSeverity));
            const std::size_t space = head.find(' ');
            const std::string_view word = space == std::string_view::npos ? head : head.substr(0, space);

            BuildDiagnosticSeverity severity = BuildDiagnosticSeverity::Error;
            if (!readSeverity(word, severity)) { return false; }

            out.file = std::string{line.substr(0, open)};
            out.line = lineNumber;
            out.column = columnNumber;
            out.severity = severity;

            // The code is kept: `C2065` is what a developer pastes into a search engine, and
            // dropping it would make the row less useful than the log it was read from.
            const std::string code =
                space == std::string_view::npos ? std::string{} : std::string{trimmed(head.substr(space + 1))};
            const std::string body = std::string{trimmed(rest.substr(afterSeverity + 1))};
            out.message = code.empty() ? body : code + ": " + body;
            return true;
        }

        /** @brief Splits `CMake Error at path:line (command):` — the configure step's own errors. */
        bool parseCMakeStyle(std::string_view line, BuildDiagnostic& out)
        {
            BuildDiagnosticSeverity severity = BuildDiagnosticSeverity::Error;
            std::string_view rest;

            if (line.rfind("CMake Error at ", 0) == 0)
            {
                rest = line.substr(std::string_view{"CMake Error at "}.size());
            }
            else if (line.rfind("CMake Warning at ", 0) == 0)
            {
                severity = BuildDiagnosticSeverity::Warning;
                rest = line.substr(std::string_view{"CMake Warning at "}.size());
            }
            else { return false; }

            // `CMakeLists.txt:12 (message):` — the trailing command and colon are decoration.
            std::size_t colon = rest.rfind(':');
            if (colon != std::string_view::npos && colon + 1 == rest.size())
            {
                rest = rest.substr(0, colon);
            }
            rest = trimmed(rest);

            const std::size_t paren = rest.rfind(" (");
            if (paren != std::string_view::npos) { rest = trimmed(rest.substr(0, paren)); }

            colon = rest.rfind(':');
            if (colon == std::string_view::npos) { return false; }

            int lineNumber = 0;
            if (!readNumber(rest.substr(colon + 1), lineNumber)) { return false; }

            out.file = std::string{rest.substr(0, colon)};
            out.line = lineNumber;
            out.column = 0;
            out.severity = severity;

            // Deliberately empty: CMake puts the message on the lines that follow, and the caller
            // fills it in from them. A row reading "CMake Error at CMakeLists.txt:12" and nothing
            // else would name the place and not the problem.
            out.message.clear();
            return true;
        }
    }

    std::string_view buildDiagnosticSeverityName(BuildDiagnosticSeverity severity)
    {
        switch (severity)
        {
            case BuildDiagnosticSeverity::Error: return "error";
            case BuildDiagnosticSeverity::Warning: return "warning";
            case BuildDiagnosticSeverity::Note: return "note";
        }
        return "error";
    }

    std::string BuildDiagnostic::toRowText() const
    {
        std::string text = file;
        if (line > 0) { text += ":" + std::to_string(line); }
        if (column > 0) { text += ":" + std::to_string(column); }
        if (!message.empty()) { text += ": " + message; }
        return text;
    }

    std::size_t BuildDiagnostics::errorCount() const
    {
        return static_cast<std::size_t>(
            std::count_if(entries.begin(), entries.end(), [](const BuildDiagnostic& entry) {
                return entry.severity == BuildDiagnosticSeverity::Error;
            }));
    }

    std::size_t BuildDiagnostics::warningCount() const
    {
        return static_cast<std::size_t>(
            std::count_if(entries.begin(), entries.end(), [](const BuildDiagnostic& entry) {
                return entry.severity == BuildDiagnosticSeverity::Warning;
            }));
    }

    BuildDiagnostics studioParseBuildDiagnostics(std::string_view log)
    {
        BuildDiagnostics result;

        // Set while a CMake diagnostic is collecting the indented lines that carry its message.
        bool collectingCMakeMessage = false;

        std::size_t start = 0;
        while (start <= log.size())
        {
            const std::size_t end = log.find('\n', start);
            const std::string_view raw =
                log.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
            start = end == std::string_view::npos ? log.size() + 1 : end + 1;

            const std::string_view line = trimmed(raw);

            if (collectingCMakeMessage)
            {
                // CMake indents the message under the location and ends it with a blank line.
                if (!line.empty() && raw.size() > line.size() && !result.entries.empty())
                {
                    std::string& message = result.entries.back().message;
                    if (!message.empty()) { message += ' '; }
                    message += line;
                    continue;
                }
                collectingCMakeMessage = false;
                if (line.empty()) { continue; }
            }

            if (line.empty()) { continue; }

            BuildDiagnostic entry;
            if (parseCMakeStyle(line, entry))
            {
                result.entries.push_back(std::move(entry));
                collectingCMakeMessage = true;
                continue;
            }

            if (!parseColumnStyle(line, entry) && !parseParenStyle(line, entry)) { continue; }

            if (entry.severity == BuildDiagnosticSeverity::Note && !result.entries.empty())
            {
                // Attached to the diagnostic it explains. A note with nothing above it is the
                // only kind that stands alone, and it is rare enough to be listed as itself.
                result.entries.back().notes.push_back(std::move(entry));
                continue;
            }

            result.entries.push_back(std::move(entry));
        }

        // A CMake error whose message never arrived still names a place, and a row with a place
        // and no message is more useful than no row -- but it has to say something.
        for (BuildDiagnostic& entry : result.entries)
        {
            if (entry.message.empty()) { entry.message = "see the build log"; }
        }
        return result;
    }
}
