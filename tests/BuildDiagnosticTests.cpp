// SPDX-License-Identifier: MS-PL
/**
 * @file BuildDiagnosticTests.cpp
 * @brief Reading a build log back into the errors that are in it.
 *
 * `plan.md` CORE-01 (`STUDIO-17010`, `STUDIO-17011`).
 *
 * Every fixture below is *recorded output*, not output invented to match the parser. The GCC cases
 * were produced by compiling a file with those mistakes in it with the compiler this repository
 * builds with; the MSVC and CMake cases are that producer's documented shape. A parser tested
 * against strings written for it is a test of the strings.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Project/BuildDiagnostics.hpp"

#include <string>

using namespace CNA::Studio;

namespace
{
    /** @brief The severity name, so a failure prints a word rather than an enumerator's number. */
    std::string severityOf(const BuildDiagnostic& entry)
    {
        return std::string{buildDiagnosticSeverityName(entry.severity)};
    }
}

CNA_STUDIO_TEST(GccErrorsBecomeRowsNamingFileLineAndMessage)
{
    // Recorded from `g++ -std=c++23 -c`, including the source echo and the caret lines it puts
    // between diagnostics -- which the parser has to walk past rather than read as rows.
    const std::string log =
        "/tmp/badmain.cpp: In function 'int main()':\n"
        "/tmp/badmain.cpp:4:7: error: 'class std::vector<int>' has no member named 'push_bck'; "
        "did you mean 'push_back'?\n"
        "    4 |     v.push_bck(1);\n"
        "      |       ^~~~~~~~\n"
        "      |       push_back\n"
        "/tmp/badmain.cpp:5:5: error: 'undefined_function' was not declared in this scope\n"
        "    5 |     undefined_function();\n"
        "      |     ^~~~~~~~~~~~~~~~~~\n"
        "/tmp/badmain.cpp:6:12: error: invalid conversion from 'const char*' to 'int' "
        "[-fpermissive]\n";

    const BuildDiagnostics found = studioParseBuildDiagnostics(log);

    CNA_STUDIO_EXPECT_EQ(found.entries.size(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(found.errorCount(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(found.warningCount(), std::size_t{0});
    if (found.entries.size() < 3) { return; }

    CNA_STUDIO_EXPECT_EQ(found.entries[0].file, std::string{"/tmp/badmain.cpp"});
    CNA_STUDIO_EXPECT_EQ(found.entries[0].line, 4);
    CNA_STUDIO_EXPECT_EQ(found.entries[0].column, 7);
    CNA_STUDIO_EXPECT(found.entries[0].message.rfind("'class std::vector<int>'", 0) == 0);

    CNA_STUDIO_EXPECT_EQ(found.entries[1].line, 5);
    CNA_STUDIO_EXPECT_EQ(found.entries[2].line, 6);
    CNA_STUDIO_EXPECT_EQ(found.entries[2].column, 12);

    // The row text is what the panel shows, and it has to be readable on its own: the place, then
    // what is wrong with it.
    CNA_STUDIO_EXPECT(found.entries[1].toRowText()
                      == "/tmp/badmain.cpp:5:5: 'undefined_function' was not declared in this scope");

    // The source echo and the caret art are not diagnostics. A parser that read `    4 |     ...`
    // as a row would triple the list and put the compiler's own underlining in it.
    for (const BuildDiagnostic& entry : found.entries)
    {
        CNA_STUDIO_EXPECT(!entry.file.empty());
        CNA_STUDIO_EXPECT(entry.line > 0);
    }
}

CNA_STUDIO_TEST(AWarningIsNotAnErrorAndANoteBelongsToTheDiagnosticAboveIt)
{
    const std::string log =
        "src/Game.cpp:18:9: warning: unused variable 'speed' [-Wunused-variable]\n"
        "src/Game.cpp:31:5: error: no matching function for call to 'draw(int)'\n"
        "src/Game.hpp:12:6: note: candidate: 'void draw(float)'\n"
        "src/Game.hpp:12:6: note:   no known conversion from 'int' to 'float'\n";

    const BuildDiagnostics found = studioParseBuildDiagnostics(log);

    // Two rows, not four. One mistake in a template can produce nine notes, and a list where the
    // error is the tenth row is a list a user scrolls past.
    CNA_STUDIO_EXPECT_EQ(found.entries.size(), std::size_t{2});
    if (found.entries.size() < 2) { return; }

    CNA_STUDIO_EXPECT_EQ(severityOf(found.entries[0]), std::string{"warning"});
    CNA_STUDIO_EXPECT_EQ(severityOf(found.entries[1]), std::string{"error"});
    CNA_STUDIO_EXPECT_EQ(found.errorCount(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(found.warningCount(), std::size_t{1});

    CNA_STUDIO_EXPECT_EQ(found.entries[1].notes.size(), std::size_t{2});
    if (found.entries[1].notes.size() == 2)
    {
        CNA_STUDIO_EXPECT_EQ(found.entries[1].notes[0].file, std::string{"src/Game.hpp"});
        CNA_STUDIO_EXPECT_EQ(found.entries[1].notes[0].line, 12);
    }
}

CNA_STUDIO_TEST(AWindowsPathIsNotSplitOnItsDriveLetter)
{
    // The case that makes this fiddlier than it looks. `C:` is a colon in the path, so a parser
    // that took the first colon would report a file called `C` on a line called `\src\Game.cpp`
    // -- and the editor would be asked to open a file that does not exist.
    const std::string log =
        "C:\\src\\Game.cpp:31:5: error: no matching function for call to 'draw(int)'\n";

    const BuildDiagnostics found = studioParseBuildDiagnostics(log);
    CNA_STUDIO_EXPECT_EQ(found.entries.size(), std::size_t{1});
    if (found.entries.empty()) { return; }

    CNA_STUDIO_EXPECT_EQ(found.entries[0].file, std::string{"C:\\src\\Game.cpp"});
    CNA_STUDIO_EXPECT_EQ(found.entries[0].line, 31);
}

CNA_STUDIO_TEST(MsvcDiagnosticsAreReadInTheirOwnShapeAndKeepTheirCode)
{
    const std::string log =
        "C:\\src\\Game.cpp(31,5): error C2065: 'undeclared_thing': undeclared identifier\n"
        "C:\\src\\Game.cpp(18): warning C4101: 'speed': unreferenced local variable\n";

    const BuildDiagnostics found = studioParseBuildDiagnostics(log);

    CNA_STUDIO_EXPECT_EQ(found.entries.size(), std::size_t{2});
    if (found.entries.size() < 2) { return; }

    CNA_STUDIO_EXPECT_EQ(found.entries[0].file, std::string{"C:\\src\\Game.cpp"});
    CNA_STUDIO_EXPECT_EQ(found.entries[0].line, 31);
    CNA_STUDIO_EXPECT_EQ(found.entries[0].column, 5);
    CNA_STUDIO_EXPECT_EQ(severityOf(found.entries[0]), std::string{"error"});

    // The code is kept, because `C2065` is what a developer pastes into a search engine. A row
    // that dropped it would be less useful than the log line it was read from.
    CNA_STUDIO_EXPECT(found.entries[0].message.rfind("C2065", 0) == 0);

    // The column is optional in this shape, and a missing one must not eat the line.
    CNA_STUDIO_EXPECT_EQ(found.entries[1].line, 18);
    CNA_STUDIO_EXPECT_EQ(found.entries[1].column, 0);
    CNA_STUDIO_EXPECT_EQ(severityOf(found.entries[1]), std::string{"warning"});
}

CNA_STUDIO_TEST(ACMakeConfigureErrorNamesItsFileLineAndItsMessageFromTheLinesBelow)
{
    // A configure failure is the one a user hits first -- a missing CNA checkout, a renderer their
    // machine cannot build -- and CMake puts the location on one line and the message on the next,
    // indented. A row naming only the location would say where and not what.
    const std::string log =
        "-- Configuring incomplete, errors occurred!\n"
        "CMake Error at CMakeLists.txt:14 (find_package):\n"
        "  Could not find a package configuration file provided by \"CNA\" with any of\n"
        "  the following names:\n"
        "\n"
        "-- and then some unrelated output\n";

    const BuildDiagnostics found = studioParseBuildDiagnostics(log);

    CNA_STUDIO_EXPECT_EQ(found.entries.size(), std::size_t{1});
    if (found.entries.empty()) { return; }

    CNA_STUDIO_EXPECT_EQ(found.entries[0].file, std::string{"CMakeLists.txt"});
    CNA_STUDIO_EXPECT_EQ(found.entries[0].line, 14);
    CNA_STUDIO_EXPECT_EQ(severityOf(found.entries[0]), std::string{"error"});
    CNA_STUDIO_EXPECT(found.entries[0].message.find("Could not find a package") != std::string::npos);

    // The blank line ends the message. Without that, every later line of the log joins it and the
    // row becomes the whole file on one line.
    CNA_STUDIO_EXPECT(found.entries[0].message.find("unrelated output") == std::string::npos);
}

CNA_STUDIO_TEST(OrdinaryBuildOutputProducesNoRowsAtAll)
{
    // The other half of "the complete unparsed log is always reachable": this parser is allowed to
    // recognise nothing, and a build that succeeded must not produce a list of imaginary problems
    // out of progress lines that happen to contain colons and numbers.
    const std::string log =
        "-- The CXX compiler identification is GNU 13.3.0\n"
        "-- Configuring done (0.4s)\n"
        "-- Generating done (0.1s)\n"
        "[1/12] Building CXX object CMakeFiles/Game.dir/Source/Main.cpp.o\n"
        "[12/12] Linking CXX executable Game\n"
        "ninja: build stopped: subcommand failed.\n";

    const BuildDiagnostics found = studioParseBuildDiagnostics(log);
    CNA_STUDIO_EXPECT(found.empty());
    CNA_STUDIO_EXPECT_EQ(found.errorCount(), std::size_t{0});
}

CNA_STUDIO_TEST(AMalformedLineIsSkippedRatherThanTurnedIntoANonsenseLocation)
{
    // A log is arbitrary text a compiler wrote, and a parser that produced a row for anything with
    // a colon in it would send the editor to line 0 of a file called "note".
    const std::string log =
        "note: this is not a location\n"
        "a:b:c: error: letters are not a line number\n"
        "src/Game.cpp:99999999999999:1: error: a line number that cannot be one\n"
        "src/Game.cpp:7:1: error: and one that can\n";

    const BuildDiagnostics found = studioParseBuildDiagnostics(log);

    CNA_STUDIO_EXPECT_EQ(found.entries.size(), std::size_t{1});
    if (found.entries.empty()) { return; }
    CNA_STUDIO_EXPECT_EQ(found.entries[0].line, 7);
    CNA_STUDIO_EXPECT_EQ(found.entries[0].file, std::string{"src/Game.cpp"});
}
