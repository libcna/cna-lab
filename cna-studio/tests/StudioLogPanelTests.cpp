// SPDX-License-Identifier: MS-PL
/**
 * @file StudioLogPanelTests.cpp
 * @brief The first panel ported off Dear ImGui, and the machinery it needed (plan.md STUDIO-07005).
 *
 * Three things are under test here, because porting the first panel is what forced all three into
 * existence: a log model no UI owns, a scrolling region, and the shell's seam for panel content.
 * Each is checked on its own before the panel that uses them, so a failure says which one broke.
 *
 * The cases drive the real widgets through both frame passes and assert on behaviour, not pixels.
 * "Does clicking Errors hide the info lines" and "does a hundred-thousand-line log still describe
 * only a screenful" are the questions that decide whether this panel is usable; a golden image
 * answers neither.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Ui/StudioLog.hpp"
#include "CNA/Studio/UiCore/StudioLogPanel.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"
#include "CNA/Studio/UiCore/StudioWidgets.hpp"
#include "CNA/Studio/Ui/StudioUi.hpp"

#include <functional>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    constexpr float kWidth = 1280.0f;
    constexpr float kHeight = 720.0f;

    /** @brief How much geometry a draw produced, across every list. */
    std::size_t vertexCount(const UiDrawData& data)
    {
        std::size_t total = 0;
        for (const UiDrawList& list : data.lists) { total += list.vertices.size(); }
        return total;
    }

    UiInputState at(float x, float y, bool leftDown = false)
    {
        UiInputState input;
        input.displayWidth = kWidth;
        input.displayHeight = kHeight;
        input.mouseX = x;
        input.mouseY = y;
        input.mouseInWindow = true;
        input.setMouseDown(UiMouseButton::Left, leftDown);
        return input;
    }

    /** @brief A shell with its default workspace and the Output Log raised. */
    std::unique_ptr<StudioShell> shellShowingTheLog()
    {
        auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
        shell->resetLayout();
        shell->renderFrame(at(-1.0f, -1.0f));
        CNA_STUDIO_EXPECT(shell->activatePanel("output"));
        return shell;
    }
}

// -------------------------------------------------------------------------------------------
// The log model
// -------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(ARepeatedMessageIsOneEntryWithACount)
{
    // Four hundred identical lines is one thing that happened four hundred times. Collapsing keeps
    // the interesting lines on screen, which is the entire purpose of a console.
    StudioLog log;
    for (int i = 0; i < 400; ++i) { log.append(LogSeverity::Warning, "texture not found"); }

    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(log.entries().front().repeats, std::size_t{400});

    // And nothing is hidden: the count reaches the clipboard too.
    CNA_STUDIO_EXPECT(log.toText().find("(x400)") != std::string::npos);

    // Only *consecutive* repeats collapse. Two failures with something in between are two events,
    // and merging them would misreport the order things happened in.
    log.append(LogSeverity::Info, "loaded scene");
    log.append(LogSeverity::Warning, "texture not found");
    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{3});
}

CNA_STUDIO_TEST(TheLogIsBoundedAndSaysHowMuchItDropped)
{
    // An editor left open for a day with a noisy import can produce millions of lines. A log that
    // grew without limit would turn that into an out-of-memory crash that loses the user's scene.
    StudioLog log{16};
    for (int i = 0; i < 100; ++i) { log.append(LogSeverity::Info, "line " + std::to_string(i)); }

    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{16});
    CNA_STUDIO_EXPECT_EQ(log.droppedCount(), std::size_t{84});

    // The *most recent* lines are the ones kept: a console that discarded the newest messages to
    // preserve the oldest would be exactly backwards.
    CNA_STUDIO_EXPECT_EQ(log.entries().back().message, std::string{"line 99"});
    CNA_STUDIO_EXPECT_EQ(log.entries().front().message, std::string{"line 84"});

    // Clearing resets the dropped count as well: the history the user is now looking at is
    // complete, and saying otherwise would be false.
    log.clear();
    CNA_STUDIO_EXPECT_EQ(log.droppedCount(), std::size_t{0});
}

