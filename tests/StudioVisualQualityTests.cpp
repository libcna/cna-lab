// SPDX-License-Identifier: MS-PL
/**
 * @file StudioVisualQualityTests.cpp
 * @brief The properties that make the Studio shell read as a professional tool rather than a
 *        debug window.
 *
 * `plan.md` Phase 35, the CNA Studio Visual Quality 1.0 workstream.
 *
 * ### Why these are tests and not screenshots
 *
 * A screenshot catches a frame that changed and says nothing about *why* it changed or whether the
 * change was the intended one. The properties below are the ones a reviewer would otherwise have to
 * check by eye on every theme, every scale and every panel: that a layered UI actually has layers,
 * that a colour coding is consistent with the gizmo it teaches, that an icon distinguishes the
 * thing it names. Each is cheap, deterministic, and fails with a sentence rather than with a diff.
 *
 * The golden images stay, and they catch what these cannot: that the pixels are where the layout
 * says they are.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/ShellPanels/StudioOutlinerPanel.hpp"
#include "CNA/Studio/UiCore/StudioIcons.hpp"
#include "CNA/Studio/UiCore/StudioDrawList.hpp"
#include "CNA/Studio/UiCore/StudioFrame.hpp"
#include "CNA/Studio/UiCore/StudioTheme.hpp"
#include "CNA/Studio/UiCore/StudioTreeView.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    /** @brief The perceived lightness of a colour, 0..255, weighted as the eye weights it. */
    float luminance(const StudioColor& colour)
    {
        return 0.2126f * static_cast<float>(colour.r)
             + 0.7152f * static_cast<float>(colour.g)
             + 0.0722f * static_cast<float>(colour.b);
    }

    /** @brief How far apart two colours are, as the largest channel difference. */
    int channelDistance(const StudioColor& a, const StudioColor& b)
    {
        return std::max({std::abs(static_cast<int>(a.r) - static_cast<int>(b.r)),
                         std::abs(static_cast<int>(a.g) - static_cast<int>(b.g)),
                         std::abs(static_cast<int>(a.b) - static_cast<int>(b.b))});
    }

    /**
     * @brief An entity with a transform and, optionally, one more component.
     *
     * A transform on every one, because that is what an entity in a scene has and because the
     * icon rule under test is specifically "a transform and nothing else is still an entity".
     */
    StudioEntity entityWith(std::string name, const char* componentTypeId)
    {
        StudioEntity entity{Uuid::generate(), std::move(name)};
        entity.addComponent(StudioComponent{BuiltinComponentIds::kTransform});
        if (componentTypeId != nullptr)
        {
            entity.addComponent(StudioComponent{componentTypeId});
        }
        return entity;
    }

    /** @brief Both shipped themes, so nothing below is only true of the dark one. */
    std::vector<StudioTheme> bothThemes()
    {
        return {StudioTheme::dark(), StudioTheme::light()};
    }
}

// ------------------------------------------------------------------------------------------------
// Layering (STUDIO-35020, STUDIO-35021)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(TheBackgroundLayersAreActuallyDistinctFromOneAnother)
{
    // The failure this catches is the one the whole workstream started from: a dark UI whose
    // panel, tab strip, tab and chrome are all within a value or two of each other, so nothing
    // reads as sitting on anything. It is invisible in the tokens -- rgb(32,34,38) and
    // rgb(34,36,40) look like two decisions -- and obvious the moment somebody looks at it.
    for (const StudioTheme& theme : bothThemes())
    {
        const StudioColor panel = theme.color(StudioColorRole::PanelBackground);
        const StudioColor strip = theme.color(StudioColorRole::TabStripBackground);
        const StudioColor inactive = theme.color(StudioColorRole::TabInactive);
        const StudioColor active = theme.color(StudioColorRole::PanelHeaderActive);
        const StudioColor chrome = theme.color(StudioColorRole::WindowChrome);

        // A tab strip is a recess: below the panel it sits beside and below its own tabs.
        CNA_STUDIO_EXPECT(luminance(strip) < luminance(panel));
        CNA_STUDIO_EXPECT(luminance(strip) < luminance(inactive));
        // And the active tab is above the inactive ones, which is what "active" looks like.
        CNA_STUDIO_EXPECT(luminance(active) > luminance(inactive));

        // Four pairs that must be separable. Four values is about where a step stops being
        // visible on an LCD at a normal brightness; below that the layering is notional.
        CNA_STUDIO_EXPECT(channelDistance(strip, panel) >= 4);
        CNA_STUDIO_EXPECT(channelDistance(strip, inactive) >= 4);
        CNA_STUDIO_EXPECT(channelDistance(inactive, active) >= 4);
        CNA_STUDIO_EXPECT(channelDistance(chrome, panel) >= 3);
    }
}

