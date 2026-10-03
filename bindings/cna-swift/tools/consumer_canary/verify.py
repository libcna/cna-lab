#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run the template's canary and check what it printed.

Every suite in this repository disposes what it creates, so no test could see
the defect Foundation 101 found: `ContentManager` leaked its native handle when
a consumer dropped it, and `cna_game_destroy` refused the game around it. The
one thing here that behaves like a game -- the template's `HelloGame` -- had
been failing on every run for that reason, with its README recording the
verdict line and not the failure printed under it.

Nothing ran it. A canary nobody launches is a canary in a sealed jar, so this
launches it.

**What is checked, and what deliberately is not.** The canary's verdict line
mixes three kinds of field:

  * invariants of the qualified runtime -- the viewport, the adapter count,
    every `...Refused=true` -- which must hold on any host that qualifies;
  * counts tied to the request, where `draws` is exactly `requested` but
    `updates` is only `>=` it, because CNA catches a fixed time step up with
    extra `Update` calls that carry no `Draw`. Three runs at 600 frames
    answered 600, 601 and 602, and the template's README used to pin one of
    those as though it were the value;
  * host facts -- how many pictures the media library holds, whether a
    controller is attached -- which are properties of this machine and are
    read back into the report rather than asserted.

Asserting a host fact would make this gate fail on a different machine for a
reason that is not a defect, which is the failure mode that teaches people to
ignore a gate.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

VERDICT = "CNA_SWIFT_CANARY"

# field -> None for "must be present", or the exact value required.
INVARIANTS: dict[str, str | None] = {
    "viewport": "800x480",
    "texture": "128x128",
    "offscreen": "64x64",
    "adapters": "1",
    "default": "true",
    "reach": "true",
    "isDisposed": "false",
    "disposeRefused": "true",
    "presentRefused": "true",
    "cached": "true",
    "installed": "true",
    "kindRefused": "true",
    "missingRefused": "true",
    "rearmRefused": "true",
    "earlyCountRefused": "true",
    "played": "true",
    "limitRefused": "true",
    "loopRefused": "true",
    "badBufferRefused": "true",
    "readAfterDisposeRefused": "true",
    "noContextReported": "true",
    "ownSourceNil": "true",
    "gameHasControl": "true",
    "display": "800x480",
    "modes": None,
    "master": None,
    "state": None,
}

# Read back, never asserted: these are facts about the host.
HOST_FACTS = (
    "librarySongs", "libraryArtists", "libraryPlaylists", "libraryPictures",
    "mediaSources", "connected", "maxTouches", "touches", "gestures",
    "pixels", "waited", "ms", "queued", "pending", "track", "name",
    "playerState", "handle", "client", "resizing", "root",
)


def parse_verdict(line: str) -> dict[str, str]:
    """The verdict is `key=value` pairs separated by spaces.

    Values never contain a space, but a token is not always one pair: the
    canary writes its sections as `section=field=value`, so `device=` is
    followed by `isDisposed=false` inside the SAME token. Splitting on the
    first `=` reads the section name as the field, which is how the first
    version of this parser lost `isDisposed` and `played` and reported them
    missing from a line that had them.

    The value is therefore the last `=`-separated segment and the key the one
    before it, which reads `viewport=800x480`, `device=isDisposed=false`,
    `content=root=` (an empty value) and `name=\\\\.\\DISPLAY1` alike. A key
    appearing twice keeps the last: `queued` is written by both the audio
    section and the media one.
    """
    found: dict[str, str] = {}
    for token in line.split():
        if "=" not in token:
            continue
        parts = token.split("=")
        found[parts[-2]] = parts[-1]
    return found


