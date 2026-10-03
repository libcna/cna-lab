"""What went wrong, in categories that say what a caller can do about it.

These are **CNA** errors, not XNA ones. DebugDraw and the ASCII effect have no
XNA counterpart at all, so translating their refusals into
``NotSupportedException`` or ``InvalidOperationException`` would claim an
equivalence that does not exist and would throw away the reason CNA gave.

The distinction the whole family turns on is between two answers that share one
CNA result code:

* :class:`EngineUnavailableError` -- **this build has no graphics extension
  layer** (CNA was configured without ``CNA_CNAEXT``). Every route is exported
  in every CNA build, and the ones needing the layer answer
  ``CNA_RESULT_NOT_SUPPORTED`` when it was configured out. Nothing about the
  machine or the renderer will change that.
* :class:`EngineUnsupportedError` -- **this renderer or device cannot do it.**
  The layer is present and answered; the capability is not there.

Which one applies is measured by asking ``cna_graphics_ext_is_available``, not
inferred from the renderer's name. The exact result code, CNA's error category
and CNA's own diagnostic stay on every instance.
"""

from __future__ import annotations

__all__ = [
    "EngineError",
    "EngineUnavailableError",
    "EngineUnsupportedError",
    "EngineStateError",
    "EngineArgumentError",
    "EngineDisposedError",
    "EngineThreadError",
    "EngineInternalError",
]


class EngineError(Exception):
    """Base of every failure the engine extension reports.

    ``result`` is CNA's own result code and ``category`` its error category,
    both preserved verbatim; ``native_message`` is the diagnostic CNA produced.
    """

    def __init__(self, operation: str, result: int, category: int | None,
                 native_message: str) -> None:
        self.operation = operation
        self.result = int(result)
        self.category = None if category is None else int(category)
        self.native_message = native_message
        detail = native_message or f"{operation} failed with CNA result {result}"
        super().__init__(detail)


class EngineUnavailableError(EngineError):
    """The loaded CNA build contains no graphics extension layer.

    Measured, not guessed: ``cna_graphics_ext_is_available`` answered false.
    Every route still exists as a symbol, which is what keeps one recorded ABI
    baseline meaningful across build options, so symbol presence proves nothing
    here. :func:`cna.extensions.engine.is_available` reports the same fact
    without raising.
    """


class EngineUnsupportedError(EngineError):
    """The extension layer is present and this renderer or device cannot do it.

    A capability boundary, not a defect and not a missing binding. Where CNA
    offers a support query the public object exposes it, so a caller can ask
    before it acts rather than catching this.
    """


class EngineStateError(EngineError):
    """The object is not in a state where this operation is valid.

    Ending a debug batch that never began, for example.
    """


class EngineArgumentError(EngineError):
    """An argument violates the contract CNA documents for the route.

    Also raised for a value that cannot be represented in the native width, or
    for text that is not well-formed UTF-8.
    """


class EngineDisposedError(EngineError):
    """The object, or something it borrows, has been closed.

    Raised by this binding before reaching CNA, so a closed handle is never
    passed to native code.
    """


class EngineThreadError(EngineError):
    """The operation ran on a thread CNA does not allow it on.

    Graphics work is graphics-thread affine. This binding does not dispatch calls
    to another thread on the caller's behalf, because doing so would hide the
    contract rather than keep it.
    """


class EngineInternalError(EngineError):
    """CNA failed for a reason that is none of the above.

    Out of memory, an internal invariant, a platform service, a shutdown in
    progress, or a handle this binding should never have passed. Kept distinct
    so it is never mistaken for a statement about the caller's arguments.
    """
