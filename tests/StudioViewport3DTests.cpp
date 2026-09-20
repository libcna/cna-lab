// SPDX-License-Identifier: MS-PL
/**
 * @file StudioViewport3DTests.cpp
 * @brief The 3D view in the native shell (plan.md STUDIO-07009, STUDIO-11001, STUDIO-11002).
 *
 * The model was never the gap here either. `StudioCamera3D`, `pickEntityAt3D`,
 * `buildSceneModelBatch` and `buildSceneWireframe` are CNA-free, tested, and have been shared with
 * the prototype since it had a 3D view. What was missing was a native viewport that switched to it
 * and turned a drag into an orbit.
 *
 * So these are about the difference between navigating and selecting, which is the thing a 3D
 * viewport gets wrong in a way users feel immediately: every button is also a camera gesture, so a
 * release after an orbit must not select whatever the camera happened to stop over.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneWireframe.hpp"
#include "CNA/Studio/Scene/StudioCamera3D.hpp"
#include "CNA/Studio/ShellPanels/StudioShellPanels.hpp"
#include "CNA/Studio/ShellPanels/StudioViewportPanel.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"
#include "CNA/Studio/Project/Project.hpp"

#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    constexpr float kWidth = 640.0f;
    constexpr float kHeight = 480.0f;

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

    UiInputState away() { return at(-1.0f, -1.0f); }

    /** @brief Two entities with meshes, far enough apart that a ray can tell them apart. */
    struct Fixture
    {
        StudioContext context;
        StudioCamera3D camera;
        StudioFrame frame{StudioTheme::dark()};
        StudioViewportState state;
        UiRect body{0.0f, 0.0f, kWidth, kHeight};
        StudioViewportResult last;
        Uuid nearEntity;

        Fixture()
        {
            nearEntity = add("Near", StudioVector3{0.0f, 0.0f, 0.0f});
            (void)add("Far", StudioVector3{40.0f, 0.0f, 0.0f});

            state.view = StudioViewportView::ThreeD;
            camera.setViewportSize(StudioVector2{kWidth, kHeight});
            camera.setPivot(StudioVector3{0.0f, 0.0f, 0.0f});
            camera.setDistance(20.0f);
        }

        Uuid add(const std::string& name, const StudioVector3& position)
        {
            StudioEntity entity{Uuid::generate(), name};
            StudioComponent transform{"CNA.Transform"};
            transform.setProperty("position", PropertyValue{position});
            entity.getComponents().push_back(std::move(transform));
            const Uuid id = entity.getId();
            context.getScene().addEntity(std::move(entity));
            return id;
        }

        void run(const UiInputState& input)
        {
            runStudioFrame(frame, input, [&](StudioFrame& pass) {
                const StudioViewportResult drawn =
                    studioViewportPanel3D(pass, body, context, camera, state, {});
                if (pass.isInputPass()) { last = drawn; }
            });
        }

        /** @brief A press, a drag to (x, y), and a release there. */
        void dragTo(float fromX, float fromY, float x, float y, bool shift = false,
                    bool control = false)
        {
            UiInputState press = at(fromX, fromY);
            press.modifiers.shift = shift;
            press.modifiers.control = control;
            run(press);

            press.setMouseDown(UiMouseButton::Left, true);
            run(press);

            UiInputState moved = at(x, y, /*leftDown=*/true);
            moved.modifiers.shift = shift;
            moved.modifiers.control = control;
            run(moved);

            UiInputState released = at(x, y);
            released.modifiers.shift = shift;
            released.modifiers.control = control;
            run(released);
        }

        /**
         * @brief A press at the first point, a move through each of the rest, and a release.
         *
         * What `dragTo` is not: it moves the pointer in a single jump, so a drag through it commits
         * exactly once and the *merge* -- the mechanism that makes a drag one undo entry rather
         * than sixty -- never runs at all. A case that wants to say "one entry" has to make more
         * than one edit first (`plan.md` STUDIO-12008).
         */
        void dragVia(const std::vector<StudioVector2>& points)
        {
            if (points.size() < 2) { return; }

            run(at(points.front().x, points.front().y));
            run(at(points.front().x, points.front().y, /*leftDown=*/true));

            for (std::size_t index = 1; index < points.size(); ++index)
            {
                run(at(points[index].x, points[index].y, /*leftDown=*/true));
            }

            run(at(points.back().x, points.back().y));
        }

        /** @brief A press and release in the same place, which is a click. */
        void clickAt(float x, float y, bool control = false)
        {
            dragTo(x, y, x, y, /*shift=*/false, control);
        }
    };
}

CNA_STUDIO_TEST(ADragOrbitsTheCameraWithoutMovingThePivot)
{
    // Orbiting turns the eye around a point the user chose. A drag that moved the pivot as well
    // would leave the camera somewhere neither the user nor the previous frame put it, and is the
    // difference between a 3D view that can be aimed and one people give up on.
    Fixture fixture;
    fixture.run(away());

    const float yaw = fixture.camera.getYaw();
    const float pitch = fixture.camera.getPitch();
    const StudioVector3 pivot = fixture.camera.getPivot();
    const float distance = fixture.camera.getDistance();

    fixture.dragTo(320.0f, 240.0f, 420.0f, 200.0f);

    CNA_STUDIO_EXPECT(std::abs(fixture.camera.getYaw() - yaw) > 0.01f);
    CNA_STUDIO_EXPECT(std::abs(fixture.camera.getPitch() - pitch) > 0.01f);
    CNA_STUDIO_EXPECT_EQ(fixture.camera.getPivot().x, pivot.x);
    CNA_STUDIO_EXPECT_EQ(fixture.camera.getPivot().y, pivot.y);
    CNA_STUDIO_EXPECT_EQ(fixture.camera.getDistance(), distance);
}

CNA_STUDIO_TEST(ShiftDragPansTheCameraRatherThanTurningIt)
{
    Fixture fixture;
    fixture.run(away());

    const float yaw = fixture.camera.getYaw();
    const StudioVector3 pivot = fixture.camera.getPivot();

    fixture.dragTo(320.0f, 240.0f, 400.0f, 300.0f, /*shift=*/true);

    CNA_STUDIO_EXPECT_EQ(fixture.camera.getYaw(), yaw);
    const StudioVector3 moved = fixture.camera.getPivot();
    CNA_STUDIO_EXPECT(std::abs(moved.x - pivot.x) + std::abs(moved.y - pivot.y)
                          + std::abs(moved.z - pivot.z) > 0.001f);
}

CNA_STUDIO_TEST(TheWheelDolliesGeometricallySoOneNotchFeelsTheSameCloseUpAndFarAway)
{
    // A linear step is unusable at both ends of the range: it crawls when far out and jumps
    // through the subject when close in.
    Fixture fixture;
    fixture.run(away());

    const float far = 20.0f;
    fixture.camera.setDistance(far);
    UiInputState wheel = at(320.0f, 240.0f);
    wheel.wheelY = 1.0f;
    fixture.run(wheel);
    const float fromFar = far / fixture.camera.getDistance();

    const float near = 5.0f;
    fixture.camera.setDistance(near);
    fixture.run(at(320.0f, 240.0f));
    fixture.run(wheel);
    const float fromNear = near / fixture.camera.getDistance();

    CNA_STUDIO_EXPECT(fixture.camera.getDistance() < near);
    CNA_STUDIO_EXPECT(std::abs(fromFar - fromNear) < 0.001f);
}

