# Phase 17 — Build profiles

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-17001` … `STUDIO-17999` and are never reused.

**Purpose.** Model the target matrix properly: platform, architecture, renderer, configuration and feature profile.

**Exit criteria.** A user picks a named profile, and Studio offers only combinations that can actually be built.

**Progress:** 6 of 12 complete `██████░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-17001` | Target profile data model | ✅ | `STUDIO-02040` |
| `STUDIO-17002` | Profile editing UI | ✅ | `STUDIO-17001` |
| `STUDIO-17003` | Target OS and platform implementation selection | ✅ | `STUDIO-17001` |
| `STUDIO-17004` | CPU architecture selection | ✅ | `STUDIO-17001` |
| `STUDIO-17005` | Renderer selection, validated against what CNA can build | ✅ | `STUDIO-17001`, `STUDIO-02030` |
| `STUDIO-17006` | Build configuration: Debug, Release, RelWithDebInfo, MinSizeRel | ✅ | `STUDIO-17001` |
| `STUDIO-17007` | Feature profile: what the game requires of a renderer | ⬜ | `STUDIO-17005` |
| `STUDIO-17008` | The GUI offers only meaningful combinations | ⬜ | `STUDIO-17007` |
| `STUDIO-17009` | Studio drives the project's own CMake, showing the exact commands | ⬜ | `STUDIO-17001` |
| `STUDIO-17010` | Build output parsed for navigation, with the original log always retained | ⬜ | `STUDIO-17009` |
| `STUDIO-17011` | Clean and incremental build | ⬜ | `STUDIO-17009` |
| `STUDIO-17012` | Features that are tri-state in CNA are tri-state in the profile | ⬜ | `STUDIO-17001` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-17001` — Target profile data model

**Delivered by `STUDIO-02040`.** `CNA/Studio/Project/TargetProfile.hpp`: six axes as a value, with a
project carrying as many profiles as it ships on and none of them privileged

### `STUDIO-17002` — Profile editing UI

**Acceptance.** A project's list of build targets can be managed from Studio — added to, renamed,
duplicated and removed — and every change to it survives the editor closing and can be taken back.

**✅ Done, and it found a defect the panel's own header described as solved.** `StudioBuildPanel`
says it exists because *"the legacy panel kept the chosen platform and backend in its own members,
so they were forgotten when the panel was closed and were never saved"*, and promises *"the profile
the next save writes"*. **There was no next save.** The panel wrote target profiles straight into
the open `Project` and set `result.profileChanged`, a flag **nothing anywhere read**. So a user who
chose a renderer got exactly what they asked for — in memory, until they quit. No undo entry, no
dirty marker, no prompt on close, nothing in the log. The one gesture that told them was pressing
Save All, which no one has a reason to press after changing a drop-down.

This is the third instance this session of the same shape: a rule stated in the file that breaks it.
The two project commands that already existed say it outright — *"a project change that lived only
in memory would be lost by a crash the recovery snapshot cannot help with; that snapshot holds the
scene, not the project"* — and the Build panel is the one project editor that did not follow them.

**Panels report, the binder acts.** The panel now returns a `StudioTargetProfileEdit` and
`StudioShellPanels` runs a `SetTargetProfilesCommand`, which is where the undo entry and the
write-through live. The whole list travels rather than a diff, because adding and removing a target
move the *selection* as well as the list and the two have to undo together — a user who undid an
"Add target" and found the selection pointing past the end of the list would be looking at a bug.

**And the list itself had no interface at all.** The six axes could be edited; the list they belong
to could not. A project shipping on two things could only say so by hand-editing its `.cnaproject`,
which is exactly the state this panel exists to remove. It now has Add, Duplicate, Remove and an
editable name.

Four decisions worth stating:

- **Add gives the host's defaults; Duplicate copies the current target.** Two different intentions,
  and a panel that made them the same would leave one button doing nothing anyone could see. The
  test asserts a duplicate equals its original in everything but its name, which is what makes it a
  duplicate rather than a second Add.
- **A new target gets a name nothing else is using.** Two rows called "Default" is not illegal — the
  list is ordered and the drop-down shows positions — but the user who added the second one cannot
  tell which they are editing. The duplicate gets a number, as a file manager does, rather than the
  panel refusing the gesture.
- **Remove is disabled on the last target rather than hidden** (`STUDIO-12004`). Removing it would
  leave a project that cannot be built and no row to add one from, and a button that vanished when
  it was the one thing the user was looking for reads as the panel being broken rather than as the
  operation being refused. The command refuses an empty list too, so the guard is not only in the
  pixels.
