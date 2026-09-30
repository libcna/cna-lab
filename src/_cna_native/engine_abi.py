"""Generated ctypes layouts and constants for CNA's ``graphics_ext.h``.

Do not edit. ``tools/generate_family_abi.py`` derives this from the canonical
headers, and ``--check`` fails when the checked-in copy is not what the current
headers produce. Every size, alignment, field offset and constant here is
re-measured against the C compiler by ``tools/audit_cna_abi.py``.

Nothing in this module is public. ``cna.extensions.engine`` holds the
public projection; a ctypes object never crosses that boundary.
"""

from __future__ import annotations

import ctypes as c

from . import abi

# --- scalar identities -----------------------------------------------------

#: Fixed-width identities these headers declare as typedefs of a scalar.
#: They are enums in spirit and integers in the ABI; the public projection
#: turns them into Python enums, and this is only their width.
CNA_AsciiQuantizeMode = c.c_uint32
CNA_CRTMaskType = c.c_uint32
CNA_DepthEffectMode = c.c_uint32
CNA_DitherMode = c.c_uint32

#: Every opaque handle in this family is a ``CNA_Handle``. The names are kept
#: so a manifest entry can say which object a handle parameter refers to.
ENGINE_HANDLE_TYPES = (
    "CNA_AsciiPostProcessEffectHandle",
    "CNA_DebugDrawHandle",
)


# --- constants -------------------------------------------------------------

CNA_ASCII_QUANTIZE_MODE_BLACK_WHITE = 0
CNA_ASCII_QUANTIZE_MODE_COLOR = 1
CNA_CRT_MASK_TYPE_APERTURE_GRILLE = 1
CNA_CRT_MASK_TYPE_NONE = 0
CNA_CRT_MASK_TYPE_SHADOW_MASK = 2
CNA_DEBUG_DRAW_MAX_SEGMENTS = 128
CNA_DEBUG_DRAW_MIN_SEGMENTS = 4
CNA_DEPTH_EFFECT_MODE_COLOR_16_BIT = 0
CNA_DEPTH_EFFECT_MODE_COLOR_8_BIT = 1
CNA_DEPTH_EFFECT_MODE_GRAYSCALE_1_BIT = 4
CNA_DEPTH_EFFECT_MODE_GRAYSCALE_2_BIT = 3
CNA_DEPTH_EFFECT_MODE_GRAYSCALE_4_BIT = 2
CNA_DEPTH_EFFECT_MODE_PALETTE_16 = 6
CNA_DEPTH_EFFECT_MODE_PALETTE_256 = 5
CNA_DITHER_MODE_BAYER_4X4 = 1
CNA_DITHER_MODE_BAYER_8X8 = 2
CNA_DITHER_MODE_NONE = 0

# --- structures ------------------------------------------------------------

class CNA_ImageBasedLightEXT(c.Structure):
    _fields_ = [
        ("struct_size", c.c_uint32),
        ("struct_version", c.c_uint32),
        ("irradiance", c.c_uint64),
        ("prefiltered_specular", c.c_uint64),
        ("brdf_lut", c.c_uint64),
        ("prefiltered_mip_count", c.c_int32),
        ("intensity", c.c_float),
    ]

class CNA_IndirectDrawArguments(c.Structure):
    _fields_ = [
        ("vertex_count", c.c_uint32),
        ("instance_count", c.c_uint32),
        ("first_vertex", c.c_uint32),
        ("base_instance", c.c_uint32),
    ]

class CNA_IndirectDrawIndexedArguments(c.Structure):
    _fields_ = [
        ("index_count", c.c_uint32),
        ("instance_count", c.c_uint32),
        ("first_index", c.c_uint32),
        ("base_vertex", c.c_int32),
        ("base_instance", c.c_uint32),
    ]

class CNA_BoundingBox(c.Structure):
    _fields_ = [
        ("min", abi.CNA_Vector3),
        ("max", abi.CNA_Vector3),
    ]

class CNA_BoundingSphere(c.Structure):
    _fields_ = [
        ("center", abi.CNA_Vector3),
        ("radius", c.c_float),
    ]

class CNA_BoundingFrustum(c.Structure):
    _fields_ = [
        ("matrix", abi.CNA_Matrix),
    ]


#: This family hands CNA no function pointers.
ENGINE_CALLBACKS = ()
ENGINE_CALLBACK_CONST_PARAMETERS = {}

# --- constants derived from a generated layout ------------------------------


#: Each structure field's own ``@brief`` from the canonical header, so a
#: public projection documents a field with CNA's own words rather than a
#: second summary that can drift from it.
ENGINE_FIELD_DOCUMENTATION = {
    "CNA_ImageBasedLightEXT": {
        "struct_size": "Size of this caller-provided structure in bytes.",
        "struct_version": "Version of this caller-provided structure.",
        "irradiance": "The irradiance cube, or `CNA_INVALID_HANDLE`.",
        "prefiltered_specular": "The prefiltered specular cube, or `CNA_INVALID_HANDLE`.",
        "brdf_lut": "The BRDF lookup texture, or `CNA_INVALID_HANDLE`.",
        "prefiltered_mip_count": "How many mip levels the prefiltered cube has; at least one.",
        "intensity": "Scalar multiplier on the light.",
    },
    "CNA_IndirectDrawArguments": {
        "vertex_count": "How many vertices to fetch.",
        "instance_count": "How many instances to draw; one for an ordinary draw, zero to draw nothing.",
        "first_vertex": "The first vertex, in elements of the bound stream.",
        "base_instance": "The first instance.",
    },
    "CNA_IndirectDrawIndexedArguments": {
        "index_count": "How many indices to fetch.",
        "instance_count": "How many instances to draw.",
        "first_index": "The first index, in index elements.",
        "base_vertex": "Added to every decoded index, in vertex elements; signed, as the API is.",
        "base_instance": "The first instance; must be zero on GL ES, for the reason above.",
    },
}

#: Every generated structure, in declaration order, for the ABI audit.
ENGINE_STRUCTURES = (
    CNA_ImageBasedLightEXT,
    CNA_IndirectDrawArguments,
    CNA_IndirectDrawIndexedArguments,
    CNA_BoundingBox,
    CNA_BoundingSphere,
    CNA_BoundingFrustum,
)

#: Every generated constant, for the ABI audit to re-read from C.
ENGINE_CONSTANTS = (
    "CNA_ASCII_QUANTIZE_MODE_BLACK_WHITE",
    "CNA_ASCII_QUANTIZE_MODE_COLOR",
    "CNA_CRT_MASK_TYPE_APERTURE_GRILLE",
    "CNA_CRT_MASK_TYPE_NONE",
    "CNA_CRT_MASK_TYPE_SHADOW_MASK",
    "CNA_DEBUG_DRAW_MAX_SEGMENTS",
    "CNA_DEBUG_DRAW_MIN_SEGMENTS",
    "CNA_DEPTH_EFFECT_MODE_COLOR_16_BIT",
    "CNA_DEPTH_EFFECT_MODE_COLOR_8_BIT",
    "CNA_DEPTH_EFFECT_MODE_GRAYSCALE_1_BIT",
    "CNA_DEPTH_EFFECT_MODE_GRAYSCALE_2_BIT",
    "CNA_DEPTH_EFFECT_MODE_GRAYSCALE_4_BIT",
    "CNA_DEPTH_EFFECT_MODE_PALETTE_16",
    "CNA_DEPTH_EFFECT_MODE_PALETTE_256",
    "CNA_DITHER_MODE_BAYER_4X4",
    "CNA_DITHER_MODE_BAYER_8X8",
    "CNA_DITHER_MODE_NONE",
)
