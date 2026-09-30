#!/usr/bin/env python3
"""Qualify the canonical CNA GamePad routes in crash-isolated Swift tests."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ROUTE_TEST = "NativeLifecycleTests.testGamePadNativeRoutesAndDisconnectedOrHardwareSnapshot"
SUPPORT_TESTS = (
    ("generation", "NativeLifecycleTests.testGamePadQueriesFollowCurrentGeneration"),
    ("wrongThread", "NativeLifecycleTests.testGamePadWrongThreadQueryRejectsBeforeNativeEntry"),
)


def run_test(executable: str, test: str, environment: dict[str, str]) -> dict[str, object]:
    completed = subprocess.run(
        [executable, "--filter", test],
        cwd=ROOT,
        env=environment,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    return {
        "test": test,
        "exitCode": completed.returncode,
        "crashed": completed.returncode < 0 or completed.returncode >= 128,
    }


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--library", required=True, type=Path)
    parser.add_argument("--swift-test", default="swift-test")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    if not args.library.is_absolute() or not args.library.is_file():
        parser.error("--library must be an absolute existing file")

    environment = os.environ.copy()
    environment["CNA_NATIVE_LIBRARY"] = str(args.library)
    (ROOT / "build-probe").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="cna-swift-gamepad-", dir=ROOT / "build-probe") as directory:
        snapshot_path = Path(directory) / "snapshot.json"
        route_environment = environment.copy()
        route_environment["CNA_GAMEPAD_EVIDENCE_OUTPUT"] = str(snapshot_path)
        route_result = run_test(args.swift_test, ROUTE_TEST, route_environment)
        snapshot = (
            json.loads(snapshot_path.read_text(encoding="utf-8"))
            if snapshot_path.is_file()
            else {}
        )

    support = {
        name: run_test(args.swift_test, test, environment)
        for name, test in SUPPORT_TESTS
    }
    failures = int(route_result["exitCode"] != 0) + sum(
        int(result["exitCode"] != 0) for result in support.values()
    )
    connected = bool(snapshot.get("connected", False))
    route_status = "VERIFIED_NATIVE" if connected else "HARDWARE_PENDING"
    state_routes = (
        ("GetState default", "cna_gamepad_get_state", "default", "IndependentAxes"),
        ("GetState None", "cna_gamepad_get_state_with_dead_zone", "none", "None"),
        ("GetState IndependentAxes", "cna_gamepad_get_state_with_dead_zone", "independentAxes", "IndependentAxes"),
        ("GetState Circular", "cna_gamepad_get_state_with_dead_zone", "circular", "Circular"),
    )
    routes: list[dict[str, object]] = []
    for operation, symbol, key, mode in state_routes:
        routes.append({
            "operation": operation,
            "canonicalSymbol": symbol,
            "playerSlot": snapshot.get("playerSlot", 0),
            "deadZoneMode": mode,
            "cnaResult": "SUCCESS" if route_result["exitCode"] == 0 else "TEST_FAILED",
            "returned": snapshot.get(key),
            "routeStatus": "VERIFIED_NATIVE_ROUTE" if route_result["exitCode"] == 0 else "FAILED",
            "positiveHardwareStatus": route_status,
            "proven": (
                "canonical route returned a real controller snapshot"
                if connected
                else "canonical route returned its real disconnected snapshot; no state was synthesized"
            ),
        })
    routes.extend((
        {
            "operation": "GetCapabilities",
            "canonicalSymbol": "cna_gamepad_get_capabilities",
            "playerSlot": snapshot.get("playerSlot", 0),
            "cnaResult": "SUCCESS" if route_result["exitCode"] == 0 else "TEST_FAILED",
            "returned": snapshot.get("capabilities"),
            "routeStatus": "VERIFIED_NATIVE_ROUTE" if route_result["exitCode"] == 0 else "FAILED",
            "positiveHardwareStatus": route_status,
            "proven": (
                "canonical route returned its per-control capability snapshot"
                if connected
                else "canonical route returned its real all-false disconnected capabilities"
            ),
        },
        {
            "operation": "SetVibration",
            "canonicalSymbol": "cna_gamepad_set_vibration",
            "playerSlot": snapshot.get("playerSlot", 0),
            "requested": {"left": 0.0, "right": 0.0},
            "cnaResult": "SUCCESS" if route_result["exitCode"] == 0 else "TEST_FAILED",
            "applied": snapshot.get("vibrationApplied"),
            "routeStatus": "VERIFIED_NATIVE_ROUTE" if route_result["exitCode"] == 0 else "FAILED",
            "positiveHardwareStatus": route_status,
            "proven": (
                "canonical device acceptance result was returned"
                if connected
                else "canonical disconnected route returned applied=false; no success was fabricated"
            ),
        },
    ))

    report = {
        "schemaVersion": 1,
        "authority": "CANONICAL_CNA_NATIVE_QUALIFICATION",
        # Only the artifact identity is retained. The absolute developer path is
        # deliberately not recorded: the exact source archive audit rejects
        # developer path leaks, and identity is the reproducible fact.
        "library": args.library.name,
        "librarySHA256": sha256(args.library),
        "backend": snapshot.get("backend", "UNKNOWN"),
        "hardwareAvailable": connected,
        "positiveStatePath": "VERIFIED" if connected else "HARDWARE_PENDING",
        "positiveCapabilityPath": "VERIFIED" if connected else "HARDWARE_PENDING",
        "positiveVibrationPath": "VERIFIED" if connected and snapshot.get("vibrationApplied") else "HARDWARE_PENDING",
        "routeTest": route_result,
        "routes": routes,
        "stress": {
            "stateCyclesPerDeadZoneRoute": snapshot.get("stateCycles", 0),
            "totalStateCalls": int(snapshot.get("stateCycles", 0)) * 4,
            "capabilityCycles": snapshot.get("capabilityCycles", 0),
            "repeatedVibrationStress": "NOT_RUN_WITHOUT_HARDWARE",
        },
        "generation": support["generation"],
        "wrongThread": support["wrongThread"],
        "failures": failures,
    }
    rendered = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    else:
        print(rendered, end="")
    print(
        f"GAMEPAD_NATIVE_FAILURES={failures} "
        f"HARDWARE_AVAILABLE={'YES' if connected else 'NO'} "
        f"GAMEPAD_GET_STATE_CYCLES={snapshot.get('stateCycles', 0)} "
        f"GAMEPAD_CAPABILITIES_CYCLES={snapshot.get('capabilityCycles', 0)}"
    )
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
