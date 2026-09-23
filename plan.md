# CNA Studio — the active roadmap

> **This is the one authoritative roadmap. Work is authorised from this file and from nowhere
> else.**
>
> Three other documents describe unfinished work, and none of them authorises any of it:
> [`docs/ROADMAP-BACKLOG.md`](docs/ROADMAP-BACKLOG.md) is the conditional future backlog,
> [`docs/ROADMAP-OUT-OF-SCOPE.md`](docs/ROADMAP-OUT-OF-SCOPE.md) is what has left the product, and
> [`docs/ROADMAP-ARCHIVE.md`](docs/ROADMAP-ARCHIVE.md) with [`plans/`](plans/) is the archived
> programme roadmap kept for its history. **A ⬜ in any of those three means *not built*. It does
> not mean *planned*.**
>
> Why the scope changed, and what CNA Studio is now:
> [`docs/ADR-001-SCOPE-REDUCTION.md`](docs/ADR-001-SCOPE-REDUCTION.md).
> Architecture: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## What CNA Studio is

**A lightweight visual development companion for CNA applications.**

C++ source code, CMake, CLion and other IDEs, and ordinary CNA application development remain
first-class and authoritative. Studio provides visual workflows only where they materially improve
CNA development, and a developer must be able to stop using it at any point without losing
anything.

**What CNA Studio is not**, stated so that a future reader does not have to infer it: not a
replacement for CLion or Visual Studio, not a C++ IDE, not Unity, not Unreal, not Godot, not a
general-purpose DCC tool, and not a reason to reproduce functionality that standard development
tools already provide adequately.

**The invariant is unchanged and is not negotiable:** a project authored in Studio is an ordinary
CNA project that builds, runs and ships with Studio uninstalled. `docs/ARCHITECTURE.md` §1.

## The Core workflow

Everything in this roadmap exists to make these eleven steps reliable, and nothing else is
authorised:

1. create or open a CNA project
2. browse, import and manage its assets
3. create, open and edit scenes
4. use the viewport, selection and transform gizmos
5. edit the object, material and light properties Studio already models
6. save reliably
7. launch and play the CNA application
8. configure and invoke normal CMake builds
9. see useful build and runtime diagnostics
10. recover safely from crashes and interrupted work
11. continue normal C++ work in CLion or another external IDE

When those eleven are reliable and tested, **CNA Studio is complete and enters maintenance mode.**

## Where Core stands

Most of it is already built. This is the reason the remaining plan is small, and a reader who
starts implementing from the archived roadmap will rebuild things that work.

| Core step | State today |
|-----------|-------------|
| 1. Create / open a project | **Done.** Project Hub with New, Open and Recent; four templates, each created, reopened, configured, compiled and run by a CTest case |
| 2. Assets | **Done, bar finding them.** UUID-stable asset database, importers, Content Browser with card grid and cached thumbnails. No search or type filter — `CORE-05` |
| 3. Scenes | **Done.** Scene and prefab documents, undo on every mutation, format migration, tilemaps, layers and tags |
| 4. Viewport, selection, gizmos | **Done, bar seeing the selection.** 2D and 3D viewports, camera navigation, translate/rotate/scale gizmos, band selection. Nothing marks the selected entity in the viewport — `CORE-03` |
| 5. Properties | **Done, bar the grid's shape.** Details Inspector on the descriptor model, materials, lights, environment. The label column is a fixed 38% — `CORE-04` |
| 6. Save | **Done.** Atomic writes, dirty tracking, autosave, partial-file recovery |
| 7. Play | **Done.** Play, Pause, Step, Stop, Restart against a real separate player process, with crash isolation, log routing, live asset reload and screenshot capture |
| 8. CMake builds | **Partly.** Target profiles, renderer and platform validation, and a build runner that drives the project's own CMake. Clean vs incremental, and the tri-state CNA options, are open — `CORE-01` |
| 9. Diagnostics | **Partly.** Console with source attribution, Problems panel, validation. Compiler errors are not navigable — `CORE-01` |
| 10. Crash recovery | **Done.** Recovery store, autosave, crash reporting from the player, format-migration refusals with upgrades |
| 11. External IDE | **Not wired.** `StudioPreferences::externalEditor` exists and has no caller — `CORE-02` |

