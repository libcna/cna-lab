# Phase 31 — Reliability

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-31001` … `STUDIO-31999` and are never reused.

**Purpose.** This is a content-authoring application. Losing work is unacceptable.

**Exit criteria.** Interrupted saves, corrupt files and crashes cost a user nothing they cannot recover, and nothing is repaired silently.

**Progress:** 11 of 13 complete `██████████░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-31001` | Autosave | ✅ | — |
| `STUDIO-31002` | Crash recovery snapshots, offered rather than silently applied | ✅ | `STUDIO-31001` |
| `STUDIO-31003` | Atomic writes for every authored file | ✅ | — |
| `STUDIO-31004` | Undo and redo stability under every editing path | ✅ | `STUDIO-02035` |
| `STUDIO-31005` | Format migration chain runs on every load | ✅ | — |
| `STUDIO-31006` | Dirty-state tracking | ✅ | `STUDIO-31001` |
| `STUDIO-31007` | Crash isolation from the game process | ⬜ | `STUDIO-16003` |
| `STUDIO-31008` | Malformed project and scene diagnostics that permit repair | ✅ | — |
| `STUDIO-31009` | Unknown plugin components preserved through save and load | ✅ | — |
| `STUDIO-31010` | Tests for interrupted saves and partial files | ✅ | `STUDIO-31003` |
| `STUDIO-31011` | Nothing is silently repaired; every change to user data is reported | ✅ | `STUDIO-31008` |
| `STUDIO-31020` | Deterministic, version-control-friendly output throughout | ✅ | `STUDIO-02037` |
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

### `STUDIO-31004` — Undo and redo stability under every editing path

**Acceptance.** Undo puts the document back — all of it, every time, on every path into it.

**✅ Done, and it found two defects that every existing test was structurally unable to see.**

`tests/CommandTests.cpp` checks each command against the fields it was expected to touch: the entity
is back, the name is back, the property is back. That is the right test for *does this command do
its job*, and it is blind to the failure this row is about — a command that restores what it **knew**
it changed and leaves behind something it did not know it changed.

So the property here is stronger and blunter: **undo restores the document byte for byte.**

> serialise → execute → serialise → undo → serialise → redo → serialise → undo → serialise

with the first, third and fifth equal and the second and fourth equal. Byte equality is only
meaningful because `STUDIO-02037` made the writer deterministic; before that this could not have
been written. Fourteen scene-editing paths, a composite, and the project's four, each also asserting
the command **changed something** — a factory that quietly built an invalid command is the commonest
way a table like this goes vacuous, and it would pass every check that follows.

**Defect one: undoing a delete reordered the file.** `DeleteEntityCommand::undo` re-added through
`addEntity`, which appends, so an entity taken from the middle of the list came back at the end.
Nothing about the *scene* changes — hierarchy is by parent id, not by position, which is exactly why
nothing noticed — but the saved bytes do. "Make a change, undo it, save" produced a modified file,
and a user who took an edit back still had a diff to explain. `SceneDocument::insertEntity` puts
each one back where it was, ascending, so earlier insertions have already made the list the right
length for later ones.

**Defect two: a build-target edit rewrote a field it was not asked to touch.** `kDefaultRenderer` is
CNA's upper-case identity `OPENGLES3` and is the right fallback for a project file that names no
renderer. But `Project` also used it to initialise the in-memory mirror, while every setter that
touches a profile writes the **catalogue's** lower-case `opengles3` — deliberately, because
`validateStudioTargetProfile` normalises to it so that renderer comparisons are not case-insensitive
everywhere. So a fresh project carried `OPENGLES3` until the first target edit and `opengles3` for
ever after: adding a build target and undoing it left `defaultGraphicsBackend` changed, in a field
the source calls *"a serialized contract that the player's discovery and existing project files
depend on"*. The mirror now initialises from the default profile, which is where every other value
of it comes from. `kDefaultRenderer` stays exactly where it belongs.

Both are the same shape, and it is the shape byte-comparison exists to find: a change nobody
intended, in a part of the document the command never looked at.

**Verification.** `tests/UndoStabilityTests.cpp` —
`EveryEditingPathUndoesToTheDocumentItStartedFrom` (fourteen paths: create, delete with children,
delete a leaf, duplicate, rename, disable, lock, reparent, reparent to the root, set a property, add
a component, remove a component, transform several at once, change the environment),
`ACompositeUndoesAsOneAndInReverse` — `studio.entity.group` is a create plus N reparents and the
order matters both ways, since undoing forwards would delete the group while its children still
point at it — and `EveryProjectEditUndoesToTheProjectItStartedFrom`.

Each path runs **two** full cycles, because a command can restore correctly once and not twice: one
that moved rather than copied its saved state has nothing left the second time.

Checked by causing each: `undo` appending instead of inserting, and the mirror initialised from
`kDefaultRenderer` again. Both fail by name, which is how the defects were found in the first place.

**What this row is not.** It does not cover the commands whose document is a *file* — materials,
environment maps, importer settings, prefabs on disk. Those write through on every apply and their
undo replays the previous bytes through the same writer, which `STUDIO-31003` covers; a byte
comparison of their documents would be a comparison of the same call. Nor does it cover the
`AssetDatabase` commands, which mutate a database rather than a serialisable document.

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

### `STUDIO-31006` — Dirty-state tracking

**Acceptance.** Studio's answer to "does this have unsaved changes?" is one answer, it is the
document's, and it is shown wherever a user would look for it.

**✅ Done, and the mechanism was already right.** `CommandHistory` tracks a *saved cursor* rather
than a flag: `isDirty()` is "the cursor is not where the last save left it", so undoing back to the
saved position makes the document clean again and nothing has to remember to clear anything. Six
places ask it — the quit dialog, the status bar, New Scene, Play (which saves first), the comparison
service and autosave — and none of them keeps a copy.

**Two defects in what is *shown*, and they are the same defect twice.**

The status bar's mark was `hasProject() && isDirty()`. `requestQuit` is plain `isDirty()`. So a user
who built a scene before creating a project — which Studio allows on purpose, because *"refusing
them would be refusing the first thing they try"* — saw a bar reporting nothing modified, and was
then stopped on the way out by a dialog telling them the scene had unsaved changes. Two answers to
one question, and the wrong one was the one they read while deciding whether to close the window.
The bar's own comment says why that is worse than no mark: *"an unsaved-changes mark that can be
wrong is worse than none, because it is the one thing a user checks before closing the window."*
This is the second row this session to find `hasProject()` used as a proxy for "there is a document"
when the document exists either way; `STUDIO-31001` was the first.

And **`StudioShell::setPanelModified` had no callers at all** — not in `src/`, not in the suite. The
method has existed since the shell did, `studioTab` draws a dot for it, and nothing ever set it. A
dead affordance is worse than a missing one: a user who looks at a tab for a mark and never finds
one learns that the tabs do not have marks, and stops looking on the day it would have mattered. The
Viewport's tab now carries the scene's mark, from the same `isDirty()` the bar reads, so the two are
one answer rather than two that agree today.

**What is deliberately *not* dirty-tracked, and why that is not a gap.** Materials, environment maps,
importer settings and the project's own document are **written through** on every edit — each is one
command that saves as it applies. So there is no unsaved state for them to be in, which is the same
bargain `SetImporterSettingCommand` struck long before this row and the one `STUDIO-17002` extended
to build targets. The scene is the only document Studio holds unsaved, and it is the only one with a
mark.

**Verification.** `tests/StudioStatusBarTests.cpp` — `AnUnsavedSceneIsMarkedAndTheMarkGoesWhenItIsSaved`
(the existing case, with a project), `AnUnsavedSceneIsMarkedEvenBeforeThereIsAProjectToSaveItIn`, and
`TheViewportTabCarriesTheScenesUnsavedMark` — which also asserts the mark *goes*, so it is a mark
rather than a badge the panel wears once and keeps, and that the bar and the tab give the same
answer. Every one drives a real `DeleteEntityCommand`, because "the document is dirty" is the
history's answer and a test that set a flag would not be asking it.

Checked by causing each: the `hasProject()` condition put back (both cases fail), and the
`setPanelModified` call removed (the tab case alone fails, which is the shape of two independent
properties rather than one).

### `STUDIO-31008` — Malformed project and scene diagnostics that permit repair

**Acceptance.** A tool that refuses to open a slightly broken file is one you cannot use to fix a
broken file.

**✅ Done.** The refusals were already right — `STUDIO-31010` probed every authored loader and found
each one refusing a partial file cleanly and leaving the caller's document untouched. What was wrong
was that a refusal is not a diagnostic.

**A byte offset is not a location.** Three loaders reported `'<path>': <message> at offset 48213`.
That is the right thing for the parser to produce and the wrong thing to show a person: nobody can
find offset 48213 in a file without counting, and the tool that produced the number is the one that
could have counted. `Json::locate` turns an offset into a line, a column and **that line's text**;
`Json::describeFailure` puts them together in one row, so the usual case — a missing colon, a
trailing comma, a smart quote a word processor put in — is recognisable without opening the file at
all. Scenes, prefabs, projects and `.cnaasset` sidecars all report it now.

Four things the description has to survive, because it runs on files that are *already* broken: an
offset one past the end (where "unexpected end of input" points, and the most common failure a
truncated file produces); a CRLF file, whose carriage return belongs to the terminator and not to
the excerpt; a four-thousand-character line, trimmed around the fault rather than wrapped, because
this reaches a log row and a status bar; and an unprintable byte, which is often the fault itself
and shows as a placeholder rather than vanishing into the message.

**The column is counted in bytes**, and the header says so rather than glossing it: a line with a
multi-byte character before the fault reads one column further along than some editors show. The
line number is what finds the fault and it is exact.

**And the reason was being thrown away at three boundaries.** Each is the same shape — a layer that
had the answer, replacing it with the fact:

- `StudioShellPanels::openProjectFromHub` logged `"'<path>' could not be opened."` one line after
  `StudioContext::openProject` had already worked out and logged exactly what was wrong.
- `openStudioStartupDocument` set `"Could not open '<path>'."`, with a comment saying that sentence
  exists *"for the places a log sink does not reach — standard error, and the status bar"* — which
  is precisely where a bare refusal leaves a user with nothing to act on.
- **`StudioStatusModel::problem` had no setter anywhere.** The field, and the code in `publishStatus`
  that clears it when a project opens, have existed since the bar did, and nothing ever assigned it.
  So the one surface a user is guaranteed to be looking at said "No project open" — true and useless
  — while the reason sat in a panel they may not have had open. That is the second dead affordance
  in two rows, after `setPanelModified` in `STUDIO-31006`.

`openProject` and `openScene` now hand the reason back through an optional out-param as well as
logging it, and all three callers use it.

**Verification.** `tests/CoreTests.cpp` — `AParseFailureIsDescribedWhereAPersonCanFindIt` (the line,
the column, the excerpt, and that the parser's own words are *kept*: this says where, it does not
replace what) and `ALocationSurvivesTheEdgesOfTheTextItIsGiven` (first byte, past the end, empty
document, CRLF, a 4 000-character line, an unprintable byte). `tests/PartialFileTests.cpp` —
`ARefusedDocumentSaysWhichLineIsWrong` across scene, prefab and project, which also asserts the path
is still in the message (a line number without a file is useless in a project with forty scenes) and
that "offset" is *gone* rather than sitting beside the new text — and
`AMalformedSidecarsWarningSaysWhichLineIsWrong`. `tests/StudioStartupDocumentTests.cpp` —
`AStartupFailureSaysWhichLineOfTheFileIsWrong`, both halves, plus a missing file, which must read as
one sentence rather than one with an empty reason bolted on.

Checked by causing each: `locate` counting the wrong terminator (twelve assertions across four
files), and the startup path dropping the reason again.

**One piece of wiring this does not cover with a case, stated rather than implied.** The status-bar
assignment in `openProjectFromHub` is reached only by pressing Open on a rendered Project Hub, and
no harness drives that panel through the shell yet. The reason reaching that function *is* covered,
by the `openProject` out-param; what is not is the three lines that put it on the bar.

**What this row is not.** It does not repair a malformed document — a truncated scene is refused,
not reconstructed, because the missing entities are not in the bytes. The one thing that *is*
recovered is a sidecar's id, which `STUDIO-31010` did and which works only because the id is written
early enough to survive. Nor does it fix the material and environment-map inspectors, which say
*"written by a newer Studio, or is not valid JSON"* — two states a user needs to tell apart, and
`MaterialLoadProblem` gives them one value. That is a narrower gap and a change to a public enum;
recorded here rather than folded in.

### `STUDIO-31011` — Nothing is silently repaired; every change to user data is reported

**Acceptance.** Studio repairs rather than refuses — and says so, every time.

**✅ Done, and the finding is that the repairing was right and one of the reportings was missing.**

Studio repairs in a dozen places, and each decision is right on its own: a scene whose parent entity
was deleted by a bad merge should still open, or the user cannot fix it, which is `STUDIO-31008`'s
whole complaint. What makes the *set* dangerous is that every repair is written back on the next
save. A repair nobody was told about is a change to somebody's file that they did not make, did not
see, and will find in a diff weeks later with no idea what caused it.

So: **repair freely, report always.** Eleven of the twelve already did.

**The twelfth was a property whose value the declared type cannot hold.** `docs/FORMATS.md` states
it as a rule — *"Property present but the wrong shape | Falls back to the type's zero value"* — and
the fallback was silent. A `"position": "over there"` became `[0, 0, 0]`, the scene opened looking
fine, and the next save wrote the zeros over what the user had typed. It is the quietest kind of
loss: the file still parses, the editor still works, and the value is simply gone.

`studioJsonMatchesPropertyType` answers "can this be read as the declared type without losing what
it says", and `propertyValueFromJson` reports when the answer is no — through the whole nesting, so
a field inside a structure inside a list is named by its path. The message carries the entity and
the component as well as the property, because *"position holds text"* in a scene with four hundred
entities names the one thing the user already knows, and it names the **consequence**: saving will
write the replacement, which is the moment their value is actually gone.

**The conservatism is the design, not a shortcut.** A warning that fires on ordinary documents is a
warning users learn to scroll past, and then the one that mattered scrolls past with it. So:

- **Absent is not a mismatch.** It means "use the default" by contract, and that is what lets a
  component gain a property without every document already written becoming one with a hole in it.
- **Too many elements is not a mismatch.** A four-element array read as a `Vector3` loses nothing
  the type can hold. Too *few* is, because the reader fills the rest from a default and the document
  said something shorter.
- **A broken reference is not a mismatch.** An asset reference naming an id nothing has is
  well-formed text; scene validation reports the missing reference, with a message this could not
  give. Two reports of one fact is how a Problems panel becomes noise.

**Verification.** The inventory is the deliverable. `EveryAutomaticChangeToADocumentIsReported` in
`tests/PartialFileTests.cpp` loads **one** scene carrying six repairs at once — a loader that
reported the first and stopped would pass six single-fault cases — and asserts each is named: a
scene with no id, an entity with no id, a missing parent, a duplicate entity id, a component nothing
registered, and a value the declared type cannot hold. Then the same for the asset scan's two: a
malformed sidecar's recovered id and a half-written file skipped. A repair added later without a
message fails here rather than shipping, which is the point of gathering them in one place.

Plus `APropertyTheFileCannotHoldIsReplacedAndSaidSo` and `AnOrdinarySceneProducesNoReplacementWarnings`
in `tests/SceneTests.cpp` — the second being the half that makes the first usable, covering an
absent property, a longer-than-needed array, a reference to nothing, and a scene Studio wrote
itself, none of which may say a word.

Checked by causing each: the missing-parent warning deleted, and the replacement note silenced.
Both fail the inventory *by name*, which is what the inventory is for.

**What this row is not.** It does not make every report equally visible. Warnings reach the log, and
scene *validation* findings reach the Problems panel, which is a better surface for something a user
should act on — but a load-time repair happens once, before any panel has drawn, and routing it into
a panel would mean holding it somewhere. Recorded rather than done: which of these belong in
Problems is a question about that panel, not about the repairs.

### `STUDIO-31020` — Deterministic, version-control-friendly output throughout

**Acceptance.** Stable ordering, no unnecessary timestamps, no formatting churn, no opaque binary
state for ordinary project metadata.

**✅ Done.** Three of the four were already held and are now *checked*; the fourth was not held, and
the templates themselves were breaking it.

**Stable ordering and no timestamps** are `STUDIO-02037`'s, done as part of it: a round trip through
a fresh document for every authored format, and a source scan refusing the clock in every writer.

**No opaque binary state** is held by construction and was never a gap. Every project format is
JSON, for the reasons `docs/FORMATS.md` opens with, and everything Studio can regenerate lives under
the user's state directory rather than in the project — `STUDIO-09015`'s rule, which chose that over
a `Library/` folder precisely because *"not version-controlled then depends on a `.gitignore` entry,
and a rule enforced by a file somebody can delete is a rule that will eventually be broken by
somebody who did not know it existed."* `ASessionLeavesNothingInTheProjectButSidecars` is the gate.

**No formatting churn is the one that was broken**, and it is the one a user meets first: they clone
a colleague's project, open it, look around, press Ctrl+S out of habit, and `git status` should have
nothing to say. `OpeningAProjectAndSavingItLeavesEveryFileAsItWas` creates a project from **every
shipped template** — a template being exactly the project somebody else generated and handed over —
opens it through a real `StudioContext`, saves, and compares every byte of every file.

**All three template scenes failed.** They were hand-written with one-space indentation and
alphabetically sorted keys; Studio writes two spaces and insertion order. So the first save of every
new project reformatted its own starting scene, and the user's first commit carried two hundred
lines they did not write. The templates are now regenerated through Studio's own writer, which is
what a template should be: the output of the generator, not a hand copy of it.

**And the case had to learn a distinction rather than assert a simpler rule.** A `.cnaasset` sidecar
*does* change on the first scan of a new project, because the scan reads facts out of the files
themselves — a texture's dimensions — and records them. That is new information, not a rewrite, and
`StudioContext::openProject` already says so: *"Only what changed is written back, so opening a
project twice produces no diff."* So the case runs **two** sessions: a sidecar may move once,
everything else may not move at all, and **nothing** may move twice. A rule that forbade the first
change would have been wrong, and one that allowed any change would have caught nothing.

**Verification.** `OpeningAProjectAndSavingItLeavesEveryFileAsItWas` in
`tests/DeterministicOutputTests.cpp`, over all four shipped templates, checking in both directions
so that a file *disappearing* is caught too. Plus `STUDIO-02037`'s round trips and clock scan, and
`STUDIO-09015`'s session gate.

Checked by causing it: a template scene put back to its hand-written formatting fails the case by
file and by template id.


