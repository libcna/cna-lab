// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Assets/AssetWatcher.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <system_error>

namespace CNA::Studio
{
    namespace
    {
        /** @brief One file's size and modification time, or absent when it is not there. */
        struct FileStamp
        {
            bool exists = false;
            std::uint64_t size = 0;
            std::int64_t modifiedTime = 0;
        };

        FileStamp stampOf(const std::string& absolutePath)
        {
            std::error_code errorCode;

            // One `directory_entry` rather than three free functions (`plan.md` STUDIO-30031).
            // `exists`, `file_size` and `last_write_time` are three trips into the filesystem for
            // one inode; a `directory_entry` caches what its refresh read and answers all three
            // from it. At a hundred thousand assets that was three hundred thousand syscalls a
            // sweep, and this is the two thirds of them that bought nothing.
            const std::filesystem::directory_entry file{std::filesystem::path{absolutePath},
                                                        errorCode};

            FileStamp stamp;
            stamp.exists = !errorCode && file.exists(errorCode) && !errorCode;
            if (!stamp.exists) { return stamp; }

            stamp.size = static_cast<std::uint64_t>(file.file_size(errorCode));
            if (errorCode) { stamp.size = 0; errorCode.clear(); }

            // Seconds, matching what a scan records. The clock's native ticks are around 4.6e18,
            // past the range a double holds exactly, and the sidecar's numbers are doubles -- so
            // storing ticks would make every asset look modified on every comparison.
            const auto writeTime = file.last_write_time(errorCode);
            stamp.modifiedTime =
                errorCode
                    ? 0
                    : std::chrono::duration_cast<std::chrono::seconds>(writeTime.time_since_epoch()).count();

            return stamp;
        }
    }

    void AssetWatcher::setInterval(double seconds)
    {
        interval_ = std::max(0.0, seconds);
    }

    void AssetWatcher::setMaxRecordsPerPoll(std::size_t records)
    {
        maxRecordsPerPoll_ = std::max<std::size_t>(1, records);
    }

    AssetWatchResult AssetWatcher::poll(AssetDatabase& assets, double deltaSeconds)
    {
        AssetWatchResult result;

        // The interval gates the *start* of a lap, not each slice of one (`plan.md`
        // STUDIO-30031). A sweep already under way continues on every call: waiting the interval
        // between slices would take a hundred-thousand-asset project, at sixty-four records a poll
        // and half a second between them, thirteen minutes to get round once, which is not a
        // watcher.
        if (!sweeping_)
        {
            elapsed_ += std::max(0.0, deltaSeconds);
            if (elapsed_ < interval_) { return result; }
            elapsed_ = 0.0;
        }
        result.polled = true;

        // Walked through the path index rather than `getAll()`, which copies a pointer per record
        // in the project to look at a slice of them -- the cost `STUDIO-09016` added the index to
        // avoid, paid here every poll.
        const std::map<std::string, Uuid>& byPath = assets.getPathIndex();
        auto cursor = sweeping_ ? byPath.upper_bound(sweepCursorPath_) : byPath.begin();

        std::size_t stated = 0;
        for (; cursor != byPath.end() && stated < maxRecordsPerPoll_; ++cursor, ++stated)
        {
            sweepCursorPath_ = cursor->first;

            const AssetRecord* record = assets.find(cursor->second);
            if (record == nullptr) { continue; }

            const FileStamp stamp = stampOf(assets.resolvePath(record->sourcePath));

            // The presence cache is refreshed here rather than by whoever asks (STUDIO-30012).
            // This loop already stats every tracked file, so keeping the cache in step costs
            // nothing that was not being spent -- and it is what makes `isMissing` free everywhere
            // else, including once per row per pass in the Content Browser. Refreshed a slice at a
            // time now, so on a very large project it lags by up to a lap; it was never a promise
            // of freshness finer than the poll interval, and a lap is the poll interval's
            // replacement on a project that size.
            (void)assets.setAssetPresent(record->id, stamp.exists);

            // A record that has never been stamped -- size and time both zero -- is one whose file
            // was already absent when it was scanned. Comparing against that would report it as
            // changed on the first poll of every session.
            const bool wasKnownPresent = record->sourceSize != 0 || record->sourceModifiedTime != 0;

            if (!stamp.exists)
            {
                if (wasKnownPresent)
                {
                    result.removed.push_back(record->id);

                    // Zeroed so the disappearance is reported once. Leaving the old stamp would
                    // make every subsequent poll report it again, and the console would fill up
                    // with the same line twice a second.
                    if (AssetRecord* mutableRecord = assets.findMutable(record->id))
                    {
                        mutableRecord->sourceSize = 0;
                        mutableRecord->sourceModifiedTime = 0;
                    }
                }
                continue;
            }

            if (stamp.size == record->sourceSize && stamp.modifiedTime == record->sourceModifiedTime)
            {
                continue;
            }

            AssetRecord* mutableRecord = assets.findMutable(record->id);
            if (mutableRecord == nullptr) { continue; }

            // A file coming back is worth telling apart from one being edited: the first fixes a
            // broken reference, the second means reloading something already on screen.
            (wasKnownPresent ? result.changed : result.restored).push_back(record->id);

            mutableRecord->sourceSize = stamp.size;
            mutableRecord->sourceModifiedTime = stamp.modifiedTime;
        }

        // Round, or not yet. The cursor is deliberately left where it is when the lap is over: the
        // next one starts from the beginning because `sweeping_` says so, not because the string
        // was cleared, and a caller reading the result gets told which of the two happened.
        sweeping_ = cursor != byPath.end();
        result.sweepComplete = !sweeping_;
        return result;
    }
}