CNA_STUDIO_TEST(FilteringCountsAndTextAgreeWithEachOther)
{
    StudioLog log;
    log.append(LogSeverity::Trace, "a");
    log.append(LogSeverity::Info, "b");
    log.append(LogSeverity::Warning, "c");
    log.append(LogSeverity::Error, "d");

    CNA_STUDIO_EXPECT_EQ(log.countAtLeast(LogSeverity::Trace), std::size_t{4});
    CNA_STUDIO_EXPECT_EQ(log.countAtLeast(LogSeverity::Warning), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(log.countAtLeast(LogSeverity::Error), std::size_t{1});

    // Text pasted into a bug report must say what the panel said. A Copy that ignored the filter
    // would attach a hundred trace lines to a report about one error.
    const std::string errorsOnly = log.toText(LogSeverity::Error);
    CNA_STUDIO_EXPECT(errorsOnly.find('d') != std::string::npos);
    CNA_STUDIO_EXPECT(errorsOnly.find('a') == std::string::npos);
    CNA_STUDIO_EXPECT(errorsOnly.find('c') == std::string::npos);
}

// -------------------------------------------------------------------------------------------
// The panel
// -------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(TheOutputLogDrawsItsMessagesThroughTheShell)
{
    StudioLog log;
    log.append(LogSeverity::Info, "the scene loaded");
    log.append(LogSeverity::Error, "a shader would not compile");

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    bool drew = false;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            drew = true;
            CNA_STUDIO_EXPECT(bounds.width > 0.0f);
            CNA_STUDIO_EXPECT(bounds.height > 0.0f);
            (void)studioLogPanel(frame, bounds, log);
        }));

    const std::size_t before = vertexCount(shell->drawData());
    shell->renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT(drew);
    // Geometry, not a flag: a content function that ran and drew nothing would set `drew` just as
    // happily as one that drew the log.
    CNA_STUDIO_EXPECT(vertexCount(shell->drawData()) > before);
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(APanelWithNoContentCostsNothingAndDrawsAnEmptySurface)
{
    // Every panel starts here, and most of them stay here until Phase 7 reaches them. An unported
    // panel must not be a broken one.
    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();
    shell->renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT(vertexCount(shell->drawData()) > 0);
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});

    // Content can be given and taken away again without the shell minding.
    CNA_STUDIO_EXPECT(shell->setPanelContent("output", {}));
    CNA_STUDIO_EXPECT(!shell->setPanelContent("no.such.panel",
        [](StudioFrame&, const UiRect&) {}));
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(ClearingTheLogIsReportedRatherThanDoneBehindTheOwnersBack)
{
    // The panel is handed a const log. Clearing is the owner's decision because the owner may be
    // writing to it from somewhere the panel knows nothing about.
    StudioLog log;
    log.append(LogSeverity::Info, "something happened");

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    UiRect clearBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log);
            if (result.cleared) { log.clear(); }
            if (frame.isDrawPass()) { clearBounds = bounds; }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!clearBounds.isEmpty());
    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{1});

    // Clear sits second in the toolbar, after Copy. Found by walking the top row rather than by
    // hard-coding a pixel, so a spacing change does not silently make this test press nothing.
    bool clearedIt = false;
    for (float x = clearBounds.left(); x < clearBounds.left() + 260.0f && !clearedIt; x += 4.0f)
    {
        const float y = clearBounds.top() + 12.0f;
        shell->renderFrame(at(x, y, true));
        shell->renderFrame(at(x, y, false));
        clearedIt = log.entries().empty();
    }
    CNA_STUDIO_EXPECT(clearedIt);
}

CNA_STUDIO_TEST(TheSeverityOfALineIsVisibleWithoutReadingIt)
{
    // Colour is the only thing that separates an error from a trace at a glance, and a console
    // where they look the same is a console nobody scans.
    const StudioTheme dark = StudioTheme::dark();

    const StudioColor error = studioLogSeverityColor(dark, LogSeverity::Error);
    const StudioColor warning = studioLogSeverityColor(dark, LogSeverity::Warning);
    const StudioColor info = studioLogSeverityColor(dark, LogSeverity::Info);
    const StudioColor trace = studioLogSeverityColor(dark, LogSeverity::Trace);

    const auto differs = [](const StudioColor& a, const StudioColor& b) {
        return a.r != b.r || a.g != b.g || a.b != b.b;
    };

    CNA_STUDIO_EXPECT(differs(error, info));
    CNA_STUDIO_EXPECT(differs(warning, info));
    CNA_STUDIO_EXPECT(differs(error, warning));
    CNA_STUDIO_EXPECT(differs(trace, info));

    // And the same holds in the light theme, which is where a colour chosen by eye on a dark
    // background usually stops working.
    const StudioTheme light = StudioTheme::light();
    CNA_STUDIO_EXPECT(differs(studioLogSeverityColor(light, LogSeverity::Error),
                              studioLogSeverityColor(light, LogSeverity::Info)));
    CNA_STUDIO_EXPECT(differs(studioLogSeverityColor(light, LogSeverity::Warning),
                              studioLogSeverityColor(light, LogSeverity::Info)));
}

