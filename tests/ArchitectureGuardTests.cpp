// SPDX-License-Identifier: MS-PL
/**
 * @file ArchitectureGuardTests.cpp
 * @brief Makes the architecture's invariants enforceable by machinery rather than by review.
 *
 * `plan.md` STUDIO-02032, STUDIO-02033, STUDIO-02034, and `docs/ARCHITECTURE.md` §10.
 *
 * Every rule here is one that a comment has never once prevented anyone from breaking. They are
 * checked by scanning the source tree, which is crude but has the property that matters: a
 * violation fails the build on the commit that introduces it, when it is cheap to fix, rather than
 * being found months later by someone wondering why Studio will not build on a new renderer.
 *
 * A scan can produce a false positive -- the word `vkCreateDevice` inside a comment explaining why
 * Studio must not call it, for instance. Each check therefore ignores comments and strings where
 * that matters, and the failure message names the file, the line and the rule, so a genuine
 * exception can be made deliberately rather than by loosening the pattern until it stops
 * complaining.
 */

#include "SourceScan.hpp"
#include "TestHarness.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <cstring>
#include <map>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    using CnaStudioTest::Scan::SourceFile;
    using CnaStudioTest::Scan::collectSources;
    using CnaStudioTest::Scan::lineOf;
    using CnaStudioTest::Scan::sourceRoot;
    using CnaStudioTest::Scan::stripCommentsAndStrings;

    /**
     * @brief Fails the current test for every occurrence of @p needle in Studio's own code.
     *
     * @param subdirectories Directories to scan.
     * @param needle Forbidden text.
     * @param rule Why it is forbidden, included in the failure so the message is actionable.
     * @return Number of violations found.
     */
    std::size_t expectAbsent(const std::vector<std::string>& subdirectories,
                             std::string_view needle, std::string_view rule)
    {
        std::size_t violations = 0;
        for (const SourceFile& file : collectSources(subdirectories))
        {
            const std::string code = stripCommentsAndStrings(file.text);
            std::size_t position = code.find(needle);
            while (position != std::string::npos)
            {
                ++violations;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    file.relativePath + ":" + std::to_string(lineOf(code, position)) + " contains '"
                    + std::string{needle} + "'. " + std::string{rule});
                position = code.find(needle, position + 1);
            }
        }
        return violations;
    }
} // namespace

CNA_STUDIO_TEST(TheGuardScanCanActuallySeeTheSourceTree)
{
    // Every check below silently passes if the scan finds no files, which would make this whole
    // file a very convincing no-op. This is the check that the checks are running.
    CNA_STUDIO_EXPECT(!sourceRoot().empty());
    const std::vector<SourceFile> files = collectSources({"src", "include"});
    CNA_STUDIO_EXPECT(files.size() > 100);
}

CNA_STUDIO_TEST(TheGuardScanIgnoresCommentsAndStrings)
{
    // Proves the stripper works, so a failure below is a real violation and not a comment
    // mentioning the thing it forbids. Without this, the fix people reach for is deleting the
    // explanatory comment.
    const std::string sample =
        "int a; // CNA::Internal::Foo\n"
        "/* CNA::Internal::Bar */\n"
        "const char* s = \"CNA::Internal::Baz\";\n"
        "int CNA_Internal_real;\n";
    const std::string stripped = stripCommentsAndStrings(sample);

    CNA_STUDIO_EXPECT(stripped.find("CNA::Internal") == std::string::npos);
    CNA_STUDIO_EXPECT(stripped.find("CNA_Internal_real") != std::string::npos);
    // Line numbers must survive, or every failure message points at the wrong place.
    CNA_STUDIO_EXPECT_EQ(std::count(stripped.begin(), stripped.end(), '\n'),
                         std::count(sample.begin(), sample.end(), '\n'));
}

CNA_STUDIO_TEST(NoStudioCodeReachesIntoCnaInternals)
{
    // `docs/ARCHITECTURE.md` §1: Studio uses CNA's public API only. Reaching into CNA::Internal
    // would hide a CNA gap instead of reporting it, and the gaps register is the whole point of
    // Studio being one of CNA's largest consumers.
    CNA_STUDIO_EXPECT_EQ(expectAbsent({"src", "include"}, "CNA::Internal",
        "Studio uses CNA's public API only. If CNA cannot do what is needed, file it in "
        "docs/CNA-GAPS.md rather than reaching past the API."), std::size_t{0});
}

CNA_STUDIO_TEST(NoStudioCodeCallsAGraphicsBackendDirectly)
{
    // Backend-specific behaviour belongs in CNA. A direct call here would make Studio need
    // per-renderer source, which is exactly the thing the capability contract exists to avoid.
    const std::vector<std::string> scan{"src", "include"};
    const std::string rule =
        "Backend-specific graphics code belongs in CNA, not in Studio. If Studio needs it, that is "
        "a missing CNA abstraction -- file it in docs/CNA-GAPS.md.";

    std::size_t violations = 0;
    for (const char* symbol : {"vkCreate", "vkCmd", "vkQueue",
                               "ID3D11Device", "ID3D12Device", "IDirect3D",
                               "glGenBuffers", "glDrawArrays", "glDrawElements", "glBindTexture",
                               "MTLDevice", "wgpuDevice"})
    {
        violations += expectAbsent(scan, symbol, rule);
    }
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

CNA_STUDIO_TEST(NoStudioCodeIncludesABackendHeader)
{
    const std::vector<std::string> scan{"src", "include"};
    std::size_t violations = 0;
    for (const char* header : {"<vulkan/", "<d3d11.h>", "<d3d12.h>", "<GL/gl.h>",
                               "<GLES3/", "<Metal/", "<webgpu/"})
    {
        violations += expectAbsent(scan, header,
            "Only CNA may include a graphics backend's headers.");
    }
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

CNA_STUDIO_TEST(OnlyTheTwoCnaLinkedModulesIncludeCnaHeaders)
{
    // Enforced by the build graph already -- a stray include elsewhere fails to link. Stated here
    // as a test so the property is asserted rather than inferred from a linker error, and so the
    // failure names the rule instead of naming a missing symbol.
    //
    // Two modules, since STUDIO-04001 split the UI GPU renderer out of the viewport: they answer
    // different questions (docs/UI-RENDER-PATH.md layers 2 and 3) and were only together by
    // history. Two is still a closed list -- adding a third means editing this line, which is
    // exactly the review this guard is for.
    std::size_t violations = 0;
    for (const SourceFile& file : collectSources({"src", "include"}))
    {
        const bool isCnaLinked = file.relativePath.find("viewport") != std::string::npos
                              || file.relativePath.find("Viewport") != std::string::npos
                              || file.relativePath.find("ui-renderer") != std::string::npos
                              || file.relativePath.find("UiRenderer") != std::string::npos;
        if (isCnaLinked) { continue; }

        const std::string code = stripCommentsAndStrings(file.text);
        for (const char* cnaInclude : {"<Microsoft/Xna/", "\"Microsoft/Xna/",
                                       "<CNA/Graphics/", "<CNA/Platform/"})
        {
            const std::size_t position = code.find(cnaInclude);
            if (position != std::string::npos)
            {
                ++violations;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    file.relativePath + ":" + std::to_string(lineOf(code, position))
                    + " includes a CNA header. Only cna-studio-viewport and cna-studio-ui-renderer "
                      "may link CNA; everything else stays CNA-free so it can be tested with no "
                      "GPU.");
            }
        }
    }
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

CNA_STUDIO_TEST(TheNativeStudioUiHasNoDearImGuiDependency)
{
    // The end state of the UI migration is that production Studio UI does not depend on Dear
    // ImGui at all. STUDIO-07030 deleted the panel implementations that depended on it, and
    // STUDIO-07031 removed the vendored source and the CNA_STUDIO_WITH_IMGUI option that built
    // it -- so there is no longer a narrower "the parts that have been ported" scope to hold this
    // to. It covers the whole tree now, and the build files a dependency could come back through
    // without a single #include appearing anywhere the source scan below looks (STUDIO-07099).
    std::size_t violations = 0;
    for (const SourceFile& file : collectSources({"src", "include"}))
    {
        const std::string code = stripCommentsAndStrings(file.text);
        for (const char* imgui : {"imgui.h", "ImGui::", "ImDrawList", "ImVec2"})
        {
            const std::size_t position = code.find(imgui);
            if (position != std::string::npos)
            {
                ++violations;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    file.relativePath + ":" + std::to_string(lineOf(code, position))
                    + " depends on Dear ImGui. The native Studio UI replaces it and must not "
                      "acquire a dependency on it.");
            }
        }
    }

    // Through CMake as well as through code: an option or a vendored library target can bring
    // the dependency back with no line the source scan above would ever see. Specific build-graph
    // tokens rather than the word "imgui" itself, which also appears in comments recording this
    // history and in `--ui=imgui`, the CLI name STUDIO-07030 deliberately kept as a synonym for
    // headless rendering -- neither of those is the dependency returning.
    for (const char* relative : {"CMakeLists.txt", "tests/CMakeLists.txt"})
    {
        std::ifstream stream{sourceRoot() / relative, std::ios::binary};
        const std::string text{std::istreambuf_iterator<char>{stream},
                               std::istreambuf_iterator<char>{}};
        for (const char* token :
             {"CNA_STUDIO_WITH_IMGUI", "CNA_STUDIO_HAS_IMGUI", "third_party/imgui",
              "cna-studio-imgui-vendor", "cna-studio-ui-imgui"})
        {
            if (text.find(token) != std::string::npos)
            {
                ++violations;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    std::string{relative} + " contains '" + token
                    + "'. The option and the vendored library it built (STUDIO-07031) are gone; "
                      "a build file naming this again is the dependency returning through CMake "
                      "rather than through code.");
            }
        }
    }

    // And the vendored source itself, which a re-add would restore before any build file changed
    // to build it.
    if (std::filesystem::exists(sourceRoot() / "third_party" / "imgui"))
    {
        ++violations;
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "third_party/imgui/ exists. STUDIO-07031 removed it; its return is the dependency "
            "coming back at the source level, ahead of anything that would build it.");
    }

    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