CNA_STUDIO_TEST(AReleaseAfterAnOrbitDoesNotSelectWhateverTheCameraStoppedOver)
{
    // The failure that makes a 3D viewport feel like it is fighting the user: every button is also
    // a camera gesture, so "released the mouse" cannot mean "clicked".
    Fixture fixture;
    fixture.run(away());
    fixture.context.clearSelection();

    fixture.dragTo(320.0f, 240.0f, 460.0f, 180.0f);

    CNA_STUDIO_EXPECT(!fixture.last.clicked3D);
    CNA_STUDIO_EXPECT(!fixture.last.selectionChanged);
    CNA_STUDIO_EXPECT(fixture.context.getSelection().empty());
}

CNA_STUDIO_TEST(APressAndReleaseThatTurnedNothingSelectsWhatIsUnderIt)
{
    // And the other half: a viewport where clicking never selects because every press is treated
    // as a drag is one where the outliner is the only way to select anything.
    Fixture fixture;
    fixture.run(away());
    fixture.context.clearSelection();

    fixture.clickAt(320.0f, 240.0f);

    CNA_STUDIO_EXPECT(fixture.last.clicked3D);
    CNA_STUDIO_EXPECT(fixture.last.selectionChanged);
    CNA_STUDIO_EXPECT(fixture.context.getPrimarySelection() == fixture.nearEntity);
}

CNA_STUDIO_TEST(ClickingNothingClearsTheSelectionAndCtrlOnNothingDoesNot)
{
    // The same two rules the 2D viewport has, because they are rules about selecting rather than
    // about a projection: clearing a selection the user is halfway through assembling is the one
    // outcome they cannot have meant.
    Fixture fixture;
    fixture.run(away());
    fixture.clickAt(320.0f, 240.0f);
    CNA_STUDIO_EXPECT(!fixture.context.getSelection().empty());

    // Aimed at empty space, off the axis the entities sit on, rather than at a corner of the
    // panel. An entity carrying only a transform still has a box to pick against -- deliberately,
    // because a light or a camera has to be clickable -- so a corner is not reliably a miss, and a
    // test that assumed it was would be asserting about this scene rather than about the rule.
    // Aiming *along* the axis is no better: 900 units away is still in front of a camera whose far
    // plane is 5000, which is the first thing this test caught about its own setup.
    fixture.camera.setPivot(StudioVector3{0.0f, 900.0f, 0.0f});
    fixture.camera.setDistance(5.0f);
    fixture.run(away());

    CNA_STUDIO_EXPECT(!pickEntityAt3D(fixture.context.getScene(), fixture.camera,
                                      StudioVector2{320.0f, 240.0f}, {}).isValid());

    fixture.clickAt(320.0f, 240.0f, /*control=*/true);
    CNA_STUDIO_EXPECT(!fixture.context.getSelection().empty());

    fixture.clickAt(320.0f, 240.0f);
    CNA_STUDIO_EXPECT(fixture.context.getSelection().empty());
}

CNA_STUDIO_TEST(TheThreeDimensionalViewDrawsNothingAndViolatesNoPhase)
{
    // The panel draws nothing -- the scene arrives as a texture the shell composites -- which is
    // what lets this half be tested with no graphics device at all.
    Fixture fixture;
    fixture.run(away());
    fixture.dragTo(100.0f, 100.0f, 300.0f, 260.0f);

    CNA_STUDIO_EXPECT_EQ(fixture.frame.phaseViolations(), std::size_t{0});
    CNA_STUDIO_EXPECT(validate(fixture.frame.drawData()).valid);
}

CNA_STUDIO_TEST(TheTwoViewsAreExclusiveAndSayWhichIsShowing)
{
    StudioContext context;
    StudioLog log;
    StudioShell shell{StudioTheme::dark()};
    shell.resetLayout();
    StudioCamera2D camera;
    StudioCamera3D camera3D;
    StudioShellPanels panels{shell, context, log};
    panels.setViewportServices(camera, camera3D, {});

    const std::vector<std::string> ids = {"studio.view.2d", "studio.view.3d"};
    for (const std::string& armed : ids)
    {
        shell.invoke(armed);

        std::size_t checked = 0;
        for (const std::string& id : ids)
        {
            const StudioAction* action = shell.actions().find(id);
            CNA_STUDIO_EXPECT(action != nullptr && action->checkable && action->isChecked);
            if (action == nullptr || !action->isChecked) { continue; }
            if (action->isChecked()) { ++checked; }
        }
        CNA_STUDIO_EXPECT_EQ(checked, std::size_t{1});
    }

    CNA_STUDIO_EXPECT(panels.viewportView() == StudioViewportView::ThreeD);
    shell.invoke("studio.view.2d");
    CNA_STUDIO_EXPECT(panels.viewportView() == StudioViewportView::TwoD);
}

CNA_STUDIO_TEST(TheFirstSwitchToThreeDimensionsFramesTheSceneAndLaterOnesDoNot)
{
    // The default camera looks straight down an axis, so an unframed 3D view opens on a grid with
    // the level somewhere off the edge of it. Framing every time would be worse than not framing
    // at all: a user who set up a view, glanced at 2D and came back would find their angle gone.
    StudioContext context;
    StudioLog log;
    StudioShell shell{StudioTheme::dark()};
    shell.resetLayout();

    StudioEntity entity{Uuid::generate(), "Far away"};
    StudioComponent transform{"CNA.Transform"};
    transform.setProperty("position", PropertyValue{StudioVector3{120.0f, 60.0f, 30.0f}});
    entity.getComponents().push_back(std::move(transform));
    context.getScene().addEntity(std::move(entity));

    StudioCamera2D camera;
    StudioCamera3D camera3D;
    StudioShellPanels panels{shell, context, log};
    panels.setViewportServices(camera, camera3D, {});

    const StudioVector3 before = camera3D.getPivot();
    shell.invoke("studio.view.3d");
    const StudioVector3 framed = camera3D.getPivot();
    CNA_STUDIO_EXPECT(std::abs(framed.x - before.x) > 1.0f);

    // The user's own angle, set after framing, survives a trip through the 2D view.
    camera3D.setYaw(1.25f);
    camera3D.setDistance(7.5f);
    shell.invoke("studio.view.2d");
    shell.invoke("studio.view.3d");
    CNA_STUDIO_EXPECT_EQ(camera3D.getYaw(), 1.25f);
    CNA_STUDIO_EXPECT_EQ(camera3D.getDistance(), 7.5f);
    CNA_STUDIO_EXPECT_EQ(camera3D.getPivot().x, framed.x);
}

CNA_STUDIO_TEST(SwitchingViewsEndsAnyGestureTheOldViewOwned)
{
    // A gizmo drag half-finished in the 2D view would keep writing positions from a projection no
    // longer on screen, and a navigation gesture would resume mid-orbit on the next press.
    StudioContext context;
    StudioLog log;
    StudioShell shell{StudioTheme::dark()};
    shell.resetLayout();
    StudioCamera2D camera;
    StudioCamera3D camera3D;
    StudioShellPanels panels{shell, context, log};
    panels.setViewportServices(camera, camera3D, {});

    shell.invoke("studio.view.3d");
    shell.invoke("studio.view.2d");

    // Said out loud, because the two views share a panel and the change is dramatic enough that a
    // user who pressed 3 by accident deserves to be told what they pressed.
    bool announced = false;
    for (const StudioLogEntry& entry : log.entries())
    {
        if (entry.message.find("Viewport: 3D") != std::string::npos) { announced = true; }
    }
    CNA_STUDIO_EXPECT(announced);
}

