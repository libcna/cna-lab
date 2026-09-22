# ADR-002 — The four questions the roadmap left silent

| | |
|---|---|
| **Status** | Accepted |
| **Date** | 2026-09-22 |
| **Decided by** | The repository owner |
| **Implements** | [`../plan.md`](../plan.md) `CORE-08` |
| **Answers** | `STUDIO-01014`, `STUDIO-01015`, `STUDIO-01016`, `STUDIO-00015` |
| **Affects** | [`ARCHITECTURE.md`](ARCHITECTURE.md), [`../ANALYSIS.md`](../ANALYSIS.md), `include/CNA/Studio/Core/UserPaths.hpp` |

## Context

Four rows survived the whole programme as *unanswered questions* rather than as unbuilt features.
[`ADR-001`](ADR-001-SCOPE-REDUCTION.md) put CNA Studio on a path to maintenance mode, and under
maintenance nobody returns to open questions: they are either answered now or they are answered by
whoever trips over them, years from now, with none of the context that would make the answer cheap.

`CORE-08` exists to close them. For two of the four the answer is *no work is needed*, and the
value of this document is that it says so with the reasoning attached, rather than leaving a ⬜ that
reads as an unpaid debt. `STUDIO-01014`'s own acceptance put it best: **silence is not an answer.**

Each decision below is stated, then justified, then made checkable — because a decision recorded
only in prose is one the next refactor can undo without noticing.

---

## Decision 1 — No compatibility shims are provided for the renamed public API (`STUDIO-01014`)

**Decision.** CNA Studio ships **no** deprecated `CNA::Editor` aliases, no `cna-editor` CMake target
aliases and no transitional headers. The rename performed by `STUDIO-01002`…`STUDIO-01013` is
complete and final, and there is no removal date to publish because there is nothing to remove.

**Why.** A compatibility shim is owed to a consumer. This repository has none, and the absence is
structural rather than incidental:

| Surface a consumer could depend on | State |
|------------------------------------|-------|
| Installed headers | None. `CMakeLists.txt` contains no `install()` rule of any kind |
| An exported CMake package | None. No `export()`, no `*Config.cmake`, no `find_package` contract |
| A released binary or tagged version | None. The repository carries no tags and no release workflow |
| A published source archive | None. The one import is recorded in [`ORIGIN.md`](ORIGIN.md) and went the other way |

The prototype it was renamed from lived inside the `cna-lab` monorepo and was never distributed
under its own name. A shim would therefore be maintained for a consumer that cannot be named, which
is the definition of a permanent tax with no payer — the same argument `ADR-001` used to put a
plugin SDK out of scope.

**The one real compatibility surface is the plugin C ABI, and it is already versioned.**
`CNA/Studio/Plugins/Plugin.hpp` declares `kStudioPluginApiVersion`, a plugin records the version it
was built against, and `StudioPluginManifest::isCompatible()` requires an exact match before
`dlopen` gets anywhere near the code. That mechanism is the compatibility policy for the only thing
that has ever had a version boundary, and it is unchanged by this decision.

**If an external consumer ever appears**, the shim question reopens on its own terms — with a named
consumer, a named version to be compatible with, and a removal date that means something. That is a
new decision, not this one being wrong.

**Checked by.** `LegacyEditorIdentifiersSurviveOnlyInHistoricalRecords` in
`tests/ArchitectureGuardTests.cpp` fails if a `CNA::Editor`, `CnaEditor`, `CNA_EDITOR` or
`cna-editor` identifier reappears in the source, headers, tests or build files — so the decision
cannot be silently half-reversed by a shim added "just in case".

---

## Decision 2 — The state and configuration directories stay `cna-studio`, and nothing is migrated (`STUDIO-01015`)

**Decision.** Studio's user files live under `cna-studio`, resolved by
`getStudioConfigDirectory()` and `getStudioStateDirectory()` in
`include/CNA/Studio/Core/UserPaths.hpp`. **No migration from the prototype's `cna-editor`
directories is written**, and Studio never reads them.

**Why the rename half is already done.** The row's title says *rename the state and configuration
directory*, and that happened as a side effect rather than as a task: the prototype resolved its
paths inline — a layout file under `$CONFIG/cna-editor` in `Main.cpp`, recovery snapshots under
`$STATE/cna-editor/recovery` in `RecoveryStore.cpp` — and both were replaced when `UserPaths` was
extracted. Studio has written nothing but `cna-studio` since.

