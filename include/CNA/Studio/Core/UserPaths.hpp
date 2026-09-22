// SPDX-License-Identifier: MS-PL
/**
 * @file CNA/Studio/Core/UserPaths.hpp
 * @brief Where Studio keeps a user's own files, by each platform's own convention.
 *
 * `plan.md` STUDIO-05014, STUDIO-06010.
 *
 * Three kinds of user file, kept apart because the platforms keep them apart and because what
 * should happen when one is lost differs:
 *
 * - **Configuration** — the workspace layout, preferences, recent projects. Losing it costs the
 *   user their arrangement. It is the thing a person would think to back up.
 * - **State** — crash-recovery snapshots, logs, caches of things that can be recomputed. Losing it
 *   costs nothing a rebuild cannot replace, and a machine sweeping it is not a bug.
 *
 * Studio resolves both from the environment rather than hard-coding a path, so a user who has moved
 * their home directory, a CI job with a scratch `HOME`, and a test that wants neither, all get what
 * they asked for. Every resolution ends somewhere: a user with no home directory still deserves an
 * autosave, so the last fallback is the temporary directory rather than nothing.
 *
 * **Nothing is migrated from the prototype's `cna-editor` directories, and Studio never reads
 * them.** The prototype resolved these paths inline and wrote a layout under `$CONFIG/cna-editor`
 * and recovery snapshots under `$STATE/cna-editor/recovery`; it was never released, never installed
 * and never packaged, so nobody holds files in that location that a distributed build put there.
 * Neither format is readable by today's code in any case. A migration would therefore be an
 * unexercisable branch running on every start-up forever, for data belonging to nobody, which is
 * worse than its absence. `docs/ADR-002-THE-FOUR-SILENT-QUESTIONS.md` Decision 2 (STUDIO-01015)
 * states the reasoning and the manual remedy; `StudioUserDirectoriesAreStudioNamedAndMigrateNothing`
 * holds this file to it.
 */

#pragma once

#include <string>

namespace CNA::Studio
{
    /**
     * @brief Directory for the user's Studio configuration.
     *
     * `$XDG_CONFIG_HOME/cna-studio`, `%APPDATA%\cna-studio`, `$HOME/.config/cna-studio`, or a
     * temporary directory, in that order of preference.
     *
     * @return An absolute path, which may not exist yet. Empty only when even a temporary directory
     *         could not be found, which is a machine with nowhere to write at all.
     */
    [[nodiscard]] std::string getStudioConfigDirectory();

    /**
     * @brief Directory for the user's Studio state: recovery snapshots, logs, caches.
     *
     * `$XDG_STATE_HOME/cna-studio`, `%LOCALAPPDATA%\cna-studio`, `%APPDATA%\cna-studio`,
     * `$HOME/.local/state/cna-studio`, or a temporary directory.
     *
     * @return An absolute path, which may not exist yet.
     */
    [[nodiscard]] std::string getStudioStateDirectory();
}