Beneath that: 405 completed tasks, 1784 unit assertions across the dependency-free suite, a
CNA-backed CTest suite, architecture guard tests, golden-image tests, sanitizer and `-Werror` CI
legs.

## The active roadmap

**9 of 11 active deliverables complete.**

Estimates are Opus 5 engineering hours at this repository's working standard — implementation, the
test its acceptance names, and the documentation it changes.

| Id | Deliverable | Status | Est | Traces |
|----|-------------|:------:|----:|--------|
| `CORE-01` | Build the project, and understand the failure | ✅ | 6 | `STUDIO-17009`, `STUDIO-17010`, `STUDIO-17011`, `STUDIO-17012`, `STUDIO-15012` |
| `CORE-02` | Hand the developer back to their IDE | ✅ | 2 | `STUDIO-15010` |
| `CORE-03` | The viewport shows what is selected | ✅ | 2 | `STUDIO-35053` |
| `CORE-04` | The Details panel reads as a property grid | ✅ | 2 | `STUDIO-35033` |
| `CORE-05` | Find an asset, find an entity | ✅ | 3 | `STUDIO-35042`, `STUDIO-35061` |
| `CORE-06` | Paint order separable from input order | ✅ | 4 | `STUDIO-03041` |
| `CORE-07` | Resize without artefacts | ✅ | 2 | `STUDIO-04011` |
| `CORE-08` | Answer the four questions the plan left silent | ✅ | 2 | `STUDIO-00015`, `STUDIO-01014`, `STUDIO-01015`, `STUDIO-01016` |
| `CORE-09` | Documentation to use and maintain the product | ✅ | 5 | `STUDIO-33001`, `STUDIO-33002`, `STUDIO-33003`, `STUDIO-29006` |
| `CORE-10` | A regression baseline for the Core workflow | ⬜ | 5 | `STUDIO-33020`, `STUDIO-21009`, `STUDIO-35010` |
| `CORE-11` | Declare Core complete and enter maintenance mode | ⬜ | 2 | `STUDIO-34001` |

**35 hours estimated.** Budget: a **40-hour target** and a **60-hour hard ceiling**. The ceiling is
part of the product definition, not a forecast: if a deliverable below turns out to need
substantial new infrastructure, it is cut down or moved to the backlog. It is never a reason to
raise the budget.

`CORE-01` … `CORE-11` are the whole of the authorised work. The ids are stable and are never
reused. Traced `STUDIO-*` ids are the archived tasks each deliverable subsumes; they keep commits,
tests and code comments that cite them resolvable, and they are the only `STUDIO-*` ids this
document authorises.

---

## The deliverables

### `CORE-01` — Build the project, and understand the failure

**Why it is required.** Core steps 8 and 9. A visual companion that cannot build the project it is
looking at sends the developer to a terminal for the one operation they perform most, and a build
that fails with nothing but *Build failed* is worse than no build button.

**Done when.**

- The Build panel configures and builds the open project through the project's own CMake, and every
  command it runs is displayed, selectable and produces the same result when pasted into a shell.
- Clean and incremental builds are separate gestures, and each does what its name says.
- A compiler error appears as a row naming file, line and message; activating the row opens that
  file at that line in the configured external editor (`CORE-02`).
- The complete unparsed build log is always reachable, whatever the parser made of it.
- A CNA option that is tri-state in CNA (`OFF` / `AUTO` / `ON`, as `CNA_ENABLE_VIDEO` is) is
  tri-state in the target profile, which retires the stop-gap that maps *on* to `AUTO`.

**Not included.** Feature profiles (`STUDIO-17007`) and combination filtering (`STUDIO-17008`);
cooking, packaging and export; build timing; any generated build system Studio would own.

**Done.**

