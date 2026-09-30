#!/usr/bin/env python3
"""Adds routes to ``bindings.json`` from the live headers, with stated ownership.

The manifest is the list of native symbols this binding demands from
``libcna_c_api``, and it is checked against the live headers on every build.
Everything mechanical about an entry -- its header, return type and parameter
types -- is therefore read from the header rather than typed, so a manifest
entry cannot describe a signature CNA does not have.  The one thing that
cannot be derived is ``ownership``: who owns what after the call, which is
prose, is the reason the manifest is reviewed at all, and is supplied here.

    python3 tools/native-abi/add_binding.py --cna-root ../cna \
        cna_frustum_culler_ext_create="no handle; returns an owned culler handle"
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from inventory import inventory  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "tools/native-abi/bindings.json"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cna-root", required=True)
    parser.add_argument("entries", nargs="+",
                        help="symbol=ownership prose, one per route")
    arguments = parser.parse_args()

    catalogue = inventory(Path(arguments.cna_root) / "modules/c-api/include")["functions"]
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    known = {entry["name"] for entry in manifest["functions"]}

    added = 0
    for entry in arguments.entries:
        symbol, _, ownership = entry.partition("=")
        if not ownership:
            print(f"MISSING_OWNERSHIP={symbol}")
            return 1
        if symbol not in catalogue:
            print(f"NOT_IN_HEADERS={symbol}")
            return 1
        if symbol in known:
            print(f"ALREADY_BOUND={symbol}")
            continue
        declaration = catalogue[symbol]
        manifest["functions"].append({
            "name": symbol,
            "header": declaration["header"],
            "ownership": ownership,
            "returnType": declaration["returnType"],
            "parameters": [parameter["type"] for parameter in declaration["parameters"]],
        })
        added += 1

    manifest["functions"].sort(key=lambda function: function["name"])
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"ADDED={added} TOTAL={len(manifest['functions'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
