// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Assets/AssetWatcher.hpp
 * @brief Notices assets changed outside the editor, so the viewport stops showing stale art.
 *
 * **Polling, not a native watcher.** inotify, kqueue and ReadDirectoryChangesW are three separate
 * implementations with three separate failure modes, and every one of them still needs a polling
 * fallback for network and container mounts -- which is exactly where a team's assets often live.
 * Native watchers become worth their cost when a project reaches tens of thousands of assets, and
 * can be added behind this same interface without any caller changing.
 *
 * ### The sweep is spread over frames, because it used to be a hang
 *
 * This file used to say that re-stating the tracked files "costs one syscall each and happens twice
 * a second at most; on a project large enough for that to matter, the interval is a knob". The
 * analysis was right and the numbers were never taken. `plan.md` STUDIO-30031 took them: at a
 * hundred thousand assets a poll cost **half a second, on the frame**, and ran twice a second. That
 * is not a slow frame, it is a hang, and lengthening the interval makes it rarer rather than
 * shorter.
 *
 * Two things changed. Each file is now stated **once** rather than three times -- `exists`,
 * `file_size` and `last_write_time` were three trips into the filesystem for one inode, and one
 * `directory_entry` answers all three. And a poll now visits at most @ref getMaxRecordsPerPoll
 * records, resuming from where the last one stopped, so a sweep of a large project is spread across
 * frames the way every other piece of background work here is budgeted.
 *
 * **What that costs is latency, and it is the right trade.** On a project small enough for one
 * slice -- which is every project that fits in the default budget -- nothing changes at all: the
 * whole set is swept on the poll it is due, exactly as before. On a hundred-thousand-asset project
 * a lap takes many frames, so an edit made outside the editor is noticed seconds later rather than
 * within the interval. Seconds of latency on a project that size is unnoticeable; half a second of
 * frozen UI, twice a second, is not.
 *
 * The cursor is a **path**, not an index, so adding or deleting assets mid-sweep cannot make the
 * lap skip a record that merely moved position. A record added behind the cursor waits for the next
 * lap, which is the same promise the interval always made.
 *
 * The clock is passed in rather than read, so a test can advance time exactly and never has to
 * sleep. A watcher whose tests sleep is a watcher whose tests are flaky on a loaded machine.
 */

#include <cstddef>
#include <string>
#include <vector>

#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/Core/Uuid.hpp"

namespace CNA::Studio
{
    /** @brief What a poll found. */
    struct AssetWatchResult
    {
        /** @brief Tracked assets whose file changed on disk. */
        std::vector<Uuid> changed;

        /** @brief Tracked assets whose file has gone. */
        std::vector<Uuid> removed;

        /** @brief Tracked assets whose file has come back. */
        std::vector<Uuid> restored;

        /** @brief True when a poll actually ran this call, whether or not it found anything. */
        bool polled = false;

        /**
         * @brief True when this call finished a lap over every tracked record.
         *
         * A poll visits at most `AssetWatcher::getMaxRecordsPerPoll()` records, so on a large
         * project a lap spans several calls (`plan.md` STUDIO-30031). Reported because "nothing
         * changed" and "nothing changed *yet*" are different answers, and a caller that wants the
         * first -- a test, or a refresh that means to wait for a verdict -- has no other way to
         * tell them apart.
         */
        bool sweepComplete = false;

        [[nodiscard]] bool hasChanges() const
        {
            return !changed.empty() || !removed.empty() || !restored.empty();
        }
    };

    /**
     * @brief Watches the tracked assets for changes made outside the editor.
     *
     * Detects modification and disappearance of files the database already knows about. New files
     * are a scan's job, not a watcher's: discovering one means reading a sidecar, assigning an id
     * and deciding whether it is a move, and that is a decision the database owns.
     */
    class AssetWatcher
    {
    public:
        /** @brief Seconds between polls. Values below zero are clamped to zero. */
        void setInterval(double seconds);
        [[nodiscard]] double getInterval() const { return interval_; }

        /**
         * @brief Most records one poll may stat before yielding the frame.
         *
         * @param records Clamped to at least one: a budget of zero would be a watcher that never
         *        finishes a lap, which is worse than one that costs too much.
         */
        void setMaxRecordsPerPoll(std::size_t records);
        [[nodiscard]] std::size_t getMaxRecordsPerPoll() const { return maxRecordsPerPoll_; }

        /**
         * @brief Advances the clock by @p deltaSeconds and polls when one is due.
         *
         * Updates the stamps of whatever it reports, so a change is reported exactly once rather
         * than on every poll until something else fixes it.
         *
         * **The interval gates the start of a lap, not each slice.** Once a sweep is under way it
         * continues on every call until it has been all the way round, because a lap that waited
         * the interval between slices would take a hundred-thousand-asset project six minutes to
         * get round once. When the lap finishes, the interval applies again.
         */
        AssetWatchResult poll(AssetDatabase& assets, double deltaSeconds);

        /**
         * @brief Forces the next poll() to run regardless of the interval.
         *
         * Does not restart a lap that is already under way: "check now" means the next slice
         * happens now, and throwing away the sweep in progress to begin a fresh one would make a
         * caller that asked repeatedly never reach the end.
         */
        void requestImmediatePoll() { elapsed_ = interval_; }

    private:
        /**
         * @brief What one poll is allowed to stat, by default.
         *
         * Chosen from measurement rather than taste (`plan.md` STUDIO-30031), and the measurement
         * was not what was first guessed. Stating a record costs about **4.4 us** on the container
         * this was taken on -- read off the benchmark rather than estimated: a slice of 128 raised
         * `content-list-100k`'s median frame by 566 us, which is 4.4 us a record to three figures.
         * Sixty-four keeps the sweep's contribution near 280 us, under seven per cent of the
         * 4 167 us a frame gives the UI.
         *
         * **What this buys is survivability, not suitability.** At a hundred thousand assets a lap
         * takes about 1 500 polls -- twenty-odd seconds of wall-clock -- so an edit made outside
         * the editor is noticed in that time rather than within the interval, and the sweep never
         * stops, because the next lap is due before the last one ends. That is the polling
         * fallback being made to degrade gracefully at a size this file already says it is the
         * wrong mechanism for. A native watcher is the answer at that size and goes behind this
         * same interface.
         *
         * Every project small enough to fit one slice is unaffected: it is swept whole, on the poll
         * it is due, exactly as before.
         */
        static constexpr std::size_t kDefaultMaxRecordsPerPoll = 64;

        double interval_ = 0.5;
        double elapsed_ = 0.0;
        std::size_t maxRecordsPerPoll_ = kDefaultMaxRecordsPerPoll;

        /**
         * @brief Where the lap in progress got to: the path of the last record stated.
         *
         * A path rather than an index, because the records are ordered by path and that order is
         * stable under insertion and removal -- an index is not, and a sweep that resumed by index
         * would skip a record whenever one behind the cursor was deleted.
         */
        std::string sweepCursorPath_;

        /** @brief Whether @ref sweepCursorPath_ means anything, so an empty first path still works. */
        bool sweeping_ = false;
    };
}