| Acceptance | How | Evidence |
|------------|-----|----------|
| Every command displayed, selectable, and the same pasted into a shell | The rows in the panel are truncated to its width, so a `Copy Commands` button hands over every step untruncated and shell-quoted | `TheCommandsCanBeTakenAwayAndPastedIntoAShell` |
| Clean and incremental are separate gestures | Two buttons and two commands, `Build` and `Clean Build`; a clean job runs the project's own `clean` target *between* the configure and the build, so it works on a build tree nobody has generated yet | `CleanAndIncrementalAreSeparateGesturesAndEachDoesWhatItsNameSays` |
| A compiler error is a row naming file, line and message, and activating it opens that place | `BuildDiagnostics.hpp` parses GCC/Clang, MSVC and CMake shapes out of the log; the rows go through `CORE-02`'s editor command | `BuildDiagnosticTests.cpp` (7 cases, fixtures recorded from real output); `ACompilerErrorBecomesARowThatOpensTheFileAtTheLine` |
| The complete unparsed log is always reachable | `Open Build Log` opens the file itself, in the same editor, whatever the parser recognised — which is allowed to be nothing | `TheWholeUnparsedLogIsReachableWhateverTheParserMadeOfIt` |
| A tri-state CNA option is tri-state in the profile | `StudioFeatureState` is `Off`/`Auto`/`On`; the stop-gap that spelled *on* as `AUTO` is retired, and a project file written before it still builds exactly as it did | `ATriStateOptionCanSayOffAutoOrOnAndEachReachesCMakeUnchanged`, `AProjectFileWrittenBeforeTheTriStateStillBuildsTheWayItDid` |

### `CORE-02` — Hand the developer back to their IDE

**Why it is required.** Core step 11, and the clearest statement of what Studio is. The preference
already exists and has no caller, so the product currently claims an integration it does not have.

**Done when.** Commands open the project and a named source file in the editor named by
`StudioPreferences::externalEditor`, resolving source locations through
`StudioLanguageDescriptor::sourceDirectory`; an unset or unlaunchable editor is refused at the
gesture with a message that says what to configure, not after the fact.

**Not included.** Any editing of C++ inside Studio. Any awareness of what an IDE is doing.

**Done.** `CNA/Studio/Project/StudioExternalEditor.hpp` plans the command and refuses before
launching anything; `Tools > Open Project in External Editor` and
`Tools > Open Main Source in External Editor` invoke it — the first rows that menu has ever had.
A relative path resolves against `StudioLanguageDescriptor::sourceDirectory` before the project
root, which is what makes a compiler's own path work unchanged in `CORE-01`. Planning is separate
from launching so that every case below is checkable on a machine with no IDE and no display.

| Acceptance | Evidence in `StudioExternalEditorTests.cpp` |
|------------|---------------------------------------------|
| Opens the project | `OpeningTheProjectHandsOverItsDirectory` |
| Opens a named source file | `ARelativeSourceFileResolvesThroughTheLanguagesSourceDirectory`, `AnAbsolutePathIsOpenedAsGivenAndAMissingOneIsRefused` |
| Resolves through `sourceDirectory` | `TheSourceDirectoryWinsOverTheProjectRootForTheSameName` |
| Unset editor refused at the gesture, naming what to configure | `AnUnsetExternalEditorIsRefusedAtTheGestureAndSaysWhatToConfigure` |
| Unlaunchable editor refused at the gesture | `AnEditorThatCannotBeLaunchedIsRefusedBeforeAnythingIsStarted` |
| At a line, for `CORE-01` | `EachKnownEditorIsToldAboutTheLineInItsOwnSyntax`, `AnUnknownEditorStillOpensTheFileAndDoesNotClaimTheLine` |
| Reachable by a user | `TheHandOffCommandsAreRegisteredAndReachableFromTheToolsMenu` |

### `CORE-03` — The viewport shows what is selected

**Why it is required.** Core step 4. Selection works, gizmos work, and nothing in the viewport says
which entity the gizmo belongs to — which makes the one visual workflow Studio exists for guesswork
on a scene with more than a few objects.

**Done when.** The selected entity is outlined in both viewports with a visible pivot; a
multi-selection shows a combined bounds; the golden-image suite covers selected and unselected
states so a regression is caught by CI rather than by eye.

**Done, and the row was partly stale.** The outline already existed in both viewports — a box in
the 2D renderer, a thicker box in the selection colour in the 3D wireframe — which the table above
did not know. What was actually missing was everything a *multi-selection* needs: eight selected
crates were eight identical boxes with nothing saying they were one selection, and nothing at all
marking the point a rotation would happen about. So two marks were added and no more.

`SceneSelectionOverlay.hpp` computes where they go, in `cna-studio-scene`, and both viewports ask
it — which is what stops a user finding the pivot in one place in 2D and another in 3D.