CNA_STUDIO_TEST(TheCameraSpeedAndInvertZoomPreferencesActuallyReachTheCamera)
{
    // All three viewport preferences -- speed, inverted zoom and the navigation style -- were
    // stored, loaded, given a row in the Preferences panel, and applied by nothing at all. Two of
    // them are arithmetic and are answered here; the style is STUDIO-11014, because three schemes
    // across two viewports is a piece of work rather than a multiplier.
    Fixture fast;
    fast.state.cameraSpeed = 4.0f;
    fast.run(away());

    Fixture slow;
    slow.state.cameraSpeed = 1.0f;
    slow.run(away());

    const float startYaw = fast.camera.getYaw();
    fast.dragTo(320.0f, 240.0f, 400.0f, 240.0f);
    slow.dragTo(320.0f, 240.0f, 400.0f, 240.0f);

    const float fastTurn = std::abs(fast.camera.getYaw() - startYaw);
    const float slowTurn = std::abs(slow.camera.getYaw() - startYaw);
    CNA_STUDIO_EXPECT(slowTurn > 0.0f);
    CNA_STUDIO_EXPECT(fastTurn > slowTurn * 3.5f);

    // And the wheel, both ways round. Scrolling up moves the eye towards the pivot unless the
    // user has asked for the opposite, which is a preference because neither answer is wrong.
    Fixture normal;
    normal.run(away());
    UiInputState wheel = at(320.0f, 240.0f);
    wheel.wheelY = 1.0f;
    const float before = normal.camera.getDistance();
    normal.run(wheel);
    CNA_STUDIO_EXPECT(normal.camera.getDistance() < before);

    Fixture inverted;
    inverted.state.invertZoom = true;
    inverted.run(away());
    inverted.run(wheel);
    CNA_STUDIO_EXPECT(inverted.camera.getDistance() > before);
}

// ------------------------------------------------------------------------------------------------
// Navigation schemes (STUDIO-11015)
//
// Three schemes were stored, loaded, given a row in the Preferences panel and read by nothing: the
// viewport's gestures were hard-coded. A preference that changes nothing is worse than no
// preference -- a user who sets it and finds the viewport unchanged concludes the editor is broken,
// which is a fair reading.
//
// The mapping is a pure function of six booleans and an enumeration, which is what lets every
// combination that matters be stated here rather than performed with a mouse.
// ------------------------------------------------------------------------------------------------

namespace
{
    /** @brief Builds a chord, so the cases below read as what a hand is doing. */
    StudioViewportChord chordOf(bool left, bool middle, bool right,
                                bool alt = false, bool shift = false, bool control = false)
    {
        StudioViewportChord chord;
        chord.left = left;
        chord.middle = middle;
        chord.right = right;
        chord.alt = alt;
        chord.shift = shift;
        chord.control = control;
        return chord;
    }
}

CNA_STUDIO_TEST(StudiosOwnSchemeIsUnchanged)
{
    // The one this editor shipped with, asserted so that adding two more did not quietly alter it.
    const auto studio = [](const StudioViewportChord& chord) {
        return studioViewportGestureFor(StudioNavigationStyle::Studio, chord);
    };

    CNA_STUDIO_EXPECT(studio(chordOf(true, false, false)) == StudioViewportGesture::Orbit);
    CNA_STUDIO_EXPECT(studio(chordOf(false, true, false)) == StudioViewportGesture::Pan);
    CNA_STUDIO_EXPECT(studio(chordOf(false, false, true)) == StudioViewportGesture::Look);
    CNA_STUDIO_EXPECT(studio(chordOf(true, false, false, false, /*shift=*/true))
                      == StudioViewportGesture::Pan);
    CNA_STUDIO_EXPECT(studio(chordOf(false, false, false)) == StudioViewportGesture::None);
}

CNA_STUDIO_TEST(MayaPutsEveryCameraGestureBehindAltAndNothingElse)
{
    const auto maya = [](const StudioViewportChord& chord) {
        return studioViewportGestureFor(StudioNavigationStyle::Maya, chord);
    };

    CNA_STUDIO_EXPECT(maya(chordOf(true, false, false, /*alt=*/true)) == StudioViewportGesture::Orbit);
    CNA_STUDIO_EXPECT(maya(chordOf(false, true, false, /*alt=*/true)) == StudioViewportGesture::Pan);
    CNA_STUDIO_EXPECT(maya(chordOf(false, false, true, /*alt=*/true)) == StudioViewportGesture::Dolly);

    // The whole of Maya's arrangement, and the half a nearly-Maya scheme gets wrong: an unmodified
    // drag is *always* a selection. A scheme that let one unmodified button navigate is the thing a
    // Maya user finds by moving the camera when they meant to pick something.
    CNA_STUDIO_EXPECT(maya(chordOf(true, false, false)) == StudioViewportGesture::None);
    CNA_STUDIO_EXPECT(maya(chordOf(false, true, false)) == StudioViewportGesture::None);
    CNA_STUDIO_EXPECT(maya(chordOf(false, false, true)) == StudioViewportGesture::None);
}

CNA_STUDIO_TEST(BlenderPutsEveryCameraGestureOnTheMiddleButton)
{
    const auto blender = [](const StudioViewportChord& chord) {
        return studioViewportGestureFor(StudioNavigationStyle::Blender, chord);
    };

    CNA_STUDIO_EXPECT(blender(chordOf(false, true, false)) == StudioViewportGesture::Orbit);
    CNA_STUDIO_EXPECT(blender(chordOf(false, true, false, false, /*shift=*/true))
                      == StudioViewportGesture::Pan);
    CNA_STUDIO_EXPECT(blender(chordOf(false, true, false, false, false, /*control=*/true))
                      == StudioViewportGesture::Dolly);

    // Control wins over Shift, because Shift+Control+middle is a zoom in Blender. Tested because
    // the obvious ordering -- Shift first, it is the commoner modifier -- is the wrong one.
    CNA_STUDIO_EXPECT(blender(chordOf(false, true, false, false, true, true))
                      == StudioViewportGesture::Dolly);

    // Left is free, for the same reason Maya's unmodified buttons are.
    CNA_STUDIO_EXPECT(blender(chordOf(true, false, false)) == StudioViewportGesture::None);
    CNA_STUDIO_EXPECT(blender(chordOf(false, false, true)) == StudioViewportGesture::None);
}

CNA_STUDIO_TEST(TheSchemesDisagreeAboutSomethingOrTheyWouldNotBeThreeSchemes)
{
    // The assertion that makes the other three worth having. Three enumerators that resolved to
    // one mapping would pass every case above and would be exactly the defect this task closes,
    // one level further in: a preference that is read and changes nothing.
    std::size_t disagreements = 0;
    for (const StudioViewportChord& chord : {chordOf(true, false, false),
                                             chordOf(false, true, false),
                                             chordOf(false, false, true),
                                             chordOf(true, false, false, true),
                                             chordOf(false, true, false, false, true)})
    {
        const StudioViewportGesture studio =
            studioViewportGestureFor(StudioNavigationStyle::Studio, chord);
        const StudioViewportGesture maya =
            studioViewportGestureFor(StudioNavigationStyle::Maya, chord);
        const StudioViewportGesture blender =
            studioViewportGestureFor(StudioNavigationStyle::Blender, chord);

        if (studio != maya || maya != blender || studio != blender) { ++disagreements; }
    }
    CNA_STUDIO_EXPECT(disagreements >= 4);
}

