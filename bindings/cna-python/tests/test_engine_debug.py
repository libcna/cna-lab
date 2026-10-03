"""Debug drawing, checked by counting lines rather than by looking at pixels.

Every shape's line count is exact and determined by its arguments, so what a
shape built can be asserted without a rasterizer deciding anything: a box is
twelve, a cross three, a sphere three rings of its clamped segment count.
:mod:`tests.engine_oracles` states each of those independently, and the vertices themselves are read back and
checked against the geometry they should have.

The drawer needs a device, so these need a build with the graphics extension
layer (``CNA_CNAEXT``); the draw itself needs a renderer and is separated out.
"""

from __future__ import annotations

import math
import unittest

from Microsoft.Xna.Framework import (
    BoundingBox, BoundingFrustum, BoundingSphere, Color, Matrix, Rectangle, Vector3,
)

from cna.extensions.engine import (
    AsciiEffect, AsciiQuantizeMode, DEBUG_DRAW_BOX_EDGE_COUNT, DEBUG_DRAW_DEFAULT_SEGMENTS,
    DEBUG_DRAW_MAXIMUM_SEGMENTS, DEBUG_DRAW_MINIMUM_SEGMENTS, DebugDraw, DebugLineVertex,
)
from cna.extensions.engine.errors import (
    EngineDisposedError, EngineError, EngineUnavailableError,
)

from . import engine_oracles as oracle
from .engine_fixtures import (
    ENGINE_PRESENT, in_game, requires_engine, requires_engine_gpu, requires_native,
)

PROJECTION = Matrix.CreatePerspectiveFieldOfView(0.9773843811168246,
                                                 1.7777777777777777, 0.35, 47.5)
VIEW = Matrix.CreateLookAt(Vector3(3.0, 2.0, 7.0), Vector3(-1.0, 0.5, -4.0),
                           Vector3(0.0, 1.0, 0.0))
#: A box that is neither a cube nor centred, so a corner read in the wrong order
#: gives different vertices.
BOX = BoundingBox(Vector3(-2.0, 0.0, 1.0), Vector3(6.0, 3.0, 9.0))
RED = Color(255, 0, 0, 255)
GREEN = Color(0, 255, 0, 255)


