# Phase 31 — Reliability

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-31001` … `STUDIO-31999` and are never reused.

**Purpose.** This is a content-authoring application. Losing work is unacceptable.

**Exit criteria.** Interrupted saves, corrupt files and crashes cost a user nothing they cannot recover, and nothing is repaired silently.

**Progress:** 4 of 13 complete `███░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-31001` | Autosave | ⬜ | — |
| `STUDIO-31002` | Crash recovery snapshots, offered rather than silently applied | ⬜ | `STUDIO-31001` |
| `STUDIO-31003` | Atomic writes for every authored file | ✅ | — |
| `STUDIO-31004` | Undo and redo stability under every editing path | ⬜ | `STUDIO-02035` |
| `STUDIO-31005` | Format migration chain runs on every load | ✅ | — |
| `STUDIO-31006` | Dirty-state tracking | ⬜ | `STUDIO-31001` |
| `STUDIO-31007` | Crash isolation from the game process | ⬜ | `STUDIO-16003` |
| `STUDIO-31008` | Malformed project and scene diagnostics that permit repair | ⬜ | — |
| `STUDIO-31009` | Unknown plugin components preserved through save and load | ✅ | — |
| `STUDIO-31010` | Tests for interrupted saves and partial files | ✅ | `STUDIO-31003` |
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

### `STUDIO-31010` — Tests for interrupted saves and partial files

**Acceptance.** The file an interrupted save leaves behind is recognised, refused or repaired —
never half-read and never silently re-identified.

**✅ Done, and the finding is that the reading half had a hole the writing half could not close.**
`STUDIO-31003` made every authored write atomic, so a save interrupted *from now on* leaves the
previous document whole. It does nothing about the files already out there: every sidecar and every
document written by a build before it truncated in place, and the temporary a crash leaves beside
the target survives the crash by design. Three properties, and the third was broken.

**1. A leftover temporary is not an asset.** `studioWriteFileAtomically` names its temporary
`<target>.cnatmp<ticket>`, so `Crate.png.cnatmp7` sits beside `Crate.png` after a crash. Imported,
it becomes a second asset with its own id and its own sidecar, referenced by nothing, that the user
cannot identify and will not know is safe to delete. The scan now skips it and says what it is —
**reported, not deleted** (`STUDIO-31011`), because if the *original* save was the one that failed
those bytes may be the only copy.

The shape is matched, not the substring: the suffix followed by digits, so a document a user
deliberately called `notes.cnatmp-ideas.txt` is theirs and is imported like any other file. A
scanner that hid it would be deciding what their files mean from a substring.

**2. Every authored loader refuses a partial file.** Probed with four shapes — truncated
mid-token, empty, whitespace only, and bytes that are not text — across scenes, prefabs, materials,
environment maps and the project file. **All five already refused**, and the material and
environment loaders already loaded into a local first so a refusal leaves the caller's open
document untouched rather than half-overwritten. No defect; the cases exist because the failure
they forbid is not a crash but a scene that loads with three of its ten entities, looks like a
scene, and is saved back over the original — at which point the interruption has cost the user the
seven entities the atomic write preserved.

Worth recording from the gate-verification: the scene half is held by **three** independent
refusals, and it took removing all three to let a partial file through. Deleting the JSON parse
check was not enough (the root-is-an-object check caught it), deleting that was not enough either
(the format-version gate caught it, because a truncated file reads as version 0 and the migration
chain refuses it). Defence in depth that nobody designed as such — but the case is live, not
vacuous, and now says so.

**3. A malformed sidecar does not change the asset's identity — and it used to.** This is the
defect, and it was written down three lines above the code that broke it. Where a sidecar
*migration* fails, `AssetDatabase::scan` keeps the id and explains why:

> *"The id survives even when nothing else does. Scenes reference assets by id (D-08), so
> regenerating it would break every reference in the project — a far worse outcome than an importer
> setting reverting to its default."*

The branch immediately below it, for a sidecar whose **JSON** would not parse, did exactly that:
*"a new id was assigned"*. And a half-written sidecar is precisely a sidecar whose JSON will not
parse. So the one failure `STUDIO-31003` cannot retroactively prevent — a sidecar truncated by a
build that predates it — silently re-identified the asset, and every scene, prefab and material
pointing at it stopped resolving. The project would open with materials unassigned and models
missing, and nothing would say why.

`studioRecoverAssetIdFromSidecar` reads the id straight out of the bytes. It costs nothing and it
nearly always works: `id` is the second key `recordToJson` writes, so a file cut anywhere after the
first forty bytes still holds it whole. The id is kept, the settings are lost, and **both halves are
reported** — what was rescued and what was not. Where no well-formed id can be recovered the new one
is assigned as before, but the warning now states the consequence (*"existing references to this
asset will not resolve"*) rather than reporting a routine assignment.

**Verification.** `tests/PartialFileTests.cpp` —
`ALeftoverTemporaryIsRecognisedByItsShapeAndNotByASubstring` (including a round-trip through the
writer, so a writer that changed its naming cannot agree with a test that did not),
`AnInterruptedSaveLeavesAFileTheScanSkipsAndReports`, `EveryAuthoredDocumentRefusesAPartialFile`
(five document types × four partial shapes), `AHalfWrittenSidecarKeepsTheAssetsIdentity` and
`ASidecarWithNoReadableIdSaysTheReferencesAreBroken` — the honest half, covering both a sidecar cut
before its id and one holding an id that is not a `Uuid`.

Checked by causing each: the recovery removed (the id changes, the scan counts a new asset, both
warnings vanish), the temporary skip removed (two half-written files imported as assets), the
name check reduced to a substring search (the user's own file disappears from the browser), the
material loader made to half-accept, and the scene loader stripped of all three refusals.

**What this row is not.** It does not recover a *document's* contents from a partial file — a
truncated scene is refused, not repaired, because the missing entities are not in the bytes. The
sidecar is the exception only because the one field whose loss damages other files happens to be
written early enough to survive. Recovering what is recoverable from a malformed document is
`STUDIO-31008`, and saying so is `STUDIO-31011`.

### `STUDIO-31005` — Format migration chain runs on every load

**Acceptance.** Every versioned format has an upgrade path, not just a refusal — and the two are
one piece of code, so a format cannot acquire the second without the first.

**✅ Done, and the finding is the same shape as `STUDIO-31003`'s: a rule written down, believed,
and absent.** `SceneDocument::loadFromJson` states it:

> *"The version gate and the upgrade path are one thing: refusing a file from the future and
> upgrading one from the past are both answers to 'what version is this?', and splitting them is
> how a loader comes to refuse a file it could have read."*

`docs/FORMATS.md` said it twice — *"a file from the past is read and upgraded"* and *"Both halves
are implemented"* — and **three formats had only the half that refuses.** `.cnamaterial`, `.cnaenv`
and the `.cnarecovery` envelope each carried a hand-written `if (version > kFormatVersion) return
false;` and nothing behind it. Four adherents and three holes, which is the same arithmetic the
atomic-write row found.

**Nothing would have shown it.** Every chain in the tree is empty, so a format with no chain behaves
exactly like a format with one — right up until somebody bumps a version, which is the day the files
are already written and the user is the one who finds out. That is the whole argument for holding
the rule structurally rather than by review, and `EveryVersionedFormatRunsAMigrationChain` now does:
a source file that writes a `formatVersion` and never mentions `FormatMigrator` is a format that
declares itself and cannot upgrade itself, and the guard refuses it. Two exemptions, each a file
that is not a document — the migrator itself, which stamps the version it has just upgraded *to*,
and the recent-projects list, which is rebuilt by opening projects and whose worst case is one empty
menu. Both are checked to be live.

**A second defect the hand-written gates carried, smaller and quieter.** Both read
`json["formatVersion"].asNumber(kFormatVersion)` — **defaulting a missing version to the current
one**. A `.cnamaterial` with no `formatVersion` at all was therefore read as though it were the
shape this build writes, which is a guess about a file whose shape is unknown, and it is the guess
that turns an unreadable file into a readable one with the wrong fields in it. `RecoveryStore` had
it worse: `asInt(0) > kFormatVersion` is false for a missing key, so an *unversioned snapshot* was
read and could be restored over work the user still had. The migrator refuses `version <= 0` by
name, as it always did for scenes, prefabs and projects.

**What the `.cnarecovery` chain is for, since the envelope is not an authored document.** It is the
one file whose loss costs the most — it holds the only copy of work the user has not saved — and a
snapshot from an older build with no route forward would be refused at exactly the moment it was
needed. The *scene* inside the envelope runs `SceneDocument`'s own chain when it is loaded, so the
two versions move independently, which is right: an envelope that gained a field has nothing to say
about the scene it is carrying.

**Where the guard came from is worth recording**, because the first attempt was wrong in an
instructive way. `RecoveryStore::formatMigrator()` as a static member function was rejected by
`NoServiceReachesAnotherThroughALocatorOrASingleton` — a `static` function handing out a reference
to a Studio type is a singleton accessor. The free-function shape every other migrator already used
(`getSceneFormatMigrator()`) was the convention, and an existing guard found it faster than review
would have.

**Verification.** `tests/CoreTests.cpp` — `TheRealFormatsRunTheirChainsOnEveryLoad` now names all
**seven** chains and checks each one's top against the `kFormatVersion` its document writes (a chain
stopping one below would upgrade every file on load and write it back at the version it started
from, which is a format that never settles);
`TheMaterialLoaderReadsTheUpgradedDocumentNotTheOriginal`, which matters more for a material than
for a scene because a material's *override set* is its key set (`STUDIO-19005`), so a loader that
ran the chain and read the original would upgrade the document and throw the upgrade away silently;
`TheEnvironmentMapLoaderRunsItsChainToo`; and
`AVersionFromTheFutureIsRefusedAndAMissingOneIsNotGuessedAt`, which covers both halves and the
behaviour change. `tests/ProjectAndAssetTests.cpp`'s `TheNewestSnapshotWinsAndACorruptOneIsSkipped`
gained the unversioned-snapshot case. And `EveryVersionedFormatRunsAMigrationChain` in
`tests/ArchitectureGuardTests.cpp`.

Checked by causing each: the material loader reading its input instead of its output (two
assertions, including the override set), the hand-written gates put back in both files (three cases
plus the guard), a loader's `migrate(` call removed, and a format's `FormatMigrator` mention
removed entirely.

**What this row is not.** It does not write a migration. Every chain is still empty and that is the
intended state — registering a step is not a licence to bump a version, and an additive field older
builds can ignore costs nothing and needs none. What changed is that the first real step is now a
small addition to a path that already runs on every load of every format, rather than a new path
for three of them.

### `STUDIO-31008` — Malformed project and scene diagnostics that permit repair

**Acceptance.** A tool that refuses to open a slightly broken file is one you cannot use to fix a broken file

### `STUDIO-31020` — Deterministic, version-control-friendly output throughout

**Acceptance.** Stable ordering, no unnecessary timestamps, no formatting churn, no opaque binary state for ordinary project metadata