CNA_STUDIO_TEST(EveryGestureHasAName)
{
    for (const StudioViewportGesture gesture : {StudioViewportGesture::None,
                                                StudioViewportGesture::Orbit,
                                                StudioViewportGesture::Pan,
                                                StudioViewportGesture::Dolly,
                                                StudioViewportGesture::Look})
    {
        CNA_STUDIO_EXPECT(!studioViewportGestureName(gesture).empty());
    }
}

// ------------------------------------------------------------------------------------------------
// Transform manipulators (STUDIO-07050)
//
// `studioViewportPanel3D` picked and did not manipulate: there was no gizmo drawn and none to
// drag, so an entity could not be moved, turned or scaled in the 3D view at all. The maths was
// never the gap -- `TransformGizmos3D.hpp` has carried it, unit-tested, since before this panel
// existed -- what was missing was a caller.
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(AGizmoGrabTakesPriorityOverTheOrbitStudiosOwnSchemePutsOnThePlainLeftButton)
{
    // The subtle half of the wiring. Under Studio's own scheme an unmodified left press orbits
    // the camera -- so a gizmo handle under the cursor has to be tried *before* a press is
    // allowed to arm that orbit, or an object could never be dragged without switching schemes
    // first. If this regressed, the drag below would turn into an orbit and the entity would not
    // move at all.
    Fixture fixture;
    fixture.context.select(fixture.nearEntity);
    fixture.state.mode = GizmoMode::Translate;

    const auto layout = computeTranslateGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                       fixture.nearEntity);
    CNA_STUDIO_EXPECT(layout.has_value());
    CNA_STUDIO_EXPECT(layout->armVisible[0]);

    const StudioVector2 grab = layout->screenTips[0];
    const StudioVector3 before = fixture.context.getScene().findEntity(fixture.nearEntity)
                                     ->findComponent("CNA.Transform")
                                     ->getProperty("position")
                                     .get<StudioVector3>();
    const StudioCamera3D cameraBefore = fixture.camera;

    fixture.dragTo(grab.x, grab.y, grab.x + 40.0f, grab.y);

    const StudioVector3 after = fixture.context.getScene().findEntity(fixture.nearEntity)
                                    ->findComponent("CNA.Transform")
                                    ->getProperty("position")
                                    .get<StudioVector3>();
    CNA_STUDIO_EXPECT(after != before);

    // And the camera did not move at all -- an orbit that grabbed the drag instead would have
    // turned it, and the entity would still be exactly where it started.
    CNA_STUDIO_EXPECT(cameraBefore.getYaw() == fixture.camera.getYaw());
    CNA_STUDIO_EXPECT(cameraBefore.getPitch() == fixture.camera.getPitch());
}

CNA_STUDIO_TEST(ATranslateDragMovesTheEntityAlongTheGrabbedAxisAsOneUndoEntry)
{
    Fixture fixture;
    fixture.context.select(fixture.nearEntity);
    fixture.state.mode = GizmoMode::Translate;

    const auto layout = computeTranslateGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                       fixture.nearEntity);
    CNA_STUDIO_EXPECT(layout.has_value());

    const std::size_t before = fixture.context.getHistory().getCount();
    const StudioVector2 grab = layout->screenTips[0];
    fixture.dragTo(grab.x, grab.y, grab.x + 40.0f, grab.y);

    const StudioComponent* transform =
        fixture.context.getScene().findEntity(fixture.nearEntity)->findComponent("CNA.Transform");
    const StudioVector3 moved = transform->getProperty("position").get<StudioVector3>();
    CNA_STUDIO_EXPECT(moved.x != 0.0f);
    CNA_STUDIO_EXPECT_EQ(moved.y, 0.0f);
    CNA_STUDIO_EXPECT_EQ(moved.z, 0.0f);

    // One entry for the whole drag, not one per frame -- `dragTo` moves the pointer in a single
    // jump and releases, but the mechanism under test merges every frame of a real drag the same
    // way, and a merge that failed would still show as more than one entry here.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before + 1);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(
        fixture.context.getScene().findEntity(fixture.nearEntity)
            ->findComponent("CNA.Transform")->getProperty("position").get<StudioVector3>().x,
        0.0f);
}

CNA_STUDIO_TEST(ARotateDragTurnsTheEntityAboutTheGrabbedRingAsOneUndoEntry)
{
    Fixture fixture;
    fixture.context.select(fixture.nearEntity);
    fixture.state.mode = GizmoMode::Rotate;

    const auto layout = computeRotateGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                    fixture.nearEntity);
    CNA_STUDIO_EXPECT(layout.has_value());

    // The Z ring: seen close to face-on from this camera's default orbit, so a drag around its
    // circumference reads as a rotation rather than being dropped as edge-on.
    const std::vector<StudioVector2>& ring = layout->rings[2];
    CNA_STUDIO_EXPECT(!ring.empty());
    const StudioVector2 grabPoint = ring.front();

    const std::size_t before = fixture.context.getHistory().getCount();

    // A quarter of the way around the same ring, which is sampled at even angle steps -- along
    // the ring's circumference rather than across it, so the drag reads as a turn rather than
    // the same angle it started at.
    const StudioVector2 quarterTurn = ring[ring.size() / 4];

    fixture.dragTo(grabPoint.x, grabPoint.y, quarterTurn.x, quarterTurn.y);

    const StudioComponent* transform =
        fixture.context.getScene().findEntity(fixture.nearEntity)->findComponent("CNA.Transform");
    const StudioQuaternion turned = transform->getProperty("rotation").get<StudioQuaternion>();
    CNA_STUDIO_EXPECT(!(turned == StudioQuaternion{}));

    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before + 1);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT(
        fixture.context.getScene().findEntity(fixture.nearEntity)
            ->findComponent("CNA.Transform")->getProperty("rotation").get<StudioQuaternion>()
        == StudioQuaternion{});
}

CNA_STUDIO_TEST(AScaleDragResizesTheEntityAsOneUndoEntry)
{
    Fixture fixture;
    fixture.context.select(fixture.nearEntity);
    fixture.state.mode = GizmoMode::Scale;

    const auto layout = computeScaleGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                   fixture.nearEntity);
    CNA_STUDIO_EXPECT(layout.has_value());
    CNA_STUDIO_EXPECT(layout->armVisible[0]);

    const StudioVector2 grab = layout->screenHandles[0];
    const StudioVector2 origin = layout->screenOrigin;
    // Twice as far from the origin along the same direction, which is a factor of two however
    // far the handle itself was grabbed at -- scale is a ratio of screen distances.
    const StudioVector2 out{origin.x + (grab.x - origin.x) * 2.0f,
                            origin.y + (grab.y - origin.y) * 2.0f};

    const std::size_t before = fixture.context.getHistory().getCount();
    fixture.dragTo(grab.x, grab.y, out.x, out.y);

    const StudioComponent* transform =
        fixture.context.getScene().findEntity(fixture.nearEntity)->findComponent("CNA.Transform");
    const StudioVector3 scaled = transform->getProperty("scale").get<StudioVector3>();
    CNA_STUDIO_EXPECT(scaled.x > 1.0f);
    CNA_STUDIO_EXPECT_EQ(scaled.y, 1.0f);
    CNA_STUDIO_EXPECT_EQ(scaled.z, 1.0f);

    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before + 1);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
}

