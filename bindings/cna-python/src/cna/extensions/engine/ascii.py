"""The standalone ASCII post-process effect.

It quantises any texture into a glyph grid drawn into any rectangle of the
current target. CNA retired the post-process chain this effect used to run in
(ABI 0.30); the effect itself survived in ``graphics_ext.h``, and that is what
this module projects.
"""

from __future__ import annotations

import ctypes as c
from enum import IntEnum
from typing import TYPE_CHECKING

from _cna_native import abi as _abi
from _cna_native import engine_abi as _engine
from _cna_native import engine_support as _support

from .values import _device_handle

if TYPE_CHECKING:  # pragma: no cover - annotations only
    from Microsoft.Xna.Framework.Graphics import GraphicsDevice

__all__ = ["AsciiEffect", "AsciiQuantizeMode"]


class AsciiQuantizeMode(IntEnum):
    """Whether the effect keeps colour or reduces to black and white."""

    BlackAndWhite = _engine.CNA_ASCII_QUANTIZE_MODE_BLACK_WHITE
    Color = _engine.CNA_ASCII_QUANTIZE_MODE_COLOR


class AsciiEffect:
    """Redraws a texture as characters on a grid.

    Owned: :meth:`close` releases it, and it works as a context manager.
    """

    __slots__ = ("_handle",)

    def __init__(self, device: "GraphicsDevice") -> None:
        self._handle = _support.NativeHandle(
            _support.out_handle("cna_ascii_post_process_effect_create",
                                _device_handle(device)),
            "cna_ascii_post_process_effect_destroy", "ASCII effect")

    @property
    def is_closed(self) -> bool:
        """True once this effect has been released."""
        return self._handle.closed

    def close(self) -> None:
        """Releases the effect. Calling it twice is not an error."""
        self._handle.close()

    def __enter__(self) -> "AsciiEffect":
        self._handle.value
        return self

    def __exit__(self, *_exception: object) -> None:
        self.close()

    def draw(self, source, destination_rectangle=None) -> None:
        """Quantises ``source`` and draws the glyph grid into the current target.

        ``destination_rectangle`` is in render-target pixels; ``None`` fills the
        viewport. The source is borrowed for the call.
        """
        from Microsoft.Xna.Framework import Rectangle

        if not hasattr(source, "_require_handle"):
            raise TypeError("source must be a graphics texture")
        rectangle = None
        if destination_rectangle is not None:
            if not isinstance(destination_rectangle, Rectangle):
                raise TypeError(
                    "destination_rectangle must be a "
                    "Microsoft.Xna.Framework.Rectangle or None")
            rectangle = _abi.CNA_Rectangle(
                _support.checked(int(destination_rectangle.X), "int32", "X"),
                _support.checked(int(destination_rectangle.Y), "int32", "Y"),
                _support.checked(int(destination_rectangle.Width), "int32", "Width"),
                _support.checked(int(destination_rectangle.Height), "int32", "Height"))
        _support.call("cna_ascii_post_process_effect_draw", self._handle.argument,
                      c.c_uint64(int(source._require_handle())),
                      None if rectangle is None else c.byref(rectangle))

    @property
    def quantize_mode(self) -> AsciiQuantizeMode:
        """Whether the effect keeps colour or reduces to two tones."""
        return AsciiQuantizeMode(_support.out_u32(
            "cna_ascii_post_process_effect_get_quantize_mode", self._handle.argument))

    @quantize_mode.setter
    def quantize_mode(self, value: AsciiQuantizeMode) -> None:
        _support.call("cna_ascii_post_process_effect_set_quantize_mode",
                      self._handle.argument,
                      c.c_uint32(int(AsciiQuantizeMode(value))))

    @property
    def cell_size(self) -> tuple[int, int]:
        """The character cell, in pixels, as ``(width, height)``."""
        width, height = c.c_int32(), c.c_int32()
        _support.call("cna_ascii_post_process_effect_get_cell_size",
                      self._handle.argument, c.byref(width), c.byref(height))
        return int(width.value), int(height.value)

    @cell_size.setter
    def cell_size(self, value: tuple[int, int]) -> None:
        width, height = value
        _support.call("cna_ascii_post_process_effect_set_cell_size",
                      self._handle.argument,
                      c.c_int32(_support.checked(width, "int32", "width")),
                      c.c_int32(_support.checked(height, "int32", "height")))

    @property
    def last_grid_dimensions(self) -> tuple[int, int]:
        """How the last draw quantised its *source*, as ``(columns, rows)``.

        The source's size divided by :attr:`cell_size`, measured -- **not** how
        many cells the destination covered. Drawing a 32-pixel source with an
        8-pixel cell is always four by four, whether it fills the viewport or a
        64 by 32 rectangle; the destination only decides how far apart the
        glyphs land.
        """
        columns, rows = c.c_int32(), c.c_int32()
        _support.call("cna_ascii_post_process_effect_get_last_grid_dimensions",
                      self._handle.argument, c.byref(columns), c.byref(rows))
        return int(columns.value), int(rows.value)