| Acceptance | Evidence |
|------------|----------|
| Outlined in both viewports | Already true; `TheThreeDViewportDrawsTheSameTwoMarksAsTheTwoDOne` pins that both still mark a selection |
| A visible pivot | `OneSelectedEntityIsOutlinedAndGetsAPivotButNoSecondBox`, `ThePivotMarkLandsWhereTheGizmoTurnsUnderEitherPivotMode`, `ThePivotCrossIsTheSameSizeOnScreenAtEveryZoom` |
| A multi-selection shows a combined bounds | `AMultiSelectionGetsOneBoxRoundAllOfItAndAPivotBetweenThem`, `AnEntityWithNoBoundsIsNotOutlinedAndDoesNotWidenTheSelection` |
| Selected and unselected covered by CI rather than by eye | `CnaStudioVisualScenarioSelected` / `…Unselected` capture both states with the colour floor, and `CnaStudioSelectionChangesWhatIsOnScreen` fails if the two are byte-identical. Those run on the dependency-free build, where the viewport has no CNA renderer, so they cover the *shell's* selected state; the viewport marks themselves are checked as geometry, where a wrong position fails rather than merely a missing pixel |
| The marks are tellable apart | `TheSelectionMarksAreTellableApartFromEachOtherAndFromTheSceneAroundThem` |

### `CORE-04` — The Details panel reads as a property grid

**Why it is required.** Core step 5. The label column is a fixed fraction of the panel width, so a
property called `x` and its value sit at opposite ends of a gap wider than either. Reading a value
off the wrong row is a data error, not an aesthetic one.

**Done when.** The label column is sized from content within sane bounds, values align, nested
properties indent, a modified property is marked and can be reset; golden coverage at two panel
widths.

**Not included.** Reference fields that preview their target, multi-selection editing, and the rest
of the visual-quality campaign.

**Done, and two of the four lines were already true.** Nested properties already indented and a
modified property was already marked and resettable (`STUDIO-14012`). What was wrong was the
column: a flat 38% of the panel, defended by a comment saying a content-sized one would make
controls jump as the selection changed — which is what the bounds are for, not a reason to ignore
the content.

It is now measured from the labels on screen, clamped between a pixel floor and half the panel,
rounded to an 8-pixel step, and used a frame later than it is measured so both passes of a frame
lay out identically. The floor is in *pixels* and not in panel widths on purpose: a fractional
floor grows with the panel, which is the defect restated as a bound.

| Acceptance | Evidence |
|------------|----------|
| Sized from content | `TheLabelColumnIsSizedFromItsLabelsRatherThanFromAFractionOfThePanel` — the same entity gets the same *pixel* column in a 500-wide panel and a 1400-wide one, which a fraction cannot |
| Within sane bounds | `TheColumnStaysWithinItsBoundsOnAPanelTooNarrowAndOneTooWide` |
| Values align | `EveryValueInTheGridStartsAtTheSameX`, and `TheColumnIsSteadyOnceItHasSettledAndDoesNotDriftAsThePointerMoves` — a control that moves between the frame a user aims and the frame they press is one they cannot hit |
| Nested properties indent | `ANestedPropertyIndentsSoTheNestingReads` |
| A modified property is marked and can be reset | `AnOverriddenComponentPropertyCanBeResetToItsDefault` (already true) |
| Golden coverage at two panel widths | `CnaStudioVisualScenarioDetailsNarrow` and `…Wide` — 1280 and 2560, where the content decides at one and a bound at the other |

Six fixtures had written the old 38% down and aimed clicks at it. They ask the panel now, through
`studioDetailsControlColumnLeft`, or find the control among the rectangles the frame routed input
to — which is correct by construction where a reconstructed layout is correct until it is not.

### `CORE-05` — Find an asset, find an entity

**Why it is required.** Core steps 2 and 3. Both panels already scale to large projects and neither
can be searched, which means the scaling work only made the unfindable list longer.

**Done when.** The Content Browser has a text filter and an asset-type filter; the World Outliner
has a text filter; both filter within the existing frame budget on the 100 000-asset and
large-hierarchy fixtures the performance suite already owns.

**Not included.** Saved searches, query syntax, prefab indicators, the lock concept.

**Done, and the row was stale about the controls.** Both panels already had a search box and the
Content Browser already had a type filter; what had never been checked is the half this row is
actually about — *within the existing frame budget*. It was not.