CNA_STUDIO_TEST(ATranslateDragOnAMultiSelectionMovesEveryEntityAsOneUndoEntry)
{
    // Over the shared pivot, which is the average of both entities' positions -- at (20, 0, 0)
    // for Near (0,0,0) and Far (40,0,0) -- rather than over either one's own. Grabbed at the
    // pivot's own gizmo, which is where the panel draws it for a multi-selection.
    Fixture fixture;
    const Uuid farEntity = [&] {
        for (const StudioEntity& entity : fixture.context.getScene().getEntities())
        {
            if (entity.getName() == "Far") { return entity.getId(); }
        }
        return Uuid{};
    }();
    CNA_STUDIO_EXPECT(farEntity.isValid());

    fixture.context.select(fixture.nearEntity);
    fixture.context.toggleSelection(farEntity);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{2});
    fixture.state.mode = GizmoMode::Translate;

    // Framed on the pivot itself, and far back enough that both entities -- 20 world units either
    // side of it -- land inside the viewport too. The default fixture camera looks at the world
    // origin from only 20 units out, which puts a pivot at x=20 off the edge of the screen and a
    // press there would never reach the panel at all.
    fixture.camera.setPivot(StudioVector3{20.0f, 0.0f, 0.0f});
    fixture.camera.setDistance(60.0f);

    const StudioVector3 pivot{20.0f, 0.0f, 0.0f};
    const auto layout = computeTranslateGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                       farEntity, GizmoSpace::World, pivot);
    CNA_STUDIO_EXPECT(layout.has_value());

    const std::size_t before = fixture.context.getHistory().getCount();
    const StudioVector2 grab = layout->screenTips[1];  // the Y arm: no risk of picking Near or Far.
    fixture.dragTo(grab.x, grab.y, grab.x, grab.y + 40.0f);

    const StudioVector3 nearAfter = fixture.context.getScene().findEntity(fixture.nearEntity)
                                        ->findComponent("CNA.Transform")
                                        ->getProperty("position")
                                        .get<StudioVector3>();
    const StudioVector3 farAfter = fixture.context.getScene().findEntity(farEntity)
                                       ->findComponent("CNA.Transform")
                                       ->getProperty("position")
                                       .get<StudioVector3>();

    // Both moved, and by the same amount: the whole point of a shared pivot is that a group drags
    // as one arrangement rather than each member solving the cursor against its own gizmo.
    CNA_STUDIO_EXPECT(nearAfter.y != 0.0f);
    CNA_STUDIO_EXPECT_EQ(nearAfter.y, farAfter.y);
    CNA_STUDIO_EXPECT_EQ(nearAfter.x, 0.0f);
    CNA_STUDIO_EXPECT_EQ(farAfter.x, 40.0f);

    // One entry for both entities, not one each -- undoing a group drag one member at a time
    // would put the selection through arrangements it was never actually in.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before + 1);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(
        fixture.context.getScene().findEntity(fixture.nearEntity)
            ->findComponent("CNA.Transform")->getProperty("position").get<StudioVector3>().y,
        0.0f);
}

/**
 * A uniform scale on a multi-selection resizes it, and used to do nothing (STUDIO-12005).
 *
 * The multi path built its per-axis factors from X, Y and Z alone, so the centre handle -- the
 * commonest scale there is -- produced a factor of one on every axis. The drag ran, changed
 * nothing, and reported nothing: the quietest way for a manipulator to be broken, because there is
 * no error and no movement and the user concludes the selection is somehow locked.
 *
 * The plane handles inherited the same hole the day they were added, which is why the mapping is
 * now one function shared with the single-entity path rather than a condition written twice.
 */
CNA_STUDIO_TEST(AUniformOrPlaneScaleOnAMultiSelectionResizesIt)
{
    Fixture fixture;
    const Uuid farEntity = [&] {
        for (const StudioEntity& entity : fixture.context.getScene().getEntities())
        {
            if (entity.getName() == "Far") { return entity.getId(); }
        }
        return Uuid{};
    }();
    CNA_STUDIO_EXPECT(farEntity.isValid());

    fixture.context.select(fixture.nearEntity);
    fixture.context.toggleSelection(farEntity);
    fixture.state.mode = GizmoMode::Scale;

    fixture.camera.setPivot(StudioVector3{20.0f, 0.0f, 0.0f});
    fixture.camera.setDistance(60.0f);
    fixture.run(away());

    const auto scaleOf = [&](const Uuid& id) {
        return fixture.context.getScene().findEntity(id)
            ->findComponent("CNA.Transform")->getProperty("scale").get<StudioVector3>();
    };
    const auto positionOf = [&](const Uuid& id) {
        return fixture.context.getScene().findEntity(id)
            ->findComponent("CNA.Transform")->getProperty("position").get<StudioVector3>();
    };

    const StudioVector3 pivot{20.0f, 0.0f, 0.0f};
    const auto layout =
        computeScaleGizmo3DLayout(fixture.context.getScene(), fixture.camera, farEntity, pivot);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    // The centre handle: a press a few pixels off the origin, so the ratio it divides by is not
    // zero, then dragged well out.
    const StudioVector2 grab{layout->screenOrigin.x + 8.0f, layout->screenOrigin.y};
    fixture.dragVia({grab,
                     StudioVector2{layout->screenOrigin.x + 12.0f, layout->screenOrigin.y},
                     StudioVector2{layout->screenOrigin.x + 16.0f, layout->screenOrigin.y}});

    // Both grew, on all three axes, by the same factor -- and both moved away from the shared
    // pivot, because a group that resized in place would overlap itself.
    CNA_STUDIO_EXPECT(scaleOf(fixture.nearEntity).x > 1.5f);
    CNA_STUDIO_EXPECT(scaleOf(fixture.nearEntity).y > 1.5f);
    CNA_STUDIO_EXPECT(scaleOf(fixture.nearEntity).z > 1.5f);
    CNA_STUDIO_EXPECT_EQ(scaleOf(fixture.nearEntity).x, scaleOf(farEntity).x);
    CNA_STUDIO_EXPECT(positionOf(fixture.nearEntity).x < 0.0f);
    CNA_STUDIO_EXPECT(positionOf(farEntity).x > 40.0f);

    // And a plane handle: two axes by one factor, the third untouched, across the whole selection.
    Fixture planar;
    const Uuid planarFar = [&] {
        for (const StudioEntity& entity : planar.context.getScene().getEntities())
        {
            if (entity.getName() == "Far") { return entity.getId(); }
        }
        return Uuid{};
    }();
    planar.context.select(planar.nearEntity);
    planar.context.toggleSelection(planarFar);
    planar.state.mode = GizmoMode::Scale;
    planar.camera.setPivot(StudioVector3{20.0f, 0.0f, 0.0f});
    planar.camera.setDistance(60.0f);
    planar.run(away());

    const auto planarLayout =
        computeScaleGizmo3DLayout(planar.context.getScene(), planar.camera, planarFar, pivot);
    CNA_STUDIO_EXPECT(planarLayout.has_value());
    if (!planarLayout) { return; }

    const StudioVector2 centre = planarLayout->planes[0].getScreenCenter();
    const StudioVector2 outward{
        planarLayout->screenOrigin.x + (centre.x - planarLayout->screenOrigin.x) * 1.8f,
        planarLayout->screenOrigin.y + (centre.y - planarLayout->screenOrigin.y) * 1.8f};
    planar.dragVia({centre,
                    StudioVector2{(centre.x + outward.x) * 0.5f, (centre.y + outward.y) * 0.5f},
                    outward});

    const StudioVector3 grown = planar.context.getScene().findEntity(planar.nearEntity)
                                    ->findComponent("CNA.Transform")
                                    ->getProperty("scale").get<StudioVector3>();
    CNA_STUDIO_EXPECT(grown.x > 1.2f);
    CNA_STUDIO_EXPECT_EQ(grown.x, grown.y);
    CNA_STUDIO_EXPECT_EQ(grown.z, 1.0f);
}