CNA_STUDIO_TEST(AHugeLogCostsTheSameAsASmallOne)
{
    // The property that decides whether this panel is usable at all. A console that laid out every
    // line it holds would stall the editor the moment an import went wrong, which is precisely when
    // somebody is reading it.
    StudioLog big{200000};
    for (int i = 0; i < 100000; ++i)
    {
        big.append(LogSeverity::Info, "line " + std::to_string(i));
    }

    StudioLog small;
    small.append(LogSeverity::Info, "line 0");

    const auto verticesFor = [](const StudioLog& log) {
        auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
        shell->resetLayout();
        shell->renderFrame(at(-1.0f, -1.0f));
        (void)shell->activatePanel("output");
        (void)shell->setPanelContent("output",
            [&log](StudioFrame& frame, const UiRect& bounds) {
                (void)studioLogPanel(frame, bounds, log);
            });
        shell->renderFrame(at(-1.0f, -1.0f));
        return vertexCount(shell->drawData());
    };

    const std::size_t hundredThousand = verticesFor(big);
    const std::size_t one = verticesFor(small);

    CNA_STUDIO_EXPECT(hundredThousand > one);

    // Bounded by the screen, not by the log. Ten times the geometry of a one-line log is already
    // generous for a panel a few dozen rows tall; a hundred thousand times would be the bug.
    CNA_STUDIO_EXPECT(hundredThousand < one * 40);
}

// -------------------------------------------------------------------------------------------
// Scrolling
// -------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(AScrollViewOnlyGrowsABarWhenTheContentDoesNotFit)
{
    StudioFrame frame{StudioTheme::dark()};

    const UiRect bounds{0.0f, 0.0f, 400.0f, 200.0f};
    const auto run = [&](float contentHeight) {
        StudioScrollResult result;
        runStudioFrame(frame, at(-1.0f, -1.0f), [&](StudioFrame& f) {
            StudioScrollOptions options;
            options.contentHeight = contentHeight;
            result = studioBeginScroll(f, f.ids().make("view"), bounds, options);
            studioEndScroll(f);
        });
        return result;
    };

    const StudioScrollResult fits = run(100.0f);
    CNA_STUDIO_EXPECT(!fits.hasVerticalBar);
    // The whole width goes to the content: a scrollbar gutter reserved for a bar that is not there
    // is a column of wasted space on every panel that happens to fit.
    CNA_STUDIO_EXPECT_EQ(fits.viewport.width, bounds.width);
    CNA_STUDIO_EXPECT_EQ(fits.offsetY, 0.0f);

    const StudioScrollResult overflows = run(1000.0f);
    CNA_STUDIO_EXPECT(overflows.hasVerticalBar);
    CNA_STUDIO_EXPECT(overflows.viewport.width < bounds.width);
}

CNA_STUDIO_TEST(TheWheelScrollsAndTheViewStopsAtBothEnds)
{
    StudioFrame frame{StudioTheme::dark()};
    const UiRect bounds{0.0f, 0.0f, 400.0f, 200.0f};

    const auto turn = [&](float wheel) {
        UiInputState input = at(bounds.centerX(), bounds.centerY());
        input.wheelY = wheel;

        StudioScrollResult result;
        runStudioFrame(frame, input, [&](StudioFrame& f) {
            StudioScrollOptions options;
            options.contentHeight = 1000.0f;
            result = studioBeginScroll(f, f.ids().make("view"), bounds, options);
            studioEndScroll(f);
        });
        return result;
    };

    CNA_STUDIO_EXPECT_EQ(turn(0.0f).offsetY, 0.0f);

    const float afterOneNotch = turn(-1.0f).offsetY;
    CNA_STUDIO_EXPECT(afterOneNotch > 0.0f);

    // Rolling to the end stops there rather than scrolling into empty space below the content.
    for (int i = 0; i < 50; ++i) { (void)turn(-1.0f); }
    const StudioScrollResult atEnd = turn(0.0f);
    CNA_STUDIO_EXPECT(atEnd.atEnd);
    CNA_STUDIO_EXPECT_EQ(atEnd.offsetY, 1000.0f - bounds.height);

    // And back, stopping at zero rather than at a negative offset.
    for (int i = 0; i < 100; ++i) { (void)turn(1.0f); }
    CNA_STUDIO_EXPECT_EQ(turn(0.0f).offsetY, 0.0f);
}

CNA_STUDIO_TEST(FollowingNewOutputStopsTheMomentTheUserScrollsAway)
{
    // The single most common complaint about log windows: one that yanks the view back to the
    // bottom while somebody is reading further up. "Auto-scroll" has never meant "take the
    // scrollbar away from me".
    StudioFrame frame{StudioTheme::dark()};
    const UiRect bounds{0.0f, 0.0f, 400.0f, 200.0f};

    float contentHeight = 400.0f;
    const auto grow = [&](float wheel) {
        UiInputState input = at(bounds.centerX(), bounds.centerY());
        input.wheelY = wheel;

        StudioScrollResult result;
        runStudioFrame(frame, input, [&](StudioFrame& f) {
            StudioScrollOptions options;
            options.contentHeight = contentHeight;
            options.stickToEnd = true;
            result = studioBeginScroll(f, f.ids().make("view"), bounds, options);
            studioEndScroll(f);
        });
        contentHeight += 100.0f;
        return result;
    };

    // Starting at the end, growth keeps it there.
    (void)grow(0.0f);
    for (int i = 0; i < 5; ++i)
    {
        CNA_STUDIO_EXPECT(grow(0.0f).atEnd);
    }

    // Scroll up, and growth must leave the view where the reader put it.
    const float parked = grow(3.0f).offsetY;
    CNA_STUDIO_EXPECT(!grow(0.0f).atEnd);
    CNA_STUDIO_EXPECT_EQ(grow(0.0f).offsetY, parked);
}