A search leaves the folder behind and looks at the whole project, and the browser built a complete
card for every match: a label, a kind, a location and three strings, a hundred thousand times, on
the frame a user typed the first letter, twice a frame, with forty of them on screen. The ordinary
folder listing has windowed since `STUDIO-09016`; the search path had never been given the same
treatment, so the scaling work counted for nothing on the one gesture a hundred thousand assets
exist to need.

Ranking is now separated from card-building — `studioContentSearchMatches` answers *what matches*
with a pointer and an int per record — and only the slice on screen becomes a card. Counting the
results stopped building them too.

| Acceptance | Evidence |
|------------|----------|
| Content Browser text filter, within the frame budget at 100 000 assets | `SearchingAHundredThousandAssetsStillDescribesAScreenful` — a search matching every asset builds a screenful, and the narrow one finds its three |
| Content Browser asset-type filter | The same case: the filter alone, and the filter with a search, over the same project |
| World Outliner text filter, within the budget on the large hierarchy | `SearchingTwentyThousandEntitiesStillDescribesAScreenful` — including that filtering a scene nobody changed rebuilds the hierarchy index not at all |
| No filesystem cost per keystroke | The same case: `getPresenceProbeCount()` is unchanged across a search |

Removing the windowing makes the first case report the hundred thousand cards by name, which is
how the fix was checked rather than assumed.

### `CORE-06` — Paint order separable from input order

**Why it is required.** A widget that must win a click against a row has to be described before the
row, and is therefore painted under it. That has produced four defects, each fixed individually,
each guarded by a test naming one widget. Every new panel built during maintenance inherits the
trap, so this is the one structural fix worth making before feature work stops.

**Done when.** A widget can be described before a surface and drawn after it without its author
splitting the code by hand, and the per-widget guards are replaced by one structural guard that
fails on the shape rather than on a name.

**Capped at 4 hours.** This is the highest-risk item here. If it exceeds the cap, the correct
outcome is to stop, keep the existing per-widget guards, and move the row to the conditional
backlog — not to spend more.

**Done, inside the cap.** It turned out to be a small facility rather than a redesign, for the
reason the archived row guessed at: the frame already runs input and drawing as two separate
passes, and already defers popup bodies. `StudioFrame::paintOverSurface` raises a widget's
*painting* to the end of a `StudioRaisedPaintScope`, leaving its `interact` call where the router
needs it. The author writes the widget once. Nothing about the widget changes.

The tree row — where all four defects happened — now describes its disclosure triangle and its
trailing toggles in one piece each, instead of interacting at the top of the row and redrawing a
hundred and fifty lines below from fields carried down by hand. That hand-split *was* the fourth
defect: the rule was written down and the commit that wrote it broke it.

| Acceptance | Evidence |
|------------|----------|
| Described before a surface, drawn after it, without splitting the code | `StudioTreeView.cpp`: the triangle and each toggle are one block; the `isDrawPass()` re-draw block is gone |
| One structural guard, failing on the shape rather than on a name | `NothingInteractiveIsPaintedOverByASurfaceDescribedAfterIt` in `StudioVisualQualityTests.cpp` — it asks the frame for every rectangle it routed input to and knows no widget names |
| The per-widget guard it replaces | `ARowsTrailingToggleIsDrawnOverTheRowFillRatherThanUnderIt` deleted; reverting either widget to the old shape makes the structural guard name it by rectangle, which is how the deletion was checked |
| An unbalanced scope is not silent | `StudioFrame::endFrame` counts it as a phase violation |

**Two existing guards were kept, not replaced.** `TheDisclosureTriangleAndTheRowButtonsTakeAPressAheadOfTheRow`
asserts *input* precedence, which is the other half of the trap and not what the new guard checks;
and `TheAssetInspectorsHeadingIsActuallyVisibleAndNotPaintedOver` rasterises to prove that *text* —
which routes no input and so is invisible to the new guard — is on screen. Deleting either would
have removed working coverage without a replacement.

### `CORE-07` — Resize without artefacts

**Why it is required.** Every step of the Core workflow happens inside a window a user resizes.
Half of this is already done; what remains is that resizing must change what is on screen and
nothing else.

**Done when.** No stretched frame, no stale geometry, and no silent edit to the docked arrangement
across a resize; a resize case in the visual suite.

