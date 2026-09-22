# Phase 23 — Particles and VFX

> **ARCHIVED — historical record. This file authorises no work.** It belongs to the
> [archived programme roadmap](../docs/ROADMAP-ARCHIVE.md), whose scope was retired on 2026-09-22
> by [ADR-001](../docs/ADR-001-SCOPE-REDUCTION.md). **A ⬜ below means *not built*. It no longer
> means *planned*.** The one authoritative active roadmap is [`plan.md`](../plan.md).
>
> **Disposition of this phase:** **Conditional future work.** See [ROADMAP-BACKLOG.md](../docs/ROADMAP-BACKLOG.md).
>
> Ids in this phase are `STUDIO-23001` … `STUDIO-23999` and are never reused. Every id here still resolves, so a commit, test or code comment that cites
> one keeps its meaning.

**Purpose.** Author whatever the CNA runtime can execute — without building a new particle runtime inside Studio.

**Exit criteria.** Effects authored in Studio run in the game, and the runtime boundary stays where it belongs.

**Progress:** 0 of 4 complete `░░░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-23001` | Particle definition asset model | ⬜ | `STUDIO-19001` |
| `STUDIO-23002` | Emitter property authoring | ⬜ | `STUDIO-23001` |
| `STUDIO-23003` | Preview in the viewport | ⬜ | `STUDIO-11011` |
| `STUDIO-23004` | Runtime ownership stays with CNA or a project plugin, not with Studio | ⬜ | `STUDIO-23001` |