CNA_STUDIO_TEST(ThePlayerDependsOnNoUiRenderBackend)
{
    // `plan.md` STUDIO-04027. The player runs the user's game and draws no editor UI at all, so it
    // has no business linking one of Studio's UI render backends -- and for six phases it linked
    // the classic one, for a single static that answered a different question: which *CNA*
    // renderer the binary was compiled against.
    //
    // That is the conflation `docs/UI-RENDER-PATH.md` exists to name. It matters beyond tidiness:
    // a UI backend is choosable and deletable (this is the task that deletes one), and a game
    // runtime that depends on which one Studio picked is a game runtime that has to be rebuilt
    // when the editor changes its mind. `studioHostCnaRendererName()` answers layer 3 and nothing
    // else, which is why the player can call it and this can forbid the rest.
    const std::size_t violations =
        expectAbsent({"src/player"}, "CnaUiRenderer",
                     "The player draws no editor UI. If it needs which CNA renderer this build "
                     "uses, that is studioHostCnaRendererName() in UiRenderer/StudioHostRenderer.hpp.")
        + expectAbsent({"src/player"}, "StudioModernUiRenderer",
                       "The player draws no editor UI.")
        + expectAbsent({"src/player"}, "StudioUiRenderBackend",
                       "The player draws no editor UI.");
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

CNA_STUDIO_TEST(TheNativeStudioUiIsCnaFree)
{
    // The property that keeps the UI workstream testable: layout, identity, focus and hit-testing
    // are all decided by code that runs in CI with no GPU. Only the pixels need a device.
    std::size_t violations = 0;
    for (const SourceFile& file : collectSources({"src/ui-core", "include/CNA/Studio/UiCore"}))
    {
        const std::string code = stripCommentsAndStrings(file.text);
        for (const char* cna : {"Microsoft/Xna/", "CNA/Graphics/", "CNA/Platform/"})
        {
            const std::size_t position = code.find(cna);
            if (position != std::string::npos)
            {
                ++violations;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    file.relativePath + ":" + std::to_string(lineOf(code, position))
                    + " links CNA. cna-studio-ui-core must stay CNA-free and headless-testable.");
            }
        }
    }
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

/**
 * Every change to the edited scene is a command (`plan.md` STUDIO-13012, D-06).
 *
 * The rule the undo stack rests on. A panel that wrote to the document directly would make one
 * change Ctrl+Z cannot reach -- and a user who finds *one* such change stops trusting undo for all
 * of them, which costs far more than the edit.
 *
 * It was already true when this guard was written: nothing in `src/shell-panels`, `src/ui-core` or
 * `src/viewport` touches `SceneDocument`'s mutating API. That is exactly when a guard is worth
 * adding -- a rule with no violations is cheap to enforce and expensive to restore once it has one.
 *
 * The allow-list is closed and every entry carries its reason, so adding to it is a decision
 * somebody makes in review rather than a pattern that quietly stops complaining.
 */
CNA_STUDIO_TEST(OnlyCommandsChangeTheEditedScene)
{
    struct Allowed
    {
        const char* path;
        const char* why;
    };

    // Matched as a suffix of the relative path, so the separator style of the host does not decide
    // whether the guard works.
    static constexpr Allowed kAllowed[] = {
        {"src/scene/SceneDocument.cpp", "the document's own implementation"},
        {"src/scene/SceneCommands.cpp", "the commands themselves"},
        {"src/scene/PrefabCommands.cpp", "the prefab commands"},
        {"src/scene/Tilemap.cpp", "PaintTilesCommand's read-modify-write of one stroke"},
        {"src/context/PrefabWorkflow.cpp", "CreatePrefabCommand and its undo"},
        {"src/context/StudioContext.cpp",
         "the default camera of a brand-new scene, before there is a document to undo into"},
        {"src/app/Main.cpp",
         "benchmark scenario setup; routing it through the history would be measuring the history"},
        {"src/player/PlayerHost.cpp",
         "the player's own runtime scene, which is not the document being edited"},
    };

    // `clear()` is deliberately not among these: it is too common a method name to scan for
    // textually, and a document cleared outside a command is a whole-file operation rather than an
    // edit -- opening or closing a scene, which is not what undo is for.
    static constexpr const char* kMutators[] = {
        "findEntityForEdit", "addEntity", "removeEntityRecursive", "reparentEntity",
    };

    std::size_t violations = 0;
    std::size_t allowedSeen = 0;

    for (const SourceFile& file : collectSources({"src"}))
    {
        const auto endsWith = [&file](const char* suffix) {
            const std::string tail{suffix};
            return file.relativePath.size() >= tail.size()
                && file.relativePath.compare(file.relativePath.size() - tail.size(), tail.size(),
                                             tail)
                    == 0;
        };

        const Allowed* exemption = nullptr;
        for (const Allowed& entry : kAllowed)
        {
            if (endsWith(entry.path)) { exemption = &entry; }
        }

        const std::string code = stripCommentsAndStrings(file.text);
        bool mutates = false;
        for (const char* mutator : kMutators)
        {
            const std::size_t position = code.find(mutator);
            if (position == std::string::npos) { continue; }
            mutates = true;

            if (exemption != nullptr) { continue; }

            ++violations;
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                file.relativePath + ":" + std::to_string(lineOf(code, position)) + " calls '"
                + std::string{mutator}
                + "' on the scene. Every change to the edited document goes through a "
                  "StudioCommand (ANALYSIS.md D-06), or it is a change Ctrl+Z cannot reach.");
        }

        if (exemption != nullptr && mutates) { ++allowedSeen; }
    }

    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});

    // The allow-list is checked against reality too. An entry whose file no longer mutates anything
    // is an exemption nobody needs, and a list that only ever grows is one that stops being read.
    CNA_STUDIO_EXPECT_EQ(allowedSeen, std::size(kAllowed));
}

CNA_STUDIO_TEST(EverySourceFileCarriesItsLicenceIdentifier)
{
    // A house rule matching CNA's own, and the kind that decays silently without a check.
    std::size_t violations = 0;
    for (const SourceFile& file : collectSources({"src", "include", "tests"}))
    {
        if (file.text.find("SPDX-License-Identifier") == std::string::npos)
        {
            ++violations;
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                file.relativePath + " has no SPDX-License-Identifier header.");
        }
    }
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

CNA_STUDIO_TEST(NoTwoPublicHeadersDeclareTheSameTypeName)
{
    // Everything public lives in one namespace, CNA::Studio, so two headers declaring the same
    // type name is an ODR violation: each translation unit believes whichever definition it saw,
    // and the two disagree about layout.
    //
    // This guard exists because that happened. A second CNA::Studio::StudioCommand -- the undoable
    // document mutation already had the name -- compiled cleanly, linked cleanly, and corrupted
    // memory at run time. It presented as a std::string destructor freeing a pointer into the data
    // segment, in a test hundreds of cases away from either definition, and moved when unrelated
    // code changed the allocation pattern. Nothing about the symptom pointed at the cause.
    //
    // The scan looks for definitions at namespace indentation (four spaces), which is this
    // codebase's convention. Nested types are indented further and are correctly ignored: they are
    // scoped by their enclosing type and cannot collide.
    std::map<std::string, std::string> declaredIn;
    std::size_t violations = 0;

    for (const SourceFile& file : collectSources({"include"}))
    {
        const std::string code = stripCommentsAndStrings(file.text);
        std::size_t lineStart = 0;

        // Names are qualified by their enclosing namespace before being compared. Without this the
        // guard reports CNA::Studio::SceneLoadResult and CNA::Studio::Runtime::SceneLoadResult as
        // a collision, which they are not -- and a guard that cries wolf gets switched off.
        std::string currentNamespace;

        while (lineStart < code.size())
        {
            const std::size_t lineEnd = std::min(code.find('\n', lineStart), code.size());
            const std::string line = code.substr(lineStart, lineEnd - lineStart);
            lineStart = lineEnd + 1;

            if (line.rfind("namespace ", 0) == 0)
            {
                std::size_t end = 10;
                while (end < line.size()
                       && (std::isalnum(static_cast<unsigned char>(line[end])) != 0
                           || line[end] == '_' || line[end] == ':'))
                {
                    ++end;
                }
                const std::string name = line.substr(10, end - 10);
                // An anonymous or extension namespace block re-opening the same scope keeps it.
                if (!name.empty()) { currentNamespace = name; }
                continue;
            }

            if (line.rfind("    ", 0) != 0 || line.size() < 10) { continue; }
            if (line[4] == ' ') { continue; }   // nested: indented deeper

            std::string keyword;
            std::size_t nameStart = 0;
            for (const char* candidate : {"class ", "struct ", "enum class "})
            {
                const std::string prefix = std::string{"    "} + candidate;
                if (line.rfind(prefix, 0) == 0) { keyword = candidate; nameStart = prefix.size(); break; }
            }
            if (keyword.empty()) { continue; }

            std::size_t nameEnd = nameStart;
            while (nameEnd < line.size()
                   && (std::isalnum(static_cast<unsigned char>(line[nameEnd])) != 0
                       || line[nameEnd] == '_'))
            {
                ++nameEnd;
            }
            if (nameEnd == nameStart) { continue; }

            const std::string name = currentNamespace + "::" + line.substr(nameStart, nameEnd - nameStart);

            // A forward declaration repeats a name legitimately; only definitions collide.
            const std::string rest = line.substr(nameEnd);
            if (rest.find(';') != std::string::npos && rest.find('{') == std::string::npos)
            {
                continue;
            }

            const auto existing = declaredIn.find(name);
            if (existing != declaredIn.end() && existing->second != file.relativePath)
            {
                ++violations;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "type '" + name + "' is defined in both " + existing->second + " and "
                    + file.relativePath + ". Both are in namespace CNA::Studio, so this is an ODR "
                      "violation: it compiles, links, and corrupts memory at run time.");
            }
            else
            {
                declaredIn[name] = file.relativePath;
            }
        }
    }

    // The scan must actually have found types, or it passes by finding nothing.
    CNA_STUDIO_EXPECT(declaredIn.size() > 50);
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

// -------------------------------------------------------------------------------------------
// The language seam (STUDIO-02085)
//
// CNA Studio is C++-first and, for a long time yet, C++-only. That is a product decision and this
// guard does not argue with it. What it refuses is the *other* thing, which arrives by accident and
// is not recoverable: C++ welded into the Studio model, one call at a time, until the Project Hub,
// the Build panel and the export command each know what a compiler is and each has to be rewritten
// before a second CNA binding can be authored at all.
//
// The rule is a directory. Everything under `Project/Cpp/` and `src/project/cpp/` is the C++
// language adapter; nothing else in Studio may include one of its headers or name one of its
// symbols. Generic code reaches a project's toolchain through `StudioLanguageAdapter`, which can
// tell it the toolchain's *name* and nothing about how to drive one.
//
// A directory rather than a list of exempt files, deliberately. A list has to be edited when a file
// is added, which means it is edited by whoever is adding the file that breaks the rule.
// -------------------------------------------------------------------------------------------

namespace
{
    /** @brief Whether @p relativePath is part of the C++ language adapter. */
    bool isCppAdapterFile(const std::string& relativePath)
    {
        return relativePath.rfind("include/CNA/Studio/Project/Cpp/", 0) == 0
            || relativePath.rfind("src/project/cpp/", 0) == 0;
    }
}

