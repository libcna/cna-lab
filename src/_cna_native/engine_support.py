"""Private plumbing shared by the ``cna.extensions.engine`` modules.

The graphics extension layer differs from the strict families in one way that
shapes all of this: **a CNA build may not contain it at all.** Every route is
exported in every build, and the ones that need the layer answer
``CNA_RESULT_NOT_SUPPORTED`` when CNA was configured without ``CNA_CNAEXT``. That is a
different fact from "this renderer cannot do that", and collapsing the two would
tell a caller to change GPUs when the answer is to change builds. So the two are
separated here, once, by asking CNA which case applies.

Also here, for the same reason they are in the compiled-content family: the
two-call size/copy protocol, checked width conversion, and deterministic handle
lifetime with no reliance on ``__del__``.

Nothing here is public, and no object defined here reaches a caller.
"""

from __future__ import annotations

import ctypes as c
from typing import Iterable

from .errors import NativeError
from .loader import get_library

#: Inclusive bounds of every fixed-width parameter this family passes.
_WIDTHS = {
    "uint8": (0, 0xFF),
    "uint16": (0, 0xFFFF),
    "uint32": (0, 0xFFFFFFFF),
    "uint64": (0, 0xFFFFFFFFFFFFFFFF),
    "int32": (-0x80000000, 0x7FFFFFFF),
    "int64": (-0x8000000000000000, 0x7FFFFFFFFFFFFFFF),
}

#: ``CNA_RESULT_BUFFER_TOO_SMALL``. A copy route that cannot fit its output
#: still writes the required byte count, which is how the size half of the
#: two-call protocol asks its question.
_BUFFER_TOO_SMALL = 14

#: ``CNA_RESULT_NOT_SUPPORTED``. The one result whose meaning depends on which
#: build is loaded, which is why it is named rather than folded into the table.
_NOT_SUPPORTED = 6

#: CNA result code -> the public exception class that names what it means.
_RESULT_CLASSES = {
    1: "EngineArgumentError",     # INVALID_ARGUMENT
    2: "EngineInternalError",     # INVALID_HANDLE -- this layer never passes one
    3: "EngineStateError",        # INVALID_STATE
    4: "EngineInternalError",     # OUT_OF_MEMORY
    5: "EngineInternalError",     # IO
    6: "EngineUnsupportedError",  # NOT_SUPPORTED -- refined below
    7: "EngineInternalError",     # PLATFORM
    8: "EngineThreadError",       # THREAD
    9: "EngineInternalError",     # CALLBACK
    10: "EngineArgumentError",    # OVERFLOW
    11: "EngineArgumentError",    # ENCODING
    12: "EngineInternalError",    # INTERNAL
    13: "EngineInternalError",    # SHUTTING_DOWN
}


def graphics_ext_is_available() -> bool:
    """Whether the loaded library was built with the graphics extension layer.

    This is the one route in the family that is meaningful in a build without
    the layer, and it is what separates "configured out" from "this renderer
    cannot".
    """
    library = get_library()
    value = c.c_uint8()
    library.check(library.cna_graphics_ext_is_available(c.byref(value)),
                  "cna_graphics_ext_is_available")
    return value.value != 0


def _translate(error: NativeError):
    """Maps one CNA failure onto the public engine exception for it.

    Imported at the point of failure rather than at module scope, so the private
    layer keeps not depending on the public one at import time.
    """
    from cna.extensions.engine import errors as public

    name = _RESULT_CLASSES.get(error.result, "EngineInternalError")
    if error.result == _NOT_SUPPORTED:
        # Asked, not assumed. A build without the layer answers the same
        # result code as a renderer that lacks a feature, and only one of those
        # is fixed by running somewhere else.
        try:
            present = graphics_ext_is_available()
        except NativeError:  # pragma: no cover - the availability route itself failed
            present = True
        if not present:
            name = "EngineUnavailableError"
    return getattr(public, name)(
        error.operation, error.result, error.category, error.native_message)


def call(operation: str, *arguments: object) -> None:
    """Invokes one CNA route, raising the public exception for any failure."""
    library = get_library()
    function = getattr(library, operation)
    try:
        library.check(function(*arguments), operation)
    except NativeError as error:
        raise _translate(error) from None