def run_canary(
    template: Path, library: Path, frames: int, build: bool,
) -> tuple[int, str, str]:
    environment = dict(os.environ)
    environment["CNA_NATIVE_LIBRARY"] = str(library)
    environment.setdefault("CNA_RENDERER", "HEADLESS")
    # Never the user's display. The canary opens no window, but a fallback to
    # x11 anywhere below must land on a virtual screen and not on :0.
    environment.setdefault("SDL_VIDEODRIVER", "dummy")
    environment.setdefault("DISPLAY", ":99")

    if build:
        built = subprocess.run(
            ["swift", "build", "-j", environment.get("CNA_JOBS", "3")],
            cwd=template, capture_output=True, text=True, check=False,
            env=environment,
        )
        if built.returncode != 0:
            return built.returncode, "", built.stdout + built.stderr

    binary = template / ".build/debug/HelloGame"
    if not binary.exists():
        return 127, "", f"{binary} does not exist; run without --no-build"
    completed = subprocess.run(
        [str(binary), "--frames", str(frames)],
        cwd=template, capture_output=True, text=True, check=False,
        env=environment, timeout=600,
    )
    return completed.returncode, completed.stdout, completed.stderr


def check(
    code: int, stdout: str, stderr: str, frames: int,
) -> tuple[list[str], int, dict[str, str]]:
    findings: list[str] = []
    checks = 0

    checks += 1
    if code != 0:
        # The message the canary printed is the finding. Reproducing it here
        # rather than saying "exit 1" is the difference between a gate that
        # reports and one that merely refuses.
        detail = next(
            (line for line in stderr.splitlines() if "canary failed" in line),
            stderr.strip().splitlines()[-1] if stderr.strip() else "",
        )
        findings.append(f"the canary exited {code}: {detail}")

    line = next(
        (item for item in stdout.splitlines() if item.startswith(VERDICT)), None)
    checks += 1
    if line is None:
        findings.append("the canary printed no verdict line")
        return findings, checks, {}

    fields = parse_verdict(line)

    checks += 1
    if fields.get("requested") != str(frames):
        findings.append(
            f"requested={fields.get('requested')} but {frames} were asked for")

    checks += 1
    if fields.get("draws") != str(frames):
        findings.append(
            f"draws={fields.get('draws')} for {frames} requested frames; "
            "every requested frame draws exactly once")

    checks += 1
    updates = fields.get("updates")
    if updates is None or not updates.isdigit() or int(updates) < frames:
        findings.append(
            f"updates={updates} for {frames} requested frames; the fixed time "
            "step may add Update calls but can never lose one")

    for key, expected in INVARIANTS.items():
        checks += 1
        if key not in fields:
            findings.append(f"the verdict line has no {key!r} field")
        elif expected is not None and fields[key] != expected:
            findings.append(
                f"{key}={fields[key]}, expected {expected} on the qualified "
                "runtime")

    return findings, checks, fields