CNA_STUDIO_TEST(OnlyTheCppLanguageAdapterKnowsHowACppProjectIsBuilt)
{
    std::size_t violations = 0;
    std::size_t adapterFiles = 0;
    std::size_t otherFiles = 0;

    for (const SourceFile& file : collectSources({"src", "include"}))
    {
        if (isCppAdapterFile(file.relativePath)) { ++adapterFiles; continue; }

        // The one exempt site, named rather than pattern-matched. Somewhere has to list the
        // adapters a build ships or nothing is registered, and `studioBuiltInLanguages` is that
        // list and nothing else -- which is why it is a file of its own: an exemption granted to a
        // file that does one thing cannot quietly come to cover a second.
        if (file.relativePath == "src/project/StudioBuiltInLanguages.cpp") { continue; }

        ++otherFiles;

        // The include, from the raw text: an `#include` path is a string literal, so the stripped
        // code the symbol scan below reads has already blanked it. One reference here is enough --
        // a file that cannot include the header cannot name what is in it.
        const std::size_t included = file.text.find("CNA/Studio/Project/Cpp/");
        if (included != std::string::npos)
        {
            ++violations;
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                file.relativePath + ":" + std::to_string(lineOf(file.text, included))
                + " includes a C++ language-adapter header. Whatever it needs, ask "
                  "StudioLanguageAdapter for it: that is what keeps the rest of Studio buildable "
                  "against a CNA binding that is not C++.");
        }

        // And the symbols, in case one is ever reached without the header -- a forward declaration,
        // or a header that grows an include of its own.
        const std::string code = stripCommentsAndStrings(file.text);
        for (const char* symbol : {"findCMake", "makeBuildRequestFromActiveProfile",
                                   "makeBuildRequest", "getDefaultBuildDirectory",
                                   "studioTargetProfileCMakeArguments", "exportStandaloneProject",
                                   "studioCppProjectFiles", "studioRuntimeSources",
                                   "StudioRuntimeSource", "kStudioRuntimeDirectory",
                                   "BuildRequest", "kCppLanguageId"})
        {
            const std::size_t position = code.find(symbol);
            if (position == std::string::npos) { continue; }

            ++violations;
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                file.relativePath + ":" + std::to_string(lineOf(code, position)) + " names '"
                + symbol + "', which belongs to the C++ language adapter. Generic Studio code goes "
                  "through StudioLanguageAdapter.");
        }
    }

    // A scan that found no adapter -- because it moved, or because the prefixes went stale --
    // would report a clean tree by checking one.
    CNA_STUDIO_EXPECT(adapterFiles >= 6);
    CNA_STUDIO_EXPECT(otherFiles >= 100);
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

CNA_STUDIO_TEST(NoStudioCodeBranchesOnWhichLanguageAProjectIsWrittenIn)
{
    // The shape this whole seam exists to prevent:
    //
    //     if (language == "cpp") { ... } else if (language == ...) { ... }
    //
    // spread through code that has no business knowing. One registered adapter owns
    // language-specific behaviour; everything else asks it. The registry's own resolution of an id
    // to an adapter is the one comparison there is, and it is inside the registry.
    std::size_t violations = 0;
    std::size_t scanned = 0;

    for (const SourceFile& file : collectSources({"src", "include"}))
    {
        if (isCppAdapterFile(file.relativePath)) { continue; }
        ++scanned;

        // Raw text: the interesting half of `== "cpp"` is a string literal, which the stripper
        // blanks. Spelled out rather than pattern-matched, because a pattern loose enough to catch
        // every spacing would also catch prose.
        for (const char* comparison : {"== \"cpp\"", "!= \"cpp\"", "== kCppLanguageId",
                                       "!= kCppLanguageId"})
        {
            const std::size_t position = file.text.find(comparison);
            if (position == std::string::npos) { continue; }

            ++violations;
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                file.relativePath + ":" + std::to_string(lineOf(file.text, position))
                + " branches on a language id. Ask the project's adapter what to do instead; a "
                  "chain of these is what makes a second CNA binding a rewrite rather than an "
                  "addition.");
        }
    }

    CNA_STUDIO_EXPECT(scanned >= 100);
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

namespace
{
    /** @brief One responsibility the language seam claims, and where the interface carries it. */
    struct LanguageResponsibility
    {
        /** @brief The boundary, as `docs/ARCHITECTURE.md` §13 names it. */
        const char* boundary;
        /** @brief A declaration that must appear in `LanguageAdapter.hpp`. */
        const char* declaration;
    };

    /**
     * @brief Every boundary the seam was introduced to own.
     *
     * A table rather than prose, for the reason `STUDIO-07003` needed one: a document claiming the
     * seam covers ten things can stop being true without anybody editing it, and the interface is
     * the only place the claim can be checked against. A boundary dropped from the interface fails
     * here; a boundary dropped from the document fails the pairing below.
     */
    const std::vector<LanguageResponsibility>& languageResponsibilities()
    {
        static const std::vector<LanguageResponsibility> responsibilities = {
            {"project language/toolchain identity", "descriptor() const = 0"},
            {"project/template compatibility with a language", "supportsProjectKind("},
            {"creation of source/project files", "projectFiles("},
            {"configure/build commands", "planBuild("},
            {"toolchain availability", "probeToolchain("},
            {"package/export workflow", "exportStandalone("},
            {"standalone build verification", "standaloneBuildInstructions("},
            {"generated code ownership", "generatedDirectory"},
            {"hand-written source ownership", "sourceDirectory"},
            {"gameplay-component metadata", "sourceFileExtensions"},
        };
        return responsibilities;
    }
}

CNA_STUDIO_TEST(TheLanguageSeamStillCarriesEveryBoundaryItWasIntroducedFor)
{
    std::ifstream stream{sourceRoot() / "include" / "CNA" / "Studio" / "Project"
                         / "LanguageAdapter.hpp", std::ios::binary};
    const std::string header{std::istreambuf_iterator<char>{stream},
                             std::istreambuf_iterator<char>{}};
    CNA_STUDIO_EXPECT(!header.empty());

    for (const LanguageResponsibility& responsibility : languageResponsibilities())
    {
        if (header.find(responsibility.declaration) == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"the language seam no longer declares '"} + responsibility.declaration
                + "', so nothing owns '" + responsibility.boundary
                + "'. Either it moved and this table is stale, or it went back into generic code.");
        }
    }

    // And the architecture document lists the same boundaries, so the two cannot drift apart in
    // the direction a test would otherwise not notice: an interface that keeps a method the
    // document has stopped claiming is fine; a document claiming coverage the interface dropped is
    // the failure above, and a boundary in neither is one nobody will remember was considered.
    std::ifstream doc{sourceRoot() / "docs" / "ARCHITECTURE.md", std::ios::binary};
    const std::string architecture{std::istreambuf_iterator<char>{doc},
                                   std::istreambuf_iterator<char>{}};
    CNA_STUDIO_EXPECT(!architecture.empty());

    for (const LanguageResponsibility& responsibility : languageResponsibilities())
    {
        if (architecture.find(responsibility.boundary) == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"docs/ARCHITECTURE.md does not mention the boundary '"}
                + responsibility.boundary + "', which the language seam claims to own.");
        }
    }
}

/**
 * @brief Every field `EffectLighting` carries is read by the one pass that applies it.
 *
 * `plan.md` STUDIO-20004. The reduction is CNA-free and fully tested; *applying* it needs a
 * device, so the model pass cannot be asserted on headlessly and its half of the contract is the
 * half that quietly went missing. `ambientOverridesDefault` was added because exactly that had
 * happened one field over: the scene's ambient was computed correctly, written into the draw, and
 * dropped on the floor by a path that returned early.
 *
 * A source scan rather than a behavioural test, and it is honest about being one: it cannot say
 * the field is applied *correctly*, only that `CnaModelPass` mentions it at all. That is the
 * difference between a field nobody wired up and a field wired up wrongly, and only the first of
 * those is invisible to every other test in the suite.
 */
CNA_STUDIO_TEST(TheModelPassReadsEveryFieldTheLightingReductionFillsIn)
{
    std::ifstream pass{sourceRoot() / "src" / "viewport" / "CnaModelPass.cpp", std::ios::binary};
    const std::string source{std::istreambuf_iterator<char>{pass},
                             std::istreambuf_iterator<char>{}};
    CNA_STUDIO_EXPECT(!source.empty());

    // Named rather than parsed out of the header: a list somebody has to extend when they add a
    // field is a list they extend while the field is still fresh in their mind, and a parser would
    // silently start covering nothing the day the header's formatting changed.
    static const char* const kFields[] = {
        "useDefaultLighting", "ambientColor", "ambientOverridesDefault", "lightCount",
        "hasPunctual", "punctual",
    };

    for (const char* field : kFields)
    {
        if (source.find(field) == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"CnaModelPass.cpp never mentions EffectLighting::"} + field
                + ", so the reduction fills it in and nothing applies it.");
        }
    }

    // And the same question one layer up (`plan.md` STUDIO-20007). `GameView::perspective` is
    // decided over the document and tested there; whether the host *acts* on it is a call into a
    // device, and the defect this row fixed was precisely that nothing acted on the projection at
    // all -- the game view ran the 2D sprite pass whatever the camera said, so a 3D project
    // previewed as its clear colour and nothing else.
    std::ifstream hostFile{sourceRoot() / "src" / "viewport" / "CnaStudioShellHost.cpp",
                           std::ios::binary};
    const std::string host{std::istreambuf_iterator<char>{hostFile},
                           std::istreambuf_iterator<char>{}};
    CNA_STUDIO_EXPECT(!host.empty());

    for (const char* mention : {"perspective", "renderGame3D"})
    {
        if (host.find(mention) == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"CnaStudioShellHost.cpp never mentions '"} + mention
                + "', so a perspective game camera is decided over the document and drawn by "
                  "nothing.");
        }
    }
}

/**
 * @brief The shadow plan the batch carries is what the device half actually renders.
 *
 * `plan.md` STUDIO-20006. The plan itself is arithmetic over a document and is tested against one
 * in `SceneTests.cpp`; what cannot be tested there is whether anything *calls* the device with it,
 * because the call is `ShadowMap::begin` and CI has no device. So this reads the source, exactly
 * as the lighting guard above does, and for the same reason: `castShadows` and `receiveShadows`
 * were editable, serialised, defaulted to true and read by nothing for nineteen phases.
 */
