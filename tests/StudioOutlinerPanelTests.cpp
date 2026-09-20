// SPDX-License-Identifier: MS-PL
/**
 * @file StudioOutlinerPanelTests.cpp
 * @brief The World Outliner, and the tree widget underneath it (plan.md STUDIO-07006).
 *
 * The second panel ported off Dear ImGui, and the first that reads the document model rather than
 * a log. The cases split along the seam that matters: what the tree *is* -- which rows, in which
 * order, at which depth -- is decided by `studioOutlinerRows` and asserted without a frame; what
 * the user can *do* to it is driven through the real widget with synthesised input.
 *
 * Splitting them is not tidiness. A wrong hierarchy and a dead click look identical from a
 * screenshot, and telling them apart is the difference between a five-minute fix and an afternoon.
 */

#include "TestHarness.hpp"

#include <tuple>

#include "CNA/Studio/Scene/AssetDrop.hpp"
#include "CNA/Studio/Scene/BuiltinComponents.hpp"

#include "CNA/Studio/ShellPanels/StudioContentBrowser.hpp"
#include "CNA/Studio/ShellPanels/StudioOutlinerPanel.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    constexpr float kWidth = 1280.0f;
    constexpr float kHeight = 720.0f;

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

    /** @brief Adds an entity to @p scene and returns its id. */
    Uuid addEntity(SceneDocument& scene, const std::string& name, const Uuid& parent = {})
    {
        StudioEntity entity{Uuid::generate(), name};
        entity.setParentId(parent);
        const Uuid id = entity.getId();
        scene.addEntity(std::move(entity));
        return id;
    }

    /** @brief A context holding a small hierarchy: two roots, one of them with two children. */
    struct Fixture
    {
        StudioContext context;
        Uuid camera;
        Uuid player;
        Uuid weapon;
        Uuid shield;

        Fixture()
        {
            camera = addEntity(context.getScene(), "Main Camera");
            player = addEntity(context.getScene(), "Player");
            weapon = addEntity(context.getScene(), "Weapon", player);
            shield = addEntity(context.getScene(), "Shield", player);
        }
    };

    /** @brief Finds a row by label, or nullptr. */
    const StudioTreeRow* rowNamed(const std::vector<StudioTreeRow>& rows, std::string_view label)
    {
        for (const StudioTreeRow& row : rows)
        {
            if (row.label == label) { return &row; }
        }
        return nullptr;
    }

    /** @brief The shell, with the outliner raised and given the panel's content. */
    std::unique_ptr<StudioShell> shellShowingTheOutliner()
    {
        auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
        shell->resetLayout();
        shell->renderFrame(at(-1.0f, -1.0f));
        CNA_STUDIO_EXPECT(shell->activatePanel("outliner"));
        return shell;
    }
}

