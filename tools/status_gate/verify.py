#!/usr/bin/env python3
"""Gate over every *normative* status claim in this repository's prose.

`plan.md` opened with "Foundation Milestones 1 through 47 are complete" while
the tree was at Foundation 57 and `NEXT.md` said so: a normative statement had
drifted ten milestones behind the work it describes, and nothing compared them.
`docs/generated/native-abi-report.json` had drifted further still -- it was last
written at `BOUND_FUNCTIONS=55` while a live run answers 87 -- so the generated
report a reader is told to trust over the prose was itself the stale one.

This tool exists so neither can happen again. It has two halves.

**Freshness.** A generated report is only evidence if it is what a live run
produces now. With `--symbol-graph` the strict verifier, the missing-type
inventory and the dependency graph are regenerated into a temporary directory
and required to be byte-identical to the committed copies; with `--cna-include`
and `--library` the native ABI report is regenerated and compared the same way.
A stale committed report fails the gate rather than being read as truth.

**Claims.** Every `KEY=VALUE` token in the normative documents whose KEY is one
this gate derives from a generated report must carry the derived value, and
every normative Foundation-status claim must name the current Foundation. The
current Foundation is the highest `docs/foundation-<N>-*-evidence.md` on disk,
so it moves when a milestone lands its evidence and cannot be edited into
agreement on its own.

Historical prose is not policed and is not deleted: a region between
`<!-- status-gate:historical -->` and `<!-- status-gate:/historical -->` (or the
end of the file) is skipped, which is what lets `NEXT.md` keep the Foundation
30-36 handoff and its Foundation 36 numbers verbatim.

Run from the repository root:

    python3 tools/status_gate/verify.py --self-test
    python3 tools/status_gate/verify.py \
        --symbol-graph .build/x86_64-pc-linux-gnu/symbolgraph/CNA.symbols.json \
        --cna-include /path/to/cna/modules/c-api/include \
        --library "$CNA_NATIVE_LIBRARY"
"""

from __future__ import annotations

import argparse
import ast
import importlib.util
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
GENERATED = ROOT / "docs/generated"

# Every normative document. A file listed here is policed outside its historical
# regions; a file not listed here is not a status authority.
NORMATIVE_DOCUMENTS = ["plan.md", "README.md", "NEXT.md"]

# Generated reports the derived facts are read out of. Each entry is
# (path, prefix-free keys to take). A key present in two reports with two
# different values is refused rather than silently policed against one of them.
# A claim key the gate cannot derive, and why. Anything NOT listed here and not
# derivable is a finding: `if key not in facts: continue` used to skip every
# unknown key in silence, which is how WITHDRAWN_IN_SOURCE sat at 5 while the
# source held 26, and how a bare CAUGHT= meaning two different numbers was
# policed as neither. Shrinking this table is the work; adding to it is a
# decision that has to be written down.
UNPOLICED_CLAIMS = {
}

FACT_REPORTS = [
    "api-compat-report.json",
    "content-reader-closure.json",
    "native-abi-report.json",
    "bcl-authority-audit.json",
    "pinned-assembly-audit.json",
    "gamepad-native-report.json",
    "native-stress-report.json",
    "package-qualification-report.json",
    "message-coverage.json",
    # A RUN record, not a property of the tree, so it is not regenerated here:
    # the harness mutates the working tree and takes a lock, and this gate also
    # checks for mutations left standing. It cannot go stale unnoticed anyway
    # -- NATIVE_ABI_MUTATIONS is derived from the harness source as well, so a
    # mutation added without re-running makes the two sources disagree and
    # derive_facts fails.
    "native-abi-mutations.json",
    # A full projection-mutation pass is expensive and mutates one source site
    # at a time, so this is a committed run record rather than a report the
    # status gate regenerates. The harness-derived PROJECTION_MUTATIONS count
    # below makes an older record disagree as soon as a mutation is added.
    "projection-mutations.json",
    # Also a RUN record, and for a stronger reason than the others: its host
    # facts -- how many pictures the media library holds, whether a controller
    # is attached -- are properties of the machine, so byte-comparing it across
    # machines would fail for something that is not a defect. What is derived
    # from it are the tool's own counters.
    "consumer-canary-report.json",
    # Read for its observation counts and never re-run: the committed copy said
    # 2125 where a live run says 2322, so it had been trailing the suite by
    # nearly two hundred observations. Regenerated below -- it costs a `swift
    # test`, which is the price of a corpus claim that cannot drift.
    "behavior-corpus-report.json",
]

# Pinned references live under tools/, not docs/generated. Reading facts from
# them directly avoids keeping a second copy of the same data in
# docs/generated -- the mistake this list replaces.
PINNED_REFERENCES = [
    "tools/api_compat/reference/xna40-accessor-fallibility.json",
]

# A category whose non-zero value would mean the projection DISAGREES with the
# pinned metadata, rather than merely lacking something. Every summary key that
# ends in one of these suffixes, or is named outright, is required to be zero.
#
# `OVERLOAD_MAPPING_MISMATCH` is the one exclusion, and it is an exclusion of
# meaning rather than of convenience: every entry it can carry reads "required
# overload is absent", which is an absence like `MISSING_MEMBER` and is counted
# with them. Nothing else is excluded, so a category that appears in the report
# for the first time is policed the moment it is non-zero.
DISAGREEMENT_SUFFIXES = ("_MISMATCH", "_LEAK")
DISAGREEMENT_NAMED = (
    "UNEXPECTED_TYPE", "UNEXPECTED_MEMBER", "UNMEASURED_STRUCTURAL_CATEGORY",
    "ALLOWLIST_ENTRIES", "APPLIED_ALLOWLIST_ENTRIES",
)
DISAGREEMENT_EXCLUDED = ("OVERLOAD_MAPPING_MISMATCH",)

