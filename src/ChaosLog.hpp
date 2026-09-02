// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace CnaKiller
{
    /**
     * @brief Append-only, flush-per-line reproduction log plus a best-effort crash banner.
     *
     * cna-killer exists to crash the CNA runtime, sometimes hard enough that no C++
     * destructor or iostream buffer ever gets a chance to run. ChaosLog therefore:
     *
     *  - fsyncs every line to disk the instant it is written, so the log on disk is never
     *    more than one action behind reality even if the process dies a moment later; and
     *  - on POSIX platforms, installs signal handlers for the signals a driver or runtime
     *    crash typically raises (SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS) that write a
     *    one-line, async-signal-safe "how to reproduce this" banner to stderr before
     *    restoring the default handler and re-raising, so the process still terminates
     *    (and core-dumps) normally.
     *
     * The whole point is the seed: because ChaosEngine's action sequence depends only on
     * the seed and the tick count (never on wall-clock time), rerunning
     * `cna-killer --seed <seed>` replays the identical sequence of malicious actions up to
     * whatever tick the crash happened on.
     *
     * At most one ChaosLog should exist per process -- it installs process-wide signal
     * handlers on construction and restores the previous ones on destruction.
     */
    class ChaosLog
    {
    public:
        ChaosLog(const std::string& path, std::uint64_t seed, const std::string& intensityName,
                 int argc, char** argv);
        ~ChaosLog();

        ChaosLog(const ChaosLog&) = delete;
        ChaosLog& operator=(const ChaosLog&) = delete;

        /**
         * @brief Records the action about to run and refreshes the async-signal-safe crash banner.
         *
         * Call this immediately BEFORE performing the action itself: if the action crashes,
         * the log's last line and the crash banner shown on stderr both already name it.
         */
        void BeginAction(std::uint64_t tick, const std::string& actionName, const std::string& details);

        /** @brief Appends a free-form informational line (pool sizes, milestones, etc). */
        void Note(const std::string& text);

        /** @brief Path this log is writing to. */
        [[nodiscard]] const std::string& Path() const { return path_; }

    private:
        std::string path_;
        std::ofstream file_;
        std::uint64_t seed_;

        void WriteLine(const std::string& line);
    };
}