**Not included.** Device loss and render-resource recreation (`STUDIO-04010`), which needs a real
device to fail before it can be tested honestly.

**Done.** The arrangement half was already true and already tested — a split stores a fraction, a
float stores its intent and is clamped into `bounds` rather than into its own fields, and
`ShrinkingTheWindowAndGrowingItBackLeavesTheFloatsWhereTheyWere` holds it. What was left was the
stretched frame, which the archived row recorded and left: the CNA host renders the scene into an
offscreen texture *before* the shell describes the frame, so it sized that texture from the panel
rectangle as of the previous one — and on the frame a window was resized, the shell drew a texture
of the old size into a rectangle of the new.

`StudioShell::prepareLayout` is the fix: the host lays the workspace out for this frame's window
before rendering into it. It is the layout pass `renderFrame` already runs, called once more, so it
describes nothing and routes nothing.

| Acceptance | Evidence |
|------------|----------|
| No stretched frame | `TheLayoutIsAvailableBeforeTheFrameThatDrawsIt` — the rectangle the host sizes the scene from is the one the frame is about to use, and is not the stale one. Emptying `prepareLayout` makes it name all four fields |
| No stale geometry | `AResizeFillsTheNewWindowRatherThanStretchingTheOldFrame` — the frame after a resize is byte-identical to one rendered at that size from the start, over a magenta clear no pixel of which survives |
| No silent edit to the docked arrangement | `ProportionsSurviveAResize`, `ShrinkingTheWindowAndGrowingItBackLeavesTheFloatsWhereTheyWere` (already true) |
| A resize case in the visual suite | `CnaStudioVisualScenarioResizeSmall` / `…Large`, and `CnaStudioAResizeChangesWhatIsOnScreen`, which fails if two window sizes produce the same picture |

### `CORE-08` — Answer the four questions the plan left silent

**Why it is required.** Four rows survived the whole programme as unanswered questions rather than
unbuilt features. Under maintenance mode nobody will return to them, so each gets a recorded answer
now — which for at least two of them is *no work is needed, and here is why*.

**Done when.** Each of the four has a decision written where its readers will find it:

- the compatibility-shim policy for the renamed public API — aliases with a removal date, or a
  recorded decision that none exist because no external consumer does (`STUDIO-01014`);
- the state and configuration directory — Studio already writes to `cna-studio`, so what is left is
  whether a migration from the prototype's location is owed to anybody (`STUDIO-01015`);
- `ANALYSIS.md`'s retirement in favour of `docs/ARCHITECTURE.md` (`STUDIO-01016`);
- the start-up and frame-cost baseline, which the benchmark suite has since overtaken — record the
  numbers it already produces, or record that it supersedes the row (`STUDIO-00015`).

**Done.** All four are answered in [`docs/ADR-002-THE-FOUR-SILENT-QUESTIONS.md`](docs/ADR-002-THE-FOUR-SILENT-QUESTIONS.md),
and three of the four are held there by a test rather than by prose alone:

| Question | Answer | Evidence |
|----------|--------|----------|
| `STUDIO-01014` | No shims. Nothing is installed, exported, released or tagged under the old name, so no consumer is owed one; the plugin C ABI is the one versioned surface and already has `kStudioPluginApiVersion` | `LegacyEditorIdentifiersSurviveOnlyInHistoricalRecords` |
| `STUDIO-01015` | `cna-studio` stays, and nothing is migrated: the prototype was never distributed and neither of its formats is readable, so the migration would be an unexercisable branch running on every start-up | `StudioUserDirectoriesAreStudioNamedAndMigrateNothing` |
| `STUDIO-01016` | `ANALYSIS.md` is kept unedited behind its banner — ~80 source comments cite its decisions by id — and `docs/ARCHITECTURE.md` §11 restates the ones that still hold | `AnalysisMdIsRetiredAndArchitectureMdCarriesItsDecisions` |
| `STUDIO-00015` | Superseded. Headless frames are free — 120 extra cost 0 ms — so a headless frame-cost baseline measures nothing; `--ui-benchmark` gates what the row was reaching for. Start-up figures recorded once: 152 ms Release, 513 ms Debug | ADR-002 Decision 4; `StudioUiBenchmarkTests.cpp` |

### `CORE-09` — Documentation to use and maintain the product