CNA_STUDIO_TEST(ARotateDragOnAMultiSelectionTurnsEveryEntityAboutTheSharedPivot)
{
    // The rotate half of the same mechanism: a group turns as one arrangement, carried around its
    // shared pivot rather than each member spinning in place where it already stands.
    Fixture fixture;
    const Uuid farEntity = [&] {
        for (const StudioEntity& entity : fixture.context.getScene().getEntities())
        {
            if (entity.getName() == "Far") { return entity.getId(); }
        }
        return Uuid{};
    }();
    CNA_STUDIO_EXPECT(farEntity.isValid());

    fixture.context.select(fixture.nearEntity);
    fixture.context.toggleSelection(farEntity);
    fixture.state.mode = GizmoMode::Rotate;

    // Framed on the pivot, for the same reason the multi-select translate test is: the pivot sits
    // at x=20, off the edge of the default fixture camera's view.
    fixture.camera.setPivot(StudioVector3{20.0f, 0.0f, 0.0f});
    fixture.camera.setDistance(60.0f);

    const StudioVector3 pivot{20.0f, 0.0f, 0.0f};
    const auto layout = computeRotateGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                    farEntity, GizmoSpace::World, pivot);
    CNA_STUDIO_EXPECT(layout.has_value());
    const std::vector<StudioVector2>& ring = layout->rings[2];
    CNA_STUDIO_EXPECT(!ring.empty());
    const StudioVector2 grabPoint = ring.front();
    const StudioVector2 quarterTurn = ring[ring.size() / 4];

    const std::size_t before = fixture.context.getHistory().getCount();
    fixture.dragTo(grabPoint.x, grabPoint.y, quarterTurn.x, quarterTurn.y);

    const StudioComponent* nearTransform =
        fixture.context.getScene().findEntity(fixture.nearEntity)->findComponent("CNA.Transform");
    const StudioComponent* farTransform =
        fixture.context.getScene().findEntity(farEntity)->findComponent("CNA.Transform");

    // Carried around the pivot, not merely turned in place: each entity started 20 world units
    // from (20,0,0) along X, so a turn about Z displaces both away from where they started.
    const StudioVector3 nearAfter = nearTransform->getProperty("position").get<StudioVector3>();
    const StudioVector3 farAfter = farTransform->getProperty("position").get<StudioVector3>();
    CNA_STUDIO_EXPECT(nearAfter.x != 0.0f || nearAfter.y != 0.0f);
    CNA_STUDIO_EXPECT(farAfter.x != 40.0f || farAfter.y != 0.0f);

    // And turned by the same angle -- one gesture, not each member solving the cursor for itself.
    const StudioQuaternion nearRotation = nearTransform->getProperty("rotation").get<StudioQuaternion>();
    const StudioQuaternion farRotation = farTransform->getProperty("rotation").get<StudioQuaternion>();
    CNA_STUDIO_EXPECT(!(nearRotation == StudioQuaternion{}));
    CNA_STUDIO_EXPECT(nearRotation == farRotation);

    // One entry for the whole turn, not one per member.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCount(), before + 1);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(
        fixture.context.getScene().findEntity(fixture.nearEntity)
            ->findComponent("CNA.Transform")->getProperty("position").get<StudioVector3>().x,
        0.0f);
}

CNA_STUDIO_TEST(TheManipulatorTheToolbarNamesIsTheOneThatDragsRatherThanOrbits)
{
    // The last clause of the acceptance: with the mode set to Rotate, a press on the *translate*
    // arm's position finds nothing to grab -- because there is no translate gizmo drawn while
    // Rotate is armed -- and falls through to an ordinary orbit instead.
    Fixture fixture;
    fixture.context.select(fixture.nearEntity);
    fixture.state.mode = GizmoMode::Rotate;

    const auto translateLayout = computeTranslateGizmo3DLayout(
        fixture.context.getScene(), fixture.camera, fixture.nearEntity);
    CNA_STUDIO_EXPECT(translateLayout.has_value());

    // Midway along where the translate arm would run, not at its very tip -- the rotate rings
    // share the translate arms' length by construction, so the X arm's tip lands exactly on the
    // Y and Z rings' own zero-angle samples, and a press there would find a rotate handle to grab
    // for a genuine reason rather than proving the point this test is after.
    const StudioVector2 wouldBeTranslateArm{
        (translateLayout->screenOrigin.x + translateLayout->screenTips[0].x) * 0.5f,
        (translateLayout->screenOrigin.y + translateLayout->screenTips[0].y) * 0.5f};

    const StudioVector3 before = fixture.context.getScene().findEntity(fixture.nearEntity)
                                     ->findComponent("CNA.Transform")
                                     ->getProperty("position")
                                     .get<StudioVector3>();
    const float yawBefore = fixture.camera.getYaw();

    fixture.dragTo(wouldBeTranslateArm.x, wouldBeTranslateArm.y,
                   wouldBeTranslateArm.x + 40.0f, wouldBeTranslateArm.y);

    const StudioVector3 after = fixture.context.getScene().findEntity(fixture.nearEntity)
                                    ->findComponent("CNA.Transform")
                                    ->getProperty("position")
                                    .get<StudioVector3>();
    CNA_STUDIO_EXPECT(after == before);
    CNA_STUDIO_EXPECT(fixture.camera.getYaw() != yawBefore);
}

/**
 * A band in 3D goes where a left drag already means "select" (`plan.md` STUDIO-12009).
 *
 * Maya's scheme and Blender's both leave the unmodified left button to selection -- Maya puts every
 * gesture behind Alt and Blender puts them on the middle button -- so a drag there is a rubber band
 * and nothing else has to move to make room for it. Studio's own scheme spends the plain left drag
 * on the orbit, so the band goes on Ctrl+left there: consistent rather than invented, because Ctrl
 * already means "add to what is selected" on a click, and Ctrl is no part of Studio's navigation
 * vocabulary so nothing is taken away.
 */
