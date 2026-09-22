// SPDX-License-Identifier: MS-PL
/**
 * @file CNA/Studio/Ui/StudioLog.hpp
 * @brief The message log, as a model no UI owns.
 *
 * `plan.md` STUDIO-07005.
 *
 * The prototype kept its log inside the Dear ImGui `StudioUi`, which meant the Console panel could
 * only be drawn by the UI that also stored its contents. Porting the panel therefore starts here:
 * the log moves out, both presentations read the same one, and the migration can proceed a panel at
 * a time with both UIs showing identical output — which is the only way anybody can tell whether
 * the port is faithful.
 *
 * ### Bounded, because a log is not a database
 *
 * An editor left open for a day with a noisy import can produce millions of lines, and a log that
 * grows without limit turns that into an out-of-memory crash that loses the user's scene. The store
 * keeps a fixed number of the most recent entries and counts what it dropped, so the panel can say
 * "12,043 earlier messages" rather than quietly presenting a partial history as a complete one.
 *
 * ### Repeats are collapsed, and counted
 *
 * A message repeated four hundred times is one thing that happened four hundred times, not four
 * hundred things. Collapsing them keeps the interesting lines on screen, which is the entire
 * purpose of a console; the count is kept so nothing is hidden.
 *
 * ### Where a line came from is a field, not a prefix
 *
 * `plan.md` STUDIO-27020. The game's output used to reach this log as text beginning "Player: ",
 * which is a source written into the message because there was nowhere else to put it. A prefix
 * cannot be filtered on without matching strings, cannot be coloured, and is lost the moment
 * somebody rewords it. @ref LogSource is the field that prefix was standing in for.
 *
 * ### The clock is set, not read
 *
 * Entries carry the time they arrived, and the log is *told* what time it is rather than asking.
 * That is the same rule the asset watcher and the recovery session follow, and for the same reason:
 * a test that has to sleep to make a timestamp differ is a test that fails on a loaded machine.
 *
 * Seconds since the session began, not a wall clock. In an editor log the useful question is "how
 * long after I pressed Play did this happen", which a relative time answers and a wall clock makes
 * the reader subtract; it also has no timezone, no locale and no midnight.
 */

#pragma once

#include "CNA/Studio/Core/Uuid.hpp"

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

namespace CNA::Studio
{
    /**
     * @brief Severity of a console message, used for filtering and colouring.
     *
     * Declared here rather than beside `StudioUi` because it belongs to the log, and the log now
     * outlives any particular UI: both the Dear ImGui console and the Studio one read this model,
     * which is what makes the ported panel a *port* rather than a second console showing something
     * else.
     */
    enum class LogSeverity
    {
        Trace,
        Info,
        Warning,
        Error
    };

    /** @brief Returns the short display name of @p severity, e.g. "warn". */
    const char* toString(LogSeverity severity);

    /**
     * @brief Which part of the system said it (`plan.md` STUDIO-27020).
     *
     * Three, because three is what a person actually sorts by when a console is busy: what the
     * editor is doing, what a build is doing, and what the game they are running is saying. A
     * finer taxonomy -- a category per subsystem -- is a filter list nobody reads; a coarser one
     * is the undifferentiated scroll this exists to fix.
     *
     * **One stream, not three panels**, and the choice is worth keeping written down because
     * `STUDIO-16002` made it first and this row inherits it: the sources genuinely interleave --
     * "Player ready on opengles3." and the game's first line belong next to each other in time,
     * and splitting them would make a user correlate by hand what they are already reading in
     * order. What they must be able to do is tell which is which at a glance and hide the ones
     * they are not asking about, and a field does both where the "Player: " prefix this replaced
     * could only do the first.
     */
    enum class LogSource
    {
        /** @brief The editor itself: scans, saves, imports, commands. */
        Studio,
        /** @brief A build or a package export. */
        Build,
        /** @brief The running game, relayed over the player channel. */
        Game
    };

    /** @brief Returns the short display name of @p source, e.g. "game". */
    const char* toString(LogSource source);

    /**
     * @brief Something a log line points at, which the panel can offer to open.
     *
     * **Attached by whoever logs, never parsed out of the message** (`plan.md` STUDIO-27020). A
     * console that scanned its own text for things that look like paths would find them in prose,
     * miss them in quotes it did not expect, and break the day somebody reworded a message. The
     * site that writes "could not import 'Content/Hero.png'" is the site that already holds the id
     * of what it was importing, so that is where the link comes from.
     *
     * Entities and assets travel as ids rather than paths, per ANALYSIS.md D-08: a path is what a
     * thing is called today, and a log line outlives a rename.
     */
    struct StudioLogLink
    {
        enum class Kind
        {
            /** @brief Nothing to open. The ordinary case. */
            None,
            /** @brief A file on disk, by project-relative path. */
            File,
            /** @brief An entity in the open scene, by id. */
            Entity,
            /** @brief An asset in the project, by id. */
            Asset
        };

        Kind kind = Kind::None;

        /** @brief Project-relative path, for @ref Kind::File. Empty otherwise. */
        std::string path;

