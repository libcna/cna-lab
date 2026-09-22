# Phase 24 — Audio

> **ARCHIVED — historical record. This file authorises no work.** It belongs to the
> [archived programme roadmap](../docs/ROADMAP-ARCHIVE.md), whose scope was retired on 2026-09-22
> by [ADR-001](../docs/ADR-001-SCOPE-REDUCTION.md). **A ⬜ below means *not built*. It no longer
> means *planned*.** The one authoritative active roadmap is [`plan.md`](../plan.md).
>
> **Disposition of this phase:** **Conditional future work.** See [ROADMAP-BACKLOG.md](../docs/ROADMAP-BACKLOG.md).
>
> Ids in this phase are `STUDIO-24001` … `STUDIO-24999` and are never reused. Every id here still resolves, so a commit, test or code comment that cites
> one keeps its meaning.

**Purpose.** Grow audio preview into an authoring workflow.

**Exit criteria.** An artist can audition, configure and place audio without leaving Studio.

**Progress:** 0 of 8 complete `░░░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-24001` | Audio source component authoring | ⬜ | `STUDIO-14001` |
| `STUDIO-24002` | Listener configuration | ⬜ | `STUDIO-24001` |
| `STUDIO-24003` | Volume, pitch, looping and attenuation | ⬜ | `STUDIO-24001` |
| `STUDIO-24004` | Spatial preview in the viewport | ⬜ | `STUDIO-11001` |
| `STUDIO-24005` | Play and stop from the Content Browser and the Inspector | ⬜ | `STUDIO-09001` |
| `STUDIO-24006` | Zones and buses where the runtime supports them | ⬜ | `STUDIO-24001` |
| `STUDIO-24007` | Waveform preview | ⬜ | `STUDIO-24005` |
| `STUDIO-24008` | Audio asset metadata | ⬜ | `STUDIO-09014` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-24005` — Play and stop from the Content Browser and the Inspector

**Acceptance.** Carried forward from the prototype, which could already audition a clip as the component will play it or as the file was imported

