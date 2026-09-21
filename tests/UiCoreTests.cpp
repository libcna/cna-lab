// SPDX-License-Identifier: MS-PL
/**
 * @file UiCoreTests.cpp
 * @brief Tests for the CNA Studio UI foundations: design tokens, widget identity, retained state.
 *
 * Everything here runs with no window, no GPU and no CNA. That is the property Phase 3 exists to
 * have: if the UI's identity, styling and state can only be checked by looking at a screenshot,
 * they will not be checked.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/UiCore/StudioFrame.hpp"
#include "CNA/Studio/UiCore/StudioTheme.hpp"
#include "CNA/Studio/UiCore/StudioWidgets.hpp"
#include "CNA/Studio/UiCore/WidgetId.hpp"
#include "CNA/Studio/UiCore/WidgetStateStore.hpp"

#include <cmath>
#include <set>
#include <string>

using namespace CNA::Studio;

// ------------------------------------------------------------------------------------------------
// Design tokens (STUDIO-03004, STUDIO-03005, STUDIO-03006)
// ------------------------------------------------------------------------------------------------

// The point of making tokens enumerable rather than struct members: a theme that forgot a role is
// caught by iteration, on the commit that added the role, instead of by someone noticing a magenta
// rectangle in a screenshot months later.
CNA_STUDIO_TEST(BothShippedThemesDefineEveryColourRole)
{
    for (const StudioTheme& theme : {StudioTheme::dark(), StudioTheme::light()})
    {
        StudioColorRole missing{};
        const bool complete = theme.isComplete(&missing);
        if (!complete)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                std::string{theme.name()} + " leaves role '"
                + std::string{studioColorRoleName(missing)} + "' at the placeholder");
        }
        CNA_STUDIO_EXPECT(complete);
    }
}

CNA_STUDIO_TEST(ADefaultConstructedThemeIsObviouslyUnconfigured)
{
    // Silence would be the wrong failure mode here: a theme nobody configured must be impossible
    // to mistake for a designed one.
    const StudioTheme theme;
    CNA_STUDIO_EXPECT(!theme.isComplete());
    CNA_STUDIO_EXPECT(theme.color(StudioColorRole::PanelBackground) == kStudioPlaceholderColor);
}

CNA_STUDIO_TEST(EveryTokenHasAStableName)
{
    // Names are written into preferences and diagnostics, so a token with no name is a token whose
    // value cannot be round-tripped.
    for (std::uint16_t i = 0; i < static_cast<std::uint16_t>(StudioColorRole::Count); ++i)
    {
        CNA_STUDIO_EXPECT(!studioColorRoleName(static_cast<StudioColorRole>(i)).empty());
    }
    for (std::uint16_t i = 0; i < static_cast<std::uint16_t>(StudioMetric::Count); ++i)
    {
        CNA_STUDIO_EXPECT(!studioMetricName(static_cast<StudioMetric>(i)).empty());
    }
    for (std::uint16_t i = 0; i < static_cast<std::uint16_t>(StudioFontRole::Count); ++i)
    {
        CNA_STUDIO_EXPECT(!studioFontRoleName(static_cast<StudioFontRole>(i)).empty());
    }
}

CNA_STUDIO_TEST(TokenNamesAreUnique)
{
    std::set<std::string_view> seen;
    for (std::uint16_t i = 0; i < static_cast<std::uint16_t>(StudioColorRole::Count); ++i)
    {
        CNA_STUDIO_EXPECT(seen.insert(studioColorRoleName(static_cast<StudioColorRole>(i))).second);
    }
}

CNA_STUDIO_TEST(EveryMetricHasANonZeroDefault)
{
    const StudioTheme theme = StudioTheme::dark();
    for (std::uint16_t i = 0; i < static_cast<std::uint16_t>(StudioMetric::Count); ++i)
    {
        const auto metric = static_cast<StudioMetric>(i);
        if (theme.logicalMetric(metric) <= 0)
        {
            CnaStudioTest::reportFailure(__FILE__, __LINE__,
                "metric '" + std::string{studioMetricName(metric)} + "' has no default");
        }
        CNA_STUDIO_EXPECT(theme.logicalMetric(metric) > 0);
    }
}

CNA_STUDIO_TEST(MetricsScaleWithDpiAndLogicalValuesDoNot)
{
    StudioTheme theme = StudioTheme::dark();
    const int logical = theme.logicalMetric(StudioMetric::ControlHeight);

    theme.setScale(2.0f);
    CNA_STUDIO_EXPECT_EQ(theme.metric(StudioMetric::ControlHeight), logical * 2);
    // The authored value is the source of truth and must not drift as the scale changes.
    CNA_STUDIO_EXPECT_EQ(theme.logicalMetric(StudioMetric::ControlHeight), logical);

    theme.setScale(1.0f);
    CNA_STUDIO_EXPECT_EQ(theme.metric(StudioMetric::ControlHeight), logical);
}

CNA_STUDIO_TEST(MetricsAreCorrectAtEveryDpiScaleStudioSupports)
{
    StudioTheme theme = StudioTheme::dark();
    const int logical = theme.logicalMetric(StudioMetric::RowHeight);

    for (const float scale : {1.0f, 1.25f, 1.5f, 1.75f, 2.0f})
    {
        theme.setScale(scale);
        const int expected = static_cast<int>(std::lround(static_cast<float>(logical) * scale));
        CNA_STUDIO_EXPECT_EQ(theme.metric(StudioMetric::RowHeight), expected);
    }
}

CNA_STUDIO_TEST(HairlineMetricsNeverRoundAwayToNothing)
{
    // A 1px border at 0.5 scale rounds to 0 and the control silently loses its outline. This is
    // the clamp that prevents it, and it is worth a test because the failure is invisible in code
    // review and only shows up on an unusual display.
    StudioTheme theme = StudioTheme::dark();
    theme.setScale(0.5f);

    CNA_STUDIO_EXPECT(theme.metric(StudioMetric::BorderWidth) >= 1);
    CNA_STUDIO_EXPECT(theme.metric(StudioMetric::SeparatorThickness) >= 1);
    CNA_STUDIO_EXPECT(theme.metric(StudioMetric::FocusRingWidth) >= 1);
}

CNA_STUDIO_TEST(AnAbsurdDpiScaleIsClampedRatherThanProducingABrokenWindow)
{
    StudioTheme theme = StudioTheme::dark();
    theme.setScale(0.0f);
    CNA_STUDIO_EXPECT(theme.scale() > 0.0f);
    theme.setScale(-3.0f);
    CNA_STUDIO_EXPECT(theme.scale() > 0.0f);
    theme.setScale(1000.0f);
    CNA_STUDIO_EXPECT(theme.scale() <= 4.0f);
}

CNA_STUDIO_TEST(FontSizesScaleWithDpi)
{
    StudioTheme theme = StudioTheme::dark();
    const float base = theme.font(StudioFontRole::Body).sizePx;
    theme.setScale(2.0f);
    CNA_STUDIO_EXPECT_EQ(theme.font(StudioFontRole::Body).sizePx, base * 2.0f);
}

CNA_STUDIO_TEST(EveryInteractiveStateResolvesToADistinctControlBackground)
{
    // The reason states are real tokens rather than "base colour, 8% lighter": a computed hover is
    // invisible on a dark panel and garish on an accent. If two states ever resolve to the same
    // colour the user cannot tell them apart, which is the whole failure this guards.
    const StudioTheme theme = StudioTheme::dark();
    std::set<std::uint32_t> packed;
    for (const StudioControlState state : {StudioControlState::Normal, StudioControlState::Hover,
                                           StudioControlState::Pressed, StudioControlState::Selected,
                                           StudioControlState::Disabled})
    {
        const StudioColor c = theme.controlBackground(state);
        const auto key = static_cast<std::uint32_t>((c.r << 24) | (c.g << 16) | (c.b << 8) | c.a);
        CNA_STUDIO_EXPECT(packed.insert(key).second);
    }
}

CNA_STUDIO_TEST(ADisabledControlNeverDrawsInTheAccent)
{
    // The accent means "this does something". A disabled control does not.
    const StudioTheme theme = StudioTheme::dark();
    CNA_STUDIO_EXPECT(theme.accent(StudioControlState::Disabled) != theme.color(StudioColorRole::Accent));
    CNA_STUDIO_EXPECT(theme.controlText(StudioControlState::Disabled)
                      == theme.color(StudioColorRole::TextDisabled));
}

CNA_STUDIO_TEST(FocusAndSelectionAreDistinguishable)
{
    // A focused row inside a selection must be visibly both. Making the focus ring the accent --
    // which is also the selection fill -- would make that impossible on exactly the row where it
    // matters most.
    for (const StudioTheme& theme : {StudioTheme::dark(), StudioTheme::light()})
    {
        CNA_STUDIO_EXPECT(theme.color(StudioColorRole::FocusRing)
                          != theme.color(StudioColorRole::Selection));
    }
}

CNA_STUDIO_TEST(AThemeIsAValueThatCanBeCustomisedByCopying)
{
    const StudioTheme base = StudioTheme::dark();
    StudioTheme custom = base;
    custom.setName("Custom");
    custom.setColor(StudioColorRole::Accent, StudioColor{10, 20, 30, 255});

    CNA_STUDIO_EXPECT(custom.color(StudioColorRole::Accent) != base.color(StudioColorRole::Accent));
    CNA_STUDIO_EXPECT(custom.isComplete());
    CNA_STUDIO_EXPECT_EQ(std::string{base.name()}, std::string{"CNA Studio Dark"});
}

CNA_STUDIO_TEST(TheTwoShippedThemesAreActuallyDifferent)
{
    const StudioTheme dark = StudioTheme::dark();
    const StudioTheme light = StudioTheme::light();
    CNA_STUDIO_EXPECT(dark.color(StudioColorRole::PanelBackground)
                      != light.color(StudioColorRole::PanelBackground));
    CNA_STUDIO_EXPECT(dark.color(StudioColorRole::TextPrimary)
                      != light.color(StudioColorRole::TextPrimary));
}

CNA_STUDIO_TEST(TextIsReadableAgainstItsOwnBackground)
{
    // Not a full contrast-ratio audit -- that is STUDIO-32003 -- but a floor: if body text and the
    // panel behind it ever converge, the tool is unusable and no screenshot test would phrase the
    // failure as clearly as this does.
    for (const StudioTheme& theme : {StudioTheme::dark(), StudioTheme::light()})
    {
        const StudioColor text = theme.color(StudioColorRole::TextPrimary);
        const StudioColor background = theme.color(StudioColorRole::PanelBackground);
        const int delta = std::abs(static_cast<int>(text.r) - static_cast<int>(background.r))
                        + std::abs(static_cast<int>(text.g) - static_cast<int>(background.g))
                        + std::abs(static_cast<int>(text.b) - static_cast<int>(background.b));
        CNA_STUDIO_EXPECT(delta > 250);
    }
}

// ------------------------------------------------------------------------------------------------
// Widget identity (STUDIO-03002)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(TheSameWidgetKeepsItsIdAcrossFrames)
{
    WidgetIdStack ids;
    ids.beginFrame();
    ids.push("Hierarchy");
    const WidgetId first = ids.make("Delete");
    ids.pop();

    ids.beginFrame();
    ids.push("Hierarchy");
    const WidgetId second = ids.make("Delete");
    ids.pop();

    CNA_STUDIO_EXPECT(first == second);
    CNA_STUDIO_EXPECT(first.isValid());
}

CNA_STUDIO_TEST(TheSameLabelInTwoScopesIsTwoWidgets)
{
    // This is what makes labels reusable: "Delete" in the Hierarchy and "Delete" in the Content
    // Browser are different buttons without either having to invent unique visible text.
    WidgetIdStack ids;
    ids.beginFrame();
    ids.push("Hierarchy");
    const WidgetId inHierarchy = ids.make("Delete");
    ids.pop();
    ids.push("ContentBrowser");
    const WidgetId inBrowser = ids.make("Delete");
    ids.pop();

    CNA_STUDIO_EXPECT(inHierarchy != inBrowser);
}

CNA_STUDIO_TEST(AWidgetIdentifiedByAStableKeySurvivesInsertionAbroveIt)
{
    // The bug this prevents: identity from iteration position. Insert a row above row three and,
    // with positional ids, row three inherits row four's scroll offset and half-typed text.
    const auto idForEntity = [](WidgetIdStack& ids, std::string_view uuid) {
        ids.push("Outliner");
        ids.push(uuid);
        const WidgetId id = ids.make("name");
        ids.pop();
        ids.pop();
        return id;
    };

    WidgetIdStack ids;
    ids.beginFrame();
    const WidgetId before = idForEntity(ids, "entity-c");

    // Next frame, two entities have been inserted ahead of it in the list.
    ids.beginFrame();
    (void) idForEntity(ids, "entity-new-1");
    (void) idForEntity(ids, "entity-new-2");
    const WidgetId after = idForEntity(ids, "entity-c");

    CNA_STUDIO_EXPECT(before == after);
}

CNA_STUDIO_TEST(TwoWidgetsSharingAnIdIsDetectedRatherThanDrawnWrong)
{
    // Two buttons both labelled "Delete" in one scope become one widget: pressing either lights
    // both and only one works. It reads as a rendering glitch, so it is detected here instead.
    WidgetIdStack ids;
    ids.beginFrame();
    (void) ids.make("Delete");
    CNA_STUDIO_EXPECT_EQ(ids.collisionCount(), std::size_t{0});

    (void) ids.make("Delete");
    CNA_STUDIO_EXPECT_EQ(ids.collisionCount(), std::size_t{1});
}

CNA_STUDIO_TEST(TheHashSuffixSeparatesWidgetsWithIdenticalVisibleText)
{
    WidgetIdStack ids;
    ids.beginFrame();
    const WidgetId a = ids.make("Delete##entity");
    const WidgetId b = ids.make("Delete##component");

    CNA_STUDIO_EXPECT(a != b);
    CNA_STUDIO_EXPECT_EQ(ids.collisionCount(), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(std::string{WidgetIdStack::visibleLabel("Delete##entity")}, std::string{"Delete"});
    CNA_STUDIO_EXPECT_EQ(std::string{WidgetIdStack::visibleLabel("Delete")}, std::string{"Delete"});
}

CNA_STUDIO_TEST(CollisionsAreClearedEachFrame)
{
    WidgetIdStack ids;
    ids.beginFrame();
    (void) ids.make("Same");
    (void) ids.make("Same");
    CNA_STUDIO_EXPECT_EQ(ids.collisionCount(), std::size_t{1});

    ids.beginFrame();
    (void) ids.make("Same");
    CNA_STUDIO_EXPECT_EQ(ids.collisionCount(), std::size_t{0});
}

CNA_STUDIO_TEST(AnUnbalancedScopeDoesNotCorruptLaterFrames)
{
    // A panel that threw between push and pop has already failed once. Letting it make every
    // subsequent frame's ids wrong would turn one visible bug into an inexplicable one.
    WidgetIdStack ids;
    ids.beginFrame();
    ids.push("Leaked");
    ids.push("AlsoLeaked");
    const WidgetId leaked = ids.make("Button");

    ids.beginFrame();
    const WidgetId clean = ids.make("Button");

    CNA_STUDIO_EXPECT_EQ(ids.depth(), std::size_t{0});
    CNA_STUDIO_EXPECT(leaked != clean);
}

CNA_STUDIO_TEST(PoppingPastTheRootScopeIsHarmless)
{
    WidgetIdStack ids;
    ids.beginFrame();
    ids.pop();
    ids.pop();
    CNA_STUDIO_EXPECT_EQ(ids.depth(), std::size_t{0});
    CNA_STUDIO_EXPECT(ids.make("Button").isValid());
}

CNA_STUDIO_TEST(AWidgetIdIsNeverTheInvalidSentinel)
{
    WidgetIdStack ids;
    ids.beginFrame();
    for (int i = 0; i < 2000; ++i)
    {
        CNA_STUDIO_EXPECT(ids.makeIndex(i) != kInvalidWidgetId);
    }
}

CNA_STUDIO_TEST(DistinctKeysProduceDistinctIds)
{
    // Not a proof of no collisions -- that is not achievable with a 64-bit hash -- but a check
    // that the mixing is not degenerate over the shapes of key a real UI produces.
    WidgetIdStack ids;
    ids.beginFrame();
    std::set<std::uint64_t> seen;
    for (int scope = 0; scope < 40; ++scope)
    {
        ids.pushIndex(scope);
        for (int widget = 0; widget < 40; ++widget)
        {
            CNA_STUDIO_EXPECT(seen.insert(ids.makeIndex(widget).value()).second);
        }
        ids.pop();
    }
    CNA_STUDIO_EXPECT_EQ(seen.size(), std::size_t{1600});
    CNA_STUDIO_EXPECT_EQ(ids.collisionCount(), std::size_t{0});
}

// ------------------------------------------------------------------------------------------------
// Retained widget state (STUDIO-03003)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(WidgetStateSurvivesBetweenFrames)
{
    WidgetStateStore store;
    const WidgetId id{1234};

    store.beginFrame();
    store.get(id).expanded = true;
    store.get(id).scrollY = 42.0f;

    store.beginFrame();
    CNA_STUDIO_EXPECT(store.get(id).expanded);
    CNA_STUDIO_EXPECT_EQ(store.get(id).scrollY, 42.0f);
}

CNA_STUDIO_TEST(AWidgetSeenForTheFirstTimeGetsNeutralDefaults)
{
    WidgetStateStore store;
    const WidgetState& state = store.get(WidgetId{7});
    CNA_STUDIO_EXPECT(!state.expanded);
    CNA_STUDIO_EXPECT(!state.checked);
    CNA_STUDIO_EXPECT_EQ(state.scrollY, 0.0f);
    CNA_STUDIO_EXPECT(state.text.empty());
}

CNA_STUDIO_TEST(FindDoesNotCreateState)
{
    WidgetStateStore store;
    CNA_STUDIO_EXPECT(store.find(WidgetId{99}) == nullptr);
    CNA_STUDIO_EXPECT_EQ(store.size(), std::size_t{0});
}

CNA_STUDIO_TEST(StateForAWidgetNobodyDescribesAnyMoreIsReclaimed)
{
    // The leak this design exists to prevent: every tree node ever expanded and every folder ever
    // scrolled, retained for the life of the process. Nothing goes wrong, it just grows.
    WidgetStateStore store;
    store.setRetentionFrames(4);

    store.get(WidgetId{1}).expanded = true;
    store.get(WidgetId{2}).expanded = true;
    CNA_STUDIO_EXPECT_EQ(store.size(), std::size_t{2});

    for (int frame = 0; frame < 10; ++frame)
    {
        store.beginFrame();
        store.touch(WidgetId{1});   // widget 1 is still being described; widget 2 is not
    }
    store.sweep();

    CNA_STUDIO_EXPECT(store.find(WidgetId{1}) != nullptr);
    CNA_STUDIO_EXPECT(store.find(WidgetId{2}) == nullptr);
}

CNA_STUDIO_TEST(StateSurvivesBeingOffScreenForAWhile)
{
    // A widget in a collapsed section, on a hidden tab, or scrolled out of a virtualised list is
    // not described this frame and must not lose its state for it.
    WidgetStateStore store;
    const WidgetId id{55};
    store.get(id).scrollY = 17.0f;

    for (std::uint64_t frame = 0; frame < WidgetStateStore::kDefaultRetentionFrames / 2; ++frame)
    {
        store.beginFrame();
    }

    CNA_STUDIO_EXPECT(store.find(id) != nullptr);
    CNA_STUDIO_EXPECT_EQ(store.get(id).scrollY, 17.0f);
}

CNA_STUDIO_TEST(NothingIsReclaimedEarlyInASession)
{
    // Frame numbers start low and the retention window is large, so a cutoff computed by
    // subtracting on unsigned values would wrap and reclaim everything. This is that regression.
    WidgetStateStore store;
    store.touch(WidgetId{1});
    store.touch(WidgetId{2});

    for (int frame = 0; frame < 5; ++frame) { store.beginFrame(); }
    store.sweep();

    CNA_STUDIO_EXPECT_EQ(store.size(), std::size_t{2});
}

CNA_STUDIO_TEST(TheRetentionWindowCannotBeSetSoLowThatOffScreenStateIsLost)
{
    WidgetStateStore store;
    store.setRetentionFrames(0);
    CNA_STUDIO_EXPECT(store.retentionFrames() >= 2);
}

CNA_STUDIO_TEST(StateCanBeForgottenExplicitly)
{
    WidgetStateStore store;
    store.get(WidgetId{3}).checked = true;
    CNA_STUDIO_EXPECT(store.forget(WidgetId{3}));
    CNA_STUDIO_EXPECT(!store.forget(WidgetId{3}));
    CNA_STUDIO_EXPECT_EQ(store.size(), std::size_t{0});
}

CNA_STUDIO_TEST(TextEditingStateRoundTrips)
{
    WidgetStateStore store;
    WidgetState& state = store.get(WidgetId{8});
    state.text = "Player Speed";
    state.caret = 6;
    state.selectionAnchor = 0;

    store.beginFrame();
    const WidgetState* found = store.find(WidgetId{8});
    CNA_STUDIO_EXPECT(found != nullptr);
    CNA_STUDIO_EXPECT_EQ(found->text, std::string{"Player Speed"});
    CNA_STUDIO_EXPECT_EQ(found->caret, std::size_t{6});
}

CNA_STUDIO_TEST(WidgetStateIsKeyedByIdentitySoTwoWidgetsDoNotShareIt)
{
    WidgetIdStack ids;
    WidgetStateStore store;
    ids.beginFrame();

    ids.push("PanelA");
    const WidgetId a = ids.make("Tree");
    ids.pop();
    ids.push("PanelB");
    const WidgetId b = ids.make("Tree");
    ids.pop();

    store.get(a).scrollY = 100.0f;
    store.get(b).scrollY = 5.0f;

    CNA_STUDIO_EXPECT_EQ(store.get(a).scrollY, 100.0f);
    CNA_STUDIO_EXPECT_EQ(store.get(b).scrollY, 5.0f);
}

// ------------------------------------------------------------------------------------------------
// Slider (STUDIO-19003)
// ------------------------------------------------------------------------------------------------

namespace
{
    UiInputState sliderAt(float x, float y, bool leftDown = false)
    {
        UiInputState input;
        input.displayWidth = 640.0f;
        input.displayHeight = 480.0f;
        input.mouseX = x;
        input.mouseY = y;
        input.mouseInWindow = true;
        input.setMouseDown(UiMouseButton::Left, leftDown);
        return input;
    }

    /** @brief One slider over a known rectangle, driven a frame at a time. */
    struct SliderFixture
    {
        StudioFrame frame{StudioTheme::dark()};
        UiRect bounds{40.0f, 60.0f, 200.0f, 24.0f};
        float value = 0.5f;
        StudioSliderOptions options;
        StudioWidgetResult last;

        void run(const UiInputState& input)
        {
            runStudioFrame(frame, input, [&](StudioFrame& f) {
                const StudioWidgetResult result =
                    studioSlider(f, f.ids().make("slider"), bounds, value, options);
                if (f.isInputPass()) { last = result; }
            });
        }

        void settle() { run(sliderAt(400.0f, 400.0f)); }

        /**
         * @brief Presses at @p x on the track, which is the gesture that jumps to a point.
         *
         * Inset by a pixel at the caller's request rather than clamped here: a rectangle's right
         * edge is *outside* it, so a press at `bounds.right()` hovers nothing and a case that did
         * that would pass by never reaching the widget at all.
         */
        void pressAt(float x)
        {
            run(sliderAt(x, bounds.centerY()));
            run(sliderAt(x, bounds.centerY(), /*leftDown=*/true));
        }

        /** @brief Holds the press and moves the pointer to @p x, wherever that is. */
        void dragTo(float x)
        {
            run(sliderAt(x, bounds.centerY(), /*leftDown=*/true));
        }

        void release(float x)
        {
            run(sliderAt(x, bounds.centerY()));
        }

        /** @brief Presses @p key with the pointer parked away from the track. */
        void key(UiKey pressed)
        {
            UiInputState input = sliderAt(400.0f, 400.0f);
            input.setKeyDown(pressed, true);
            run(input);
            run(sliderAt(400.0f, 400.0f));
        }
    };
}

