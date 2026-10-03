# Phase 34 — Release engineering

> **ARCHIVED — historical record. This file authorises no work.** It belongs to the
> [archived programme roadmap](../docs/ROADMAP-ARCHIVE.md), whose scope was retired on 2026-09-22
> by [ADR-001](../docs/ADR-001-SCOPE-REDUCTION.md). **A ⬜ below means *not built*. It no longer
> means *planned*.** The one authoritative active roadmap is [`plan.md`](../plan.md).
>
> **Disposition of this phase:** `STUDIO-34001` is now part of `CORE-11`. Packaging Studio itself is [conditional](../docs/ROADMAP-BACKLOG.md).
>
> Ids in this phase are `STUDIO-34001` … `STUDIO-34999` and are never reused. Every id here still resolves, so a commit, test or code comment that cites
> one keeps its meaning.

**Purpose.** Ship Studio itself.

**Exit criteria.** A user can install a versioned CNA Studio without building it.

**Progress:** 0 of 6 complete `░░░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-34001` | Versioning scheme and release notes process | ⬜ | — |
| `STUDIO-34002` | Linux packaging | ⬜ | `STUDIO-34001` |
| `STUDIO-34003` | Windows packaging | ⬜ | `STUDIO-34001` |
| `STUDIO-34004` | macOS packaging | ⬜ | `STUDIO-34001` |
| `STUDIO-34005` | Reproducible release builds | ⬜ | `STUDIO-34002` |
| `STUDIO-34006` | Upgrade path for preferences and workspace layouts | ⬜ | `STUDIO-05011`, `STUDIO-06010` |
