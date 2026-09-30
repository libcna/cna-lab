"""Private conversions and lifetime shared by the modules of this package."""

from __future__ import annotations

import ctypes as c

from Microsoft.Xna.Framework import Matrix, Vector3

from _cna_native import abi as _abi
from _cna_native import engine_support as _support

__all__: list[str] = []


def _vector(value: _abi.CNA_Vector3) -> Vector3:
    return Vector3(float(value.x), float(value.y), float(value.z))


def _native_vector(value: Vector3) -> _abi.CNA_Vector3:
    if not isinstance(value, Vector3):
        raise TypeError("expected a Microsoft.Xna.Framework.Vector3")
    return _abi.CNA_Vector3(float(value.X), float(value.Y), float(value.Z))


def _native_matrix(value: Matrix) -> _abi.CNA_Matrix:
    if not isinstance(value, Matrix):
        raise TypeError("expected a Microsoft.Xna.Framework.Matrix")
    return _abi.CNA_Matrix(*tuple(value))


def _device_handle(device: object) -> c.c_uint64:
    """The native handle of a strict-XNA graphics device, refusing anything else."""
    if not hasattr(device, "_require_handle"):
        raise TypeError("device must be a Microsoft.Xna.Framework.Graphics.GraphicsDevice")
    return c.c_uint64(int(device._require_handle()))


class _EngineObject:
    """Deterministic lifetime for one owned native handle.

    Public subclasses take the arguments a caller has -- a device -- and never a
    handle: a signature naming one would publish a private native type.
    """

    __slots__ = ("_handle", "_device")

    _DESTROY: str = ""

    def _attach(self, handle: int, device: object = None) -> None:
        self._handle = _support.NativeHandle(handle, self._DESTROY, type(self).__name__)
        self._device = device

    @property
    def is_closed(self) -> bool:
        """True once :meth:`close` has run."""
        return self._handle.closed

    def close(self) -> None:
        """Releases the object. Calling it twice is not an error."""
        self._handle.close()

    def __enter__(self):
        self._handle.value
        return self

    def __exit__(self, *_exception: object) -> None:
        self.close()
