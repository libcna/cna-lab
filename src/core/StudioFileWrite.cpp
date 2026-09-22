// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Core/StudioFileWrite.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <string>
#include <system_error>

namespace CNA::Studio
{
    namespace
    {
        /**
         * @brief A suffix nothing else will pick, so two saves in flight cannot share a temporary.
         *
         * Two documents saved in the same millisecond -- Save All does exactly that -- must not
         * write to one temporary and rename it twice. A counter is enough because the contention
         * this guards against is within one process; across processes the rename still resolves to
         * one winner, which is the honest limit the header states.
         */
        std::uint64_t nextWriteTicket()
        {
            static std::atomic<std::uint64_t> counter{0};
            return counter.fetch_add(1, std::memory_order_relaxed) + 1;
        }

        StudioFileWriteResult failure(std::string message)
        {
            return StudioFileWriteResult{false, std::move(message)};
        }
    }

    bool studioIsWriteTemporaryName(std::string_view fileName)
    {
        // The suffix and then digits, rather than the suffix anywhere in the name. A document a
        // user deliberately called `notes.cnatmp-ideas.txt` is theirs, and a scanner that hid it
        // would be deciding what their files mean from a substring.
        const std::size_t at = fileName.rfind(kStudioWriteTemporarySuffix);
        if (at == std::string_view::npos) { return false; }

        const std::string_view tail =
            fileName.substr(at + std::string_view{kStudioWriteTemporarySuffix}.size());
        if (tail.empty()) { return false; }

        return std::all_of(tail.begin(), tail.end(),
                           [](char character) { return character >= '0' && character <= '9'; });
    }

    StudioFileWriteResult studioWriteFileAtomically(const std::filesystem::path& path,
                                                    std::string_view bytes)
    {
        if (path.empty()) { return failure("no path was given"); }

        std::error_code code;

        const std::filesystem::path parent = path.parent_path();
        if (!parent.empty())
        {
            std::filesystem::create_directories(parent, code);

            // Checked rather than trusted: `create_directories` reports failure through the code,
            // and a caller told "saved" into a directory that does not exist has been told the
            // opposite of what happened.
            if (code && !std::filesystem::is_directory(parent))
            {
                return failure("cannot create the folder '" + parent.generic_string()
                               + "': " + code.message());
            }
            code.clear();
        }

        // Beside the target, for the reason the header gives: a rename is atomic only within one
        // filesystem, and the system temp directory is routinely on another.
        std::filesystem::path temporary = path;
        temporary += std::string{kStudioWriteTemporarySuffix}
                   + std::to_string(nextWriteTicket());

        {
            std::ofstream stream{temporary, std::ios::binary | std::ios::trunc};
            if (!stream)
            {
                return failure("cannot open '" + temporary.generic_string() + "' for writing");
            }

            stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));

            // Closed explicitly and *then* checked, rather than checked while open. A stream that
            // has buffered the whole document and not yet flushed it reports success right up
            // until the flush that fails -- which on a full disk is the only moment it can fail.
            stream.close();
            if (!stream)
            {
                std::filesystem::remove(temporary, code);
                return failure("writing '" + temporary.generic_string() + "' failed, so '"
                               + path.generic_string() + "' was left as it was");
            }
        }

        std::filesystem::rename(temporary, path, code);
        if (code)
        {
            // The temporary goes, because leaving one behind turns a failed save into a folder
            // slowly filling with files the user did not make and cannot identify.
            std::error_code cleanup;
            std::filesystem::remove(temporary, cleanup);
            return failure("cannot replace '" + path.generic_string() + "': " + code.message());
        }

        return StudioFileWriteResult{true, {}};
    }

    StudioFileWriteResult studioWriteFileAtomically(const std::string& path, std::string_view bytes)
    {
        return studioWriteFileAtomically(std::filesystem::path{path}, bytes);
    }
}
