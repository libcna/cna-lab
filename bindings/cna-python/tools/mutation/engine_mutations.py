#!/usr/bin/env python3
"""Plants one defect at a time in ``cna.extensions.engine`` and reports which test kills it.

A test suite that has never been shown to fail is a suite nobody has measured.
Each mutation below is a plausible mistake in the wrapper or in its oracle, and
each one must make a *focused* test fail, not merely something somewhere. The
package shrank to DebugDraw and the ASCII effect when CNA retired its engine layer
at C ABI 0.30, and so did this list.

Needs ``CNA_NATIVE_LIBRARY`` pointing at a build with the graphics extension layer
(``CNA_CNAEXT``) on a rasterizing renderer: without one every test here skips and
every mutation would survive.
"""

from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

#: (label, file, old, new, the test module the kill is expected in)
MUTATIONS = [
    # --- debug drawing --------------------------------------------------------
    ("debug: count a sphere as one ring rather than three",
     "tests/engine_oracles.py",
     "    return 3 * debug_segments(segments)",
     "    return debug_segments(segments)",
     "tests.test_engine_debug"),
    ("debug: clamp the segment count to the wrong bounds",
     "tests/engine_oracles.py",
     "    return min(max(requested, DEBUG_MINIMUM_SEGMENTS), DEBUG_MAXIMUM_SEGMENTS)",
     "    return requested",
     "tests.test_engine_debug"),
    ("debug wrapper: always read the depth-tested list",
     "src/cna/extensions/engine/debug.py",
     "            (self._handle.argument, c.c_uint8(1 if depth_tested else 0)))",
     "            (self._handle.argument, c.c_uint8(1)))",
     "tests.test_engine_debug"),
    ("debug wrapper: send the frustum as the identity",
     "src/cna/extensions/engine/debug.py",
     "        native.matrix = _native_matrix(frustum.Matrix)",
     "        native.matrix = _native_matrix(Matrix.Identity)",
     "tests.test_engine_debug"),
    # --- the ASCII effect -----------------------------------------------------
    ("ascii effect: ignore the destination rectangle",
     "src/cna/extensions/engine/ascii.py",
     "                      None if rectangle is None else c.byref(rectangle))",
     "                      None)",
     "tests.test_engine_debug"),
    ("ascii effect: swap the cell width and height",
     "src/cna/extensions/engine/ascii.py",
     "        return int(width.value), int(height.value)",
     "        return int(height.value), int(width.value)",
     "tests.test_engine_debug"),
    # --- availability ---------------------------------------------------------
    ("availability: report every build as having the layer",
     "src/_cna_native/engine_support.py",
     "    return value.value != 0",
     "    return True",
     "tests.test_engine_debug"),
]


def run(module: str) -> tuple[bool, str]:
    environment = dict(os.environ)
    environment["PYTHONPATH"] = f"{ROOT / 'src'}{os.pathsep}{ROOT}"
    completed = subprocess.run(
        [sys.executable, "-m", "unittest", module],
        cwd=str(ROOT), env=environment, capture_output=True, text=True, timeout=1800)
    return completed.returncode == 0, completed.stderr


def main() -> int:
    killed, survived, inapplicable = [], [], []
    for label, relative, old, new, module in MUTATIONS:
        path = ROOT / relative
        original = path.read_text(encoding="utf-8")
        if original.count(old) != 1:
            inapplicable.append((label, f"anchor appears {original.count(old)} times"))
            continue
        path.write_text(original.replace(old, new), encoding="utf-8")
        try:
            for cache in ROOT.rglob("__pycache__"):
                subprocess.run(["rm", "-rf", str(cache)], check=False)
            passed, stderr = run(module)
        finally:
            path.write_text(original, encoding="utf-8")
            for cache in ROOT.rglob("__pycache__"):
                subprocess.run(["rm", "-rf", str(cache)], check=False)
        if passed:
            survived.append((label, module))
        else:
            names = [line.split(" ")[1] for line in stderr.splitlines()
                     if line.startswith(("FAIL: ", "ERROR: "))]
            killed.append((label, module, names[:3]))

    print(f"PLANTED={len(MUTATIONS)}")
    print(f"KILLED={len(killed)}")
    print(f"SURVIVED={len(survived)}")
    print(f"INAPPLICABLE={len(inapplicable)}")
    for label, module, names in killed:
        print(f"  KILLED  {label}\n            by {module}: {', '.join(names) or '(import)'}")
    for label, module in survived:
        print(f"  SURVIVED {label} (ran {module})")
    for label, reason in inapplicable:
        print(f"  SKIPPED  {label}: {reason}")
    return 1 if survived or inapplicable else 0


if __name__ == "__main__":
    raise SystemExit(main())