def size_call(operation: str, *arguments: object) -> None:
    """Runs the sizing half of a two-call protocol whose count is not a byte count.

    ``CNA_RESULT_BUFFER_TOO_SMALL`` is the expected answer for a non-empty
    output and success for an empty one; anything else is a real failure. The
    text protocol in :func:`copied_text` handles its own sizing; this is for the
    routes that copy a range of *values* instead.
    """
    library = get_library()
    result = int(getattr(library, operation)(*arguments))
    if result in (0, _BUFFER_TOO_SMALL):
        return
    try:
        library.check(result, operation)
    except NativeError as error:
        raise _translate(error) from None


def checked(value: object, width: str, what: str) -> int:
    """Range-checks a Python integer against the native width it is about to take."""
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{what} must be an int, not {type(value).__name__}")
    low, high = _WIDTHS[width]
    if not low <= value <= high:
        raise ValueError(f"{what} must be in {low}..{high}, got {value}")
    return int(value)


def real(value: object, what: str) -> float:
    """Accepts a real number and refuses a bool, which is an int in Python."""
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"{what} must be a real number, not {type(value).__name__}")
    return float(value)


def out_u8(operation: str, *arguments: object) -> int:
    value = c.c_uint8()
    call(operation, *arguments, c.byref(value))
    return int(value.value)


def out_bool(operation: str, *arguments: object) -> bool:
    return out_u8(operation, *arguments) != 0


def out_u32(operation: str, *arguments: object) -> int:
    value = c.c_uint32()
    call(operation, *arguments, c.byref(value))
    return int(value.value)


def out_i32(operation: str, *arguments: object) -> int:
    value = c.c_int32()
    call(operation, *arguments, c.byref(value))
    return int(value.value)


def out_handle(operation: str, *arguments: object) -> int:
    handle = c.c_uint64()
    call(operation, *arguments, c.byref(handle))
    return int(handle.value)


def copied_values(element: type, operation: str, arguments: Iterable[object]):
    """The two-call size/copy protocol for a route that copies a range of values.

    For the routes whose count is a number of *values* rather than a number of
    bytes. Sized first because the length is
    CNA's and not the caller's: asking for a guessed number answers
    ``CNA_RESULT_BUFFER_TOO_SMALL`` rather than truncating, which is the right
    refusal and the reason it is asked for at all.

    Returns the filled ``element`` array and the number of entries CNA wrote,
    which is never larger than the size it asked for.
    """
    arguments = tuple(arguments)
    count = c.c_uint64()
    size_call(operation, *arguments, None, c.c_uint64(0), c.byref(count))
    if count.value == 0:
        return (element * 0)(), 0
    destination = (element * count.value)()
    written = c.c_uint64()
    call(operation, *arguments, destination, c.c_uint64(count.value), c.byref(written))
    return destination, int(written.value)


class NativeHandle:
    """One owned CNA engine handle with a deterministic, explicit lifetime.

    ``close`` is the mechanism; ``__del__`` is not implemented at all, because
    interpreter shutdown may already have unloaded the library and a finalizer
    that calls into it would be a crash rather than a cleanup. Every public
    engine object is a context manager for the same reason.
    """

    __slots__ = ("_value", "_destroy", "_what", "_closed", "_parent", "_children")

    def __init__(self, value: int, destroy: str | None, what: str,
                 parent: "NativeHandle | None" = None) -> None:
        if value == 0:
            raise ValueError(f"{what}: CNA returned an invalid handle")
        self._value = int(value)
        self._destroy = destroy
        self._what = what
        self._closed = False
        self._parent = parent
        self._children: list["NativeHandle"] = []
        if parent is not None:
            parent._children.append(self)

    @property
    def closed(self) -> bool:
        return self._closed or (self._parent is not None and self._parent.closed)

    @property
    def value(self) -> int:
        if self._closed:
            raise _disposed(self._what)
        if self._parent is not None and self._parent.closed:
            raise _disposed(f"{self._what} (its owner is closed)")
        return self._value

    @property
    def argument(self) -> c.c_uint64:
        return c.c_uint64(self.value)

    def close(self) -> None:
        if self._closed:
            return
        # Children first: CNA refuses to destroy a parent whose views are live,
        # and that refusal is a contract to keep rather than a race to lose.
        for child in list(reversed(self._children)):
            child.close()
        self._children.clear()
        if self._destroy is not None:
            call(self._destroy, c.c_uint64(self._value))
        self._closed = True
        self._value = 0
        if self._parent is not None and self in self._parent._children:
            self._parent._children.remove(self)


def _disposed(what: str):
    from cna.extensions.engine import errors as public

    return public.EngineDisposedError(
        "use after close", 0, None, f"{what} is closed")
