// SPDX-License-Identifier: MS-PL
/**
 * @file StudioLog.cpp
 * @brief The bounded message log.
 */

#include "CNA/Studio/Ui/StudioLog.hpp"

#include <algorithm>
#include <array>
#include <cstdio>

namespace CNA::Studio
{
    const char* toString(LogSource source)
    {
        switch (source)
        {
            case LogSource::Studio: return "studio";
            case LogSource::Build: return "build";
            case LogSource::Game: return "game";
        }
        return "studio";
    }

    void StudioLog::setNow(double seconds)
    {
        now_ = std::max(0.0, seconds);
    }

    void StudioLog::append(LogSeverity severity, std::string message)
    {
        append(severity, LogSource::Studio, std::move(message));
    }

    void StudioLog::append(LogSeverity severity, LogSource source, std::string message,
                           StudioLogLink link)
    {
        // A message repeated four hundred times is one thing that happened four hundred times.
        // Collapsing keeps the interesting lines on screen, which is the entire purpose of a
        // console; the count is kept so nothing is hidden.
        //
        // Source and link join the comparison (`plan.md` STUDIO-27020). The same words from the
        // build and from the game are two events, and two lines that point at different assets are
        // two lines however alike they read -- collapsing either would merge things a person is
        // filtering precisely in order to tell apart. The timestamp deliberately does *not* join
        // it: the entry keeps the time of the first arrival, because "this started at 12.4 s and
        // has happened four hundred times" is the reading that helps.
        if (!entries_.empty() && entries_.back().severity == severity
            && entries_.back().source == source && entries_.back().message == message
            && entries_.back().link == link)
        {
            ++entries_.back().repeats;
            return;
        }

        StudioLogEntry entry;
        entry.severity = severity;
        entry.source = source;
        entry.message = std::move(message);
        entry.repeats = 1;
        entry.timeSeconds = now_;
        entry.link = std::move(link);
        entries_.push_back(std::move(entry));

        while (entries_.size() > capacity_)
        {
            entries_.pop_front();
            ++dropped_;
        }
    }

    void StudioLog::clear()
    {
        entries_.clear();
        // The dropped count goes too. It exists to tell the user their history is incomplete, and
        // after a clear the history they are looking at is exactly what they asked for.
        dropped_ = 0;
    }

    std::size_t StudioLog::countAtLeast(LogSeverity minimumSeverity) const
    {
        std::size_t count = 0;
        for (const StudioLogEntry& entry : entries_)
        {
            if (entry.severity >= minimumSeverity) { ++count; }
        }
        return count;
    }

    std::size_t StudioLog::countFrom(LogSource source) const
    {
        std::size_t count = 0;
        for (const StudioLogEntry& entry : entries_)
        {
            if (entry.source == source) { ++count; }
        }
        return count;
    }

    std::string StudioLog::toTextLine(const StudioLogEntry& entry)
    {
        // Fixed width and three decimals, so a pasted log's times form a column somebody can read
        // down rather than a ragged edge. `snprintf` rather than a stream: this runs once per line
        // of a copy that may be ten thousand lines, and it is the one formatting job in this file.
        std::array<char, 32> stamp{};
        const int written = std::snprintf(stamp.data(), stamp.size(), "[%9.3f] ", entry.timeSeconds);

        std::string text;
        if (written > 0) { text.assign(stamp.data(), static_cast<std::size_t>(written)); }

        text += '[';
        text += toString(entry.severity);
        text += "] [";
        text += toString(entry.source);
        text += "] ";
        text += entry.message;
        if (entry.repeats > 1)
        {
            text += " (x" + std::to_string(entry.repeats) + ")";
        }
        return text;
    }

    std::string StudioLog::toText(LogSeverity minimumSeverity) const
    {
        std::string text;
        for (const StudioLogEntry& entry : entries_)
        {
            if (entry.severity < minimumSeverity) { continue; }
            text += toTextLine(entry);
            text += '\n';
        }
        return text;
    }
}
