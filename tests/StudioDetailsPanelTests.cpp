// SPDX-License-Identifier: MS-PL
/**
 * @file StudioDetailsPanelTests.cpp
 * @brief The Details panel: the first ported panel that writes to the document.
 *
 * `plan.md` STUDIO-07007.
 *
 * That is the whole difference between this and the outliner, and it decides what is worth testing.
 * Showing a scene wrong is a bad afternoon; editing one wrong is a lost afternoon's work. So the
 * cases here are about *when* a value reaches the document (on commit, never per keystroke), *how*
 * it gets there (through the command history, so Ctrl+Z reaches it), and what happens to input that
 * is not a value at all.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Assets/AssetCommands.hpp"
#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/Assets/AssetDependencies.hpp"
#include "CNA/Studio/Assets/AssetDocumentCache.hpp"
#include "CNA/Studio/Assets/MaterialDocument.hpp"
#include "CNA/Studio/Assets/AssetImporters.hpp"
#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneCommands.hpp"
#include "CNA/Studio/Core/NumberText.hpp"
#include "CNA/Studio/Scene/SceneTransform.hpp"
#include "CNA/Studio/ShellPanels/StudioDetailsPanel.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"
#include "CNA/Studio/UiCore/StudioWidgets.hpp"
#include "CNA/Studio/UiCore/UiSoftwareRasterizer.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <set>
#include <cmath>
#include <memory>
#include <string>

using namespace CNA::Studio;

namespace
{
    float metricOf(const StudioTheme& theme, StudioMetric metric)
    {
        return static_cast<float>(theme.metric(metric));
    }

    UiInputState at(float x, float y, bool leftDown = false)
    {
        UiInputState input;
        input.displayWidth = 1280.0f;
        input.displayHeight = 720.0f;
        input.mouseX = x;
        input.mouseY = y;
        input.mouseInWindow = true;
        input.setMouseDown(UiMouseButton::Left, leftDown);
        return input;
    }

    /** @brief A context with one entity carrying a Transform, selected. */
    struct Fixture
    {
        StudioContext context;
        Uuid entity;

        Fixture()
        {
            StudioEntity subject{Uuid::generate(), "Player"};

            StudioComponent transform{"CNA.Transform"};
            transform.setProperty("position", PropertyValue{StudioVector3{1.0f, 2.0f, 3.0f}});
            subject.getComponents().push_back(std::move(transform));

            entity = subject.getId();
            context.getScene().addEntity(std::move(subject));
            context.select(entity);
        }

        [[nodiscard]] StudioVector3 position() const
        {
            const StudioEntity* found = context.getScene().findEntity(entity);
            if (found == nullptr) { return {}; }
            const StudioComponent* transform = found->findComponent("CNA.Transform");
            if (transform == nullptr) { return {}; }
            return transform->getProperty("position").get<StudioVector3>();
        }

        [[nodiscard]] std::string name() const
        {
            const StudioEntity* found = context.getScene().findEntity(entity);
            return found != nullptr ? found->getName() : std::string{};
        }
    };

    /** @brief The shell with the Details panel raised and driving @p fixture. */
    struct Harness
    {
        std::unique_ptr<StudioShell> shell = std::make_unique<StudioShell>(StudioTheme::dark());
        UiRect bounds;
        StudioDetailsResult last;

        /** @brief Which sections are folded (`plan.md` STUDIO-14001), owned here like the shell's. */
        StudioDetailsState state;

        explicit Harness(StudioContext& context)
        {
            shell->resetLayout();
            shell->renderFrame(at(-1.0f, -1.0f));
            CNA_STUDIO_EXPECT(shell->activatePanel("details"));
            CNA_STUDIO_EXPECT(shell->setPanelContent("details",
                [this, &context](StudioFrame& frame, const UiRect& area) {
                    const StudioDetailsResult result =
                        studioDetailsPanel(frame, area, context, {}, &state);
                    if (frame.isInputPass()) { last = result; }
                    if (frame.isDrawPass()) { bounds = area; }
                }));
            shell->renderFrame(at(-1.0f, -1.0f));
        }

        /** @brief Clicks, starting from an un-pressed frame so the router sees a real press. */
        void click(float x, float y)
        {
            shell->renderFrame(at(x, y, false));
            shell->renderFrame(at(x, y, true));
            shell->renderFrame(at(x, y, false));
        }

        void type(const std::vector<char16_t>& characters, float x, float y)
        {
            UiInputState input = at(x, y);
            input.characters = characters;
            shell->renderFrame(input);
        }

        void press(UiKey key, float x, float y)
        {
            UiInputState input = at(x, y);
            input.setKeyDown(key, true);
            shell->renderFrame(input);
            shell->renderFrame(at(x, y));
        }

        /**
         * @brief Presses at (@p x, @p y) and drags @p dx pixels sideways, then releases.
         *
         * `STUDIO-07055`. The move is delivered in one frame rather than several on purpose: a
         * scrub is anchored to the value at the press, so a gesture that arrives in one jump and
         * one that arrives in twenty have to end at the same number. A test that only ever moved a
         * pixel at a time would pass for an implementation that accumulated frame deltas, which is
         * the implementation this widget deliberately is not.
         */
        void drag(float x, float y, float dx)
        {
            shell->renderFrame(at(x, y, false));
            shell->renderFrame(at(x, y, true));
            shell->renderFrame(at(x + dx, y, true));
            shell->renderFrame(at(x + dx, y, false));
            shell->renderFrame(at(x + dx, y, false));
        }

        /**
         * @brief The same gesture delivered over @p steps frames, as a real pointer delivers it.
         *
         * The one-frame @ref drag above cannot exercise the *merge* at all: it produces a single
         * command, so an implementation with no merge key at all passes it. Only a drag that lands
         * a command per frame can say whether they fold into one undo entry (`STUDIO-07055`).
         */
        void dragOverFrames(float x, float y, float dx, int steps)
        {
            shell->renderFrame(at(x, y, false));
            shell->renderFrame(at(x, y, true));
            for (int step = 1; step <= steps; ++step)
            {
                const float moved = dx * static_cast<float>(step) / static_cast<float>(steps);
                shell->renderFrame(at(x + moved, y, true));
            }
            shell->renderFrame(at(x + dx, y, false));
            shell->renderFrame(at(x + dx, y, false));
        }
    };
}

CNA_STUDIO_TEST(TheDetailsPanelShowsTheSelectedEntitysComponents)
{
    Fixture fixture;
    Harness harness{fixture.context};

    CNA_STUDIO_EXPECT(!harness.bounds.isEmpty());
    CNA_STUDIO_EXPECT_EQ(harness.last.componentCount, std::size_t{1});

    // Name, Enabled, a gap, the component header and its properties.
    CNA_STUDIO_EXPECT(harness.last.rowsDrawn >= std::size_t{4});
    CNA_STUDIO_EXPECT_EQ(harness.shell->frame().phaseViolations(), std::size_t{0});
}

/**
 * A component section folds (`plan.md` STUDIO-14001).
 *
 * Every component was drawn fully expanded with no disclosure and nowhere to remember one, so an
 * entity carrying eight components was a wall a user had to scroll past to reach the ninth. The
 * state is caller-owned, like the World Outliner's tree state and for the same reason: the panel is
 * rebuilt from scratch every frame, so anything it must remember belongs to whoever outlives one.
 */
CNA_STUDIO_TEST(FoldingAComponentSectionHidesItsPropertiesAndKeepsItsHeading)
{
    Fixture fixture;
    Harness harness{fixture.context};

    const std::size_t openRows = harness.last.rowsDrawn;
    const std::size_t openMeasured = harness.last.rowsMeasured;
    CNA_STUDIO_EXPECT(openRows >= std::size_t{4});
    CNA_STUDIO_EXPECT_EQ(harness.state.collapsedCount(), std::size_t{0});

    // The triangle is the leftmost control on the component's header row. Swept rather than
    // computed from the metrics, so a spacing change cannot turn this into a case that clicks
    // empty space and passes for the wrong reason.
    bool folded = false;
    for (float y = harness.bounds.top() + 4.0f;
         y < harness.bounds.top() + 200.0f && !folded; y += 4.0f)
    {
        harness.click(harness.bounds.left() + 12.0f, y);
        folded = harness.state.collapsedCount() == 1;
    }
    CNA_STUDIO_EXPECT(folded);
    CNA_STUDIO_EXPECT(!harness.state.isExpanded("CNA.Transform"));

    // The heading stays -- a section that vanished when closed would be one a user cannot reopen.
    // Everything under it goes, which is the point.
    harness.shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(harness.last.rowsDrawn < openRows);
    CNA_STUDIO_EXPECT_EQ(harness.last.componentCount, std::size_t{1});

    // The measure moved by exactly as much as the draw. The scroll view is sized from the measure,
    // so a pre-pass that kept counting rows the draw had stopped drawing would let the panel
    // scroll past its own last control -- and nothing about the drawn rows would show it. The two
    // are separate `continue`s over the same predicate and each needs its own gate.
    CNA_STUDIO_EXPECT_EQ(openMeasured - harness.last.rowsMeasured,
                         openRows - harness.last.rowsDrawn);

    // And it reopens to exactly what it was. The state holds the *collapsed* set, so a fresh panel
    // starts open: one that began closed would show a column of headings and make the user work to
    // discover their entity has anything on it.
    harness.state.setExpanded("CNA.Transform", true);
    harness.shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(harness.last.rowsDrawn, openRows);
    CNA_STUDIO_EXPECT_EQ(harness.state.collapsedCount(), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(harness.shell->frame().phaseViolations(), std::size_t{0});
}

/**
 * And with no state at all the panel is exactly what it was (`plan.md` STUDIO-14001).
 *
 * What keeps every headless path and the many cases that care about a property rather than about
 * folding meaning what they meant.
 */
CNA_STUDIO_TEST(APanelWithNoFoldStateDrawsEverySectionOpen)
{
    Fixture fixture;

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult result = studioDetailsPanel(frame, area, fixture.context);
            if (frame.isInputPass()) { last = result; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));

    Harness folded{fixture.context};
    CNA_STUDIO_EXPECT_EQ(last.rowsDrawn, folded.last.rowsDrawn);
}

CNA_STUDIO_TEST(WithNothingSelectedThePanelSaysWhatToDoNext)
{
    // "Select an entity to see its details" is a next action; "nothing to show" is a dead end.
    Fixture fixture;
    fixture.context.clearSelection();
    Harness harness{fixture.context};

    CNA_STUDIO_EXPECT_EQ(harness.last.rowsDrawn, std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(harness.last.componentCount, std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(harness.shell->frame().phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(RenamingAnEntityGoesThroughTheHistorySoUndoReachesIt)
{
    Fixture fixture;
    Harness harness{fixture.context};

    // The Name field is the first row's control, which sits in the right-hand column.
    const float x = harness.bounds.left() + harness.bounds.width * 0.75f;
    const float y = harness.bounds.top() + 14.0f;

    harness.click(x, y);
    harness.type({u'!'}, x, y);
    // Nothing yet: a field that wrote per keystroke would put one undo entry per letter.
    CNA_STUDIO_EXPECT_EQ(fixture.name(), std::string{"Player"});

    harness.press(UiKey::Enter, x, y);
    CNA_STUDIO_EXPECT_EQ(fixture.name(), std::string{"Player!"});

    // And the history has it, which is the point of going through a command at all.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().canUndo());
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(fixture.name(), std::string{"Player"});
}

CNA_STUDIO_TEST(EditingOneAxisOfAVectorLeavesTheOthersAlone)
{
    // The failure this catches is the classic one: a vector row that rebuilds all three components
    // from the field being edited and writes the other two back as whatever it last formatted.
    Fixture fixture;
    Harness harness{fixture.context};

    const StudioVector3 before = fixture.position();
    CNA_STUDIO_EXPECT_EQ(before.x, 1.0f);
    CNA_STUDIO_EXPECT_EQ(before.y, 2.0f);
    CNA_STUDIO_EXPECT_EQ(before.z, 3.0f);

    // The y field of the position row: the middle of the three boxes in the right-hand column.
    // Found by sweeping the row rather than by computing a pixel, so a metric change does not turn
    // this into a test that types into nothing and passes.
    bool edited = false;
    for (float y = harness.bounds.top() + 20.0f;
         y < harness.bounds.top() + 200.0f && !edited; y += 6.0f)
    {
        const float columnLeft = harness.bounds.left() + harness.bounds.width * 0.40f;
        const float x = columnLeft + (harness.bounds.right() - columnLeft) * 0.5f;

        harness.click(x, y);
        harness.type({u'9'}, x, y);
        harness.press(UiKey::Enter, x, y);
        edited = fixture.position().y != before.y;
    }

    CNA_STUDIO_EXPECT(edited);
    CNA_STUDIO_EXPECT_EQ(fixture.position().x, 1.0f);
    CNA_STUDIO_EXPECT_EQ(fixture.position().z, 3.0f);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().canUndo());
}

CNA_STUDIO_TEST(TextThatIsNotANumberIsRejectedRatherThanTurnedIntoZero)
{
    // "3abc" is not three, and "" is not zero. Accepting a prefix, or treating a failed parse as a
    // default, is how a typo silently becomes a value the user did not enter and cannot see is
    // wrong -- in a position, which puts their sprite somewhere else.
    Fixture fixture;
    Harness harness{fixture.context};

    const StudioVector3 before = fixture.position();

    // Below the Name and Enabled rows. A string field would accept "abc" perfectly correctly, so
    // sweeping across it would leave an undo entry this case would then blame on the number.
    const StudioTheme theme = StudioTheme::dark();
    const float rowPitch = static_cast<float>(std::max(theme.metric(StudioMetric::ControlHeight),
                                                       theme.metric(StudioMetric::MinimumHitTarget))
                                              + theme.metric(StudioMetric::SpacingXSmall));
    const float firstPropertyRow = harness.bounds.top() + rowPitch * 3.0f;

    for (float y = firstPropertyRow; y < firstPropertyRow + 200.0f; y += 6.0f)
    {
        const float columnLeft = harness.bounds.left() + harness.bounds.width * 0.40f;
        const float x = columnLeft + (harness.bounds.right() - columnLeft) * 0.2f;

        harness.click(x, y);
        harness.type({u'a', u'b', u'c'}, x, y);
        harness.press(UiKey::Enter, x, y);
    }

    CNA_STUDIO_EXPECT_EQ(fixture.position().x, before.x);
    CNA_STUDIO_EXPECT_EQ(fixture.position().y, before.y);
    CNA_STUDIO_EXPECT_EQ(fixture.position().z, before.z);
    CNA_STUDIO_EXPECT(!fixture.context.getHistory().canUndo());
}

CNA_STUDIO_TEST(AnEntityDeletedWhileSelectedIsSaidSoRatherThanCrashedOn)
{
    // The selection outlives the entity whenever something else removes it -- an undo, a script, a
    // second view. Dereferencing what is no longer there would take the editor down with it.
    Fixture fixture;
    Harness harness{fixture.context};
    CNA_STUDIO_EXPECT(harness.last.componentCount > 0);

    (void)fixture.context.getScene().removeEntityRecursive(fixture.entity);
    harness.shell->renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT_EQ(harness.last.componentCount, std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(harness.last.rowsDrawn, std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(harness.shell->frame().phaseViolations(), std::size_t{0});
}

// ---------------------------------------------------------------------------------------------
// The kinds that used to be read-only (plan.md STUDIO-07018, STUDIO-07019)
// ---------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(TurningAnEntityOffGoesThroughTheHistorySoUndoReachesIt)
{
    // It was the one edit in the panel Ctrl+Z could not reach, and it is the one somebody does by
    // accident: the flag decides whether an entity renders, ticks and answers queries at all.
    Fixture fixture;
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(fixture.entity)->isEnabled());

    auto command = std::make_unique<SetEntityEnabledCommand>(fixture.context.getScene(),
                                                             fixture.entity, false);
    CNA_STUDIO_EXPECT(command->isValid());
    fixture.context.execute(std::move(command));

    CNA_STUDIO_EXPECT(!fixture.context.getScene().findEntity(fixture.entity)->isEnabled());
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), std::size_t{1});

    fixture.context.getHistory().undo();
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(fixture.entity)->isEnabled());
}

CNA_STUDIO_TEST(SettingTheEnabledFlagToWhatItAlreadyIsIsRefused)
{
    // An undo stack with no-ops in it makes Ctrl+Z appear to do nothing, which is worse than doing
    // the wrong thing: the user cannot tell how many more to press.
    Fixture fixture;
    const SetEntityEnabledCommand command{fixture.context.getScene(), fixture.entity, true};
    CNA_STUDIO_EXPECT(!command.isValid());

    const SetEntityEnabledCommand missing{fixture.context.getScene(), Uuid::generate(), false};
    CNA_STUDIO_EXPECT(!missing.isValid());
}

CNA_STUDIO_TEST(RepeatedEnabledFlipsMergeIntoOneUndoStep)
{
    Fixture fixture;
    fixture.context.execute(
        std::make_unique<SetEntityEnabledCommand>(fixture.context.getScene(), fixture.entity, false),
        MergePolicy::MergeWithPrevious);
    fixture.context.execute(
        std::make_unique<SetEntityEnabledCommand>(fixture.context.getScene(), fixture.entity, true),
        MergePolicy::MergeWithPrevious);

    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), std::size_t{1});

    // And undoing that one step returns to where it started, not to the intermediate state.
    fixture.context.getHistory().undo();
    CNA_STUDIO_EXPECT(fixture.context.getScene().findEntity(fixture.entity)->isEnabled());
}