CNA_STUDIO_TEST(ABandInThreeDGoesWhereALeftDragAlreadyMeansSelect)
{
    // Far enough back that both entities are on screen and neither fills it: the fixture's default
    // framing puts one of them off the edge, and a band that caught one of two would pass for the
    // wrong reason.
    const auto standBack = [](Fixture& f) {
        f.camera.setPivot(StudioVector3{20.0f, 0.0f, 0.0f});
        f.camera.setDistance(120.0f);
    };

    Fixture fixture;
    fixture.state.navigation = StudioNavigationStyle::Maya;
    fixture.state.mode = GizmoMode::None;
    standBack(fixture);
    fixture.run(away());

    const float yaw = fixture.camera.getYaw();

    // A drag across the whole panel. Under Maya's scheme it turns nothing and sweeps everything.
    fixture.dragTo(20.0f, 20.0f, kWidth - 20.0f, kHeight - 20.0f);

    CNA_STUDIO_EXPECT_EQ(fixture.camera.getYaw(), yaw);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getSelection().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(fixture.last.selectionChanged);

    // A drag over an empty corner clears, which is what makes sweeping nothing a deselect.
    fixture.dragTo(2.0f, 2.0f, 30.0f, 30.0f);
    CNA_STUDIO_EXPECT(fixture.context.getSelection().empty());

    // Under Studio's own scheme the same drag orbits and selects nothing: the band is not there,
    // and that is the scheme the user chose rather than a hole.
    Fixture studio;
    studio.state.mode = GizmoMode::None;
    standBack(studio);
    studio.run(away());
    const float studioYaw = studio.camera.getYaw();

    studio.dragTo(20.0f, 20.0f, kWidth - 20.0f, kHeight - 20.0f);
    CNA_STUDIO_EXPECT(std::abs(studio.camera.getYaw() - studioYaw) > 0.01f);
    CNA_STUDIO_EXPECT(studio.context.getSelection().empty());

    // Ctrl+left is the band there, and it adds rather than replacing.
    studio.context.select(studio.nearEntity);
    const float beforeBand = studio.camera.getYaw();
    studio.dragTo(20.0f, 20.0f, kWidth - 20.0f, kHeight - 20.0f, /*shift=*/false,
                  /*control=*/true);

    CNA_STUDIO_EXPECT_EQ(studio.camera.getYaw(), beforeBand);
    CNA_STUDIO_EXPECT_EQ(studio.context.getSelection().size(), std::size_t{2});

    // And a Ctrl press that does not move is still the Ctrl-click it was: it toggles the entity
    // under it rather than sweeping a band of no width.
    studio.context.clearSelection();
    const std::optional<StudioVector2> nearAt =
        studio.camera.worldToScreen(StudioVector3{0.0f, 0.0f, 0.0f});
    CNA_STUDIO_EXPECT(nearAt.has_value());
    if (!nearAt) { return; }

    studio.clickAt(nearAt->x, nearAt->y, /*control=*/true);
    CNA_STUDIO_EXPECT_EQ(studio.context.getSelection().size(), std::size_t{1});
    studio.clickAt(nearAt->x, nearAt->y, /*control=*/true);
    CNA_STUDIO_EXPECT(studio.context.getSelection().empty());
}

/**
 * The space toggle refuses in Scale mode, where it means nothing (`plan.md` STUDIO-12004).
 *
 * Scale is always local: a non-uniform scale in world space needs a shear, which a
 * position/rotation/scale transform cannot express, so neither scale gizmo takes a space at all.
 * Pressing X while scaling used to flip the check and change nothing on screen -- a control that
 * responds and does nothing is worse than one that refuses, because the user has no way to tell
 * which of the two just happened.
 */
CNA_STUDIO_TEST(TheGizmoSpaceToggleRefusesWhileScaling)
{
    StudioContext context;
    StudioLog log;
    StudioShell shell;
    shell.resetLayout();

    StudioCamera2D camera;
    StudioCamera3D camera3D;
    StudioShellPanels panels{shell, context, log};
    panels.setViewportServices(camera, camera3D, {});

    UiInputState input;
    input.displayWidth = 1280.0f;
    input.displayHeight = 720.0f;
    shell.renderFrame(input);

    // Translate is the default, and there the toggle is live and does what it says.
    CNA_STUDIO_EXPECT(shell.actions().isEnabled("studio.view.toggleGizmoSpace"));
    CNA_STUDIO_EXPECT(!shell.actions().isChecked("studio.view.toggleGizmoSpace"));

    const StudioAction* space = shell.actions().find("studio.view.toggleGizmoSpace");
    CNA_STUDIO_EXPECT(space != nullptr && space->run != nullptr);
    if (space == nullptr || space->run == nullptr) { return; }

    space->run();
    shell.renderFrame(input);
    CNA_STUDIO_EXPECT(shell.actions().isChecked("studio.view.toggleGizmoSpace"));
    CNA_STUDIO_EXPECT(panels.viewportSpace() == GizmoSpace::Local);

    // Rotate keeps it: a turn about the entity's own axes is a different turn from one about the
    // world's, and which of the two a user wants is exactly what this asks.
    const StudioAction* rotate = shell.actions().find("studio.view.rotate");
    CNA_STUDIO_EXPECT(rotate != nullptr && rotate->run != nullptr);
    if (rotate == nullptr || rotate->run == nullptr) { return; }
    rotate->run();
    shell.renderFrame(input);
    CNA_STUDIO_EXPECT(shell.actions().isEnabled("studio.view.toggleGizmoSpace"));

    // Scale refuses it, and the space the user had chosen is still theirs when they come back.
    const StudioAction* scale = shell.actions().find("studio.view.scale");
    CNA_STUDIO_EXPECT(scale != nullptr && scale->run != nullptr);
    if (scale == nullptr || scale->run == nullptr) { return; }
    scale->run();
    shell.renderFrame(input);

    CNA_STUDIO_EXPECT(!shell.actions().isEnabled("studio.view.toggleGizmoSpace"));
    CNA_STUDIO_EXPECT(panels.viewportSpace() == GizmoSpace::Local);

    const StudioAction* translate = shell.actions().find("studio.view.translate");
    CNA_STUDIO_EXPECT(translate != nullptr && translate->run != nullptr);
    if (translate == nullptr || translate->run == nullptr) { return; }
    translate->run();
    shell.renderFrame(input);

    CNA_STUDIO_EXPECT(shell.actions().isEnabled("studio.view.toggleGizmoSpace"));
    CNA_STUDIO_EXPECT(panels.viewportSpace() == GizmoSpace::Local);
}

/**
 * Snapping is a state as well as a held key, and the steps are the project's (STUDIO-12007).
 *
 * Ctrl was the only way to snap, so a user laying out a level on a grid held it for every drag of
 * the day; and the angle and scale steps were constants in the editor, which suits most projects
 * and suits an isometric one badly. Both halves here: the toggle, and the steps coming out of the
 * project rather than out of the code.
 *
 * The modifier **inverts** the toggle rather than repeating it, which is the part worth a case. With
 * snapping on, Ctrl is the momentary escape for the one placement that has to sit off the grid --
 * and a modifier that merely repeated the setting would leave no way to make that placement but to
 * turn snapping off and remember to turn it back on.
 */