CNA_STUDIO_TEST(TheShadowPlanTheBatchCarriesIsWhatTheDeviceHalfRenders)
{
    std::ifstream passFile{sourceRoot() / "src" / "viewport" / "CnaModelPass.cpp", std::ios::binary};
    const std::string pass{std::istreambuf_iterator<char>{passFile},
                           std::istreambuf_iterator<char>{}};
    CNA_STUDIO_EXPECT(!pass.empty());

    // Generating the map, and attaching it. Both halves, because either alone is a pass that
    // costs a render target per frame and changes no pixel.
    static const char* const kGenerates[] = {
        "ShadowMap", "SupportsShadowSamplingEXT", "begin(", "applyCaster", "castsShadow",
    };
    static const char* const kAttaches[] = {
        "setShadowMapEXT", "setLightViewProjectionEXT", "setShadowsEnabledEXT", "receivesShadow",
    };

    for (const char* mention : kGenerates)
    {
        if (pass.find(mention) == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"CnaModelPass.cpp never mentions '"} + mention
                + "', so the batch carries a shadow plan that nothing renders into a map.");
        }
    }
    for (const char* mention : kAttaches)
    {
        if (pass.find(mention) == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"CnaModelPass.cpp never mentions '"} + mention
                + "', so a shadow map is generated and no draw ever samples it.");
        }
    }

    // And the sky's device half (`plan.md` STUDIO-20005), which cannot be asserted headlessly for
    // the same reason: the calls are `Skybox::draw` and `setImageBasedLightEXT`, and CI has no
    // device. Both halves again, because either alone is a panorama processed for nothing -- a
    // cube generated and never drawn, or a sky on screen that lights nothing while the scene says
    // it should.
    static const char* const kSky[] = {
        "Skybox", "EnvironmentProcessor", "convertEquirectangular", "setImageBasedLightEXT",
        "generateIrradiance", "batch.sky",
    };
    for (const char* mention : kSky)
    {
        if (pass.find(mention) == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"CnaModelPass.cpp never mentions '"} + mention
                + "', so the batch carries a sky that nothing draws or lights with.");
        }
    }

    // **The CNA extensions go through `IShadowReceiverEXT`, never through `PbrEffect`.** Both
    // effects implement that interface -- punctual lights and shadow maps alike -- and this build
    // draws through `BasicEffect` (`kPreferPbrEffect` is false). So reaching an EXT setter through
    // the concrete `pbr` pointer compiles, passes review, and switches the feature off on the only
    // path anybody runs. That is exactly how STUDIO-20003 first shipped, and it is the same
    // mistake as the withdrawn gaps G-13 and G-14: one type's header read as if it were the API.
    //
    // **One exemption, and it is the fact that makes the rule worth stating.**
    // `setImageBasedLightEXT` is declared on `PbrEffect` and `SkinnedPbrEffect` and on *nothing*
    // else -- it is not on `IShadowReceiverEXT`, where the punctual light and the shadow map live.
    // So image-based lighting is genuinely PBR-only, the concrete pointer is the only way to reach
    // it, and a `BasicEffect` build draws the sky without being lit by it (`plan.md`
    // STUDIO-20005). The editor says so through `studioEnvironmentCapabilityIssues` rather than
    // leaving it to be discovered, which is what makes the exemption acceptable rather than a hole.
    static const char* const kPbrOnlySetters[] = {"pbr->setImageBasedLightEXT"};

    if (pass.find("pbr->set") != std::string::npos
        && pass.find("EXT(") != std::string::npos)
    {
        std::size_t at = pass.find("pbr->set");
        while (at != std::string::npos)
        {
            const std::size_t end = pass.find('(', at);
            const bool exempt =
                end != std::string::npos
                && std::any_of(std::begin(kPbrOnlySetters), std::end(kPbrOnlySetters),
                               [&](const char* allowed) {
                                   return pass.compare(at, end - at, allowed) == 0;
                               });

            if (!exempt && end != std::string::npos
                && pass.compare(end - 3, 3, "EXT") == 0)
            {
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "CnaModelPass.cpp reaches a CNA extension through the concrete PbrEffect ('"
                    + pass.substr(at, end - at)
                    + "'). BasicEffect implements IShadowReceiverEXT too, and this build draws "
                      "through BasicEffect, so that call never runs. Use shadowReceiver().");
            }
            at = pass.find("pbr->set", at + 1);
        }
    }

    // And the ordering the whole thing rests on: `ShadowMap::end` restores the *back buffer*, not
    // whatever was bound when the pass started, so the shadow pass has to run before the scene's
    // target is bound. Called after it, the rest of the frame draws into the window -- a defect
    // whose symptom is the editor's own chrome flickering, nowhere near the shadow code.
    std::ifstream rendererFile{sourceRoot() / "src" / "viewport" / "CnaSceneRenderer.cpp",
                               std::ios::binary};
    const std::string renderer{std::istreambuf_iterator<char>{rendererFile},
                               std::istreambuf_iterator<char>{}};
    CNA_STUDIO_EXPECT(!renderer.empty());

    // The sky is drawn *inside* the scene's target and before the models, which is the opposite
    // constraint to the shadow map's and is just as easy to get backwards. `Skybox` draws a
    // fullscreen triangle with no depth configuration, so drawn after the models it would erase
    // them -- a defect whose symptom is a viewport showing only sky.
    const std::size_t skyCall = renderer.find("renderSky");
    const std::size_t modelCall = renderer.find("modelPass.render(models)");
    if (skyCall == std::string::npos)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "CnaSceneRenderer.cpp never calls renderSky, so a scene with an environment map draws "
            "no sky however complete the pass below it is.");
    }
    else if (modelCall != std::string::npos && skyCall > modelCall)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "CnaSceneRenderer.cpp draws the sky after the models. Skybox draws a fullscreen "
            "triangle with no depth test, so it would erase them.");
    }

    const std::size_t shadowCall = renderer.find("renderShadowMap");
    if (shadowCall == std::string::npos)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "CnaSceneRenderer.cpp never calls renderShadowMap, so the 3D view draws no shadows "
            "however complete the pass below it is.");
    }
    else
    {
        const std::size_t bind = renderer.find("SetRenderTarget(impl_->target.get())", shadowCall);
        const std::size_t earlierBind = renderer.rfind("SetRenderTarget(impl_->target.get())",
                                                       shadowCall);
        CNA_STUDIO_EXPECT(bind != std::string::npos);

        // There are earlier binds in this file -- the 2D pass has one -- so "no bind before it"
        // is not the question. The question is whether the *next* thing after the shadow call is
        // the bind rather than the other way round, within the same function, which is what the
        // distance to each side measures.
        const std::size_t forward = bind - shadowCall;
        const std::size_t backward =
            earlierBind == std::string::npos ? forward + 1 : shadowCall - earlierBind;
        if (backward < forward)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "CnaSceneRenderer.cpp calls renderShadowMap after binding the scene's render "
                "target. ShadowMap::end restores the back buffer, so everything drawn afterwards "
                "goes to the window instead of the viewport's texture.");
        }
    }
}

/**
 * @brief Every authored file is written atomically, and the exemptions are named.
 *
 * `plan.md` STUDIO-31003. This rule was already written down and already believed: `RecoveryStore`
 * explains it exactly -- *a half-written recovery file fails to load at the one moment it is
 * needed, having already convinced the user their work was safe* -- and three other files followed
 * it, each with its own copy of the temp-and-rename dance. Every **document** truncated in place.
 * The crash-recovery snapshot was being written safely to protect files that were not.
 *
 * A convention with four adherents and eight holes is not a convention, so the procedure lives in
 * `studioWriteFileAtomically` and this refuses any other `std::ofstream` in `src/`. The exemptions
 * are named here rather than pattern-matched, because a pattern that happened to spare them would
 * spare the next document too.
 */
CNA_STUDIO_TEST(EveryAuthoredFileIsWrittenAtomically)
{
    struct Exemption
    {
        const char* file;
        const char* because;
    };

    // Four, and each is a file that is not a user's document.
    static const Exemption kExemptions[] = {
        {"src/core/StudioFileWrite.cpp", "is the writer itself"},
        {"src/project/BuildRunner.cpp",
         "truncates a build *log* at the start of a build and appends to it; a log is not a "
         "document, and a half-written one costs nothing"},
        {"src/project/ProjectCreation.cpp",
         "writes a zero-byte probe to find out whether a directory is writable, and deletes it "
         "immediately"},
        {"src/app/Main.cpp",
         "generates benchmark scaffolding -- a placeholder bitmap and stub asset files -- which "
         "no user authored and nobody can lose"},
    };

    std::size_t violations = 0;
    for (const SourceFile& file : collectSources({"src"}))
    {
        if (file.text.find("std::ofstream") == std::string::npos) { continue; }

        const bool exempt =
            std::any_of(std::begin(kExemptions), std::end(kExemptions),
                        [&file](const Exemption& allowed) {
                            return file.relativePath.find(allowed.file) != std::string::npos;
                        });
        if (exempt) { continue; }

        ++violations;
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            file.relativePath
            + " opens a std::ofstream. Every authored file goes through "
              "studioWriteFileAtomically (plan.md STUDIO-31003), because truncating a document "
              "empties it before the replacement is written -- and a crash inside that window "
              "loses the user's work. If this one genuinely is not a document, name it in "
              "kExemptions with the reason.");
    }
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});

    // And the exemptions are *live*: one naming a file that no longer writes anything is a licence
    // nobody revoked, which is how an exemption list becomes a way around the rule.
    for (const Exemption& allowed : kExemptions)
    {
        std::ifstream stream{sourceRoot() / allowed.file, std::ios::binary};
        const std::string contents{std::istreambuf_iterator<char>{stream},
                                   std::istreambuf_iterator<char>{}};
        if (contents.find("std::ofstream") == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{allowed.file} + " is exempted from the atomic-write rule because it "
                + allowed.because
                + ", and it no longer opens a std::ofstream at all. Remove the exemption.");
        }
    }

    // **And the writer actually renames a temporary**, which is the property none of the
    // behavioural cases in `StudioFileWriteTests.cpp` can hold. A writer that opened the target
    // directly and then renamed it onto itself passes every one of them: the bytes are right, the
    // folder is clean, the failures are reported. The difference only shows under an interruption
    // a test cannot create, and the permission tricks that would force one are ignored for a
    // process running as root -- which is how CI runs. So this is a source scan, honest about
    // being one, and it is the only thing standing between the rule and a quiet regression.
    {
        std::ifstream stream{sourceRoot() / "src/core/StudioFileWrite.cpp", std::ios::binary};
        const std::string writer{std::istreambuf_iterator<char>{stream},
                                 std::istreambuf_iterator<char>{}};
        CNA_STUDIO_EXPECT(!writer.empty());

        // `temporary +=` is the line that gives it a *different name* from the target, and it is
        // the one a regression removes: a "temporary" that equals the path renames a file onto
        // itself and truncates the document on the way, while still mentioning every other word
        // this guard could look for. Asked for by name because the first version of this check
        // looked for the words and passed that exact break.
        for (const char* step :
             {"std::filesystem::rename", "temporary +=", "nextWriteTicket"})
        {
            if (writer.find(step) == std::string::npos)
            {
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    std::string{"StudioFileWrite.cpp no longer mentions '"} + step
                    + "', so it is not writing a uniquely named temporary and renaming it over "
                      "the target -- which is the whole of what makes the write atomic.");
            }
        }

        // The stream is opened on the *temporary*, never on the caller's path. One character, and
        // it is the entire difference between a save that cannot lose a document and one that can.
        if (writer.find("std::ofstream stream{temporary") == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "StudioFileWrite.cpp does not open its stream on the temporary. Opening the "
                "target directly empties the user's document before the replacement is written, "
                "which is exactly the defect STUDIO-31003 removed.");
        }
    }

    // The writer is reached by the documents that matter, named one at a time. A guard that only
    // refused `std::ofstream` would pass a file that stopped writing anything at all.
    static const char* const kAuthored[] = {
        "src/scene/SceneDocument.cpp", "src/scene/PrefabDocument.cpp",
        "src/assets/AssetCommands.cpp", "src/assets/AssetDatabase.cpp",
        "src/project/Project.cpp", "src/project/RecoveryStore.cpp",
    };
    for (const char* authored : kAuthored)
    {
        std::ifstream stream{sourceRoot() / authored, std::ios::binary};
        const std::string contents{std::istreambuf_iterator<char>{stream},
                                   std::istreambuf_iterator<char>{}};
        if (contents.find("studioWriteFileAtomically") == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{authored}
                + " never calls studioWriteFileAtomically, so whatever it writes can be lost "
                  "halfway through being written.");
        }
    }
}

/**
 * @brief Every file that declares a `formatVersion` runs a migration chain when it reads one back.
 *
 * `plan.md` STUDIO-31005. The rule is stated in `SceneDocument::loadFromJson` and it is the whole
 * reason the chain exists: *"The version gate and the upgrade path are one thing: refusing a file
 * from the future and upgrading one from the past are both answers to 'what version is this?', and
 * splitting them is how a loader comes to refuse a file it could have read."*
 *
 * Three formats had split them. `.cnamaterial`, `.cnaenv` and the `.cnarecovery` envelope each
 * carried a hand-written `if (version > kFormatVersion) return false;` and no upgrade at all — the
 * half that refuses, without the half that reads. Nothing would have shown it: every chain in the
 * tree is empty, so a format with no chain behaves exactly like a format with one right up until
 * the day somebody bumps a version, which is the day the files are already written.
 *
 * So the rule is held structurally. Writing a `formatVersion` into a document is how a file
 * *declares a format*; declaring one and never mentioning `FormatMigrator` means the upgrade path
 * for it does not exist.
 */