CNA_STUDIO_TEST(OnlyTheRowsOnScreenAreWorthDescribing)
{
    StudioScrollResult result;
    result.viewport = UiRect{0.0f, 0.0f, 400.0f, 200.0f};
    result.offsetY = 0.0f;

    std::size_t first = 0;
    std::size_t last = 0;

    result.visibleRows(20.0f, 100000, first, last);
    CNA_STUDIO_EXPECT_EQ(first, std::size_t{0});
    // Ten rows of viewport plus a row of slack at each end, so an edge row half out of view is
    // still described and the list does not pop as the offset crosses a boundary.
    CNA_STUDIO_EXPECT_EQ(last, std::size_t{12});

    result.offsetY = 1000.0f;
    result.visibleRows(20.0f, 100000, first, last);
    CNA_STUDIO_EXPECT_EQ(first, std::size_t{50});
    CNA_STUDIO_EXPECT_EQ(last, std::size_t{62});

    // Past the end, and an empty list, must both come back empty rather than wrapping around.
    result.offsetY = 100000.0f * 20.0f;
    result.visibleRows(20.0f, 100000, first, last);
    CNA_STUDIO_EXPECT_EQ(first, last);

    result.offsetY = 0.0f;
    result.visibleRows(20.0f, 0, first, last);
    CNA_STUDIO_EXPECT_EQ(first, std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(last, std::size_t{0});
}

// -------------------------------------------------------------------------------------------
// The migration seam
// -------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(BothConsolesReadOneLog)
{
    // The property that makes this a *port* rather than a second console showing something else.
    // Two logs would make the migration impossible to check: every difference between the panels
    // would be a difference in what was logged rather than in how it was drawn, and nobody could
    // tell a faithful port from a plausible-looking one.
    NullStudioUi legacy;
    legacy.log(LogSeverity::Info, "the scene loaded");
    legacy.log(LogSeverity::Error, "a shader would not compile");

    // What the legacy UI collected is exactly what the Studio panel is handed.
    const StudioLog& shared = legacy.getLogModel();
    CNA_STUDIO_EXPECT_EQ(shared.entries().size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(shared.countAtLeast(LogSeverity::Error), std::size_t{1});

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();
    std::size_t rowsSeen = 0;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            (void)studioLogPanel(frame, bounds, shared);
            if (frame.isDrawPass()) { rowsSeen = shared.entries().size(); }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(rowsSeen, std::size_t{2});

    // And clearing through either reaches both, because there is only one.
    legacy.clearLog();
    CNA_STUDIO_EXPECT(shared.entries().empty());

    // The legacy console's own accessors keep working on it, unchanged: the ImGui panels are a
    // compatibility fallback until they are deleted, not something to break on the way past.
    CNA_STUDIO_EXPECT(legacy.getLog().empty());
    legacy.log(LogSeverity::Warning, "still here");
    CNA_STUDIO_EXPECT_EQ(legacy.getLog().size(), std::size_t{1});
    CNA_STUDIO_EXPECT(legacy.getLogText().find("still here") != std::string::npos);
}

// -------------------------------------------------------------------------------------------
// Source, time and links on the model (plan.md STUDIO-27020)
// -------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(ALineRemembersWhenItArrivedAndWhoSaidIt)
{
    StudioLog log;

    log.setNow(1.5);
    log.append(LogSeverity::Info, "the scene loaded");

    log.setNow(12.25);
    log.append(LogSeverity::Error, LogSource::Game, "null reference");

    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(log.entries()[0].source == LogSource::Studio);
    CNA_STUDIO_EXPECT(log.entries()[0].timeSeconds == 1.5);
    CNA_STUDIO_EXPECT(log.entries()[1].source == LogSource::Game);
    CNA_STUDIO_EXPECT(log.entries()[1].timeSeconds == 12.25);

    // Told, not read. A log that asked a clock would need this test to sleep to make two
    // timestamps differ, and a test that sleeps is a test that fails on a loaded machine.
    CNA_STUDIO_EXPECT(log.now() == 12.25);

    // The default source is Studio, so the hundred-odd existing call sites mean what they meant.
    CNA_STUDIO_EXPECT_EQ(log.countFrom(LogSource::Studio), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(log.countFrom(LogSource::Game), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(log.countFrom(LogSource::Build), std::size_t{0});
}

CNA_STUDIO_TEST(TwoSourcesSayingTheSameWordsAreTwoEntries)
{
    StudioLog log;
    log.append(LogSeverity::Warning, LogSource::Build, "out of memory");
    log.append(LogSeverity::Warning, LogSource::Game, "out of memory");

    // Not collapsed: a build running out of memory and a game running out of memory are two
    // events, and merging them would hide exactly the distinction a source filter exists for.
    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{2});

    // The same source, though, still collapses -- that is the behaviour this must not have broken.
    log.append(LogSeverity::Warning, LogSource::Game, "out of memory");
    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(log.entries().back().repeats, std::size_t{2});

    // And two lines pointing at different things are two lines however alike they read.
    StudioLogLink first;
    first.kind = StudioLogLink::Kind::File;
    first.path = "Content/a.png";
    StudioLogLink second = first;
    second.path = "Content/b.png";

    log.append(LogSeverity::Error, LogSource::Studio, "could not import", first);
    log.append(LogSeverity::Error, LogSource::Studio, "could not import", second);
    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{4});
}

CNA_STUDIO_TEST(ACollapsedRepeatKeepsTheTimeItStarted)
{
    StudioLog log;
    log.setNow(4.0);
    log.append(LogSeverity::Warning, "a shader would not compile");

    log.setNow(40.0);
    log.append(LogSeverity::Warning, "a shader would not compile");

    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(log.entries().front().repeats, std::size_t{2});

    // "This started at 4 s and has happened twice" is the useful reading. The last occurrence is
    // always approximately now, so stamping it with 40 would be recording nothing.
    CNA_STUDIO_EXPECT(log.entries().front().timeSeconds == 4.0);
}

CNA_STUDIO_TEST(PastedLogTextCarriesTheTimeAndTheSource)
{
    StudioLog log;
    log.setNow(2.5);
    log.append(LogSeverity::Error, LogSource::Game, "null reference");

    const std::string line = StudioLog::toTextLine(log.entries().front());
    CNA_STUDIO_EXPECT(line.find("2.500") != std::string::npos);
    CNA_STUDIO_EXPECT(line.find("[error]") != std::string::npos);
    CNA_STUDIO_EXPECT(line.find("[game]") != std::string::npos);
    CNA_STUDIO_EXPECT(line.find("null reference") != std::string::npos);

    // A pasted log is read by somebody who was not there: without the time and the source it is a
    // list of sentences, and most bug reports turn on which of them came first and who said it.
    CNA_STUDIO_EXPECT_EQ(log.toText(), line + "\n");
}

// -------------------------------------------------------------------------------------------
// The panel's new controls (plan.md STUDIO-27020)
// -------------------------------------------------------------------------------------------

namespace
{
    /** @brief One press and release at a point, driving both passes each time. */
    void clickAt(StudioShell& shell, float x, float y)
    {
        shell.renderFrame(at(x, y, true));
        shell.renderFrame(at(x, y, false));
    }

    /**
     * @brief Walks a toolbar row **right to left**, clicking, until @p done says to stop.
     *
     * Found by walking rather than by a hard-coded pixel, the way the Clear case does: a spacing
     * change should move a button, not silently make a test press nothing.
     *
     * Right to left because the controls this reaches -- Pause, Follow, the source filters -- sit
     * at the right, and Clear sits at the left. A sweep that started at the left would empty the
     * log on its way past and then assert about a console with nothing in it, which is a test that
     * passes for the wrong reason or fails for one.
     *
     * @return Whether it ever became true.
     */
    bool clickAlongRow(StudioShell& shell, const UiRect& panel, float rowOffsetY,
                       const std::function<bool()>& done)
    {
        for (float x = panel.right() - 2.0f; x > panel.left() + 2.0f; x -= 4.0f)
        {
            clickAt(shell, x, panel.top() + rowOffsetY);
            if (done()) { return true; }
        }
        return false;
    }
}

CNA_STUDIO_TEST(PauseHoldsTheViewAndNeverHoldsTheLog)
{
    StudioLog log;
    log.append(LogSeverity::Info, "before the pause");

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    StudioLogPanelResult last;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log);
            if (frame.isDrawPass())
            {
                last = result;
                panelBounds = bounds;
            }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!panelBounds.isEmpty());
    CNA_STUDIO_EXPECT(!last.paused);
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{1});

    // Pause sits at the right of the first toolbar row, beside Follow.
    const bool pressed =
        clickAlongRow(*shell, panelBounds, 12.0f, [&] { return last.paused; });
    CNA_STUDIO_EXPECT(pressed);

    // Now the thing that matters: messages keep being *recorded*, and stop being *shown*.
    log.append(LogSeverity::Error, "while paused");
    log.append(LogSeverity::Error, "also while paused");
    shell->renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(last.heldBackCount, std::size_t{2});

    // And unpausing shows them, rather than having thrown them away. That is the whole difference
    // between a pause and a mute.
    const bool released =
        clickAlongRow(*shell, panelBounds, 12.0f, [&] { return !last.paused; });
    CNA_STUDIO_EXPECT(released);

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(last.heldBackCount, std::size_t{0});
}

