// SPDX-License-Identifier: MS-PL
/**
 * @file StudioLogPanel.cpp
 * @brief The Output Log on the Studio UI.
 */

#include "CNA/Studio/UiCore/StudioLogPanel.hpp"

#include "CNA/Studio/UiCore/StudioWidgets.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <string_view>

namespace CNA::Studio
{
    namespace
    {
        /** @brief Returns a metric already scaled to physical pixels. */
        float metricOf(const StudioTheme& theme, StudioMetric metric)
        {
            return static_cast<float>(theme.metric(metric));
        }

        /** @brief The filter's display name for a severity, as the buttons spell it. */
        constexpr std::array<std::pair<LogSeverity, const char*>, 4> kSeverityNames{{
            {LogSeverity::Trace, "All"},
            {LogSeverity::Info, "Info"},
            {LogSeverity::Warning, "Warnings"},
            {LogSeverity::Error, "Errors"},
        }};

        /** @brief The severity prefix each row carries, matching what the clipboard text says. */
        std::string_view severityPrefix(LogSeverity severity)
        {
            switch (severity)
            {
                case LogSeverity::Trace: return "trace";
                case LogSeverity::Info: return "info";
                case LogSeverity::Warning: return "warn";
                case LogSeverity::Error: return "error";
            }
            return "info";
        }

        /** @brief The three sources, in the order their buttons appear. */
        constexpr std::array<std::pair<LogSource, const char*>, 3> kSourceNames{{
            {LogSource::Studio, "Studio"},
            {LogSource::Build, "Build"},
            {LogSource::Game, "Game"},
        }};

        /** @brief The bit @p source occupies in the source filter mask. */
        std::int64_t sourceBit(LogSource source)
        {
            return std::int64_t{1} << static_cast<int>(source);
        }



        /** @brief ASCII lower-case, matching what the Outliner's search does with a name. */
        char lowerAscii(char character)
        {
            return (character >= 'A' && character <= 'Z')
                       ? static_cast<char>(character - 'A' + 'a')
                       : character;
        }

        /**
         * @brief Whether @p haystack contains @p needle, ignoring ASCII case.
         *
         * An empty needle matches everything, which is what makes an empty search box mean "no
         * search" rather than "match nothing".
         */
        bool containsFolded(std::string_view haystack, std::string_view needle)
        {
            if (needle.empty()) { return true; }
            if (needle.size() > haystack.size()) { return false; }

            const std::size_t last = haystack.size() - needle.size();
            for (std::size_t start = 0; start <= last; ++start)
            {
                std::size_t i = 0;
                for (; i < needle.size(); ++i)
                {
                    if (lowerAscii(haystack[start + i]) != lowerAscii(needle[i])) { break; }
                }
                if (i == needle.size()) { return true; }
            }
            return false;
        }

        /** @brief `12.345` in a fixed six-character column, so the times read down as a column. */
        std::string formatTime(double seconds)
        {
            std::array<char, 24> text{};
            const int written =
                std::snprintf(text.data(), text.size(), "%8.3f", std::max(0.0, seconds));
            if (written <= 0) { return "       -"; }
            return std::string{text.data(), static_cast<std::size_t>(written)};
        }
    }

    StudioColor studioLogSeverityColor(const StudioTheme& theme, LogSeverity severity)
    {
        switch (severity)
        {
            case LogSeverity::Trace: return theme.color(StudioColorRole::TextDisabled);
            case LogSeverity::Info: return theme.color(StudioColorRole::TextPrimary);
            case LogSeverity::Warning: return theme.color(StudioColorRole::Warning);
            case LogSeverity::Error: return theme.color(StudioColorRole::Error);
        }
        return theme.color(StudioColorRole::TextPrimary);
    }