CNA_STUDIO_TEST(EveryVersionedFormatRunsAMigrationChain)
{
    struct Exemption
    {
        const char* file;
        const char* because;
    };

    static const Exemption kExemptions[] = {
        {"src/core/FormatMigration.cpp",
         "is the migrator itself, and stamps the version it has just upgraded a document to"},
        {"src/project/RecentProjects.cpp",
         "writes the user's recent-projects list, which is not a document and is rebuilt by "
         "opening projects; a version it cannot read is dropped and the list starts empty, which "
         "costs the user one menu and nothing else"},
    };

    std::size_t violations = 0;
    for (const SourceFile& file : collectSources({"src"}))
    {
        // Writing the key is what declares a format. A file that only *reads* one is a consumer --
        // `Main.cpp` passing a migrator along, a panel showing a number -- and has no format of
        // its own to upgrade.
        if (file.text.find("set(\"formatVersion\"") == std::string::npos) { continue; }

        const bool exempt =
            std::any_of(std::begin(kExemptions), std::end(kExemptions),
                        [&file](const Exemption& allowed) {
                            return file.relativePath.find(allowed.file) != std::string::npos;
                        });
        if (exempt) { continue; }

        if (file.text.find("FormatMigrator") != std::string::npos) { continue; }

        ++violations;
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            file.relativePath
            + " writes a 'formatVersion' and never mentions FormatMigrator, so it declares a "
              "format with no way to upgrade one written by an older build (plan.md "
              "STUDIO-31005). A hand-written `version > kFormatVersion` refusal is the half that "
              "says no; the chain is the half that says yes. If this genuinely is not a document, "
              "name it in kExemptions with the reason.");
    }
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});

    // The exemptions are *live*, for the reason the atomic-write guard gives: one naming a file
    // that no longer declares a format is a licence nobody revoked.
    for (const Exemption& allowed : kExemptions)
    {
        std::ifstream stream{sourceRoot() / allowed.file, std::ios::binary};
        const std::string contents{std::istreambuf_iterator<char>{stream},
                                   std::istreambuf_iterator<char>{}};
        if (contents.find("formatVersion") == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{allowed.file} + " is exempted from the migration-chain rule because it "
                + allowed.because
                + ", and it no longer mentions formatVersion at all. Remove the exemption.");
        }
    }

    // And the loaders actually *run* the chain rather than only naming its type. `migrate(` is the
    // call; a file holding a `const FormatMigrator&` it never asks anything is a format whose
    // upgrade path is a declaration.
    static const char* const kLoaders[] = {
        "src/scene/SceneDocument.cpp",       "src/scene/PrefabDocument.cpp",
        "src/project/Project.cpp",           "src/project/RecoveryStore.cpp",
        "src/assets/AssetDatabase.cpp",      "src/assets/MaterialDocument.cpp",
        "src/assets/EnvironmentMapDocument.cpp",
    };
    for (const char* loader : kLoaders)
    {
        std::ifstream stream{sourceRoot() / loader, std::ios::binary};
        const std::string contents{std::istreambuf_iterator<char>{stream},
                                   std::istreambuf_iterator<char>{}};
        CNA_STUDIO_EXPECT(!contents.empty());

        if (contents.find(".migrate(") == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{loader}
                + " never calls migrate(), so the document it reads is whatever version the file "
                  "claims to be and no upgrade runs on the way in.");
        }
    }
}

namespace
{
    /** @brief Whether the identifier ending at @p at is a mutating method name. */
    [[nodiscard]] bool isMutatorName(std::string_view text, std::size_t at, std::size_t end)
    {
        // `set`, `add`, `remove`, `clear`, `insert`, `erase`, `rename` or `move`, followed by a
        // capital -- `setPosition`, not `settle`. A prefix without the capital is a different word.
        static const char* const kPrefixes[] = {"set",    "add",   "remove", "clear",
                                                "insert", "erase", "rename", "move"};
        const std::string_view name = text.substr(at, end - at);
        for (const char* prefix : kPrefixes)
        {
            const std::size_t length = std::char_traits<char>::length(prefix);
            if (name.size() > length && name.compare(0, length, prefix) == 0
                && name[length] >= 'A' && name[length] <= 'Z')
            {
                return true;
            }
        }
        return false;
    }

    /** @brief The identifier starting at @p from, or an empty view. */
    [[nodiscard]] std::string_view identifierAt(std::string_view text, std::size_t from)
    {
        std::size_t end = from;
        while (end < text.size()
               && (std::isalnum(static_cast<unsigned char>(text[end])) != 0 || text[end] == '_'))
        {
            ++end;
        }
        return text.substr(from, end - from);
    }

    /** @brief Skips spaces, tabs and newlines forward from @p from. */
    [[nodiscard]] std::size_t skipBlank(std::string_view text, std::size_t from)
    {
        while (from < text.size()
               && (text[from] == ' ' || text[from] == '\t' || text[from] == '\n'
                   || text[from] == '\r'))
        {
            ++from;
        }
        return from;
    }

    /** @brief One `receiver.mutator(` found in a file. */
    struct DocumentMutation
    {
        std::string receiver;
        std::string method;
        int line = 0;
    };

    /**
     * @brief Finds mutations of the *open document*, reached through the context's accessors.
     *
     * Two forms, because the defect that motivated this guard used the second. Directly:
     * `context.getScene().addEntity(...)`. Or through a reference bound from an accessor:
     * `Project& project = context_.getProject();` and then `project.setTargetProfiles(...)` — which
     * is what `StudioBuildPanel` did for as long as it existed.
     *
     * A `const` binding is not tracked: it cannot mutate anything, and the Details panel
     * legitimately holds a non-const `Project&` only to hand to a command constructor, which is
     * the compliant path and is not a call on the reference.
     */
    [[nodiscard]] std::vector<DocumentMutation> findDocumentMutations(const std::string& text)
    {
        static const char* const kAccessors[] = {"getScene()", "getProject()", "getAssets()"};
        static const char* const kDocumentTypes[] = {"SceneDocument", "Project", "AssetDatabase"};

        std::vector<DocumentMutation> found;

        // --- Directly off the accessor ----------------------------------------------------------
        for (const char* accessor : kAccessors)
        {
            const std::size_t length = std::char_traits<char>::length(accessor);
            for (std::size_t at = text.find(accessor); at != std::string::npos;
                 at = text.find(accessor, at + 1))
            {
                std::size_t cursor = skipBlank(text, at + length);
                if (cursor >= text.size() || text[cursor] != '.') { continue; }
                cursor = skipBlank(text, cursor + 1);

                const std::string_view method = identifierAt(text, cursor);
                if (method.empty()) { continue; }
                if (skipBlank(text, cursor + method.size()) >= text.size()
                    || text[skipBlank(text, cursor + method.size())] != '(')
                {
                    continue;
                }
                if (!isMutatorName(text, cursor, cursor + method.size())) { continue; }

                found.push_back(DocumentMutation{accessor, std::string{method},
                                                 lineOf(text, at)});
            }
        }

        // --- Through a non-const reference bound from one ----------------------------------------
        std::vector<std::string> bound;
        for (const char* type : kDocumentTypes)
        {
            const std::string pattern = std::string{type} + "&";
            for (std::size_t at = text.find(pattern); at != std::string::npos;
                 at = text.find(pattern, at + 1))
            {
                // `const Project&` cannot mutate, so it is not a binding this cares about.
                if (at >= 6 && text.compare(at - 6, 6, "const ") == 0) { continue; }

                const std::size_t nameAt = skipBlank(text, at + pattern.size());
                const std::string_view name = identifierAt(text, nameAt);
                if (name.empty()) { continue; }

                const std::size_t equals = skipBlank(text, nameAt + name.size());
                if (equals >= text.size() || text[equals] != '=') { continue; }

                // Only when the initialiser is one of the accessors: a reference to a document a
                // function was *given* is that function's business, and the caller is what this
                // guard is about.
                const std::size_t statementEnd = text.find(';', equals);
                if (statementEnd == std::string::npos) { continue; }

                const std::string initialiser = text.substr(equals, statementEnd - equals);
                bool fromAccessor = false;
                for (const char* accessor : kAccessors)
                {
                    if (initialiser.find(accessor) != std::string::npos) { fromAccessor = true; }
                }
                if (fromAccessor) { bound.push_back(std::string{name}); }
            }
        }

        for (const std::string& name : bound)
        {
            for (std::size_t at = text.find(name); at != std::string::npos;
                 at = text.find(name, at + 1))
            {
                // A whole identifier, not a prefix of a longer one.
                if (at > 0
                    && (std::isalnum(static_cast<unsigned char>(text[at - 1])) != 0
                        || text[at - 1] == '_'))
                {
                    continue;
                }
                std::size_t cursor = at + name.size();
                if (cursor < text.size()
                    && (std::isalnum(static_cast<unsigned char>(text[cursor])) != 0
                        || text[cursor] == '_'))
                {
                    continue;
                }

                cursor = skipBlank(text, cursor);
                if (cursor >= text.size() || text[cursor] != '.') { continue; }
                cursor = skipBlank(text, cursor + 1);

                const std::string_view method = identifierAt(text, cursor);
                if (method.empty()) { continue; }
                const std::size_t afterName = skipBlank(text, cursor + method.size());
                if (afterName >= text.size() || text[afterName] != '(') { continue; }
                if (!isMutatorName(text, cursor, cursor + method.size())) { continue; }

                found.push_back(DocumentMutation{name, std::string{method}, lineOf(text, at)});
            }
        }

        return found;
    }
}

/**
 * @brief **Every document mutation goes through a command** (`plan.md` STUDIO-02035, decision D-06).
 *
 * The decision is one sentence — *every document mutation is a command* — and it is what makes undo
 * work at all. An editor where some edits undo and others quietly do not is worse than one where
 * nothing does: the user learns that Ctrl+Z is unreliable and stops trusting it, which costs them
 * the feature everywhere rather than in the one panel that broke it.
 *
 * It was held by review until `STUDIO-17002`, which found `StudioBuildPanel` writing target
 * profiles straight into the open `Project` — no command, no undo entry, and (because nothing else
 * marked the project changed) no save either. It had been there since the panel was written, and
 * nothing would have found it but somebody reading the file.
 *
 * So it is held structurally. Two forms, because the Build panel used the second: a mutating call
 * directly off `getScene()`, `getProject()` or `getAssets()`, and one through a non-const reference
 * bound from one of them.
 *
 * **What it does not claim.** A panel building a *detached* value — a `StudioEntity` that exists
 * only to be handed to a `CreateEntityCommand` — mutates nothing in the document, and the guard
 * does not look at it, correctly: `studio.entity.group` does exactly that. And it scans the
 * consumers rather than the documents, so `src/context`, `src/scene`, `src/project`, `src/assets`
 * and `src/core` are out of scope — those are where commands and documents live, and a command that
 * did not mutate a document would be a command that did nothing.
 */