CNA_STUDIO_TEST(APanelsOutlineIsDarkerThanEitherPanelItDivides)
{
    // Lighter would read as a highlight -- as though something were raised along that edge -- and
    // a workspace whose every seam is raised is busier than one made of seams. In the light theme
    // the same rule holds for the same reason: an outline lighter than the panels would vanish.
    for (const StudioTheme& theme : bothThemes())
    {
        const StudioColor outline = theme.color(StudioColorRole::PanelOutline);
        CNA_STUDIO_EXPECT(luminance(outline) < luminance(theme.color(StudioColorRole::PanelBackground)));
        CNA_STUDIO_EXPECT(luminance(outline) < luminance(theme.color(StudioColorRole::WindowChrome)));
        // And it has to be visible against what it divides, or it is a token nothing shows.
        CNA_STUDIO_EXPECT(
            channelDistance(outline, theme.color(StudioColorRole::PanelBackground)) >= 8);
    }
}

CNA_STUDIO_TEST(TheAlternatingRowFillIsVisibleAndIsNotAStripe)
{
    // Two failures with one test, because they are the two ends of one judgement. Too close and
    // the stripe does nothing; too far and a list looks like a 1990s table. The window is narrow
    // and stating it is what stops somebody "fixing" it in either direction.
    for (const StudioTheme& theme : bothThemes())
    {
        const int distance = channelDistance(theme.color(StudioColorRole::RowAlternate),
                                             theme.color(StudioColorRole::PanelBackground));
        CNA_STUDIO_EXPECT(distance >= 3);
        CNA_STUDIO_EXPECT(distance <= 12);

        // Hover has to beat the stripe, or half a list's rows would not respond to the pointer.
        CNA_STUDIO_EXPECT(channelDistance(theme.color(StudioColorRole::RowHover),
                                          theme.color(StudioColorRole::PanelBackground))
                          > distance);
    }
}

// ------------------------------------------------------------------------------------------------
// Axis colour coding (STUDIO-35032)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(TheThreeAxisColoursAreTellableApartAndAreRedGreenBlueInOrder)
{
    // Red, green, blue in axis order is the convention every 3D tool shares, and a tool that chose
    // differently would be asking its users to unlearn something true everywhere else. Asserted as
    // "each axis's own channel dominates" rather than against exact values, so the colours can be
    // retuned for contrast without the test becoming a copy of the theme.
    for (const StudioTheme& theme : bothThemes())
    {
        const StudioColor x = theme.color(StudioColorRole::AxisX);
        const StudioColor y = theme.color(StudioColorRole::AxisY);
        const StudioColor z = theme.color(StudioColorRole::AxisZ);
        const StudioColor w = theme.color(StudioColorRole::AxisW);

        CNA_STUDIO_EXPECT(x.r > x.g && x.r > x.b);
        CNA_STUDIO_EXPECT(y.g > y.r && y.g > y.b);
        CNA_STUDIO_EXPECT(z.b > z.r && z.b > z.g);

        // W is a fourth component rather than a fourth axis, and is deliberately neutral: a
        // coloured W beside a coloured Z would read as a direction it does not have.
        CNA_STUDIO_EXPECT(channelDistance(w, StudioColor{w.r, w.r, w.r, w.a}) <= 16);

        // And no two of them may be confusable, which is the whole point of colouring them.
        CNA_STUDIO_EXPECT(channelDistance(x, y) >= 48);
        CNA_STUDIO_EXPECT(channelDistance(y, z) >= 48);
        CNA_STUDIO_EXPECT(channelDistance(x, z) >= 48);
    }
}

