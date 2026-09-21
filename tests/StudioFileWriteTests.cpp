// SPDX-License-Identifier: MS-PL
/**
 * @file StudioFileWriteTests.cpp
 * @brief A file the user cannot lose (`plan.md` STUDIO-31003, and `STUDIO-31010`'s first cases).
 *
 * Phase 31's purpose line is one sentence — *losing work is unacceptable* — and the property these
 * cases pin is narrow and absolute: **the target is never observed half-written.** Not "usually",
 * not "unless the disk is full". Either the previous document is there whole or the new one is.
 *
 * The interesting cases are all failures, because the success path is the one everybody tests. A
 * write that cannot open its temporary, a write that cannot replace its target, a write into a
 * folder that is not there — every one of them must leave what was on disk exactly as it was, and
 * must say which step failed rather than "could not save".
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/Core/StudioFileWrite.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

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
                  / ("cna-studio-write-" + name + "-" + std::to_string(counter()++));
            std::error_code code;
            std::filesystem::remove_all(path_, code);
            std::filesystem::create_directories(path_, code);
        }

        ~ScopedDirectory()
        {
            std::error_code code;
            std::filesystem::permissions(path_, std::filesystem::perms::owner_all,
                                         std::filesystem::perm_options::add, code);
            std::filesystem::remove_all(path_, code);
        }

        ScopedDirectory(const ScopedDirectory&) = delete;
        ScopedDirectory& operator=(const ScopedDirectory&) = delete;

        [[nodiscard]] std::filesystem::path at(const std::string& relative) const
        {
            return path_ / relative;
        }

        [[nodiscard]] const std::filesystem::path& root() const { return path_; }

        [[nodiscard]] std::string read(const std::string& relative) const
        {
            std::ifstream stream{path_ / relative, std::ios::binary};
            if (!stream) { return {}; }
            return std::string{std::istreambuf_iterator<char>{stream},
                               std::istreambuf_iterator<char>{}};
        }

        void write(const std::string& relative, const std::string& text) const
        {
            std::ofstream stream{path_ / relative, std::ios::binary | std::ios::trunc};
            stream << text;
        }

        /** @brief How many files are in the tree, so a stray temporary is visible. */
        [[nodiscard]] std::size_t fileCount() const
        {
            std::size_t files = 0;
            std::error_code code;
            for (const auto& entry : std::filesystem::recursive_directory_iterator{path_, code})
            {
                if (entry.is_regular_file()) { ++files; }
            }
            return files;
        }

    private:
        static int& counter() { static int value = 0; return value; }
        std::filesystem::path path_;
    };
}

/** @brief The ordinary case: a file appears, with exactly the bytes it was given. */
CNA_STUDIO_TEST(AnAtomicWriteProducesTheBytesItWasGiven)
{
    ScopedDirectory directory{"basic"};

    const std::string bytes = "{\n \"formatVersion\": 1\n}\n";
    const StudioFileWriteResult wrote =
        studioWriteFileAtomically(directory.at("scene.cnascene"), bytes);

    CNA_STUDIO_EXPECT(wrote.succeeded);
    CNA_STUDIO_EXPECT(wrote.error.empty());
    CNA_STUDIO_EXPECT_EQ(directory.read("scene.cnascene"), bytes);

    // Byte for byte, including embedded nulls and no line-ending translation: a generated source
    // file must come out of Studio as the bytes that went in, or the embedded runtime no longer
    // matches the copy it was embedded from.
    const std::string awkward = std::string{"a\r\nb\0c\n", 7};
    CNA_STUDIO_EXPECT(studioWriteFileAtomically(directory.at("raw.bin"), awkward).succeeded);
    CNA_STUDIO_EXPECT_EQ(directory.read("raw.bin"), awkward);

    // And the folder holds exactly the two files. A temporary left behind turns every save into a
    // folder slowly filling with files the user did not make and cannot identify.
    CNA_STUDIO_EXPECT_EQ(directory.fileCount(), std::size_t{2});
}

