#!/usr/bin/env python3
"""Falsifiability controls for the native ABI verifier.

Each control plants one realistic defect of a named class, runs
`tools/native_abi/verify.py` unchanged, and requires it to fail. A gate that
has never been shown to fail is not evidence, so this is a gate of its own: a
surviving mutation is reported as a failure here.

Every mutation is a single exact textual substitution and is undone in a
`finally`, and the harness re-reads each touched file at the end to prove the
tree is byte-identical to how it started.
"""

from __future__ import annotations

import argparse
import json
import fcntl
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "Sources/CNA/Native/NativeManifest.swift"
FUNCTIONS = ROOT / "Sources/CNA/Native/NativeFunctions.swift"
SHIM = ROOT / "Sources/CNAShim/include/CNAShim.h"
PROBE = ROOT / "tools/native_abi/probe.c"
KEYS = ROOT / "Sources/CNA/Xna/Input/Keyboard.swift"
VERIFY = ROOT / "tools/native_abi/verify.py"

# Two mutation harnesses editing the same working tree at once corrupts both.
# This one mutates NativeManifest.swift, NativeFunctions.swift, CNAShim.h and
# Keyboard.swift; `tools/projection_mutations/run.py` mutates twenty other
# files under Sources/ and runs the whole test suite for each. Run them
# together and that harness's `swift test` can compile a defect planted here,
# reporting a CAUGHT its own mutation did not earn -- a false pass, which is
# the direction that hides a survivor. It happened once, during Foundation 48.
#
# The lock is advisory and holds between these two scripts only. It is not a
# claim that the tree is otherwise untouched.
TREE_LOCK = ROOT / ".mutation-gate.lock"


def acquire_tree_lock(name: str):
    """Take the exclusive tree lock, or return None if another gate holds it."""
    handle = TREE_LOCK.open("w", encoding="utf-8")
    try:
        fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        handle.close()
        return None
    handle.write(f"{os.getpid()} {name}\n")
    handle.flush()
    return handle

MUTATIONS: list[tuple[str, str, Path, str, str]] = [
    (
        # Aimed through the SYMBOL as well as the parameter pair. Foundation 67
        # bound seventy effect routes, nine of which copy a name into a caller
        # buffer with exactly this parameter pair, and the site stopped being
        # unique -- the harness reported it as a survivor rather than a stale
        # site, which is the failure the pre-check exists to make loud.
        "wrong-parameter-width", "a bound route's parameter width", MANIFEST,
        '"cna_error_copy_last_message", swiftField: "errorMessageCopy", '
        'routeType: "ErrorCopyLastMessageRoute", cReturn: "CNA_Result", '
        'cParameters: ["char* destination", "uint64_t capacity"',
        '"cna_error_copy_last_message", swiftField: "errorMessageCopy", '
        'routeType: "ErrorCopyLastMessageRoute", cReturn: "CNA_Result", '
        'cParameters: ["char* destination", "uint32_t capacity"',
    ),
    (
        "wrong-parameter-spelling", "a canonical parameter type recorded as a compatible alias",
        MANIFEST,
        '"cna_graphics_device_manager_apply_changes", swiftField: "graphicsManagerApplyChanges", routeType: "GraphicsDeviceManagerApplyChangesRoute", cReturn: "CNA_Result", cParameters: ["CNA_GraphicsDeviceManagerHandle manager"]',
        '"cna_graphics_device_manager_apply_changes", swiftField: "graphicsManagerApplyChanges", routeType: "GraphicsDeviceManagerApplyChangesRoute", cReturn: "CNA_Result", cParameters: ["CNA_Handle manager"]',
    ),
    (
        "stale-symbol", "a bound symbol that no longer exists", MANIFEST,
        'symbol: "cna_game_run", swiftField:',
        'symbol: "cna_game_run_retired", swiftField:',
    ),
    (
        "swapped-route-symbol", "one property resolving another route's symbol", FUNCTIONS,
        'gameRun = try library.resolve("cna_game_run", as: GameRunRoute.self)',
        'gameRun = try library.resolve("cna_game_run_one_frame", as: GameRunRoute.self)',
    ),
    (
        "swapped-route-type", "one property resolving through another route's type", FUNCTIONS,
        'gameRunOneFrame = try library.resolve("cna_game_run_one_frame", as: GameRunOneFrameRoute.self)',
        'gameRunOneFrame = try library.resolve("cna_game_run_one_frame", as: GameRunRoute.self)',
    ),
    (
        "shared-route-type", "two routes sharing one structurally identical type", FUNCTIONS,
        "    let gameRequestExit: GameRequestExitRoute",
        "    let gameRequestExit: GameRunRoute",
    ),
    (
        "wrong-swift-position-width", "a Swift route position with the wrong width", FUNCTIONS,
        "typealias GamepadSetVibrationRoute = @convention(c) (UInt64, UInt32, Float, Float, UnsafeMutablePointer<UInt8>?) -> UInt32",
        "typealias GamepadSetVibrationRoute = @convention(c) (UInt64, UInt32, Double, Float, UnsafeMutablePointer<UInt8>?) -> UInt32",
    ),
    (
        "wrong-abi-window", "an admitted ABI window the canonical header does not satisfy", FUNCTIONS,
        "static let minimumMinor: UInt32 = 35",
        "static let minimumMinor: UInt32 = 99",
    ),
    (
        "omitted-struct-field", "a mirrored structure missing one canonical field", SHIM,
        "    uint32_t pressed_buttons;\n    uint32_t reserved1;\n",
        "    uint32_t pressed_buttons;\n",
    ),
    (
        "reordered-struct-fields", "two mirrored fields transposed", SHIM,
        "    int32_t packet_number;\n    uint32_t pressed_buttons;\n",
        "    uint32_t pressed_buttons;\n    int32_t packet_number;\n",
    ),
    (
        "narrowed-struct-field", "a mirrored field of the wrong width", SHIM,
        "    uint64_t pressed_key_words[4];",
        "    uint32_t pressed_key_words[4];",
    ),
    (
        "wrong-callback-signature", "a mirrored callback missing one parameter", SHIM,
        "typedef CNASwift_Result (*CNASwift_GameBeginDrawCallback)(\n"
        "    CNASwift_Handle game,\n"
        "    const CNASwift_GameTime* game_time,\n"
        "    void* context,\n"
        "    CNASwift_Bool* out_should_draw,\n"
        "    CNASwift_CallbackError* out_error);",
        "typedef CNASwift_Result (*CNASwift_GameBeginDrawCallback)(\n"
        "    CNASwift_Handle game,\n"
        "    const CNASwift_GameTime* game_time,\n"
        "    void* context,\n"
        "    CNASwift_CallbackError* out_error);",
    ),
    (
        "wrong-constant", "a canonical constant value", PROBE,
        "_Static_assert(CNA_GAMEPAD_BUTTON_A == 4096, \"Buttons.A\");",
        "_Static_assert(CNA_GAMEPAD_BUTTON_A == 4097, \"Buttons.A\");",
    ),
    (
        "wrong-key-constant", "a projected Keys literal", KEYS,
        "case Space = 32",
        "case Space = 33",
    ),
]