CNA_STUDIO_TEST(EveryDocumentMutationGoesThroughACommand)
{
    struct Exemption
    {
        const char* file;
        const char* because;
    };

    static const Exemption kExemptions[] = {
        {"src/app/Main.cpp",
         "builds the UI benchmark's scenes -- a thousand entities pushed in to measure drawing, "
         "not a gesture a user made, and nothing a user could undo"},
    };

    // The directories that *consume* documents. The ones holding commands and documents are not
    // scanned, for the reason the doc comment gives.
    static const char* const kConsumers[] = {"src/shell-panels", "src/ui-core", "src/app",
                                             "src/viewport", "src/player"};

    std::size_t scanned = 0;
    std::size_t violations = 0;

    for (const char* directory : kConsumers)
    {
        for (const SourceFile& file : collectSources({directory}))
        {
            ++scanned;

            const bool exempt =
                std::any_of(std::begin(kExemptions), std::end(kExemptions),
                            [&file](const Exemption& allowed) {
                                return file.relativePath.find(allowed.file) != std::string::npos;
                            });
            if (exempt) { continue; }

            // Comments and strings stripped, so a sentence *about* the rule is not a breach of it.
            const std::string code = stripCommentsAndStrings(file.text);
            for (const DocumentMutation& mutation : findDocumentMutations(code))
            {
                ++violations;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    file.relativePath + ":" + std::to_string(mutation.line) + ": '"
                    + mutation.receiver + "." + mutation.method
                    + "' changes the open document directly. Every document mutation goes through "
                      "a command (ANALYSIS.md D-06), or it does not undo -- and an editor whose "
                      "Ctrl+Z works in some panels is one whose Ctrl+Z nobody trusts. Panels "
                      "report; the binder builds the command.");
            }
        }
    }

    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});

    // The scan can see the tree. Every check above passes by finding nothing, so a scan pointed at
    // an empty directory would make all of them vacuous.
    CNA_STUDIO_EXPECT(scanned > 20);

    // **And the detection still detects.** The failure mode this guards against is its own: a
    // pattern that stops matching passes silently and for ever. Checked against text rather than
    // against the tree, so it stays true whatever the tree contains.
    {
        const std::vector<DocumentMutation> direct =
            findDocumentMutations("context.getScene().addEntity(std::move(entity));");
        CNA_STUDIO_EXPECT_EQ(direct.size(), std::size_t{1});

        const std::vector<DocumentMutation> bound = findDocumentMutations(
            "Project& project = context_.getProject();\nproject.setTargetProfiles(profiles);");
        CNA_STUDIO_EXPECT_EQ(bound.size(), std::size_t{1});

        // And does not detect what it must not. A const binding cannot mutate; a read is not a
        // mutation; a detached value is nobody's document; and handing an accessor to a command is
        // the compliant path.
        CNA_STUDIO_EXPECT(findDocumentMutations(
            "const Project& project = context_.getProject();\nproject.getLayers();").empty());
        CNA_STUDIO_EXPECT(findDocumentMutations("context.getScene().findEntity(id);").empty());
        CNA_STUDIO_EXPECT(
            findDocumentMutations("StudioEntity group;\ngroup.setParentId(shared);").empty());
        CNA_STUDIO_EXPECT(findDocumentMutations(
            "context.execute(std::make_unique<CreateEntityCommand>(context.getScene(), e));")
                              .empty());

        // `settle()` is not `setTitle()`: a prefix without the capital is a different word.
        CNA_STUDIO_EXPECT(findDocumentMutations("context.getScene().settled();").empty());
    }

    // The exemptions are live, for the reason the atomic-write guard gives.
    for (const Exemption& allowed : kExemptions)
    {
        std::ifstream stream{sourceRoot() / allowed.file, std::ios::binary};
        const std::string contents{std::istreambuf_iterator<char>{stream},
                                   std::istreambuf_iterator<char>{}};
        if (findDocumentMutations(stripCommentsAndStrings(contents)).empty())
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{allowed.file} + " is exempted from D-06 because it " + allowed.because
                + ", and it no longer changes a document directly at all. Remove the exemption.");
        }
    }
}

/**
 * @brief **No authored document writer reads the clock** (`plan.md` STUDIO-02037).
 *
 * The behavioural half of byte-determinism is in `DeterministicOutputTests.cpp` and it cannot hold
 * this one. A writer that stamped the current time passes "write it twice and compare" whenever
 * both writes land in the same second, which is every run on a fast machine — the case goes green
 * and the user finds out when their colleague's checkout disagrees with theirs. Forcing a real
 * second to pass would put a `sleep` in a suite whose whole doctrine is counted, not timed.
 *
 * So the property is held structurally instead, and this is honest about being a source scan: the
 * files that serialise a user's document must not mention the clock at all.
 *
 * **What is not a leak.** A time that is a *fact about something else* is data, not a stamp. An
 * asset sidecar records its source file's modification time, which is the same on every machine
 * that has the same file and is what makes "has this asset changed?" answerable without hashing it.
 * A recovery snapshot records when it was taken, which is the whole point of a recovery snapshot
 * and is shown to the user in the offer. Both are exempted by name, with their reason, and checked
 * to be live.
 */
CNA_STUDIO_TEST(NoDocumentWriterReadsTheClock)
{
    struct Exemption
    {
        const char* file;
        const char* because;
    };

    static const Exemption kExemptions[] = {
        {"src/assets/AssetDatabase.cpp",
         "records the *source file's* modification time in its sidecar -- a fact about the asset, "
         "the same on every machine that has it, and what makes 'has this changed?' answerable "
         "without hashing the file"},
        {"src/project/RecoveryStore.cpp",
         "records when a snapshot was taken, which is the whole point of a snapshot and is what "
         "the recovery offer shows the user"},
    };

    // Every exemption names a file that is *in* the list below. One naming a file nothing scans is
    // a licence for a rule that was never applied -- which is how an exemption list stops being
    // read. Found by writing this guard with two of them.


    // The files that serialise an authored document. Named one at a time rather than scanned for,
    // because a guard that swept a directory would grow quiet the day somebody added a file to it.
    static const char* const kWriters[] = {
        "src/scene/SceneDocument.cpp",  "src/scene/PrefabDocument.cpp",
        "src/scene/EntityJson.cpp",     "src/project/Project.cpp",
        "src/assets/MaterialDocument.cpp", "src/assets/EnvironmentMapDocument.cpp",
        "src/assets/AssetDatabase.cpp", "src/core/Json.cpp",
        "src/core/PropertyValue.cpp",   "src/core/ComponentDescriptor.cpp",
        "src/project/RecoveryStore.cpp",
    };

    for (const Exemption& allowed : kExemptions)
    {
        const bool scanned = std::any_of(std::begin(kWriters), std::end(kWriters),
                                         [&allowed](const char* writer) {
                                             return std::string_view{writer} == allowed.file;
                                         });
        if (!scanned)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{allowed.file}
                + " is exempted from the no-clock rule but is not one of the writers this scans, "
                  "so the exemption excuses nothing and hides that the file is unchecked.");
        }
    }

    static const char* const kClocks[] = {"std::time(", "system_clock::now", "steady_clock::now",
                                          "std::localtime", "std::gmtime", "std::strftime"};

    for (const char* writer : kWriters)
    {
        const bool exempt =
            std::any_of(std::begin(kExemptions), std::end(kExemptions),
                        [writer](const Exemption& allowed) {
                            return std::string_view{writer} == allowed.file;
                        });
        if (exempt) { continue; }

        std::ifstream stream{sourceRoot() / writer, std::ios::binary};
        const std::string contents{std::istreambuf_iterator<char>{stream},
                                   std::istreambuf_iterator<char>{}};

        // Non-empty, so a renamed file does not turn this into a check of nothing.
        CNA_STUDIO_EXPECT(!contents.empty());

        const std::string code = stripCommentsAndStrings(contents);
        for (const char* clock : kClocks)
        {
            if (code.find(clock) == std::string::npos) { continue; }

            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{writer} + " reads the clock ('" + clock
                + "'). A document that records when it was saved differs from itself on every "
                  "save, so two people who each opened and saved it produce two conflicting "
                  "rewrites of a file neither of them edited (plan.md STUDIO-02037). If the time "
                  "is a fact about something else rather than a stamp, name this file in "
                  "kExemptions with the reason.");
        }
    }

    // The exemptions are live: one naming a file that no longer touches a clock is a licence
    // nobody revoked.
    for (const Exemption& allowed : kExemptions)
    {
        std::ifstream stream{sourceRoot() / allowed.file, std::ios::binary};
        const std::string contents{std::istreambuf_iterator<char>{stream},
                                   std::istreambuf_iterator<char>{}};
        const std::string code = stripCommentsAndStrings(contents);

        bool touchesClock = code.find("last_write_time") != std::string::npos;
        for (const char* clock : kClocks)
        {
            if (code.find(clock) != std::string::npos) { touchesClock = true; }
        }

        if (!touchesClock)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{allowed.file} + " is exempted from the no-clock rule because it "
                + allowed.because + ", and it no longer reads a clock at all. Remove the exemption.");
        }
    }
}

CNA_STUDIO_TEST(NoStudioCodeHardCodesARendererName)
{
    // `docs/ARCHITECTURE.md` §2.2 and the roadmap's rule against hard-coding today's renderer
    // count: classification lives in one place, and scattered `if (name == "vulkan")` comparisons
    // are how adding a CNA renderer becomes an archaeology exercise.
    //
    // The catalogue itself is where the names legitimately live, so it is exempt -- an exemption
    // stated here rather than achieved by writing a pattern that happens not to match it.
    std::size_t violations = 0;
    for (const SourceFile& file : collectSources({"src", "include"}))
    {
        if (file.relativePath.find("RendererCatalog") != std::string::npos) { continue; }

        const std::string code = stripCommentsAndStrings(file.text);
        for (const char* comparison : {"== \"vulkan\"", "== \"easygl\"", "== \"directx11\"",
                                       "== \"opengl33\"", "== \"metal\""})
        {
            const std::size_t position = code.find(comparison);
            if (position != std::string::npos)
            {
                ++violations;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    file.relativePath + ":" + std::to_string(lineOf(code, position))
                    + " compares against a renderer name. Ask the catalogue in "
                      "CNA/Studio/Project/RendererCatalog.hpp instead.");
            }
        }
    }
    CNA_STUDIO_EXPECT_EQ(violations, std::size_t{0});
}

// -------------------------------------------------------------------------------------------
// Plan integrity
// -------------------------------------------------------------------------------------------
//
// The roadmap is only worth reading if its status is true, and the failure mode is not dishonesty
// but drift: a task gets its tick, the phase file's own header and `plan.md`'s table keep the
// number they had, and the discrepancy survives because nobody adds up a column by hand. These
// checks add it up. They assert arithmetic, never judgement -- whether a ✅ is *deserved* is a
// question no test can answer, and pretending otherwise would be worse than not checking.

namespace
{
    /** @brief One row of a phase file's task table. */
    struct PlanTask
    {
        std::string id;
        std::string status;

        /** @brief The ids in the "Depends on" cell. Empty for a row that reads "—". */
        std::vector<std::string> dependsOn;
    };

    /** @brief Reads a whole file, or returns an empty string when it is not there. */
    std::string readFileOrEmpty(const std::filesystem::path& path)
    {
        std::ifstream stream{path, std::ios::binary};
        if (!stream) { return {}; }
        return std::string{std::istreambuf_iterator<char>{stream},
                           std::istreambuf_iterator<char>{}};
    }

