"""Independent expectations for the debug drawer's line counts.

Every function here computes what CNA should answer **without calling CNA**,
from the rule CNA's own source states rather than from its output. A getter is
never allowed to be its own oracle.

An oracle that is wrong in the same way as the implementation proves nothing, so
these are gated too: :mod:`tests.test_engine_oracles` checks each one against a
worked case computed by hand in the test itself, and the mutation harness plants
defects here to prove that a wrong oracle is caught rather than agreed with.

Everything is plain Python and needs no native library.
"""

from __future__ import annotations


# --- debug drawing -------------------------------------------------------------

#: A box and a frustum have the same twelve edges: XNA numbers their corners the
#: same way, so one edge table draws both.
DEBUG_BOX_EDGES = 12
DEBUG_MINIMUM_SEGMENTS, DEBUG_MAXIMUM_SEGMENTS = 4, 128
DEBUG_DEFAULT_SEGMENTS = 24


def debug_segments(requested: int) -> int:
    """A ring's segment count, clamped rather than refused."""
    return min(max(requested, DEBUG_MINIMUM_SEGMENTS), DEBUG_MAXIMUM_SEGMENTS)


def debug_sphere_lines(segments: int = DEBUG_DEFAULT_SEGMENTS) -> int:
    """Three rings, one per axis pair, each of the clamped segment count."""
    return 3 * debug_segments(segments)


def debug_cross_lines() -> int:
    """Three axis-aligned segments through a point."""
    return 3