@requires_engine
class DebugDrawShapeTests(unittest.TestCase):
    """What each shape builds, counted and read back."""

    def _drawn(self, body):
        def run(game, device, observed):
            with DebugDraw(device) as debug:
                debug.begin(VIEW, PROJECTION)
                body(debug, device, observed)

        return in_game(run)

    def test_a_new_batch_holds_nothing_and_is_depth_tested(self) -> None:
        observed = self._drawn(lambda debug, device, out: out.update(
            count=debug.line_count, depth=debug.depth_tested,
            depth_vertices=debug.vertices(True), overlay=debug.vertices(False)))
        self.assertEqual(observed["count"], 0)
        self.assertTrue(observed["depth"])
        self.assertEqual(observed["depth_vertices"], ())
        self.assertEqual(observed["overlay"], ())

    def test_one_line_is_two_vertices_with_the_colour_it_was_given(self) -> None:
        def body(debug, device, out):
            debug.add_line(Vector3(1.0, 2.0, 3.0), Vector3(-4.0, 5.0, -6.0), RED)
            out.update(count=debug.line_count, vertices=debug.vertices(True))

        observed = self._drawn(body)
        self.assertEqual(observed["count"], 1)
        vertices = observed["vertices"]
        self.assertEqual(len(vertices), 2)
        self.assertEqual(tuple(vertices[0].position), (1.0, 2.0, 3.0))
        self.assertEqual(tuple(vertices[1].position), (-4.0, 5.0, -6.0))
        for vertex in vertices:
            self.assertEqual((vertex.color.R, vertex.color.G, vertex.color.B,
                              vertex.color.A), (255, 0, 0, 255))

    def test_a_box_is_its_twelve_edges_over_its_own_eight_corners(self) -> None:
        def body(debug, device, out):
            debug.add_box(BOX, RED)
            out.update(count=debug.line_count, vertices=debug.vertices(True))

        observed = self._drawn(body)
        self.assertEqual(observed["count"], DEBUG_DRAW_BOX_EDGE_COUNT)
        self.assertEqual(len(observed["vertices"]), DEBUG_DRAW_BOX_EDGE_COUNT * 2)
        # Every endpoint is a corner of the box, and all eight appear.
        corners = {tuple(corner) for corner in BOX.GetCorners()}
        seen = {tuple(vertex.position) for vertex in observed["vertices"]}
        self.assertEqual(seen, corners)
        # And each edge joins two corners differing on exactly one axis.
        pairs = list(zip(observed["vertices"][::2], observed["vertices"][1::2]))
        for start, end in pairs:
            differing = sum(1 for axis in "XYZ"
                            if getattr(start.position, axis)
                            != getattr(end.position, axis))
            self.assertEqual(differing, 1)

    def test_a_cross_is_three_segments_through_its_point(self) -> None:
        def body(debug, device, out):
            debug.add_cross(Vector3(1.0, -2.0, 3.0), 0.5, RED)
            out.update(count=debug.line_count, vertices=debug.vertices(True))

        observed = self._drawn(body)
        self.assertEqual(observed["count"], oracle.debug_cross_lines())
        pairs = list(zip(observed["vertices"][::2], observed["vertices"][1::2]))
        for index, axis in enumerate("XYZ"):
            start, end = pairs[index]
            self.assertAlmostEqual(getattr(end.position, axis)
                                   - getattr(start.position, axis), 1.0, places=5)
            for other in "XYZ":
                if other != axis:
                    self.assertEqual(getattr(start.position, other),
                                     getattr(end.position, other))

    def test_a_sphere_is_three_rings_of_the_clamped_segment_count(self) -> None:
        for requested in (0, 4, 8, 24, 128, 1000):
            def body(debug, device, out, requested=requested):
                debug.add_sphere(Vector3(0.0, 0.0, 0.0), 2.0, RED, requested)
                out.update(count=debug.line_count, vertices=debug.vertices(True))

            observed = self._drawn(body)
            with self.subTest(segments=requested):
                self.assertEqual(observed["count"],
                                 oracle.debug_sphere_lines(requested))
                # And every point is on the sphere, which a ring on the wrong
                # axis pair would not be.
                for vertex in observed["vertices"]:
                    position = vertex.position
                    self.assertAlmostEqual(
                        math.sqrt(position.X ** 2 + position.Y ** 2
                                  + position.Z ** 2), 2.0, places=4)

    def test_a_bounding_sphere_draws_the_same_rings(self) -> None:
        def body(debug, device, out):
            debug.add_sphere(Vector3(1.0, 2.0, 3.0), 2.0, RED, 8)
            first = debug.vertices(True)
            debug.clear()
            debug.add_bounding_sphere(
                BoundingSphere(Vector3(1.0, 2.0, 3.0), 2.0), RED, 8)
            out.update(first=first, second=debug.vertices(True))

        observed = self._drawn(body)
        self.assertEqual(len(observed["first"]), len(observed["second"]))
        for a, b in zip(observed["first"], observed["second"]):
            self.assertEqual(tuple(a.position), tuple(b.position))

    def test_a_frustum_is_twelve_edges_over_its_own_corners(self) -> None:
        frustum = BoundingFrustum(Matrix.Multiply(VIEW, PROJECTION))

        def body(debug, device, out):
            debug.add_frustum(frustum, GREEN)
            out.update(count=debug.line_count, vertices=debug.vertices(True))

        observed = self._drawn(body)
        self.assertEqual(observed["count"], DEBUG_DRAW_BOX_EDGE_COUNT)
        corners = [tuple(round(value, 3) for value in corner)
                   for corner in frustum.GetCorners()]
        for vertex in observed["vertices"]:
            self.assertIn(tuple(round(value, 3) for value in vertex.position), corners)

    def test_the_two_lists_are_separate_and_chosen_when_a_line_is_added(self) -> None:
        def body(debug, device, out):
            debug.add_line(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), RED)
            debug.depth_tested = False
            debug.add_line(Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 0.0), GREEN)
            debug.add_cross(Vector3(0.0, 0.0, 0.0), 1.0, GREEN)
            out.update(total=debug.line_count, depth=debug.vertices(True),
                       overlay=debug.vertices(False), flag=debug.depth_tested)

        observed = self._drawn(body)
        self.assertFalse(observed["flag"])
        self.assertEqual(len(observed["depth"]), 2)
        self.assertEqual(len(observed["overlay"]), 2 + 6)
        self.assertEqual(observed["total"], 1 + 1 + 3)

    def test_clearing_drops_both_lists(self) -> None:
        def body(debug, device, out):
            debug.add_box(BOX, RED)
            debug.depth_tested = False
            debug.add_box(BOX, GREEN)
            out["before"] = debug.line_count
            debug.clear()
            out.update(after=debug.line_count, depth=debug.vertices(True),
                       overlay=debug.vertices(False))

        observed = self._drawn(body)
        self.assertEqual(observed["before"], DEBUG_DRAW_BOX_EDGE_COUNT * 2)
        self.assertEqual(observed["after"], 0)
        self.assertEqual(observed["depth"], ())
        self.assertEqual(observed["overlay"], ())

    def test_beginning_again_clears_and_restores_depth_testing(self) -> None:
        def body(debug, device, out):
            debug.depth_tested = False
            debug.add_box(BOX, RED)
            debug.begin(VIEW, PROJECTION)
            out.update(count=debug.line_count, depth=debug.depth_tested)

        observed = self._drawn(body)
        self.assertEqual(observed["count"], 0)
        self.assertTrue(observed["depth"], "begin puts depth testing back on")

    def test_the_segment_bounds_are_the_measured_constants(self) -> None:
        self.assertEqual(DEBUG_DRAW_MINIMUM_SEGMENTS, 4)
        self.assertEqual(DEBUG_DRAW_MAXIMUM_SEGMENTS, 128)
        self.assertEqual(DEBUG_DRAW_DEFAULT_SEGMENTS, 24)
        self.assertEqual(DEBUG_DRAW_BOX_EDGE_COUNT, 12)

    def test_a_vertex_compares_and_reprs_by_its_own_values(self) -> None:
        one = DebugLineVertex(Vector3(1.0, 2.0, 3.0), RED)
        same = DebugLineVertex(Vector3(1.0, 2.0, 3.0), RED)
        other = DebugLineVertex(Vector3(1.0, 2.0, 4.0), RED)
        self.assertEqual(one, same)
        self.assertNotEqual(one, other)
        self.assertNotEqual(one, object())
        self.assertIn("DebugLineVertex", repr(one))

    def test_it_refuses_the_wrong_kind_of_argument(self) -> None:
        def body(debug, device, out):
            for call in (lambda: debug.add_line(object(), Vector3(0.0, 0.0, 0.0), RED),
                         lambda: debug.add_box(object(), RED),
                         lambda: debug.add_frustum(object(), RED),
                         lambda: debug.add_box(BOX, object()),
                         lambda: debug.vertices(1)):
                try:
                    call()
                    out.setdefault("results", []).append("no error")
                except TypeError:
                    out.setdefault("results", []).append("TypeError")

        self.assertEqual(self._drawn(body)["results"], ["TypeError"] * 5)

    def test_a_closed_drawer_refuses_rather_than_reaching_freed_memory(self) -> None:
        def body(game, device, out):
            debug = DebugDraw(device)
            debug.close()
            debug.close()
            out["closed"] = debug.is_closed
            try:
                debug.line_count
                out["raised"] = None
            except EngineDisposedError as error:
                out["raised"] = type(error).__name__

        observed = in_game(body)
        self.assertTrue(observed["closed"])
        self.assertEqual(observed["raised"], "EngineDisposedError")