CNA_STUDIO_TEST(AQuaternionIsEditedAsAnglesRatherThanAsFourRawNumbers)
{
    // Nobody knows what to type into w to turn something thirty degrees, and four independent
    // numbers is how you produce a value that is not a rotation at all. The panel therefore shows
    // Euler degrees, in the convention the runtime reads back.
    Fixture fixture;

    StudioEntity* entity = fixture.context.getScene().findEntityForEdit(fixture.entity);
    StudioComponent* transform = entity->findComponent("CNA.Transform");
    transform->setProperty("rotation",
                           PropertyValue{quaternionFromEulerDegrees(StudioVector3{0.0f, 90.0f, 0.0f})});

    Harness harness{fixture.context};
    CNA_STUDIO_EXPECT(!harness.bounds.isEmpty());

    const StudioVector3 shown = eulerDegreesOf(
        fixture.context.getScene().findEntity(fixture.entity)
            ->findComponent("CNA.Transform")->getProperty("rotation").get<StudioQuaternion>());
    CNA_STUDIO_EXPECT(std::abs(shown.y - 90.0f) < 0.01f);
}

CNA_STUDIO_TEST(EveryPropertyKindTheSchemaDeclaresGetsAControlRatherThanASummary)
{
    // The failure this catches is a property kind silently falling through to "(not editable
    // yet)": it looks deliberate, reads as a decision, and is how a kind stays unimplemented long
    // after the widget it needed arrived.
    Fixture fixture;

    StudioEntity* entity = fixture.context.getScene().findEntityForEdit(fixture.entity);
    StudioComponent extras{"Test.Kinds"};
    extras.setProperty("colour", PropertyValue{StudioColor{10, 20, 30, 40}});
    extras.setProperty("rect", PropertyValue{StudioRectangle{1, 2, 3, 4}});
    extras.setProperty("four", PropertyValue{StudioVector4{1.0f, 2.0f, 3.0f, 4.0f}});
    extras.setProperty("turn", PropertyValue{StudioQuaternion{}});
    extras.setProperty("asset", PropertyValue{PropertyValue::AssetReference{Uuid{}}});
    extras.setProperty("entity", PropertyValue{PropertyValue::EntityReference{Uuid{}}});
    entity->addComponent(std::move(extras));

    Harness harness{fixture.context};

    // Six kinds, none of them falling through to a summary. Counting the fall-throughs is what
    // makes this assertion mean something: a row count would be satisfied by six summaries.
    CNA_STUDIO_EXPECT_EQ(harness.shell->frame().phaseViolations(), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(harness.last.componentCount, std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(harness.last.readOnlyProperties, std::size_t{0});

    // And a list is no longer one of them (STUDIO-07054). It used to fall through to "1 item" and
    // be counted here; it now gets a row that expands into its elements, so the count stays at
    // zero. This assertion is kept rather than deleted because it is the one that would notice a
    // kind quietly *losing* its editor again.
    StudioComponent nested{"Test.Nested"};
    PropertyValue::ListValue list;
    list.items.push_back(PropertyValue{1});
    nested.setProperty("items", PropertyValue{std::move(list)});
    fixture.context.getScene().findEntityForEdit(fixture.entity)->addComponent(std::move(nested));

    Harness second{fixture.context};
    CNA_STUDIO_EXPECT_EQ(second.last.readOnlyProperties, std::size_t{0});
}

// ------------------------------------------------------------------------------------------------
// Adding and removing components (STUDIO-07040)
//
// The gap that stopped Dear ImGui being deleted. The prototype's Inspector has had an Add Component
// control since it existed and the native Details panel had none -- so an entity created in the
// native shell could never be given anything to do. The migration inventory did not catch it
// because it accounts for panels, menus, toolbars and shortcuts, and this is a button inside a
// panel. These tests are at the level the inventory did not reach.
//
// Both sweep for the control rather than computing a pixel, like every other test in this file: a
// test that computes a coordinate becomes, the first time a metric changes, a test that clicks
// nothing and passes.
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(AComponentCanBeAddedToTheSelectedEntityAndUndone)
{
    Fixture fixture;
    Harness harness{fixture.context};

    const auto componentCount = [&fixture] {
        const StudioEntity* found = fixture.context.getScene().findEntity(fixture.entity);
        return found != nullptr ? found->getComponents().size() : std::size_t{0};
    };

    const std::size_t before = componentCount();
    CNA_STUDIO_EXPECT_EQ(before, std::size_t{1});

    // The Add button is at the right-hand end of the last row. Swept from the bottom of the panel
    // upward, because the row it is on moves with the number of properties above it.
    bool added = false;
    for (float y = harness.bounds.bottom() - 8.0f;
         y > harness.bounds.top() && !added; y -= 5.0f)
    {
        harness.click(harness.bounds.right() - 24.0f, y);
        added = componentCount() > before;
    }

    CNA_STUDIO_EXPECT(added);
    CNA_STUDIO_EXPECT_EQ(componentCount(), before + 1);

    // Through the history like every other edit. A component added outside it is one Undo cannot
    // take back, and adding the wrong one is exactly the mistake a click makes.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().canUndo());
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(componentCount(), before);
}

/**
 * What the Add Component list offers (`plan.md` STUDIO-14002).
 *
 * The case above presses Add and checks that something appeared, which passes just as well for a
 * control hard-wired to one type. The *list* is the half that makes the feature a feature, and it
 * is a function of the registry and the entity rather than something the widget works out -- so it
 * can be asserted without a frame, a dropdown or a popup, which is the only way this half was ever
 * going to be gated honestly.
 *
 * A first attempt drove the dropdown through the shell and passed against an implementation that
 * ignored the choice entirely: the list *shortens* as unique components are added, so the entry at
 * index zero changes on its own and "a different component arrived" was true either way. Recorded
 * because that shape of false pass is easy to write again.
 */
/**
 * A changed property can be put back (`plan.md` STUDIO-14012).
 *
 * Reset existed only for importer settings, in the asset inspector. A component property that
 * differed from what its descriptor declares had no way back at all -- a user who scrubbed a scale
 * and wanted 1 again had to know the default and retype it, and for a colour or a quaternion they
 * would have had to guess.
 */
/**
 * A property value round-trips through text (`plan.md` STUDIO-14014).
 *
 * The same JSON a scene file holds, because that is already the one written-down definition of what
 * a `PropertyValue` looks like -- a second encoding invented for the clipboard would be a second
 * thing to keep in step with the first, and the first is the one that has to survive a release.
 */
/**
 * The Inspector over several entities (`plan.md` STUDIO-14017).
 *
 * It showed the *last* selected entity and edited only it, saying nothing about the rest -- so a
 * user who selected five crates and set their scale changed one, and found out later.
 */
/**
 * What is wrong with a component is said on the component (`plan.md` STUDIO-14015).
 *
 * The Details panel showed no validation at all: the only way to learn an entity was broken was to
 * open the Problems panel and find it in a list -- a panel away from the one where the fix is made.
 */
/**
 * A read-only component property is shown, not offered (`plan.md` STUDIO-14011).
 *
 * The asset inspector has honoured `PropertyDescriptor::readOnly` since it was written -- a value
 * an importer computes is drawn as text rather than as a control the user can put a caret in and
 * then find refuses them. The *component* grid did not look at the flag at all, so a plugin
 * declaring a computed field got a fully editable one.
 */
CNA_STUDIO_TEST(AReadOnlyComponentPropertyIsDrawnAsTextRatherThanAControl)
{
    StudioContext context;

    // A component whose second property the descriptor declares read-only. Registered here rather
    // than relying on a builtin, because no builtin declares one and a case that cannot be written
    // without inventing the situation is a case that proves the situation is handled.
    ComponentDescriptor descriptor;
    descriptor.typeId = "Test.Computed";
    descriptor.displayName = "Computed";
    {
        PropertyDescriptor editable;
        editable.name = "speed";
        editable.displayName = "Speed";
        editable.type = PropertyType::Float;
        editable.defaultValue = PropertyValue{1.0f};
        descriptor.properties.push_back(std::move(editable));

        PropertyDescriptor computed;
        computed.name = "derived";
        computed.displayName = "Derived";
        computed.type = PropertyType::Float;
        computed.defaultValue = PropertyValue{0.0f};
        computed.readOnly = true;
        descriptor.properties.push_back(std::move(computed));
    }
    CNA_STUDIO_EXPECT(context.getComponentRegistry().registerComponent(descriptor));

    StudioEntity subject{Uuid::generate(), "Widget"};
    StudioComponent component{"Test.Computed"};
    component.applyDefaults(*context.getComponentRegistry().find("Test.Computed"));
    subject.getComponents().push_back(std::move(component));
    const Uuid entity = subject.getId();
    context.getScene().addEntity(std::move(subject));
    context.select(entity);

    Harness harness{context};

    // One of the two properties is reported as a kind with no editor, which is the same flag the
    // asset inspector sets and the same one a type the panel cannot edit sets.
    CNA_STUDIO_EXPECT_EQ(harness.last.readOnlyProperties, std::size_t{1});

    const auto derived = [&context, entity] {
        return context.getScene().findEntity(entity)
            ->findComponent("Test.Computed")->getProperty("derived").get<float>();
    };
    const auto speed = [&context, entity] {
        return context.getScene().findEntity(entity)
            ->findComponent("Test.Computed")->getProperty("speed").get<float>();
    };

    CNA_STUDIO_EXPECT_EQ(derived(), 0.0f);
    CNA_STUDIO_EXPECT_EQ(speed(), 1.0f);

    // Dragged across every row of the panel, with the history as evidence the sweep is landing --
    // "nothing changed" would otherwise be true of a case that clicked outside the panel.
    //
    // The assertion that carries this case is `readOnlyProperties` above: making the panel ignore
    // the flag fails *that* one by name. `derived()` sitting still is corroboration rather than
    // the gate, because whether a given sweep reaches a given row depends on the layout, and a
    // corroboration honestly labelled is worth more than one that reads like proof.
    for (float y = harness.bounds.top() + 2.0f; y < harness.bounds.bottom() - 2.0f; y += 2.0f)
    {
        const float columnLeft = harness.bounds.left() + harness.bounds.width * 0.40f;
        for (const float fraction : {0.25f, 0.5f})
        {
            harness.drag(columnLeft + (harness.bounds.right() - columnLeft) * fraction, y, 40.0f);
        }
    }

    CNA_STUDIO_EXPECT(context.getHistory().canUndo());
    CNA_STUDIO_EXPECT_EQ(derived(), 0.0f);
}

CNA_STUDIO_TEST(AnIssueIsPickedApartByTheEntityAndTheComponentItNames)
{
    const Uuid mine = Uuid::generate();
    const Uuid theirs = Uuid::generate();

    std::vector<SceneIssue> raw;

    SceneIssue sceneWide;
    sceneWide.ruleId = "no-primary-camera";
    raw.push_back(sceneWide);

    SceneIssue onComponent;
    onComponent.ruleId = "sprite-without-texture";
    onComponent.entityId = mine;
    onComponent.componentTypeId = "CNA.SpriteRenderer";
    onComponent.severity = SceneIssue::Severity::Warning;
    raw.push_back(onComponent);

    SceneIssue onEntity;
    onEntity.ruleId = "empty-entity";
    onEntity.entityId = mine;
    raw.push_back(onEntity);

    SceneIssue elsewhere;
    elsewhere.ruleId = "zero-scale";
    elsewhere.entityId = theirs;
    elsewhere.componentTypeId = "CNA.Transform";
    raw.push_back(elsewhere);

    const StudioInspectorIssues picked = studioInspectorIssues(raw, mine);

    // This entity's two, and neither the scene-wide one nor the other entity's: hanging "two
    // primary cameras" on whichever camera happens to be selected would name a culprit the rule
    // does not have.
    CNA_STUDIO_EXPECT_EQ(picked.forEntity.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(picked.forComponent("CNA.SpriteRenderer").size(), std::size_t{1});
    CNA_STUDIO_EXPECT(picked.forComponent("CNA.Transform").empty());

    // An issue naming no component belongs to the entity rather than to any one section.
    CNA_STUDIO_EXPECT_EQ(picked.forEntityItself().size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(picked.forEntityItself().front()->ruleId, std::string{"empty-entity"});

    // And an entity nothing names has nothing to show.
    CNA_STUDIO_EXPECT(studioInspectorIssues(raw, Uuid::generate()).forEntity.empty());
    CNA_STUDIO_EXPECT(studioInspectorIssues(raw, Uuid{}).forEntity.empty());
}

/**
 * And the panel gives each one a row (`plan.md` STUDIO-14015).
 */
CNA_STUDIO_TEST(TheInspectorDrawsAComponentsIssuesAboveItsProperties)
{
    Fixture fixture;

    // Measured with nothing wrong first, so the difference is the issue rows and not the fixture.
    Harness clean{fixture.context};
    const std::size_t quiet = clean.last.rowsDrawn;
    const std::size_t quietMeasured = clean.last.rowsMeasured;

    SceneIssue issue;
    issue.ruleId = "zero-scale";
    issue.entityId = fixture.entity;
    issue.componentTypeId = "CNA.Transform";
    issue.severity = SceneIssue::Severity::Warning;
    issue.message = "Scale is zero on at least one axis.";

    SceneIssue second;
    second.ruleId = "made-up";
    second.entityId = fixture.entity;
    second.componentTypeId = "CNA.Transform";
    second.severity = SceneIssue::Severity::Error;
    second.message = "And another thing.";

    const StudioInspectorIssues picked =
        studioInspectorIssues({issue, second}, fixture.entity);

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult result =
                studioDetailsPanel(frame, area, fixture.context, {}, nullptr, picked);
            if (frame.isInputPass()) { last = result; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));

    // One row per issue, drawn *and* measured -- the scroll view is sized from the measure, and a
    // pre-pass that did not count them would leave the panel scrolling past its last control.
    CNA_STUDIO_EXPECT_EQ(last.rowsDrawn, quiet + 2);
    CNA_STUDIO_EXPECT_EQ(last.rowsMeasured, quietMeasured + 2);
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});

    // An issue naming a component the entity does not carry is nobody's row.
    SceneIssue foreign = issue;
    foreign.componentTypeId = "CNA.SpriteRenderer";
    const StudioInspectorIssues unrelated =
        studioInspectorIssues({foreign}, fixture.entity);

    StudioDetailsResult other;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult result =
                studioDetailsPanel(frame, area, fixture.context, {}, nullptr, unrelated);
            if (frame.isInputPass()) { other = result; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(other.rowsDrawn, quiet);
}

CNA_STUDIO_TEST(TheInspectorShowsOnlyTheComponentsEveryoneSelectedHas)
{
    StudioContext context;

    const auto add = [&context](const char* name, std::vector<const char*> types) {
        StudioEntity subject{Uuid::generate(), name};
        for (const char* type : types)
        {
            StudioComponent component{type};
            if (const ComponentDescriptor* descriptor = context.getComponentRegistry().find(type))
            {
                component.applyDefaults(*descriptor);
            }
            subject.getComponents().push_back(std::move(component));
        }
        const Uuid id = subject.getId();
        context.getScene().addEntity(std::move(subject));
        return id;
    };

    const Uuid both = add("Both", {"CNA.Transform", "CNA.SpriteRenderer"});
    const Uuid transformOnly = add("TransformOnly", {"CNA.Transform"});

    // Selected alone, the entity's own components -- which is what makes this the only path rather
    // than a second one for the multi case.
    const std::vector<std::string> alone = studioSharedComponents(context.getScene(), {both});
    CNA_STUDIO_EXPECT_EQ(alone.size(), std::size_t{2});

    // Together, the intersection. A component only one of them has is exactly the case the task's
    // title excludes: there is no unambiguous answer to what editing it should do.
    const std::vector<std::string> shared =
        studioSharedComponents(context.getScene(), {both, transformOnly});
    CNA_STUDIO_EXPECT_EQ(shared.size(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(shared.front(), std::string{"CNA.Transform"});

    // In the last selected entity's order, because that is the entity the panel is built around.
    const std::vector<std::string> reversed =
        studioSharedComponents(context.getScene(), {transformOnly, both});
    CNA_STUDIO_EXPECT_EQ(reversed.size(), std::size_t{1});

    // An empty selection has nothing to show, and an id the scene has lost is not a component
    // everybody has.
    CNA_STUDIO_EXPECT(studioSharedComponents(context.getScene(), {}).empty());
    CNA_STUDIO_EXPECT(
        studioSharedComponents(context.getScene(), {both, Uuid::generate()}).empty());
}

/**
 * And a value is only shown when they agree (`plan.md` STUDIO-14017).
 *
 * A field reading 3 over five entities of which four are 7 is a field that lies, and the user finds
 * out by overwriting the four.
 */
CNA_STUDIO_TEST(ASharedPropertyValueIsNothingWhenTheEntitiesDisagree)
{
    StudioContext context;
    const ComponentDescriptor* descriptor =
        context.getComponentRegistry().find("CNA.Transform");
    CNA_STUDIO_EXPECT(descriptor != nullptr);

    const auto add = [&context, descriptor](const StudioVector3& position) {
        StudioEntity subject{Uuid::generate(), "Crate"};
        StudioComponent transform{"CNA.Transform"};
        transform.applyDefaults(*descriptor);
        transform.setProperty("position", PropertyValue{position});
        subject.getComponents().push_back(std::move(transform));
        const Uuid id = subject.getId();
        context.getScene().addEntity(std::move(subject));
        return id;
    };

    const Uuid first = add(StudioVector3{1.0f, 0.0f, 0.0f});
    const Uuid same = add(StudioVector3{1.0f, 0.0f, 0.0f});
    const Uuid different = add(StudioVector3{9.0f, 0.0f, 0.0f});

    const auto sharedOf = [&](const std::vector<Uuid>& selection) {
        return studioSharedPropertyValue(context.getScene(), selection, "CNA.Transform",
                                         "position", descriptor);
    };

    CNA_STUDIO_EXPECT(sharedOf({first}).has_value());
    CNA_STUDIO_EXPECT(sharedOf({first, same}).has_value());
    CNA_STUDIO_EXPECT(sharedOf({first, same})->get<StudioVector3>().x == 1.0f);
    CNA_STUDIO_EXPECT(!sharedOf({first, different}).has_value());

    // An entity that never wrote the property and one that wrote the default agree, because they
    // do as far as the game is concerned -- reporting a difference nothing can see would be worse
    // than useless.
    StudioEntity bare{Uuid::generate(), "Bare"};
    bare.getComponents().push_back(StudioComponent{"CNA.Transform"});
    const Uuid unwritten = bare.getId();
    context.getScene().addEntity(std::move(bare));

    StudioEntity explicitDefault{Uuid::generate(), "Explicit"};
    StudioComponent written{"CNA.Transform"};
    written.applyDefaults(*descriptor);
    explicitDefault.getComponents().push_back(std::move(written));
    const Uuid wrote = explicitDefault.getId();
    context.getScene().addEntity(std::move(explicitDefault));

    CNA_STUDIO_EXPECT(sharedOf({unwritten, wrote}).has_value());

    // An entity without the component at all is not a disagreement, it is an absence -- and the
    // panel does not offer the section for it in the first place.
    StudioEntity nothing{Uuid::generate(), "Nothing"};
    const Uuid without = nothing.getId();
    context.getScene().addEntity(std::move(nothing));
    CNA_STUDIO_EXPECT(!sharedOf({first, without}).has_value());
}

/**
 * And an edit reaches every one of them, as one undo entry (`plan.md` STUDIO-14017).
 */
CNA_STUDIO_TEST(EditingAPropertyOverASelectionChangesAllOfThemInOneStep)
{
    StudioContext context;
    const ComponentDescriptor* descriptor = context.getComponentRegistry().find("CNA.Transform");
    CNA_STUDIO_EXPECT(descriptor != nullptr);

    std::vector<Uuid> crates;
    for (int i = 0; i < 3; ++i)
    {
        StudioEntity subject{Uuid::generate(), "Crate " + std::to_string(i)};
        StudioComponent transform{"CNA.Transform"};
        transform.applyDefaults(*descriptor);
        transform.setProperty("position", PropertyValue{StudioVector3{1.0f, 2.0f, 3.0f}});
        subject.getComponents().push_back(std::move(transform));
        crates.push_back(subject.getId());
        context.getScene().addEntity(std::move(subject));
    }
    context.setSelection(crates);

    Harness harness{context};
    CNA_STUDIO_EXPECT_EQ(harness.last.entitiesEdited, std::size_t{3});

    const auto positionOf = [&context](const Uuid& id) {
        return context.getScene().findEntity(id)
            ->findComponent("CNA.Transform")->getProperty("position").get<StudioVector3>();
    };

    // The same sweep the single-entity scrub case uses.
    bool scrubbed = false;
    for (float y = harness.bounds.top() + 20.0f;
         y < harness.bounds.top() + 200.0f && !scrubbed; y += 6.0f)
    {
        const float columnLeft = harness.bounds.left() + harness.bounds.width * 0.40f;
        const float x = columnLeft + (harness.bounds.right() - columnLeft) * 0.5f;
        harness.drag(x, y, 40.0f);
        scrubbed = positionOf(crates.front()).y != 2.0f;
    }
    CNA_STUDIO_EXPECT(scrubbed);

    // All three, not the one the panel is built around. They started equal, so they end equal.
    const StudioVector3 moved = positionOf(crates.front());
    CNA_STUDIO_EXPECT(moved.y > 2.0f);
    for (const Uuid& crate : crates)
    {
        CNA_STUDIO_EXPECT_EQ(positionOf(crate).y, moved.y);
    }

    // And one press of Ctrl+Z takes all three back: the user typed once. Three commands would be
    // three presses and -- worse -- would undo one at a time, leaving arrangements that never
    // existed.
    CNA_STUDIO_EXPECT(context.getHistory().undo());
    for (const Uuid& crate : crates)
    {
        CNA_STUDIO_EXPECT_EQ(positionOf(crate).y, 2.0f);
    }
}

CNA_STUDIO_TEST(APropertyValueSurvivesBeingCopiedAndPastedBack)
{
    const auto roundTrips = [](const PropertyValue& value, PropertyType type) {
        const std::optional<PropertyValue> back =
            studioPastedProperty(studioCopyPropertyText(value), type);
        return back.has_value() && *back == value;
    };

    CNA_STUDIO_EXPECT(roundTrips(PropertyValue{3.5f}, PropertyType::Float));
    CNA_STUDIO_EXPECT(roundTrips(PropertyValue{std::int64_t{42}}, PropertyType::Integer));
    CNA_STUDIO_EXPECT(roundTrips(PropertyValue{true}, PropertyType::Boolean));
    CNA_STUDIO_EXPECT(roundTrips(PropertyValue{std::string{"hello"}}, PropertyType::String));
    CNA_STUDIO_EXPECT(
        roundTrips(PropertyValue{StudioVector2{1.0f, 2.0f}}, PropertyType::Vector2));
    CNA_STUDIO_EXPECT(
        roundTrips(PropertyValue{StudioVector3{1.0f, 2.0f, 3.0f}}, PropertyType::Vector3));
    CNA_STUDIO_EXPECT(roundTrips(PropertyValue{StudioColor{10, 20, 30, 40}}, PropertyType::Color));
    CNA_STUDIO_EXPECT(roundTrips(PropertyValue{StudioQuaternion{0.0f, 0.7071f, 0.0f, 0.7071f}},
                                 PropertyType::Quaternion));
    CNA_STUDIO_EXPECT(
        roundTrips(PropertyValue{StudioRectangle{1, 2, 3, 4}}, PropertyType::Rectangle));

    // Plain text, and readable: a user who copies a position into a bug report or a script should
    // get something they can read, and one who pastes a readable thing back should be understood.
    const std::string text = studioCopyPropertyText(PropertyValue{StudioVector3{1.0f, 2.0f, 3.0f}});
    CNA_STUDIO_EXPECT(text.find('1') != std::string::npos);
    CNA_STUDIO_EXPECT(text.find('\n') == std::string::npos);
}

/**
 * And a paste of the wrong thing is refused (`plan.md` STUDIO-14014).
 *
 * Pasting a colour into a number is a mistake, and coercing it would put a value the user did not
 * ask for into a field they were not looking at. Refusing says so while they can still do something
 * about it.
 */
CNA_STUDIO_TEST(PastingSomethingThatIsNotThePropertysKindIsRefused)
{
    // Not JSON at all: the most ordinary case, because the clipboard usually holds prose.
    CNA_STUDIO_EXPECT(!studioPastedProperty("hello world", PropertyType::Float).has_value());
    CNA_STUDIO_EXPECT(!studioPastedProperty("", PropertyType::Float).has_value());
    CNA_STUDIO_EXPECT(!studioPastedProperty("{ unterminated", PropertyType::Vector3).has_value());

    // JSON, and a perfectly good value -- of the wrong kind. A vector's text read as a float is
    // the case that would silently produce a zero.
    const std::string vector =
        studioCopyPropertyText(PropertyValue{StudioVector3{1.0f, 2.0f, 3.0f}});
    CNA_STUDIO_EXPECT(!studioPastedProperty(vector, PropertyType::Float).has_value());
    CNA_STUDIO_EXPECT(!studioPastedProperty(vector, PropertyType::Boolean).has_value());
    CNA_STUDIO_EXPECT(studioPastedProperty(vector, PropertyType::Vector3).has_value());

    // A number where a vector belongs. `fromJson` is forgiving by design, so the round-trip check
    // is what catches this rather than the type comparison.
    CNA_STUDIO_EXPECT(!studioPastedProperty("7", PropertyType::Vector3).has_value());

    // And a property with no declared type has nothing to check against, so nothing is accepted.
    CNA_STUDIO_EXPECT(!studioPastedProperty("7", PropertyType::None).has_value());
}

/**
 * And the row's menu carries it (`plan.md` STUDIO-14014).
 *
 * On the *label*, not the row: the row is mostly editor, and a target covering it would take the
 * press before the fields inside it got one -- the router gives a press to the first widget
 * described under the pointer, which is the defect the World Outliner's rows had (STUDIO-13005).
 */
CNA_STUDIO_TEST(APropertyValueIsCopiedAndPastedThroughTheRowsMenu)
{
    Fixture fixture;
    Harness harness{fixture.context};

    const StudioVector3 original = fixture.position();
    CNA_STUDIO_EXPECT(std::fabs(original.x - 1.0f) < 0.001f);

    const auto rightClick = [&](float x, float y) {
        UiInputState down = at(x, y);
        down.setMouseDown(UiMouseButton::Right, true);
        harness.shell->renderFrame(at(x, y));
        harness.shell->renderFrame(down);
        harness.shell->renderFrame(at(x, y));
    };

    const auto dismiss = [&] {
        UiInputState escape = at(harness.bounds.centerX(), harness.bounds.bottom() - 4.0f);
        escape.setKeyDown(UiKey::Escape, true);
        harness.shell->renderFrame(escape);
        harness.shell->renderFrame(at(harness.bounds.centerX(), harness.bounds.bottom() - 4.0f));
    };

    // The label column of whichever row carries the position. Swept, because a computed coordinate
    // becomes a click on nothing the first time a metric moves. One menu per attempt: a sweep that
    // kept clicking into an open menu would be choosing rows rather than opening them.
    const float labelX = harness.bounds.left() + 16.0f;
    bool copied = false;
    float rowY = -1.0f;
    for (float y = harness.bounds.top() + 4.0f;
         y < harness.bounds.bottom() - 4.0f && !copied; y += 4.0f)
    {
        dismiss();
        rightClick(labelX, y);

        // The menu's corner is at the point that opened it, so its rows run downwards from there.
        for (float offset = 4.0f; offset < 120.0f && !copied; offset += 4.0f)
        {
            harness.click(labelX + 20.0f, y + offset);
            harness.shell->renderFrame(at(labelX + 20.0f, y + offset));
            if (harness.last.propertiesCopied > 0) { copied = true; rowY = y; }
        }
    }

    CNA_STUDIO_EXPECT(copied);
    CNA_STUDIO_EXPECT(rowY > 0.0f);

    // What went on the clipboard is the value, readable, and nothing was edited by copying it.
    CNA_STUDIO_EXPECT_EQ(harness.shell->frame().clipboardText(),
                         studioCopyPropertyText(PropertyValue{original}));
    CNA_STUDIO_EXPECT(!fixture.context.getHistory().canUndo());

    // Move it somewhere else, so a paste that did nothing would be visible.
    dismiss();
    fixture.context.execute(std::make_unique<SetPropertyCommand>(
        fixture.context.getScene(), fixture.entity, "CNA.Transform", "position",
        PropertyValue{StudioVector3{9.0f, 9.0f, 9.0f}}));
    harness.shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(std::fabs(fixture.position().x - 9.0f) < 0.001f);

    // And paste it back from the same menu, on the same row. The clipboard is re-seeded before
    // each attempt because the sweep passes over Copy on its way to Paste -- without this it would
    // copy the *new* value and then paste that, and the case would pass while proving nothing.
    const std::string wanted = studioCopyPropertyText(PropertyValue{original});
    bool pasted = false;
    for (float offset = 4.0f; offset < 120.0f && !pasted; offset += 4.0f)
    {
        dismiss();
        harness.shell->frame().setClipboardText(wanted);
        rightClick(labelX, rowY);
        harness.click(labelX + 20.0f, rowY + offset);
        harness.shell->renderFrame(at(labelX + 20.0f, rowY + offset));
        pasted = harness.last.propertiesPasted > 0;
    }

    CNA_STUDIO_EXPECT(pasted);
    CNA_STUDIO_EXPECT(std::fabs(fixture.position().x - original.x) < 0.001f);

    // Through the history, like every other edit: a paste is a change, and pasting the wrong thing
    // is exactly the mistake a menu makes easy.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT(std::fabs(fixture.position().x - 9.0f) < 0.001f);
}

CNA_STUDIO_TEST(AnOverriddenComponentPropertyCanBeResetToItsDefault)
{
    Fixture fixture;

    const auto position = [&fixture] { return fixture.position(); };
    const StudioVector3 before = position();
    CNA_STUDIO_EXPECT(std::fabs(before.x - 1.0f) < 0.001f);

    Harness harness{fixture.context};

    // The button appears only where it would do something, and on the label's side of the row --
    // never the control's, because narrowing the control would move the fields inside it the
    // moment a property became overridden. Swept, so a metric change moves the click with it.
    bool reset = false;
    for (float y = harness.bounds.top() + 4.0f;
         y < harness.bounds.bottom() - 4.0f && !reset; y += 4.0f)
    {
        const float x = harness.bounds.left() + harness.bounds.width * 0.36f;
        harness.click(x, y);
        reset = harness.last.propertiesReset > 0;
    }

    CNA_STUDIO_EXPECT(reset);

    // Back to the descriptor's own default, which for a Transform's position is the origin.
    const StudioVector3 after = position();
    CNA_STUDIO_EXPECT(std::fabs(after.x) < 0.001f);
    CNA_STUDIO_EXPECT(std::fabs(after.y) < 0.001f);
    CNA_STUDIO_EXPECT(std::fabs(after.z) < 0.001f);

    // Through the history like every other edit -- a reset is still a change, and a user who
    // clicks it by mistake needs the same way out as one who typed by mistake.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().canUndo());
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT(std::fabs(position().x - before.x) < 0.001f);
}

/**
 * And it is not offered where it would do nothing (`plan.md` STUDIO-14012).
 *
 * A column of Reset buttons dead on every untouched row is a column of noise -- and the button
 * being present is also the only indication the panel gives that a property has been changed from
 * what the component was born with, which it could not be if it were always there.
 */
CNA_STUDIO_TEST(APropertyThatMatchesItsDefaultOffersNoReset)
{
    StudioContext context;
    StudioEntity subject{Uuid::generate(), "Untouched"};

    // Every property left at what the descriptor declares, which is what `applyDefaults` gives.
    StudioComponent transform{"CNA.Transform"};
    transform.applyDefaults(*context.getComponentRegistry().find("CNA.Transform"));
    subject.getComponents().push_back(std::move(transform));

    const Uuid entity = subject.getId();
    context.getScene().addEntity(std::move(subject));
    context.select(entity);

    Harness harness{context};

    // The same sweep the case above uses, over a panel where nothing is overridden: no click
    // anywhere in the label column resets anything, and nothing is edited by trying.
    for (float y = harness.bounds.top() + 4.0f; y < harness.bounds.bottom() - 4.0f; y += 4.0f)
    {
        const float x = harness.bounds.left() + harness.bounds.width * 0.36f;
        harness.click(x, y);
    }

    CNA_STUDIO_EXPECT_EQ(harness.last.propertiesReset, std::size_t{0});
    CNA_STUDIO_EXPECT(!context.getHistory().canUndo());
}

CNA_STUDIO_TEST(TheAddComponentListLeavesOutWhatTheEntityAlreadyHas)
{
    Fixture fixture;
    const ComponentRegistry& registry = fixture.context.getComponentRegistry();
    const StudioEntity* entity = fixture.context.getScene().findEntity(fixture.entity);
    CNA_STUDIO_EXPECT(entity != nullptr);
    if (entity == nullptr) { return; }

    const std::vector<StudioComponentChoice> choices = studioAddComponentChoices(registry, *entity);
    CNA_STUDIO_EXPECT(!choices.empty());

    const auto offers = [&choices](std::string_view typeId) {
        for (const StudioComponentChoice& choice : choices)
        {
            if (choice.typeId == typeId) { return true; }
        }
        return false;
    };

    // The entity carries a Transform, which is unique, so the list must not offer a second one --
    // `AddComponentCommand` would refuse it, and a control that refuses is indistinguishable from
    // one that is broken.
    CNA_STUDIO_EXPECT(registry.find("CNA.Transform") != nullptr);
    CNA_STUDIO_EXPECT(registry.find("CNA.Transform")->unique);
    CNA_STUDIO_EXPECT(!offers("CNA.Transform"));

    // Every other registered type is offered, and each entry carries the id it will add rather
    // than a position in a list that changes length as the entity gains components.
    for (const StudioComponentChoice& choice : choices)
    {
        CNA_STUDIO_EXPECT(!choice.typeId.empty());
        CNA_STUDIO_EXPECT(!choice.label.empty());
        CNA_STUDIO_EXPECT(registry.find(choice.typeId) != nullptr);
    }

    // The label carries the category so a long registry reads as a menu rather than a wall of
    // names, and falls back to the display name when there is no category to show.
    for (const StudioComponentChoice& choice : choices)
    {
        const ComponentDescriptor* descriptor = registry.find(choice.typeId);
        if (descriptor == nullptr) { continue; }
        CNA_STUDIO_EXPECT_EQ(choice.label,
                             descriptor->category.empty()
                                 ? descriptor->displayName
                                 : descriptor->category + " / " + descriptor->displayName);
    }

    // An entity with nothing on it is offered the Transform the fixture's is not.
    StudioEntity bare{Uuid::generate(), "Bare"};
    CNA_STUDIO_EXPECT(studioAddComponentChoices(registry, bare).size() == choices.size() + 1);
}

CNA_STUDIO_TEST(AComponentIsRemovedFromItsOwnHeaderAndUndone)
{
    // A Transform alone cannot be removed -- its descriptor marks it required -- so the entity
    // gets a second component to take away. That the required one *refuses* is the other half, and
    // is asserted by the count not falling to zero.
    Fixture fixture;
    {
        StudioEntity* entity = fixture.context.getScene().findEntityForEdit(fixture.entity);
        CNA_STUDIO_EXPECT(entity != nullptr);
        entity->getComponents().push_back(StudioComponent{"CNA.SpriteRenderer"});
    }

    Harness harness{fixture.context};

    const auto componentCount = [&fixture] {
        const StudioEntity* found = fixture.context.getScene().findEntity(fixture.entity);
        return found != nullptr ? found->getComponents().size() : std::size_t{0};
    };

    const std::size_t before = componentCount();
    CNA_STUDIO_EXPECT_EQ(before, std::size_t{2});

    bool removed = false;
    for (float y = harness.bounds.top() + 8.0f;
         y < harness.bounds.bottom() && !removed; y += 5.0f)
    {
        harness.click(harness.bounds.right() - 12.0f, y);
        removed = componentCount() < before;
    }

    CNA_STUDIO_EXPECT(removed);
    CNA_STUDIO_EXPECT_EQ(componentCount(), before - 1);

    // And it is the *right* one. The sweep starts at the top, where the Transform's header is, so
    // a Remove that ignored `required` would have taken the Transform first -- and an entity
    // without one is not an entity. Asserting only the count would pass either way.
    {
        const StudioEntity* found = fixture.context.getScene().findEntity(fixture.entity);
        CNA_STUDIO_EXPECT(found->findComponent("CNA.Transform") != nullptr);
        CNA_STUDIO_EXPECT(found->findComponent("CNA.SpriteRenderer") == nullptr);
    }

    CNA_STUDIO_EXPECT(fixture.context.getHistory().canUndo());
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(componentCount(), before);
}

// ------------------------------------------------------------------------------------------------
// The asset inspector (STUDIO-07045)
//
// One of the five Inspector sections `STUDIO-07041` found with no native answer, and therefore one
// of the five things that stop Dear ImGui being deleted. The prototype shows a selected asset's
// identity and its importer's settings; the native Details panel showed the scene settings, because
// it could not see that an asset had been selected at all.
// ------------------------------------------------------------------------------------------------

namespace
{
    /** @brief A context with one texture asset whose importer declares settings. */
    struct AssetFixture
    {
        StudioContext context;
        Uuid textureId;

        AssetFixture()
        {
            AssetRecord record;
            record.id = Uuid::generate();
            record.sourcePath = "Assets/Textures/hero.png";
            record.type = AssetType::Texture2D;
            record.importerId = AssetDatabase::defaultImporterFor(record.type);
            textureId = record.id;
            (void)context.getAssets().add(std::move(record));
        }

        /** @brief Runs one full frame of the asset inspector over @p area. */
        StudioDetailsResult draw(StudioShell& shell, UiInputState input)
        {
            StudioDetailsResult last;
            (void)shell.setPanelContent("details", [&](StudioFrame& frame, const UiRect& area) {
                const StudioDetailsResult pass = studioDetailsPanel(frame, area, context);
                if (frame.isDrawPass()) { last = pass; }
            });
            shell.renderFrame(input);
            return last;
        }
    };
}

CNA_STUDIO_TEST(SelectingAnAssetShowsItRatherThanTheSceneSettings)
{
    // The defect this task closes, stated directly. The native Content Browser wrote the selected
    // asset into a member of StudioShellPanels while StudioContext had a selectedAsset_ of its own
    // that only the prototype ever wrote -- so the two native panels had different ideas of what
    // was selected, and the Details panel's was always "nothing".
    AssetFixture fixture;
    StudioShell shell{StudioTheme::dark()};
    shell.resetLayout();
    CNA_STUDIO_EXPECT(shell.activatePanel("details"));

    // Nothing selected and no project: the panel says so and draws no rows at all.
    const StudioDetailsResult idle = fixture.draw(shell, at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(idle.rowsDrawn, std::size_t{0});

    fixture.context.selectAsset(fixture.textureId);
    const StudioDetailsResult asset = fixture.draw(shell, at(-1.0f, -1.0f));

    // Identity, a gap, the importer's heading, and one row per declared setting. An asset
    // inspector that resolved to the project message would draw none of them.
    CNA_STUDIO_EXPECT(asset.rowsDrawn > 4);
    CNA_STUDIO_EXPECT_EQ(asset.componentCount, std::size_t{0});
}

CNA_STUDIO_TEST(AnAssetAndAnEntityAreNeverBothSelected)
{
    // The inspector shows one thing at a time, and which one is decided by what was clicked last.
    // Two independent selections would leave the user unable to tell which the panel is about --
    // and the answer would change as they clicked around without either selection being cleared.
    AssetFixture fixture;
    const Uuid entity = fixture.context.getScene().addEntity(
        StudioEntity{Uuid::generate(), "Player"});

    fixture.context.select(entity);
    CNA_STUDIO_EXPECT(!fixture.context.getSelection().empty());

    fixture.context.selectAsset(fixture.textureId);
    CNA_STUDIO_EXPECT(fixture.context.getSelection().empty());
    CNA_STUDIO_EXPECT(fixture.context.getSelectedAsset() == fixture.textureId);
}

CNA_STUDIO_TEST(AnAssetDeletedWhileSelectedSaysSoRatherThanFallingBack)
{
    // Falling through to the scene settings would look exactly like the click never registered,
    // which is the failure mode that costs somebody ten minutes of clicking the same row.
    AssetFixture fixture;
    StudioShell shell{StudioTheme::dark()};
    shell.resetLayout();
    CNA_STUDIO_EXPECT(shell.activatePanel("details"));

    fixture.context.selectAsset(fixture.textureId);
    const StudioDetailsResult shown = fixture.draw(shell, at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shown.rowsDrawn > 0);

    CNA_STUDIO_EXPECT(fixture.context.getAssets().removeRecord(fixture.textureId));
    const StudioDetailsResult gone = fixture.draw(shell, at(-1.0f, -1.0f));

    // No rows: the message is not a row, and the panel is still about the asset rather than about
    // the scene. The selection is deliberately left alone -- clearing it here would make the
    // message flash for one frame and then vanish into the scene settings.
    CNA_STUDIO_EXPECT_EQ(gone.rowsDrawn, std::size_t{0});
    CNA_STUDIO_EXPECT(fixture.context.getSelectedAsset() == fixture.textureId);
}

CNA_STUDIO_TEST(AnImporterSettingIsEditedThroughTheHistoryLikeEveryOtherProperty)
{
    // An importer setting is persisted to a `.cnaasset` sidecar, which makes it the one edit in
    // Studio that could plausibly have been written straight to disk. It goes through the command
    // history like every other edit, so Ctrl+Z reaches it.
    AssetFixture fixture;

    const AssetRecord* before = fixture.context.getAssets().find(fixture.textureId);
    CNA_STUDIO_EXPECT(before != nullptr);

    auto command = std::make_unique<SetImporterSettingCommand>(
        fixture.context.getAssets(), fixture.textureId, "generateMipmaps", PropertyValue{true});
    CNA_STUDIO_EXPECT(command->isValid());
    fixture.context.execute(std::move(command));

    const AssetRecord* after = fixture.context.getAssets().find(fixture.textureId);
    CNA_STUDIO_EXPECT(after != nullptr);
    CNA_STUDIO_EXPECT(!after->importerSettings["generateMipmaps"].isNull());

    CNA_STUDIO_EXPECT(fixture.context.getHistory().canUndo());
    fixture.context.getHistory().undo();
    CNA_STUDIO_EXPECT(fixture.context.getAssets().find(fixture.textureId)
                          ->importerSettings["generateMipmaps"].isNull());
}

CNA_STUDIO_TEST(TheAssetInspectorsHeadingIsActuallyVisibleAndNotPaintedOver)
{
    // STUDIO-35063's lesson, applied to a new panel rather than rediscovered on it. Anything
    // described before the surface it sits on is drawn before that surface and painted over by it.
    // It happened twice in one session -- the disclosure triangle, then the outliner's visibility
    // toggle -- and both worked perfectly while being invisible, which is worse than missing
    // because nothing reports it.
    //
    // Asserted by *rasterising*, not by reading the draw order. An ordering heuristic over emitted
    // quads is a test of the heuristic: the first version of this case classified quads by texture
    // coordinate and alpha, passed, and went on passing when the defect was deliberately put back.
    // Rasterising asks the only question that matters -- are those pixels there -- and cannot be
    // satisfied by geometry that is submitted and then covered.
    AssetFixture fixture;
    StudioShell shell{StudioTheme::dark()};
    shell.resetLayout();
    CNA_STUDIO_EXPECT(shell.activatePanel("details"));
    fixture.context.selectAsset(fixture.textureId);

    UiRect panel;
    (void)shell.setPanelContent("details", [&](StudioFrame& frame, const UiRect& area) {
        (void)studioDetailsPanel(frame, area, fixture.context);
        if (frame.isDrawPass()) { panel = area; }
    });

    // Two frames and a texture table kept across them: the atlas is requested on the frame it is
    // rasterised and never again, so a table built from the last frame alone would have no font and
    // every glyph would draw as a solid rectangle -- which would pass this test for the wrong
    // reason.
    UiTextureTable textures;
    shell.renderFrame(at(-1.0f, -1.0f));
    textures.apply(shell.drawData());
    shell.renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT(!panel.isEmpty());

    const ImageBuffer image = rasterizeUiDrawData(
        shell.drawData(), shell.theme().color(StudioColorRole::AppBackground), textures);
    CNA_STUDIO_EXPECT(!image.isEmpty());
    if (image.isEmpty()) { return; }

    // The band the identity rows occupy: the top of the panel's content, two rows deep. The file
    // name, the Path label and the Path value are all in here.
    const int left = std::max(0, static_cast<int>(panel.x));
    const int right = std::min(image.width, static_cast<int>(panel.right()));
    const int top = std::max(0, static_cast<int>(panel.y));
    const int bottom = std::min(image.height, top + 64);
    CNA_STUDIO_EXPECT(right > left && bottom > top);

    // Distinct colours in the band. A band holding text has many -- glyphs are antialiased, so a
    // single letter contributes a dozen. A band that is nothing but the panel's own fill has one,
    // which is exactly what "described, then painted over" looks like.
    std::set<std::uint32_t> colours;
    for (int y = top; y < bottom; ++y)
    {
        for (int x = left; x < right; ++x)
        {
            const std::size_t at = (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width)
                                    + static_cast<std::size_t>(x)) * 4u;
            colours.insert(static_cast<std::uint32_t>(image.pixels[at]) << 16
                           | static_cast<std::uint32_t>(image.pixels[at + 1]) << 8
                           | static_cast<std::uint32_t>(image.pixels[at + 2]));
        }
    }

    if (colours.size() < 8)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
            "the asset inspector's identity rows rasterise to " + std::to_string(colours.size())
            + " distinct colours, which is a flat fill rather than text. The rows are being drawn "
              "and then painted over by the surface they sit on -- describe what has to win the "
              "click before the surface, and draw it after (plan.md STUDIO-35063).");
    }
    CNA_STUDIO_EXPECT(colours.size() >= 8);
}

CNA_STUDIO_TEST(DraggingANumericFieldScrubsTheValueWithoutTypingIntoIt)
{
    // `STUDIO-07055`. The prototype's vector fields scrub; the native field committed on Enter and
    // nothing else, so setting a position meant selecting the text and typing four characters --
    // for a value a user usually wants to *feel* their way to rather than know in advance.
    Fixture fixture;
    Harness harness{fixture.context};

    const StudioVector3 before = fixture.position();
    CNA_STUDIO_EXPECT_EQ(before.y, 2.0f);

    // The same sweep the typing case uses, for the same reason: found by walking the rows rather
    // than by computing a pixel, so a metric change does not turn this into a test that drags
    // empty space and passes.
    bool scrubbed = false;
    float scrubX = 0.0f;
    float scrubY = 0.0f;
    for (float y = harness.bounds.top() + 20.0f;
         y < harness.bounds.top() + 200.0f && !scrubbed; y += 6.0f)
    {
        const float columnLeft = harness.bounds.left() + harness.bounds.width * 0.40f;
        const float x = columnLeft + (harness.bounds.right() - columnLeft) * 0.5f;

        harness.drag(x, y, 40.0f);
        scrubbed = fixture.position().y != before.y;
        if (scrubbed) { scrubX = x; scrubY = y; }
    }

    CNA_STUDIO_EXPECT(scrubbed);

    // Rightwards, so upwards. The exact number depends on which row the sweep landed on and on the
    // step that row's kind uses; what this asserts is the direction and that the other two axes
    // were not touched -- the classic property-grid defect, asked of a drag rather than of typing.
    CNA_STUDIO_EXPECT(fixture.position().y > before.y);
    CNA_STUDIO_EXPECT_EQ(fixture.position().x, before.x);
    CNA_STUDIO_EXPECT_EQ(fixture.position().z, before.z);

    // And it went through the history like every other edit, so it can be taken back.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().canUndo());
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(fixture.position().y, before.y);

    // One drag is *one* entry (`STUDIO-07055`). Measured over a single further drag on the row the
    // sweep found, rather than over the sweep itself -- the sweep scrubs whatever rows it passes
    // on the way and its entry count says nothing about the merge.
    //
    // This was never asserted at all. The case pinned "something is undoable" and one `undo()`,
    // which passes just as well for forty entries, so a change that stopped the merge working
    // would have gone unnoticed -- and one nearly did: wrapping the edit in a `CompositeCommand`
    // for STUDIO-14017 removed the merge key the fold depends on, and nothing here complained.
    const std::size_t settled = fixture.context.getHistory().getCursor();
    const StudioVector3 beforeSecond = fixture.position();

    // Over eight frames, because the one-frame drag above produces a single command and therefore
    // cannot say anything about the fold: an implementation with no merge key at all passes it.
    harness.dragOverFrames(scrubX, scrubY, 40.0f, 8);

    CNA_STUDIO_EXPECT(fixture.position().y != beforeSecond.y);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCursor(), settled + 1);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(fixture.position().y, beforeSecond.y);
}

// ------------------------------------------------------------------------------------------------
// Lists and structures (STUDIO-07054)
//
// `studioPropertyEditor` draws *a control in a rect*, which is the right shape for every scalar
// kind and the wrong shape for these two: a list of four frames needs four rows, and a rect cannot
// grow. What that cost was concrete -- a sprite animation's frame list and a model renderer's
// per-part material overrides could not be edited in the native shell at all.
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(AddingToAListAppendsAnElementOfTheKindItAlreadyHolds)
{
    // A copy of the last element rather than a default-constructed value. A list holds one kind,
    // and an element that arrived as `monostate` would be a row with no editor in a list of rows
    // that have one -- which reads as the list having been broken by pressing Add.
    PropertyValue::ListValue list;
    list.items.push_back(PropertyValue{7});
    list.items.push_back(PropertyValue{9});

    CNA_STUDIO_EXPECT(studioApplyListEdit(list, StudioListEdit::Add, 0, list.items.back()));
    CNA_STUDIO_EXPECT_EQ(list.items.size(), std::size_t{3});
    CNA_STUDIO_EXPECT(list.items.back().getType() == PropertyType::Integer);
    CNA_STUDIO_EXPECT_EQ(list.items.back().get<std::int64_t>(), std::int64_t{9});
}

CNA_STUDIO_TEST(RemovingTakesTheNamedElementAndNotTheLastOne)
{
    // The off-by-one that is invisible in a screenshot: removing the *wrong* element still leaves a
    // list one shorter, and on a list of identical values it cannot be told from the right one.
    PropertyValue::ListValue list;
    for (std::int64_t i = 0; i < 4; ++i) { list.items.push_back(PropertyValue{i}); }

    CNA_STUDIO_EXPECT(studioApplyListEdit(list, StudioListEdit::Remove, 1, PropertyValue{}));
    CNA_STUDIO_EXPECT_EQ(list.items.size(), std::size_t{3});
    CNA_STUDIO_EXPECT_EQ(list.items[0].get<std::int64_t>(), std::int64_t{0});
    CNA_STUDIO_EXPECT_EQ(list.items[1].get<std::int64_t>(), std::int64_t{2});
    CNA_STUDIO_EXPECT_EQ(list.items[2].get<std::int64_t>(), std::int64_t{3});

    // Past the end, and on an empty list: refused rather than clamped. The caller pushes an undo
    // entry on true, and an entry that changes nothing is one a user presses Ctrl+Z on and watches
    // do nothing.
    CNA_STUDIO_EXPECT(!studioApplyListEdit(list, StudioListEdit::Remove, 3, PropertyValue{}));
    PropertyValue::ListValue empty;
    CNA_STUDIO_EXPECT(!studioApplyListEdit(empty, StudioListEdit::Remove, 0, PropertyValue{}));
}

CNA_STUDIO_TEST(MovingStopsAtEitherEndRatherThanWrapping)
{
    // Wrapping is never what somebody pressing Up at the top meant, and it is silent when it
    // happens -- the element they were looking at is suddenly at the other end of the list.
    PropertyValue::ListValue list;
    for (std::int64_t i = 0; i < 3; ++i) { list.items.push_back(PropertyValue{i}); }

    CNA_STUDIO_EXPECT(!studioApplyListEdit(list, StudioListEdit::MoveUp, 0, PropertyValue{}));
    CNA_STUDIO_EXPECT_EQ(list.items[0].get<std::int64_t>(), std::int64_t{0});

    CNA_STUDIO_EXPECT(!studioApplyListEdit(list, StudioListEdit::MoveDown, 2, PropertyValue{}));
    CNA_STUDIO_EXPECT_EQ(list.items[2].get<std::int64_t>(), std::int64_t{2});

    // And in the middle it swaps with its neighbour, both ways, returning to where it started.
    CNA_STUDIO_EXPECT(studioApplyListEdit(list, StudioListEdit::MoveUp, 1, PropertyValue{}));
    CNA_STUDIO_EXPECT_EQ(list.items[0].get<std::int64_t>(), std::int64_t{1});
    CNA_STUDIO_EXPECT_EQ(list.items[1].get<std::int64_t>(), std::int64_t{0});

    CNA_STUDIO_EXPECT(studioApplyListEdit(list, StudioListEdit::MoveDown, 0, PropertyValue{}));
    CNA_STUDIO_EXPECT_EQ(list.items[0].get<std::int64_t>(), std::int64_t{0});
    CNA_STUDIO_EXPECT_EQ(list.items[1].get<std::int64_t>(), std::int64_t{1});
}

CNA_STUDIO_TEST(AListPropertyGetsARowThatExpandsIntoItsElements)
{
    // The panel-level half. A list used to be a summary -- "4 items" -- and counting it as a
    // read-only kind was the honest way to say so. It now claims a row per element, which is the
    // only shape that can hold an editor for each of them.
    Fixture fixture;
    StudioComponent nested{"Test.Nested"};
    PropertyValue::ListValue list;
    list.items.push_back(PropertyValue{1});
    list.items.push_back(PropertyValue{2});
    nested.setProperty("items", PropertyValue{std::move(list)});
    fixture.context.getScene().findEntityForEdit(fixture.entity)->addComponent(std::move(nested));

    Harness harness{fixture.context};

    // Not counted as a kind without an editor any more, which is the assertion that would notice
    // the editor being lost again.
    CNA_STUDIO_EXPECT_EQ(harness.last.readOnlyProperties, std::size_t{0});

    const std::size_t collapsed = harness.last.rowsDrawn;

    // The disclosure is the first control in the row, at the left of the control column.
    bool expandedIt = false;
    for (float y = harness.bounds.top() + 20.0f;
         y < harness.bounds.top() + 260.0f && !expandedIt; y += 6.0f)
    {
        const float columnLeft = harness.bounds.left() + harness.bounds.width * 0.38f;
        harness.click(columnLeft + 16.0f, y);
        expandedIt = harness.last.rowsDrawn > collapsed;
    }

    CNA_STUDIO_EXPECT(expandedIt);
    // Two elements, so at least two rows more than the collapsed shape.
    CNA_STUDIO_EXPECT(harness.last.rowsDrawn >= collapsed + 2);
    CNA_STUDIO_EXPECT_EQ(harness.shell->frame().phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(AStructurePropertyGetsARowPerField)
{
    // A structure's fields are name/value pairs, each of which is an ordinary property -- so each
    // gets the editor its own kind already has, rather than a second set written for structures.
    Fixture fixture;
    StudioComponent nested{"Test.Struct"};
    PropertyValue::StructureValue structure;
    structure.set("width", PropertyValue{4});
    structure.set("label", PropertyValue{std::string{"left"}});
    nested.setProperty("layout", PropertyValue{std::move(structure)});
    fixture.context.getScene().findEntityForEdit(fixture.entity)->addComponent(std::move(nested));

    Harness harness{fixture.context};
    CNA_STUDIO_EXPECT_EQ(harness.last.readOnlyProperties, std::size_t{0});

    const std::size_t collapsed = harness.last.rowsDrawn;

    bool expandedIt = false;
    for (float y = harness.bounds.top() + 20.0f;
         y < harness.bounds.top() + 260.0f && !expandedIt; y += 6.0f)
    {
        const float columnLeft = harness.bounds.left() + harness.bounds.width * 0.38f;
        harness.click(columnLeft + 16.0f, y);
        expandedIt = harness.last.rowsDrawn > collapsed;
    }

    CNA_STUDIO_EXPECT(expandedIt);
    CNA_STUDIO_EXPECT(harness.last.rowsDrawn >= collapsed + 2);
    CNA_STUDIO_EXPECT_EQ(harness.shell->frame().phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(RemovingAListElementIsOneUndoEntryThatPutsItBack)
{
    // Each change is one undo entry, which is the last clause of the acceptance and the one a
    // user meets. A list edit rewrites the *whole* property -- the list is the value -- so the
    // entry has to carry the list as it was rather than the element that went.
    //
    // A dedicated entity with nothing but the list on it, rather than the shared `Fixture`: the
    // shared one also carries a Transform, and Transform's own three rows plus Name and Enabled
    // put enough buttons above this property that a pixel search for one of *this* row's three
    // (up, down, remove) risks landing on one of theirs on the way down. Fewer rows above means
    // fewer wrong buttons to walk past.
    StudioContext context;
    StudioEntity subject{Uuid::generate(), "Widget"};
    PropertyValue::ListValue list;
    list.items.push_back(PropertyValue{1});
    list.items.push_back(PropertyValue{2});
    StudioComponent nested{"Test.Nested"};
    nested.setProperty("items", PropertyValue{std::move(list)});
    subject.getComponents().push_back(std::move(nested));
    const Uuid entity = subject.getId();
    context.getScene().addEntity(std::move(subject));
    context.select(entity);

    const auto component = [&]() -> const StudioComponent* {
        const StudioEntity* found = context.getScene().findEntity(entity);
        return found == nullptr ? nullptr : found->findComponent("Test.Nested");
    };
    const auto itemCount = [&]() -> std::size_t {
        const StudioComponent* found = component();
        return found == nullptr
            ? std::size_t{0}
            : found->getProperty("items").get<PropertyValue::ListValue>().items.size();
    };
    CNA_STUDIO_EXPECT_EQ(itemCount(), std::size_t{2});

    Harness harness{context};
    const std::size_t collapsed = harness.last.rowsDrawn;

    // Expand, found the same way `AListPropertyGetsARowThatExpandsIntoItsElements` finds it.
    for (float y = harness.bounds.top() + 8.0f;
         y < harness.bounds.top() + 200.0f && harness.last.rowsDrawn == collapsed; y += 4.0f)
    {
        harness.click(harness.bounds.left() + harness.bounds.width * 0.38f + 16.0f, y);
    }
    CNA_STUDIO_EXPECT_EQ(harness.last.rowsDrawn, collapsed + 2);

    // Captured only now: the sweep above may have clicked other rows on its way to the
    // disclosure toggle -- an "Enabled" checkbox above it, say -- and each of those is a real
    // push this test has no interest in. The one edit under test is the one after expansion.
    const std::size_t before = context.getHistory().getCount();


    // The two new rows are the last two the panel drew: with nothing else on this entity, the
    // list's elements are the only rows an expand can add, and they are contiguous with the header
    // and summary above them. `RowHeight` plus the spacing `nextRow` adds after each row is what
    // one step down the panel costs.
    const float rowStep =
        metricOf(harness.shell->theme(), StudioMetric::RowHeight)
        + metricOf(harness.shell->theme(), StudioMetric::SpacingXSmall);
    const float removeX = harness.bounds.right()
        - metricOf(harness.shell->theme(), StudioMetric::ControlHeight) * 0.5f;
    const float element0Y =
        harness.bounds.top() + (static_cast<float>(collapsed) + 0.5f) * rowStep;

    harness.click(removeX, element0Y);

    CNA_STUDIO_EXPECT_EQ(itemCount(), std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(context.getHistory().getCount(), before + 1);

    CNA_STUDIO_EXPECT(context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(itemCount(), std::size_t{2});
}

// ------------------------------------------------------------------------------------------------
// Angles surviving gimbal lock (STUDIO-07057)
//
// A rotation is stored as a quaternion and edited as Euler angles, and the conversion is not
// injective: at gimbal lock, several triples of angles produce the same rotation, so recomputing
// fresh from the quaternion every frame can show a *different* triple from the one just typed --
// the field a user is looking at changes under them while they are still looking at it. The
// prototype keeps what was typed for as long as the stored value is still exactly the one it
// produced; the native editor converted afresh every frame.
// ------------------------------------------------------------------------------------------------

namespace
{
    /** @brief Drives `studioPropertyEditor` directly, over a value held outside any document. */
    struct AngleFixture
    {
        StudioFrame frame;
        UiRect bounds{100.0f, 100.0f, 240.0f, 24.0f};
        PropertyValue value{StudioQuaternion{}};

        AngleFixture() { frame.setTheme(StudioTheme::dark()); }

        /** @brief One full frame with no input, so retained state settles between gestures. */
        void settle()
        {
            frame.beginFrame(at(-1.0f, -1.0f));
            frame.beginInput();
            (void)studioPropertyEditor(frame, bounds, value, {}, {});
            frame.beginDraw();
            (void)studioPropertyEditor(frame, bounds, value, {}, {});
            frame.endFrame();
        }

        /**
         * @brief Types @p degrees into the pitch, yaw and roll fields in turn and commits each.
         *
         * The three fields divide `bounds` into equal thirds with `SpacingSmall` between them,
         * matching `numericComponents`' own layout exactly -- computed rather than guessed, so a
         * metric change moves this test's clicks along with the fields it is clicking.
         */
        void typeDegrees(const StudioVector3& degrees)
        {
            const float spacing = static_cast<float>(frame.theme().metric(StudioMetric::SpacingSmall));
            const float fieldWidth = (bounds.width - spacing * 2.0f) / 3.0f;
            const float values[3] = {degrees.x, degrees.y, degrees.z};

            for (int i = 0; i < 3; ++i)
            {
                const float x = bounds.left() + (static_cast<float>(i) + 0.5f) * fieldWidth
                              + static_cast<float>(i) * spacing;
                const float y = bounds.centerY();

                click(x, y);
                selectAll(x, y);
                const std::string text = studioFormatFloat(values[i]);
                std::vector<char16_t> characters(text.begin(), text.end());
                typeText(characters, x, y);
                pressEnter(x, y);
            }
        }

        /**
         * @brief The text actually displayed in the field named @p axis, after a `settle()`.
         *
         * Peeks the field's own retained buffer rather than recomputing anything, because
         * recomputing is exactly the defect under test -- a helper that re-derived the angle from
         * the quaternion would report the honest reading every time, whether or not the cache was
         * consulted, and the two are indistinguishable except at gimbal lock.
         *
         * @param axis One of "pitch", "yaw", "roll", matching `numericComponents`' own names.
         */
        [[nodiscard]] std::string shownText(const char* axis)
        {
            settle();
            return frame.state().get(frame.ids().make(axis)).text;
        }

    private:
        void oneFrame(const UiInputState& input)
        {
            frame.beginFrame(input);
            frame.beginInput();
            const StudioPropertyEditResult result = studioPropertyEditor(frame, bounds, value, {}, {});
            if (result.edited.has_value()) { value = *result.edited; }
            frame.beginDraw();
            (void)studioPropertyEditor(frame, bounds, value, {}, {});
            frame.endFrame();
        }

        void click(float x, float y)
        {
            oneFrame(at(x, y, false));
            oneFrame(at(x, y, true));
            oneFrame(at(x, y, false));
        }

        void selectAll(float x, float y)
        {
            UiInputState input = at(x, y);
            input.modifiers.control = true;
            input.setKeyDown(UiKey::A, true);
            oneFrame(input);
            oneFrame(at(x, y));
        }

        void typeText(const std::vector<char16_t>& characters, float x, float y)
        {
            UiInputState input = at(x, y);
            input.characters = characters;
            oneFrame(input);
        }

        void pressEnter(float x, float y)
        {
            UiInputState input = at(x, y);
            input.setKeyDown(UiKey::Enter, true);
            oneFrame(input);
            oneFrame(at(x, y));
        }
    };
}

CNA_STUDIO_TEST(TheAnglesTheUserTypedSurviveGimbalLock)
{
    // A pitch of 90 degrees is a pole: yaw and roll are no longer separable, and reading the
    // quaternion back reports the same rotation as a *different* (yaw, roll) pair.
    AngleFixture fixture;
    const StudioVector3 typed{90.0f, 40.0f, 25.0f};

    fixture.typeDegrees(typed);

    // Recomputing from the quaternion would show a folded yaw and roll 0, so the two fields
    // beside the one being edited would jump the instant pitch reached 90. Read from each
    // field's own displayed text, not re-derived from the quaternion: a helper that recomputed
    // would report the honest reading either way, and the two are indistinguishable except at
    // gimbal lock.
    CNA_STUDIO_EXPECT_EQ(fixture.shownText("yaw"), studioFormatFloat(40.0f));
    CNA_STUDIO_EXPECT_EQ(fixture.shownText("roll"), studioFormatFloat(25.0f));

    // And the honest reading really does differ, which is what makes the cache worth having --
    // a test that never entered gimbal lock would pass whether the cache existed or not.
    CNA_STUDIO_EXPECT(eulerDegreesOf(quaternionFromEulerDegrees(typed)).z != 25.0f);
}

CNA_STUDIO_TEST(EachTypedAngleFieldCommitsThroughTheHistoryAsItsOwnEntry)
{
    // The panel-level half: through a real entity, a real `SetPropertyCommand` and a real
    // undo stack, rather than `studioPropertyEditor` called directly the way the gimbal-lock
    // case above is. What the cache itself does when something else changes the rotation is
    // covered more precisely by `TheAngleCacheIsAbandonedTheInstantSomethingElseProducesA-
    // DifferentQuaternion`, which drives that case without a row to find.
    // Through the full panel and a real entity, because the acceptance is about the *document*
    // changing the rotation out from under a cached edit -- undo, specifically -- and that is not
    // something a bare `studioPropertyEditor` call can exercise.
    StudioContext context;
    StudioEntity subject{Uuid::generate(), "Widget"};
    StudioComponent transform{"CNA.Transform"};
    transform.setProperty("rotation", PropertyValue{StudioQuaternion{}});
    subject.getComponents().push_back(std::move(transform));
    const Uuid entity = subject.getId();
    context.getScene().addEntity(std::move(subject));
    context.select(entity);

    const auto rotation = [&]() -> StudioQuaternion {
        const StudioEntity* found = context.getScene().findEntity(entity);
        const StudioComponent* component = found->findComponent("CNA.Transform");
        return component->getProperty("rotation").get<StudioQuaternion>();
    };

    Harness harness{context};
    const float spacing = metricOf(harness.shell->theme(), StudioMetric::SpacingSmall);
    const float columnLeft = harness.bounds.left() + harness.bounds.width * 0.40f;
    const float fieldWidth = (harness.bounds.right() - columnLeft - spacing * 2.0f) / 3.0f;

    const auto typeInto = [&](float y, int index, float degrees) {
        const float x = columnLeft + (static_cast<float>(index) + 0.5f) * fieldWidth
                      + static_cast<float>(index) * spacing;
        harness.click(x, y);
        UiInputState select = at(x, y);
        select.modifiers.control = true;
        select.setKeyDown(UiKey::A, true);
        harness.shell->renderFrame(select);
        harness.shell->renderFrame(at(x, y));
        const std::string text = studioFormatFloat(degrees);
        harness.type(std::vector<char16_t>(text.begin(), text.end()), x, y);
        harness.press(UiKey::Enter, x, y);
    };

    // The rotation row's y is found rather than computed: a row count says how many rows there
    // are, not which one this is, and a fixed offset guessed from that count is exactly what broke
    // this test the first two times it was written. Swept the full height of the panel, pressing
    // 90 into the first field at each candidate and keeping the one that actually turned the
    // rotation -- any candidate that pressed something else is undone before the next is tried,
    // the same discipline `RemovingAListElementIsOneUndoEntryThatPutsItBack` uses for the same
    // reason.
    float rotationRowY = -1.0f;
    for (float y = harness.bounds.top() + 8.0f; y < harness.bounds.bottom(); y += 4.0f)
    {
        const std::size_t countBefore = context.getHistory().getCount();
        typeInto(y, 0, 90.0f);
        if (rotation() != StudioQuaternion{})
        {
            rotationRowY = y;
            break;
        }
        if (context.getHistory().getCount() != countBefore)
        {
            CNA_STUDIO_EXPECT(context.getHistory().undo());
        }
    }
    CNA_STUDIO_EXPECT(rotationRowY > 0.0f);

    typeInto(rotationRowY, 1, 40.0f);
    typeInto(rotationRowY, 2, 25.0f);

    CNA_STUDIO_EXPECT(rotation() == quaternionFromEulerDegrees(StudioVector3{90.0f, 40.0f, 25.0f}));
    CNA_STUDIO_EXPECT(context.getHistory().canUndo());

    // One entry per field committed -- pitch, then yaw, then roll each replace the whole
    // quaternion in turn, and none of the three is continuous input, so nothing here merges.
    // Three undoes is the round trip back to where the entity started.
    CNA_STUDIO_EXPECT(context.getHistory().undo());
    CNA_STUDIO_EXPECT(context.getHistory().undo());
    CNA_STUDIO_EXPECT(context.getHistory().undo());
    CNA_STUDIO_EXPECT(rotation() == StudioQuaternion{});
}

CNA_STUDIO_TEST(TheAngleCacheIsAbandonedTheInstantSomethingElseProducesADifferentQuaternion)
{
    // The other half of the acceptance, and the one the panel-level test above cannot isolate
    // cleanly: an undo through the real command history reverts one *field's* edit, which is a
    // different rotation from the one three individually-typed fields produce, but it is not the
    // specific "something entirely unrelated changed it" case the cache exists to be honest
    // about. This drives that case directly, the way `TheAnglesTheUserTypedSurviveGimbalLock`
    // drives entering it: through `studioPropertyEditor` itself, with no row to find.
    AngleFixture fixture;
    const StudioVector3 typed{90.0f, 40.0f, 25.0f};
    fixture.typeDegrees(typed);

    CNA_STUDIO_EXPECT_EQ(fixture.shownText("yaw"), studioFormatFloat(40.0f));
    CNA_STUDIO_EXPECT_EQ(fixture.shownText("roll"), studioFormatFloat(25.0f));

    // Something else -- a gizmo drag, an undo, a reload -- replaces the value directly, the way
    // every one of those actually reaches this editor: as a new `PropertyValue` from outside,
    // never as a call into this cache. Chosen so that it is not the rotation the cache holds.
    const StudioQuaternion external =
        quaternionFromEulerDegrees(StudioVector3{10.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(external != quaternionFromEulerDegrees(typed));
    fixture.value = PropertyValue{external};

    // The cache must stop applying at once: `quaternionFromEulerDegrees(cachedDegrees)` no
    // longer equals the stored value, so the comparison that gates the cache fails on the very
    // next frame, with nothing else having to notice or say so.
    const StudioVector3 honest = eulerDegreesOf(external);
    CNA_STUDIO_EXPECT_EQ(fixture.shownText("pitch"), studioFormatFloat(honest.x));
    CNA_STUDIO_EXPECT_EQ(fixture.shownText("yaw"), studioFormatFloat(honest.y));
    CNA_STUDIO_EXPECT_EQ(fixture.shownText("roll"), studioFormatFloat(honest.z));
}

// ------------------------------------------------------------------------------------------------
// The dependency section (STUDIO-09012)
// ------------------------------------------------------------------------------------------------

/**
 * @brief The asset inspector shows both directions, and a row is a way through the graph.
 *
 * The question a user opens an asset to answer before deleting it — and the one nothing on disk
 * records, because a scene holds a Uuid rather than a path. A section that drew its headings and
 * found no references would look exactly like an asset nothing uses, which is the opposite answer.
 */
CNA_STUDIO_TEST(TheAssetInspectorShowsWhatUsesAnAssetAndWhatItUses)
{
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cna-studio-depsection";
    std::error_code code;
    std::filesystem::remove_all(directory, code);
    std::filesystem::create_directories(directory / "Assets", code);

    StudioContext context;
    registerBuiltinComponents(context.getComponentRegistry());
    context.getAssets().setProjectRoot(directory.generic_string());

    const auto track = [&context](const std::string& path, AssetType type) {
        AssetRecord record;
        record.id = Uuid::generate();
        record.sourcePath = path;
        record.type = type;
        const Uuid id = record.id;
        CNA_STUDIO_EXPECT(context.getAssets().add(std::move(record)));
        return id;
    };

    const Uuid textureId = track("Assets/player.png", AssetType::Texture2D);
    const Uuid sceneId = track("Assets/Level.cnascene", AssetType::Scene);

    StudioEntity hero{Uuid::generate(), "Hero"};
    StudioComponent sprite{BuiltinComponentIds::kSpriteRenderer};
    sprite.applyDefaults(*context.getComponentRegistry().find(BuiltinComponentIds::kSpriteRenderer));
    sprite.setProperty("texture", PropertyValue{PropertyValue::AssetReference{textureId}});
    hero.addComponent(std::move(sprite));

    SceneDocument scene;
    scene.addEntity(std::move(hero));

    AssetDependencyIndex index;
    (void)index.build(context.getAssets(), context.getComponentRegistry());
    index.observeScene(scene, sceneId, "Assets/Level.cnascene");

    StudioDetailsServices services;
    services.dependencies = &index;

    CNA_STUDIO_EXPECT_EQ(index.referencedBy(textureId).size(), std::size_t{1});

    context.selectAsset(textureId);

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    UiRect bounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult pass = studioDetailsPanel(frame, area, context, services);
            if (frame.isInputPass()) { last = pass; }
            if (frame.isDrawPass()) { bounds = area; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));

    // The texture is used once and uses nothing.
    CNA_STUDIO_EXPECT_EQ(last.dependencyRows, std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});

    // A row is clickable, and clicking it goes to the file holding the reference. Swept rather
    // than assuming a row height, so a metric change cannot turn this into a test that clicks
    // empty space and passes for the wrong reason.
    bool navigated = false;
    for (float y = bounds.top() + 4.0f; y < bounds.bottom() - 4.0f && !navigated; y += 5.0f)
    {
        const float x = bounds.left() + 30.0f;
        shell->renderFrame(at(x, y, false));
        shell->renderFrame(at(x, y, true));
        shell->renderFrame(at(x, y, false));
        navigated = context.getSelectedAsset() == sceneId;
    }

    if (!navigated)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
                                     "no dependency row led to the scene that references the asset.");
    }
    else
    {
        // And from the scene's own inspector the other direction is the one with something in it.
        shell->renderFrame(at(-1.0f, -1.0f));
        CNA_STUDIO_EXPECT_EQ(last.dependencyRows, std::size_t{1});
    }

    std::filesystem::remove_all(directory, code);
}

/** @brief With no index, the section says so rather than reading as "nothing references this". */
CNA_STUDIO_TEST(WithoutAnIndexTheDependencySectionSaysSoRatherThanLookingEmpty)
{
    StudioContext context;
    registerBuiltinComponents(context.getComponentRegistry());

    AssetRecord record;
    record.id = Uuid::generate();
    record.sourcePath = "Assets/player.png";
    record.type = AssetType::Texture2D;
    const Uuid id = record.id;
    CNA_STUDIO_EXPECT(context.getAssets().add(std::move(record)));
    context.selectAsset(id);

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult pass = studioDetailsPanel(frame, area, context);
            if (frame.isInputPass()) { last = pass; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));

    // No rows, and the panel still drew: an inspector that threw away its other sections because
    // one seam was unset would be a build without a dependency index having no asset inspector.
    CNA_STUDIO_EXPECT_EQ(last.dependencyRows, std::size_t{0});
    CNA_STUDIO_EXPECT(last.rowsDrawn > 0);
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});
}

// ------------------------------------------------------------------------------------------------
// Finding a missing asset's file again (STUDIO-09013)
// ------------------------------------------------------------------------------------------------

/**
 * @brief A missing asset's inspector offers the repair rather than only reporting the problem.
 *
 * A record whose source has vanished is kept rather than dropped, because a scene references it by
 * id. What was missing was the *fixing*: a row marked red is a report, and the repair was to find
 * the file by hand and put it back where the path says.
 */
CNA_STUDIO_TEST(AMissingAssetsInspectorOffersToRelinkItAndDoingSoKeepsTheId)
{
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cna-studio-relinksection";
    std::error_code code;
    std::filesystem::remove_all(directory, code);
    std::filesystem::create_directories(directory / "Assets" / "Art", code);
    {
        std::ofstream stream{directory / "Assets" / "Art" / "player.png", std::ios::binary};
        stream << "pixels";
    }

    StudioContext context;
    registerBuiltinComponents(context.getComponentRegistry());
    context.getAssets().setProjectRoot(directory.generic_string());

    AssetRecord record;
    record.id = Uuid::generate();
    record.sourcePath = "Assets/player.png";
    record.type = AssetType::Texture2D;
    const Uuid id = record.id;
    CNA_STUDIO_EXPECT(context.getAssets().add(std::move(record)));
    CNA_STUDIO_EXPECT(context.getAssets().isMissing(id));
    context.selectAsset(id);

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    UiRect bounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult pass = studioDetailsPanel(frame, area, context);
            if (frame.isInputPass()) { last = pass; }
            if (frame.isDrawPass()) { bounds = area; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT_EQ(last.relinkCandidates, std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});

    // Clicking the suggestion applies it. Swept rather than assuming a row height, so a metric
    // change cannot turn this into a test that clicks empty space and passes for the wrong reason.
    bool relinked = false;
    for (float y = bounds.top() + 4.0f; y < bounds.bottom() - 4.0f && !relinked; y += 5.0f)
    {
        const float x = bounds.left() + 40.0f;
        shell->renderFrame(at(x, y, false));
        shell->renderFrame(at(x, y, true));
        shell->renderFrame(at(x, y, false));
        relinked = !context.getAssets().isMissing(id);
    }

    if (!relinked)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
                                     "no suggestion in the inspector repaired the missing asset.");
    }
    else
    {
        // The same id, which is the whole point: every scene that referenced this asset is correct
        // again without having been edited.
        CNA_STUDIO_EXPECT_EQ(context.getAssets().find(id)->sourcePath,
                             std::string{"Assets/Art/player.png"});
        CNA_STUDIO_EXPECT_EQ(context.getHistory().getCount(), std::size_t{1});

        // And it undoes, back to the state the user had.
        CNA_STUDIO_EXPECT(context.getHistory().undo());
        CNA_STUDIO_EXPECT(context.getAssets().isMissing(id));
    }

    std::filesystem::remove_all(directory, code);
}

