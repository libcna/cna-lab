#!/usr/bin/env python3
"""Prove the compiled public-surface gate kills a ContentReader helper leak.

Unlike the verifier's model-level self-test, this mutation changes the real
Swift source, asks the compiler for a fresh Symbol Graph, and then restores the
source byte-for-byte.  The mutation lock is shared with the other source
mutation harnesses so their deliberately broken trees can never overlap.
"""

from __future__ import annotations

import argparse
import fcntl
import hashlib
import json
import os
import signal
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESOURCE_MANAGER = ROOT / "Sources/CNA/CNAResourceManager.swift"
VERIFY = ROOT / "tools/api_compat/verify.py"
TREE_LOCK = ROOT / ".mutation-gate.lock"
OLD = "    internal static let resourceNotString ="
NEW = "    public static let resourceNotString ="
SUBJECT = "CNAResourceManager.resourceNotString"


def run(command: list[str], timeout: int) -> subprocess.CompletedProcess[str]:
    process = subprocess.Popen(
        command,
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        start_new_session=True,
    )
    try:
        stdout, stderr = process.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGTERM)
        try:
            stdout, stderr = process.communicate(timeout=10)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            stdout, stderr = process.communicate()
        raise RuntimeError(
            "HUNG\n" + " ".join(command) + "\n" + stdout + "\n" + stderr)
    return subprocess.CompletedProcess(command, process.returncode, stdout, stderr)


def dump_symbol_graph(swift: Path, timeout: int) -> None:
    result = run([
        "taskset", "-c", "0-2", str(swift), "package", "dump-symbol-graph",
    ], timeout)
    if result.returncode != 0:
        raise RuntimeError(
            "symbol-graph generation failed\n" + result.stdout + result.stderr)


def diagnostics(symbol_graph: Path, timeout: int) -> list[dict[str, object]]:
    (ROOT / "build-probe").mkdir(exist_ok=True)
    with tempfile.NamedTemporaryFile(
        prefix="cna-swift-content-reader-leak-", suffix=".json", delete=True,
        dir=ROOT / "build-probe",
    ) as output:
        result = run([
            sys.executable, str(VERIFY),
            "--symbol-graph", str(symbol_graph),
            "--output", output.name,
        ], timeout)
        # The global strict report deliberately remains red for unselected
        # families, so only a readable report is required here.
        if result.returncode not in (0, 1):
            raise RuntimeError("verifier failed\n" + result.stdout + result.stderr)
        return json.loads(Path(output.name).read_text(encoding="utf-8"))["diagnostics"]


def has_leak(items: list[dict[str, object]]) -> bool:
    return any(
        item.get("category") == "BASE_MAPPING_MISMATCH"
        and item.get("subject") == SUBJECT
        for item in items
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--swift", type=Path, required=True)
    parser.add_argument("--symbol-graph", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=300)
    args = parser.parse_args()

    with TREE_LOCK.open("w", encoding="utf-8") as lock:
        try:
            fcntl.flock(lock.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError:
            print("HUNG another source-mutation gate holds .mutation-gate.lock")
            return 2
        lock.write(f"{os.getpid()} content_reader_public_leak\n")
        lock.flush()

        original = RESOURCE_MANAGER.read_bytes()
        original_hash = hashlib.sha256(original).hexdigest()
        source = original.decode("utf-8")
        if source.count(OLD) != 1 or NEW in source:
            print("UNSCORED the exact internal helper anchor is absent or ambiguous")
            return 2

        verdict = "UNSCORED"
        try:
            dump_symbol_graph(args.swift, args.timeout)
            if has_leak(diagnostics(args.symbol_graph, args.timeout)):
                print("UNSCORED baseline already exposes the helper")
                return 2

            RESOURCE_MANAGER.write_text(source.replace(OLD, NEW), encoding="utf-8")
            dump_symbol_graph(args.swift, args.timeout)
            verdict = (
                "CAUGHT" if has_leak(diagnostics(args.symbol_graph, args.timeout))
                else "SURVIVED"
            )
        except RuntimeError as error:
            if str(error).startswith("HUNG"):
                verdict = "HUNG"
            print(str(error), file=sys.stderr)
        finally:
            RESOURCE_MANAGER.write_bytes(original)
            # Leave generated compiler evidence clean as well as the source.
            try:
                dump_symbol_graph(args.swift, args.timeout)
            except RuntimeError as error:
                print(str(error), file=sys.stderr)
                verdict = "HUNG"

        restored_hash = hashlib.sha256(RESOURCE_MANAGER.read_bytes()).hexdigest()
        restored = restored_hash == original_hash
        print("CONTENT_READER_PUBLIC_LEAK_MUTATIONS=1")
        print(f"CONTENT_READER_PUBLIC_LEAK_CAUGHT={int(verdict == 'CAUGHT')}")
        print(f"CONTENT_READER_PUBLIC_LEAK_SURVIVED={int(verdict == 'SURVIVED')}")
        print(f"CONTENT_READER_PUBLIC_LEAK_HUNG={int(verdict == 'HUNG')}")
        print(f"CONTENT_READER_PUBLIC_LEAK_UNSCORED={int(verdict == 'UNSCORED')}")
        print(f"CONTENT_READER_PUBLIC_LEAK_SOURCE_RESTORED={int(restored)}")
        print(f"VERDICT={verdict}")
        return 0 if verdict == "CAUGHT" and restored else 1


if __name__ == "__main__":
    raise SystemExit(main())