    /** @brief Splits text into lines, dropping the line terminators. */
    std::vector<std::string> splitLines(const std::string& text)
    {
        std::vector<std::string> lines;
        std::string current;
        for (const char character : text)
        {
            if (character == '\n') { lines.push_back(current); current.clear(); }
            else if (character != '\r') { current.push_back(character); }
        }
        if (!current.empty()) { lines.push_back(current); }
        return lines;
    }

    /** @brief The cells of a Markdown table row, trimmed, or empty when the line is not one. */
    std::vector<std::string> tableCells(const std::string& line)
    {
        if (line.size() < 2 || line.front() != '|') { return {}; }

        std::vector<std::string> cells;
        std::string current;
        for (std::size_t index = 1; index < line.size(); ++index)
        {
            if (line[index] == '|') { cells.push_back(current); current.clear(); }
            else { current.push_back(line[index]); }
        }

        for (std::string& cell : cells)
        {
            const std::size_t first = cell.find_first_not_of(" \t");
            const std::size_t last = cell.find_last_not_of(" \t");
            cell = (first == std::string::npos) ? std::string{} : cell.substr(first, last - first + 1);
        }
        return cells;
    }

    /** @brief Strips the backticks Markdown uses to set an id in code style. */
    std::string withoutBackticks(std::string text)
    {
        text.erase(std::remove(text.begin(), text.end(), '`'), text.end());
        return text;
    }

    /** @brief @p text without leading or trailing spaces and tabs. */
    std::string trimmed(const std::string& text)
    {
        const std::size_t first = text.find_first_not_of(" \t");
        if (first == std::string::npos) { return {}; }
        const std::size_t last = text.find_last_not_of(" \t");
        return text.substr(first, last - first + 1);
    }

    /** @brief Splits @p text on @p separator, keeping empty pieces. */
    std::vector<std::string> splitOn(const std::string& text, char separator)
    {
        std::vector<std::string> pieces;
        std::string current;
        for (const char character : text)
        {
            if (character == separator) { pieces.push_back(current); current.clear(); }
            else { current.push_back(character); }
        }
        pieces.push_back(current);
        return pieces;
    }

    /** @brief True for exactly `STUDIO-` followed by five digits. */
    bool isPlanTaskId(const std::string& text)
    {
        if (text.rfind("STUDIO-", 0) != 0 || text.size() != 12) { return false; }
        return text.find_first_not_of("0123456789", 7) == std::string::npos;
    }

    /**
     * @brief Reads the task rows of one phase file.
     *
     * A phase file's table is the authority on that phase: `plan.md` summarises it, and the
     * summary is what drifts.
     *
     * @param path The phase file.
     * @return Every `| `STUDIO-NNNNN` | … | status | … |` row, in file order.
     */
    std::vector<PlanTask> readPhaseTasks(const std::filesystem::path& path)
    {
        std::vector<PlanTask> tasks;
        for (const std::string& line : splitLines(readFileOrEmpty(path)))
        {
            const std::vector<std::string> cells = tableCells(line);
            if (cells.size() < 3) { continue; }

            const std::string id = withoutBackticks(cells[0]);
            if (!isPlanTaskId(id)) { continue; }

            // The "Depends on" cell, which is a comma-separated list of backticked ids or an em
            // dash. Anything that is not a well-formed id is dropped rather than reported: the
            // column carries prose in a few rows and a guard that failed on it would be a guard
            // people edit the plan around.
            std::vector<std::string> dependsOn;
            if (cells.size() >= 4)
            {
                for (const std::string& piece : splitOn(cells[3], ','))
                {
                    const std::string dependency = withoutBackticks(trimmed(piece));
                    if (isPlanTaskId(dependency)) { dependsOn.push_back(dependency); }
                }
            }

            tasks.push_back(PlanTask{id, cells[2], std::move(dependsOn)});
        }
        return tasks;
    }
}

CNA_STUDIO_TEST(EveryPhaseFileAgreesWithItsOwnProgressHeader)
{
    // The header line each phase file carries above its table. It is written by hand and read by
    // everyone, which is the worst combination a number can have.
    std::size_t phasesChecked = 0;
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{sourceRoot() / "plans"})
    {
        if (entry.path().extension() != ".md") { continue; }

        const std::string text = readFileOrEmpty(entry.path());
        const std::string relative = "plans/" + entry.path().filename().string();

        const std::vector<PlanTask> tasks = readPhaseTasks(entry.path());
        if (tasks.empty()) { continue; }
        ++phasesChecked;

        const auto complete = static_cast<std::size_t>(
            std::count_if(tasks.begin(), tasks.end(),
                          [](const PlanTask& task) { return task.status == "✅"; }));

        const std::size_t headerStart = text.find("**Progress:** ");
        if (headerStart == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                relative + " has a task table but no '**Progress:** N of M complete' header.");
            continue;
        }

        const std::string header =
            text.substr(headerStart, text.find('\n', headerStart) - headerStart);

        const std::string expected = "**Progress:** " + std::to_string(complete) + " of "
                                   + std::to_string(tasks.size()) + " complete";
        if (header.rfind(expected, 0) != 0)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                relative + " says '" + header + "' but its table holds " + std::to_string(tasks.size())
                + " tasks of which " + std::to_string(complete) + " are ✅. Expected it to start '"
                + expected + "'.");
        }
    }

    // A scan that found no phase files would report perfect agreement.
    CNA_STUDIO_EXPECT(phasesChecked >= 30);
}

CNA_STUDIO_TEST(TheMasterPlanTableAgreesWithEveryPhaseFile)
{
    // `plan.md`'s phase table is a summary of thirty-six files nobody re-reads when ticking a box
    // in one of them, so it is the number most likely to be wrong and the one most likely to be
    // quoted. Each row is checked against the file it links to, and the totals against the rows.
    std::size_t rowsChecked = 0;
    std::size_t totalTasks = 0;
    std::size_t totalComplete = 0;
    std::size_t declaredTotal = 0;

    for (const std::string& line : splitLines(readFileOrEmpty(sourceRoot() / "plan.md")))
    {
        const std::vector<std::string> cells = tableCells(line);

        if (cells.size() >= 2 && cells[0] == "**Total**")
        {
            declaredTotal = static_cast<std::size_t>(
                std::stoul(withoutBackticks(cells[1]).substr(2)));
            continue;
        }

        // | N | [Name](plans/phase-NN-….md) | `STUDIO-NNNNN` | status | tasks | complete | bar |
        if (cells.size() < 6) { continue; }
        const std::size_t linkStart = cells[1].find("(plans/");
        if (linkStart == std::string::npos) { continue; }

        const std::size_t linkEnd = cells[1].find(')', linkStart);
        const std::string relative =
            cells[1].substr(linkStart + 1, linkEnd - linkStart - 1);

        std::size_t declaredTasksInRow = 0;
        std::size_t declaredCompleteInRow = 0;
        try
        {
            declaredTasksInRow = static_cast<std::size_t>(std::stoul(cells[4]));
            declaredCompleteInRow = static_cast<std::size_t>(std::stoul(cells[5]));
        }
        catch (const std::exception&)
        {
            continue;
        }

        ++rowsChecked;
        totalTasks += declaredTasksInRow;
        totalComplete += declaredCompleteInRow;

        const std::vector<PlanTask> tasks = readPhaseTasks(sourceRoot() / relative);
        const auto complete = static_cast<std::size_t>(
            std::count_if(tasks.begin(), tasks.end(),
                          [](const PlanTask& task) { return task.status == "✅"; }));

        if (tasks.size() != declaredTasksInRow || complete != declaredCompleteInRow)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "plan.md says " + relative + " holds " + std::to_string(declaredTasksInRow)
                + " tasks with " + std::to_string(declaredCompleteInRow) + " complete, but the file "
                  "holds " + std::to_string(tasks.size()) + " with " + std::to_string(complete)
                + " complete.");
        }
    }

    CNA_STUDIO_EXPECT(rowsChecked >= 30);
    CNA_STUDIO_EXPECT_EQ(declaredTotal, totalTasks);

    // The headline figure, which is the one that ends up in a commit message or a status report.
    const std::string headline =
        std::to_string(totalComplete) + " of " + std::to_string(totalTasks) + " tasks complete";
    const std::string plan = readFileOrEmpty(sourceRoot() / "plan.md");
    if (plan.find("**" + headline + "**") == std::string::npos)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "plan.md's headline does not read '**" + headline
            + "**', which is what its own phase table adds up to.");
    }
}

/**
 * @brief Each phase row's status marker says what that phase's own task list says.
 *
 * `plan.md` STUDIO-33028. The status column is the first thing anybody reads off that table --
 * it is what a ⬜ beside a finished phase costs, and what an ✅ beside an unfinished one costs
 * more. Every *other* cell in the row was already derived from the phase file and checked here;
 * the status was the one left to a person, and by the time anybody looked seven of thirty-six
 * rows disagreed with the counts printed two cells to their right. Two phases were complete and
 * marked not started; one was complete and marked in progress.
 *
 * The rule is the one the thirty-six rows already agreed on before this was written, read off the
 * twenty-nine that were right:
 *
 * - **✅** when every task is complete or superseded. A superseded task is work that later work
 *   made moot, so it leaves nothing to do -- which is exactly what `STUDIO-07001`'s retirement
 *   established when that status was added.
 * - **⬜** when no task is complete and none is in progress. A deferred or blocked task does not
 *   start a phase: it is work that is still there and still not begun.
 * - **🔄** otherwise, which includes a phase with everything done but one thing blocked. The
 *   blocker is the reason it is not finished, not a reason to call it finished.
 */
CNA_STUDIO_TEST(EveryPhasesStatusMarkerAgreesWithItsOwnTaskList)
{
    std::size_t rowsChecked = 0;

    for (const std::string& line : splitLines(readFileOrEmpty(sourceRoot() / "plan.md")))
    {
        const std::vector<std::string> cells = tableCells(line);

        // | N | [Name](plans/phase-NN-….md) | `STUDIO-NNNNN` | status | tasks | complete | bar |
        if (cells.size() < 4) { continue; }
        const std::size_t linkStart = cells[1].find("(plans/");
        if (linkStart == std::string::npos) { continue; }

        const std::size_t linkEnd = cells[1].find(')', linkStart);
        const std::string relative = cells[1].substr(linkStart + 1, linkEnd - linkStart - 1);

        const std::vector<PlanTask> tasks = readPhaseTasks(sourceRoot() / relative);
        if (tasks.empty()) { continue; }

        std::size_t settled = 0;
        std::size_t started = 0;
        for (const PlanTask& task : tasks)
        {
            if (task.status == "✅") { ++settled; ++started; }
            else if (task.status == "⊘") { ++settled; }
            else if (task.status == "🔄") { ++started; }
        }

        const std::string expected = (settled == tasks.size()) ? "✅"
                                     : (started == 0)          ? "⬜"
                                                               : "🔄";

        ++rowsChecked;
        if (cells[3] != expected)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "plan.md marks " + relative + " '" + cells[3] + "', but its task list is "
                + expected + ".");
        }
    }

    // A scan that matched no rows would agree with everything.
    CNA_STUDIO_EXPECT(rowsChecked >= 30);
}

