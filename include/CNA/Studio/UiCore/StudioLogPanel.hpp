// SPDX-License-Identifier: MS-PL
/**
 * @file CNA/Studio/UiCore/StudioLogPanel.hpp
 * @brief The Output Log, on the Studio UI. The first panel ported off Dear ImGui.
 *
 * `plan.md` STUDIO-07005.
 *
 * Chosen first deliberately. It is the simplest panel that is still a real one — a filtered,
 * scrolling, selectable list with a toolbar — so it exercises the parts of the new UI a panel
 * actually needs (scrolling, virtualised rows, a row of controls, retained view state) without also
 * needing a property grid, a tree, drag and drop or a graphics device. If the strangler seam is
 * wrong, this is where it is cheapest to find out.
 *
 * ### It owns its view state, not the log
 *
 * The log is a `StudioLog` the panel is handed. What the panel owns is what the *user chose to look
 * at*: the severity filter, the source filter, the search text, whether output is paused and
 * whether to follow new output. Those are not document state, not undoable, and not shared between
 * two windows showing the same log — a person filtering one console to errors has not asked the
 * other to hide anything.
 *
 * ### Pause holds the view, it never holds the log (`plan.md` STUDIO-27020)
 *
 * Pausing a console has one job: let somebody read a line that is scrolling away. It must not stop
 * messages being *recorded*, because the whole reason the line is interesting is usually what
 * follows it. So Pause freezes a **mark** — how many entries had arrived when it was pressed — and
 * the panel draws up to there, saying how many are waiting. Unpausing shows them. Nothing is ever
 * dropped by pausing, which is the difference between a pause and a mute.
 *
 * The mark counts entries *ever appended*, not entries currently held, because the log is bounded
 * and drops its oldest. A mark that counted the living would slide backwards under a busy log and
 * quietly reveal lines the user had paused to avoid.
 *
 * ### Search commits, it does not filter per keystroke
 *
 * Enter or leaving the field applies it, which is what the Outliner's search does (`STUDIO-13002`)
 * and the reason is that they should agree rather than that either is obviously right: two search
 * boxes in one application that respond differently to the same typing is a worse answer than
 * either behaviour.
 *
 * ### What a very large log costs (`plan.md` STUDIO-27021)
 *
 * The geometry has been bounded by the screen since this panel was written. The *work* was not: the
 * filter walked every retained entry, in both passes, every frame, and `STUDIO-27020` added three
 * more walks per pass to put counts on the source buttons. At the two hundred thousand entries a
 * log may be configured to hold, that is one and a half million comparisons a frame to draw forty
 * rows.
 *
 * Three things fix it, and the panel reports @ref StudioLogPanelResult::entriesExamined so the
 * property is *counted* rather than timed — the same doctrine the rest of the suite follows.
 *
 * 1. The source counts are maintained by the log, so they cost nothing to ask for.
 * 2. **With no filter in force, nothing is walked at all.** Every entry is visible, row *r* is
 *    entry *r*, and there is no list to build. This is the state a console is in almost all of the
 *    time, and it is now free regardless of size.
 * 3. With a filter in force the matching set is **cached and extended**, not rebuilt: a steady log
 *    costs nothing, a log gaining lines costs the lines it gained, and only a change of filter pays
 *    for a full pass. Caching needs somewhere to live, which is what @ref StudioLogPanelState is;
 *    a caller that passes none gets the old behaviour, correct and slower.
 */

#pragma once

#include "CNA/Studio/Ui/StudioLog.hpp"
#include "CNA/Studio/UiCore/StudioFrame.hpp"
#include "CNA/Studio/UiCore/UiRect.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace CNA::Studio
{
    /** @brief What the user asked the log panel to do this frame. */
    struct StudioLogPanelResult
    {
        /** @brief The Clear button was pressed. True only in the input pass. */
        bool cleared = false;

        /** @brief The Copy button was pressed; @ref copyText is what to put on the clipboard. */
        bool copyRequested = false;

        /** @brief The text Copy asked for, filtered exactly as the panel is showing it. */
        std::string copyText;

        /** @brief How many rows the panel actually put on screen this pass. */
        std::size_t rowsDrawn = 0;

        /** @brief How many entries the current filter is showing, drawn or scrolled out of view. */
        std::size_t rowsMatching = 0;

        /** @brief A row's link was clicked; @ref link says what to open. Input pass only. */
        bool linkActivated = false;

        /**
         * @brief What the clicked row points at.
         *
         * Reported, not opened: the panel has no scene, no asset database and no business
         * selecting anything. The binder acts.
         */
        StudioLogLink link;

        /** @brief Whether the panel is currently holding new output back. */
        bool paused = false;

        /** @brief How many entries have arrived since Pause was pressed. Zero when not paused. */
        std::size_t heldBackCount = 0;

        /**
         * @brief How many log entries this pass had to look at to decide what to show.
         *
         * `plan.md` STUDIO-27021. The counted quantity behind "very large logs stay responsive": a
         * timing assertion on a shared machine fails for reasons that have nothing to do with the
         * code, and this is the number a regression would actually move. Zero when no filter is in
         * force, however large the log; the size of the filter's own work otherwise, which a cache
         * keeps at "what arrived since last frame".
         */
        std::size_t entriesExamined = 0;
    };

    /**
     * @brief Where the panel keeps what it worked out last frame (`plan.md` STUDIO-27021).
     *
     * Opaque to the caller: own one per Output Log, hand it to @ref studioLogPanel, and do not
     * write to it. It holds no *decisions* — the filter, the pause and the search are still the
     * panel's own retained widget state — only the answer it last computed and enough to know
     * whether that answer is still good.
     *
     * A caller that passes nothing gets a panel that is correct and recomputes everything, which
     * is what keeps the headless paths and every existing test meaning what they meant.
     */
    struct StudioLogPanelState
    {
        /**
         * @brief Matching entries, as positions in the log's **ever-retained** sequence.
         *
         * Absolute — `droppedCount() + index` — rather than relative to `entries()`, because the
         * log drops its oldest and every relative index would shift underneath the cache when it
         * did. An absolute position stays true; the ones that fall off the front are dropped from
         * the front of this.
         */
        std::vector<std::size_t> matching;

        /** @brief One past the last absolute position the scan has considered. */
        std::size_t scannedTo = 0;

        /** @brief The filter @ref matching was built for; a change of any of these rebuilds it. */
        std::int64_t severity = -1;
        std::int64_t hiddenSources = -1;
        std::string search;

        /** @brief False until the first build, so a default-constructed state is not mistaken for one. */
        bool built = false;
    };

    /**
     * @brief The Output Log panel: a toolbar and a virtualised, filtered list.
     *
     * Draws in whichever pass the frame is in, like every other widget here.
     *
     * @param frame The frame.
     * @param bounds The panel's content rectangle.
     * @param log The log to show. Not modified — clearing is reported, not done, so the owner of
     *        the log decides.
     * @param state Where to keep the filtered view between frames, or nullptr to recompute it.
     * @return What the user asked for.
     */
    StudioLogPanelResult studioLogPanel(StudioFrame& frame, const UiRect& bounds,
                                        const StudioLog& log,
                                        StudioLogPanelState* state = nullptr);

    /**
     * @brief The colour a severity is drawn in.
     *
     * Exposed so a test can assert that an error does not read as ordinary text without repeating
     * the mapping and thereby being able to agree with itself while both are wrong.
     *
     * @param theme Theme supplying colours.
     * @param severity The severity.
     * @return Its text colour.
     */
    [[nodiscard]] StudioColor studioLogSeverityColor(const StudioTheme& theme, LogSeverity severity);
}