// ------------------------------------------------------------------------------------------------
// Iconography (STUDIO-35030)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(AnOutlinerRowSaysWhatItIsFromWhatItCarries)
{
    // The first thing anybody looks for in an outliner is which row is the camera. Reading that
    // off a detail column of component names is reading rather than scanning, and scanning is what
    // an outliner is for.
    SceneDocument scene;

    const Uuid empty = scene.addEntity(entityWith("Empty", nullptr));
    const Uuid camera = scene.addEntity(entityWith("Main Camera", BuiltinComponentIds::kCamera));
    const Uuid light = scene.addEntity(entityWith("Key Light", BuiltinComponentIds::kLight));
    const Uuid model = scene.addEntity(entityWith("Crate", BuiltinComponentIds::kModelRenderer));
    const Uuid sprite = scene.addEntity(entityWith("Player", BuiltinComponentIds::kSpriteRenderer));

    StudioTreeState state;
    const std::vector<StudioTreeRow> rows = studioOutlinerRows(scene, {}, state);

    const auto iconOf = [&rows](const Uuid& id) {
        for (const StudioTreeRow& row : rows)
        {
            if (row.id == id.toString()) { return row.icon; }
        }
        return StudioIcon::None;
    };

    CNA_STUDIO_EXPECT(iconOf(camera) == StudioIcon::Camera);
    CNA_STUDIO_EXPECT(iconOf(light) == StudioIcon::Light);
    CNA_STUDIO_EXPECT(iconOf(model) == StudioIcon::Mesh);
    CNA_STUDIO_EXPECT(iconOf(sprite) == StudioIcon::Sprite);

    // An entity with nothing on it is still an entity and still gets a picture. A blank where
    // every other row has one reads as a row that failed to load rather than as an empty.
    CNA_STUDIO_EXPECT(iconOf(empty) == StudioIcon::Entity);

    // And every row has one, which is the property that makes the column worth its width.
    for (const StudioTreeRow& row : rows)
    {
        CNA_STUDIO_EXPECT(row.icon != StudioIcon::None);
    }
}

CNA_STUDIO_TEST(ACameraWithALightOnItIsACamera)
{
    // Ordered by how much the answer tells a user rather than by how common the component is. An
    // entity carrying both is what somebody was looking for when they scanned for the camera.
    SceneDocument scene;
    StudioEntity entity = entityWith("Sun Camera", BuiltinComponentIds::kLight);
    entity.addComponent(StudioComponent{BuiltinComponentIds::kCamera});
    scene.addEntity(std::move(entity));

    StudioTreeState state;
    CNA_STUDIO_EXPECT(studioOutlinerRows(scene, {}, state).front().icon == StudioIcon::Camera);
}

CNA_STUDIO_TEST(EveryIconHasAUniqueNameThatRoundTripsThroughText)
{
    // The names are what a plugin manifest and a saved toolbar would name an icon by, so a
    // collision is two commands that cannot be told apart in a file. Checked over the whole set
    // rather than over the ones added today, because the set is what has to stay consistent.
    std::set<std::string> names;
    for (std::size_t i = 0; i < static_cast<std::size_t>(StudioIcon::Count); ++i)
    {
        const auto icon = static_cast<StudioIcon>(i);
        const std::string name{studioIconName(icon)};
        CNA_STUDIO_EXPECT(!name.empty());
        CNA_STUDIO_EXPECT(names.insert(name).second);

        StudioIcon parsed = StudioIcon::None;
        CNA_STUDIO_EXPECT(parseStudioIcon(name, parsed));
        CNA_STUDIO_EXPECT(parsed == icon);
    }
}