**Why it is required.** Maintenance mode is a documentation bet: the people fixing Studio in two
years are reading, not remembering. This is the documentation the reduced product needs, and no
more.

**Done when.**

- A getting-started document takes a developer from a clone to a running game through the Core
  workflow.
- A user guide covers the eleven Core steps, and says plainly which things Studio does not do.
- `docs/ARCHITECTURE.md` matches the code, including the language seam and the scope reduction.
- The renderer and platform model is documented for contributors (`STUDIO-29006`).

**Not included.** Plugin SDK documentation, public API reference coverage, tutorials, videos.

**Done.**

| Acceptance | Where |
|------------|-------|
| Clone to a running game, through the Core workflow | [`docs/GETTING-STARTED.md`](docs/GETTING-STARTED.md) — eight steps, the last of which is building the game with Studio uninstalled, because that is the rule worth proving on your own machine rather than reading about |
| The eleven Core steps, and what Studio does not do | [`docs/USER-GUIDE.md`](docs/USER-GUIDE.md) — one section per step, and a *What Studio does not do* list that names each absence as a decision |
| `docs/ARCHITECTURE.md` matches the code | The language seam (§13) and the scope reduction (§15) were already there; §16 was added by `CORE-08`, and the invariant's own wording, which still called Studio "the professional authoring environment", now says what `ADR-001` decided it is |
| The renderer and platform model, for contributors | [`docs/RENDERERS-AND-PLATFORMS.md`](docs/RENDERERS-AND-PLATFORMS.md) — where each piece lives, what may know what, how to add or rename an identity, and which guard fails when you get it wrong. It does not repeat `ARCHITECTURE.md` §2–§4; it points at it |

Three stale claims were corrected rather than left: `cna-studio --help` described the product as a
"professional authoring environment", and the README offered a "plugin SDK" that `ADR-001` says
does not arrive.

### `CORE-10` — A regression baseline for the Core workflow

**Why it is required.** "Complete" has to mean something a test can check, or maintenance mode is
an assertion rather than a state.

**Done when.**

- One CTest case walks the whole Core workflow end to end against a real CNA build: create a
  project from a template, import an asset, edit a scene, save, reopen, build, play, kill the
  player, recover.
- Every Core subsystem has a headless seam, so the workflow above is testable without a GPU
  (`STUDIO-33020`).
- The existing sprite-animation, material and lighting behaviour is covered against regression
  (`STUDIO-21009`).
- No unfinished debug text appears anywhere in the Core workflow (`STUDIO-35010`).

### `CORE-11` — Declare Core complete and enter maintenance mode

**Why it is required.** Without an explicit transition, a finished product looks like a stalled one.

**Done when.** A version number and a release-notes convention exist; the completion checklist
below is ticked with evidence against each line; this file records the transition and the date; and
`HANDOFF.md` says that feature development has stopped and what maintenance now means.

---

## Definition of done

**CNA Studio Core is complete when, on a supported Linux host with a CNA checkout, a developer
can:**

| # | Requirement | Evidence |
|---|-------------|----------|
| 1 | Create a project from each shipped template, and reopen it | The per-template CTest case that creates, reopens, configures, compiles and runs it |
| 2 | Import and browse assets, and find one by name or type | Asset database tests; the Content Browser filter tests of `CORE-05` |
| 3 | Create, open, edit and save a scene, and undo every mutation | Scene document and undo-stability tests |
| 4 | Select and transform objects, and see what is selected | Gizmo tests; the golden-image coverage of `CORE-03` |
| 5 | Edit the object, material and light properties Studio models | Inspector, material and lighting tests; the grid coverage of `CORE-04` |
| 6 | Save without losing work, including across a crash | Atomic-write, autosave, recovery and partial-file tests |
| 7 | Play the application and stop it | The Play-mode tests that drive a real player process |
| 8 | Configure and run a clean or incremental CMake build | `CORE-01`'s tests |
| 9 | Read a compiler error, and open the file at the line | `CORE-01`'s tests |
| 10 | Recover after a crash or an interrupted session | Recovery-store tests |
| 11 | Open the project in an external IDE and keep working there | `CORE-02`'s tests |
| 12 | Trust that the above keeps working | `CORE-10`'s end-to-end case, green in CI on the dependency-free, CNA-backed and sanitizer legs |