CNA_STUDIO_TEST(ClickingASlidersTrackJumpsToThatPointRatherThanStepping)
{
    // A slider is a position, and the gesture that says "put it here" should put it there. A
    // control that stepped towards the click would take five presses to cross its own track.
    SliderFixture fixture;
    fixture.settle();

    fixture.pressAt(fixture.bounds.left());
    CNA_STUDIO_EXPECT(fixture.value <= 0.05f);
    CNA_STUDIO_EXPECT(fixture.last.changed);

    fixture.release(400.0f);
    fixture.pressAt(fixture.bounds.right() - 1.0f);
    CNA_STUDIO_EXPECT(fixture.value >= 0.95f);

    // And the middle is the middle, which is the assertion that catches a thumb-width the
    // arithmetic forgot to account for at both ends.
    fixture.release(400.0f);
    fixture.pressAt(fixture.bounds.centerX());
    CNA_STUDIO_EXPECT(std::fabs(fixture.value - 0.5f) < 0.06f);
}

CNA_STUDIO_TEST(ASliderClampsRatherThanRefusingAndNeverWritesOutsideItsRange)
{
    // A value out of range arrives from a hand-edited file and from an older build. Refusing to
    // show it would leave the user unable to see what is wrong, let alone fix it -- so the thumb
    // pins and the value is left alone until they move it.
    SliderFixture fixture;
    fixture.value = 7.0f;
    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.value, 7.0f);
    CNA_STUDIO_EXPECT(!fixture.last.changed);

    // The first drag brings it into range and cannot take it back out.
    fixture.pressAt(fixture.bounds.right() - 1.0f);
    CNA_STUDIO_EXPECT(fixture.value <= 1.0f);
    CNA_STUDIO_EXPECT(fixture.value >= 0.0f);

    // And a drag that leaves the widget entirely still cannot take it out of range. Driven as a
    // real drag rather than as a press at a far-off point, because a press out there hovers
    // nothing -- a case written that way would pass by never reaching the slider at all, which is
    // how the first draft of this one passed while the clamp did nothing.
    fixture.release(400.0f);
    fixture.pressAt(fixture.bounds.centerX());
    fixture.dragTo(fixture.bounds.left() - 500.0f);
    CNA_STUDIO_EXPECT(fixture.value >= 0.0f);
    CNA_STUDIO_EXPECT(fixture.value <= 0.05f);

    fixture.dragTo(fixture.bounds.right() + 500.0f);
    CNA_STUDIO_EXPECT(fixture.value <= 1.0f);
    CNA_STUDIO_EXPECT(fixture.value >= 0.95f);
}