**Why the migration half is not owed.** A migration is owed to a person with files in the old
place, and the population of such people is empty by the same argument as Decision 1: the prototype
was never released, never installed and never packaged. It ran from a build tree inside `cna-lab`.
There is no user who has a `$XDG_CONFIG_HOME/cna-editor` written by a *distributed* build, because
no distributed build existed.

What such a directory would hold, if one somehow did, sharpens the point rather than blunting it:

| Prototype file | What losing it costs | Would a migration even work? |
|----------------|----------------------|------------------------------|
| `$CONFIG/cna-editor` layout | A window arrangement, rebuilt by dragging two panels | No. The prototype's layout format predates docking, saved layouts and the workspace store; there is no mapping from it to a `StudioWorkspaceStore` document |
| `$STATE/cna-editor/recovery` | Unsaved work from a crash — the one thing here that genuinely matters | No. The snapshots are pre-migration scene documents whose format the loader would refuse, and a recovery snapshot is only of interest for hours, not for the years since |

So the migration would be code that cannot be exercised, for data that cannot be read, belonging to
nobody — and it would run on every start-up forever, on the strength of a `stat` that always fails.
**Writing it would be worse than not writing it**, because an untestable path that runs on every
start-up is a defect waiting for a machine that has an unrelated `cna-editor` directory for some
other reason.

**A developer who does have a prototype directory** — someone who ran `cna-lab`'s build tree in
2026 — loses a window arrangement and nothing else, and can delete `~/.config/cna-editor` and
`~/.local/state/cna-editor` by hand. That is the entire remedy, and it is written down here so that
the person who wonders about it finds an answer instead of a silence.

**Checked by.** `StudioUserDirectoriesAreStudioNamedAndMigrateNothing` in
`tests/StudioWorkspaceStoreTests.cpp`: both resolved directories end in `cna-studio` under a
controlled environment, and a `cna-editor` directory planted beside them is still there, untouched
and unread, after Studio has resolved both paths.

---

## Decision 3 — `ANALYSIS.md` is retired, in place, as a historical record (`STUDIO-01016`)

**Decision.** `ANALYSIS.md` stays in the repository, keeps its original text, and carries a banner
marking it historical and naming [`ARCHITECTURE.md`](ARCHITECTURE.md) as the document that
supersedes it. Its sixteen decisions `D-01`…`D-16` are restated in `ARCHITECTURE.md` §11. It is
**not** deleted and **not** edited to look current.

**Why not delete it.** Around eighty references across the source tree cite `ANALYSIS.md` by
decision id — `D-03` for the CNA-optional build, `D-06` for undo as a hard rule, `D-14` for the
toolkit boundary. Those comments explain why code looks the way it does, and deleting their target
would turn each into a dangling citation. The archive rule in `plan.md` applies here for the same
reason it applies to `plans/`: a record is only useful if the things pointing at it still resolve.

**Why not edit it current.** The document's value is that it says what was believed and verified on
a particular day, against a particular CNA revision (`ac3aaae`, 2026-08-03). A record quietly
revised to match today stops being evidence of anything. `ARCHITECTURE.md` §2 exists precisely to
state what changed in CNA since — the renderer axes, the 50 renderer identities, the capability
model — rather than to have those corrections back-fitted into the older text.

**What this means for a reader.** `ARCHITECTURE.md` is correct where the two disagree; that
sentence is in the banner. `ANALYSIS.md` is read for *why*, never for *what is true now*.

**Checked by.** `AnalysisMdIsRetiredAndArchitectureMdCarriesItsDecisions` in
`tests/ArchitectureGuardTests.cpp`: `ANALYSIS.md` opens with a historical banner naming
`docs/ARCHITECTURE.md`, and `ARCHITECTURE.md` carries the section that restates `D-01`…`D-16`.

---

## Decision 4 — The benchmark suite supersedes the start-up and frame-cost row (`STUDIO-00015`)

**Decision.** `STUDIO-00015` asked for start-up time and headless frame cost to be measured cheaply
and recorded for later comparison. The row is **superseded** by `--ui-benchmark`, which measures the
thing the row was reaching for, continuously and under a CI gate. The numbers the row itself asked
for are recorded once, below, and are not maintained thereafter.

