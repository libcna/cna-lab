"""ctypes manifest for what CNA kept of its engine layer, in ``graphics_ext.h``.

CNA retired ``engine_layer.h`` at C ABI 0.30 (MOD-RETIRE-1). DebugDraw and the
ASCII post-process effect survived in ``graphics_ext.h``, and they are what
``cna.extensions.engine`` still projects. Every entry is proven against the
canonical C declaration by ``tools/verify_prototypes.py``, and the fourth column
states the ownership contract the Python wrapper has to keep.

Routes are added as their consumers land, never speculatively: an imported route
with no caller is dead native surface, and ``tools/verify_route_reachability.py``
fails on one.
"""

from __future__ import annotations

import ctypes as c

from . import abi
from . import engine_abi as engine

#: Whether this build carries the standalone graphics extensions at all. Every
#: route below is exported by every CNA build, and the ones that need the layer
#: answer ``CNA_RESULT_NOT_SUPPORTED`` when it was configured out, so this is
#: the route that tells "configured out" apart from "this renderer cannot".
ENGINE_IDENTITY_MANIFEST: tuple[tuple[str, object, list[object], str], ...] = (
    ("cna_graphics_ext_is_available", c.c_uint32, [c.POINTER(c.c_uint8)],
     "pure function over caller-owned output; nothing is retained"),
)

#: Debug drawing: world-space lines and the shapes built from them.
ENGINE_DEBUG_MANIFEST: tuple[tuple[str, object, list[object], str], ...] = (
    ("cna_debug_draw_create", c.c_uint32, [c.c_uint64, c.POINTER(c.c_uint64)],
     "owned debug drawer; borrows the graphics device for the call"),
    ("cna_debug_draw_begin", c.c_uint32,
     [c.c_uint64, c.POINTER(abi.CNA_Matrix), c.POINTER(abi.CNA_Matrix)],
     "borrowed drawer; clears both lists and copies the two matrices"),
    ("cna_debug_draw_end", c.c_uint32, [c.c_uint64],
     "borrowed drawer; draws both lists and clears them"),
    ("cna_debug_draw_clear", c.c_uint32, [c.c_uint64],
     "borrowed drawer; drops every line without drawing"),
    ("cna_debug_draw_add_line", c.c_uint32,
     [c.c_uint64, c.POINTER(abi.CNA_Vector3), c.POINTER(abi.CNA_Vector3), abi.CNA_Color],
     "borrowed drawer; copies both endpoints and the colour"),
    ("cna_debug_draw_add_box", c.c_uint32,
     [c.c_uint64, c.POINTER(engine.CNA_BoundingBox), abi.CNA_Color],
     "borrowed drawer; twelve lines from the box's own corners"),
    ("cna_debug_draw_add_sphere", c.c_uint32,
     [c.c_uint64, c.POINTER(abi.CNA_Vector3), c.c_float, abi.CNA_Color, c.c_int32],
     "borrowed drawer; three rings of the clamped segment count"),
    ("cna_debug_draw_add_bounding_sphere", c.c_uint32,
     [c.c_uint64, c.POINTER(engine.CNA_BoundingSphere), abi.CNA_Color, c.c_int32],
     "borrowed drawer; the same three rings, from a bounding sphere"),
    ("cna_debug_draw_add_frustum", c.c_uint32,
     [c.c_uint64, engine.CNA_BoundingFrustum, abi.CNA_Color],
     "borrowed drawer; the frustum crosses BY VALUE, unlike every other bound here"),
    ("cna_debug_draw_add_cross", c.c_uint32,
     [c.c_uint64, c.POINTER(abi.CNA_Vector3), c.c_float, abi.CNA_Color],
     "borrowed drawer; three axis-aligned lines through the point"),
    ("cna_debug_draw_is_depth_tested", c.c_uint32, [c.c_uint64, c.POINTER(c.c_uint8)],
     "caller output; borrowed drawer"),
    ("cna_debug_draw_set_depth_tested", c.c_uint32, [c.c_uint64, c.c_uint8],
     "borrowed drawer; decides which of the two lists later lines join"),
    ("cna_debug_draw_get_line_count", c.c_uint32, [c.c_uint64, c.POINTER(c.c_int32)],
     "caller output; both lists together"),
    ("cna_debug_draw_copy_vertices", c.c_uint32,
     [c.c_uint64, c.c_uint8, c.POINTER(abi.CNA_VertexPositionColor), c.c_uint64,
      c.POINTER(c.c_uint64)],
     "caller output; two-call size/copy protocol over one of the two lists"),
    ("cna_debug_draw_destroy", c.c_uint32, [c.c_uint64],
     "consumes the drawer"),
)

#: The standalone ASCII post-process effect: quantize any texture into any
#: rectangle, with its cell size and quantize mode.
ENGINE_ASCII_EFFECT_MANIFEST: tuple[tuple[str, object, list[object], str], ...] = (
    ("cna_ascii_post_process_effect_create", c.c_uint32,
     [c.c_uint64, c.POINTER(c.c_uint64)],
     "owned ASCII effect; borrows the graphics device for the call"),
    ("cna_ascii_post_process_effect_draw", c.c_uint32,
     [c.c_uint64, c.c_uint64, c.POINTER(abi.CNA_Rectangle)],
     "borrowed effect; the source texture is borrowed for the call"),
    ("cna_ascii_post_process_effect_get_cell_size", c.c_uint32,
     [c.c_uint64, c.POINTER(c.c_int32), c.POINTER(c.c_int32)],
     "caller output; borrowed ASCII effect"),
    ("cna_ascii_post_process_effect_set_cell_size", c.c_uint32,
     [c.c_uint64, c.c_int32, c.c_int32], "borrowed ASCII effect; copies both values"),
    ("cna_ascii_post_process_effect_get_quantize_mode", c.c_uint32,
     [c.c_uint64, c.POINTER(c.c_uint32)], "caller output; borrowed ASCII effect"),
    ("cna_ascii_post_process_effect_set_quantize_mode", c.c_uint32,
     [c.c_uint64, c.c_uint32], "borrowed ASCII effect; copies the value"),
    ("cna_ascii_post_process_effect_get_last_grid_dimensions", c.c_uint32,
     [c.c_uint64, c.POINTER(c.c_int32), c.POINTER(c.c_int32)],
     "caller output; borrowed ASCII effect"),
    ("cna_ascii_post_process_effect_destroy", c.c_uint32, [c.c_uint64],
     "consumes the ASCII effect"),
)

#: Every engine route this binding imports, in one tuple for the loader.
#: Every engine route this binding imports, in one tuple for the loader.
ENGINE_FUNCTION_MANIFEST: tuple[tuple[str, object, list[object], str], ...] = (
    ENGINE_IDENTITY_MANIFEST
    + ENGINE_DEBUG_MANIFEST
    + ENGINE_ASCII_EFFECT_MANIFEST
)

# ``engine`` is imported for the structures the debug drawer passes by pointer;
# the reference keeps the module a declared dependency of this manifest rather
# than an accident of import order.
_STRUCTURES = engine.ENGINE_STRUCTURES