/** @brief Replacing an existing file leaves it whole at every instant, and leaves no temporary. */
CNA_STUDIO_TEST(AnAtomicWriteReplacesAnExistingFileWithoutTruncatingIt)
{
    ScopedDirectory directory{"replace"};
    directory.write("scene.cnascene", "the previous document");

    CNA_STUDIO_EXPECT(
        studioWriteFileAtomically(directory.at("scene.cnascene"), "the new document").succeeded);
    CNA_STUDIO_EXPECT_EQ(directory.read("scene.cnascene"), std::string{"the new document"});
    CNA_STUDIO_EXPECT_EQ(directory.fileCount(), std::size_t{1});

    // A shorter document does not leave the tail of the longer one behind, which is the failure
    // mode of writing over a file without truncating and the reason "just don't truncate" is not
    // the fix.
    CNA_STUDIO_EXPECT(studioWriteFileAtomically(directory.at("scene.cnascene"), "short").succeeded);
    CNA_STUDIO_EXPECT_EQ(directory.read("scene.cnascene"), std::string{"short"});

    // An empty document is a document, not a failure: a scene with everything deleted is a legal
    // thing to save.
    CNA_STUDIO_EXPECT(studioWriteFileAtomically(directory.at("scene.cnascene"), "").succeeded);
    CNA_STUDIO_EXPECT(directory.read("scene.cnascene").empty());
    CNA_STUDIO_EXPECT(std::filesystem::exists(directory.at("scene.cnascene")));
}

/** @brief The folders above the target are created, which is what every caller wanted. */
CNA_STUDIO_TEST(AnAtomicWriteCreatesTheFoldersAboveItsTarget)
{
    ScopedDirectory directory{"folders"};

    CNA_STUDIO_EXPECT(
        studioWriteFileAtomically(directory.at("Assets/Skies/Overcast.cnaenv"), "{}").succeeded);
    CNA_STUDIO_EXPECT_EQ(directory.read("Assets/Skies/Overcast.cnaenv"), std::string{"{}"});
}

/**
 * @brief **A failed write leaves the previous document exactly as it was.**
 *
 * The whole point of the row. A save that fails must not be worse than a save that never happened,
 * and the way a truncating write fails is by having already destroyed the file before discovering
 * it could not write the replacement.
 */
CNA_STUDIO_TEST(AFailedWriteLeavesThePreviousDocumentIntact)
{
    ScopedDirectory directory{"failure"};
    directory.write("scene.cnascene", "the document that must survive");

    // A read-only folder: the temporary cannot be created, so the write fails at its first step --
    // which is precisely the step a truncating write would have taken *after* emptying the target.
    std::error_code code;
    std::filesystem::permissions(directory.root(), std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::remove, code);

    // Skipped rather than failed where permissions do not bite -- a container running as root
    // writes through a read-only bit, and a case that asserted otherwise would fail for a reason
    // that has nothing to do with the code.
    const StudioFileWriteResult wrote =
        studioWriteFileAtomically(directory.at("scene.cnascene"), "the replacement");

    std::filesystem::permissions(directory.root(), std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::add, code);

    if (!wrote.succeeded)
    {
        // Left exactly as it was, and the error names the step rather than saying "could not
        // save" -- a missing folder, a read-only file and a full disk send a user to three
        // different places.
        CNA_STUDIO_EXPECT_EQ(directory.read("scene.cnascene"),
                             std::string{"the document that must survive"});
        CNA_STUDIO_EXPECT(!wrote.error.empty());
        CNA_STUDIO_EXPECT_EQ(directory.fileCount(), std::size_t{1});
    }
    else
    {
        // The write went through, which means this process can write through the bit. The
        // document is still the new one and nothing is left behind, which is all that can be
        // asserted here.
        CNA_STUDIO_EXPECT_EQ(directory.read("scene.cnascene"), std::string{"the replacement"});
        CNA_STUDIO_EXPECT_EQ(directory.fileCount(), std::size_t{1});
    }

    // An empty path is refused rather than producing a file somewhere surprising.
    const StudioFileWriteResult nowhere = studioWriteFileAtomically(std::string{}, "x");
    CNA_STUDIO_EXPECT(!nowhere.succeeded);
    CNA_STUDIO_EXPECT(!nowhere.error.empty());
}