/** @brief With nothing that looks like it, the inspector says so rather than offering nothing. */
CNA_STUDIO_TEST(AMissingAssetWithNoCandidatesSaysSoRatherThanShowingAnEmptySection)
{
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cna-studio-relinkempty";
    std::error_code code;
    std::filesystem::remove_all(directory, code);
    std::filesystem::create_directories(directory / "Assets", code);

    StudioContext context;
    registerBuiltinComponents(context.getComponentRegistry());
    context.getAssets().setProjectRoot(directory.generic_string());

    AssetRecord record;
    record.id = Uuid::generate();
    record.sourcePath = "Assets/player.png";
    record.type = AssetType::Texture2D;
    const Uuid id = record.id;
    CNA_STUDIO_EXPECT(context.getAssets().add(std::move(record)));
    context.selectAsset(id);

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult pass = studioDetailsPanel(frame, area, context);
            if (frame.isInputPass()) { last = pass; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));

    CNA_STUDIO_EXPECT_EQ(last.relinkCandidates, std::size_t{0});

    // The rest of the inspector is still there: an honest dead end for one section is not a reason
    // to stop showing the asset.
    CNA_STUDIO_EXPECT(last.rowsDrawn > 0);
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});

    std::filesystem::remove_all(directory, code);
}

// ------------------------------------------------------------------------------------------------
// Asset metadata and import settings (STUDIO-09014)
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(AByteCountReadsTheWayAFileManagerWritesIt)
{
    // An asset browser that disagrees with the file manager beside it is one people stop trusting
    // for the numbers they can check, so these are binary units.
    CNA_STUDIO_EXPECT_EQ(studioDescribeByteSize(0), std::string{"0 B"});
    CNA_STUDIO_EXPECT_EQ(studioDescribeByteSize(1023), std::string{"1023 B"});
    CNA_STUDIO_EXPECT_EQ(studioDescribeByteSize(1024), std::string{"1.0 KB"});
    CNA_STUDIO_EXPECT_EQ(studioDescribeByteSize(1536), std::string{"1.5 KB"});

    // One decimal below ten and none above: a tenth of a megabyte is noise on a number somebody is
    // comparing against their file manager.
    CNA_STUDIO_EXPECT_EQ(studioDescribeByteSize(9u * 1024u * 1024u + 512u * 1024u),
                         std::string{"9.5 MB"});
    CNA_STUDIO_EXPECT_EQ(studioDescribeByteSize(100u * 1024u * 1024u), std::string{"100 MB"});

    // It stops at TB rather than running off the end of the unit table.
    CNA_STUDIO_EXPECT(studioDescribeByteSize(std::uint64_t{1} << 50).find("TB")
                      != std::string::npos);

    // Zero means unknown for a stamp, and says so rather than showing 1970.
    CNA_STUDIO_EXPECT_EQ(studioDescribeFileTime(0), std::string{"unknown"});
}

