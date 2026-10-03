# `plans/` — archived phase detail

> **Everything in this directory is a historical record. None of it authorises work.**

These thirty-six files hold the per-task detail of the **archived programme roadmap**
([`../docs/ROADMAP-ARCHIVE.md`](../docs/ROADMAP-ARCHIVE.md)), whose scope was retired on 2026-09-22
by [ADR-001](../docs/ADR-001-SCOPE-REDUCTION.md).

**The one authoritative active roadmap is [`../plan.md`](../plan.md).**

## How to read a ⬜ here

It means **not built**. It does *not* mean planned, owed, queued or next. Of the 176 unfinished
rows in this directory:

| Where the row went | Count | Document |
|--------------------|------:|----------|
| Active, authorised work | 24 | [`../plan.md`](../plan.md), consolidated into 11 `CORE-*` deliverables |
| Conditional — reconsidered only on a demonstrated need | 100 | [`../docs/ROADMAP-BACKLOG.md`](../docs/ROADMAP-BACKLOG.md) |
| Out of active product scope | 52 | [`../docs/ROADMAP-OUT-OF-SCOPE.md`](../docs/ROADMAP-OUT-OF-SCOPE.md) |

Each phase file carries a banner naming its own disposition. A guard test in
`tests/ArchitectureGuardTests.cpp` fails if any unfinished row is dropped from that classification,
appears in two of the three, or names an id that does not exist.

## Why these files are kept

- **405 completed tasks** with their acceptance criteria, their verification, the defects each one
  found, and the reasoning behind decisions that are still in force. This is the most detailed
  record of why CNA Studio is built the way it is.
- **The `STUDIO-PPNNN` id space**, which commits, code comments and tests cite by name. Ids are
  stable, are never reused and still resolve — including for work that will never be built.
- **The dependency graph** between tasks, which is still checked for dangling references and
  cycles.

The arithmetic of every file here is still verified against `docs/ROADMAP-ARCHIVE.md` by the test
suite, so the record cannot quietly rot. That is maintenance of a record, not of a plan.

## If you are about to implement something from here

Don't — unless it appears in [`../plan.md`](../plan.md). If you believe a row here should be built,
the route is the four-part test in `plan.md` under *How something legitimately becomes active
work*, which starts with a real maintained CNA application and a demonstrated bottleneck.