// ------------------------------------------------------------------------------------------------
// Every primitive has to reach the GPU (STUDIO-35033)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(EveryPrimitiveTheDrawListEmitsNamesADrawableTexture)
{
    // The defect this closes: `drawLine`'s diagonal branch emitted its command against
    // `kUiTextureNone`, and both UI render backends skip a command whose texture id they cannot
    // resolve -- because a command naming a texture that was never created would otherwise sample
    // whatever happens to be bound. So every diagonal line in Studio was dropped on a real device.
    //
    // It survived because almost nothing drew one. It surfaced the day an icon set made of
    // diagonals arrived, as icons that came out as their axis-aligned parts alone: an isometric
    // cube drew as three bars.
    //
    // And the software rasterizer drew them correctly the whole time, so no headless capture
    // showed it. That is the failure mode a preview harness has -- it is a second implementation,
    // and the two agreeing is the thing being tested rather than a given.
    StudioFrame frame;
    frame.setTheme(StudioTheme::dark());

    UiInputState input;
    input.displayWidth = 320.0f;
    input.displayHeight = 240.0f;
    frame.beginFrame(input);
    frame.beginInput();
    frame.beginDraw();

    StudioDrawList& list = frame.drawList();
    const StudioColor colour = frame.theme().color(StudioColorRole::TextPrimary);

    // Standing in for the font atlas, which is what a real shell binds here: an id and the
    // coordinates of the reserved opaque white texel every untextured primitive samples. Without
    // one the default *is* kUiTextureNone -- which is correct for a draw list nobody will render,
    // and is exactly the state in which this assertion would say nothing.
    constexpr UiTextureId kAtlas = 7;
    list.setDefaultTexture(kAtlas, 0.5f, 0.5f);

    list.fillRect(UiRect{10.0f, 10.0f, 40.0f, 20.0f}, colour);
    list.drawLine(10.0f, 40.0f, 60.0f, 40.0f, colour, 2.0f);   // horizontal
    list.drawLine(10.0f, 50.0f, 10.0f, 90.0f, colour, 2.0f);   // vertical
    list.drawLine(20.0f, 100.0f, 80.0f, 160.0f, colour, 2.0f); // diagonal
    list.fillTriangle(100.0f, 10.0f, 140.0f, 10.0f, 120.0f, 50.0f, colour);
    list.strokeRect(UiRect{150.0f, 10.0f, 40.0f, 40.0f}, colour, 1.0f);

    frame.endFrame();

    const UiDrawData& data = frame.drawData();
    std::size_t commands = 0;
    for (const UiDrawList& drawList : data.lists)
    {
        for (const UiDrawCommand& command : drawList.commands)
        {
            if (command.indexCount == 0) { continue; }
            ++commands;
            // The whole assertion. A backend resolves this id or drops the command, and dropping
            // it is silent -- no warning, no missing-texture pink, just geometry that is not there.
            CNA_STUDIO_EXPECT(command.texture != kUiTextureNone);
            CNA_STUDIO_EXPECT_EQ(command.texture, kAtlas);
        }
    }
    CNA_STUDIO_EXPECT(commands > 0);
}

// ------------------------------------------------------------------------------------------------
// Paint order used to be guarded one widget at a time (STUDIO-35063)
//
// `ARowsTrailingToggleIsDrawnOverTheRowFillRatherThanUnderIt` stood here. It pressed nothing and
// named one widget: it read the vertices in the rightmost strip of a tree row and required the last
// thing drawn there not to be the row's own selection fill. It was right, and it was the third
// individual fix for one shape -- so the next widget on a row started from the same trap.
//
// `NothingInteractiveIsPaintedOverByASurfaceDescribedAfterIt`, at the end of this file, is that
// claim made structural (`plan.md` CORE-06). It knows no widget names: it asks the frame for every
// rectangle it routed input to and fails on any that a later, larger interactive surface paints
// over. Reverting the toggle to the old in-place drawing makes it name all three toggles in the
// fixture, by rectangle, which is how this deletion was checked rather than assumed.
// ------------------------------------------------------------------------------------------------