**And the product is documented** (`CORE-09`) **and versioned** (`CORE-11`).

That is the whole definition. "Production ready", "professional", "polished" and "complete parity"
are not completion criteria here and never become them.

## Explicitly not required for Core

Named so that their absence is a decision rather than an omission. None of these blocks the
declaration of done:

- a native file-browser dialog for New and Open Project — the path field and the recent list open
  any project
- packaging, cooking or export from Studio — the project's own CMake already produces a runnable
  game, and a CTest already builds an exported project with Studio absent
- installers for Studio itself on any platform
- non-Latin text rendering, IME input, accessibility APIs and localisation
- an integrated profiler, a frame debugger, or any in-Studio performance tooling
- skeletal animation, particles, audio authoring, terrain, physics or navigation tooling
- a shader graph, or any node-based authoring surface
- project-defined C++ component reflection, generated component boilerplate, live property
  injection into a running game, or native code reload
- a plugin SDK beyond the plugin host that already exists
- Windows and macOS CI, GPU CI, and renderer-matrix coverage beyond what runs today
- the CNA Studio Visual Quality 1.0 campaign

## Maintenance mode

**On the day `CORE-11` lands, planned feature development stops.** Studio is then maintained, and
maintenance is:

- bug fixes
- compatibility fixes, including adaptation to CNA API changes
- security and correctness work, including sanitizer and warning findings
- small usability fixes inside the Core workflow
- features justified by a demonstrated, recurring need in a real maintained CNA application

**Maintenance is not** implementing an item because it is listed in
`docs/ROADMAP-BACKLOG.md`, in `docs/ROADMAP-ARCHIVE.md` or in `plans/`. Those documents describe
work that was considered, not work that is owed.

### How something legitimately becomes active work

The backlog is not a queue. An item moves from `docs/ROADMAP-BACKLOG.md` into this file only when
**all four** hold, and the case is written down in the ADR that authorises it:

1. A **real, maintained CNA application** — not a hypothetical one, not a demo written to make the
   case — hits the need repeatedly.
2. The existing code, data and external-tool workflow has become a **demonstrated bottleneck** for
   that application, with an example of the work it is costing.
3. The proposed deliverable is **bounded**: it has a completion condition, an estimate, and it does
   not oblige Studio to own a subsystem the CNA runtime should own.
4. Someone accepts the **maintenance cost** after it ships.

Each backlog entry carries its own activation condition, which is that test made concrete. Neither
"a professional editor has it" nor "it was already in the plan" is an argument, and neither is the
fact that an item has sat in the backlog for a long time.

## History, ids and traceability

- **`STUDIO-PPNNN` ids remain stable and are never reused**, including for tasks that will never be
  built. Commits, tests and code comments that cite them keep resolving, to the archived row that
  states what they meant.
- **`plans/` keeps every phase file unchanged**, including its acceptance criteria, its verification
  notes and the design reasoning on completed rows. Each carries a banner saying it is historical.
- **`docs/ROADMAP-ARCHIVE.md` is the programme roadmap as it stood**, with its 582 tasks and its
  405 completions, and its arithmetic is still checked by the test suite so that the record cannot
  rot.
- **Every one of the 176 unfinished tasks is classified exactly once** — here, in the backlog, or
  in the out-of-scope document. `ArchitectureGuardTests.cpp` fails if one is dropped, duplicated or
  invented.
- **Code comments cite tasks as `plan.md STUDIO-NNNNN`**, which is where those ids used to be
  listed. They are not rewritten: hundreds of them across the source tree would be churn for no
  correctness gain, and the id is the part that identifies the task. Read that form as *the
  archived roadmap*, and find the row in [`plans/`](plans/).
- **Nothing was renumbered.** The `CORE-*` ids are a new, separate, eleven-entry space.

## Rules this plan is held to

1. **A deliverable is done when its acceptance holds and its test passes** — not when scaffolding
   for it exists.
2. **Every new behaviour comes with a test**, at the cheapest layer that can catch the failure.
3. **Nothing working is removed without a replacement.**
4. **The repository stays buildable and green after every tranche.**
5. **Architectural invariants are enforced by test**, not by comment.
6. **Scope reduces, never grows.** A deliverable that turns out to need substantial new
   infrastructure is cut down or moved to the backlog. The budget does not move.
