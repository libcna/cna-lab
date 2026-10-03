# Phase 21 — Animation

> **ARCHIVED — historical record. This file authorises no work.** It belongs to the
> [archived programme roadmap](../docs/ROADMAP-ARCHIVE.md), whose scope was retired on 2026-09-22
> by [ADR-001](../docs/ADR-001-SCOPE-REDUCTION.md). **A ⬜ below means *not built*. It no longer
> means *planned*.** The one authoritative active roadmap is [`plan.md`](../plan.md).
>
> **Disposition of this phase:** **Conditional future work**, apart from `STUDIO-21009`, folded into `CORE-10`. See [ROADMAP-BACKLOG.md](../docs/ROADMAP-BACKLOG.md).
>
> Ids in this phase are `STUDIO-21001` … `STUDIO-21999` and are never reused. Every id here still resolves, so a commit, test or code comment that cites
> one keeps its meaning.

**Purpose.** Skeletal and sprite animation authoring and preview.

**Exit criteria.** An animator can browse, preview and configure animation without leaving Studio.

**Progress:** 0 of 9 complete `░░░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-21001` | Skeletal animation preview | ⬜ | `STUDIO-10009` |
| `STUDIO-21002` | Animation clip assets and clip browser | ⬜ | `STUDIO-21001` |
| `STUDIO-21003` | Playback controls: play, pause, step, scrub | ⬜ | `STUDIO-21001` |
| `STUDIO-21004` | Timeline | ⬜ | `STUDIO-21003` |
| `STUDIO-21005` | Keyframes | ⬜ | `STUDIO-21004` |
| `STUDIO-21006` | Curve editing | ⬜ | `STUDIO-21005` |
| `STUDIO-21007` | Events and notifies | ⬜ | `STUDIO-21004` |
| `STUDIO-21008` | Blend configuration where the runtime supports it | ⬜ | `STUDIO-21001` |
| `STUDIO-21009` | Preserve the existing sprite animation support without regression | ⬜ | `STUDIO-07007` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-21009` — Preserve the existing sprite animation support without regression

**Acceptance.** Clip of sheet frame indices, Inspector preview with play/pause/step, viewport drawing the same frame, playback deliberately outside the document — all carried forward