        /** @brief The entity or asset, for @ref Kind::Entity and @ref Kind::Asset. */
        Uuid id;

        /** @brief What the panel offers to open. Empty when there is nothing. */
        [[nodiscard]] bool isEmpty() const { return kind == Kind::None; }

        friend bool operator==(const StudioLogLink& left, const StudioLogLink& right)
        {
            return left.kind == right.kind && left.path == right.path && left.id == right.id;
        }
    };

    /** @brief One line in the log. */
    struct StudioLogEntry
    {
        LogSeverity severity = LogSeverity::Info;

        /** @brief Which part of the system said it. */
        LogSource source = LogSource::Studio;

        std::string message;

        /** @brief How many times in a row this message arrived. One for an ordinary line. */
        std::size_t repeats = 1;

        /**
         * @brief Seconds since the session began, as the log was told them.
         *
         * The time of the **first** arrival when repeats are collapsed: "this started happening at
         * 12.4 s and has happened four hundred times" is the useful reading, and the last
         * occurrence is always approximately now.
         */
        double timeSeconds = 0.0;

        /** @brief What this line points at, if anything. */
        StudioLogLink link;
    };

    /**
     * @brief A bounded, UI-independent message log.
     *
     * Not thread-safe, and deliberately so: everything that logs today does so from the frame
     * thread, and a mutex whose only purpose is to be uncontended is a mutex that hides where the
     * real threading boundary should go. When a background job needs to log, it will need a queue,
     * not a lock.
     */
    class StudioLog
    {
    public:
        /** @brief How many entries are kept before the oldest are dropped. */
        static constexpr std::size_t kDefaultCapacity = 10000;

        StudioLog() = default;

        /** @brief Constructs a log holding at most @p capacity entries. Zero means the default. */
        explicit StudioLog(std::size_t capacity)
            : capacity_(capacity == 0 ? kDefaultCapacity : capacity)
        {
        }

        /**
         * @brief Appends a message from Studio itself, collapsing it if identical to the previous.
         *
         * @param severity How serious it is.
         * @param message The text. Newlines are kept: a stack trace is one message.
         */
        void append(LogSeverity severity, std::string message);

        /**
         * @brief Appends a message from @p source, with an optional thing to open.
         *
         * Collapsed into the previous entry only when severity, source, message **and** link all
         * match. Two lines with the same words from the build and from the game are two events, and
         * a console that merged them would be answering a question nobody asked.
         *
         * @param severity How serious it is.
         * @param source Which part of the system said it.
         * @param message The text.
         * @param link What the line points at, if anything.
         */
        void append(LogSeverity severity, LogSource source, std::string message,
                    StudioLogLink link = {});

        /**
         * @brief Sets the session clock entries are stamped with.
         *
         * Told rather than read (see the file comment). The host advances this once a frame from
         * the same monotonic clock every other timed thing here uses.
         *
         * @param seconds Seconds since the session began. Values below zero are clamped.
         */
        void setNow(double seconds);

        /** @brief The session time the next entry will be stamped with. */
        [[nodiscard]] double now() const { return now_; }

        /** @brief Every retained entry, oldest first. */
        [[nodiscard]] const std::deque<StudioLogEntry>& entries() const { return entries_; }

        /** @brief How many entries were dropped to stay within capacity. */
        [[nodiscard]] std::size_t droppedCount() const { return dropped_; }

        /** @brief The most entries this log will retain. */
        [[nodiscard]] std::size_t capacity() const { return capacity_; }

        /** @brief Removes everything, including the dropped count: the user asked for a clean slate. */
        void clear();

        /**
         * @brief How many retained entries are at least @p minimumSeverity.
         * @param minimumSeverity Lowest severity to count.
         * @return The count.
         */
        [[nodiscard]] std::size_t countAtLeast(LogSeverity minimumSeverity) const;

        /**
         * @brief How many retained entries came from @p source.
         * @param source The source to count.
         * @return The count.
         */
        [[nodiscard]] std::size_t countFrom(LogSource source) const;

        /**
         * @brief The log as plain text, for the clipboard or a bug report.
         *
         * Each line is `[     0.000] [severity] [source] message`, with a repeat count appended
         * where there is one, so text pasted into an issue says the same thing the panel said --
         * including *when* and *who*, which is most of what makes a pasted log useful to somebody
         * who was not there.
         *
         * @param minimumSeverity Lowest severity to include.
         * @return The text, newline-terminated per entry.
         */
        [[nodiscard]] std::string toText(LogSeverity minimumSeverity = LogSeverity::Trace) const;

        /**
         * @brief One entry as the clipboard spells it.
         *
         * Exposed so the panel can copy exactly what it is showing -- which is not always
         * "everything at or above a severity" once a search and a source filter are involved --
         * without a second implementation of the format that could drift from this one.
         *
         * @param entry The entry.
         * @return The line, without a trailing newline.
         */
        [[nodiscard]] static std::string toTextLine(const StudioLogEntry& entry);

    private:
        std::deque<StudioLogEntry> entries_;
        std::size_t capacity_ = kDefaultCapacity;
        std::size_t dropped_ = 0;
        double now_ = 0.0;
    };
}
