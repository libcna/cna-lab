// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/RuntimeBridge/PlayerProcess.hpp
 * @brief The editor's side of play mode: find a player build, launch it, talk to it, clean up.
 *
 * Discovery is a first-class part of this, not a detail. Because CNA fixes its backend at compile
 * time (ANALYSIS.md finding F-01), "run the game on the Software backend" means "launch
 * `cna-player-software`", and whether that binary exists is a question with a real answer. The
 * editor offers the user only the backends it can actually find, rather than a menu of fourteen
 * entries of which two work.
 */

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CNA/Studio/RuntimeBridge/StudioProtocol.hpp"
#include "CNA/Studio/RuntimeBridge/MessageChannel.hpp"

namespace CNA::Studio
{
    /** @brief One `cna-player-<backend>` binary found on disk. */
    struct PlayerBuild
    {
        /** @brief Lower-case backend name, e.g. "software". */
        std::string backend;

        /** @brief Absolute path to the executable. */
        std::string executablePath;
    };

    /**
     * @brief Finds the player builds installed alongside the editor.
     *
     * @param searchDirectory Directory to scan, normally the editor's own.
     * @return One entry per `cna-player-<backend>` (or `cna-player` itself, reported as "default"),
     *         ordered by backend name.
     */
    std::vector<PlayerBuild> discoverPlayerBuilds(const std::string& searchDirectory);

    /** @brief How a play session ended. */
    enum class PlayerExitReason
    {
        StillRunning,
        /** @brief The player ran to completion and returned success. */
        Exited,
        /** @brief The player died on a signal or returned a failure code. */
        Crashed,
        /** @brief The editor asked it to stop. */
        StoppedByStudio,
        /** @brief The player could not be started at all. */
        FailedToStart
    };

    /** @brief Returns the display name of @p reason. */
    const char* toString(PlayerExitReason reason);

    /**
     * @brief A running (or finished) player process and its bridge.
     *
     * The editor listens *before* spawning, so the port is known and the listener is guaranteed
     * to be up by the time the player tries to connect -- no retry loop, no race.
     */
    class PlayerProcess
    {
    public:
        PlayerProcess();
        ~PlayerProcess();

        PlayerProcess(const PlayerProcess&) = delete;
        PlayerProcess& operator=(const PlayerProcess&) = delete;

        /**
         * @brief Binds a port, then launches @p build with the project and that port.
         *
         * A failure to launch is reported here rather than as an immediate exit: the child says
         * so over a close-on-exec pipe, so a missing or unrunnable player binary comes back from
         * this call on every platform instead of surfacing later as a process that started and
         * vanished.
         *
         * @param build The player binary to run.
         * @param projectPath Absolute path to the `.cnaproject`.
         * @param scenePath Optional project-relative scene, overriding the startup scene.
         * @return False when the listener could not be bound or the process could not be spawned;
         *         getError() says which.
         */
        bool start(const PlayerBuild& build,
                   const std::string& projectPath,
                   const std::string& scenePath = {});

        /**
         * @brief Pumps the bridge and returns whatever the player sent.
         *
         * Call once per editor frame. Also completes the connection handshake and reaps the
         * process when it exits.
         *
         * The Hello is sent from here, on the first poll after the connection comes up, rather
         * than being left to the caller: the player treats a missing Hello as an incomplete
         * handshake, and a caller that forgot it would get a session that looks connected and is
         * quietly degraded.
         */
        std::vector<StudioMessage> poll();

        /** @brief Sends @p message to the player. Returns false when not connected. */
        bool send(const StudioMessage& message);

        /**
         * @brief Asks the player to exit, then waits briefly for it to do so.
         *
         * A player that ignores the request is terminated. Leaving an orphan game window behind
         * is worse than a hard kill on something that was already unresponsive.
         */
        void stop();

        [[nodiscard]] bool isRunning() const;
        [[nodiscard]] PlayerExitReason getExitReason() const { return exitReason_; }

        /**
         * @brief How the player ended, in the terms the operating system used.
         *
         * `plan.md` STUDIO-16003. `PlayerExitReason::Crashed` covers three different bugs — a
         * segfault, an abort, and a game that returned a failure code on purpose — and a report
         * that cannot tell them apart sends the user looking in the wrong place. The status is
         * collected by whichever wait sees the child first, so it is recorded when it is read and
         * handed out afterwards rather than asked for later, when it would already be gone.
         */
        struct Ending
        {
            /** @brief The process's exit code, or zero when a signal ended it. */
            int exitCode = 0;

            /**
             * @brief The signal that killed it, or zero.
             *
             * Zero on Windows, which reports an exception code through @ref exitCode instead.
             */
            int signal = 0;

            /** @brief Whether a signal ended it, which distinguishes a zero signal from none. */
            bool killedBySignal = false;
        };

        /** @brief How the player ended. Meaningless while it is still running. */
        [[nodiscard]] Ending getEnding() const;

        /**
         * @brief One sentence naming what the operating system said, for a log or a notification.
         *
         * Empty while the player is still running or was never started. Names the signal where it
         * has a name a person would recognise (`SIGSEGV` rather than `signal 11`, and both), because
         * the number alone is the part a user has to go and look up.
         */
        [[nodiscard]] std::string describeEnding() const;
        [[nodiscard]] const std::string& getError() const { return error_; }
        [[nodiscard]] std::uint16_t getPort() const { return channel_.getPort(); }
        [[nodiscard]] const MessageChannel& getChannel() const { return channel_; }

        /** @brief Returns the backend the player reported in its Ready message, if it has. */
        [[nodiscard]] const std::string& getReportedBackend() const { return reportedBackend_; }

        /**
         * @brief Returns true once the Hello has gone out.
         *
         * The player considers the handshake incomplete until it arrives, and nothing it sends
         * back says so -- without this, "Studio forgot to introduce itself" is a state no test
         * could observe from this side.
         */
        [[nodiscard]] bool isHelloSent() const { return helloSent_; }

    private:
        /**
         * @brief Settles the exit reason if the player has ended since it was last asked.
         *
         * Collecting a child is a one-shot act: whichever call waits on it first gets the status
         * and every later one gets nothing. So the reason is settled wherever the death is first
         * *noticed*, rather than only in poll() -- otherwise a toolbar asking isRunning() to
         * decide whether to grey out Stop would quietly swallow the exit, and the editor would
         * never report that the game had ended.
         */
        void refreshExitReason() const;

        struct Impl;
        std::unique_ptr<Impl> impl_;
        MessageChannel channel_;
        std::string error_;
        std::string projectPath_;
        bool helloSent_ = false;
        bool started_ = false;
        std::string reportedBackend_;
        mutable PlayerExitReason exitReason_ = PlayerExitReason::StillRunning;
    };
}