@requires_engine_gpu
class DebugDrawRenderingTests(unittest.TestCase):
    """The parts that need a rasterizer."""

    def test_ending_a_batch_draws_it_and_clears_both_lists(self) -> None:
        def body(game, device, out):
            with DebugDraw(device) as debug:
                debug.begin(VIEW, PROJECTION)
                debug.add_box(BOX, RED)
                debug.depth_tested = False
                debug.add_cross(Vector3(0.0, 0.0, 0.0), 1.0, GREEN)
                out["before"] = debug.line_count
                debug.end()
                out.update(after=debug.line_count, depth=debug.vertices(True),
                           overlay=debug.vertices(False))

        observed = in_game(body)
        self.assertEqual(observed["before"], DEBUG_DRAW_BOX_EDGE_COUNT + 3)
        self.assertEqual(observed["after"], 0)
        self.assertEqual(observed["depth"], ())
        self.assertEqual(observed["overlay"], ())

    def test_ending_without_beginning_draws_nothing_rather_than_refusing(self) -> None:
        def body(game, device, out):
            with DebugDraw(device) as debug:
                debug.end()
                out["ended"] = True

        self.assertTrue(in_game(body)["ended"])

    def test_an_empty_batch_draws_nothing(self) -> None:
        def body(game, device, out):
            with DebugDraw(device) as debug:
                debug.begin(VIEW, PROJECTION)
                debug.end()
                out["count"] = debug.line_count

        self.assertEqual(in_game(body)["count"], 0)


