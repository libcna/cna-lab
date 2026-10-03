// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Core/StudioFileWrite.hpp
 * @brief Writing a file the user cannot lose (`plan.md` STUDIO-31003).
 *
 * Phase 31's purpose line is one sentence — *this is a content-authoring application, losing work
 * is unacceptable* — and the ordinary way to write a file breaks it. Opening a document with
 * `std::ios::trunc` empties it *before* the first byte of the replacement is written, so the window
 * between those two moments is a window in which the user's scene is a zero-byte file. A crash, a
 * full disk, a killed process or a power cut inside it loses the document and leaves nothing to
 * recover from.
 *
 * The fix is old and unremarkable: write a temporary beside the target, close it, then rename over
 * the target. `rename` within a directory replaces atomically, so at every instant the path holds
 * either the whole previous document or the whole new one. Nothing observes a half-written file
 * because one never exists under that name.
 *
 * ### Why this is a function rather than a habit
 *
 * **It was already a habit, and the habit had four adherents and eight holes.**
 * `RecoveryStore.cpp` states the rule and follows it, as do `StudioPreferences`,
 * `StudioWorkspaceStore` and `RecentProjects` — each with its own copy of the temp-and-rename
 * dance. Every *document* — scenes, prefabs, materials, environment maps, `.cnaasset` sidecars,
 * the project file — truncated in place. The crash-recovery snapshot was being written safely to
 * protect files that were not.
 *
 * That is the shape this project keeps finding: a rule written down, believed, and absent. Five
 * copies of a procedure is also five chances to get one of them subtly wrong, and a guard test can
 * watch one function far more easily than it can watch a convention.
 *
 * ### What it does not promise
 *
 * **Not durability across a power cut** — that needs an `fsync` of the file *and* of the directory,
 * which the C++ standard library cannot express and which would cost a synchronous disk write on
 * every keystroke-driven save. What is promised is *atomicity*: the path is never observed
 * half-written. A power cut may lose the most recent save; it may not corrupt the document.
 *
 * **Not a lock.** Two processes writing one document still race, and the loser's content is simply
 * gone rather than interleaved. Studio has one writer per project, and a scheme that pretended
 * otherwise would be a bigger promise than it could keep.
 */

#include <filesystem>
#include <string>
#include <string_view>

namespace CNA::Studio
{
    /**
     * @brief The suffix a half-written file carries, so everything else can recognise one.
     *
     * The writer appends this plus a number to the target's name. **It is published rather than
     * private because a crash leaves one behind**, and a leftover temporary sitting in the asset
     * folder is not a detail of this file -- it is a file the asset scanner would otherwise
     * discover, give a `Uuid`, track as an asset of unknown type, show in the Content Browser and
     * write a sidecar beside. One spelling, shared, so the thing that creates them and the thing
     * that must ignore them cannot disagree.
     */
    inline constexpr const char* kStudioWriteTemporarySuffix = ".cnatmp";

    /** @brief True when @p fileName is one of @ref studioWriteFileAtomically's temporaries. */
    [[nodiscard]] bool studioIsWriteTemporaryName(std::string_view fileName);

    /** @brief What a write did, and why it did not. */
    struct StudioFileWriteResult
    {
        bool succeeded = false;

        /**
         * @brief A sentence naming the step that failed, or empty on success.
         *
         * Named per step -- creating the directory, opening the temporary, writing it, renaming it
         * -- because the four have different causes and different fixes: a missing folder, a
         * read-only file, a full disk, and a target that is open elsewhere. "Could not save" is a
         * message that sends a user to look in the wrong place.
         */
        std::string error;
    };

    /**
     * @brief Writes @p bytes to @p path so that the path is never seen half-written.
     *
     * Creates the directories above @p path when they are missing, which is what every caller
     * wanted and several were doing separately.
     *
     * **The temporary is written beside the target, never in the system temp directory.** A rename
     * is only atomic within one filesystem, and the system temp directory is routinely on another
     * one -- where `std::filesystem::rename` fails outright on some platforms and silently
     * degrades to copy-then-delete on others, which is exactly the non-atomic write this exists to
     * avoid.
     *
     * On failure the temporary is removed and **the target is left exactly as it was**, which is
     * the whole point: a failed save must not be worse than no save.
     */
    [[nodiscard]] StudioFileWriteResult studioWriteFileAtomically(const std::filesystem::path& path,
                                                                  std::string_view bytes);

    /**
     * @brief The same, taking a path as a string, for callers that hold one.
     *
     * Most of the editor speaks in `std::string` paths -- the asset database's `resolvePath`
     * returns one -- and making every caller construct a `std::filesystem::path` at the call site
     * would be noise at twelve of them.
     */
    [[nodiscard]] StudioFileWriteResult studioWriteFileAtomically(const std::string& path,
                                                                  std::string_view bytes);
}
