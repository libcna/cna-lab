# Phase 31 — Reliability

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-31001` … `STUDIO-31999` and are never reused.

**Purpose.** This is a content-authoring application. Losing work is unacceptable.

**Exit criteria.** Interrupted saves, corrupt files and crashes cost a user nothing they cannot recover, and nothing is repaired silently.

**Progress:** 6 of 13 complete `█████░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-31001` | Autosave | ✅ | — |
| `STUDIO-31002` | Crash recovery snapshots, offered rather than silently applied | ✅ | `STUDIO-31001` |
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

**Acceptance.** Carried forward from the prototype and retested through the Studio UI.

**What "autosave" means here, since the word is ambiguous.** It does **not** write over the user's
document on a timer. It writes a `.cnarecovery` snapshot beside it, every `autosaveSeconds`, while
the document differs from its file — the shape ED-903 arrived in, and the reason is that a timer
that saved the document would make "I have not saved yet" impossible to mean. The file on disk stays
the user's last deliberate save until they take the offer in `STUDIO-31002`.

The flow itself was already carried forward and already driven by the native shell:
`StudioRecoverySession` holds the timer and the store, `StudioShellPanels::pollRecovery` runs it off
the clock the host already passes, the interval is read from the preference every poll (so changing
it takes effect without a restart, and so does a host assigning preferences straight from disk), and
zero means off — with the snapshot dropped the moment the document matches its file, because an
offer to recover work that is already saved teaches users to dismiss the one that mattered.

**✅ Done, and the finding is a defect in the half nobody looks at: the user with no project.**

`update()` has no condition about a project, so a scene edited before one exists was snapshotted
every interval, correctly and atomically. `scan()` began with `if (!context_.hasProject()) return
false;`, so nothing could ever offer one back. And `pollRecovery` scanned when the project *path
changed* — which, with no project open, is empty on both sides from the first frame, so the scan
never ran at all. Two halves that hid each other: the snapshots existed, were well-formed, and were
unreachable forever.

**Which user this cost is the whole point.** Creating an entity is deliberately enabled with no
project open, and `StudioShellActions` says why: *"a user trying out the editor before creating a
project can still build a scene, and refusing them would be refusing the first thing they try."* So
the one person with nothing saved anywhere — the person who has most to lose to a crash and least
idea that they do — was the one person autosave silently could not help.

Both halves are fixed where they are: a project-less scene's snapshot is filed under an empty
project path, which is exactly what `getFilePath()` returns with no project open, so it is found the
same way everything else is; and the scan now runs on the first poll rather than on a change, with
an explicit "have not looked yet" flag, because *"no project"* and *"have not looked"* are the same
string and are not the same state.

**A loose end recorded rather than fixed.** A snapshot written with an empty project path is not
dropped when the user later opens a project — the scene it belonged to is gone from the editor, the
session's `written_` flag now tracks a different scene id, and nothing sweeps the directory. It costs
a file, not work, and inventing a retention policy for `.cnarecovery` is a decision of its own rather
than a detail of this row. `STUDIO-31002`'s offer will surface it at the next start-up with no
project, which is where it can be discarded from the File menu.

**Verification.** `tests/StudioRecoveryTests.cpp` —
`TheNativeShellWritesSnapshotsWhileTheSceneIsUnsaved`, `ASnapshotIsDroppedOnceTheDocumentMatchesItsFile`,
`AnAutosaveIntervalOfZeroWritesNothing`, `TurningAutosaveOffDoesNotHideWorkThatIsAlreadyOnDisk`, and
the two this row added: `ASceneBuiltBeforeAProjectExistsIsAutosavedAndCanBeRecovered` and
`TheRecoveryScanRunsOnTheFirstPollRatherThanOnAChange` — the second pinned separately because it is
the invisible half, and because with a project open the path *does* change from empty to the
project's, which is why every existing case passed over it.

Checked by causing each: `scan()`'s project condition put back, and the first-poll scan reduced to a
change-detector again. Both fail the new cases by name and leave every older one green, which is the
shape of a gap rather than a regression.

### `STUDIO-31002` — Crash recovery snapshots, offered rather than silently applied

**Acceptance.** A snapshot from a previous session is *offered*. The document in front of the user
is never replaced without them saying so.

**✅ Done.** There is no crash handler, on purpose, and `RecoveryStore` gives the reason: the
reliable half of crash recovery is the part that runs *before* the crash. One serialising a document
from inside `SIGSEGV` calls `malloc` and the filesystem with a corrupted heap.

What a found snapshot gets is a **warning** in the log, a **sticky notification** carrying the scene
name and when it was written, and two File-menu commands — Recover Unsaved Scene and Discard
Recovered Scene — both greyed out when there is nothing to answer for, because a Discard that is
live with nothing to discard is a row a user has to read twice to be sure of. The scene on screen is
untouched until one of them is pressed.

Four decisions that make the offer honest:

- **Recovering marks the document unsaved**, and the log says the file on disk is unchanged. The
  recovered work has never been written anywhere, and saying otherwise would let the user close the
  editor believing it had.
- **A recovery that fails keeps the snapshot.** It is not a reason to delete the only copy of the
  work it was holding.
- **Autosave is suspended while an offer stands**, and says so once. The snapshot file is keyed by
  scene id, so the current session's unsaved seconds would overwrite the previous session's unsaved
  hours.
- **Turning autosave off does not hide work already on disk.** Off stops new snapshots being
  written; it cannot mean one already there becomes unreachable — which is what the Dear ImGui host
  did, and would have cost a user who reached for the setting *after* a crash everything.

**Verification.** `tests/StudioRecoveryTests.cpp` — `WorkFromAPreviousSessionIsOfferedRatherThanFound`
(including that the scene is *not* silently replaced), `RecoveringTakesTheOfferAwayAndLeavesTheWorkUnsaved`,
`DiscardingRemovesTheSnapshotAndTheOffer`, `BothCommandsAreGreyedOutWhenThereIsNothingToAnswerFor`,
`BothCommandsAreOnTheFileMenuWhereTheLogSaysTheyAre` — the log tells the user where to find them, so
a message naming a menu item that is not there is a message that wastes their time —
`AutosaveIsSuspendedWhileWorkFromAPreviousSessionIsWaiting`, `TheOfferIsMadeAgainWhenAnotherProjectIsOpened`,
and `TurningAutosaveOffDoesNotHideWorkThatIsAlreadyOnDisk`. All driven through a *second* `StudioShellPanels`
over the same snapshot directory, because `scan` reads the disk and nothing short of a second editor
exercises it.

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

