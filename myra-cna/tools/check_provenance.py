#!/usr/bin/env python3
"""Validate that every direct source translation has a manifest entry.

The checker deliberately recognises only the required ``Ported from:`` marker.
Original Myra-CNA bootstrap code does not need an upstream source entry. Once a
source file declares itself as ported, a matching destination path must appear
in UPSTREAM_MANIFEST.md and the header must direct redistributors to NOTICE.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys


SOURCE_SUFFIXES = {".cpp", ".cxx", ".cc", ".hpp", ".h"}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    args = parser.parse_args()

    root = args.root.resolve()
    manifest_path = root / "UPSTREAM_MANIFEST.md"
    notice_path = root / "NOTICE.md"
    errors: list[str] = []

    if not manifest_path.is_file():
        errors.append("UPSTREAM_MANIFEST.md is missing")
        manifest_text = ""
    else:
        manifest_text = manifest_path.read_text(encoding="utf-8")

    if not notice_path.is_file():
        errors.append("NOTICE.md is missing")

    for directory in (root / "include", root / "src"):
        if not directory.exists():
            continue
        for path in sorted(directory.rglob("*")):
            if not path.is_file() or path.suffix not in SOURCE_SUFFIXES:
                continue
            text = path.read_text(encoding="utf-8")
            if "Ported from:" not in text:
                continue
            relative = path.relative_to(root).as_posix()
            if relative not in manifest_text:
                errors.append(f"{relative}: missing UPSTREAM_MANIFEST.md entry")
            if "NOTICE.md" not in text:
                errors.append(f"{relative}: ported header does not reference NOTICE.md")

    if errors:
        print("Myra-CNA provenance check failed:", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1

    print("Myra-CNA provenance check passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
