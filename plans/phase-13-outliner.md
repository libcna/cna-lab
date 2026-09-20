# Phase 13 — World Outliner 2

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-13001` … `STUDIO-13999` and are never reused.

**Purpose.** Large-hierarchy editing that stays responsive and never loses a mutation.

**Exit criteria.** A scene with tens of thousands of entities browses and edits smoothly.

**Progress:** 4 of 12 complete `████░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-13001` | Nested entity tree with expand and collapse | ✅ | `STUDIO-07006` |
| `STUDIO-13002` | Search and filter | ✅ | `STUDIO-13001` |
| `STUDIO-13003` | Multi-selection, shift-range and Ctrl-additive | ✅ | `STUDIO-13001` |
| `STUDIO-13004` | Drag to reparent | ⬜ | `STUDIO-03023` |
| `STUDIO-13005` | Visibility and lock toggles | ⬜ | `STUDIO-13001` |
| `STUDIO-13006` | Rename, duplicate and delete | ⬜ | `STUDIO-13001` |
| `STUDIO-13007` | Context menu | ⬜ | `STUDIO-06005` |
| `STUDIO-13008` | Folder and group organisation | ⬜ | `STUDIO-13001` |
| `STUDIO-13009` | Prefab status indication | ⬜ | `STUDIO-13001` |
| `STUDIO-13010` | Type icons and component warnings | ⬜ | `STUDIO-13001` |
| `STUDIO-13011` | Virtualisation for large worlds | ✅ | `STUDIO-30010` |
| `STUDIO-13012` | Every mutation goes through a command | ⬜ | `STUDIO-02035` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-13001` — Nested entity tree with expand and collapse

**Acceptance.** The scene's hierarchy is shown as a tree, parents before their children, and a
branch can be opened and closed.

**Already built, and ticked here after checking rather than after writing.** The panel flattens the
scene through `StudioTreeState`, which holds the *collapsed* set rather than the expanded one — so
the default is open, and a tree that started entirely closed would show a user one line and make
them work to discover their scene has anything in it. `STUDIO-13011` later made the walk count
without building, so a scene of fifty thousand entities costs fifty thousand increments rather than
fifty thousand rows of three strings, twice a frame.

**Verification, already present.** `tests/StudioOutlinerPanelTests.cpp`: the hierarchy comes out
parents before children; collapsing a row hides its children and nothing else; a tree starts open; a
deep scene is drawn rather than descended; and the two empties — "no project" and "empty scene" —
are said apart.

### `STUDIO-13002` — Search and filter

**Acceptance.** A user can find an entity by name in a large scene, and what they find makes sense
in the hierarchy it came from.

**Filtering a tree is not filtering a list**, and that is the whole of the task. A row that matches
is useless without the ancestors above it: "Weapon" on its own says nothing about which of four
Players it belongs to, and a tree that showed matches at depth zero would be inventing a hierarchy
the document does not have. So the kept set is the matches **and every ancestor of a match**, and
the depth a row is drawn at is the scene's.

**The two sets are kept apart**, so a row can be drawn as what it is — the thing searched for, or
the way to it. A path row is `muted`: dimmed and still fully clickable, which is exactly what that
flag exists for. A path the user cannot click is one they have to clear the search to walk.

**Matches first, then the paths, walked upwards.** Whether to keep a node depends on what is
*underneath* it, which a descent cannot know until it has come back up; walking up from each match
is one step per ancestor and visits nothing that is not on a path. The ancestor walk stops the
moment it reaches a node already kept, so a hundred matches under one root reach that root once.

**A search opens the branches it needs, and does not keep them open.** A match hidden inside a
closed branch is a match the search did not find, as far as anybody looking at the screen can tell.
The collapsed set is *ignored* while filtering rather than rewritten, so clearing the box leaves the
tree exactly as the user left it — a search that silently expanded forty branches and left them that
way would be a search with an undo cost.

**Empty text is an inactive filter, not one nothing fails.** The two are the same picture and
different costs, and only one of them walks the scene. It is also what keeps every existing caller —
and the headless paths — meaning what they meant, since the parameter is defaulted.

**A search matching nothing says so in its own words.** It looks exactly like an empty scene
otherwise, and a user who cannot tell them apart starts wondering where their level went rather than
clearing the box.

**Verification.** `tests/StudioOutlinerPanelTests.cpp`: the kept set is the match and its parent and
neither the sibling nor the other root; the two sets are distinct; the rows come out as the path
then the match, the path muted and still enabled, at the scene's own depth; the search is
case-insensitive and a substring; nothing matching keeps nothing rather than falling back to
everything; and a match inside a branch the user had closed is reached without the collapse being
thrown away. Checked by causing both: a filter that keeps matches without their ancestors, and one
that honours the collapsed set. Each fails by name.

### `STUDIO-13003` — Multi-selection, shift-range and Ctrl-additive

**Acceptance.** Ctrl-click adds one row to the selection; Shift-click takes every row between the
last plain click and this one; both go through the context, and both mean what they mean in every
other list the user has ever used.

**Shift was a second spelling of Control, and that was the defect.** `StudioTreeView` reported one
flag — `result.additive = control || shift` — so the tree could say *that a modifier was held* but
not *which*. The gesture every file manager and every list box uses for "that many" picked up a
single row, and a user building a selection of forty had to click forty times. The two are
different questions: one asks for one more, the other for everything between here and the place the
user last clicked. They are reported as `additive` and `rangeSelect` now, and the panel decides.

