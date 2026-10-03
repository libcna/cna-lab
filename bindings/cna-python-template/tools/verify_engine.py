"""An optional smoke test for CNA's graphics extension layer.

Deliberately small and deliberately separate. The starter's own 60- and
600-frame runs must not need it: ``cna.extensions.engine`` is a CNA-only
vocabulary (CNA kept DebugDraw and the ASCII effect when it retired its engine
layer at C ABI 0.30), and a game drawing sprites never touches it.

What this proves, in one real frame, is that the package is importable from the
installed wheel, that the loaded CNA build reports whether it carries the layer,
and -- when it does -- that a debug batch builds exactly the lines its shapes
are made of and an ASCII effect keeps the cell size it was given.

**A build without the layer is a result, not a failure.** CNA can be configured
without ``CNA_CNAEXT``, and every route then reports itself unavailable. This
says so and exits cleanly, because "absent" and "broken" are different facts.
"""

from __future__ import annotations

import argparse

from Microsoft.Xna.Framework import (
    BoundingBox, Color, Game, GraphicsDeviceManager, Matrix, Vector3,
)
from cna.extensions import engine


def _measure_in_one_frame() -> dict:
    observed: dict = {}
    failure: list[BaseException] = []

    class Probe(Game):
        def __init__(self) -> None:
            super().__init__()
            self.manager = GraphicsDeviceManager(self)
            self.done = False

        def Draw(self, gameTime) -> None:
            if self.done:
                return
            self.done = True
            try:
                device = self.GraphicsDevice
                with engine.DebugDraw(device) as debug:
                    debug.begin(Matrix.Identity, Matrix.Identity)
                    debug.add_box(BoundingBox(Vector3(0, 0, 0), Vector3(1, 2, 3)), Color.Red)
                    debug.add_cross(Vector3(0, 0, 0), 1.0, Color.Green)
                    observed["lines"] = debug.line_count
                    observed["vertices"] = len(debug.vertices(True))
                with engine.AsciiEffect(device) as ascii_effect:
                    ascii_effect.cell_size = (6, 10)
                    observed["cell"] = ascii_effect.cell_size
            except BaseException as error:  # re-raised outside the frame
                failure.append(error)
            self.Exit()

        def Update(self, gameTime) -> None:
            if self.done:
                self.Exit()

    game = Probe()
    try:
        game.Run()
    finally:
        game.Dispose()
    if failure:
        raise failure[0]
    return observed


def verify() -> str:
    """Checks the extension layer, and returns a one-line result."""
    probe = _availability()
    if not probe:
        return ("ENGINE_VERIFICATION=absent this CNA build has no graphics extension "
                "layer, which is a supported configuration")
    observed = _measure_in_one_frame()
    expected_lines = engine.DEBUG_DRAW_BOX_EDGE_COUNT + 3
    if observed.get("lines") != expected_lines:
        raise RuntimeError(f"a box and a cross built {observed.get('lines')} lines, "
                           f"not {expected_lines}")
    if observed.get("vertices") != 2 * expected_lines:
        raise RuntimeError(f"the depth-tested list holds {observed.get('vertices')} "
                           f"vertices, not {2 * expected_lines}")
    if observed.get("cell") != (6, 10):
        raise RuntimeError(f"the ASCII effect kept cell size {observed.get('cell')}")
    return (f"ENGINE_VERIFICATION=ok debug lines {observed['lines']}, "
            f"ASCII cell {observed['cell'][0]}x{observed['cell'][1]}")


def _availability() -> bool:
    """Whether the layer exists; asked inside a live game, as CNA's routes need one."""
    answer: list[bool] = []

    class Ask(Game):
        def __init__(self) -> None:
            super().__init__()
            self.manager = GraphicsDeviceManager(self)

        def Update(self, gameTime) -> None:
            answer.append(engine.is_available())
            self.Exit()

    game = Ask()
    try:
        game.Run()
    finally:
        game.Dispose()
    return bool(answer and answer[0])


def run() -> str:
    """Runs the check and prints its one-line result."""
    line = verify()
    print(f"cna-python-template: {line}")
    return line


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Check CNA's graphics extension layer through cna.extensions.engine")
    parser.parse_args()
    run()


if __name__ == "__main__":
    main()
