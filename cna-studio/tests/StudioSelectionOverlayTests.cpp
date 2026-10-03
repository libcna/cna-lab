// SPDX-License-Identifier: MS-PL
/**
 * @file StudioSelectionOverlayTests.cpp
 * @brief The viewport says what is selected, and where it turns (`plan.md` CORE-03).
 *
 * ### What was already true
 *
 * A selected entity has been outlined since the viewport was written. What CORE-03 adds is
 * everything a *multi-selection* needs: eight selected crates were eight identical boxes with
 * nothing saying they were one selection, and nothing at all marking the point a rotation would
 * happen about — which is the one thing a user has to know before they press R.
 *
 * ### Why the geometry rather than the pixels
 *
 * The marks reach the screen through CNA's sprite batch in 2D and a wire list in 3D, on a machine
 * with a GPU. *Where they go* is computed in `cna-studio-scene`, which is what makes the
 * interesting half checkable here: does the pivot follow the gizmo, does the combined box cover
 * every selected entity, does a single selection correctly get no combined box at all. The
 * capture cases in `tests/CMakeLists.txt` cover the other half — that something is drawn.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/Scene/SceneSelectionOverlay.hpp"
#include "CNA/Studio/Scene/SceneWireframe.hpp"
#include "CNA/Studio/Scene/StudioCamera3D.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    ComponentRegistry makeRegistry()
    {
        ComponentRegistry registry;
        registerBuiltinComponents(registry);
        return registry;
    }

    /** @brief An entity with a transform at @p x, @p y and nothing else. */
    StudioEntity makeEntity(const ComponentRegistry& registry, std::string name, float x, float y)
    {
        StudioEntity entity{Uuid::generate(), std::move(name)};
        StudioComponent transform{BuiltinComponentIds::kTransform};
        transform.applyDefaults(*registry.find(BuiltinComponentIds::kTransform));
        transform.setProperty("position", PropertyValue{StudioVector3{x, y, 0.0f}});
        entity.addComponent(std::move(transform));
        return entity;
    }

    /** @brief A sprite entity at @p x, @p y, so it has bounds the overlay can measure. */
    Uuid addSprite(SceneDocument& scene, const ComponentRegistry& registry, const std::string& name,
                   float x, float y)
    {
        StudioEntity entity = makeEntity(registry, name, x, y);
        StudioComponent renderer{BuiltinComponentIds::kSpriteRenderer};
        renderer.applyDefaults(*registry.find(BuiltinComponentIds::kSpriteRenderer));
        entity.addComponent(std::move(renderer));
        return scene.addEntity(std::move(entity));
    }

    /** @brief Every sprite is 32 by 32, so the arithmetic below is checkable by hand. */
    const SpriteSizeProvider& sizes()
    {
        static const SpriteSizeProvider provider = [](const Uuid&) {
            return StudioVector2{32.0f, 32.0f};
        };
        return provider;
    }

    StudioCamera3D makeCamera()
    {
        StudioCamera3D camera;
        camera.setViewportSize(StudioVector2{1600.0f, 900.0f});
        camera.setPivot(StudioVector3{});
        camera.setDistance(400.0f);
        camera.setPitch(0.5f);
        return camera;
    }

    std::size_t countColoured(const WireframeResult& result, const StudioColor& colour)
    {
        return static_cast<std::size_t>(
            std::count_if(result.segments.begin(), result.segments.end(),
                          [&colour](const WireSegment& segment) { return segment.color == colour; }));
    }
}

CNA_STUDIO_TEST(NothingIsSelectedAndNothingIsMarked)
{
    // The state Studio spends most of its time in. An overlay that drew a pivot for an empty
    // selection would put a cross at the world origin of every scene anybody opens.
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    (void)addSprite(scene, registry, "Alone", 0.0f, 0.0f);

    const StudioSelectionOverlay2D overlay = studioSelectionOverlay2D(scene, {}, sizes());
    CNA_STUDIO_EXPECT(overlay.isEmpty());
    CNA_STUDIO_EXPECT(overlay.outlines.empty());
    CNA_STUDIO_EXPECT(!overlay.combined.has_value());
    CNA_STUDIO_EXPECT(!overlay.pivot.has_value());

    CNA_STUDIO_EXPECT(studioSelectionOverlay3D(scene, {}, sizes()).isEmpty());
}