namespace
{
    /**
     * @brief Reports every widget a frame buried under a surface described after it.
     *
     * The shape, stated without naming a widget: `StudioInputRouter` gives a press to the *first*
     * widget described under the pointer, so a control that must win a click against the surface
     * it sits on has to be described before that surface. The draw list is emitted in call order,
     * so being described first also means being painted first -- under the very surface it had to
     * out-rank. Four defects have come out of that shape, each fixed individually.
     *
     * So: for every pair of interactive rectangles where the outer one contains the inner one and
     * was described *later*, something of the inner one must be drawn after the outer one was
     * described. If nothing was, the inner widget is whatever the outer one painted -- which for a
     * tree row is an opaque fill across the whole line.
     *
     * @param frame A frame that has completed both passes.
     * @return One sentence per buried widget, naming the rectangle rather than the widget, because
     *         a guard that knows the widget's name is the kind this replaces.
     */
    std::vector<std::string> buriedWidgets(const StudioFrame& frame)
    {
        std::vector<std::string> problems;

        const std::vector<StudioRecordedInteraction>& widgets = frame.recordedInteractions();

        // Every vertex the draw pass emitted, in order, so "was anything of mine drawn after that"
        // is a scan rather than a second bookkeeping channel through the widgets.
        struct Emitted { float x; float y; std::size_t index; };
        std::vector<Emitted> vertices;
        std::size_t index = 0;
        for (const UiDrawList& list : frame.drawData().lists)
        {
            for (const UiVertex& vertex : list.vertices)
            {
                vertices.push_back(Emitted{vertex.x, vertex.y, index});
                ++index;
            }
        }

        const auto contains = [](const UiRect& outer, const UiRect& inner) {
            // A hair of tolerance, because a row and a control on it are laid out from the same
            // edges and rounding can put one half a pixel outside the other.
            constexpr float slack = 0.5f;
            return outer.left() <= inner.left() + slack && outer.top() <= inner.top() + slack
                && outer.right() + slack >= inner.right()
                && outer.bottom() + slack >= inner.bottom()
                && outer.width * outer.height > inner.width * inner.height;
        };

        for (const StudioRecordedInteraction& inner : widgets)
        {
            if (inner.bounds.width <= 0.0f || inner.bounds.height <= 0.0f) { continue; }

            for (const StudioRecordedInteraction& outer : widgets)
            {
                if (outer.id == inner.id) { continue; }

                // `<` rather than `<=` on the surface's mark: two widgets described with no
                // geometry between them share a vertex count, and a row's toggles are exactly
                // that -- described back to back, then the row. Requiring a strict increase would
                // exempt every widget that draws nothing of its own before the surface arrives,
                // which is the whole population this guard is about. Description *order* is what
                // matters, and the vector is in that order.
                if (outer.describedAtVertex < inner.describedAtVertex) { continue; }
                if (outer.describedAtVertex == inner.describedAtVertex && &outer < &inner)
                {
                    continue;
                }
                if (!contains(outer.bounds, inner.bounds)) { continue; }

                // Anything of the inner widget emitted after the outer one was described. A single
                // vertex is enough: a control painted over the fill puts several there, and one
                // painted under it puts none.
                //
                // Counted over the middle of the rectangle rather than all of it, because widgets
                // laid out side by side share an edge: a label that starts exactly where the
                // disclosure triangle ends puts its first glyph quad on the triangle's right-hand
                // boundary, and reading that as "the triangle was repainted" is how an earlier
                // version of this guard passed a tree whose triangle was buried.
                const float insetX = std::max(1.0f, inner.bounds.width * 0.2f);
                const float insetY = std::max(1.0f, inner.bounds.height * 0.2f);
                const UiRect middle = inner.bounds.inset(UiEdges{insetX, insetY, insetX, insetY});

                bool repainted = false;
                for (const Emitted& vertex : vertices)
                {
                    if (vertex.index < outer.describedAtVertex) { continue; }
                    if (vertex.x < middle.left() || vertex.x > middle.right()) { continue; }
                    if (vertex.y < middle.top() || vertex.y > middle.bottom()) { continue; }
                    repainted = true;
                    break;
                }

                if (repainted) { break; }

                std::ostringstream message;
                message << "an interactive " << inner.bounds.width << "x" << inner.bounds.height
                        << " rectangle at (" << inner.bounds.left() << ", " << inner.bounds.top()
                        << ") is described before a larger interactive surface that covers it, and "
                           "nothing of it is drawn afterwards -- so the surface paints over it. "
                           "Describe it first, as the router requires, and hand its drawing to "
                           "StudioFrame::paintOverSurface (plan.md CORE-06).";
                problems.push_back(message.str());
                break;
            }
        }

        return problems;
    }