CNA_STUDIO_TEST(TheSourceFilterHidesAGameWithoutMatchingStrings)
{
    StudioLog log;
    log.append(LogSeverity::Info, LogSource::Studio, "the scene loaded");
    log.append(LogSeverity::Info, LogSource::Game, "the game started");
    log.append(LogSeverity::Info, LogSource::Build, "the build started");

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    StudioLogPanelResult last;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log);
            if (frame.isDrawPass())
            {
                last = result;
                panelBounds = bounds;
            }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));

    // Everything, until somebody says otherwise. A filter that starts excluding things hides
    // output from a user who never asked it to.
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{3});

    // The source buttons sit on the second toolbar row, at its right.
    const bool narrowed =
        clickAlongRow(*shell, panelBounds, 40.0f, [&] { return last.rowsMatching < 3; });
    CNA_STUDIO_EXPECT(narrowed);
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{2});
}

CNA_STUDIO_TEST(AConsoleLineThatPointsAtSomethingReportsItRatherThanOpeningIt)
{
    StudioLog log;
    log.append(LogSeverity::Info, "an ordinary line");

    StudioLogLink link;
    link.kind = StudioLogLink::Kind::Asset;
    link.id = Uuid::generate();
    log.append(LogSeverity::Error, LogSource::Studio, "could not import 'Content/Hero.png'", link);

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    StudioLogPanelResult activated;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log);
            if (result.linkActivated) { activated = result; }
            if (frame.isDrawPass()) { panelBounds = bounds; }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!panelBounds.isEmpty());

    // Down the list, which starts below the two toolbar rows. The linked row is the second one.
    bool found = false;
    for (float y = panelBounds.top() + 56.0f; y < panelBounds.bottom() - 2.0f && !found; y += 4.0f)
    {
        clickAt(*shell, panelBounds.left() + 320.0f, y);
        found = activated.linkActivated;
    }

    CNA_STUDIO_EXPECT(found);
    CNA_STUDIO_EXPECT(activated.link.kind == StudioLogLink::Kind::Asset);
    CNA_STUDIO_EXPECT_EQ(activated.link.id.toString(), link.id.toString());

    // Reported, not acted on. The panel is handed a log and nothing else -- no scene, no asset
    // database -- and a panel that could select an asset would be a panel that needed both.
    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{2});
}