/**
 * @brief An overridden import setting says so and can be put back, without leaving the panel.
 *
 * A setting the user chose and one that happens to equal the default look identical otherwise, and
 * only one of them is a decision. Before this the only way back to the default was Ctrl+Z, which
 * stops being an option the moment anything else is edited.
 */
CNA_STUDIO_TEST(AnOverriddenImportSettingCanBeResetFromTheInspector)
{
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cna-studio-importreset-ui";
    std::error_code code;
    std::filesystem::remove_all(directory, code);
    std::filesystem::create_directories(directory / "Textures", code);
    {
        std::ofstream stream{directory / "Textures" / "Hero.png", std::ios::binary};
        stream << "not really a png";
    }

    StudioContext context;
    registerBuiltinComponents(context.getComponentRegistry());
    registerBuiltinImporters(context.getImporterRegistry());
    context.getAssets().setProjectRoot(directory.generic_string());
    CNA_STUDIO_EXPECT(context.getAssets().scan("Textures").succeeded);

    const Uuid id = context.getAssets().findByPath("Textures/Hero.png")->id;
    context.selectAsset(id);

    // Overridden, through the same command the inspector's own editors use.
    context.execute(std::make_unique<SetImporterSettingCommand>(
        context.getAssets(), id, "generateMipmaps", PropertyValue{false}));
    CNA_STUDIO_EXPECT(!context.getAssets().find(id)->importerSettings["generateMipmaps"].isNull());

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    UiRect bounds;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult pass = studioDetailsPanel(frame, area, context);
            if (frame.isInputPass()) { last = pass; }
            if (frame.isDrawPass()) { bounds = area; }
        }));
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(last.rowsDrawn > 0);

    // The reset sits at the right-hand end of the row, so the sweep runs down that edge. Swept
    // rather than assuming a row height, so a metric change cannot make this click empty space --
    // and scrolled between sweeps, because the inspector is taller than the panel and the row this
    // is looking for need not be on the first screenful. A sweep that only searched what happened
    // to be visible would pass or fail on how many settings the importer declares that week.
    // Two x positions, because the inspector is taller than the panel and therefore has a
    // scrollbar: the one that reaches the reset is inside the content, which the scrollbar's
    // thickness has already been taken out of. Sweeping the bare right edge alone would click the
    // track and pass for nobody.
    const float scrollbar = metricOf(StudioTheme::dark(), StudioMetric::ScrollbarThickness);
    const float wheelAt = bounds.centerX();

    bool reset = false;
    for (int screenful = 0; screenful < 8 && !reset; ++screenful)
    {
        // Scrolled between sweeps, so the row this is looking for does not have to be on the first
        // screenful. A sweep that only searched what happened to be visible would pass or fail on
        // how many settings the importer declares that week.
        if (screenful > 0)
        {
            for (int turn = 0; turn < 3; ++turn)
            {
                UiInputState wheel = at(wheelAt, bounds.centerY());
                wheel.wheelY = -1.0f;
                shell->renderFrame(wheel);
            }
        }

        for (float y = bounds.top() + 4.0f; y < bounds.bottom() - 4.0f && !reset; y += 4.0f)
        {
            for (const float x : {bounds.right() - 14.0f, bounds.right() - scrollbar - 14.0f})
            {
                shell->renderFrame(at(x, y, false));
                shell->renderFrame(at(x, y, true));
                shell->renderFrame(at(x, y, false));
                reset = context.getAssets().find(id)->importerSettings["generateMipmaps"].isNull();
                if (reset) { break; }
            }
        }
    }

    if (!reset)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
                                     "no control in the inspector reset the overridden setting.");
    }
    else
    {
        // Removed rather than written back as a default, and undoable like every other edit.
        CNA_STUDIO_EXPECT(context.getHistory().canUndo());
        CNA_STUDIO_EXPECT(context.getHistory().undo());
        CNA_STUDIO_EXPECT(
            !context.getAssets().find(id)->importerSettings["generateMipmaps"].isNull());
    }

    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});
    std::filesystem::remove_all(directory, code);
}