    StudioLogPanelResult studioLogPanel(StudioFrame& frame, const UiRect& bounds,
                                        const StudioLog& log)
    {
        StudioLogPanelResult result;
        const StudioTheme& theme = frame.theme();

        const float padding = metricOf(theme, StudioMetric::SpacingSmall);
        const float spacing = metricOf(theme, StudioMetric::SpacingXSmall);
        const float controlHeight = metricOf(theme, StudioMetric::ControlHeightSmall);
        const float rowHeight = metricOf(theme, StudioMetric::RowHeight);

        UiRect area = bounds.inset(UiEdges{padding});
        if (area.width <= 0.0f || area.height <= 0.0f) { return result; }

        // -----------------------------------------------------------------------------------
        // Toolbar
        // -----------------------------------------------------------------------------------
        UiRect toolbar = area.splitTop(std::min(controlHeight, area.height));
        area.splitTop(std::min(spacing, area.height));

        // The filter is the panel's own view state, not a document property, so it is retained
        // per-widget rather than commanded: what a person chooses to look at is not an edit, and
        // putting it in the undo stack would be actively wrong.
        WidgetState& view = frame.state().get(frame.ids().make("logview"));

        const auto severityOf = [](std::int64_t index) {
            const auto clamped = static_cast<std::size_t>(
                std::clamp<std::int64_t>(index, 0, static_cast<std::int64_t>(kSeverityNames.size()) - 1));
            return kSeverityNames[clamped].first;
        };
        const LogSeverity minimumSeverity = severityOf(view.integer);

        const auto buttonWidth = [&](std::string_view label) {
            return std::ceil(studioLabelWidth(frame, label, StudioFontRole::BodySmall)
                             + metricOf(theme, StudioMetric::ControlPaddingHorizontal));
        };

        StudioButtonOptions buttonOptions;
        buttonOptions.font = StudioFontRole::BodySmall;

        {
            const UiRect copyBounds = toolbar.splitLeft(std::min(buttonWidth("Copy"), toolbar.width));
            StudioButtonOptions copyOptions = buttonOptions;
            copyOptions.enabled = !log.entries().empty();
            // Only the flag here. What Copy puts on the clipboard is "exactly what the panel is
            // showing", and the panel does not know that yet -- the search box has not been drawn
            // this frame. Filled in once the visible set exists, below.
            result.copyRequested =
                studioButton(frame, frame.ids().make("copy"), copyBounds, "Copy", copyOptions)
                    .activated;
            toolbar.splitLeft(std::min(spacing, toolbar.width));

            const UiRect clearBounds = toolbar.splitLeft(std::min(buttonWidth("Clear"), toolbar.width));
            StudioButtonOptions clearOptions = buttonOptions;
            clearOptions.enabled = !log.entries().empty();
            result.cleared =
                studioButton(frame, frame.ids().make("clear"), clearBounds, "Clear", clearOptions)
                    .activated;
            toolbar.splitLeft(std::min(metricOf(theme, StudioMetric::SpacingMedium), toolbar.width));
        }

        // Severity as a row of buttons rather than a dropdown: four options, all of them one click
        // away, and the current one readable without opening anything. A combo would cost a click
        // and hide three quarters of the answer.
        for (std::size_t i = 0; i < kSeverityNames.size(); ++i)
        {
            const char* label = kSeverityNames[i].second;
            const UiRect filterBounds = toolbar.splitLeft(std::min(buttonWidth(label), toolbar.width));
            if (filterBounds.width <= 0.0f) { break; }

            StudioButtonOptions options = buttonOptions;
            options.selected = static_cast<std::size_t>(view.integer) == i;
            if (studioButton(frame, frame.ids().make(label), filterBounds, label, options).activated)
            {
                view.integer = static_cast<std::int64_t>(i);
            }
            toolbar.splitLeft(std::min(spacing, toolbar.width));
        }

        // Follow and Pause sit at the right, away from the filters: they are the two controls here
        // that change what happens next rather than what is shown now.
        WidgetState& pauseState = frame.state().get(frame.ids().make("logpause"));
        {
            const auto checkboxWidth = [&](std::string_view label) {
                return std::ceil(studioLabelWidth(frame, label, StudioFontRole::BodySmall)
                                 + metricOf(theme, StudioMetric::IconSizeSmall)
                                 + metricOf(theme, StudioMetric::SpacingMedium));
            };

            const float followWidth = checkboxWidth("Follow");
            if (toolbar.width > followWidth)
            {
                const UiRect followBounds = toolbar.splitRight(followWidth);
                bool follow = view.checked;
                if (studioCheckbox(frame, frame.ids().make("follow"), followBounds, "Follow", follow)
                        .changed)
                {
                    view.checked = follow;
                }
            }

            const float pauseWidth = checkboxWidth("Pause");
            if (toolbar.width > pauseWidth)
            {
                const UiRect pauseBounds = toolbar.splitRight(pauseWidth);
                bool paused = pauseState.checked;
                if (studioCheckbox(frame, frame.ids().make("pause"), pauseBounds, "Pause", paused)
                        .changed)
                {
                    pauseState.checked = paused;

                    // The mark is taken the moment it is pressed, and counts entries ever
                    // appended rather than entries currently held -- the log drops its oldest, so
                    // a mark over the living would slide backwards and reveal lines the user
                    // paused precisely to avoid.
                    if (paused)
                    {
                        pauseState.integer = static_cast<std::int64_t>(log.entries().size()
                                                                       + log.droppedCount());
                    }
                }
            }
        }

        // -----------------------------------------------------------------------------------
        // Second row: the search box and the source filter
        // -----------------------------------------------------------------------------------
        // Its own row because the first is full, and because these two answer a different
        // question: the first row is what to do with the log, this is which part of it to look at.
        // Dropped entirely when the panel is too short for it rather than overlapping the list --
        // a console squeezed into a strip should still be a console.
        WidgetState& searchState = frame.state().get(frame.ids().make("logsearchvalue"));
        WidgetState& sourceState = frame.state().get(frame.ids().make("logsources"));

        // The mask holds the sources that are **hidden**, not the ones that are shown, so that a
        // freshly created state -- integer zero -- already means "hide nothing". Storing the shown
        // set would have needed a second flag to tell "not initialised yet" from "the user has
        // turned everything off", and a filter that starts excluding things hides output from a
        // user who never asked it to.
        const auto hiddenSources = [&] { return sourceState.integer; };

        if (area.height > controlHeight + spacing)
        {
            UiRect filters = area.splitTop(controlHeight);
            area.splitTop(std::min(spacing, area.height));

            for (std::size_t i = kSourceNames.size(); i-- > 0;)
            {
                const char* label = kSourceNames[i].second;

                // A count on the button, because "Build" with nothing behind it and "Build" hiding
                // two hundred lines look identical otherwise, and the difference is the whole
                // reason somebody is looking at this row. Measured from the text that is actually
                // drawn, count included -- a width taken from the bare label would clip the
                // number the moment a busy project put four digits in it.
                const std::size_t count = log.countFrom(kSourceNames[i].first);
                const std::string text =
                    count > 0 ? std::string{label} + " (" + std::to_string(count) + ")" : label;

                const float width = buttonWidth(text);
                if (filters.width <= width) { break; }

                const UiRect sourceBounds = filters.splitRight(width);
                const std::int64_t bit = sourceBit(kSourceNames[i].first);

                StudioButtonOptions options = buttonOptions;
                options.selected = (hiddenSources() & bit) == 0;

                if (studioButton(frame, frame.ids().make(label), sourceBounds, text, options)
                        .activated)
                {
                    sourceState.integer ^= bit;
                }
                filters.splitRight(std::min(spacing, filters.width));
            }

            if (filters.width > 0.0f)
            {
                StudioTextFieldOptions searchOptions;
                searchOptions.placeholder = "Search messages";
                searchOptions.font = StudioFontRole::BodySmall;
                (void)studioTextField(frame, frame.ids().make("logsearch"), filters,
                                      searchState.text, searchOptions);
            }
        }

        const std::string& search = searchState.text;
        const std::int64_t hidden = hiddenSources();

        if (area.height <= 0.0f) { return result; }

        // -----------------------------------------------------------------------------------
        // The list
        // -----------------------------------------------------------------------------------
        // Where the paused view stops. Entry `i` was the `dropped + i`-th ever appended, so this
        // survives the log dropping its oldest while the user reads.
        const std::size_t appended = log.entries().size() + log.droppedCount();
        result.paused = pauseState.checked;

        std::size_t limit = log.entries().size();
        if (result.paused)
        {
            const auto mark = static_cast<std::size_t>(std::max<std::int64_t>(0, pauseState.integer));
            result.heldBackCount = appended > mark ? appended - mark : 0;
            limit = mark > log.droppedCount() ? std::min(limit, mark - log.droppedCount()) : 0;
        }

        // Filtered into indices rather than copied: a hundred thousand log lines copied every
        // frame, in two passes, is the difference between a console and a stall.
        std::vector<std::size_t> visible;
        visible.reserve(limit);
        for (std::size_t i = 0; i < limit; ++i)
        {
            const StudioLogEntry& entry = log.entries()[i];
            if (entry.severity < minimumSeverity) { continue; }
            if ((hidden & sourceBit(entry.source)) != 0) { continue; }
            if (!containsFolded(entry.message, search)) { continue; }
            visible.push_back(i);
        }

        // What Copy puts on the clipboard, now that "what the panel is showing" is known. Built
        // through `StudioLog::toTextLine`, so the pasted text cannot drift from what a full
        // `toText()` produces -- two spellings of one format is one of them being wrong later.
        if (result.copyRequested)
        {
            for (const std::size_t index : visible)
            {
                result.copyText += StudioLog::toTextLine(log.entries()[index]);
                result.copyText += '\n';
            }
        }

        StudioScrollOptions scroll;
        scroll.contentHeight = static_cast<float>(visible.size()) * rowHeight;
        scroll.stickToEnd = view.checked;
        scroll.wheelStep = rowHeight * 3.0f;

        const StudioScrollResult view_ =
            studioBeginScroll(frame, frame.ids().make("scroll"), area, scroll);

        std::size_t first = 0;
        std::size_t last = 0;
        view_.visibleRows(rowHeight, visible.size(), first, last);

        result.rowsMatching = visible.size();
        result.rowsDrawn = last - first;

        const float timeWidth = std::ceil(
            studioLabelWidth(frame, "00000.000", StudioFontRole::Monospace)
            + metricOf(theme, StudioMetric::SpacingMedium));
        const float prefixWidth = std::ceil(
            studioLabelWidth(frame, "error", StudioFontRole::Monospace)
            + metricOf(theme, StudioMetric::SpacingMedium));
        const float sourceWidth = std::ceil(
            studioLabelWidth(frame, "studio", StudioFontRole::Monospace)
            + metricOf(theme, StudioMetric::SpacingMedium));

        // Laid out in both passes, because a row that can be clicked has to be routed in the input
        // pass and drawn in the other, and the two have to agree about where it is.
        frame.ids().push("logrows");
        for (std::size_t row = first; row < last; ++row)
        {
            // Scoped by the entry's index in the log rather than by its row on screen, so the id
            // of a line follows the line: scrolling must not hand row 0's identity to whatever has
            // arrived above it, or a click would land on a different message than the one under
            // the pointer when the frame began.
            frame.ids().pushIndex(static_cast<std::int64_t>(visible[row]));

            const StudioLogEntry& entry = log.entries()[visible[row]];

            const UiRect rowBounds{
                view_.viewport.left(),
                std::round(view_.viewport.top() - view_.offsetY
                           + static_cast<float>(row) * rowHeight),
                view_.viewport.width, rowHeight};

            UiRect cursor = rowBounds.inset(
                UiEdges{metricOf(theme, StudioMetric::SpacingSmall), 0.0f,
                        metricOf(theme, StudioMetric::SpacingSmall), 0.0f});
            const UiRect timeBounds = cursor.splitLeft(std::min(timeWidth, cursor.width));
            const UiRect prefixBounds = cursor.splitLeft(std::min(prefixWidth, cursor.width));
            const UiRect sourceBounds = cursor.splitLeft(std::min(sourceWidth, cursor.width));

            // Only the message is the link, not the whole row: the timestamp and the severity are
            // there to be read, and a row that navigated away when somebody clicked to place a
            // text cursor would be a trap.
            const bool linked = !entry.link.isEmpty();
            StudioInteraction interaction;
            if (linked && cursor.width > 0.0f)
            {
                interaction = frame.interact(frame.ids().make("link"), cursor);
                if (interaction.clicked)
                {
                    result.linkActivated = true;
                    result.link = entry.link;
                }
            }

            if (!frame.isDrawPass())
            {
                frame.ids().pop();
                continue;
            }

            // Alternating bands rather than separators: a thousand hairlines is visual noise,
            // and the eye tracks a long line across a shaded row without one.
            if (row % 2 == 1)
            {
                frame.drawList().fillRect(rowBounds,
                                          theme.color(StudioColorRole::ControlBackground));
            }

            studioDrawText(frame, timeBounds, formatTime(entry.timeSeconds),
                           StudioFontRole::Monospace,
                           theme.color(StudioColorRole::TextDisabled));
            studioDrawText(frame, prefixBounds, severityPrefix(entry.severity),
                           StudioFontRole::Monospace,
                           studioLogSeverityColor(theme, entry.severity));

            // The source in secondary text rather than the severity colour: it says *who*, and
            // colouring it like the severity would make a routine line from the game read as
            // urgent as an error from it.
            studioDrawText(frame, sourceBounds, toString(entry.source), StudioFontRole::Monospace,
                           theme.color(StudioColorRole::TextSecondary));

            std::string text = entry.message;
            if (entry.repeats > 1) { text += "  (x" + std::to_string(entry.repeats) + ")"; }

            // A linked message takes the accent colour, and underlines under the pointer. Colour
            // alone would be the only cue, and colour alone is not a cue for everyone
            // (`STUDIO-32xxx`'s rule, applied here rather than waited for).
            const StudioColor messageColor =
                linked ? theme.color(interaction.hovered ? StudioColorRole::AccentHover
                                                         : StudioColorRole::Accent)
                       : studioLogSeverityColor(theme, entry.severity);

            const std::string shown =
                studioTruncateText(frame, theme.font(StudioFontRole::Monospace), text, cursor.width);
            studioDrawText(frame, cursor, shown, StudioFontRole::Monospace, messageColor);

            if (linked && interaction.hovered)
            {
                const float width =
                    std::min(studioLabelWidth(frame, shown, StudioFontRole::Monospace), cursor.width);
                const float baseline = std::round(cursor.bottom() - std::max(2.0f, rowHeight * 0.2f));
                frame.drawList().fillRect(UiRect{cursor.left(), baseline, width, 1.0f},
                                          messageColor);
            }

            frame.ids().pop();
        }
        frame.ids().pop();

        if (frame.isDrawPass() && visible.empty())
        {
            // An empty state that says which of the three empties this is. "Nothing here" when a
            // filter is hiding four hundred errors is the panel lying to the user, and so is
            // "nothing here" when the user has paused output.
            std::string message;
            if (log.entries().empty()) { message = "No messages yet."; }
            else if (result.paused && limit == 0)
            {
                message = "Paused. " + std::to_string(result.heldBackCount)
                          + " message" + (result.heldBackCount == 1 ? "" : "s")
                          + " waiting.";
            }
            else
            {
                message = "No messages match. " + std::to_string(log.entries().size())
                          + " are hidden by the filter.";
            }
            studioDrawText(frame,
                           view_.viewport.inset(
                               UiEdges{metricOf(theme, StudioMetric::SpacingMedium)}),
                           message, StudioFontRole::Body,
                           theme.color(StudioColorRole::TextSecondary));
        }

        studioEndScroll(frame);
        return result;
    }
}
