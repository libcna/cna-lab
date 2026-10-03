# Foundation 58 — the normative status gate

## What was wrong

Two normative claims had drifted, in two different ways, and nothing compared
either of them to anything.

**`plan.md` was ten Foundations behind.** Its first paragraph opened:

```text
Foundation Milestones 1 through 47 are
complete: …
```

while `NEXT.md`'s first line said *Current as of Foundation 57* and
`docs/foundation-57-texture-data-evidence.md` was on disk. `plan.md` is the
document that states what is normatively true, so the one file a reader is told
to trust for project rules was the one that had stopped moving.

**A generated report was forty-seven routes behind.** `NEXT.md` tells a reader
to trust the generated reports over the prose. `docs/generated/native-abi-report.json`
was last written during Foundation 40 and said:

```text
BOUND_FUNCTIONS=55  PROTOTYPE_TYPE_POSITIONS=170  LAYOUTS=21
LAYOUT_FIELDS=157   CONSTANTS=212
```

A live run of the same tool, unchanged, against the same qualified library
answers:

```text
BOUND_FUNCTIONS=87  PROTOTYPE_TYPE_POSITIONS=283  LAYOUTS=28
LAYOUT_FIELDS=243   CONSTANTS=215
```

The prose in `plan.md` and `NEXT.md` was right and the *generated evidence* was
wrong — the reverse of the failure the "trust the generated report" rule was
written to prevent. Every other generated report was checked the same way and
was current: the strict report, the missing-type inventory, the dependency
graph, the message-coverage report, the pinned-assembly audit, the gamepad
native report and the rendered runtime-capabilities table all reproduce
byte-identically.

## The gate

`tools/status_gate/verify.py`. Two halves, because there were two defects.

**Freshness.** With `--symbol-graph`, `--cna-include` and `--library` the tool
regenerates the strict report, the missing-type inventory, the dependency graph
and the native ABI report into a temporary directory and requires each to be
byte-identical to the committed copy. `STATUS_GATE_REPORTS_COMPARED=4`. A
committed report that a live run no longer produces is a failure, not evidence.

**Claims.** Every `KEY=VALUE` token in `plan.md`, `README.md` and `NEXT.md`
whose key the gate derives from a generated report must carry the derived value.
149 facts are derived, from eight generated reports plus three sources that
cannot be read out of one:

| Fact | Derived from |
|---|---|
| `FOUNDATION` | the highest `docs/foundation-<N>-*-evidence.md` on disk |
| `PROJECTION_MUTATIONS`, `NATIVE_ABI_MUTATIONS` | `len(MUTATIONS)` in each harness, read without running either |
| `API_COMPAT_SELF_TESTS` | `tools/api_compat/verify.py --self-test` (0.9 s) |
| the other 145 | the eight generated JSON reports |

A key that two reports define with two different values is **refused** rather
than policed against whichever was read first.

The current Foundation is deliberately *not* a number written down anywhere. It
is the highest evidence file, so it moves when a milestone lands its evidence
and a session cannot edit the tree into agreement with a claim by changing the
claim alone.

**Historical prose is not policed and is not deleted.** A region between
`<!-- status-gate:historical -->` and `<!-- status-gate:/historical -->` (or the
end of file) is skipped. `NEXT.md`'s Foundation 30–36 handoff keeps its
`COMPLETE_TYPES=135`, `MISSING_TYPE=115`, `AUDIT_SELF_TESTS=64` and the rest
verbatim, because those measurements were real when they were written. The
marker goes *after* the blockquote that introduces the section, so the sentence
"Foundation Milestones 37 through 58 have landed since" stays policed: it is a
statement about the present sitting inside a historical chapter.

Only six sentence forms are treated as normative status claims. A bare
"Foundation 48 is the reminder that …" is prose about a past milestone and is
deliberately not one of them; the gate polices statements of *where the work
stands*, not every mention of a number.

## The claims are matched across line breaks

The first version of this gate did not catch the very defect it was written for.
`plan.md`'s sentence was wrapped:

```text
…Foundation Milestones 1 through 47 are
complete: the native migration off…
```