def self_test() -> int:
    checks = 0
    failures: list[str] = []

    def expect(condition: bool, description: str) -> None:
        nonlocal checks
        checks += 1
        if not condition:
            failures.append(description)

    good = (
        f"{VERDICT} requested=60 updates=60 draws=60 viewport=800x480 "
        "texture=128x128 offscreen=64x64 adapters=1 name=\\\\.\\DISPLAY1 "
        "default=true modes=1 reach=true window=handle=0 client=0x0 "
        "resizing=false device=isDisposed=false disposeRefused=true "
        "presentRefused=true content=root= cached=true installed=true "
        "kindRefused=true missingRefused=true query=pixels=1 waited=0 "
        "rearmRefused=true earlyCountRefused=true audio=played=true ms=100 "
        "state=Stopped queued=64 pending=64 limitRefused=true "
        "loopRefused=true badBufferRefused=true master=1.0 "
        "song=name=canary track=0 missingRefused=true "
        "readAfterDisposeRefused=true noContextReported=true librarySongs=0 "
        "libraryArtists=0 libraryPlaylists=0 libraryPictures=47 "
        "ownSourceNil=true mediaSources=1 queued=1 playerState=Playing "
        "gameHasControl=true touch=connected=false maxTouches=0 touches=0 "
        "display=800x480 gestures=1"
    )

    findings, _, fields = check(0, good, "", 60)
    expect(findings == [], f"a good run must pass, got {findings}")
    # The adapter name is checked on its own, not through `fields`: the real
    # verdict line writes `name` twice -- once for the adapter and once for the
    # song -- and last wins, which is the documented rule and not a defect.
    expect(parse_verdict("name=\\\\.\\DISPLAY1")["name"] == "\\\\.\\DISPLAY1",
           "a value containing backslashes must survive parsing")
    expect(fields["name"] == "canary",
           "a repeated key keeps the last occurrence")

    # The defect this tool exists for: a clean verdict line and a failed exit.
    findings, _, _ = check(
        1, good,
        "CNA Swift canary failed: CNA operation cna_game_destroy failed with "
        "result 3: All owned C child resources must be destroyed before the "
        "game.\n", 60)
    expect(len(findings) == 1 and "cna_game_destroy" in findings[0],
           f"a non-zero exit must be a finding naming the message, got {findings}")

    # updates may exceed the request; it may never fall short.
    findings, _, _ = check(0, good.replace("updates=60", "updates=62"), "", 60)
    expect(findings == [], "extra fixed-time-step updates are not a defect")
    findings, _, _ = check(0, good.replace("updates=60", "updates=59"), "", 60)
    expect(any("updates" in item for item in findings),
           "a lost update must be a finding")
    findings, _, _ = check(0, good.replace("draws=60", "draws=59"), "", 60)
    expect(any("draws" in item for item in findings),
           "a lost draw must be a finding")

    # An invariant that flips is a defect; a host fact that differs is not.
    findings, _, _ = check(
        0, good.replace("limitRefused=true", "limitRefused=false"), "", 60)
    expect(any("limitRefused" in item for item in findings),
           "a refusal that stopped refusing must be a finding")
    findings, _, _ = check(
        0, good.replace("libraryPictures=47", "libraryPictures=0"), "", 60)
    expect(findings == [],
           "a host fact must not be asserted; another machine is not a defect")
    findings, _, _ = check(
        0, good.replace(" viewport=800x480", ""), "", 60)
    expect(any("viewport" in item for item in findings),
           "a missing field must be a finding")
    findings, _, _ = check(0, "nothing here", "", 60)
    expect(any("no verdict" in item for item in findings),
           "a missing verdict line must be a finding")

    for line in failures:
        print(f"  SELF_TEST_FAILURE {line}")
    print(f"CANARY_SELF_TESTS={checks} "
          f"CANARY_SELF_TEST_STATUS={'PASS' if not failures else 'FAIL'}")
    return 1 if failures else 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--template", type=Path,
                        help="the cna-swift-template checkout")
    parser.add_argument("--library", type=Path)
    parser.add_argument("--frames", type=int, default=60)
    parser.add_argument("--no-build", action="store_true",
                        help="run the binary already in the template's .build")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    if args.self_test:
        return self_test()
    if args.template is None or args.library is None:
        print("CANARY_STATUS=FAIL --template and --library are required")
        return 1

    code, stdout, stderr = run_canary(
        args.template, args.library, args.frames, not args.no_build)
    findings, checks, fields = check(code, stdout, stderr, args.frames)

    report = {
        "schemaVersion": 1,
        "CANARY_EXIT_CODE": code,
        "CANARY_CHECKS": checks,
        "CANARY_FINDINGS": len(findings),
        "CANARY_STATUS": "PASS" if not findings else "FAIL",
        "requestedFrames": args.frames,
        "hostFacts": {key: fields[key] for key in HOST_FACTS if key in fields},
        "findings": findings,
    }
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2) + "\n",
                               encoding="utf-8")
    for finding in findings:
        print(f"  {finding}")
    print(f"CANARY_EXIT_CODE={code} CANARY_CHECKS={checks} "
          f"CANARY_FINDINGS={len(findings)} "
          f"CANARY_STATUS={report['CANARY_STATUS']}")
    return 0 if not findings else 1


if __name__ == "__main__":
    raise SystemExit(main())