**A range is over what is shown, not over what exists.** "Between" is a statement about the screen:
a user looking at a closed branch cannot have meant the rows inside it, and a user with a search
active cannot have meant the rows the search hid. So `studioOutlinerRange` flattens the tree through
the same `walkRoots` the panel draws with, honouring both the collapsed set and the filter, and
takes the run between the two ends of *that*. A range function with its own idea of the display
order would disagree with the panel the first time either changed.

**The ids come out of the traversal that builds the rows.** `walk` and `walkRoots` gained an
optional `std::vector<Uuid>*` sink beside the row output, so one descent produces both. A separate
ids-only mode would be a second set of rules — the collapse, the filter, the sibling order — to keep
in step with the first, for the sake of a gesture a user makes a few times a minute.

**Either end off the screen is no range rather than a guess.** An anchor inside a branch the user
has since closed, or an end the search filtered away, returns empty and the click falls through to a
plain selection. Guessing which rows they *did* mean would silently select things they cannot see,
which is worse than the gesture doing nothing.

**The anchor does not move on a Shift-click.** Extending a run by shift-clicking further down means
"from the same place, further"; an anchor that moved to the far end would make the second
shift-click measure from there and *shorten* the selection. Control does move it, which is what
makes Ctrl+Shift add a second run to the first.

**Either end order is the same range.** A user drags a selection upwards as often as downwards, and
`from` is simply where the anchor happens to be.

**Verification.** `tests/StudioOutlinerPanelTests.cpp`, in two cases that gate the two halves.
`AShiftRangeTakesEverythingBetweenTheAnchorAndTheClick` pins what a range *is*: the whole tree
between the two roots; a run crossing a hierarchy boundary; either end order; one row as a range of
one; a closed branch excluded and an end inside it refused; a search narrowing what "between" means;
an unknown id refused. It also asserts the display order it depends on, because siblings sort by
name rather than by when they were added, and a reader who assumed insertion order would mis-read
every expectation under it.

`ShiftClickingTakesTheRunAndControlClickingTakesOneMoreRow` drives real clicks with real modifiers
through the shell and pins that a user pressing Shift *gets* one. The two are separate gates on
purpose, and the reason is the defect itself: with Shift conflated with Control, a range function
that worked perfectly was never called, and every assertion in the first case passed over a dead
feature. Checked by causing all three failures — Shift reported as Control again, a range that
ignores the collapse and the filter, and an anchor that moves on a Shift-click. Each fails by name.

### `STUDIO-13004` — Drag to reparent

**Acceptance.** Cycles are rejected; the operation is one undo entry

### `STUDIO-13011` — Virtualisation for large worlds

**Verification.** Stress test at 10,000+ entities with deep nesting

**Done.** The World Outliner builds the rows it shows and counts the rest, and
`tests/StudioLargeProjectTests.cpp` holds it there at twenty thousand entities in chains fifty
deep.

**It had looked virtualised since `STUDIO-03034`, and was not.** The tree widget has culled its
drawing from the first day, so `rowsDrawn` was already a screenful and every draw-call assertion
passed. What the widget cannot bound is the model the panel hands it: `studioOutlinerRows` flattened
the whole scene into `StudioTreeRow`s — three strings apiece — twice a frame, to show forty. This is
the same defect the Content Browser's list view had (`STUDIO-09016`), found the same way, and the
reason `StudioOutlinerResult` now reports `rowsBuilt` beside `rowsDrawn`: a panel that builds fifty
thousand rows and draws forty has a perfectly bounded `rowsDrawn`.

**One function counts and builds.** `walk` takes a running index, a window and an optional output
vector; counting is the same traversal with nowhere to put the rows. Two functions agreeing by
inspection is exactly how a scrollbar and its rows end up quietly out of step, and the symptom —
the last entity in a large scene being unreachable — is one nobody reports, because nobody can tell
it is missing. `TheOutlinersWindowIsTheSameRowsTheWholeListWouldHaveHadThere` asserts slice equality
at the start, the middle, across a chain boundary and running off the end.

**A hierarchy has no index to seek into**, so reaching row *n* still costs *n* steps of the walk.
That is not the same as costing *n* rows: a step is two hash lookups and an increment, where a row
is three string allocations. And the walk no longer formats an id per node when nothing is
collapsed — which is every tree until the user closes something — because `collapsedCount()` is
cheaper to ask than a thirty-six-character string is to build.

**Measured** (`--ui-benchmark=outliner`, Release, 120 frames at 1920×1080, median µs/frame):

| scenario | before | after |
|---|---:|---:|
| `outliner-2000` | 2587 | 660 |
| `outliner-scrolling` | 2384 | 779 |
| `outliner-20000` | 30 490 | 8101 |
| `outliner-20000-deep` | 37 934 | 20 674 |
| `outliner-20000-scrolling` | 44 721 | 20 798 |

**What is left is not virtualisation, and it is named.** Twenty thousand entities still cost about
20 ms a frame, and the remaining cost is `SceneDocument::getChildrenByParent()`: a pass over every
entity building a map of child vectors, once per drawing pass. It is uncached deliberately —
`findEntity` hands out a mutable entity and `setParentId` is public, so a cached hierarchy would go
stale silently, and a stale hierarchy index presents as entities vanishing from the outliner. The
panel already derives it once and shares it between the count and the window; making it not happen
at all is `STUDIO-30011`, which is about exactly this and is where it belongs. `STUDIO-30020` is the
benchmark that says so with a number.

