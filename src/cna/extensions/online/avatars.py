"""What CNA adds to XNA's avatars: a native subscription to one description's changes.

CNA retired its real-rendering avatar extension at C ABI 0.33 (the standard
avatar types now behave as real avatars); what remains here is the native form of
``AvatarDescription.Changed``.
"""

from __future__ import annotations

import ctypes as c
from typing import Callable

from Microsoft.Xna.Framework.GamerServices import AvatarDescription

from _cna_native import online_abi as _online
from _cna_native import online_support as _on
from _cna_native.family_support import CallbackRoot

__all__ = ["on_avatar_description_changed"]

_support = _on.support
_roots = CallbackRoot()


def on_avatar_description_changed(description: AvatarDescription,
                                  handler: Callable[[], None]) -> int:
    """Calls ``handler()`` when CNA raises ``Changed`` for this description.

    The registration borrows the description and must be released with
    ``cna_gamer_unsubscribe_ext`` before the description is disposed. CNA
    documents that nothing in its runtime raises this event today.
    """
    if not isinstance(description, AvatarDescription):
        raise TypeError("description must be an AvatarDescription")
    if not callable(handler):
        raise TypeError("handler must be callable")

    def adapt(_context) -> None:
        handler()

    key = object()
    trampoline = _roots.root(key, _online.CNA_GamerAsyncCallback, adapt)
    try:
        registration = _support.out_handle(
            "cna_avatar_description_subscribe_changed_ext", description._value,
            trampoline, None)
    except BaseException:
        _roots.release(key)
        raise
    _roots.root(registration, _online.CNA_GamerAsyncCallback, adapt)
    _roots.release(key)
    return registration