def run_verifier(include: Path, library: Path, cc: str) -> tuple[int, str]:
    result = subprocess.run(
        [sys.executable, str(VERIFY), "--cna-include", str(include),
         "--library", str(library), "--cc", cc],
        text=True, capture_output=True,
    )
    return result.returncode, (result.stdout + result.stderr)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cna-include", required=True, type=Path)
    parser.add_argument("--library", required=True, type=Path)
    parser.add_argument("--cc", default="cc")
    parser.add_argument("--output", type=Path,
                        help="write the run's outcome as JSON. Until this "
                             "existed the caught/survivor counts reached the "
                             "documents by hand off a terminal line, which is "
                             "the one path the status gate cannot police.")
    args = parser.parse_args()
    tree_lock = acquire_tree_lock("native_abi_mutations")
    if tree_lock is None:
        print("MUTATION_GATE=BUSY — another mutation harness holds "
              f"{TREE_LOCK.name}; these two gates cannot share a working tree")
        return 1


    originals = {path: path.read_text(encoding="utf-8") for path in
                 {MANIFEST, FUNCTIONS, SHIM, PROBE, KEYS}}

    baseline_code, baseline_output = run_verifier(args.cna_include, args.library, args.cc)
    if baseline_code != 0:
        print("MUTATION_BASELINE=RED — the unmutated tree already fails:")
        print(baseline_output)
        return 1

    survivors: list[str] = []
    for name, description, path, old, new in MUTATIONS:
        text = originals[path]
        if text.count(old) != 1:
            survivors.append(f"{name}: the mutation site occurs {text.count(old)} times, not once")
            continue
        try:
            path.write_text(text.replace(old, new), encoding="utf-8")
            code, _ = run_verifier(args.cna_include, args.library, args.cc)
        finally:
            path.write_text(text, encoding="utf-8")
        status = "CAUGHT" if code != 0 else "SURVIVED"
        print(f"{status:9} {name:28} {description}")
        if code == 0:
            survivors.append(f"{name}: {description}")

    for path, text in originals.items():
        if path.read_text(encoding="utf-8") != text:
            survivors.append(f"{path} was not restored")

    # See the note in tools/projection_mutations/run.py: a bare CAUGHT= meant
    # two different numbers across the documents, so neither was policed.
    caught = len(MUTATIONS) - len(
        [s for s in survivors if not s.endswith("restored")])
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps({
            "NATIVE_ABI_MUTATIONS": len(MUTATIONS),
            "NATIVE_ABI_MUTATIONS_CAUGHT": caught,
            "NATIVE_ABI_MUTATION_SURVIVORS": len(survivors),
            "survivors": survivors,
        }, indent=2) + "\n", encoding="utf-8")
    print(f"NATIVE_ABI_MUTATIONS={len(MUTATIONS)} "
          f"NATIVE_ABI_MUTATIONS_CAUGHT={caught} "
          f"NATIVE_ABI_MUTATION_SURVIVORS={len(survivors)}")
    for survivor in survivors:
        print(f"  SURVIVOR {survivor}")
    return 1 if survivors else 0


if __name__ == "__main__":
    raise SystemExit(main())