CNA_STUDIO_TEST(AnOrdinaryLineIsNotAccidentallyALink)
{
    StudioLog log;
    log.append(LogSeverity::Info, "an ordinary line mentioning Content/Hero.png in passing");

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    bool everActivated = false;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log);
            if (result.linkActivated) { everActivated = true; }
            if (frame.isDrawPass()) { panelBounds = bounds; }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));

    // A console that scanned its own text for things that look like paths would find this one, in
    // prose, and navigate somewhere the user did not ask to go. Links are attached by whoever
    // logs, and this line attached none.
    for (float y = panelBounds.top() + 56.0f; y < panelBounds.bottom() - 2.0f; y += 6.0f)
    {
        clickAt(*shell, panelBounds.left() + 320.0f, y);
    }
    CNA_STUDIO_EXPECT(!everActivated);
}

CNA_STUDIO_TEST(SearchingTheConsoleIgnoresCaseAndMatchesAnywhereInTheLine)
{
    StudioLog log;
    log.append(LogSeverity::Info, "the scene loaded");
    log.append(LogSeverity::Error, "a SHADER would not compile");
    log.append(LogSeverity::Info, "recompiled the shader");

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    StudioLogPanelResult last;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log);
            if (frame.isDrawPass())
            {
                last = result;
                panelBounds = bounds;
            }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{3});

    // The field fills the left of the second toolbar row, so a click well inside the left edge
    // lands in it whatever the source buttons at the right are sized at.
    const float fieldX = panelBounds.left() + 40.0f;
    const float fieldY = panelBounds.top() + 40.0f;
    shell->renderFrame(at(fieldX, fieldY));
    shell->renderFrame(at(fieldX, fieldY, true));
    shell->renderFrame(at(fieldX, fieldY));

    UiInputState typing = at(fieldX, fieldY);
    typing.characters = {u'S', u'h', u'A', u'd'};
    shell->renderFrame(typing);

    UiInputState enter = at(fieldX, fieldY);
    enter.setKeyDown(UiKey::Enter, true);
    shell->renderFrame(enter);

    shell->renderFrame(at(-1.0f, -1.0f));

    // Two of the three: the one spelling it in capitals and the one spelling it in lower case,
    // matched mid-word in both. A case-sensitive search would find one, and a prefix search none.
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{2});
}