CNA_STUDIO_TEST(ASliderWithAStepLandsOnStopsAndNeverPastTheEnd)
{
    // A step that does not divide the range is the case worth pinning: rounding to a multiple can
    // put the last stop past the maximum, and a slider that wrote 1.05 into a 0..1 property would
    // be the thing this control exists to prevent.
    // 0.4 rather than a step that divides the range, deliberately: rounding 1.0 to a multiple of
    // 0.4 gives 1.2, so this is the value the re-clamp exists for. A step of 0.3 rounds *down* to
    // 0.9 and never reaches the clamp at all, which is how the first draft of this case passed
    // with the re-clamp removed.
    SliderFixture fixture;
    fixture.options.step = 0.4f;
    fixture.settle();

    fixture.pressAt(fixture.bounds.right() - 1.0f);
    CNA_STUDIO_EXPECT(fixture.value <= 1.0f);
    CNA_STUDIO_EXPECT(fixture.value >= 0.0f);

    fixture.release(400.0f);
    fixture.pressAt(fixture.bounds.left() + fixture.bounds.width * 0.5f);

    // On a stop: 0, 0.4, 0.8 or the clamped end.
    const float remainder = std::fabs(std::fmod(fixture.value, 0.4f));
    CNA_STUDIO_EXPECT(remainder < 0.01f || std::fabs(remainder - 0.4f) < 0.01f
                      || fixture.value == 1.0f);
}

