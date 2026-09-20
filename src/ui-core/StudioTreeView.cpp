// SPDX-License-Identifier: MS-PL
/**
 * @file StudioTreeView.cpp
 * @brief The scrolling, selectable tree.
 */

#include <cstdio>
#include <cstdlib>
#include "CNA/Studio/UiCore/StudioTreeView.hpp"

#include "CNA/Studio/UiCore/StudioWidgets.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Studio
{
    namespace
    {
        /** @brief Returns a metric already scaled to physical pixels. */
        float metricOf(const StudioTheme& theme, StudioMetric metric)
        {
            return static_cast<float>(theme.metric(metric));
        }

        /**
         * @brief Draws a disclosure triangle, pointing right when closed and down when open.
         *
         * Drawn rather than glyphed. Studio has no icon font yet (`STUDIO-04008`), and a triangle
         * is three points: reaching for a typeface to get one would be the wrong trade even once
         * there is a typeface to reach for.
         */
        void drawDisclosure(StudioFrame& frame, const UiRect& box, bool expanded, StudioColor color)
        {
            const float size = std::round(std::min(box.width, box.height) * 0.45f);
            if (size < 2.0f) { return; }

            const float centerX = std::round(box.centerX());
            const float centerY = std::round(box.centerY());

            if (expanded)
            {
                frame.drawList().fillTriangle(centerX - size, centerY - size * 0.5f,
                                             centerX + size, centerY - size * 0.5f,
                                             centerX, centerY + size * 0.7f, color);
            }
            else
            {
                frame.drawList().fillTriangle(centerX - size * 0.5f, centerY - size,
                                             centerX - size * 0.5f, centerY + size,
                                             centerX + size * 0.7f, centerY, color);
            }
        }
    }

    float studioTreeRowHeight(const StudioTheme& theme)
    {
        return std::max(metricOf(theme, StudioMetric::RowHeight),
                        metricOf(theme, StudioMetric::MinimumHitTarget));
    }

    StudioTreeWindow studioTreeWindow(const StudioScrollResult& view, std::size_t totalRows,
                                      const StudioTheme& theme)
    {
        StudioTreeWindow window;
        window.totalRows = totalRows;
        if (totalRows == 0) { return window; }

        std::size_t first = 0;
        std::size_t last = 0;
        view.visibleRows(studioTreeRowHeight(theme), totalRows, first, last);

        window.firstRow = first;
        window.rowCount = last - first;
        return window;
    }

    StudioTreeResult studioTreeView(StudioFrame& frame, const UiRect& bounds,
                                    const std::vector<StudioTreeRow>& rows,
                                    StudioTreeState& state,
                                    std::string_view emptyMessage)
    {
        StudioTreeResult result;
        const StudioTheme& theme = frame.theme();

        if (bounds.width <= 0.0f || bounds.height <= 0.0f) { return result; }

        if (rows.empty())
        {
            if (frame.isDrawPass() && !emptyMessage.empty())
            {
                studioDrawText(frame, bounds.inset(UiEdges{metricOf(theme, StudioMetric::SpacingMedium)}),
                               emptyMessage, StudioFontRole::Body,
                               theme.color(StudioColorRole::TextSecondary));
            }
            return result;
        }

        StudioScrollOptions scroll;
        scroll.contentHeight = static_cast<float>(rows.size()) * studioTreeRowHeight(theme);
        scroll.wheelStep = studioTreeRowHeight(theme) * 3.0f;

        const StudioScrollResult view =
            studioBeginScroll(frame, frame.ids().make("treescroll"), bounds, scroll);

        result = studioTreeRows(frame, view, rows, state);

        studioEndScroll(frame);
        return result;
    }

    StudioTreeResult studioTreeRows(StudioFrame& frame, const StudioScrollResult& view,
                                    const std::vector<StudioTreeRow>& rows,
                                    StudioTreeState& state, const StudioTreeWindow& window)
    {
        StudioTreeResult result;
        const StudioTheme& theme = frame.theme();

        if (rows.empty()) { return result; }

        const float rowHeight = studioTreeRowHeight(theme);
        const float indent = metricOf(theme, StudioMetric::IndentWidth);
        const float padding = metricOf(theme, StudioMetric::SpacingSmall);

        // The whole list, which is what the scroll region was sized from and therefore what the
        // visible range has to be computed against. `index` below is a position in *that*, so a
        // windowed caller's rows land where the scrollbar says they are.
        const std::size_t total = window.isWindowed() ? window.totalRows : rows.size();
        const std::size_t windowFirst = window.isWindowed() ? window.firstRow : 0;

        std::size_t first = 0;
        std::size_t last = 0;
        view.visibleRows(rowHeight, total, first, last);

        // Clamped to what the caller actually built. A window is asked for and then filled, and a
        // caller whose list shrank between the two -- a filter applied on the input pass, a watcher
        // dropping a record -- must get an empty range rather than an index past the end.
        first = std::max(first, windowFirst);
        last = std::min(last, windowFirst + rows.size());
        if (first > last) { first = last; }
        result.rowsDrawn = last - first;

        for (std::size_t index = first; index < last; ++index)
        {
            // Reported in the result, because a windowed caller holds the slice and not the whole.
            const std::size_t slot = index - windowFirst;
            const StudioTreeRow& row = rows[slot];

            const UiRect rowBounds{
                view.viewport.left(),
                std::round(view.viewport.top() - view.offsetY
                           + static_cast<float>(index) * rowHeight),
                view.viewport.width, rowHeight};

            frame.ids().push(row.id);

            const bool renaming = !state.renaming().empty() && state.renaming() == row.id;

            UiRect cursor = rowBounds.inset(UiEdges{padding, 0.0f, padding, 0.0f});
            cursor.splitLeft(std::min(indent * static_cast<float>(row.depth), cursor.width));

            const UiRect disclosure = cursor.splitLeft(std::min(indent, cursor.width));
            bool expanded = state.isExpanded(row.id);
            bool disclosureHovered = false;

            // ### The small controls are described *before* the row, and that is load-bearing
            //
            // `StudioInputRouter` gives a press to the **first** widget described under the
            // pointer: `interact` sets `active_` there and then, and every widget described
            // afterwards at the same point returns an empty interaction until the button comes
            // back up. The row covers the whole line, so anything described after it -- the
            // disclosure triangle, the trailing toggles -- could never be pressed.
            //
            // It could never be pressed. The triangle and the eye were both written with a comment
            // claiming the later of two overlapping widgets wins the click, which is the opposite
            // of what the router does, and neither had a test that pressed one through a frame:
            // every case set the expansion or read the row's fields directly. So expanding a
            // branch by clicking its triangle, and hiding an entity by clicking its eye, were both
            // dead in the editor while their tests passed (`plan.md` STUDIO-13005, found while
            // adding the lock beside the eye).
            //
            // Drawing stays where it was, further down: the row's background is painted over
            // everything above it, and a triangle drawn here vanishes under the alternating fill.
            // It is the *description* that has to come first, not the painting.
            if (row.hasChildren && !renaming)
            {
                // Its own widget, so clicking the triangle opens the row rather than selecting it.
                // Those are different intentions and a tree that conflated them would make it
                // impossible to look inside a group without also selecting it.
                const StudioInteraction toggle =
                    frame.interact(frame.ids().make("disclosure"), disclosure);
                if (frame.isInputPass() && toggle.clicked)
                {
                    expanded = !expanded;
                    state.setExpanded(row.id, expanded);
                    result.toggled = slot;
                }
                // Remembered rather than drawn here, for the reason given above.
                disclosureHovered = toggle.hovered;
            }

            // The trailing toggles, described *before* the row for the reason set out above the
            // disclosure triangle: the row covers the whole line and the first widget described
            // under the pointer takes the press, so a toggle described after it can never be
            // clicked. That is what was wrong with the eye.
            //
            // Interaction here, *drawing* further down: everything below this point paints the
            // row's background over whatever was drawn before it. Drawing a toggle here put the
            // selection, hover and alternating fills straight over the eye, and the symptom was a
            // feature that could not be seen on top of one that could not be clicked.
            //
            // One entry per toggle the row declared, laid out from the right-hand edge inwards in
            // the order they were given, so the same button is in the same column on every row.
            struct DrawnToggle
            {
                UiRect box;
                StudioIcon glyph = StudioIcon::None;
                StudioControlState state = StudioControlState::Normal;

                /** @brief The pointer is on this button. */
                bool selfHovered = false;

                /**
                 * @brief This button is *not* in its default state, so it is shown regardless.
                 *
                 * Kept apart from the row's own hover, which is not known yet: the row is now
                 * described after the toggles, so whether the pointer is anywhere on the line is a
                 * question only the draw pass below can answer.
                 */
                bool alwaysShown = false;
            };
            std::vector<DrawnToggle> drawnToggles;
            drawnToggles.reserve(row.toggles.size());

            for (std::size_t which = 0; which < row.toggles.size(); ++which)
            {
                const StudioRowToggle& entry = row.toggles[which];
                if (entry.icon == StudioIcon::None) { continue; }

                const float size = metricOf(theme, StudioMetric::IconSize);

                // From the right edge, one slot per toggle already placed. Measured rather than
                // accumulated from `cursor`, because `cursor` is also what the label is trimmed
                // against and reading the column positions out of it would tie the two together.
                const float slotWidth = size + padding;
                const float right = rowBounds.right() - padding
                    - static_cast<float>(drawnToggles.size()) * slotWidth;

                DrawnToggle drawn;
                drawn.box = UiRect{right - size, std::round(rowBounds.centerY() - size * 0.5f),
                                   size, size};

                // Described in both passes and at the same size either way, so the hit area does
                // not appear and vanish under the pointer; only the drawing is conditional. A
                // button that existed only while hovered would be one a user cannot click, because
                // the frame in which they press is the frame it was there.
                //
                // Indexed inside a scope of its own: two toggles sharing an id would be a single
                // control that answers about whichever asked last, and a bare index here would
                // collide with the drop targets above, which index by accepted type in the row's
                // own scope. The collision detector reports both, and the scope is what stops it
                // having to.
                frame.ids().push("toggle");
                const WidgetId toggleId = frame.ids().makeIndex(static_cast<std::int64_t>(which));
                frame.ids().pop();

                const StudioInteraction toggle =
                    frame.interact(toggleId, drawn.box, /*enabled=*/true);
                if (std::getenv("CNA_DBG_TOGGLE") != nullptr && frame.isInputPass()
                    && frame.input().mouseX > 100.0f && frame.input().mouseY > 60.0f)
                {
                    std::fprintf(stderr, "T which=%zu box=%.0f,%.0f,%.0f,%.0f m=%.0f,%.0f hov=%d clk=%d\n",
                                 which, drawn.box.x, drawn.box.y, drawn.box.width, drawn.box.height,
                                 frame.input().mouseX, frame.input().mouseY,
                                 (int)toggle.hovered, (int)toggle.clicked);
                }

                if (!entry.tooltip.empty())
                {
                    (void)frame.requestTooltip(toggleId, entry.tooltip, drawn.box);
                }
                if (frame.isInputPass() && toggle.clicked)
                {
                    result.toggledRowAction = slot;
                    result.toggledRowActionIndex = which;
                }

                // Shown only while the row is hovered or the toggle is off, which is what every
                // outliner that has one does: a column of forty identical eyes is a column of
                // noise, and the rows that matter are the ones *not* in the default state.
                drawn.selfHovered = toggle.hovered;
                drawn.alwaysShown = !entry.on;
                drawn.glyph = (!entry.on && entry.offIcon != StudioIcon::None) ? entry.offIcon
                                                                               : entry.icon;
                drawn.state = toggle.held ? StudioControlState::Pressed
                            : toggle.hovered ? StudioControlState::Hover
                                             : StudioControlState::Normal;

                drawnToggles.push_back(drawn);

                // And the label stops where the toggles start, shown or not: text that reflowed as
                // the pointer crossed a row would be the most distracting thing in the panel.
                cursor.splitRight(std::min(cursor.width, slotWidth));
            }


            // Issued once and kept, rather than asked for again where the drag needs it. Two calls
            // return the same id -- it is derived from the scope and the key -- but each one also
            // *records* it, and a key recorded twice is what the collision detector is there to
            // report. It was reporting it: one per draggable row, every frame.
            const WidgetId rowId = frame.ids().make("row");

            // The whole row is the target, not just the text. A tree where a click lands only on
            // the label is a tree with a different hit area on every line. Described last, so the
            // small controls on top of it get first refusal on a press.
            //
            // Except while it is being renamed. A row that is a text field must not also be a
            // selectable, draggable row: clicking to place the caret would reselect, and dragging
            // to select a word would pick the entity up.
            const StudioInteraction interaction = renaming
                ? StudioInteraction{}
                : frame.interact(rowId, rowBounds, row.enabled);

            if (frame.isInputPass() && interaction.clicked && !result.toggled.has_value())
            {
                result.clicked = slot;
                result.additive = frame.input().modifiers.control;
                result.rangeSelect = frame.input().modifiers.shift;
            }

            if (frame.isInputPass() && interaction.rightClicked)
            {
                result.rightClicked = slot;
            }

            // A row that says what it carries can be dragged off. Declared on the row rather than
            // wired up by the caller, so there is no second list to keep in step with these.
            if (!row.dragType.empty() && row.enabled && !renaming)
            {
                StudioFrame::StudioDragPayload payload;
                payload.type = row.dragType;
                payload.value = row.dragValue.empty() ? row.id : row.dragValue;
                payload.label = row.label;
                if (studioDragSource(frame, rowId, interaction, std::move(payload)))
                {
                    result.dragStarted = slot;
                }
            }

            // And a row that says what it accepts is a target. Its own id, because a row is
            // already a control and two interactions sharing one id would be one entry.
            bool dropHovered = false;
            if (!renaming)
            {
                // One target id per accepted type, because two `acceptDrop` calls sharing an id
                // would be one target that answers about whichever type asked last.
                for (std::size_t type = 0; type < row.dropTypes.size(); ++type)
                {
                    const StudioFrame::StudioDropResult drop = frame.acceptDrop(
                        frame.ids().makeIndex(static_cast<std::int64_t>(type)), rowBounds,
                        row.dropTypes[type]);

                    dropHovered = dropHovered || drop.hovered;
                    if (drop.dropped && !result.dropped.has_value())
                    {
                        result.dropped = slot;
                        result.droppedValue = drop.value;
                        result.droppedType = row.dropTypes[type];
                    }
                }
            }

            if (renaming)
            {
                // Over the whole row, indent and all: the field is *where the name is*, so it has
                // to start where the name started or the text jumps sideways as editing begins.
                const UiRect field = cursor;

                const WidgetId id = frame.ids().make("rename");
                if (state.renameStarting())
                {
                    // Focused by the widget rather than by whoever asked for the rename, because a
                    // field the user has to click before typing is a rename that begins by making
                    // them find the thing they just asked to rename.
                    frame.router().setFocus(id);
                    state.clearRenameStarting();
                }

                if (frame.isDrawPass())
                {
                    frame.drawList().fillRect(rowBounds, theme.color(StudioColorRole::Selection));
                }

                StudioTextFieldOptions options;
                options.selectAllOnFocus = true;

                const StudioTextFieldResult edit =
                    studioTextField(frame, id, field, state.renameText(), options);

                if (frame.isInputPass())
                {
                    if (edit.cancelled) { state.cancelRename(); }
                    else if (edit.committed || !edit.interaction.focused)
                    {
                        // Focus leaving commits, the way every other field in Studio does: a user
                        // who typed a name and clicked away meant the name.
                        std::string name = state.renameText();
                        state.cancelRename();
                        if (!name.empty() && name != row.label)
                        {
                            result.renamed = slot;
                            result.renamedTo = std::move(name);
                        }
                    }
                }

                frame.ids().pop();
                continue;
            }

            if (frame.isDrawPass())
            {
                if (dropHovered)
                {
                    // The target says so before the drop, not after: a drag with no feedback is a
                    // drag the user has to complete to discover whether it would have worked.
                    frame.drawList().fillRect(rowBounds, theme.color(StudioColorRole::Selection));
                    frame.drawList().strokeRect(rowBounds, theme.color(StudioColorRole::Accent),
                                                metricOf(theme, StudioMetric::BorderWidth));
                }
                else if (row.selected)
                {
                    frame.drawList().fillRect(rowBounds, theme.color(StudioColorRole::Selection));
                }
                else if (interaction.hovered)
                {
                    frame.drawList().fillRect(rowBounds, theme.color(StudioColorRole::RowHover));
                }
                else if (index % 2 == 1)
                {
                    // `STUDIO-35031`. Every other row, four values off the panel: enough to trace
                    // a row across nine hundred pixels of outliner, not enough to read as a
                    // stripe. Keyed on the row's index in the *model* rather than on its position
                    // on screen, so scrolling does not make the whole list flicker between two
                    // phases -- which is what keying on a visible counter does, and it is far
                    // worse than no striping at all.
                    frame.drawList().fillRect(rowBounds, theme.color(StudioColorRole::RowAlternate));
                }

                // Indent guides, one per level the row is nested under. Drawn under everything
                // else so a selected row covers them: a hierarchy line crossing a selection fill
                // reads as a scratch on the highlight.
                //
                // Only for rows that are actually nested, and never for the level the row itself
                // sits at -- a guide beside a row's own disclosure triangle is a line through the
                // triangle.
                for (int level = 0; level < row.depth; ++level)
                {
                    const float x = std::round(rowBounds.left() + padding
                                               + indent * (static_cast<float>(level) + 0.5f));
                    frame.drawList().fillRect(
                        UiRect{x, rowBounds.top(),
                               metricOf(theme, StudioMetric::SeparatorThickness),
                               rowBounds.height},
                        theme.color(StudioColorRole::Separator));
                }

                // Over every fill above it, which is the whole reason it is drawn here.
                if (row.hasChildren)
                {
                    drawDisclosure(frame, disclosure, expanded,
                                   theme.color(disclosureHovered ? StudioColorRole::TextPrimary
                                                                 : StudioColorRole::TextSecondary));
                }

                // And the toggle, for the same reason. Its own surface only when the pointer is on
                // it: a ghost button's whole point is that it is an icon until it is a target.
                for (const DrawnToggle& drawn : drawnToggles)
                {
                    // Shown only while the row is hovered or the toggle is off, which is what every
                    // outliner that has one does: a column of forty identical eyes is a column of
                    // noise, and the rows that matter are the ones *not* in the default state.
                    const bool visible =
                        interaction.hovered || drawn.selfHovered || drawn.alwaysShown;
                    if (!visible || drawn.glyph == StudioIcon::None) { continue; }

                    if (drawn.state != StudioControlState::Normal)
                    {
                        frame.drawList().fillRoundedRect(
                            drawn.box.inset(UiEdges{-2.0f, -2.0f, -2.0f, -2.0f}),
                            theme.controlBackground(drawn.state),
                            metricOf(theme, StudioMetric::CornerRadius));
                    }
                    studioDrawIcon(frame, drawn.box, drawn.glyph, theme.controlText(drawn.state));
                }

                const StudioColorRole labelRole = (!row.enabled || row.muted)
                    ? StudioColorRole::TextDisabled
                    : StudioColorRole::TextPrimary;

                UiRect labelArea = cursor;
                if (row.icon != StudioIcon::None)
                {
                    // The full icon size rather than the small one. These are read at a glance and
                    // never studied, and twelve pixels is where an isometric cube stops being a
                    // cube -- the interior edges land on the same pixel as the silhouette and it
                    // comes out a grey hexagon. Sixteen fits a 22-pixel row with three to spare.
                    const float iconSize = metricOf(theme, StudioMetric::IconSize);
                    const UiRect iconArea =
                        labelArea.splitLeft(std::min(iconSize + metricOf(theme,
                                                        StudioMetric::SpacingSmall),
                                                     labelArea.width));
                    studioDrawIcon(frame,
                                   UiRect{iconArea.left(),
                                          std::round(iconArea.centerY() - iconSize * 0.5f),
                                          iconSize, iconSize},
                                   row.icon,
                                   theme.color((!row.enabled || row.muted)
                                                   ? StudioColorRole::TextDisabled
                                                   : row.iconRole));
                }
                if (!row.detail.empty())
                {
                    const float detailWidth =
                        std::ceil(studioLabelWidth(frame, row.detail, StudioFontRole::BodySmall)
                                  + metricOf(theme, StudioMetric::SpacingMedium));
                    if (labelArea.width > detailWidth * 2.0f)
                    {
                        const UiRect detailArea = labelArea.splitRight(detailWidth);
                        studioDrawText(frame, detailArea, row.detail, StudioFontRole::BodySmall,
                                       theme.color(row.enabled ? row.detailRole
                                                               : StudioColorRole::TextDisabled),
                                       StudioTextAlign::Right);
                    }
                }

                studioDrawText(frame, labelArea,
                               studioTruncateText(frame, theme.font(StudioFontRole::Body),
                                                  row.label, labelArea.width),
                               StudioFontRole::Body, theme.color(labelRole));

                if (interaction.focused)
                {
                    frame.drawList().drawFocusRing(rowBounds,
                                                   theme.color(StudioColorRole::FocusRing),
                                                   metricOf(theme, StudioMetric::FocusRingWidth));
                }
            }

            frame.ids().pop();
        }

        return result;
    }
}