HISTORICAL_OPEN = "<!-- status-gate:historical -->"
HISTORICAL_CLOSE = "<!-- status-gate:/historical -->"

TOKEN = re.compile(r"\b([A-Z][A-Z0-9_]{3,})=(\d+)\b")

# The normative Foundation-status claims. Each must name the current Foundation.
# A bare "Foundation 48 is the reminder that ..." is prose about a past
# milestone and is deliberately not in this set; only a statement of *where the
# work stands* is.
#
# Every space here is `\s+` and the patterns are matched against the whole
# normative text rather than line by line, because the statement that drifted --
# "Foundation Milestones 1 through 47 are\ncomplete" -- was wrapped across two
# lines, and a line-by-line gate walks straight past it.
FOUNDATION_CLAIMS = [
    r"Foundation\s+Milestones?\s+1\s+through\s+(\d+)\s+(?:are|is)\s+complete",
    r"Current\s+as\s+of\s+Foundation\s+(\d+)",
    r"Foundation\s+Milestones?\s+1[-–](\d+)\s+(?:are|is)\s+complete",
    r"through\s+Foundation\s+(\d+)\s+(?:have|has)\s+landed",
    r"Milestones?\s+\d+\s+through\s+(\d+)\s+(?:have|has)\s+landed",
    r"the\s+tree\s+is\s+at\s+Foundation\s+(\d+)",
    r"Foundations?\s+1\s+through\s+(\d+)\s+(?:are|is)\s+complete",
]
FOUNDATION_CLAIM_PATTERNS = [re.compile(pattern) for pattern in FOUNDATION_CLAIMS]

FOUNDATION_EVIDENCE = re.compile(r"^foundation-(\d+)-.*-evidence\.md$")


class Failure(Exception):
    pass


def current_foundation(root: Path = ROOT) -> int:
    """The highest Foundation with an evidence file, which is what "current" means."""
    numbers = [
        int(match.group(1))
        for path in (root / "docs").glob("foundation-*-evidence.md")
        if (match := FOUNDATION_EVIDENCE.match(path.name))
    ]
    if not numbers:
        raise Failure("no docs/foundation-<N>-*-evidence.md exists")
    return max(numbers)


def mutation_harness_sizes(root: Path = ROOT) -> dict[str, int]:
    """`len(MUTATIONS)` from each harness, read without running either.

    Parsed rather than scanned. The first version counted brackets from the
    `MUTATIONS` assignment until the depth returned to zero, which is wrong the
    moment a mutation's own text contains one: Foundation 67 aimed a native-ABI
    mutation at `cParameters: ["CNA_Handle texture", ...` -- a legal Python
    string with an unbalanced `[` -- and the scanner ran off the end of the file
    and crashed the whole gate. `ast` cannot be fooled by a string's contents.
    """
    sizes = {}
    for name, path in [
        ("PROJECTION_MUTATIONS", root / "tools/projection_mutations/run.py"),
        ("NATIVE_ABI_MUTATIONS", root / "tools/native_abi/mutations.py"),
    ]:
        tree = ast.parse(path.read_text(encoding="utf-8"))
        found = None
        for node in tree.body:
            targets = (
                [node.target] if isinstance(node, ast.AnnAssign) else
                node.targets if isinstance(node, ast.Assign) else []
            )
            for target in targets:
                if isinstance(target, ast.Name) and target.id == "MUTATIONS":
                    found = node.value
        if not isinstance(found, (ast.List, ast.Tuple)):
            raise Failure(f"{path.name} has no MUTATIONS list literal")
        sizes[name] = len(found.elts)
    return sizes


def mutation_record_counts(root: Path = ROOT) -> dict[str, int]:
    """Withdrawals and replaced no-ops, counted from the harness source.

    Both numbers were claimed in the documents and derivable from neither
    report, so nothing checked them: `WITHDRAWN_IN_SOURCE=5` was introduced
    once and never moved while the source grew to 26, and the replaced no-ops
    were claimed as 1 while the source records 2. An honest withdrawal record
    is the thing this project's discipline rests on, so a count of it that
    only a human maintains is the wrong shape.

    The rule the source already follows: a withdrawal is a comment block
    containing the word WITHDRAWN which NAMES the mutations it withdraws, in
    double quotes or backticks. A block that names none is a failure here
    rather than a zero -- that is how the record degrades quietly.
    """
    source = (root / "tools/projection_mutations/run.py").read_text(encoding="utf-8")
    blocks: list[list[str]] = []
    current: list[str] = []
    for line in source.split("\n"):
        if line.lstrip().startswith("#"):
            current.append(line)
        else:
            if current:
                blocks.append(current)
            current = []
    if current:
        blocks.append(current)

    name = re.compile(r"[`\"]([a-z][a-z0-9-]{6,})[`\"]")
    withdrawn: set[str] = set()
    replaced = 0
    for block in blocks:
        text = " ".join(block)
        names = name.findall(text)
        if "WITHDRAWN" in text:
            if not names:
                raise Failure(
                    "a WITHDRAWN block in tools/projection_mutations/run.py "
                    "names no mutation, so the withdrawal cannot be counted"
                )
            withdrawn.update(names)
        if re.search(r"NO-OP|no-op", text) and "SURVIVED" in text:
            # The no-op itself is the first name in the block; the others are
            # the mutations that replaced it.
            replaced += 1
    return {
        "WITHDRAWN_IN_SOURCE": len(withdrawn),
        "REPLACED_NO_OPS_IN_SOURCE": replaced,
    }