CNA_STUDIO_TEST(CopyTakesWhatThePanelIsShowingRatherThanTheWholeLog)
{
    StudioLog log;
    log.append(LogSeverity::Info, LogSource::Studio, "the scene loaded");
    log.append(LogSeverity::Error, LogSource::Game, "null reference");

    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    std::string copied;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log);
            if (result.copyRequested) { copied = result.copyText; }
            if (frame.isDrawPass()) { panelBounds = bounds; }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));

    // Narrow to the editor's line by turning Game off. The source buttons run Studio, Build,
    // Game left to right, so a sweep from the right reaches Game first.
    StudioLogPanelResult last;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log);
            if (result.copyRequested) { copied = result.copyText; }
            if (frame.isDrawPass()) { last = result; }
        }));
    CNA_STUDIO_EXPECT(clickAlongRow(*shell, panelBounds, 40.0f,
                                    [&] { return last.rowsMatching == 1; }));

    // Copy is the first control on the top row, so this sweep starts at the left deliberately --
    // and Clear sits immediately after it, which is why it stops the moment Copy fires.
    for (float x = panelBounds.left() + 2.0f; x < panelBounds.left() + 200.0f && copied.empty();
         x += 3.0f)
    {
        clickAt(*shell, x, panelBounds.top() + 12.0f);
    }

    CNA_STUDIO_EXPECT(!copied.empty());
    CNA_STUDIO_EXPECT(copied.find("the scene loaded") != std::string::npos);

    // The filtered-out line is absent. A Copy that pasted the whole log would make the filter a
    // decoration: the reason somebody narrows a console is usually that they are about to copy it.
    CNA_STUDIO_EXPECT(copied.find("null reference") == std::string::npos);

    // And it is spelled the way a pasted log is spelled, time and source included.
    CNA_STUDIO_EXPECT(copied.find("[studio]") != std::string::npos);
}

// -------------------------------------------------------------------------------------------
// Very large logs (plan.md STUDIO-27021)
// -------------------------------------------------------------------------------------------

namespace
{
    /** @brief A log of @p count lines, a tenth of which are errors. */
    StudioLog aVeryLargeLog(int count)
    {
        StudioLog log{static_cast<std::size_t>(count) * 2};
        for (int i = 0; i < count; ++i)
        {
            log.setNow(static_cast<double>(i) / 60.0);
            log.append(i % 10 == 0 ? LogSeverity::Error : LogSeverity::Info,
                       "line " + std::to_string(i));
        }
        return log;
    }
}

CNA_STUDIO_TEST(AnUnfilteredConsoleLooksAtNoEntriesAtAllHoweverLargeTheLogIs)
{
    // The state a console is in almost all of the time. Every entry is visible, row r is entry r,
    // and there is nothing to work out -- so the answer must not depend on the size of the log.
    StudioLog big = aVeryLargeLog(200000);
    StudioLog small;
    small.append(LogSeverity::Info, "line 0");

    const auto examinedFor = [](const StudioLog& log) {
        StudioLogPanelState state;
        auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
        shell->resetLayout();
        shell->renderFrame(at(-1.0f, -1.0f));
        (void)shell->activatePanel("output");

        // Summed across both passes, because that is the work one *frame* does. The scan now
        // happens on the input pass and the draw pass reuses it, so reading either alone would
        // measure half the answer -- and would read zero for the half that does no work.
        std::size_t examined = 0;
        (void)shell->setPanelContent("output",
            [&](StudioFrame& frame, const UiRect& bounds) {
                examined += studioLogPanel(frame, bounds, log, &state).entriesExamined;
            });
        shell->renderFrame(at(-1.0f, -1.0f));
        return examined;
    };

    CNA_STUDIO_EXPECT_EQ(examinedFor(big), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(examinedFor(small), std::size_t{0});
}

CNA_STUDIO_TEST(AFilteredVeryLargeLogIsScannedOnceAndThenOnlyExtended)
{
    // Counted rather than timed, like everything else here: a wall-clock assertion on a shared
    // machine fails for reasons that have nothing to do with the code. What a regression would
    // actually move is how many entries the filter walks, so that is what this asserts.
    StudioLog log = aVeryLargeLog(200000);

    StudioLogPanelState state;
    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    StudioLogPanelResult last;
    UiRect panelBounds;
    std::size_t examined = 0;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log, &state);
            examined += result.entriesExamined;
            if (frame.isDrawPass())
            {
                last = result;
                panelBounds = bounds;
            }
        }));

    // Per frame, across both passes: the scan happens on the input pass and the draw pass reuses
    // it, so either alone measures half of what a frame costs.
    const auto frameExamining = [&] {
        examined = 0;
        shell->renderFrame(at(-1.0f, -1.0f));
        return examined;
    };

    CNA_STUDIO_EXPECT_EQ(frameExamining(), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{200000});

    // Engage a filter. Errors is the rightmost severity button, so a sweep from the right reaches
    // it before the others -- and before Clear, which sits at the far left.
    CNA_STUDIO_EXPECT(clickAlongRow(*shell, panelBounds, 12.0f,
                                    [&] { return last.rowsMatching < 200000; }));
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{20000});

    // Every frame after it pays nothing, because the answer is kept rather than recomputed. The
    // pass that engaged the filter paid for the whole log once, which is inherent -- nothing can
    // know which lines match without looking at them -- and it is the only pass that does.
    CNA_STUDIO_EXPECT_EQ(frameExamining(), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{20000});
    CNA_STUDIO_EXPECT_EQ(frameExamining(), std::size_t{0});

    // A log that grows costs the lines it gained, not the lines it holds. This is the case that
    // matters: a console filtered to errors while an import spews warnings must not re-walk two
    // hundred thousand entries sixty times a second.
    for (int i = 0; i < 5; ++i)
    {
        log.append(LogSeverity::Error, "a new error " + std::to_string(i));
    }

    CNA_STUDIO_EXPECT_EQ(frameExamining(), std::size_t{5});
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{20005});
}