CNA_STUDIO_TEST(ATexturesInspectorSaysWhatItsSettingsActuallyProduce)
{
    // `plan.md` STUDIO-10003. The settings above this section are what the user asked for; this is
    // what they get. The two differ often enough that a user who could not see the difference would
    // meet it as a texture that is quietly the wrong format.
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cna-studio-textureplan-ui";
    std::error_code code;
    std::filesystem::remove_all(directory, code);
    std::filesystem::create_directories(directory / "Textures", code);

    // A PNG header and nothing more. `readImageDescription` never decodes and never checks a CRC,
    // so this is the whole of what the facts pass reads -- and keeping it to that makes the case
    // legible in a review, which a committed binary would not be.
    const auto writePngHeader = [&](const std::string& name, std::uint32_t width,
                                    std::uint32_t height, unsigned char colorType) {
        std::vector<unsigned char> png{0x89u, 'P', 'N', 'G', 0x0Du, 0x0Au, 0x1Au, 0x0Au};
        const auto bigEndian = [&](std::uint32_t value) {
            png.push_back(static_cast<unsigned char>((value >> 24) & 0xFFu));
            png.push_back(static_cast<unsigned char>((value >> 16) & 0xFFu));
            png.push_back(static_cast<unsigned char>((value >> 8) & 0xFFu));
            png.push_back(static_cast<unsigned char>(value & 0xFFu));
        };
        bigEndian(13);
        for (const char letter : std::string{"IHDR"}) { png.push_back(static_cast<unsigned char>(letter)); }
        bigEndian(width);
        bigEndian(height);
        png.push_back(8);
        png.push_back(colorType);
        png.push_back(0);
        png.push_back(0);
        png.push_back(0);
        bigEndian(0);

        std::ofstream stream{directory / "Textures" / name, std::ios::binary};
        stream.write(reinterpret_cast<const char*>(png.data()),
                     static_cast<std::streamsize>(png.size()));
    };

    writePngHeader("Crate.png", 256, 256, 2);  // opaque
    {
        std::ofstream stream{directory / "Textures" / "Broken.png", std::ios::binary};
        stream << "not really a png";
    }

    StudioContext context;
    registerBuiltinComponents(context.getComponentRegistry());
    registerBuiltinImporters(context.getImporterRegistry());
    context.getAssets().setProjectRoot(directory.generic_string());
    CNA_STUDIO_EXPECT(context.getAssets().scan("Textures").succeeded);
    (void)applyImporterFacts(context.getAssets());

    const Uuid crate = context.getAssets().findByPath("Textures/Crate.png")->id;
    const Uuid broken = context.getAssets().findByPath("Textures/Broken.png")->id;

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult pass = studioDetailsPanel(frame, area, context);
            if (frame.isInputPass()) { last = pass; }
        }));

    context.selectAsset(crate);
    shell->renderFrame(at(-1.0f, -1.0f));

    // Defaults: uncompressed, no hardware sRGB, no mips. One level, four bytes a pixel.
    CNA_STUDIO_EXPECT_EQ(last.texturePlan.surfaceFormat, std::string{"Color"});
    CNA_STUDIO_EXPECT_EQ(last.texturePlan.mipLevels, std::uint32_t{1});
    CNA_STUDIO_EXPECT_EQ(last.texturePlan.estimatedBytes, std::uint64_t{256} * 256u * 4u);
    CNA_STUDIO_EXPECT(last.texturePlan.isExactlyAsAsked());

    // A setting changed through the same command the inspector's own editors use, and the section
    // follows it on the very next frame. That is the property that makes this a derivation rather
    // than a fact: nothing was reimported, nothing was written to the sidecar to say so.
    context.execute(std::make_unique<SetImporterSettingCommand>(
        context.getAssets(), crate, "outputFormat",
        PropertyValue{PropertyValue::EnumValue{"DxtCompressed"}}));
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(last.texturePlan.surfaceFormat, std::string{"Dxt1"});
    CNA_STUDIO_EXPECT_EQ(last.texturePlan.estimatedBytes, std::uint64_t{256} * 256u / 2u);

    context.execute(std::make_unique<SetImporterSettingCommand>(
        context.getAssets(), crate, "generateMipmaps", PropertyValue{true}));
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(last.texturePlan.mipLevels, std::uint32_t{9});

    // sRGB on an opaque compressed texture is the case CNA has no format for, and the inspector
    // is where a user meets it: the substitution is a row of its own rather than a silent change.
    context.execute(std::make_unique<SetImporterSettingCommand>(
        context.getAssets(), crate, "srgbRead", PropertyValue{true}));
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT_EQ(last.texturePlan.surfaceFormat, std::string{"Dxt5SrgbEXT"});
    CNA_STUDIO_EXPECT(!last.texturePlan.isExactlyAsAsked());

    // A file whose header Studio cannot read gets no resolved format at all, rather than one built
    // out of struct defaults that would read exactly like a real answer.
    context.selectAsset(broken);
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(last.texturePlan.surfaceFormat.empty());
    CNA_STUDIO_EXPECT_EQ(last.texturePlan.notes.size(), std::size_t{1});

    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});
    std::filesystem::remove_all(directory, code);
}