A line-by-line scan walks straight past it. Every space in the six claim
patterns is `\s+` and they are matched against the whole normative text with
offsets mapped back to line numbers, which is why the finding reads `plan.md:4`.
The self-test's clean fixture carries a wrapped claim for exactly this reason.

## The marker is a whole line, not a substring

Adding rule 11 to `plan.md` — the rule that describes this gate, and therefore
names `<!-- status-gate:historical -->` in its prose — dropped
`STATUS_GATE_CLAIMS_CHECKED` from 80 to 40 while still reporting `PASS`. The
marker was matched as a substring, so the sentence documenting it switched
policing off for the remaining half of the file. A gate that quietly stops
looking is worse than no gate, and the only reason this was noticed is that the
claim count is printed on every run. Markers now count only as a whole stripped
line, and a self-test writes a fixture that mentions the marker in prose and
requires its stale count to still be caught.

Print the size of what a gate examined, not just its verdict.

## Demonstrated to fail

Eleven self-tests (`STATUS_GATE_SELF_TESTS=11 STATUS_GATE_SELF_TEST_STATUS=PASS`),
including a stale count, a stale native-ABI count, a stale wrapped Foundation
number, a fixture whose historical markers are removed (which must then expose
the two stale claims the markers were hiding), a fixture that names the marker
in prose (which must stay policed), and a key no report derives, which must
*not* be policed.

Three defects were then planted in the real tree and restored:

```text
plan.md: "1 through 57" -> "1 through 47"
  plan.md:4: normative status names Foundation 47 but the current Foundation is 57
  STATUS_GATE_FINDINGS=1 STATUS_GATE_STATUS=FAIL   exit=1

plan.md: COMPLETE_TYPES=150 -> 149
  plan.md:142: COMPLETE_TYPES=149 but the generated evidence says COMPLETE_TYPES=150
  STATUS_GATE_FINDINGS=1 STATUS_GATE_STATUS=FAIL

docs/generated/native-abi-report.json: BOUND_FUNCTIONS 87 -> 55
  plan.md:174: BOUND_FUNCTIONS=87 but the generated evidence says BOUND_FUNCTIONS=55
  NEXT.md:24: BOUND_FUNCTIONS=87 but the generated evidence says BOUND_FUNCTIONS=55
  docs/generated/native-abi-report.json is stale: tools/native_abi/verify.py
    produces different bytes
  STATUS_GATE_FINDINGS=3 STATUS_GATE_STATUS=FAIL
```

The third is the interesting one: it fails *twice over*, once because the prose
disagrees with the report and once because the report disagrees with a live run.
A gate that only compared prose to the report would have reported the prose as
the defect and been wrong, which is what a reader following the "trust the
generated report" rule would have concluded by hand.

## What was repaired

* `docs/generated/native-abi-report.json` regenerated: 55→87 bound functions,
  170→283 prototype positions, 21→28 layouts, 157→243 layout fields, 212→215
  constants. `MISSING_HEADER_SYMBOLS=0 MISSING_LIBRARY_SYMBOLS=0
  ABI_MISMATCHES=0` before and after.
* `plan.md`'s opening paragraph moved to Foundation 58 and says what
  Foundations 48–57 added.
* `NEXT.md` gained the historical marker.
* `README.md` and `NEXT.md` gained the gate in their verification lists.

## Live baseline this was measured against

```text
609 tests, 0 failures (debug)
REFERENCE_TYPES=257  REFERENCE_MEMBERS=2964
TARGET_TYPES=157     TARGET_MEMBERS=1973
COMPLETE_TYPES=150   PARTIAL_TYPES=7   MISSING_TYPES=100
TOTAL_DIAGNOSTICS=169  MISSING_TYPE=100  MISSING_MEMBER=63
OVERLOAD_MAPPING_MISMATCH=6   ALLOWLIST_ENTRIES=0
BOUND_FUNCTIONS=87  ABI_MISMATCHES=0
STATUS_GATE_FOUNDATION=58  STATUS_GATE_DERIVED_FACTS=149
STATUS_GATE_CLAIMS_CHECKED=80  STATUS_GATE_REPORTS_COMPARED=4
STATUS_GATE_FINDINGS=0
```
