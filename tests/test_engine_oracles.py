"""The debug-drawer oracles, checked against worked cases computed by hand."""

from __future__ import annotations

import unittest

from . import engine_oracles as oracle


class DebugLineCountTests(unittest.TestCase):
    """Every debug shape's line count is exact, which is what makes it testable."""

    def test_segments_clamp_rather_than_refuse(self) -> None:
        self.assertEqual(oracle.debug_segments(0), 4)
        self.assertEqual(oracle.debug_segments(-10), 4)
        self.assertEqual(oracle.debug_segments(4), 4)
        self.assertEqual(oracle.debug_segments(24), 24)
        self.assertEqual(oracle.debug_segments(128), 128)
        self.assertEqual(oracle.debug_segments(1000), 128)

    def test_a_sphere_is_three_rings(self) -> None:
        self.assertEqual(oracle.debug_sphere_lines(8), 24)
        self.assertEqual(oracle.debug_sphere_lines(), 72)
        self.assertEqual(oracle.debug_sphere_lines(1), 12)


if __name__ == "__main__":  # pragma: no cover
    unittest.main()
