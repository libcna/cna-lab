"""What CNA kept of its engine layer: debug lines and the ASCII effect.

This is a **CNA extension**, not part of XNA. CNA retired its modern engine
layer (``engine_layer.h``: PBR materials, post-process chains, shadows,
clustered lighting, probes, compute) at C ABI 0.30 and kept two standalone
pieces in ``graphics_ext.h``: :class:`DebugDraw` and :class:`AsciiEffect`. Those
two are what this package still projects.

**The strict XNA projection is untouched by this package.** No name in
``Microsoft.Xna.Framework`` changes, gains a member, or learns that this package
exists. The dependency runs one way -- this package accepts strict
``GraphicsDevice``, ``Texture2D``, ``Matrix``, ``Vector3`` and ``Color`` values --
and the extension gate asserts in a fresh interpreter that importing the XNA
namespace loads no ``cna`` module.

Two kinds of "not supported"
----------------------------

Every route is exported in every CNA build. The ones that need the extension
layer answer ``CNA_RESULT_NOT_SUPPORTED`` when CNA was configured without
``CNA_CNAEXT`` -- the same result code a renderer without a capability gives.
:func:`is_available` asks ``cna_graphics_ext_is_available``;
:class:`~cna.extensions.engine.errors.EngineUnavailableError` means the build has
no layer, and :class:`~cna.extensions.engine.errors.EngineUnsupportedError`
means a present layer still cannot do it.

Ownership
---------

Every object holding a CNA handle has an explicit ``close`` and works as a
context manager. Nothing relies on ``__del__``.

Importing this module needs no native library. Constructing anything in it does.
"""

from __future__ import annotations

from _cna_native import engine_support as _support

from . import ascii, debug, errors
from .ascii import AsciiEffect, AsciiQuantizeMode
from .debug import (
    DEBUG_DRAW_BOX_EDGE_COUNT, DEBUG_DRAW_DEFAULT_SEGMENTS, DEBUG_DRAW_MAXIMUM_SEGMENTS,
    DEBUG_DRAW_MINIMUM_SEGMENTS, DebugDraw, DebugLineVertex,
)
from .errors import (
    EngineArgumentError, EngineDisposedError, EngineError, EngineInternalError,
    EngineStateError, EngineThreadError, EngineUnavailableError, EngineUnsupportedError,
)

__all__ = [
    "AsciiEffect",
    "AsciiQuantizeMode",
    "DEBUG_DRAW_BOX_EDGE_COUNT",
    "DEBUG_DRAW_DEFAULT_SEGMENTS",
    "DEBUG_DRAW_MAXIMUM_SEGMENTS",
    "DEBUG_DRAW_MINIMUM_SEGMENTS",
    "DebugDraw",
    "DebugLineVertex",
    "EngineArgumentError",
    "EngineDisposedError",
    "EngineError",
    "EngineInternalError",
    "EngineStateError",
    "EngineThreadError",
    "EngineUnavailableError",
    "EngineUnsupportedError",
    "is_available",
]


def is_available() -> bool:
    """Whether the loaded CNA build contains the graphics extension layer.

    This is the question to ask before constructing anything here. It is
    measured from CNA, never inferred from the renderer's name or from a route
    existing.
    """
    return _support.graphics_ext_is_available()