- **A project that already has no targets gets a way out**, not the dead end the old version drew.
  "This project declares no build target." followed by nothing was a state only a text editor could
  leave.

**Verification.** `tests/StudioBuildPanelTests.cpp` —
`ATargetEditIsWrittenToTheProjectFileAndCanBeUndone` (both halves, because they fail independently:
a command that did not write through would undo correctly and still lose the change on quit, and a
write-through that skipped the history would persist a change the user could not take back),
`ATargetEditThatChangesNothingIsNotACommand`, `TheTargetListCanBeEditedFromThePanel` — driven
through the real buttons, with every gesture checked on disk and then undone one at a time — and
`RemovingTheLastTargetIsRefusedRatherThanLeavingAProjectWithNone`.

The test harness now does what the binder does, which is what keeps these cases about the panel
rather than about a shortcut only the tests take.

**A vacuous assertion the gate-verification caught**, and it is the interesting one. The last-target
case first asserted `!result.profileEdit.has_value()` — which the harness had already cleared while
applying it, so the case passed for a click that simply missed the button. Enabling Remove
unconditionally did not fail it. The count of edits the panel has *reported*, across every frame and
surviving the settle that follows a click, is what distinguishes "the button is disabled" from "the
coordinates are wrong".

Checked by causing each: Remove enabled on the last target, the command's write-through removed, and
undo replaying the new value instead of the old.

**What this row is not.** It does not add validation the model lacks — which combinations are
offered is `STUDIO-17008`, and it still depends on `STUDIO-17007`'s feature profile. Nor does it
make the project *dirty*: the write-through means there is nothing to be dirty about, which is the
same bargain importer settings and the two existing project commands strike.

### `STUDIO-17003` — Target OS and platform implementation selection

**Acceptance.** Operating system and CNA platform implementation are separate choices, and a
platform CNA reserves but has not implemented is refused rather than quietly replaced by the default

**Also fixed here, and it was a real defect.** Studio's build runner passed
`-DCNA_GRAPHICS_BACKEND` when configuring a *user's game*. Current CNA does not define that variable
at all, so every game Studio configured silently took CNA's default renderer instead of the one the
user chose — and the build **succeeded**, which is exactly why it survived. It now passes
`CNA_GRAPHICS_RENDERER` and `CNA_PLATFORM`, and a test asserts the old name appears nowhere in the
configure command

### `STUDIO-17005` — Renderer selection, validated against what CNA can build

**Acceptance.** A profile naming a renderer CNA cannot build for its operating system is refused
before the build, with CNA's own reason. Note the scope: this is *buildability*, not capability —
whether the chosen renderer can do what the game needs is `STUDIO-17007`, and whether it can host
Studio is a different question again that must never be asked here

### `STUDIO-17006` — Build configuration

**Acceptance.** CMake's four: Debug, Release, RelWithDebInfo, MinSizeRel. The task originally said
"Debug, Development, Release, Shipping", which is another engine's vocabulary; Studio drives the
project's own CMake, so it uses CMake's

### `STUDIO-17007` — Feature profile: what the game requires of a renderer

**Acceptance.** A project needing modern CNAEXT rendering says so, and incompatible targets are reported before the build starts rather than after it fails

### `STUDIO-17009` — Studio drives the project's own CMake, showing the exact commands

**Acceptance.** The commands are visible and runnable by hand; the generated CMake stays editable

### `STUDIO-17010` — Build output parsed for navigation, with the original log always retained

**Acceptance.** Compiler errors are never hidden behind "Build failed"

### `STUDIO-17012` — Features that are tri-state in CNA are tri-state in the profile

**Acceptance.** A profile can say *off*, *use it if the machine has it*, or *require it* for any CNA
option that has three states, and the Build panel offers all three. Today the model is a boolean per
feature, with a per-feature spelling of "on" as the escape hatch

**Why it is not just tidiness.** `CNA_ENABLE_VIDEO` takes `OFF`, `AUTO` or `ON`, and `ON` *requires*
FFmpeg: it fails the configure on a machine without it. Studio's boolean mapped "on" to `ON`, which
made every exported project require FFmpeg to build — found by `STUDIO-02051` building an exported
game rather than reading it. The stop-gap maps "on" to `AUTO`, which is right for a project that
merely wants video and wrong for one that cannot ship without it; that project currently has to
override `CNA_ENABLE_VIDEO` by hand

**Verification.** `TurningVideoOnAsksCnaToUseFfmpegIfPresentRatherThanToRequireIt` pins the current
behaviour and will need rewriting when this lands, which is the intent