def api_compat_self_tests(root: Path = ROOT) -> dict[str, int]:
    completed = subprocess.run(
        [sys.executable, str(root / "tools/api_compat/verify.py"), "--self-test"],
        capture_output=True, text=True, cwd=root, check=False,
    )
    if completed.returncode != 0:
        raise Failure("tools/api_compat/verify.py --self-test does not pass")
    found = {}
    for match in TOKEN.finditer(completed.stdout):
        found[match.group(1)] = int(match.group(2))
    return {"API_COMPAT_SELF_TESTS": found["API_COMPAT_SELF_TESTS"]}


def planted_mutations(root: Path) -> list[str]:
    """Any projection mutation left standing in the working tree.

    A mutation is applied to the WORKING TREE while its test runs, so anything
    that reads the tree during a run sees it — and `git add -A` during a run
    commits it. That is how `texture-size-limit-not-checked` reached a commit:
    a profile bound weakened by four, in a tree that was green because the
    harness had already restored it by the time the tests were re-run.

    The mutation harness knows every replacement string, so asking it is both
    cheap and exact. This gate runs on every verification, which is the point:
    "do not commit during a mutation run" is a rule, and this is the part that
    does not depend on remembering it.
    """
    harness = root / "tools/projection_mutations/run.py"
    if not harness.exists():
        return ["the projection-mutation harness is absent, so the tree cannot "
                "be audited for a planted mutation"]
    result = subprocess.run(
        [sys.executable, str(harness), "--audit-tree"],
        capture_output=True, text=True, cwd=root)
    if result.returncode == 0:
        return []
    planted = [
        line.strip()
        for line in result.stdout.splitlines()
        if line.strip().startswith("PLANTED ")
    ] or [f"the working-tree mutation audit failed: {result.stdout.strip()}"]

    # A harness that is RUNNING has a mutation applied on purpose, and every
    # verification during a run would otherwise read as "someone committed
    # one". Both are findings -- verifying a tree that is being mutated
    # measures nothing -- but they are different findings, and a gate that
    # cries the wrong wolf is a gate people learn to ignore.
    #
    # The harness holds `.mutation-gate.lock` with flock for the whole run, so
    # failing to take it is the exact question "is one running right now".
    if _mutation_harness_running(root):
        return ["a projection-mutation run is in progress, so the tree holds a "
                "mutation on purpose and nothing here was measured against a "
                "clean tree — re-run this gate after it finishes"] + planted
    return planted


def _mutation_harness_running(root: Path) -> bool:
    """Whether something holds the mutation harness's tree lock."""
    lock = root / ".mutation-gate.lock"
    if not lock.exists():
        return False
    try:
        import fcntl
        with lock.open("a", encoding="utf-8") as handle:
            try:
                fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
            except OSError:
                return True
            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
    except OSError:
        return False
    return False


def disagreements(summary: dict[str, int]) -> dict[str, int]:
    """Every disagreement category the strict report reports as non-zero.

    Foundation 59 shipped with `PARAMETER_MAPPING_MISMATCH=2` because the prose
    claim -- "every category that would mean DISAGREEMENT with XNA: 0" -- names
    the categories without their values, so no `KEY=VALUE` token existed for the
    gate to check. This reads the report instead of the prose.
    """
    return {
        key: value for key, value in summary.items()
        if key not in DISAGREEMENT_EXCLUDED
        and (key.endswith(DISAGREEMENT_SUFFIXES) or key in DISAGREEMENT_NAMED)
        and value != 0
    }


def flatten(document: Any, into: dict[str, int]) -> None:
    if isinstance(document, dict):
        for key, value in document.items():
            if isinstance(value, bool):
                continue
            if isinstance(value, int) and key.isupper():
                into[key] = value
            elif key == "summary" and isinstance(value, dict):
                flatten(value, into)