CNA_STUDIO_TEST(ASliderWithNoRangeIsDrawnAndTakesNothing)
{
    // A property whose descriptor declares `minimum == maximum` is a fixed thing. Drawn rather
    // than skipped, so the row looks like what it is instead of silently vanishing -- and inert,
    // because there is nothing to choose.
    SliderFixture fixture;
    fixture.options.minimum = 1.0f;
    fixture.options.maximum = 1.0f;
    fixture.value = 1.0f;
    fixture.settle();

    fixture.pressAt(fixture.bounds.right() - 1.0f);
    CNA_STUDIO_EXPECT_EQ(fixture.value, 1.0f);
    CNA_STUDIO_EXPECT(!fixture.last.changed);
    CNA_STUDIO_EXPECT_EQ(fixture.frame.phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(TheArrowsNudgeASliderAndCannotPushItPastEitherEnd)
{
    // The gesture a pointer cannot do precisely, and the one place the clamp inside `commit` is
    // load-bearing: a drag's value is already derived from a clamped fraction, so a nudge is the
    // only way to ask for a value outside the range. A first draft of these cases drove only the
    // pointer, and removing the clamp broke nothing.
    SliderFixture fixture;
    fixture.settle();

    // Focused by clicking it, which is how a user reaches it before typing.
    fixture.pressAt(fixture.bounds.centerX());
    fixture.release(fixture.bounds.centerX());

    const float afterClick = fixture.value;
    fixture.key(UiKey::RightArrow);
    CNA_STUDIO_EXPECT(fixture.value > afterClick);

    // A hundredth of the range where no step is declared, so a 0..1 property moves in hundredths.
    CNA_STUDIO_EXPECT(fixture.value - afterClick < 0.02f);

    fixture.key(UiKey::LeftArrow);
    CNA_STUDIO_EXPECT(std::fabs(fixture.value - afterClick) < 0.001f);

    // End and Home are the ends themselves.
    fixture.key(UiKey::End);
    CNA_STUDIO_EXPECT_EQ(fixture.value, 1.0f);

    // And nudging past one is refused rather than written: this is what the clamp is for.
    fixture.key(UiKey::RightArrow);
    fixture.key(UiKey::RightArrow);
    CNA_STUDIO_EXPECT_EQ(fixture.value, 1.0f);

    fixture.key(UiKey::Home);
    CNA_STUDIO_EXPECT_EQ(fixture.value, 0.0f);
    fixture.key(UiKey::LeftArrow);
    fixture.key(UiKey::LeftArrow);
    CNA_STUDIO_EXPECT_EQ(fixture.value, 0.0f);
}