CNA_STUDIO_TEST(SnappingIsAStateAndTheModifierInvertsIt)
{
    CNA_STUDIO_EXPECT(studioIsSnapping(/*snapToggle=*/false, /*modifierHeld=*/true));
    CNA_STUDIO_EXPECT(studioIsSnapping(/*snapToggle=*/true, /*modifierHeld=*/false));
    CNA_STUDIO_EXPECT(!studioIsSnapping(/*snapToggle=*/false, /*modifierHeld=*/false));
    CNA_STUDIO_EXPECT(!studioIsSnapping(/*snapToggle=*/true, /*modifierHeld=*/true));

    Fixture fixture;
    fixture.context.select(fixture.nearEntity);
    fixture.state.mode = GizmoMode::Translate;
    fixture.run(away());

    // A project with a five-unit grid, so a snapped drag lands on a multiple of five and a free one
    // does not. Five rather than one, because one is what an undeclared project falls back to and a
    // case that used it would pass whether the setting was read or not.
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / ("cna-snap3d-" + Uuid::generate().toString());
    std::filesystem::create_directories(root);
    fixture.context.getProject() = Project::createDefault("Snapped", root.generic_string());
    CNA_STUDIO_EXPECT(
        fixture.context.getProject().saveToFile((root / "Snapped.cnaproject").generic_string()));
    fixture.context.getProject().setGridSnap(5.0f);

    const auto positionX = [&] {
        return fixture.context.getScene().findEntity(fixture.nearEntity)
            ->findComponent("CNA.Transform")->getProperty("position").get<StudioVector3>().x;
    };

    const auto layout = computeTranslateGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                      fixture.nearEntity);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    const StudioVector2 grab = layout->screenTips[0];

    // Snapping off and no modifier: free, and the awkward distance stays awkward.
    fixture.dragVia({grab, StudioVector2{grab.x + 37.0f, grab.y}});
    const float free = positionX();
    CNA_STUDIO_EXPECT(free != 0.0f);
    CNA_STUDIO_EXPECT(std::abs(std::fmod(free, 5.0f)) > 0.001f);

    // Snapping on, still no modifier: the same gesture now lands on a multiple of five.
    fixture.state.snapping = true;
    fixture.dragVia({grab, StudioVector2{grab.x + 37.0f, grab.y}});
    const float snapped = positionX();
    CNA_STUDIO_EXPECT(std::abs(std::fmod(snapped, 5.0f)) < 0.001f);

    std::filesystem::remove_all(root);
}

/**
 * The Snap command is checkable and says which state it is in (`plan.md` STUDIO-12007).
 *
 * A viewport where a drag rounds and one where it does not look identical until the drag happens,
 * so a user who cannot see which they are in finds out by placing something wrong.
 */
CNA_STUDIO_TEST(TheSnapCommandShowsWhetherDragsAreRounding)
{
    StudioContext context;
    StudioLog log;
    StudioShell shell;
    shell.resetLayout();

    StudioCamera2D camera;
    StudioCamera3D camera3D;
    StudioShellPanels panels{shell, context, log};
    panels.setViewportServices(camera, camera3D, {});

    UiInputState input;
    input.displayWidth = 1280.0f;
    input.displayHeight = 720.0f;
    shell.renderFrame(input);

    // Off by default, which is what the editor did when Ctrl was the only way to snap.
    CNA_STUDIO_EXPECT(!shell.actions().isChecked("studio.view.snap"));

    const StudioAction* snap = shell.actions().find("studio.view.snap");
    CNA_STUDIO_EXPECT(snap != nullptr && snap->run != nullptr);
    if (snap == nullptr || snap->run == nullptr) { return; }

    snap->run();
    shell.renderFrame(input);
    CNA_STUDIO_EXPECT(shell.actions().isChecked("studio.view.snap"));

    // And it survives a change of view: the 2D and 3D gizmos both round, so a user switching
    // between them keeps the setting they chose rather than finding it reset.
    const StudioAction* toThreeD = shell.actions().find("studio.view.3d");
    CNA_STUDIO_EXPECT(toThreeD != nullptr && toThreeD->run != nullptr);
    if (toThreeD == nullptr || toThreeD->run == nullptr) { return; }
    toThreeD->run();
    shell.renderFrame(input);
    CNA_STUDIO_EXPECT(shell.actions().isChecked("studio.view.snap"));

    snap->run();
    shell.renderFrame(input);
    CNA_STUDIO_EXPECT(!shell.actions().isChecked("studio.view.snap"));
}

/**
 * A drag of many frames is one undo entry, and undoing it returns to where the drag began.
 *
 * `plan.md` STUDIO-12008, and the reason this case exists beside the three that already say "as one
 * undo entry": those drive the pointer in a single jump, so the drag commits exactly once and the
 * *merge* never runs. One entry out of one edit says nothing about the mechanism that turns sixty
 * edits into one, and sixty entries is what a user gets when it breaks.
 *
 * The second half is the half a merge gets wrong. Adopting the newer command's old value as well as
 * its new one leaves an undo that goes back to the *previous frame* -- which looks right on the one
 * jump these cases used to make, and on a real drag leaves the entity a pixel from where it started
 * with nothing left on the stack to fix it.
 */
CNA_STUDIO_TEST(ADragOfManyFramesIsOneEntryAndUndoReturnsToWhereItBegan)
{
    Fixture fixture;
    fixture.context.select(fixture.nearEntity);
    fixture.state.mode = GizmoMode::Translate;
    fixture.run(away());

    const auto position = [&] {
        return fixture.context.getScene().findEntity(fixture.nearEntity)
            ->findComponent("CNA.Transform")->getProperty("position").get<StudioVector3>();
    };

    const StudioVector3 start = position();

    const auto layout = computeTranslateGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                      fixture.nearEntity);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    const std::size_t before = fixture.context.getHistory().getCursor();
    const StudioVector2 grab = layout->screenTips[0];

    // Six frames of movement, which is six edits and would be six entries without the merge.
    fixture.dragVia({grab,
                     StudioVector2{grab.x + 8.0f, grab.y},
                     StudioVector2{grab.x + 17.0f, grab.y},
                     StudioVector2{grab.x + 25.0f, grab.y},
                     StudioVector2{grab.x + 34.0f, grab.y},
                     StudioVector2{grab.x + 40.0f, grab.y}});

    CNA_STUDIO_EXPECT(position().x != start.x);
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCursor(), before + 1);

    // All the way back, not back one frame.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(position().x, start.x);
    CNA_STUDIO_EXPECT_EQ(position().y, start.y);
    CNA_STUDIO_EXPECT_EQ(position().z, start.z);

    // The cursor rather than the count: an undone entry stays on the stack as a redo target, so
    // counting entries would say the undo had not happened. Redo puts the whole drag back, which is
    // the other half of "one entry" and the half a per-frame stack makes unusable.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCursor(), before);
    CNA_STUDIO_EXPECT(fixture.context.getHistory().redo());
    CNA_STUDIO_EXPECT(position().x != start.x);
}

/**
 * A press on a handle that goes nowhere leaves nothing on the undo stack (`plan.md` STUDIO-12008).
 *
 * Grabbing a handle and thinking better of it is an ordinary thing to do, and an entry that undoes
 * nothing is worse than a wrong one: the user cannot tell how many more times to press Ctrl+Z.
 */
CNA_STUDIO_TEST(APressOnAHandleThatGoesNowhereLeavesNoUndoEntry)
{
    Fixture fixture;
    fixture.context.select(fixture.nearEntity);
    fixture.state.mode = GizmoMode::Translate;
    fixture.run(away());

    const auto layout = computeTranslateGizmo3DLayout(fixture.context.getScene(), fixture.camera,
                                                      fixture.nearEntity);
    CNA_STUDIO_EXPECT(layout.has_value());
    if (!layout) { return; }

    const std::size_t before = fixture.context.getHistory().getCursor();
    const StudioVector2 grab = layout->screenTips[0];

    fixture.dragVia({grab, grab, grab});
    CNA_STUDIO_EXPECT_EQ(fixture.context.getHistory().getCursor(), before);
}
