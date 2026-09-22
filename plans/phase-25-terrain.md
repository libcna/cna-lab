# Phase 25 — Terrain and world tools

> **ARCHIVED — historical record. This file authorises no work.** It belongs to the
> [archived programme roadmap](../docs/ROADMAP-ARCHIVE.md), whose scope was retired on 2026-09-22
> by [ADR-001](../docs/ADR-001-SCOPE-REDUCTION.md). **A ⬜ below means *not built*. It no longer
> means *planned*.** The one authoritative active roadmap is [`plan.md`](../plan.md).
>
> **Disposition of this phase:** **Conditional future work.** See [ROADMAP-BACKLOG.md](../docs/ROADMAP-BACKLOG.md).
>
> Ids in this phase are `STUDIO-25001` … `STUDIO-25999` and are never reused. Every id here still resolves, so a commit, test or code comment that cites
> one keeps its meaning.

**Purpose.** Large environment authoring, well after core scene editing is robust.

**Exit criteria.** A large world can be authored and organised without the tool falling over.

**Progress:** 0 of 7 complete `░░░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-25001` | Terrain import | ⬜ | `STUDIO-10004` |
| `STUDIO-25002` | Height editing and sculpting | ⬜ | `STUDIO-25001` |
| `STUDIO-25003` | Painting and material layers | ⬜ | `STUDIO-19001` |
| `STUDIO-25004` | Foliage placement | ⬜ | `STUDIO-25001` |
| `STUDIO-25005` | Procedural placement | ⬜ | `STUDIO-25004` |
| `STUDIO-25006` | Large-world organisation | ⬜ | `STUDIO-13008` |
| `STUDIO-25007` | Streaming and partitioning, if a real project needs it | ⛔ | `STUDIO-25006` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-25007` — Streaming and partitioning, if a real project needs it

**Acceptance.** Deferred until a real project demonstrates the need. Building world partitioning speculatively would be a large subsystem serving nobody