CNA_STUDIO_TEST(ThePlansStatusBreakdownAddsUpAndMatchesThePhaseFiles)
{
    // Found stale, by five tasks and by a total that did not add up to its own bottom row:
    // 162 + 10 + 310 + 2 + 4 is 488 under a header saying 490. Nothing checked it, because the
    // guard above checks the *phase table* and the headline and stops there -- so this table sat
    // beside a checked one looking exactly as authoritative and being wrong.
    static const std::pair<const char*, const char*> kRows[] = {
        {"✅", "| ✅ Complete | "},
        {"🔄", "| 🔄 In progress | "},
        {"⬜", "| ⬜ Not started | "},
        {"⛔", "| ⛔ Deferred | "},
        {"🔬", "| 🔬 Blocked | "},
        // Added with STUDIO-07001's retirement. A temporary requirement that later work made moot
        // is neither complete nor cancelled, and rounding it to either would be the plan telling a
        // story about itself.
        {"⊘", "| ⊘ Superseded | "},
    };

    std::map<std::string, std::size_t> actual;
    std::size_t totalTasks = 0;
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{sourceRoot() / "plans"})
    {
        if (entry.path().extension() != ".md") { continue; }
        const std::string name = entry.path().filename().string();
        if (name.size() < 8 || name.rfind("phase-", 0) != 0) { continue; }

        for (const PlanTask& task : readPhaseTasks(entry.path()))
        {
            ++actual[task.status];
            ++totalTasks;
        }
    }

    const std::string plan = readFileOrEmpty(sourceRoot() / "plan.md");
    std::size_t declaredSum = 0;
    for (const auto& [symbol, prefix] : kRows)
    {
        const std::size_t at = plan.find(prefix);
        if (at == std::string::npos)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"plan.md has no status row starting '"} + prefix + "'.");
            continue;
        }
        const std::size_t declared =
            static_cast<std::size_t>(std::stoul(plan.substr(at + std::strlen(prefix))));
        declaredSum += declared;
        if (declared != actual[symbol])
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{"plan.md's status breakdown says "} + std::to_string(declared) + " "
                + symbol + " tasks; the phase files hold " + std::to_string(actual[symbol]) + ".");
        }
    }

    // And the column adds up to its own total, which is a separate failure: a breakdown can have
    // every row right and a bottom line that was typed rather than summed.
    CNA_STUDIO_EXPECT_EQ(declaredSum, totalTasks);
    if (plan.find("| **Total** | **" + std::to_string(totalTasks) + "** |") == std::string::npos)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "plan.md's status breakdown does not total " + std::to_string(totalTasks) + ".");
    }
}

CNA_STUDIO_TEST(TheHandoffsOwnArithmeticMatchesThePhaseFiles)
{
    // The handoff is what somebody reads first, and a count in it that is one session stale is
    // worse than no count: it is a number they will quote. plan.md's arithmetic is already checked
    // against the phase files above; this checks the handoff against the same source, so the two
    // cannot say different things about the same day's work.
    //
    // Deliberately only the *numbers*. The prose is a judgement about what was built and no test
    // can hold it to anything -- but a headline saying "140 of 479" while the plan says 159 of 486
    // is a fact, and facts are checkable.
    std::size_t totalTasks = 0;
    std::size_t totalComplete = 0;
    std::map<int, std::pair<std::size_t, std::size_t>> perPhase;

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{sourceRoot() / "plans"})
    {
        if (entry.path().extension() != ".md") { continue; }

        const std::string name = entry.path().filename().string();
        if (name.size() < 8 || name.rfind("phase-", 0) != 0) { continue; }
        const int phase = std::stoi(name.substr(6, 2));

        const std::vector<PlanTask> tasks = readPhaseTasks(entry.path());
        const auto complete = static_cast<std::size_t>(
            std::count_if(tasks.begin(), tasks.end(),
                          [](const PlanTask& task) { return task.status == "✅"; }));

        totalTasks += tasks.size();
        totalComplete += complete;
        perPhase[phase] = {tasks.size(), complete};
    }

    const std::string handoff = readFileOrEmpty(sourceRoot() / "HANDOFF.md");
    CNA_STUDIO_EXPECT(!handoff.empty());

    const std::string headline = "**" + std::to_string(totalComplete) + " of "
                               + std::to_string(totalTasks) + " tasks are complete.**";
    if (handoff.find(headline) == std::string::npos)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "HANDOFF.md's headline does not read '" + headline
            + "', which is what the phase files add up to.");
    }

    // Every "**Phase N — Name** (C of T)" it claims, against what that phase file holds. Only the
    // phases it mentions: the handoff summarises what has been worked on rather than listing all
    // thirty-six, and demanding a line for a phase nobody has started would be noise.
    std::size_t phrasesChecked = 0;
    for (const auto& [phase, counts] : perPhase)
    {
        const std::string marker = "**Phase " + std::to_string(phase) + " \u2014 ";
        std::size_t at = handoff.find(marker);
        if (at == std::string::npos) { continue; }

        const std::size_t open = handoff.find('(', at);
        const std::size_t close = handoff.find(')', open);
        if (open == std::string::npos || close == std::string::npos) { continue; }

        const std::string claim = handoff.substr(open + 1, close - open - 1);
        const std::string expected = std::to_string(counts.second) + " of "
                                   + std::to_string(counts.first);
        ++phrasesChecked;

        if (claim.rfind(expected, 0) != 0)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "HANDOFF.md says Phase " + std::to_string(phase) + " is '" + claim
                + "', but its phase file holds " + expected + ".");
        }
    }

    // A handoff that mentioned no phase at all would pass every check above by saying nothing.
    CNA_STUDIO_EXPECT(phrasesChecked >= 8);
}

CNA_STUDIO_TEST(EveryDependencyNamesARealTaskAndNoneOfThemFormACycle)
{
    // The "Depends on" column is the only thing in the plan that says what order the work can be
    // done in, and until this test existed nothing checked it at all. Two kinds of defect had
    // been sitting in it, both found by hand rather than by anything:
    //
    //   * `STUDIO-03033` depended on `STUDIO-03018`, an id that has never existed. A dependency
    //     naming nothing is not a constraint, it is a sentence that looks like one.
    //   * `STUDIO-10007` and `STUDIO-19001` each depended on the other, as did `STUDIO-10009`
    //     and `STUDIO-21001`. Honoured literally, neither of a pair could ever be started, and
    //     the plan would be telling a reader to wait for something that is waiting for them.
    //
    // Both are invisible to every other guard here: the arithmetic adds up either way, and the
    // ids are unique either way.
    std::map<std::string, std::string> phaseOf;
    std::map<std::string, std::vector<std::string>> dependencies;

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{sourceRoot() / "plans"})
    {
        if (entry.path().extension() != ".md") { continue; }
        const std::string name = entry.path().filename().string();
        if (name.size() < 8 || name.rfind("phase-", 0) != 0) { continue; }

        for (PlanTask& task : readPhaseTasks(entry.path()))
        {
            phaseOf[task.id] = "plans/" + name;
            dependencies[task.id] = std::move(task.dependsOn);
        }
    }

    std::size_t edges = 0;
    for (const auto& [id, needed] : dependencies)
    {
        for (const std::string& dependency : needed)
        {
            ++edges;
            if (phaseOf.find(dependency) == phaseOf.end())
            {
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    id + " in " + phaseOf[id] + " depends on " + dependency
                    + ", which is not a task in any phase file. Either name the right id or "
                      "remove it -- a dependency on nothing constrains nothing.");
            }
        }
    }

    // Depth-first, three colours: grey is on the current path and finding one again is a cycle.
    // Reported once per cycle with the whole loop spelt out, because "A depends on B" on its own
    // is not enough for a reader to see what is wrong with it.
    enum class Mark { Unvisited, OnPath, Done };
    std::map<std::string, Mark> marks;
    std::vector<std::string> path;
    std::size_t cycles = 0;

    const auto walk = [&](auto&& self, const std::string& id) -> void {
        marks[id] = Mark::OnPath;
        path.push_back(id);

        for (const std::string& dependency : dependencies[id])
        {
            if (phaseOf.find(dependency) == phaseOf.end()) { continue; }

            const Mark mark = marks[dependency];
            if (mark == Mark::OnPath)
            {
                std::string loop;
                const auto start = std::find(path.begin(), path.end(), dependency);
                for (auto step = start; step != path.end(); ++step)
                {
                    loop += *step + " depends on ";
                }
                ++cycles;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "dependency cycle: " + loop + dependency
                    + ". One of these edges points the wrong way -- neither task can start while "
                      "both are honoured.");
            }
            else if (mark == Mark::Unvisited)
            {
                self(self, dependency);
            }
        }

        path.pop_back();
        marks[id] = Mark::Done;
    };

    for (const auto& [id, needed] : dependencies)
    {
        (void)needed;
        if (marks[id] == Mark::Unvisited) { walk(walk, id); }
    }

    CNA_STUDIO_EXPECT_EQ(cycles, std::size_t{0});

    // A parser that found no dependencies at all would pass everything above by checking nothing.
    CNA_STUDIO_EXPECT(edges > 400);
}

CNA_STUDIO_TEST(NoTaskIdIsUsedTwiceAcrossTheWholePlan)
{
    // Ids are promised to be stable and never reused (plan.md, 'Id scheme'). A collision breaks
    // every reference to the id -- in commit messages, in code comments, in this test suite -- and
    // is invisible until someone follows one of them to the wrong task.
    std::map<std::string, std::string> seenIn;
    std::size_t collisions = 0;

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{sourceRoot() / "plans"})
    {
        if (entry.path().extension() != ".md") { continue; }
        const std::string relative = "plans/" + entry.path().filename().string();

        std::map<std::string, std::size_t> countsInThisFile;
        for (const PlanTask& task : readPhaseTasks(entry.path()))
        {
            ++countsInThisFile[task.id];

            const auto existing = seenIn.find(task.id);
            if (existing != seenIn.end() && existing->second != relative)
            {
                ++collisions;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "task id " + task.id + " appears in both " + existing->second + " and "
                    + relative + ". Ids are never reused.");
            }
            seenIn[task.id] = relative;
        }

        for (const auto& [id, count] : countsInThisFile)
        {
            if (count > 1)
            {
                ++collisions;
                CnaStudioTest::reportFailure(__FILE__, __LINE__,
                    "task id " + id + " has " + std::to_string(count) + " rows in " + relative + ".");
            }
        }

        // A task's id must belong to the phase whose file it lives in, or the id scheme's promise
        // that `STUDIO-06020` is phase 6's twentieth task means nothing.
        const std::string stem = entry.path().filename().string();
        if (stem.rfind("phase-", 0) == 0)
        {
            const std::string phaseNumber = stem.substr(6, 2);
            for (const PlanTask& task : readPhaseTasks(entry.path()))
            {
                if (task.id.substr(7, 2) != phaseNumber)
                {
                    ++collisions;
                    CnaStudioTest::reportFailure(__FILE__, __LINE__,
                        "task " + task.id + " lives in " + relative
                        + ", whose ids must all start STUDIO-" + phaseNumber + ".");
                }
            }
        }
    }

    CNA_STUDIO_EXPECT(seenIn.size() > 300);
    CNA_STUDIO_EXPECT_EQ(collisions, std::size_t{0});
}
