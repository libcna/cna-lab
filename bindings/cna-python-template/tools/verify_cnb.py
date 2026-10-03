"""An optional smoke test for the CNA-native compiled content extension.

Deliberately small and deliberately separate. The starter's own 60- and
600-frame runs must not need `.cnb` at all: `.cnb` is CNA's compiled content
format, and a game that ships `.xnb` or loose files never touches it. Making the
basic startup depend on it would be a claim that it does.

What this proves, in about a second and with no renderer, is that the extension
is importable, that the toolchain in this environment can compile a `.cnj` source
document into a `.cnb`, and that the result decodes back to the exact value it
was authored with. One asset, one exact assertion.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import tempfile

from cna.extensions import content as cnb

#: A minimal `.cnj` curve document. Curve is the right asset for a smoke test:
#: it is self-contained, so nothing here writes a sidecar, and it decodes to a
#: value that can be asserted exactly rather than approximately.
DOCUMENT = {
    "cnjVersion": 1,
    "type": "Curve",
    "preLoop": "Constant",
    "postLoop": "Linear",
    "keys": [{"position": 0, "value": 2.5},
             {"position": 1, "value": 7.5, "continuity": "Step"}],
}


def verify(directory: Path) -> str:
    """Compiles, parses and decodes one asset, and returns a one-line result."""
    source = directory / "ease.cnj"
    source.write_text(json.dumps(DOCUMENT), encoding="utf-8")

    with cnb.compile_cnj(source, content_name="curves/ease") as compiled:
        if compiled.asset_type_id != int(cnb.AssetType.Curve):
            raise RuntimeError(
                f"expected a Curve, got {cnb.asset_type_name(compiled.asset_type_id)}")
        image = compiled.cnb_bytes

    if not cnb.has_magic(image):
        raise RuntimeError("the compiled bytes are not a .cnb file")

    with cnb.CnbDocument.parse(image, origin=str(source)) as document:
        document.require_asset(int(cnb.AssetType.Curve), cnb.CURVE_SCHEMA_VERSION)
        if document.metadata.content_name != "curves/ease":
            raise RuntimeError(
                f"the logical name did not survive: {document.metadata.content_name!r}")
        curve = cnb.decode_curve(document)

    # One exact value, chosen so a codec that dropped, reordered or rounded the
    # keys would produce something else rather than something close.
    if len(curve.Keys) != 2:
        raise RuntimeError(f"expected 2 curve keys, got {len(curve.Keys)}")
    value = curve.Evaluate(0.0)
    if value != 2.5:
        raise RuntimeError(f"expected the curve to evaluate to 2.5 at 0, got {value}")
    return f"CNB_VERIFICATION=ok compiled {len(image)} bytes, Evaluate(0)={value}"


def run() -> str:
    """Runs the check in a scratch directory and prints its one-line result.

    Separate from :func:`main` so the starter can call it without a second
    argument parser seeing the starter's own flags.
    """
    with tempfile.TemporaryDirectory(prefix="cna-template-cnb-") as directory:
        line = verify(Path(directory))
    print(f"cna-python-template: {line}")
    return line


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Compile and decode one .cnj asset through cna.extensions.content")
    parser.parse_args()
    run()


if __name__ == "__main__":
    main()