CNA_STUDIO_TEST(TheOutlinerShowsTheSceneHierarchyParentsBeforeChildren)
{
    Fixture fixture;
    StudioTreeState state;

    const std::vector<StudioTreeRow> rows =
        studioOutlinerRows(fixture.context.getScene(), {}, state);

    CNA_STUDIO_EXPECT_EQ(rows.size(), std::size_t{4});

    // Order is what makes indentation readable: a child drawn above its parent is not a tree.
    //
    // Within a level it is the scene document's own order -- sort order first, then name -- not
    // the order entities happened to be added in. The outliner does not get to invent an order:
    // showing siblings differently from how the scene holds them is how a user reorders something
    // in one place and cannot find it in another.
    CNA_STUDIO_EXPECT_EQ(rows[0].label, std::string{"Main Camera"});
    CNA_STUDIO_EXPECT_EQ(rows[1].label, std::string{"Player"});
    CNA_STUDIO_EXPECT_EQ(rows[2].label, std::string{"Shield"});
    CNA_STUDIO_EXPECT_EQ(rows[3].label, std::string{"Weapon"});

    CNA_STUDIO_EXPECT_EQ(rows[0].depth, 0);
    CNA_STUDIO_EXPECT_EQ(rows[1].depth, 0);
    CNA_STUDIO_EXPECT_EQ(rows[2].depth, 1);
    CNA_STUDIO_EXPECT_EQ(rows[3].depth, 1);

    // Only a row with children gets a disclosure triangle. One on a leaf is a promise the tree
    // cannot keep, and a user clicks it once and learns to distrust the whole column.
    CNA_STUDIO_EXPECT(rows[1].hasChildren);
    CNA_STUDIO_EXPECT(!rows[0].hasChildren);
    CNA_STUDIO_EXPECT(!rows[2].hasChildren);

    // And the same order the scene itself reports, rather than one this panel arrived at
    // independently and happens to agree with today.
    const std::vector<Uuid> children = fixture.context.getScene().getChildren(fixture.player);
    CNA_STUDIO_EXPECT_EQ(children.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(rows[2].id, children[0].toString());
    CNA_STUDIO_EXPECT_EQ(rows[3].id, children[1].toString());
}

CNA_STUDIO_TEST(CollapsingARowHidesItsChildrenAndNothingElse)
{
    Fixture fixture;
    StudioTreeState state;

    state.setExpanded(fixture.player.toString(), false);
    const std::vector<StudioTreeRow> collapsed =
        studioOutlinerRows(fixture.context.getScene(), {}, state);

    CNA_STUDIO_EXPECT_EQ(collapsed.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(rowNamed(collapsed, "Main Camera") != nullptr);
    CNA_STUDIO_EXPECT(rowNamed(collapsed, "Player") != nullptr);
    CNA_STUDIO_EXPECT(rowNamed(collapsed, "Weapon") == nullptr);

    // The collapsed parent keeps its triangle -- it is how the user gets its children back.
    const StudioTreeRow* player = rowNamed(collapsed, "Player");
    CNA_STUDIO_EXPECT(player != nullptr && player->hasChildren);

    state.setExpanded(fixture.player.toString(), true);
    CNA_STUDIO_EXPECT_EQ(studioOutlinerRows(fixture.context.getScene(), {}, state).size(),
                         std::size_t{4});
}

CNA_STUDIO_TEST(ATreeStartsOpenRatherThanMakingTheUserFindItsContents)
{
    // A tree that starts entirely collapsed shows one line and makes the user work to discover
    // that their scene has anything in it. Defaulting to open costs a scroll; defaulting to closed
    // costs a click per level before anything can be seen.
    Fixture fixture;
    const StudioTreeState fresh;

    CNA_STUDIO_EXPECT(fresh.isExpanded(fixture.player.toString()));
    CNA_STUDIO_EXPECT_EQ(fresh.collapsedCount(), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(studioOutlinerRows(fixture.context.getScene(), {}, fresh).size(),
                         std::size_t{4});
}

/**
 * A search keeps what matches and the path that leads to it (`plan.md` STUDIO-13002).
 *
 * Filtering a *tree* is not filtering a list. A row that matches is useless without the ancestors
 * above it: "Weapon" on its own says nothing about which of the four Players it belongs to, and a
 * tree that showed matches at depth zero would be inventing a hierarchy the scene does not have.
 */
CNA_STUDIO_TEST(ASearchKeepsTheMatchesAndThePathToThem)
{
    Fixture fixture;
    StudioTreeState state;

    // No filter is not a filter that matches everything: the two are the same picture and
    // different costs, and an inactive one walks nothing.
    const StudioOutlinerFilter none = studioOutlinerFilter(fixture.context.getScene(), "");
    CNA_STUDIO_EXPECT(!none.active);
    CNA_STUDIO_EXPECT(none.kept.empty());

    const StudioOutlinerFilter weapon =
        studioOutlinerFilter(fixture.context.getScene(), "weapon");
    CNA_STUDIO_EXPECT(weapon.active);

    // The match, and the parent that leads to it. Not the siblings, and not the other root.
    CNA_STUDIO_EXPECT_EQ(weapon.kept.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(weapon.kept.find(fixture.weapon) != weapon.kept.end());
    CNA_STUDIO_EXPECT(weapon.kept.find(fixture.player) != weapon.kept.end());
    CNA_STUDIO_EXPECT(weapon.kept.find(fixture.shield) == weapon.kept.end());
    CNA_STUDIO_EXPECT(weapon.kept.find(fixture.camera) == weapon.kept.end());

    // And the two sets are kept apart: the parent is the way to the match, not a match.
    CNA_STUDIO_EXPECT_EQ(weapon.matched.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(weapon.matched.find(fixture.weapon) != weapon.matched.end());

    const std::vector<StudioTreeRow> rows =
        studioOutlinerRows(fixture.context.getScene(), {}, state, weapon);
    CNA_STUDIO_EXPECT_EQ(rows.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(rows.front().label, std::string{"Player"});
    CNA_STUDIO_EXPECT_EQ(rows.back().label, std::string{"Weapon"});

    // The path is dimmed and the match is not, so the eye can tell which row it searched for --
    // and dimmed rather than disabled, because a path the user cannot click is one they have to
    // clear the search to walk.
    CNA_STUDIO_EXPECT(rows.front().muted);
    CNA_STUDIO_EXPECT(!rows.back().muted);
    CNA_STUDIO_EXPECT(rows.front().enabled);

    // The depth is the scene's, not the filter's: a match shown at depth zero would be a hierarchy
    // the document does not have.
    CNA_STUDIO_EXPECT_EQ(rows.back().depth, 1);

    // Case-insensitive, which is what a name search means.
    CNA_STUDIO_EXPECT_EQ(studioOutlinerFilter(fixture.context.getScene(), "WEAPON").matched.size(),
                         std::size_t{1});

    // A substring rather than a whole name, so a user types what they remember.
    CNA_STUDIO_EXPECT_EQ(studioOutlinerFilter(fixture.context.getScene(), "ea").matched.size(),
                         std::size_t{1});

    // And nothing matching keeps nothing, rather than falling back to the whole scene.
    const StudioOutlinerFilter nothing =
        studioOutlinerFilter(fixture.context.getScene(), "nosuchthing");
    CNA_STUDIO_EXPECT(nothing.active);
    CNA_STUDIO_EXPECT(nothing.kept.empty());
    CNA_STUDIO_EXPECT_EQ(studioOutlinerRowCount(fixture.context.getScene(), state, nothing),
                         std::size_t{0});
}

/**
 * A search opens the branches it needs, whatever the user had collapsed (`plan.md` STUDIO-13002).
 *
 * A match hidden inside a closed branch is a match the search did not find, as far as anybody
 * looking at the screen can tell -- and asking the user to go and open the branch is asking them to
 * find the thing they were searching for.
 */
CNA_STUDIO_TEST(ASearchReachesIntoBranchesTheUserHadClosed)
{
    Fixture fixture;
    StudioTreeState state;

    // Closed, so without the search Weapon is not on screen at all.
    state.setExpanded(fixture.player.toString(), false);
    CNA_STUDIO_EXPECT_EQ(studioOutlinerRowCount(fixture.context.getScene(), state), std::size_t{2});

    const StudioOutlinerFilter weapon =
        studioOutlinerFilter(fixture.context.getScene(), "weapon");
    const std::vector<StudioTreeRow> rows =
        studioOutlinerRows(fixture.context.getScene(), {}, state, weapon);

    CNA_STUDIO_EXPECT_EQ(rows.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(rows.back().label, std::string{"Weapon"});

    // And the collapse is not thrown away: clearing the search leaves the tree as the user left it,
    // rather than as the search needed it.
    CNA_STUDIO_EXPECT(!state.isExpanded(fixture.player.toString()));
    CNA_STUDIO_EXPECT_EQ(studioOutlinerRowCount(fixture.context.getScene(), state), std::size_t{2});
}

CNA_STUDIO_TEST(TheOutlinerMarksWhatIsSelected)
{
    Fixture fixture;
    StudioTreeState state;

    const std::vector<StudioTreeRow> none =
        studioOutlinerRows(fixture.context.getScene(), {}, state);
    for (const StudioTreeRow& row : none) { CNA_STUDIO_EXPECT(!row.selected); }

    const std::vector<StudioTreeRow> some =
        studioOutlinerRows(fixture.context.getScene(), {fixture.weapon, fixture.camera}, state);

    CNA_STUDIO_EXPECT(rowNamed(some, "Weapon")->selected);
    CNA_STUDIO_EXPECT(rowNamed(some, "Main Camera")->selected);
    CNA_STUDIO_EXPECT(!rowNamed(some, "Player")->selected);
}

CNA_STUDIO_TEST(ClickingARowSelectsItThroughTheContext)
{
    // Through the context, not into a field of the panel's own. The viewport, the inspector and
    // the gizmos all read the context's selection, and a panel with a private one disagrees with
    // the rest of the editor the moment anything else changes it.
    Fixture fixture;
    StudioTreeState state;

    const std::unique_ptr<StudioShell> shell = shellShowingTheOutliner();

    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("outliner",
        [&](StudioFrame& frame, const UiRect& bounds) {
            (void)studioOutlinerPanel(frame, bounds, fixture.context, state);
            if (frame.isDrawPass()) { panelBounds = bounds; }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!panelBounds.isEmpty());
    CNA_STUDIO_EXPECT(fixture.context.getSelection().empty());

    // The first row, whatever height the theme gives it. Found by walking down from the top of the
    // panel rather than by assuming a row height, so a metric change does not turn this into a
    // test that clicks empty space and passes for the wrong reason.
    bool selectedSomething = false;
    for (float y = panelBounds.top() + 2.0f;
         y < panelBounds.top() + 80.0f && !selectedSomething; y += 3.0f)
    {
        const float x = panelBounds.centerX();
        shell->renderFrame(at(x, y, true));
        shell->renderFrame(at(x, y, false));
        selectedSomething = !fixture.context.getSelection().empty();
    }

    CNA_STUDIO_EXPECT(selectedSomething);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{1});
    CNA_STUDIO_EXPECT(fixture.context.getSelection().front() == fixture.camera);
}

/**
 * Shift takes a range and Ctrl takes one more row (`plan.md` STUDIO-13003).
 *
 * The tree reported the two as one flag, so Shift was a second spelling of Control -- which turns
 * the gesture every list in every application uses for "that many" into one that picks up a single
 * row. They are different questions: one asks for one more, the other for everything between here
 * and where the user last clicked.
 */
CNA_STUDIO_TEST(AShiftRangeTakesEverythingBetweenTheAnchorAndTheClick)
{
    Fixture fixture;
    StudioTreeState state;

    // Display order for this scene is Main Camera, Player, Shield, Weapon: parents before their
    // children, and siblings in the scene's own order, which sorts by name rather than by when
    // they were added. Asserted rather than assumed, because a range is *defined* by that order
    // and a reader who guessed the insertion order would mis-read every expectation below.
    const std::vector<StudioTreeRow> shownRows =
        studioOutlinerRows(fixture.context.getScene(), {}, state);
    CNA_STUDIO_EXPECT_EQ(shownRows.size(), std::size_t{4});
    CNA_STUDIO_EXPECT_EQ(shownRows[2].label, std::string{"Shield"});
    CNA_STUDIO_EXPECT_EQ(shownRows[3].label, std::string{"Weapon"});

    const std::vector<Uuid> all = studioOutlinerRange(
        fixture.context.getScene(), state, {}, fixture.camera, fixture.weapon);
    CNA_STUDIO_EXPECT_EQ(all.size(), std::size_t{4});
    CNA_STUDIO_EXPECT(all.front() == fixture.camera);
    CNA_STUDIO_EXPECT(all.back() == fixture.weapon);

    // A range crossing a hierarchy boundary is still contiguous *on the screen*: Player and the
    // first of its children, and not the second.
    const std::vector<Uuid> part = studioOutlinerRange(
        fixture.context.getScene(), state, {}, fixture.player, fixture.shield);
    CNA_STUDIO_EXPECT_EQ(part.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(part.front() == fixture.player);
    CNA_STUDIO_EXPECT(part.back() == fixture.shield);

    // Either end may be the earlier one: a user drags a selection upwards as often as downwards.
    const std::vector<Uuid> backwards = studioOutlinerRange(
        fixture.context.getScene(), state, {}, fixture.shield, fixture.player);
    CNA_STUDIO_EXPECT_EQ(backwards.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(backwards.front() == fixture.player);

    // One row is a range of one, and the same row twice is not a mistake to refuse.
    CNA_STUDIO_EXPECT_EQ(studioOutlinerRange(fixture.context.getScene(), state, {}, fixture.player,
                                             fixture.player)
                             .size(),
                         std::size_t{1});

    // A row hidden inside a closed branch is not between the two as far as anybody looking can
    // tell, so it is not in the range -- and with the branch closed, an end inside it is not a
    // range at all.
    state.setExpanded(fixture.player.toString(), false);
    CNA_STUDIO_EXPECT_EQ(
        studioOutlinerRange(fixture.context.getScene(), state, {}, fixture.camera, fixture.player)
            .size(),
        std::size_t{2});
    CNA_STUDIO_EXPECT(
        studioOutlinerRange(fixture.context.getScene(), state, {}, fixture.camera, fixture.shield)
            .empty());
    state.setExpanded(fixture.player.toString(), true);

    // And a search narrows what "between" means, for the same reason: the range is over what is
    // shown, not over what exists.
    const StudioOutlinerFilter filter =
        studioOutlinerFilter(fixture.context.getScene(), "weapon");
    const std::vector<Uuid> filtered = studioOutlinerRange(
        fixture.context.getScene(), state, filter, fixture.player, fixture.weapon);
    CNA_STUDIO_EXPECT_EQ(filtered.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(
        studioOutlinerRange(fixture.context.getScene(), state, filter, fixture.camera,
                            fixture.weapon)
            .empty());

    // An unknown id is no range rather than a guess.
    CNA_STUDIO_EXPECT(
        studioOutlinerRange(fixture.context.getScene(), state, {}, Uuid{}, fixture.weapon).empty());
}

/**
 * The two modifiers reach the panel as two questions (`plan.md` STUDIO-13003).
 *
 * The case above tests what a range *is*; this one tests that a user pressing Shift gets one. They
 * are separate gates on purpose: the tree reported Shift and Control as a single `additive` flag,
 * so a range function that worked perfectly was never called, and the whole feature was dead with
 * every assertion above it passing.
 */
CNA_STUDIO_TEST(ShiftClickingTakesTheRunAndControlClickingTakesOneMoreRow)
{
    Fixture fixture;
    StudioTreeState state;

    const std::unique_ptr<StudioShell> shell = shellShowingTheOutliner();

    std::vector<StudioTreeRow> rows;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("outliner",
        [&](StudioFrame& frame, const UiRect& bounds) {
            (void)studioOutlinerPanel(frame, bounds, fixture.context, state);
            if (frame.isDrawPass())
            {
                panelBounds = bounds;
                rows = studioOutlinerRows(fixture.context.getScene(),
                                          fixture.context.getSelection(), state);
            }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!panelBounds.isEmpty());
    CNA_STUDIO_EXPECT_EQ(rows.size(), std::size_t{4});

    // By label rather than by index: the display order is the document's, and a hard-coded row
    // number would click a different entity the day something is renamed.
    const auto rowY = [&](std::string_view label) {
        const float rowHeight =
            static_cast<float>(shell->theme().metric(StudioMetric::RowHeight));
        for (std::size_t i = 0; i < rows.size(); ++i)
        {
            if (rows[i].label == label)
            {
                return panelBounds.top() + (static_cast<float>(i) + 0.5f) * rowHeight;
            }
        }
        return -1.0f;
    };

    const auto clickRow = [&](std::string_view label, bool control, bool shift) {
        const float y = rowY(label);
        CNA_STUDIO_EXPECT(y > 0.0f);
        UiInputState down = at(panelBounds.centerX(), y, /*leftDown=*/true);
        down.modifiers.control = control;
        down.modifiers.shift = shift;
        UiInputState up = at(panelBounds.centerX(), y, /*leftDown=*/false);
        up.modifiers.control = control;
        up.modifiers.shift = shift;
        shell->renderFrame(down);
        shell->renderFrame(up);
    };

    const auto selected = [&](const Uuid& id) {
        const std::vector<Uuid>& current = fixture.context.getSelection();
        return std::find(current.begin(), current.end(), id) != current.end();
    };

    // A plain click sets the anchor the range will be measured from.
    clickRow("Main Camera", /*control=*/false, /*shift=*/false);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{1});

    // Shift takes everything down to here, which is the whole point: four rows from one gesture,
    // where a flag conflated with Control would have left two.
    clickRow("Weapon", /*control=*/false, /*shift=*/true);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{4});
    CNA_STUDIO_EXPECT(selected(fixture.camera));
    CNA_STUDIO_EXPECT(selected(fixture.player));
    CNA_STUDIO_EXPECT(selected(fixture.shield));
    CNA_STUDIO_EXPECT(selected(fixture.weapon));

    // The anchor did not move, so a second Shift-click measures from the same place rather than
    // from the far end of the first run -- shortening the selection instead of extending it.
    clickRow("Player", /*control=*/false, /*shift=*/true);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(selected(fixture.camera));
    CNA_STUDIO_EXPECT(selected(fixture.player));

    // Control takes one more row and leaves the run alone, and clicking it again gives it back.
    clickRow("Weapon", /*control=*/true, /*shift=*/false);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{3});
    CNA_STUDIO_EXPECT(selected(fixture.weapon));
    clickRow("Weapon", /*control=*/true, /*shift=*/false);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(!selected(fixture.weapon));

    // Control moved the anchor to the row it touched, so Ctrl+Shift from there adds a second run
    // to the first rather than replacing it.
    clickRow("Shield", /*control=*/true, /*shift=*/false);
    clickRow("Weapon", /*control=*/true, /*shift=*/true);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{4});
    CNA_STUDIO_EXPECT(selected(fixture.camera));
    CNA_STUDIO_EXPECT(selected(fixture.weapon));

    // And a plain click ends it: one row, everything else dropped.
    clickRow("Player", /*control=*/false, /*shift=*/false);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{1});
    CNA_STUDIO_EXPECT(selected(fixture.player));
}

CNA_STUDIO_TEST(AnEmptyOutlinerSaysWhichEmptyItIs)
{
    // "No project is open" and "this scene is empty" call for different next actions. A panel that
    // gave the same words for both would send half its readers looking in the wrong place.
    StudioContext empty;
    StudioTreeState state;

    const std::unique_ptr<StudioShell> shell = shellShowingTheOutliner();

    std::size_t rowsTotal = 1;
    CNA_STUDIO_EXPECT(shell->setPanelContent("outliner",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioOutlinerResult result = studioOutlinerPanel(frame, bounds, empty, state);
            if (frame.isDrawPass()) { rowsTotal = result.rowsTotal; }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(rowsTotal, std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(ADeepSceneIsDrawnRatherThanDescended)
{
    // Two failures guarded at once. A chain thousands deep must not recurse until the stack runs
    // out -- a crash on opening somebody's scene is the worst outcome an outliner has -- and a
    // scene with thousands of entities must cost a screenful, not a scene.
    StudioContext context;
    StudioTreeState state;

    Uuid parent;
    for (int i = 0; i < 2000; ++i)
    {
        parent = addEntity(context.getScene(), "entity " + std::to_string(i), parent);
    }

    const std::vector<StudioTreeRow> rows =
        studioOutlinerRows(context.getScene(), {}, state);

    // Depth-limited, so the flattener returns rather than running out of stack.
    CNA_STUDIO_EXPECT(!rows.empty());
    CNA_STUDIO_EXPECT(rows.size() <= std::size_t{2000});

    const std::unique_ptr<StudioShell> shell = shellShowingTheOutliner();
    std::size_t drawn = 0;
    CNA_STUDIO_EXPECT(shell->setPanelContent("outliner",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioOutlinerResult result = studioOutlinerPanel(frame, bounds, context, state);
            if (frame.isDrawPass()) { drawn = result.rowsDrawn; }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(drawn > 0);
    CNA_STUDIO_EXPECT(drawn < std::size_t{100});
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});
}

// ------------------------------------------------------------------------------------------------
// Showing and hiding from the row (STUDIO-35060)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(EveryOutlinerRowCarriesAVisibilityToggle)
{
    // An outliner where hiding an entity means selecting it, finding the Details panel and
    // unticking a box is one where nobody hides anything -- and hiding things is how a large scene
    // is worked on at all. The affordance has to be on the row.
    SceneDocument scene;
    const Uuid visible = scene.addEntity(StudioEntity{Uuid::generate(), "Visible"});
    StudioEntity hiddenEntity{Uuid::generate(), "Hidden"};
    hiddenEntity.setEnabled(false);
    const Uuid hidden = scene.addEntity(std::move(hiddenEntity));

    StudioTreeState state;
    const std::vector<StudioTreeRow> rows = studioOutlinerRows(scene, {}, state);

    const auto rowFor = [&rows](const Uuid& id) -> const StudioTreeRow* {
        for (const StudioTreeRow& row : rows)
        {
            if (row.id == id.toString()) { return &row; }
        }
        return nullptr;
    };

    CNA_STUDIO_EXPECT(rowFor(visible) != nullptr);
    CNA_STUDIO_EXPECT(rowFor(visible)->toggleIcon == StudioIcon::Visible);
    CNA_STUDIO_EXPECT(rowFor(visible)->toggleOffIcon == StudioIcon::Hidden);
    CNA_STUDIO_EXPECT(rowFor(visible)->toggleOn);

    // The pair has to be one drawing with one difference or the control reads as two unrelated
    // states rather than as on and off, and the tooltip says what the click will *do* rather than
    // what the state *is* -- "Hidden" on a button is a label a user has to invert to use.
    CNA_STUDIO_EXPECT(!rowFor(hidden)->toggleOn);
    CNA_STUDIO_EXPECT_EQ(rowFor(visible)->toggleTooltip, std::string{"Hide this entity"});
    CNA_STUDIO_EXPECT_EQ(rowFor(hidden)->toggleTooltip, std::string{"Show this entity"});
}

CNA_STUDIO_TEST(EveryOutlinerRowIsBothADragSourceAndADropTarget)
{
    // `STUDIO-07058`. Rearranging a hierarchy is dragging one entity onto another, so both roles
    // belong to every row -- there is no such thing as an entity that can be moved but cannot be
    // moved into, or the other way round.
    //
    // The dragged value is the entity's *id*, not its name. Two entities may share a name, and a
    // reparent that picked whichever one the walk found first would be a rearrangement the user did
    // not ask for and cannot undo into the one they wanted.
    Fixture fixture;
    StudioTreeState state;

    const std::vector<StudioTreeRow> rows =
        studioOutlinerRows(fixture.context.getScene(), {}, state);
    CNA_STUDIO_EXPECT(!rows.empty());

    for (const StudioTreeRow& row : rows)
    {
        CNA_STUDIO_EXPECT_EQ(row.dragType, std::string{kStudioEntityDragType});
        CNA_STUDIO_EXPECT_EQ(row.dragValue, row.id);
        CNA_STUDIO_EXPECT(Uuid::parse(row.dragValue).isValid());

        // Two accepted types, and the order matters: a row means "reparent" to an entity and
        // "put one of these in the scene" to an asset (STUDIO-09008).
        CNA_STUDIO_EXPECT_EQ(row.dropTypes.size(), std::size_t{2});
        if (row.dropTypes.size() == 2)
        {
            CNA_STUDIO_EXPECT_EQ(row.dropTypes[0], std::string{kStudioEntityDragType});
            CNA_STUDIO_EXPECT_EQ(row.dropTypes[1], std::string{kStudioAssetDragType});
        }
    }

    // And they are *different* types, so a texture dropped on a row does not read as a reparent.
    CNA_STUDIO_EXPECT(kStudioEntityDragType != kStudioAssetDragType);
}

CNA_STUDIO_TEST(DroppingARowOnAnotherReparentsItAsOneUndoEntry)
{
    // One entry for the whole move. The children come with their parent because they are found
    // *through* it, so there is nothing else to record -- and an undo that put the parent back
    // while leaving its children behind would be worse than no undo at all.
    Fixture fixture;
    StudioTreeState state;

    CNA_STUDIO_EXPECT(fixture.context.getScene().getChildren(fixture.camera).empty());
    const std::size_t before = fixture.context.getHistory().getCount();

    const std::unique_ptr<StudioShell> shell = shellShowingTheOutliner();

    std::vector<StudioTreeRow> rows;
    StudioOutlinerResult last;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("outliner",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioOutlinerResult result =
                studioOutlinerPanel(frame, bounds, fixture.context, state);
            if (frame.isInputPass()) { last = result; }
            if (frame.isDrawPass())
            {
                panelBounds = bounds;
                rows = studioOutlinerRows(fixture.context.getScene(),
                                          fixture.context.getSelection(), state);
            }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!panelBounds.isEmpty());

    // Main Camera is the row to drop onto. Found by label rather than by index, because the order
    // is the document's and a test that hard-coded a row number would move a different entity the
    // day something is renamed.
    const auto rowY = [&](std::string_view label) {
        const float rowHeight =
            static_cast<float>(shell->theme().metric(StudioMetric::RowHeight));
        for (std::size_t i = 0; i < rows.size(); ++i)
        {
            if (rows[i].label == label)
            {
                return panelBounds.top() + (static_cast<float>(i) + 0.5f) * rowHeight;
            }
        }
        return -1.0f;
    };

    const float onto = rowY("Main Camera");
    CNA_STUDIO_EXPECT(onto > 0.0f);

    // Driven through the frame's own drag, because "does a drop reparent" is the thing under test
    // rather than "does a row start a drag", which the case above covers.
    StudioFrame::StudioDragPayload payload;
    payload.type = std::string{kStudioEntityDragType};
    payload.value = fixture.player.toString();
    payload.label = "Player";

    shell->renderFrame(at(panelBounds.centerX(), onto, /*leftDown=*/true));
    CNA_STUDIO_EXPECT(shell->frame().beginDrag(shell->frame().ids().make("source"), payload));

    shell->renderFrame(at(panelBounds.centerX(), onto, /*leftDown=*/true));
    shell->renderFrame(at(panelBounds.centerX(), onto));

    CNA_STUDIO_EXPECT(last.reparented);
    CNA_STUDIO_EXPECT(!last.reparentRefused);

    const std::vector<Uuid> children = fixture.context.getScene().getChildren(fixture.camera);
    CNA_STUDIO_EXPECT_EQ(children.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(children.front() == fixture.player);

    // Exactly one entry, and undoing it puts the entity back where it was.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before + 1);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT(fixture.context.getScene().getChildren(fixture.camera).empty());

    // And the children travelled with it and are still there afterwards.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getChildren(fixture.player).size(),
                         std::size_t{2});
}

/**
 * A drag moves the selection it started on (`plan.md` STUDIO-13004).
 *
 * The drop path reparented the one entity the payload named, so a user who had just shift-selected
 * forty and dragged one of them moved one and left thirty-nine -- with the drag preview saying
 * nothing about which it would be. The decision is a CNA-free function over the scene, so the rule
 * can be asserted without a frame, a shell or a drag.
 */
CNA_STUDIO_TEST(ADragTakesTheWholeSelectionWhenItStartsOnPartOfIt)
{
    Fixture fixture;
    const SceneDocument& scene = fixture.context.getScene();

    // Nothing selected, or a row outside the selection: the drag is that row alone. It does not
    // quietly take a selection the user is not pointing at, whose result would be off the screen.
    CNA_STUDIO_EXPECT_EQ(studioOutlinerDragSet(scene, {}, fixture.player).size(), std::size_t{1});
    const std::vector<Uuid> outside =
        studioOutlinerDragSet(scene, {fixture.weapon, fixture.shield}, fixture.camera);
    CNA_STUDIO_EXPECT_EQ(outside.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(outside.front() == fixture.camera);

    // Starting on part of the selection takes all of it, in selection order.
    const std::vector<Uuid> whole =
        studioOutlinerDragSet(scene, {fixture.weapon, fixture.shield}, fixture.shield);
    CNA_STUDIO_EXPECT_EQ(whole.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(whole.front() == fixture.weapon);
    CNA_STUDIO_EXPECT(whole.back() == fixture.shield);

    // A child selected alongside its parent is left out: it is already coming, and reparenting it
    // as well would tear it out of the thing it is travelling with and leave it a sibling.
    const std::vector<Uuid> withChild =
        studioOutlinerDragSet(scene, {fixture.player, fixture.weapon}, fixture.player);
    CNA_STUDIO_EXPECT_EQ(withChild.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(withChild.front() == fixture.player);

    // Including when the drag started *on* the child: the parent is what moves either way, because
    // which row the pointer was over does not change what the selection contains.
    const std::vector<Uuid> fromChild =
        studioOutlinerDragSet(scene, {fixture.player, fixture.weapon}, fixture.weapon);
    CNA_STUDIO_EXPECT_EQ(fromChild.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(fromChild.front() == fixture.player);

    // An entity the scene does not have is not something to drag.
    CNA_STUDIO_EXPECT(studioOutlinerDragSet(scene, {}, Uuid::generate()).empty());
    CNA_STUDIO_EXPECT(studioOutlinerDragSet(scene, {}, Uuid{}).empty());
}

/**
 * What a drop would do is decided before anything is done (`plan.md` STUDIO-13004).
 */
CNA_STUDIO_TEST(AReparentPlanRefusesTheWholeDropRatherThanMovingSomeOfIt)
{
    Fixture fixture;
    const SceneDocument& scene = fixture.context.getScene();

    // The ordinary case: two roots onto a third parent.
    const StudioReparentPlan ordinary =
        studioOutlinerReparentPlan(scene, {fixture.weapon, fixture.shield}, fixture.camera);
    CNA_STUDIO_EXPECT(!ordinary.refused);
    CNA_STUDIO_EXPECT_EQ(ordinary.entities.size(), std::size_t{2});

    // One member that would make a ring refuses the whole gesture rather than moving the other.
    // A drag that moved four of five would leave a hierarchy the user did not ask for and cannot
    // see the shape of, and one Ctrl+Z would not be an obvious way back to what they had.
    const StudioReparentPlan ring =
        studioOutlinerReparentPlan(scene, {fixture.camera, fixture.player}, fixture.weapon);
    CNA_STUDIO_EXPECT(ring.refused);
    CNA_STUDIO_EXPECT(ring.entities.empty());

    // Dropping something onto itself is the same refusal.
    CNA_STUDIO_EXPECT(studioOutlinerReparentPlan(scene, {fixture.player}, fixture.player).refused);

    // An entity already directly under the target is a no-op, not a conflict: it drops out of the
    // plan and the rest still move. An undo entry that undoes nothing is history a user stops
    // trusting.
    const StudioReparentPlan partly =
        studioOutlinerReparentPlan(scene, {fixture.weapon, fixture.camera}, fixture.player);
    CNA_STUDIO_EXPECT(!partly.refused);
    CNA_STUDIO_EXPECT_EQ(partly.entities.size(), std::size_t{1});
    CNA_STUDIO_EXPECT(partly.entities.front() == fixture.camera);

    // And a set that is *entirely* no-ops is nothing to do rather than a refusal: the two are
    // different answers and the panel says different things about them.
    const StudioReparentPlan nothing =
        studioOutlinerReparentPlan(scene, {fixture.weapon}, fixture.player);
    CNA_STUDIO_EXPECT(!nothing.refused);
    CNA_STUDIO_EXPECT(nothing.entities.empty());

    // The nil parent is the root, which is a real destination -- it is what a detach means.
    const StudioReparentPlan toRoot = studioOutlinerReparentPlan(scene, {fixture.weapon}, Uuid{});
    CNA_STUDIO_EXPECT(!toRoot.refused);
    CNA_STUDIO_EXPECT_EQ(toRoot.entities.size(), std::size_t{1});

    // A target the scene no longer has is nothing to do rather than a guess.
    CNA_STUDIO_EXPECT(
        studioOutlinerReparentPlan(scene, {fixture.weapon}, Uuid::generate()).entities.empty());
}

/**
 * And the whole move is one undo entry (`plan.md` STUDIO-13004).
 */
CNA_STUDIO_TEST(ADropThatMovesASelectionIsOneUndoEntry)
{
    Fixture fixture;
    StudioTreeState state;

    // Weapon and Shield are Player's children; selected together and dropped on Main Camera they
    // both become its children, and one Ctrl+Z puts both back.
    fixture.context.setSelection({fixture.weapon, fixture.shield});
    const std::size_t before = fixture.context.getHistory().getCursor();

    const std::unique_ptr<StudioShell> shell = shellShowingTheOutliner();

    std::vector<StudioTreeRow> rows;
    StudioOutlinerResult last;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("outliner",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioOutlinerResult result =
                studioOutlinerPanel(frame, bounds, fixture.context, state);
            if (frame.isInputPass()) { last = result; }
            if (frame.isDrawPass())
            {
                panelBounds = bounds;
                rows = studioOutlinerRows(fixture.context.getScene(),
                                          fixture.context.getSelection(), state);
            }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!panelBounds.isEmpty());

    const float rowHeight = static_cast<float>(shell->theme().metric(StudioMetric::RowHeight));
    float onto = -1.0f;
    for (std::size_t i = 0; i < rows.size(); ++i)
    {
        if (rows[i].label == "Main Camera")
        {
            onto = panelBounds.top() + (static_cast<float>(i) + 0.5f) * rowHeight;
        }
    }
    CNA_STUDIO_EXPECT(onto > 0.0f);

    // The payload names one entity, as a drag always does. What it *moves* is the selection it
    // started on, which is the whole of this case.
    StudioFrame::StudioDragPayload payload;
    payload.type = std::string{kStudioEntityDragType};
    payload.value = fixture.weapon.toString();
    payload.label = "Weapon";

    shell->renderFrame(at(panelBounds.centerX(), onto, /*leftDown=*/true));
    CNA_STUDIO_EXPECT(shell->frame().beginDrag(shell->frame().ids().make("source"), payload));
    shell->renderFrame(at(panelBounds.centerX(), onto, /*leftDown=*/true));
    shell->renderFrame(at(panelBounds.centerX(), onto));

    CNA_STUDIO_EXPECT(last.reparented);
    CNA_STUDIO_EXPECT(!last.reparentRefused);
    CNA_STUDIO_EXPECT_EQ(last.reparentedCount, std::size_t{2});

    const std::vector<Uuid> moved = fixture.context.getScene().getChildren(fixture.camera);
    CNA_STUDIO_EXPECT_EQ(moved.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(fixture.context.getScene().getChildren(fixture.player).empty());

    // One entry for the gesture the user made once, and undoing it puts both back under Player.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCursor(), before + 1);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT(fixture.context.getScene().getChildren(fixture.camera).empty());
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getChildren(fixture.player).size(),
                         std::size_t{2});
    CNA_STUDIO_EXPECT(fixture.context.getHistory().redo());
    CNA_STUDIO_EXPECT_EQ(fixture.context.getScene().getChildren(fixture.camera).size(),
                         std::size_t{2});
}

CNA_STUDIO_TEST(ADropThatWouldMakeACycleIsRefusedWithoutAnUndoEntry)
{
    // `reparentEntity` rejects the cycle itself and leaves the scene untouched, so pushing the
    // command would be *harmless* -- and would put an entry on the undo stack that undoes nothing.
    // A history with entries that do nothing is a history a user stops trusting, which costs more
    // than the move they were refused.
    Fixture fixture;
    StudioTreeState state;

    // Player is Weapon's parent, so dropping Player onto Weapon would make the tree a ring.
    CNA_STUDIO_EXPECT(fixture.context.getScene().isAncestorOf(fixture.player, fixture.weapon));
    const std::size_t before = fixture.context.getHistory().getCount();

    const std::unique_ptr<StudioShell> shell = shellShowingTheOutliner();

    std::vector<StudioTreeRow> rows;
    StudioOutlinerResult last;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("outliner",
        [&](StudioFrame& frame, const UiRect& bounds) {
            const StudioOutlinerResult result =
                studioOutlinerPanel(frame, bounds, fixture.context, state);
            if (frame.isInputPass()) { last = result; }
            if (frame.isDrawPass())
            {
                panelBounds = bounds;
                rows = studioOutlinerRows(fixture.context.getScene(),
                                          fixture.context.getSelection(), state);
            }
        }));

    // Expanded, or Weapon is not a row on screen to drop onto.
    state.setExpanded(fixture.player.toString(), true);
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(!panelBounds.isEmpty());

    const float rowHeight = static_cast<float>(shell->theme().metric(StudioMetric::RowHeight));
    float onto = -1.0f;
    for (std::size_t i = 0; i < rows.size(); ++i)
    {
        if (rows[i].label == "Weapon")
        {
            onto = panelBounds.top() + (static_cast<float>(i) + 0.5f) * rowHeight;
        }
    }
    CNA_STUDIO_EXPECT(onto > 0.0f);

    StudioFrame::StudioDragPayload payload;
    payload.type = std::string{kStudioEntityDragType};
    payload.value = fixture.player.toString();
    payload.label = "Player";

    shell->renderFrame(at(panelBounds.centerX(), onto, /*leftDown=*/true));
    CNA_STUDIO_EXPECT(shell->frame().beginDrag(shell->frame().ids().make("source"), payload));

    shell->renderFrame(at(panelBounds.centerX(), onto, /*leftDown=*/true));
    shell->renderFrame(at(panelBounds.centerX(), onto));

    CNA_STUDIO_EXPECT(last.reparentRefused);
    CNA_STUDIO_EXPECT(!last.reparented);

    // Nothing moved and nothing was recorded.
    CNA_STUDIO_EXPECT(fixture.context.getScene().isAncestorOf(fixture.player, fixture.weapon));
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before);
}

// ------------------------------------------------------------------------------------------------
// Dropping an asset into the scene (STUDIO-09008)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(AnAssetBecomesTheEntityThatUsesIt)
{
    // One decision, shared by the viewport and the hierarchy: a `.gltf` is a ModelRenderer wherever
    // it lands. Two call sites each with their own switch would be two answers that drift, and the
    // drift shows up as "it works if I drop it on the tree".
    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    AssetDatabase assets;

    const auto track = [&assets](const std::string& path, AssetType type) {
        AssetRecord record;
        record.id = Uuid::generate();
        record.sourcePath = path;
        record.type = type;
        const Uuid id = record.id;
        CNA_STUDIO_EXPECT(assets.add(std::move(record)));
        return id;
    };

    for (const auto& [path, type, component, property] :
         {std::tuple{"Assets/hero.png", AssetType::Texture2D,
                     BuiltinComponentIds::kSpriteRenderer, "texture"},
          std::tuple{"Assets/crate.gltf", AssetType::Model,
                     BuiltinComponentIds::kModelRenderer, "model"},
          std::tuple{"Assets/jump.wav", AssetType::SoundEffect,
                     BuiltinComponentIds::kAudioSource, "clip"}})
    {
        const Uuid id = track(path, type);
        CNA_STUDIO_EXPECT(studioAssetDropKind(type) == StudioAssetDropKind::Entity);

        StudioEntity entity;
        const StudioVector3 where{3.0f, 4.0f, 5.0f};
        CNA_STUDIO_EXPECT(studioEntityForAsset(assets, id, registry, where, entity));

        // Named after the file, without its extension: an entity called "Entity" in a scene of
        // forty is one nobody finds twice.
        CNA_STUDIO_EXPECT(entity.getName().find('.') == std::string::npos);
        CNA_STUDIO_EXPECT(!entity.getName().empty());

        // A transform first and always, or every viewport operation would have to special-case it.
        const StudioComponent* transform = entity.findComponent(BuiltinComponentIds::kTransform);
        CNA_STUDIO_EXPECT(transform != nullptr);
        if (transform != nullptr)
        {
            const StudioVector3 position = transform->getProperty("position").get<StudioVector3>();
            CNA_STUDIO_EXPECT_EQ(position.x, 3.0f);
            CNA_STUDIO_EXPECT_EQ(position.y, 4.0f);
        }

        const StudioComponent* renderer = entity.findComponent(component);
        CNA_STUDIO_EXPECT(renderer != nullptr);
        if (renderer == nullptr) { continue; }

        const PropertyValue reference = renderer->getProperty(property);
        CNA_STUDIO_EXPECT(reference.getType() == PropertyType::AssetReference);
        CNA_STUDIO_EXPECT_EQ(reference.get<PropertyValue::AssetReference>().id.toString(),
                             id.toString());

        // Defaults applied, so a dropped asset behaves like one added through the inspector rather
        // than like an entity carrying one property and no others.
        CNA_STUDIO_EXPECT(renderer->getProperties().size() > 1);
    }
}

CNA_STUDIO_TEST(AnAssetWithNoPlaceInASceneIsRefusedWithAReasonThatNamesIt)
{
    ComponentRegistry registry;
    registerBuiltinComponents(registry);

    AssetDatabase assets;

    AssetRecord scene;
    scene.id = Uuid::generate();
    scene.sourcePath = "Assets/Level.cnascene";
    scene.type = AssetType::Scene;
    const Uuid sceneId = scene.id;
    CNA_STUDIO_EXPECT(assets.add(std::move(scene)));

    CNA_STUDIO_EXPECT(studioAssetDropKind(AssetType::Scene) == StudioAssetDropKind::Unsupported);

    StudioEntity entity;
    CNA_STUDIO_EXPECT(
        !studioEntityForAsset(assets, sceneId, registry, StudioVector3{}, entity));

    // The one refusal that is a *different action* rather than a missing one, so it points at the
    // action instead of apologising.
    const std::string reason = describeStudioAssetDropRefusal(*assets.find(sceneId));
    CNA_STUDIO_EXPECT(!reason.empty());
    CNA_STUDIO_EXPECT(reason.find("opened") != std::string::npos);

    // A kind with no answer names the kind, because that is what tells a user whether they grabbed
    // the wrong file.
    AssetRecord effect;
    effect.id = Uuid::generate();
    effect.sourcePath = "Assets/blur.fx";
    effect.type = AssetType::Effect;
    const Uuid effectId = effect.id;
    CNA_STUDIO_EXPECT(assets.add(std::move(effect)));

    const std::string other = describeStudioAssetDropRefusal(*assets.find(effectId));
    CNA_STUDIO_EXPECT(other.find(toString(AssetType::Effect)) != std::string::npos);

    // And an asset that *can* be dropped is refused nothing.
    AssetRecord texture;
    texture.id = Uuid::generate();
    texture.sourcePath = "Assets/hero.png";
    texture.type = AssetType::Texture2D;
    const Uuid textureId = texture.id;
    CNA_STUDIO_EXPECT(assets.add(std::move(texture)));
    CNA_STUDIO_EXPECT(describeStudioAssetDropRefusal(*assets.find(textureId)).empty());

    // A prefab is neither: it is instantiated, which is a different command over a whole subtree.
    CNA_STUDIO_EXPECT(studioAssetDropKind(AssetType::Prefab) == StudioAssetDropKind::Prefab);
}

CNA_STUDIO_TEST(DroppingAnAssetOnARowReportsItRatherThanReadingAsAReparent)
{
    // Told apart by the payload's *type* rather than by guessing from the id. A row means
    // "reparent" to an entity and "put one of these in the scene" to an asset, and a tree that
    // decided by looking at the value would do whichever the UUID happened to resolve to first.
    Fixture fixture;

    const Uuid texture = Uuid::generate();
    {
        AssetRecord record;
        record.id = texture;
        record.sourcePath = "Assets/hero.png";
        record.type = AssetType::Texture2D;
        CNA_STUDIO_EXPECT(fixture.context.getAssets().add(std::move(record)));
    }

    StudioTreeState state;
    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("outliner"));

    StudioOutlinerResult last;
    UiRect panelBounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("outliner",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioOutlinerResult pass =
                studioOutlinerPanel(frame, area, fixture.context, state);
            if (frame.isInputPass() && pass.assetDropped.isValid()) { last = pass; }
            if (frame.isDrawPass()) { panelBounds = area; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));

    const std::size_t before = fixture.context.getHistory().getCount();

    StudioFrame::StudioDragPayload payload;
    payload.type = std::string{kStudioAssetDragType};
    payload.value = texture.toString();
    payload.label = "hero.png";

    // Swept down the rows, because which row is at which pixel is a metric's business.
    bool dropped = false;
    for (float y = panelBounds.top() + 4.0f;
         y < panelBounds.bottom() - 4.0f && !dropped; y += 6.0f)
    {
        shell->renderFrame(at(panelBounds.centerX(), y, /*leftDown=*/true));
        if (!shell->frame().beginDrag(shell->frame().ids().make("source"), payload)) { continue; }

        shell->renderFrame(at(panelBounds.centerX(), y, /*leftDown=*/true));
        shell->renderFrame(at(panelBounds.centerX(), y));
        dropped = last.assetDropped.isValid();
    }

    if (!dropped)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
                                     "an asset dropped on an outliner row was not reported.");
        return;
    }

    CNA_STUDIO_EXPECT_EQ(last.assetDropped.toString(), texture.toString());
    CNA_STUDIO_EXPECT(last.assetDropParent.isValid());

    // Reported rather than acted on: nothing was reparented and nothing reached the undo stack,
    // because what an asset *becomes* is a decision the binder makes with the viewport.
    CNA_STUDIO_EXPECT(!last.reparented);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before);
}