CNA_STUDIO_TEST(TheCacheSurvivesTheLogDroppingItsOldestAndIsThrownAwayByAClear)
{
    // The two things that can make a cached answer wrong. Dropping shifts every relative index, so
    // the cache holds absolute ones; a clear leaves it describing a log that no longer exists.
    StudioLog log{100};
    for (int i = 0; i < 100; ++i)
    {
        log.append(i % 2 == 0 ? LogSeverity::Error : LogSeverity::Info, "line " + std::to_string(i));
    }

    StudioLogPanelState state;
    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    StudioLogPanelResult last;
    UiRect panelBounds;
    std::size_t examined = 0;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log, &state);
            examined += result.entriesExamined;
            if (frame.isDrawPass())
            {
                last = result;
                panelBounds = bounds;
            }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(clickAlongRow(*shell, panelBounds, 12.0f,
                                    [&] { return last.rowsMatching < 100; }));
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{50});

    // Twenty more at capacity pushes twenty off the front -- ten of which were errors.
    for (int i = 100; i < 120; ++i)
    {
        log.append(i % 2 == 0 ? LogSeverity::Error : LogSeverity::Info, "line " + std::to_string(i));
    }
    examined = 0;
    shell->renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{100});
    CNA_STUDIO_EXPECT_EQ(log.droppedCount(), std::size_t{20});
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{50});

    // Only the twenty that arrived were looked at. A cache that re-walked on every drop would say
    // a hundred here, and one that did not trim its front would say the wrong count above.
    CNA_STUDIO_EXPECT_EQ(examined, std::size_t{20});

    // A clear is the one thing that cannot be extended through.
    log.clear();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{0});
}

CNA_STUDIO_TEST(TheCacheDoesNotGrowForeverInASessionThatNeverStops)
{
    // The trim exists for this and nothing else. Reads are bounded at both ends whether or not it
    // has run (`studioLogPanel` derives its window rather than trusting the cache), so without a
    // case like this the trim would be untested code that looked load-bearing -- and the thing it
    // prevents is a console that has been open all day holding a list of every line that ever
    // matched, long after the log dropped them.
    StudioLog log{100};
    for (int i = 0; i < 100; ++i)
    {
        log.append(LogSeverity::Error, "line " + std::to_string(i));
    }

    StudioLogPanelState state;
    const std::unique_ptr<StudioShell> shell = shellShowingTheLog();

    StudioLogPanelResult last;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("output",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioLogPanelResult result = studioLogPanel(frame, bounds, log, &state);
            if (frame.isDrawPass())
            {
                last = result;
                panelBounds = bounds;
            }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));

    // Engage a filter everything matches, so the cache holds one position per retained entry.
    CNA_STUDIO_EXPECT(clickAlongRow(*shell, panelBounds, 12.0f,
                                    [&] { return state.built && !state.matching.empty(); }));

    // Five thousand lines through a log that holds one hundred: forty-nine turnovers.
    for (int i = 0; i < 5000; ++i)
    {
        log.append(LogSeverity::Error, "later line " + std::to_string(i));
        if (i % 50 == 0) { shell->renderFrame(at(-1.0f, -1.0f)); }
    }
    shell->renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT_EQ(log.entries().size(), std::size_t{100});
    CNA_STUDIO_EXPECT_EQ(last.rowsMatching, std::size_t{100});

    // Bounded by what the log holds, not by what it has ever held. Untrimmed this would be about
    // five thousand and one hundred.
    CNA_STUDIO_EXPECT(state.matching.size() <= log.entries().size());
}