CNA_STUDIO_TEST(OneSelectedEntityIsOutlinedAndGetsAPivotButNoSecondBox)
{
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid id = addSprite(scene, registry, "Player", 10.0f, 20.0f);

    const StudioSelectionOverlay2D overlay = studioSelectionOverlay2D(scene, {id}, sizes());

    CNA_STUDIO_EXPECT_EQ(overlay.outlines.size(), std::size_t{1});

    // No combined box for one entity. It would sit exactly on that entity's own outline -- two
    // rectangles at the same place in two colours, which is a picture of nothing.
    CNA_STUDIO_EXPECT(!overlay.combined.has_value());

    // And a pivot, which is the half that was missing: the outline says *what*, and only the cross
    // says *where a rotation will happen*.
    CNA_STUDIO_EXPECT(overlay.pivot.has_value());
    if (!overlay.pivot) { return; }
    CNA_STUDIO_EXPECT(std::abs(overlay.pivot->x - 10.0f) < 0.001f);
    CNA_STUDIO_EXPECT(std::abs(overlay.pivot->y - 20.0f) < 0.001f);
}

CNA_STUDIO_TEST(AMultiSelectionGetsOneBoxRoundAllOfItAndAPivotBetweenThem)
{
    // The case CORE-03 exists for. Before it, selecting several entities produced several
    // identical outlines and nothing saying they were one selection.
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid left = addSprite(scene, registry, "Left", -100.0f, 0.0f);
    const Uuid right = addSprite(scene, registry, "Right", 100.0f, 40.0f);

    const StudioSelectionOverlay2D overlay = studioSelectionOverlay2D(scene, {left, right}, sizes());

    CNA_STUDIO_EXPECT_EQ(overlay.outlines.size(), std::size_t{2});
    CNA_STUDIO_EXPECT(overlay.combined.has_value());
    if (!overlay.combined) { return; }

    // Every selected entity inside the combined box, which is the only claim it makes. Checked by
    // containment rather than by an arithmetic total, so a change to how a sprite's extent is
    // measured does not break a case that is not about that.
    for (const WorldBounds2D& outline : overlay.outlines)
    {
        CNA_STUDIO_EXPECT(overlay.combined->contains(outline.min));
        CNA_STUDIO_EXPECT(overlay.combined->contains(outline.max));
    }

    // And strictly larger than either, or it is one of them wearing a different colour.
    CNA_STUDIO_EXPECT(overlay.combined->max.x - overlay.combined->min.x
                      > overlay.outlines[0].max.x - overlay.outlines[0].min.x);

    // The pivot is between them, which is what `Center` means and what the gizmo uses.
    CNA_STUDIO_EXPECT(overlay.pivot.has_value());
    if (!overlay.pivot) { return; }
    CNA_STUDIO_EXPECT(std::abs(overlay.pivot->x - 0.0f) < 0.001f);
    CNA_STUDIO_EXPECT(std::abs(overlay.pivot->y - 20.0f) < 0.001f);
}

CNA_STUDIO_TEST(ThePivotMarkLandsWhereTheGizmoTurnsUnderEitherPivotMode)
{
    // A mark that said the rotation would happen somewhere it does not is worse than no mark: it
    // is a wrong answer to the question the user asked by looking. So it is asked of the same
    // function the gizmo asks, and it has to follow the mode the gizmo follows.
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid first = addSprite(scene, registry, "First", -100.0f, 0.0f);
    const Uuid last = addSprite(scene, registry, "Last", 100.0f, 0.0f);

    const std::vector<Uuid> selection{first, last};

    const StudioSelectionOverlay2D centred =
        studioSelectionOverlay2D(scene, selection, sizes(), StudioPivotMode::Center);
    const StudioSelectionOverlay2D active =
        studioSelectionOverlay2D(scene, selection, sizes(), StudioPivotMode::Active);

    CNA_STUDIO_EXPECT(centred.pivot.has_value());
    CNA_STUDIO_EXPECT(active.pivot.has_value());
    if (!centred.pivot || !active.pivot) { return; }

    const StudioVector2 gizmoCentre =
        computeSelectionPivot(scene, selection, StudioPivotMode::Center).value();
    const StudioVector2 gizmoActive =
        computeSelectionPivot(scene, selection, StudioPivotMode::Active).value();

    CNA_STUDIO_EXPECT(std::abs(centred.pivot->x - gizmoCentre.x) < 0.001f);
    CNA_STUDIO_EXPECT(std::abs(centred.pivot->y - gizmoCentre.y) < 0.001f);
    CNA_STUDIO_EXPECT(std::abs(active.pivot->x - gizmoActive.x) < 0.001f);
    CNA_STUDIO_EXPECT(std::abs(active.pivot->y - gizmoActive.y) < 0.001f);

    // And the two modes really do differ here, or the case above holds for a selection where the
    // question has one answer and proves nothing.
    CNA_STUDIO_EXPECT(std::abs(centred.pivot->x - active.pivot->x) > 1.0f);
}