def derive_facts(root: Path = ROOT, with_self_tests: bool = True) -> dict[str, int]:
    facts: dict[str, int] = {}
    sources: dict[str, str] = {}
    conflicts: list[str] = []

    def add(key: str, value: int, source: str) -> None:
        if key in facts and facts[key] != value:
            conflicts.append(f"{key}: {source} says {value}, {sources[key]} says {facts[key]}")
            return
        facts[key] = value
        sources[key] = source

    for name in FACT_REPORTS:
        path = root / "docs/generated" / name
        if not path.exists():
            raise Failure(f"docs/generated/{name} is absent")
        found: dict[str, int] = {}
        flatten(json.loads(path.read_text(encoding="utf-8")), found)
        for key, value in found.items():
            add(key, value, name)

    for name in PINNED_REFERENCES:
        path = root / name
        if not path.exists():
            raise Failure(f"{name} is absent")
        found: dict[str, int] = {}
        flatten(json.loads(path.read_text(encoding="utf-8")), found)
        for key, value in found.items():
            add(key, value, name)

    for key, value in mutation_harness_sizes(root).items():
        add(key, value, "mutation harness")
    for key, value in mutation_record_counts(root).items():
        add(key, value, "mutation harness source")
    # `PLANTED` is a property of the working tree and the audit already
    # computes it; leaving it underived meant a document could claim a clean
    # tree and nothing would check. It is derived rather than excused because
    # the answer is one subprocess away -- and because a document saying
    # PLANTED=0 while a mutation stands is the exact confusion this gate's own
    # audit exists to prevent.
    add("PLANTED", len(planted_mutations(root)), "working-tree mutation audit")
    if with_self_tests:
        for key, value in api_compat_self_tests(root).items():
            add(key, value, "verify.py --self-test")
    add("FOUNDATION", current_foundation(root), "docs/foundation-*-evidence.md")

    if conflicts:
        raise Failure("two reports disagree: " + "; ".join(sorted(conflicts)))
    return facts


def normative_lines(text: str) -> list[tuple[int, str]]:
    """Every line outside a historical region, with its 1-based number.

    A marker counts only when it is the whole line. `plan.md`'s rule 11 names
    both markers in its prose, and a substring test let that sentence switch
    policing off for the rest of the file it appears in -- the gate silently
    halved its own claim count, which is how this was found.
    """
    out, historical = [], False
    for number, line in enumerate(text.splitlines(), start=1):
        stripped = line.strip()
        if stripped == HISTORICAL_OPEN:
            historical = True
            continue
        if stripped == HISTORICAL_CLOSE:
            historical = False
            continue
        if not historical:
            out.append((number, line))
    return out


def check_document(
    path: Path, facts: dict[str, int], relative: str,
) -> tuple[list[str], int, int]:
    findings: list[str] = []
    checked = 0
    unpoliced = 0
    foundation = facts["FOUNDATION"]
    lines = normative_lines(path.read_text(encoding="utf-8"))

    for number, line in lines:
        for match in TOKEN.finditer(line):
            key, value = match.group(1), int(match.group(2))
            if key == "FOUNDATION":
                continue
            if key not in facts:
                if key not in UNPOLICED_CLAIMS:
                    findings.append(
                        f"{relative}:{number}: {key}={value} is a claim no "
                        f"generated evidence can check. Derive it, or record "
                        f"in UNPOLICED_CLAIMS why it cannot be."
                    )
                else:
                    unpoliced += 1
                continue
            checked += 1
            if value != facts[key]:
                findings.append(
                    f"{relative}:{number}: {key}={value} but the generated "
                    f"evidence says {key}={facts[key]}"
                )

    # The Foundation claims run over the joined normative text so a claim that
    # wraps across a line break is still one claim.
    joined = "\n".join(line for _, line in lines)
    starts, offset = [], 0
    for _, line in lines:
        starts.append(offset)
        offset += len(line) + 1

    def line_of(position: int) -> int:
        index = 0
        while index + 1 < len(starts) and starts[index + 1] <= position:
            index += 1
        return lines[index][0] if lines else 0

    for pattern in FOUNDATION_CLAIM_PATTERNS:
        for match in pattern.finditer(joined):
            checked += 1
            if int(match.group(1)) != foundation:
                findings.append(
                    f"{relative}:{line_of(match.start())}: normative status names "
                    f"Foundation {match.group(1)} but the current Foundation is "
                    f"{foundation}"
                )
    return findings, checked, unpoliced