    /** @brief Runs both passes over @p describe and returns what it buried. */
    std::vector<std::string> buriedBy(StudioFrame& frame, const UiInputState& input,
                                      const std::function<void(StudioFrame&)>& describe)
    {
        frame.beginFrame(input);
        frame.beginInput();
        describe(frame);
        frame.beginDraw();
        describe(frame);
        frame.endFrame();
        return buriedWidgets(frame);
    }
}

CNA_STUDIO_TEST(NothingInteractiveIsPaintedOverByASurfaceDescribedAfterIt)
{
    // `plan.md` CORE-06, and the guard the three it replaces could not be. Each of those named one
    // widget -- the outliner's eye, the Details panel's asset inspector, a tree's disclosure
    // triangle -- and so defended the three places somebody had already got wrong. This one knows
    // no widget names at all: it asks the frame for every rectangle it routed input to, and fails
    // on any that a later, larger interactive surface paints over. A fifth widget added to a tree
    // row during maintenance is covered the day it is written.
    //
    // It is deliberately about *interactive* rectangles. A decorative element under a fill is a
    // layering choice; a control under one is a feature that cannot be seen, which is worse than a
    // missing one because nothing reports it.
    StudioFrame frame;
    frame.setTheme(StudioTheme::dark());

    UiInputState input;
    input.displayWidth = 480.0f;
    input.displayHeight = 240.0f;

    // A row carrying everything a row can carry: children to disclose, two trailing toggles, and a
    // selection fill -- the largest opaque surface a row draws, and the one that covered the eye.
    std::vector<StudioTreeRow> rows;
    StudioTreeRow parent;
    parent.id = "group";
    parent.label = "Lights";
    parent.hasChildren = true;
    parent.selected = true;
    StudioRowToggle eye;
    eye.icon = StudioIcon::Visible;
    eye.offIcon = StudioIcon::Hidden;
    eye.on = false;
    StudioRowToggle lock;
    lock.icon = StudioIcon::Lock;
    lock.on = false;
    parent.toggles = {eye, lock};
    rows.push_back(parent);

    StudioTreeRow child;
    child.id = "key";
    child.label = "Key Light";
    child.depth = 1;
    child.toggles = {eye};
    rows.push_back(child);

    StudioTreeState state;
    state.setExpanded("group", true);

    const std::vector<std::string> problems =
        buriedBy(frame, input, [&](StudioFrame& f) {
            (void)studioTreeView(f, UiRect{0.0f, 0.0f, 480.0f, 240.0f}, rows, state, "");
        });

    for (const std::string& problem : problems)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__, problem);
    }

    // A scan that saw no nesting would agree with everything. The rows above put a disclosure
    // triangle and three toggles inside two rows, so the guard has something to be right about.
    std::size_t nested = 0;
    for (const StudioRecordedInteraction& inner : frame.recordedInteractions())
    {
        for (const StudioRecordedInteraction& outer : frame.recordedInteractions())
        {
            if (outer.id == inner.id) { continue; }
            if (outer.bounds.width * outer.bounds.height <= inner.bounds.width * inner.bounds.height)
            {
                continue;
            }
            if (outer.bounds.left() <= inner.bounds.left()
                && outer.bounds.right() >= inner.bounds.right()
                && outer.bounds.top() <= inner.bounds.top()
                && outer.bounds.bottom() >= inner.bounds.bottom())
            {
                ++nested;
                break;
            }
        }
    }
    CNA_STUDIO_EXPECT(nested >= 4);
}