**Why superseded rather than answered.** The row's *headless frame cost* turns out to measure
nothing. Studio's headless mode runs the null UI, which describes no geometry and submits none; on
the measurements below, **120 additional headless frames cost 0 ms above a single one**. A number
that does not move when the work it measures multiplies by a hundred is not a baseline.

`--ui-benchmark` measures what the row wanted instead: the cost of *describing* a frame of real UI,
per panel scenario, in microseconds, against an explicit interactive budget — 4167 µs, a quarter of
a 60 Hz frame, with 8333 µs for deliberately extreme scenarios. It reports the cheapest frame of
the run, because that statistic moved least under load (2.3%, against 10.6% for the median;
`STUDIO-33027`). It is a living gate, not a recorded number.

**The numbers, measured once, for the record.** GCC 13.3.0, C++23, `-DCNA_STUDIO_WITH_CNA=OFF`, no
GPU and no window; wall-clock over 9 runs, `basic-sample` project; `--ui-benchmark` over 120 frames
per scenario at 1920×1080:

| Measure | Release | Debug |
|---------|--------:|------:|
| Process start to exit, `--version` (no project, no shell) | 3 ms | — |
| Start-up: open a `basic-sample` project headless, one frame, exit | **152 ms** min, 155 ms median | **513 ms** min, 521 ms median |
| The same with 120 further headless frames | 151 ms min, 154 ms median | 512 ms min, 530 ms median |
| Implied cost of a headless frame | **~0 µs** | ~0 µs |
| Describing a frame of the shell as it opens (`baseline`) | **118 µs** min, 3% of budget | 1376 µs min, 33% of budget |
| The most expensive scenario (`outliner-20000-all-selected`) | 6251 µs min, 75% of its 8333 µs budget | 27 193 µs, over |

Two things in that table are worth a maintainer's attention. **Start-up is dominated by fixed
cost**, not by the project: `--version` returns in 3 ms, so the remaining ~149 ms is font
rasterisation, the asset database and the shell coming up, and it does not grow with scene size.
And **Debug is roughly 11× Release on UI description**, which is why the benchmark's budgets are
read on a Release build and why a Debug run reporting `OVER` is not a regression.

**How to reproduce, rather than trust this table:**

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j4
./build-release/cna-studio --ui-benchmark
```

**Checked by.** The benchmark's own gate: `StudioUiBenchmarkTests.cpp` holds the cost model to exact
hand-computed figures, and the CNA-backed leg requires the two real backends' counters to agree
with it. Nothing here needs a new test, which is the point of superseding the row.

---

## Consequences

**Good.**

- Four rows stop being outstanding work. Two of them were never work; that is now written down.
- Three of the four decisions are enforced by a test, so they cannot be reversed by accident — only
  on purpose, by someone who has to edit the test and read why it exists.
- A maintainer asking "should I write the migration?" or "is there a shim policy?" finds an answer
  with its reasoning, at the cost of one document read.

**Costs, stated plainly.**

- A developer who ran the `cna-lab` prototype loses their window arrangement. Decision 2 accepts
  that explicitly rather than paying for an untestable migration to avoid it.
- The start-up figures in Decision 4 are a snapshot on one machine and will drift. They are labelled
  as a one-time record with a reproduction recipe, not as a gate, so drift is expected rather than a
  defect.

**Neutral.**

- `ANALYSIS.md` remains in the repository indefinitely, out of date on CNA and correct about why.
  That is the intended end state, not a deferred deletion.

## Reversing this

Each decision reverses independently, and only Decision 1 and Decision 2 are likely to want to:

- **Decision 1** reopens the moment a named external consumer exists — a published package, an
  installed header set or a second repository building against these headers. The shim policy is
  then written for that consumer, with a real removal date.
- **Decision 2** reopens if a prototype-era directory is found on a machine belonging to someone who
  did not build `cna-lab` themselves, which would mean the prototype was distributed after all.
- **Decision 3** reverses only by someone prepared to repoint every `D-NN` citation in the source.
- **Decision 4** reverses if headless frames ever stop being free — that is, if Studio grows a
  headless mode that does real per-frame work. Then a headless frame cost means something and is
  worth gating.
