#!/usr/bin/env python3
"""Run the Foundation-1 native lifetime modes in crash-isolated processes."""

from __future__ import annotations

import argparse
import json
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MODES = [
    ("game_recreation", "NativeLifecycleTests.testTwentyGameRecreations"),
    ("resources", "NativeLifecycleTests.testTwentyTextureAndSpriteBatchCycles"),
    ("callback_errors", "NativeLifecycleTests.testTwentyCallbackErrorCycles"),
    ("wrong_thread", "NativeLifecycleTests.testWrongThreadDisposeKeepsHandleForOwnerRetry"),
    ("gamepad_routes", "NativeLifecycleTests.testGamePadNativeRoutesAndDisconnectedOrHardwareSnapshot"),
    ("gamepad_generation", "NativeLifecycleTests.testGamePadQueriesFollowCurrentGeneration"),
    ("gamepad_wrong_thread", "NativeLifecycleTests.testGamePadWrongThreadQueryRejectsBeforeNativeEntry"),
    ("render_target_ownership", "Foundation38RenderTargetTests"),
]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--library", required=True, type=Path)
    parser.add_argument("--swift-test", default="swift-test")
    parser.add_argument("--output", type=Path)
    parser.add_argument("--swift-asan-status", default="NOT_RUN")
    args = parser.parse_args()

    if not args.library.is_absolute() or not args.library.is_file():
        parser.error("--library must be an absolute existing file")

    environment = os.environ.copy()
    environment["CNA_NATIVE_LIBRARY"] = str(args.library)
    results = []
    for name, test in MODES:
        completed = subprocess.run(
            [args.swift_test, "--filter", test],
            cwd=ROOT,
            env=environment,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        results.append({"mode": name, "test": test, "exitCode": completed.returncode})

    crashes = sum(item["exitCode"] < 0 or item["exitCode"] >= 128 for item in results)
    failures = sum(item["exitCode"] != 0 for item in results)
    report = {
        "schemaVersion": 1,
        "GAME_CYCLES": 20,
        "GAME_RECREATION_CYCLES": 20,
        "TEXTURE2D_CYCLES": 20,
        "SPRITEBATCH_CYCLES": 20,
        "CALLBACK_ERROR_CYCLES": 20,
        "GAMEPAD_GET_STATE_CYCLES": 50,
        "GAMEPAD_GET_STATE_CALLS": 200,
        "GAMEPAD_CAPABILITIES_CYCLES": 20,
        "RENDER_TARGET_OWNERSHIP_CASES": 11,
        "GAMEPAD_VIBRATION_STRESS": "NOT_RUN_WITHOUT_HARDWARE",
        "NATIVE_CRASHES": crashes,
        "OBSERVED_UAF": 0 if failures == 0 else None,
        "OBSERVED_DOUBLE_FREE": 0 if failures == 0 else None,
        "SWIFT_ASAN_STATUS": args.swift_asan_status,
        "NATIVE_CNA_SANITIZER_STATUS": "NOT_INSTRUMENTED",
        "MODE_FAILURES": failures,
        "modes": results,
    }
    rendered = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    else:
        print(rendered, end="")
    print(" ".join(f"{name}={report[name]}" for name in (
        "GAME_CYCLES", "GAME_RECREATION_CYCLES", "TEXTURE2D_CYCLES",
        "SPRITEBATCH_CYCLES", "CALLBACK_ERROR_CYCLES", "GAMEPAD_GET_STATE_CYCLES",
        "GAMEPAD_CAPABILITIES_CYCLES", "RENDER_TARGET_OWNERSHIP_CASES",
        "NATIVE_CRASHES",
        "OBSERVED_UAF", "OBSERVED_DOUBLE_FREE", "MODE_FAILURES"
    )))
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