def regenerate_and_compare(
    root: Path,
    symbol_graph: Path | None,
    cna_include: Path | None,
    library: Path | None,
    assembly_dir: Path | None = None,
    il_cache: Path | None = None,
    bcl_dir: Path | None = None,
    bcl_controls: list[str] | None = None,
) -> tuple[list[str], int, int]:
    """A committed generated report must be what a live run writes now."""
    findings: list[str] = []
    compared = 0
    # A regeneration that was never asked for is not a regeneration that
    # passed. Counting the skips puts the difference between a full run and a
    # partial one in the summary line people paste into documents, instead of
    # letting the two look identical.
    skipped = sum(1 for argument in (symbol_graph, cna_include, assembly_dir,
                                     bcl_dir) if argument is None)
    (ROOT / "build-probe").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="cna-status-gate-", dir=ROOT / "build-probe") as raw:
        temporary = Path(raw)

        # Why a regeneration produced nothing, not merely that it did. A
        # finding that says "wrote nothing" sends the reader to run the command
        # by hand to find out; the command already told us.
        reasons: dict[str, str] = {}

        def compare(committed: Path, fresh: Path, command: str) -> None:
            nonlocal compared
            compared += 1
            if not fresh.exists():
                detail = reasons.get(command, "").strip().splitlines()
                why = f": {detail[-1]}" if detail else ""
                findings.append(f"{command} wrote nothing{why}")
            elif committed.read_bytes() != fresh.read_bytes():
                # The real path, not an assumed docs/generated/ prefix: the
                # pinned references live under tools/, and naming the wrong
                # directory sends the reader to a file that is not there.
                try:
                    where = committed.relative_to(root)
                except ValueError:
                    where = committed
                findings.append(
                    f"{where} is stale: {command} produces different bytes"
                )

        if symbol_graph is not None:
            report = temporary / "api-compat-report.json"
            inventory = temporary / "missing-type-inventory.md"
            subprocess.run(
                [sys.executable, "tools/api_compat/verify.py",
                 "--symbol-graph", str(symbol_graph),
                 "--output", str(report),
                 "--inventory-output", str(inventory)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            compare(GENERATED / "api-compat-report.json", report,
                    "tools/api_compat/verify.py")
            compare(GENERATED / "missing-type-inventory.md", inventory,
                    "tools/api_compat/verify.py --inventory-output")
            graph = temporary / "dependency-graph.json"
            subprocess.run(
                [sys.executable, "tools/api_compat/dependency_graph.py",
                 "--report", str(GENERATED / "api-compat-report.json"),
                 "--output", str(graph)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            compare(GENERATED / "dependency-graph.json", graph,
                    "tools/api_compat/dependency_graph.py")

        # Package qualification is not regenerated here -- it costs two
        # consumer builds -- so its freshness is checked by recomputing the
        # digest of the inputs it depends on. Without this the report goes
        # stale invisibly, which is exactly what happened between 805b4cd and
        # c7b1072: a canary that had not compiled for forty-one commits, and a
        # committed PASS that nobody had earned.
        qualification = GENERATED / "package-qualification-report.json"
        if qualification.exists():
            spec = importlib.util.spec_from_file_location(
                "package_qualification_verify",
                root / "tools" / "package_qualification" / "verify.py")
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            recorded = json.loads(qualification.read_text(encoding="utf-8")).get(
                "QUALIFIED_INPUTS_SHA256")
            actual = module.qualified_inputs_digest(root)
            compared += 1
            if recorded is None:
                findings.append(
                    "docs/generated/package-qualification-report.json records no "
                    "QUALIFIED_INPUTS_SHA256, so its PASS cannot be shown to be "
                    "about the current sources")
            elif recorded != actual:
                findings.append(
                    "docs/generated/package-qualification-report.json is stale: it "
                    f"qualified inputs {recorded[:12]} but the tree is {actual[:12]}; "
                    "re-run tools/package_qualification/verify.py")

        if cna_include is not None and library is not None:
            abi = temporary / "native-abi-report.json"
            subprocess.run(
                [sys.executable, "tools/native_abi/verify.py",
                 "--cna-include", str(cna_include),
                 "--library", str(library),
                 "--output", str(abi)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            compare(GENERATED / "native-abi-report.json", abi,
                    "tools/native_abi/verify.py")

        # The message-coverage report is READ for derived facts, and until
        # Foundation 88 it was never re-run: the committed copy recorded zero
        # findings while a live run returned eleven, several of them from
        # milestones committed the same day. That is the same drift this gate's
        # own header describes for the native ABI report, one report over.
        if assembly_dir is not None and il_cache is not None:
            coverage = temporary / "message-coverage.json"
            subprocess.run(
                [sys.executable, "tools/api_compat/message_coverage.py",
                 "--assembly-dir", str(assembly_dir),
                 "--il-cache", str(il_cache),
                 "--output", str(coverage)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            compare(GENERATED / "message-coverage.json", coverage,
                    "tools/api_compat/message_coverage.py")

            # The pinned-assembly audit is deterministic given the pinned
            # binaries, was READ for four derived facts, and was never re-run:
            # the committed copy said CALIBRATION_STATUS=PASS over 73 resource
            # strings while a live run said FAIL over 79. Six user-visible XNA
            # messages had been pinned and used in Sources/ without being
            # registered, so nothing checked that they came from the authority.
            # return_nullability decides Optional vs non-Optional for every
            # projected reference return, and its PINNED reference lives under
            # tools/, not docs/generated -- which is how it escaped the sweep.
            # It had IVertexType.VertexDeclaration recorded as UNKNOWN with
            # evidence "no registered implementor" while a live run finds four,
            # all explicit interface implementations. Both verdicts project
            # non-Optional, so no signature was wrong; the evidence was.
            nullability = temporary / "xna40-reference-return-nullability.json"
            nullability_md = temporary / "return-nullability-inventory.md"
            subprocess.run(
                [sys.executable, "tools/api_compat/return_nullability.py",
                 "--assembly-dir", str(assembly_dir),
                 "--il-cache", str(il_cache),
                 "--output", str(nullability),
                 "--markdown", str(nullability_md)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            compare(root / "tools/api_compat/reference"
                    / "xna40-reference-return-nullability.json", nullability,
                    "tools/api_compat/return_nullability.py")
            compare(GENERATED / "return-nullability-inventory.md",
                    nullability_md,
                    "tools/api_compat/return_nullability.py --markdown")

            pinned = temporary / "pinned-assembly-audit.json"
            subprocess.run(
                [sys.executable, "tools/api_compat/pinned_assembly_audit.py",
                 "--assembly-dir", str(assembly_dir),
                 "--il-cache", str(il_cache),
                 "--require-exact", "Microsoft.Xna.Framework.dll",
                 "--require-exact", "Microsoft.Xna.Framework.Graphics.dll",
                 "--output", str(pinned)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            compare(GENERATED / "pinned-assembly-audit.json", pinned,
                    "tools/api_compat/pinned_assembly_audit.py")

            accessors = temporary / "xna40-accessor-fallibility.json"
            inventory = temporary / "accessor-fallibility-inventory.md"
            subprocess.run(
                [sys.executable, "tools/api_compat/accessor_fallibility.py",
                 "--assembly-dir", str(assembly_dir),
                 "--il-cache", str(il_cache),
                 "--output", str(accessors),
                 "--markdown", str(inventory)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            compare(root / "tools/api_compat/reference"
                    / "xna40-accessor-fallibility.json", accessors,
                    "tools/api_compat/accessor_fallibility.py")
            compare(GENERATED / "accessor-fallibility-inventory.md", inventory,
                    "tools/api_compat/accessor_fallibility.py --markdown")

        # bcl-authority-audit.json is READ for five derived facts and, until
        # Foundation 100, was never re-run either. Foundation 100 admitted
        # System.dll and moved BCL_SENTINEL_CHECKS from 442 to 585 and
        # BCL_MUTATION_SELF_TESTS from 462 to 497; the gate passed against the
        # stale committed copy and said nothing. Same defect as the one above,
        # one report over -- which is why it is fixed the same way rather than
        # by remembering to regenerate.
        # The behaviour corpus needs no pinned assembly, header or library --
        # only a Swift toolchain, which is an INPUT like any other. Absent, it
        # is skipped and counted, not a finding: a machine with no toolchain
        # cannot build the package either, and a gate that fails for a missing
        # input rather than a defect is one people learn to ignore. Present, it
        # is regenerated and compared like the rest.
        corpus = temporary / "behavior-corpus-report.json"
        if shutil.which("swift-test") or shutil.which("swift"):
            ran = subprocess.run(
                [sys.executable, "tools/behavior/run.py",
                 "--output", str(corpus)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            reasons["tools/behavior/run.py"] = ran.stderr or ran.stdout
            compare(GENERATED / "behavior-corpus-report.json", corpus,
                    "tools/behavior/run.py")
        else:
            skipped += 1

        if bcl_dir is not None:
            audit = temporary / "bcl-authority-audit.json"
            subprocess.run(
                [sys.executable, "tools/api_compat/bcl_authority_audit.py",
                 "--assembly", f"mscorlib.dll={bcl_dir / 'mscorlib.dll'}",
                 "--assembly", f"System.dll={bcl_dir / 'System.dll'}",
                 "--il-cache", str(bcl_dir),
                 "--cross-check",
                 *[argument
                   for control in (bcl_controls or [])
                   for argument in ("--negative-control", control)],
                 "--output", str(audit)],
                cwd=root, capture_output=True, text=True, check=False,
            )
            compare(GENERATED / "bcl-authority-audit.json", audit,
                    "tools/api_compat/bcl_authority_audit.py")
    return findings, compared, skipped


SELF_TEST_DOCUMENT = """# A document

<!-- status-gate:historical -->
Foundation Milestones 1 through 3 are complete. COMPLETE_TYPES=1
<!-- status-gate:/historical -->

Foundation Milestones 1 through 57 are
complete.
COMPLETE_TYPES=150 BOUND_FUNCTIONS=87 EXCUSED_KEY=999
"""


def excused_fixture(document: str) -> str:
    """`EXCUSED_KEY` stands for whatever UNPOLICED_CLAIMS currently excuses.

    Naming a real key here once meant that making that key derivable -- the
    whole point of the table -- broke every self-test that counts findings
    exactly. An empty table is the goal, and then the token simply goes away.
    """
    if not UNPOLICED_CLAIMS:
        return document.replace(" EXCUSED_KEY=999", "")
    return document.replace("EXCUSED_KEY", sorted(UNPOLICED_CLAIMS)[0])


def self_test() -> int:
    """Every rule here must be shown to fail on a realistic defect."""
    checks = 0
    failures: list[str] = []

    def expect(condition: bool, description: str) -> None:
        nonlocal checks
        checks += 1
        if not condition:
            failures.append(description)

    facts = {"FOUNDATION": 57, "COMPLETE_TYPES": 150, "BOUND_FUNCTIONS": 87}
    (ROOT / "build-probe").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="cna-status-gate-self-", dir=ROOT / "build-probe") as raw:
        temporary = Path(raw)
        clean = temporary / "clean.md"
        clean.write_text(excused_fixture(SELF_TEST_DOCUMENT), encoding="utf-8")
        findings, checked, _ = check_document(clean, facts, "clean.md")
        expect(findings == [], f"the clean document must pass, got {findings}")
        expect(checked == 3, f"the clean document must check 3 claims, got {checked}")

        # A stale count.
        stale_count = temporary / "count.md"
        stale_count.write_text(
            excused_fixture(SELF_TEST_DOCUMENT).replace("COMPLETE_TYPES=150", "COMPLETE_TYPES=135"),
            encoding="utf-8")
        findings, _, _ = check_document(stale_count, facts, "count.md")
        expect(len(findings) == 1 and "COMPLETE_TYPES=135" in findings[0],
               f"a stale count must be caught, got {findings}")

        # A stale native-ABI count, which is the second class that drifted.
        stale_abi = temporary / "abi.md"
        stale_abi.write_text(
            excused_fixture(SELF_TEST_DOCUMENT).replace("BOUND_FUNCTIONS=87", "BOUND_FUNCTIONS=55"),
            encoding="utf-8")
        findings, _, _ = check_document(stale_abi, facts, "abi.md")
        expect(len(findings) == 1 and "BOUND_FUNCTIONS=55" in findings[0],
               f"a stale ABI count must be caught, got {findings}")

        # A stale Foundation number, which is the defect this gate was built
        # for. It is wrapped across a line break exactly as `plan.md`'s was,
        # because a line-by-line gate does not see that one at all.
        stale_foundation = temporary / "foundation.md"
        stale_foundation.write_text(
            excused_fixture(SELF_TEST_DOCUMENT).replace(
                "Foundation Milestones 1 through 57 are\ncomplete.",
                "Foundation Milestones 1 through 47 are\ncomplete."),
            encoding="utf-8")
        findings, _, _ = check_document(stale_foundation, facts, "foundation.md")
        expect(len(findings) == 1 and "Foundation 47" in findings[0],
               f"a stale Foundation number must be caught, got {findings}")
        expect("foundation.md:7:" in (findings[0] if findings else ""),
               f"the finding must name the line the claim starts on, got {findings}")

        # The historical region must not be policed, and must not be the reason
        # the clean document passes either.
        without_markers = temporary / "unmarked.md"
        without_markers.write_text(
            excused_fixture(SELF_TEST_DOCUMENT).replace(HISTORICAL_OPEN, "").replace(HISTORICAL_CLOSE, ""),
            encoding="utf-8")
        findings, _, _ = check_document(without_markers, facts, "unmarked.md")
        expect(len(findings) == 2,
               f"unmarking the historical region must expose its two stale claims, "
               f"got {findings}")

        # A document that *names* the marker in its prose must stay policed.
        # A substring test here switched policing off for the rest of `plan.md`
        # the moment rule 11 described the marker.
        names_marker = temporary / "names.md"
        names_marker.write_text(
            excused_fixture(SELF_TEST_DOCUMENT.replace(
                "COMPLETE_TYPES=150 BOUND_FUNCTIONS=87 EXCUSED_KEY=999",
                f"prose that mentions `{HISTORICAL_OPEN}` in passing.\n"
                f"COMPLETE_TYPES=135 BOUND_FUNCTIONS=87 EXCUSED_KEY=999")),
            encoding="utf-8")
        findings, _, _ = check_document(names_marker, facts, "names.md")
        expect(len(findings) == 1 and "COMPLETE_TYPES=135" in findings[0],
               f"naming the marker in prose must not disable policing, got {findings}")

        # A disagreement category must be reported the moment it is non-zero,
        # and an absence must not be. This is the check that would have stopped
        # Foundation 59 shipping PARAMETER_MAPPING_MISMATCH=2.
        expect(disagreements({"PARAMETER_MAPPING_MISMATCH": 2}) ==
               {"PARAMETER_MAPPING_MISMATCH": 2},
               "a non-zero mismatch category must be reported")
        expect(disagreements({"RAW_HANDLE_LEAK": 1}) == {"RAW_HANDLE_LEAK": 1},
               "a non-zero leak category must be reported")
        expect(disagreements({"UNEXPECTED_MEMBER": 3}) == {"UNEXPECTED_MEMBER": 3},
               "a non-zero named category must be reported")
        expect(disagreements({"A_BRAND_NEW_MISMATCH": 1}) == {"A_BRAND_NEW_MISMATCH": 1},
               "a category this gate has never seen must still be policed")
        expect(disagreements({"MISSING_MEMBER": 58, "MISSING_TYPE": 97}) == {},
               "an absence must not be reported as a disagreement")
        expect(disagreements({"OVERLOAD_MAPPING_MISMATCH": 5}) == {},
               "the one documented exclusion must stay excluded")
        expect(disagreements({"PARAMETER_MAPPING_MISMATCH": 0}) == {},
               "a zero category must not be reported")

        # A key no generated report derives used to be skipped in silence,
        # and this self-test asserted that skipping as correct behaviour. It
        # was the bug, encoded as a passing test: WITHDRAWN_IN_SOURCE sat at 5
        # while the harness source held 26, and a bare CAUGHT= meaning two
        # different numbers in two documents was policed as neither. An
        # unrecognised claim is now a finding unless UNPOLICED_CLAIMS says in
        # writing why it cannot be derived.
        unpoliced = temporary / "unpoliced.md"
        unpoliced.write_text(
            SELF_TEST_DOCUMENT.replace("EXCUSED_KEY=999", "UNPOLICED_KEY=1"),
            encoding="utf-8")
        findings, _, count = check_document(unpoliced, facts, "unpoliced.md")
        expect(len(findings) == 1 and "UNPOLICED_KEY" in findings[0],
               "an underived key that nothing excuses must be a finding")
        expect(count == 0, "an unexcused key is not counted as excused")

        excused = temporary / "excused.md"
        excused.write_text(excused_fixture(SELF_TEST_DOCUMENT), encoding="utf-8")
        findings, _, count = check_document(excused, facts, "excused.md")
        expect(findings == [],
               "a key listed in UNPOLICED_CLAIMS must not be a finding")
        expect(count == (1 if UNPOLICED_CLAIMS else 0),
               "an excused key must be counted, and an empty exception table "
               "must excuse nothing")
        expect(all(reason.strip() for reason in UNPOLICED_CLAIMS.values()),
               "every excused key must carry a written reason")

        # The current Foundation is read off the evidence files, so a new
        # milestone's evidence moves it and prose alone cannot.
        fake = temporary / "repo"
        (fake / "docs").mkdir(parents=True)
        for name in ["foundation-3-x-evidence.md", "foundation-57-y-evidence.md"]:
            (fake / "docs" / name).write_text("", encoding="utf-8")
        expect(current_foundation(fake) == 57,
               "the current Foundation is the highest evidence file")
        (fake / "docs" / "foundation-58-z-evidence.md").write_text("", encoding="utf-8")
        expect(current_foundation(fake) == 58,
               "a landed milestone's evidence file moves the current Foundation")

        # --- the planted-mutation audit ---------------------------------
        #
        # A tree with no harness cannot be audited, and saying nothing would
        # be the wrong answer: the check that cannot run is a finding, not a
        # pass. The real harness's own two directions are exercised by
        # `run.py --audit-tree`, which this calls; what is checked here is
        # that a missing harness is reported rather than skipped.
        expect(planted_mutations(fake) != [],
               "a tree with no mutation harness must report that it cannot "
               "be audited, not pass silently")
        # The real tree is clean UNLESS a mutation run holds it, in which case
        # the finding names that instead. Asserting "no findings" outright
        # would make this self-test fail for a legitimate reason, which is the
        # same mistake the gate itself was making.
        real = planted_mutations(ROOT)
        expect(real == [] or _mutation_harness_running(ROOT),
               "the real working tree must hold no planted mutation unless a "
               "run is in progress")
        if _mutation_harness_running(ROOT):
            expect(any("run is in progress" in item for item in real),
                   "a running harness must be named as the cause rather than "
                   "reported as a committed mutation")
        # No lock file at all is not a running harness.
        expect(not _mutation_harness_running(fake),
               "an empty tree holds no mutation-harness lock")

    for failure in failures:
        print(f"  SELF_TEST_FAILURE {failure}")
    print(f"STATUS_GATE_SELF_TESTS={checks} "
          f"STATUS_GATE_SELF_TEST_STATUS={'PASS' if not failures else 'FAIL'}")
    return 0 if not failures else 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--symbol-graph", type=Path,
                        help="regenerate the strict report, inventory and dependency "
                             "graph from this Symbol Graph and require the committed "
                             "copies to match")
    parser.add_argument("--cna-include", type=Path,
                        help="regenerate the native ABI report and require the "
                             "committed copy to match")
    parser.add_argument("--library", type=Path)
    parser.add_argument("--assembly-dir", type=Path,
                        help="regenerate the message-coverage report from the "
                             "pinned XNA assemblies and require the committed "
                             "copy to match")
    parser.add_argument("--il-cache", type=Path)
    parser.add_argument("--bcl-dir", type=Path,
                        help="regenerate the BCL authority audit from the "
                             "pinned mscorlib.dll and System.dll in this "
                             "directory and require the committed copy to "
                             "match")
    parser.add_argument("--bcl-negative-control", action="append", default=[],
                        metavar="NAME=PATH",
                        help="a binary the audit must refuse; the committed "
                             "report records one entry per control, so the "
                             "same set must be offered here for the bytes to "
                             "match")
    args = parser.parse_args()

    if args.self_test:
        return self_test()

    try:
        facts = derive_facts()
    except Failure as failure:
        print(f"STATUS_GATE_STATUS=FAIL {failure}")
        return 1

    findings: list[str] = []
    checked = 0
    unpoliced = 0
    for relative in NORMATIVE_DOCUMENTS:
        path = ROOT / relative
        if not path.exists():
            findings.append(f"{relative} is absent")
            continue
        document_findings, document_checked, document_unpoliced = check_document(
            path, facts, relative)
        findings += document_findings
        checked += document_checked
        unpoliced += document_unpoliced

    fresh_findings, compared, skipped = regenerate_and_compare(
        ROOT, args.symbol_graph, args.cna_include, args.library,
        args.assembly_dir, args.il_cache, args.bcl_dir,
        args.bcl_negative_control)
    findings += fresh_findings

    findings += planted_mutations(ROOT)

    summary = json.loads(
        (GENERATED / "api-compat-report.json").read_text(encoding="utf-8"))["summary"]
    nonzero = disagreements(summary)
    findings += [
        f"the projection disagrees with the pinned metadata: {key}={value}"
        for key, value in sorted(nonzero.items())
    ]

    for finding in findings:
        print(f"  {finding}")
    print(f"STATUS_GATE_FOUNDATION={facts['FOUNDATION']} "
          f"STATUS_GATE_DERIVED_FACTS={len(facts)} "
          f"STATUS_GATE_CLAIMS_CHECKED={checked} "
          f"STATUS_GATE_UNPOLICED_CLAIMS={unpoliced} "
          f"STATUS_GATE_DISAGREEMENTS={len(nonzero)} "
          f"STATUS_GATE_REPORTS_COMPARED={compared} "
          f"STATUS_GATE_REPORTS_SKIPPED={skipped} "
          f"STATUS_GATE_FINDINGS={len(findings)} "
          f"STATUS_GATE_STATUS={'PASS' if not findings else 'FAIL'}")
    return 0 if not findings else 1


if __name__ == "__main__":
    raise SystemExit(main())