// ------------------------------------------------------------------------------------------------
// The Details panel stops opening files to draw itself (STUDIO-30016)
// ------------------------------------------------------------------------------------------------

/**
 * @brief Drawing the material editor opens no file, and the test says so rather than implying it.
 *
 * It was one file open per frame for as long as the material was selected — already halved by
 * working on the input pass and keeping the result for the draw pass, and still a read per frame.
 */
CNA_STUDIO_TEST(DrawingTheMaterialEditorOpensNoFile)
{
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cna-studio-materialreads";
    std::error_code code;
    std::filesystem::remove_all(directory, code);
    std::filesystem::create_directories(directory / "Assets", code);

    MaterialDocument material;
    material.roughness = 0.25f;
    {
        std::ofstream stream{directory / "Assets" / "Stone.cnamaterial", std::ios::binary};
        stream << Json::write(material.toJson(), true);
    }

    StudioContext context;
    registerBuiltinComponents(context.getComponentRegistry());
    context.getAssets().setProjectRoot(directory.generic_string());
    CNA_STUDIO_EXPECT(context.getAssets().scan("Assets").succeeded);

    const Uuid id = context.getAssets().findByPath("Assets/Stone.cnamaterial")->id;
    context.selectAsset(id);

    StudioAssetDocumentCache documents;
    StudioDetailsServices services;
    services.documents = &documents;

    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    shell->resetLayout();
    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(shell->activatePanel("details"));

    StudioDetailsResult last;
    CNA_STUDIO_EXPECT(shell->setPanelContent("details",
        [&](StudioFrame& frame, const UiRect& area) {
            const StudioDetailsResult pass = studioDetailsPanel(frame, area, context, services);
            if (frame.isInputPass()) { last = pass; }
        }));

    shell->renderFrame(at(-1.0f, -1.0f));
    CNA_STUDIO_EXPECT(last.materialFields > 0);

    const std::uint64_t before = documents.getFileReadCount();
    CNA_STUDIO_EXPECT(before > 0);

    for (int frame = 0; frame < 5; ++frame) { shell->renderFrame(at(-1.0f, -1.0f)); }

    const std::uint64_t after = documents.getFileReadCount();
    if (after != before)
    {
        CnaStudioTest::reportFailure(__FILE__, __LINE__,
                                     "drawing five frames of the material editor opened "
                                         + std::to_string(after - before)
                                         + " files; it must open none.");
    }

    // And it is still showing the material rather than passing because it drew nothing.
    CNA_STUDIO_EXPECT(last.materialFields > 0);
    CNA_STUDIO_EXPECT_EQ(shell->frame().phaseViolations(), std::size_t{0});

    std::filesystem::remove_all(directory, code);
}
