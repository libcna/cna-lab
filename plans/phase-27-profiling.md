# Phase 27 — Profiling and diagnostics

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-27001` … `STUDIO-27999` and are never reused.

**Purpose.** Professional performance and debugging tools.

**Exit criteria.** A developer can find out why their game or their editor session is slow, from inside Studio.

**Progress:** 1 of 14 complete `░░░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-27001` | Frame time and CPU timing | ⬜ | `STUDIO-07011` |
| `STUDIO-27002` | GPU timing where CNA exposes it | ⬜ | `STUDIO-27001` |
| `STUDIO-27003` | Draw calls, triangles, texture and resource counts | ⬜ | `STUDIO-27001` |
| `STUDIO-27004` | Memory reporting | ⬜ | `STUDIO-27001` |
| `STUDIO-27005` | Asset load timing | ⬜ | `STUDIO-10011` |
| `STUDIO-27006` | Build timing | ⬜ | `STUDIO-17009` |
| `STUDIO-27007` | Renderer and platform capability reporting | ⬜ | `STUDIO-02021` |
| `STUDIO-27008` | Live player status | ⬜ | `STUDIO-16002` |
| `STUDIO-27010` | Frame debugger | ⬜ | `STUDIO-27003` |
| `STUDIO-27011` | Render-pass and resource inspection | ⬜ | `STUDIO-27010` |
| `STUDIO-27012` | Draw-call inspection | ⬜ | `STUDIO-27010` |
| `STUDIO-27013` | Remote profiling | ⛔ | `STUDIO-27008` |
| `STUDIO-27020` | Console and Output Log as a production log panel | ✅ | `STUDIO-07005` |
| `STUDIO-27021` | Very large logs stay responsive | ⬜ | `STUDIO-27020`, `STUDIO-30010` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-27002` — GPU timing where CNA exposes it

**Acceptance.** Uses CNA's `GpuTimer` and the `GpuTimers` renderer feature; absent support is reported, not faked

### `STUDIO-27020` — Console and Output Log as a production log panel

**Acceptance.** Severity, source and category, timestamps, search, filters, clear, copy, pause, auto-scroll, hyperlinks to files, entities and assets; build, game and Studio logs distinguished

**Done.** Severity, clear, copy and auto-scroll were already there from `STUDIO-07005`. Five things
were not, and two of the five turned out to be one defect wearing a disguise.

**Source was a prefix pretending to be a field.** `STUDIO-16002` gave the game's output a
`"Player: "` prefix and wrote down why — "a marker at the start of the line is what a glance lands
on" — and deferred a per-source filter here. The reasoning was right about glancing and wrong about
where the information lives: a prefix cannot be filtered without matching strings, cannot be
coloured apart from the message, and stops being true the moment somebody rewords it. `LogSource`
is now the field it was standing in for, with three values — Studio, Build, Game — and the prefix
and its constant are gone. The reason it was *one stream and not three panels* was the good half of
that comment and has moved to `LogSource`'s own documentation rather than being deleted with it.

**Timestamps are session seconds, and told rather than read.** `setNow` is advanced once a frame
from the same monotonic clock the asset watcher and the recovery session take, which is what stops
a test having to sleep to make two stamps differ. Relative, not wall-clock: the question an editor
log answers is "how long after I pressed Play did this happen", and a relative time has no
timezone, no locale and no midnight. A collapsed repeat keeps the time of its **first** arrival —
"this started at 12.4 s and has happened four hundred times" is the reading that helps, and the last
occurrence is always approximately now.

**Search commits on Enter rather than filtering per keystroke**, matching the Outliner
(`STUDIO-13002`). That is a consistency argument rather than a claim that either is better: two
search boxes in one application that answer the same typing differently is worse than either.
Case-insensitive, matching anywhere in the line.

**Pause freezes the view and never the log.** It records a mark — entries *ever appended*, not
entries currently held, because the log is bounded and a mark over the living would slide backwards
under a busy log and reveal the lines the user paused to avoid — draws up to it, and says how many
are waiting. Unpausing shows them. Nothing is dropped, which is the whole difference between a
pause and a mute.

**Links are attached by whoever logs, never parsed out of the message.** A console that scanned its
own text for things that look like paths would find them in prose, miss them in quotes it did not
expect, and break the day somebody reworded a message — and there is a test that clicks every row
of a line *mentioning* `Content/Hero.png` and requires nothing to happen. The site that writes
"could not import 'Content/Hero.png'" already holds the id of what it was importing, so that is
where the link comes from; the import-failure path is wired that way now. Entities and assets
travel as ids rather than paths (ANALYSIS.md D-08), because a log line outlives a rename. The panel
**reports** the clicked link and `StudioShellPanels::followLogLink` acts on it — the Output Log is
handed a log and nothing else, and a panel that could select an entity would be a panel that needed
a scene.

**Ten tests, each verified by deliberate breakage**: collapsing across sources, restamping a
repeat, pause not holding, the source filter ignored, every row treated as a link, a one-sided case
fold, and Copy taking the whole log instead of what is shown — each fails its own case by name.

**Left for `STUDIO-27021`**, which is what it is for: the list is virtualised already, but nothing
here has been measured against a multi-hundred-thousand-line log, and the per-frame filter is a
linear pass over every retained entry. That is fine at the ten thousand this log keeps and is the
obvious thing to measure before raising the cap.

### `STUDIO-27021` — Very large logs stay responsive

**Verification.** Stress test with a multi-hundred-thousand-line log

