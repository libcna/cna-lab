# Phase 31 — Reliability

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-31001` … `STUDIO-31999` and are never reused.

**Purpose.** This is a content-authoring application. Losing work is unacceptable.

**Exit criteria.** Interrupted saves, corrupt files and crashes cost a user nothing they cannot recover, and nothing is repaired silently.

**Progress:** 2 of 13 complete `█░░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-31001` | Autosave | ⬜ | — |
| `STUDIO-31002` | Crash recovery snapshots, offered rather than silently applied | ⬜ | `STUDIO-31001` |
| `STUDIO-31003` | Atomic writes for every authored file | ✅ | — |
| `STUDIO-31004` | Undo and redo stability under every editing path | ⬜ | `STUDIO-02035` |
| `STUDIO-31005` | Format migration chain runs on every load | ⬜ | — |
| `STUDIO-31006` | Dirty-state tracking | ⬜ | `STUDIO-31001` |
| `STUDIO-31007` | Crash isolation from the game process | ⬜ | `STUDIO-16003` |
| `STUDIO-31008` | Malformed project and scene diagnostics that permit repair | ⬜ | — |
| `STUDIO-31009` | Unknown plugin components preserved through save and load | ✅ | — |
| `STUDIO-31010` | Tests for interrupted saves and partial files | ⬜ | `STUDIO-31003` |
| `STUDIO-31011` | Nothing is silently repaired; every change to user data is reported | ⬜ | `STUDIO-31008` |
| `STUDIO-31020` | Deterministic, version-control-friendly output throughout | ⬜ | `STUDIO-02037` |
| `STUDIO-31021` | Generated files have explicit ownership and regeneration rules | ⬜ | `STUDIO-15008` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-31001` — Autosave

**Acceptance.** Carried forward from the prototype and retested through the Studio UI

### `STUDIO-31003` — Atomic writes for every authored file

**Acceptance.** A save that is interrupted — by a crash, a full disk, a killed process — leaves the
previous document whole. The path is never observed half-written.

**✅ Done, and the finding is that the rule was already written down and already believed.**
`RecoveryStore.cpp` states it exactly:

> *"Rename over the old snapshot rather than truncating it in place. A crash during a snapshot then
> leaves the previous one intact, which is the whole point: a half-written recovery file fails to
> load at the one moment it is needed, having already convinced the user their work was safe."*

Three more files followed it — the preferences, the workspace layout and the recent-projects list —
each with its own copy of the temp-and-rename dance. **Every document did not.** Scenes, prefabs,
materials, environment maps, `.cnaasset` sidecars and the project file all opened the target with
`std::ios::trunc`, which empties it *before* the first byte of the replacement is written. The
crash-recovery snapshot was being written safely to protect files that were not.

Four adherents and eight holes is not a convention. `studioWriteFileAtomically` is the procedure,
in `cna-studio-core`, and twelve call sites now go through it — including the four that were doing
it by hand, whose copies are gone.

**What it promises, and what it does not.** Atomicity: the path holds either the whole previous
document or the whole new one, because `rename` within a directory replaces in one step. Not
durability across a power cut — that needs an `fsync` of the file *and* of its directory, which the
standard library cannot express and which would cost a synchronous disk write on every save. A
power cut may lose the most recent save; it may not corrupt the document. Not a lock, either: two
processes writing one file still race, and Studio has one writer per project.

The temporary is written **beside the target**, never in the system temp directory, because a
rename is atomic only within one filesystem — and across one, `std::filesystem::rename` fails
outright on some platforms and silently degrades to copy-then-delete on others, which is the
non-atomic write this exists to remove.

**A test gap worth recording, because it was found by the gate-verification rather than by
review.** Breaking the writer so that it opened the *target* directly and renamed it onto itself
passed every behavioural case: the bytes were right, the folder was clean, the failures were
reported. Atomicity only shows under an interruption a test cannot create, and the permission
tricks that would force one are ignored for a process running as root — which is how CI runs. So
the property is held structurally instead, by a source scan honest about being one: the writer must
open its stream on the temporary and must give that temporary a distinct name. The *first* version
of that guard looked for the word `temporary` and passed the same break; it now asks for the line
that derives the name.

**Verification.** `tests/StudioFileWriteTests.cpp` —
`AnAtomicWriteProducesTheBytesItWasGiven` (byte for byte, embedded nulls and CRLF included, and no
temporary left behind), `AnAtomicWriteReplacesAnExistingFileWithoutTruncatingIt` (a shorter
document does not leave the tail of the longer one, and an empty document is a document),
`AnAtomicWriteCreatesTheFoldersAboveItsTarget`, `AFailedWriteLeavesThePreviousDocumentIntact`, and
`TwoWritesInOneMomentDoNotShareATemporary` — which is Save All, where two documents landing on one
temporary would rename it twice and put one document's bytes under the other's name.

And `EveryAuthoredFileIsWrittenAtomically` in `tests/ArchitectureGuardTests.cpp`, which refuses any
`std::ofstream` in `src/` outside four named exemptions — the writer itself, a build *log*, a
zero-byte writability probe that is deleted immediately, and benchmark scaffolding. The exemptions
carry their reasons and are checked to be **live**: one naming a file that no longer writes
anything is a licence nobody revoked.

Checked by causing each: a document returned to truncating in place (caught twice, by the
`std::ofstream` refusal and by the missing call), and the writer opening its target directly.

**A defect the fix itself created, found by running the suite and looking at the working tree.**
Making the writer create the folders above its target is right for a document being saved into a
new one — a material in a new folder, a project being created — and it turned two silent failures
into directories appearing. `AssetDatabase::writeSidecar` was being called with **no project root**
by several tests, where `resolvePath` returns the relative path unchanged, so sidecars had been
aimed at the process's working directory all along and had simply failed to land. With folders
being created they landed, in the source checkout.

Both refusals now live where the missing thing is. A database with no project root does not write a
sidecar at all, and a sidecar whose asset folder is absent is refused rather than built — metadata
belongs *beside* a file, so a missing folder means the asset is missing too. Neither changes
anything for a real project, where the folder is there because the asset is.

**What this row is not.** It does not make a *sequence* of writes atomic. Saving a scene and its
sidecar is two atomic writes, and a crash between them leaves one updated and one not — which is
consistent for each file and is what `STUDIO-31001`'s autosave and `STUDIO-31002`'s recovery
snapshots exist to cover. Nor does it verify what was written: a document that serialises wrongly
is written wrongly, atomically. That is `STUDIO-31011`'s territory.

### `STUDIO-31008` — Malformed project and scene diagnostics that permit repair

**Acceptance.** A tool that refuses to open a slightly broken file is one you cannot use to fix a broken file

### `STUDIO-31020` — Deterministic, version-control-friendly output throughout

**Acceptance.** Stable ordering, no unnecessary timestamps, no formatting churn, no opaque binary state for ordinary project metadata