@requires_engine_gpu
class StandaloneAsciiEffectTests(unittest.TestCase):
    """The ASCII effect used on its own rather than as a pass's configuration."""

    def test_it_can_be_built_directly_and_carries_the_same_state(self) -> None:
        def body(game, device, out):
            with AsciiEffect(device) as effect:
                out["default_cell"] = effect.cell_size
                effect.cell_size = (12, 20)
                out["cell"] = effect.cell_size
                effect.quantize_mode = AsciiQuantizeMode.BlackAndWhite
                out["mode"] = effect.quantize_mode
                out["closed"] = effect.is_closed

        observed = in_game(body)
        self.assertEqual(observed["cell"], (12, 20))
        self.assertEqual(observed["mode"], AsciiQuantizeMode.BlackAndWhite)
        self.assertFalse(observed["closed"])

    def test_constructing_one_needs_a_graphics_device(self) -> None:
        with self.assertRaises(TypeError):
            AsciiEffect()  # type: ignore[call-arg]
        with self.assertRaises(TypeError):
            AsciiEffect(object())

    def test_the_destination_rectangle_bounds_where_the_glyphs_land(self) -> None:
        """Rendered, because the grid dimensions cannot see the rectangle at all.

        The glyphs are drawn into a render target cleared to a colour nothing
        else produces, and the corner outside the rectangle is asserted to still
        hold it. A draw that ignored the rectangle would fill the whole target
        and that corner would change.
        """
        def body(game, device, out):
            from Microsoft.Xna.Framework.Graphics import (
                RenderTarget2D, SurfaceFormat, Texture2D,
            )

            source = Texture2D(device, 16, 16, False, SurfaceFormat.Color)
            source.SetData([Color(240, 240, 240, 255)] * 256)
            target = RenderTarget2D(device, 32, 32)
            try:
                with AsciiEffect(device) as effect:
                    effect.cell_size = (4, 4)
                    device.SetRenderTarget(target)
                    try:
                        device.Clear(Color(7, 11, 13, 255))
                        effect.draw(source, Rectangle(0, 0, 16, 16))
                    finally:
                        device.SetRenderTarget(None)
                    pixels = [Color(0, 0, 0, 0)] * (32 * 32)
                    target.GetData(pixels)
                    out["outside"] = tuple(int(value) for value in
                                           (pixels[31 * 32 + 31].R,
                                            pixels[31 * 32 + 31].G,
                                            pixels[31 * 32 + 31].B))
                    out["inside_changed"] = any(
                        (int(pixels[y * 32 + x].R), int(pixels[y * 32 + x].G),
                         int(pixels[y * 32 + x].B)) != (7, 11, 13)
                        for y in range(16) for x in range(16))
            finally:
                target.Dispose()
                source.Dispose()

        observed = in_game(body)
        self.assertEqual(observed["outside"], (7, 11, 13),
                         "a draw that ignored the rectangle would have covered this")
        self.assertTrue(observed["inside_changed"],
                        "and it must actually have drawn inside it")

    def test_the_grid_describes_the_source_and_not_the_destination(self) -> None:
        """Measured, and the opposite of what "grid dimensions" suggests.

        The grid is the *source* size divided by the cell size. A 32-pixel
        source with an 8-pixel cell is four by four whether it fills the
        viewport or a 64 by 32 rectangle; the destination only decides how far
        apart the glyphs land.
        """
        def body(game, device, out):
            from Microsoft.Xna.Framework.Graphics import SurfaceFormat, Texture2D

            source = Texture2D(device, 32, 32, False, SurfaceFormat.Color)
            source.SetData([Color(int(255 * (x % 32) / 31), 64, 128, 255)
                            for _y in range(32) for x in range(32)])
            try:
                with AsciiEffect(device) as effect:
                    for cell in ((8, 8), (16, 16), (4, 4)):
                        effect.cell_size = cell
                        effect.draw(source)
                        viewport = effect.last_grid_dimensions
                        effect.draw(source, Rectangle(0, 0, 64, 32))
                        wide = effect.last_grid_dimensions
                        effect.draw(source, Rectangle(10, 10, 80, 40))
                        offset = effect.last_grid_dimensions
                        out[cell] = (viewport, wide, offset)
            finally:
                source.Dispose()

        observed = in_game(body)
        for cell, (viewport, wide, offset) in observed.items():
            expected = (32 // cell[0], 32 // cell[1])
            self.assertEqual(viewport, expected, f"cell {cell} filling the viewport")
            self.assertEqual(wide, expected, f"cell {cell} into 64x32")
            self.assertEqual(offset, expected, f"cell {cell} into an offset rectangle")

    def test_it_refuses_the_wrong_kind_of_argument(self) -> None:
        def body(game, device, out):
            with AsciiEffect(device) as effect:
                for call in (lambda: effect.draw(object()),
                             lambda: effect.draw(object(), Rectangle(0, 0, 8, 8))):
                    try:
                        call()
                        out.setdefault("results", []).append("no error")
                    except TypeError:
                        out.setdefault("results", []).append("TypeError")

        self.assertEqual(in_game(body)["results"], ["TypeError"] * 2)


class DebugAbsenceTests(unittest.TestCase):
    """What the family answers on a build with no graphics extension layer at all."""

    @requires_native
    @unittest.skipIf(ENGINE_PRESENT, "this build has the graphics extension layer")
    def test_the_drawer_reports_the_layer_as_absent(self) -> None:
        def body(game, device, out):
            try:
                DebugDraw(device).close()
                out["raised"] = None
            except EngineUnavailableError as error:
                out["raised"] = type(error).__name__

        self.assertEqual(in_game(body)["raised"], "EngineUnavailableError")

    @requires_native
    @unittest.skipIf(ENGINE_PRESENT, "this build has the graphics extension layer")
    def test_the_standalone_ascii_effect_does_too(self) -> None:
        def body(game, device, out):
            try:
                AsciiEffect(device).close()
                out["raised"] = None
            except EngineUnavailableError as error:
                out["raised"] = type(error).__name__

        self.assertEqual(in_game(body)["raised"], "EngineUnavailableError")