CNA_STUDIO_TEST(AnEntityWithNoBoundsIsNotOutlinedAndDoesNotWidenTheSelection)
{
    // A camera has no extent, so there is nothing to outline; the badge it gets instead is the
    // 2D renderer's business. What matters here is that it does not drag the combined box out to
    // the world origin and make a tight selection look like a loose one.
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid sprite = addSprite(scene, registry, "Sprite", 50.0f, 50.0f);

    StudioEntity camera = makeEntity(registry, "Camera", -500.0f, -500.0f);
    StudioComponent lens{BuiltinComponentIds::kCamera};
    lens.applyDefaults(*registry.find(BuiltinComponentIds::kCamera));
    camera.addComponent(std::move(lens));
    const Uuid cameraId = scene.addEntity(std::move(camera));

    const StudioSelectionOverlay2D overlay =
        studioSelectionOverlay2D(scene, {sprite, cameraId}, sizes());

    CNA_STUDIO_EXPECT_EQ(overlay.outlines.size(), std::size_t{1});

    // One entity has bounds, so there is nothing for a combined box to combine -- and a box drawn
    // on top of the single outline would be the two-rectangles-at-one-place picture again.
    CNA_STUDIO_EXPECT(!overlay.combined.has_value());

    // The camera still counts for the pivot: it has a transform, and it is something the user
    // selected and is about to move.
    CNA_STUDIO_EXPECT(overlay.pivot.has_value());
}

CNA_STUDIO_TEST(TheThreeDViewportDrawsTheSameTwoMarksAsTheTwoDOne)
{
    // A user who selects three entities in one view, switches to the other and finds the pivot
    // somewhere else has half a feature. Both viewports ask the same functions.
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid left = addSprite(scene, registry, "Left", -100.0f, 0.0f);
    const Uuid right = addSprite(scene, registry, "Right", 100.0f, 0.0f);

    const StudioCamera3D camera = makeCamera();

    WireframeOptions options;
    options.drawGrid = false;

    const WireframeResult marked =
        buildSceneWireframe(scene, camera, {left, right}, sizes(), options);

    // Twelve edges for the box round the whole selection, and three strokes for the pivot cross.
    CNA_STUDIO_EXPECT_EQ(countColoured(marked, WireColors::kSelectionExtent), std::size_t{12});
    CNA_STUDIO_EXPECT_EQ(countColoured(marked, WireColors::kSelectionPivot), std::size_t{3});

    // One entity gets a pivot and no extent box, exactly as in 2D.
    const WireframeResult single = buildSceneWireframe(scene, camera, {left}, sizes(), options);
    CNA_STUDIO_EXPECT_EQ(countColoured(single, WireColors::kSelectionExtent), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(countColoured(single, WireColors::kSelectionPivot), std::size_t{3});

    // And an empty selection gets neither, which is the state the viewport is usually in.
    const WireframeResult none = buildSceneWireframe(scene, camera, {}, sizes(), options);
    CNA_STUDIO_EXPECT_EQ(countColoured(none, WireColors::kSelectionExtent), std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(countColoured(none, WireColors::kSelectionPivot), std::size_t{0});

    // Declinable, for a caller that wants the scene's geometry and no editor furniture.
    WireframeOptions bare = options;
    bare.drawSelectionExtent = false;
    const WireframeResult plain = buildSceneWireframe(scene, camera, {left, right}, sizes(), bare);
    CNA_STUDIO_EXPECT_EQ(countColoured(plain, WireColors::kSelectionPivot), std::size_t{0});
}

CNA_STUDIO_TEST(ThePivotCrossIsTheSameSizeOnScreenAtEveryZoom)
{
    // A fixed world-space cross is a speck on a level and a cage round a crate, which are the two
    // views a user switches between while placing one.
    ComponentRegistry registry = makeRegistry();
    SceneDocument scene;
    const Uuid id = addSprite(scene, registry, "Player", 0.0f, 0.0f);

    WireframeOptions options;
    options.drawGrid = false;
    options.drawEntityBounds = true;

    const auto crossWidth = [&](float distance) {
        StudioCamera3D camera = makeCamera();
        camera.setDistance(distance);

        const WireframeResult result = buildSceneWireframe(scene, camera, {id}, sizes(), options);

        float widest = 0.0f;
        for (const WireSegment& segment : result.segments)
        {
            if (!(segment.color == WireColors::kSelectionPivot)) { continue; }
            widest = std::max(widest, std::abs(segment.to.x - segment.from.x));
        }
        return widest;
    };

    const float near = crossWidth(200.0f);
    const float far = crossWidth(2000.0f);

    CNA_STUDIO_EXPECT(near > 1.0f);
    CNA_STUDIO_EXPECT(far > 1.0f);

    // Within a pixel or two of each other at a tenfold change of distance. A world-space cross
    // would differ by that same factor of ten.
    CNA_STUDIO_EXPECT(std::abs(near - far) < 2.0f);
}