/**
 * @brief Two writes in flight do not share a temporary.
 *
 * Save All saves every dirty document in one go, and two of them landing on one temporary would
 * rename it twice -- putting one document's bytes under the other's name, which is a corruption
 * that looks like the editor mixing up files.
 */
CNA_STUDIO_TEST(TwoWritesInOneMomentDoNotShareATemporary)
{
    ScopedDirectory directory{"concurrent"};

    for (int i = 0; i < 16; ++i)
    {
        const std::string name = "document" + std::to_string(i) + ".cnascene";
        CNA_STUDIO_EXPECT(
            studioWriteFileAtomically(directory.at(name), "contents " + std::to_string(i))
                .succeeded);
    }

    for (int i = 0; i < 16; ++i)
    {
        CNA_STUDIO_EXPECT_EQ(directory.read("document" + std::to_string(i) + ".cnascene"),
                             "contents " + std::to_string(i));
    }

    // Sixteen documents and nothing else.
    CNA_STUDIO_EXPECT_EQ(directory.fileCount(), std::size_t{16});
}

/**
 * @brief A sidecar is refused where writing one would invent a folder, rather than inventing it.
 *
 * Found by `STUDIO-31003` and worth a case of its own, because the defect it fixes was *created*
 * by the fix. `studioWriteFileAtomically` creates the folders above its target, which is what
 * every document writer wanted — a new material in a new folder, a project being created. A
 * sidecar is the opposite: it is metadata that belongs beside a file, so a missing folder means
 * the asset is missing too.
 *
 * Both refusals had been silent before, because the write failed anyway. Making it succeed turned
 * them into directory trees appearing in whatever the process was standing in — which, for a test
 * binary run from a checkout, was the source tree.
 */
CNA_STUDIO_TEST(ASidecarIsRefusedWhereWritingOneWouldInventAFolder)
{
    ScopedDirectory directory{"sidecar"};

    // A database with no project root resolves paths against the process's working directory, so
    // a sidecar written from one lands somewhere nobody chose.
    {
        AssetDatabase rootless;
        AssetRecord record;
        record.id = Uuid::generate();
        record.sourcePath = "Assets/Crate.png";
        record.type = AssetType::Texture2D;
        const Uuid id = record.id;
        CNA_STUDIO_EXPECT(rootless.add(std::move(record)));

        std::string problem;
        CNA_STUDIO_EXPECT(!rootless.writeSidecar(id, &problem));
        CNA_STUDIO_EXPECT(problem.find("project root") != std::string::npos);
    }

    // And with a root, a sidecar for an asset whose folder is not there is refused too -- rather
    // than building the folder and writing the identity of something that does not exist.
    {
        AssetDatabase assets;
        assets.setProjectRoot(directory.root().generic_string());

        AssetRecord record;
        record.id = Uuid::generate();
        record.sourcePath = "Assets/Missing/Crate.png";
        record.type = AssetType::Texture2D;
        const Uuid id = record.id;
        CNA_STUDIO_EXPECT(assets.add(std::move(record)));

        std::string problem;
        CNA_STUDIO_EXPECT(!assets.writeSidecar(id, &problem));
        CNA_STUDIO_EXPECT(problem.find("does not exist") != std::string::npos);
        CNA_STUDIO_EXPECT(!std::filesystem::exists(directory.at("Assets")));

        // With the folder there, it writes -- so the refusal is about the folder and not about
        // sidecars in general.
        std::error_code code;
        std::filesystem::create_directories(directory.at("Assets/Missing"), code);
        CNA_STUDIO_EXPECT(assets.writeSidecar(id, &problem));
        CNA_STUDIO_EXPECT(std::filesystem::exists(directory.at("Assets/Missing/Crate.png.cnaasset")));
    }
}
