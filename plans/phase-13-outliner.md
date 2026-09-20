# Phase 13 — World Outliner 2

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-13001` … `STUDIO-13999` and are never reused.

**Purpose.** Large-hierarchy editing that stays responsive and never loses a mutation.

**Exit criteria.** A scene with tens of thousands of entities browses and edits smoothly.

**Progress:** 11 of 12 complete `███████████░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-13001` | Nested entity tree with expand and collapse | ✅ | `STUDIO-07006` |
| `STUDIO-13002` | Search and filter | ✅ | `STUDIO-13001` |
| `STUDIO-13003` | Multi-selection, shift-range and Ctrl-additive | ✅ | `STUDIO-13001` |
| `STUDIO-13004` | Drag to reparent | ✅ | `STUDIO-03023` |
| `STUDIO-13005` | Visibility and lock toggles | ✅ | `STUDIO-13001` |
| `STUDIO-13006` | Rename, duplicate and delete | ✅ | `STUDIO-13001` |
| `STUDIO-13007` | Context menu | ✅ | `STUDIO-06005` |
| `STUDIO-13008` | Folder and group organisation | ⬜ | `STUDIO-13001` |
| `STUDIO-13009` | Prefab status indication | ✅ | `STUDIO-13001` |
| `STUDIO-13010` | Type icons and component warnings | ✅ | `STUDIO-13001` |
| `STUDIO-13011` | Virtualisation for large worlds | ✅ | `STUDIO-30010` |
| `STUDIO-13012` | Every mutation goes through a command | ✅ | `STUDIO-02035` |

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

**Acceptance.** Cycles are rejected; the operation is one undo entry.

**The drop path existed and moved one entity.** Dragging a row onto another reparented it, refused a
cycle and recorded one undo entry, all of it tested since `STUDIO-07058` — and the payload names a
single id, so a user who had just shift-selected forty entities (`STUDIO-13003`, landed the commit
before) and dragged one of them moved one and left thirty-nine. That is what this task was.

**A drag moves the selection it started on, and only the row otherwise.** Starting on a row that is
*not* selected moves that row alone: the user is pointing at something they have not highlighted,
and quietly taking forty others with it would be a surprise whose result is off the screen.

**Descendants of a moving entity are left out of the set**, because they are already coming. An
entity travels with its parent, and reparenting a selected child as well would tear it out of the
thing it is moving with and leave it a sibling. Selecting a parent and its child is therefore a drag
of the parent — from either row, because which one the pointer was over does not change what the
selection contains.

**One member that cannot move refuses the whole drop.** A drag that moved four of five would leave a
hierarchy the user did not ask for, cannot see the shape of, and would not obviously get back with
one Ctrl+Z. A member already directly under the target is *not* that case: it is a no-op, so it
drops out of the plan and the others still move. An entirely-no-op set is nothing to do rather than
a refusal, and the panel says different things about the two.

**The decision is a CNA-free function over the scene** — `studioOutlinerDragSet` and
`studioOutlinerReparentPlan` — so what a drop *means* can be asserted without a frame, a shell or a
drag, and only carrying it out needs the editor.

**One `CompositeCommand` for the whole drop.** A gesture the user made once is a gesture one Ctrl+Z
puts back. Each `ReparentEntityCommand` reads its entity's world transform as it is *built*, before
any of them runs, which is safe here precisely because of the two rules above: nothing in the set
moves anything else in it, since descendants were excluded and a target underneath a moving entity
is the cycle the plan refuses.

**The success is announced, not just the refusal.** The binder already warned about a refused drop,
with a note saying a successful drop onto a collapsed parent is the same picture — the row moves
inside something the user cannot see, so the tree afterwards looks like the tree before with rows
missing. It warned about one and said nothing about the other. It reports the count now, which is
also where a user finds out whether the drag took the one row they were pointing at or the twelve
they had chosen.

**Not done: dropping on the panel background to unparent.** The tree reports drops on *rows*, so
there is no drag gesture for "make this a root" — `studio.entity.detach` (`STUDIO-07058`) is the
only way, from the menu or its shortcut. Adding a background target means a drop area that exists
only when the rows do not fill the viewport, which is a gesture that works sometimes; left out
deliberately rather than overlooked, and recorded here so the next reader does not re-find it as a
defect. `studioOutlinerReparentPlan` already treats the nil parent as a real destination, so the
decision half is in place if the gesture is ever wanted.

**Verification.** `tests/StudioOutlinerPanelTests.cpp`, three cases beside the two that were already
there. `ADragTakesTheWholeSelectionWhenItStartsOnPartOfIt` pins the set: nothing selected and a row
outside the selection each give one; starting on part of it gives all of it in selection order; a
child selected with its parent is left out from either end; an entity the scene does not have is not
something to drag. `AReparentPlanRefusesTheWholeDropRatherThanMovingSomeOfIt` pins the plan: the
ordinary case, a ring refusing everything rather than moving the innocent member, a drop onto
itself, a no-op member dropping out while the rest move, an all-no-op set that is not a refusal, the
nil parent as a destination, and a target the scene no longer has.
`ADropThatMovesASelectionIsOneUndoEntry` drives a real drag through the shell and pins that two
entities move, that the count is reported, and that one Ctrl+Z puts both back and one Ctrl+Y returns
them. Checked by causing four failures — a drag set that ignores the selection, one that keeps
descendants, a plan that moves the members it can instead of refusing, and a command per entity
instead of one batch. Each fails by name.

### `STUDIO-13005` — Visibility and lock toggles

**Acceptance.** A row can hide its entity and lock it; a locked entity cannot be picked or moved in
the viewport; both go through the history.

**Visibility existed. Lock did not exist anywhere** — no field, no key, no accessor, nothing in any
picker. The eye came in with `STUDIO-35060`; this is the second button and everything behind it.

**A lock is editor state, not a runtime property.** It lives in the entity's studio state
(`Scene/SceneLock.hpp`, key `"locked"`), which the runtime scene compiler drops wholesale
(ANALYSIS.md D-07). The finished game has no notion of a thing the designer told the editor to keep
its hands off, so a field beside `enabled` would be shipping an authoring decision into the build —
and `enabled` is a real runtime property, which is why the two are not the same kind of flag however
alike the two buttons look. Absent means unlocked, so a scene written before this reads back exactly
as it was and an unlocked entity costs nothing to store. It round-trips through the scene file for
free, which is the point of putting it there, and is asserted anyway.

**A lock inherits downwards.** That is the whole reason to have one: a designer locks the finished
level geometry once, at the group, rather than forty times at the pieces. The walk goes *upwards*
from the entity and stops at the first lock — one step per ancestor, where marking a subtree would
cost the whole scene on every pick — and is bounded by the document's acyclicity invariant.

**Every way into the viewport honours it**: the 2D ray pick, the 2D band, the 3D ray pick and the 3D
band, each a separate loop that needs the rule of its own. The 2D **icon** pass needs it twice over,
because it runs last and *overrides* whatever the sprite pass found: without the check there, a
locked camera's icon would take a click the first loop had already refused — and take it from the
unlocked sprite behind it.

**And the gizmos refuse it**, on both the input and the drawing side. A lock that stopped a click
but not a drag would be half a lock, because the Outliner can still select a locked entity and that
is exactly the path a user takes to unlock it — so the selection reaching a gizmo routinely contains
things that must not move. The filter is inside `beginGizmoDrag`, not at its call sites, so the
anchor and the multi-drag agree: `selection.back()` is what the manipulator is built around, and
building it around an entity the drag then refuses to move is the same bug somewhere subtler. The
renderer drops the same entities, because a manipulator drawn over something a press cannot grab is
a control that does nothing when clicked — the user tries, nothing moves, and the reason is
invisible.

**The row's toggle became a list.** One set of fields was never going to hold the second button, and
two is where that becomes obvious. Fixed order on every row — eye, then lock — so a user learns one
column rather than hunting for whichever button a given row happens to carry. The row shows the
entity's *own* lock rather than the inherited one: a child of a locked group is out of reach in the
viewport, but a button whose click does not change what it is showing is a broken button.

#### The row's small controls could not be clicked at all

Found while wiring the lock up, and it is the larger half of this task.

`StudioInputRouter::interact` gives a press to the **first** widget described under the pointer: it
sets `active_` there and then, and every widget described afterwards at the same point returns an
empty interaction until the button comes back up. `studioTreeRows` described the row — one widget
covering the whole line — and *then* the disclosure triangle and the trailing toggle, under a
comment claiming the later of two overlapping widgets wins the click. That is the opposite of what
the router does. **Expanding a branch by clicking its triangle and hiding an entity by clicking its
eye were both dead in the editor**, in every tree in the application, and had been since each was
written.

Nothing caught it because no case had ever pressed one through a frame: the expansion tests set
`StudioTreeState` directly and the eye's test read the row's fields. Both features were verified at
the level below the one they were broken at.

The controls are described before the row now. Only the *description* moves — the drawing stays
where it was, because the row's background is painted over everything above it and a triangle drawn
early vanishes under the alternating fill. That was the reason the drawing was deferred in the first
place, and deferring the interaction with it is how the two got conflated.

This repairs `STUDIO-13001`'s expand-and-collapse and `STUDIO-35060`'s visibility toggle as a side
effect. Neither is re-opened: both were ticked for work that was done and correct, above a widget
that never delivered the press.

**Verification.** `tests/ViewportTests.cpp`: a locked entity in front loses the click to the
unlocked one behind it, a band takes only the unlocked one, a lock on the parent reaches the child
while the child's own flag stays clear, and a locked entity's icon is not a way around any of it.
`tests/SceneTests.cpp`: both 3D paths leave it alone; the command is refused when it would change
nothing and undoes back to *absent* rather than to `"locked": false`, because a scene where
everything the user ever clicked carries the key is a diff nobody can read; and a lock survives the
JSON round trip. `tests/StudioOutlinerPanelTests.cpp`: the row carries two toggles in that order
with the lock reading the entity's own flag; clicking the lock is one undo entry and does *not* also
select the row; and `TheDisclosureTriangleAndTheRowButtonsTakeAPressAheadOfTheRow` presses a
triangle through a real frame, which is the only way the defect above is visible at all.

Checked by causing five failures — the row described first again, 2D picking ignoring the lock, a
lock that does not inherit, undo writing `false` instead of removing the key, and 3D picking
ignoring the lock. Each fails by name.

### `STUDIO-13006` — Rename, duplicate and delete

**Acceptance.** All three reachable from the Outliner, all three undoable, all three meaning the
whole selection rather than one of it.

**Rename was complete**, and is the best-covered gesture in the panel: F2 raises the Outliner before
starting the edit, typing replaces the name rather than appending, Enter commits through a
`RenameEntityCommand`, Escape abandons, the same name is not a change, an empty name is refused,
clicking inside the field places the caret rather than reselecting, and clicking another row commits
the way every other field does. Eleven cases in `tests/StudioRenameTests.cpp`, all driving real
frames. Nothing to add.

**Delete was correct and half-covered.** It has taken the selection's *roots* since `STUDIO-07047` —
a delete removes the whole subtree, so a selected descendant of a selected entity is already
accounted for and asking for it separately would push a command that finds nothing — and it is one
`CompositeCommand`, because one press of Delete is one press of Ctrl+Z. Only the single-entity case
was tested, which is the one where the rule cannot fail.

**Duplicate had the same rule and did not apply it.** `DuplicateEntityCommand` copies an entity
*and its whole subtree*, and the action iterated the raw selection — so duplicating a rig together
with one of its selected bones produced a copy of the rig, with a copy of the bone inside it, **and
a second loose bone standing beside it**. Five entities where four were wanted, and the extra one
parented to nothing in particular.

That is the same defect in the same shape as `STUDIO-13004`'s drag set, found the same week and
fixed with the same helper: a selection of entities is never a list of things to act on until the
descendants have been dropped out of it, because in a hierarchy an operation on a parent is already
an operation on its children.

**Verification.** `tests/StudioShellActionTests.cpp`:
`DuplicatingAParentAndItsChildCopiesTheParentOnce` pins four entities rather than five, one selected
copy rather than two, and one undo entry; `DeleteTakesTheWholeSelectionAndTheSubtreesUnderIt` pins
that a selection of a rig, a bone inside it and an unrelated root empties the scene and comes back
whole on one Ctrl+Z. The duplicate case was written before the fix and failed by name against the
old code, which is how the defect was established rather than assumed. Checked afterwards by causing
both failures — duplicate back on the raw selection, and delete taking only the first id. Each fails
by name.

### `STUDIO-13007` — Context menu

**Acceptance.** Right-clicking a row offers what can be done to it, and doing it from there is the
same operation as doing it from anywhere else.

**The tree had reported `rightClicked` since it was written and the panel ignored it.** There was no
menu at all: every operation on an entity was reachable only from the menu bar, or from a shortcut a
user has to already know. The Content Browser has had one since `STUDIO-09009`; this is the same
shape for the Outliner.

**A right-click outside the selection takes the row; inside it, the selection is left alone.** That
resolves the tension the tree records at `StudioTreeResult::rightClicked` — a menu has to act on
something containing the row the user aimed at, or Delete removes what they were not pointing at,
and it must not throw away a multi-row selection they built on purpose. Every list does it this way
and the reason is the same in all of them.

**The rows are a CNA-free function of the document and the selection**, so what the menu *offers*
can be asserted without opening one. Rows that cannot be used are greyed rather than left out: a
menu that changes length with the selection is one where a user aiming at Delete from muscle memory
hits Duplicate. Attach is the row that turns on and off as a user works — it needs two entities —
which is exactly why dropping it would be worst. Detach needs something that is not already a root,
for the reason a command that changes nothing is refused everywhere else.

The state rows say what the click will **do** rather than what the state **is**: "Lock" on an
unlocked entity, "Show" on a hidden one. A label a user has to invert to use is one they misread
once and then distrust. They read the primary selection — the row the right-click landed on — so a
half-hidden selection ends up agreeing with the row the user aimed at rather than each entity
flipping to its own opposite.

**The dispatch is split, and the split is the rule.** Rename, Hide and Lock are document commands
about the row under the pointer, so the panel runs them: registering "Lock 'Crate'" as an
application action would put it in the command palette, where there is no pointer and nothing under
it. Delete, Duplicate, Attach and Detach already **are** application actions, each with its own
enable predicate and its own line in the log, so the panel reports the id and the binder invokes it
— a second implementation here would be a second set of rules to keep in step with the menu bar's.
Panels report; the binder acts.

**The menu is described above everything in the panel that returns early.** A menu is a widget like
any other and has to be described in both passes of every frame, so a frame that took a different
path out of `studioOutlinerPanel` — a toggle clicked, a drop landed — would have closed it.

**Verification.** `tests/StudioOutlinerPanelTests.cpp`:
`TheOutlinerMenuOffersTheRowsThatApplyAndGreysTheRestOut` pins the rows, the two that grey out and
when, the state labels flipping with the entity, an empty selection giving no menu, and the menu
never changing length. `ARightClickOutsideTheSelectionTakesTheRowAndInsideItLeavesTheSelectionAlone`
drives real right-clicks through the shell for both halves, dismissing the menu in between —
without that the second half would be clicking the open menu and passing whatever the panel did,
which is how the first draft of it passed. `ChoosingAMenuRowRunsTheCommandOrReportsTheAction` clicks
the rows themselves, one click per opening, and pins both halves of the dispatch: Lock is run here,
Delete is reported. Checked by causing two failures — a right-click that never takes the row, and a
Delete row that reports nothing. Each fails by name.

### `STUDIO-13009` — Prefab status indication

**Acceptance.** A user can tell a prefab instance from an ordinary subtree, and can see where it
ends.

**The links were on the entities and the Outliner showed none of it.** `kPrefabAsset` sits on the
instance root and `kPrefabEntity` on every entity of the instance, both in studio state. An editor
where a user cannot tell the two apart is one where they edit the instance expecting the prefab to
change, or edit around it expecting it not to.

**The accent runs through the whole instance, not only its root.** The question a user actually has
is "how far does this go" — a mark on the root alone answers "is this one", which they can usually
guess from the name. Marking every entity of the instance is also simply true: all of it came from
the prefab.

**The type icon is kept and only its colour changes.** A prefab instance holding a mesh is still a
mesh, and the icon is the thing the eye scans the column for; replacing it would trade the fact a
user scans for against one they can read in the detail column anyway.

**Only the root names it**, because only the root carries the asset link — and the component count
is the less useful of the two facts about an entity that came out of a prefab.

**Verification.** `tests/StudioOutlinerPanelTests.cpp`:
`TheOutlinerMarksAPrefabInstanceAndItsWholeExtent` pins the accent on the root *and* on a member,
its absence on a sibling that is no part of the instance and on an unrelated root, "Prefab" on the
root alone, and the type icon unchanged — compared against a row carrying the same components
rather than against a named glyph, so the assertion says "unchanged" rather than naming whatever
the fixture happens to get. Checked by marking only the root: the member's assertion fails by name.

### `STUDIO-13010` — Type icons and component warnings

**Acceptance.** A row says what its entity *is*, and says when something about it is wrong.

**The icons existed; the warnings did not.** `iconFor` has mapped components to glyphs since
`STUDIO-35030` — camera, light, mesh, sprite, audio, and a plain entity for a transform and nothing
else, because a blank where every other row has a picture reads as a row that failed to load. The
other half of the task was missing entirely: the only way to learn an entity was broken was to open
the Problems panel and read a list.

**The count, not the message.** The message belongs to the Problems panel, which has the width for
it and a row per issue. This column's job is to make the user go and look.

**The worst severity wins**, so one error among four warnings reads as an error. A row that reported
the *last* issue found would change colour when an unrelated rule was added to the validator — and
the case that pins this puts a warning **after** the error on purpose, because with the error last
"the worst wins" and "the last one wins" give the same answer, and the first draft of the gate
passed over either rule.

**A problem outranks the prefab mark.** "This came from a prefab" is useful and "this does not work"
is urgent, and a row can say only one thing in the colour a user scans for.

**Scene-wide issues are dropped**, because there is no row to put them on: hanging "two primary
cameras" on one of the two would name a culprit the rule does not have.

**The panel is handed the issues rather than computing them, and that is the whole of the cost
story.** `validateScene` walks the entire document and this panel is virtualised for fifty thousand
entities (`STUDIO-13011`); validating per frame would put back the exact cost that task removed. The
shell recomputes when the command history's **cursor** moves, which under D-06 — every document
mutation is a command — is an *exact* "has the scene changed" signal rather than a heuristic. Undo
and redo move it too, which is right: they change the scene as much as anything else does.

**`StudioOutlinerResult::rowsMarked`** reports how many of the built rows carry a problem, and the
shell copies it into `counts()` beside the other panel numbers. Without it, "the Outliner is marking
things" is invisible from outside the panel, and a marking that silently stopped working would look
exactly like a clean scene.

**Verification.** `tests/StudioOutlinerPanelTests.cpp`:
`TheOutlinerMarksTheEntitiesThatHaveAProblem` pins the grouping (scene-wide issues dropped, counts
per entity, worst-wins with the warning last), both severities on the row in both columns, an
untouched row for an entity with nothing wrong, the problem outranking the prefab mark, and — with
no issues supplied — rows exactly as they were, which is what keeps every existing caller and the
headless paths meaning what they meant.
`TheShellRevalidatesTheOutlinerWhenTheSceneChangesAndNotEveryFrame` drives the real binder: a
component added through a command makes the mark appear, and undoing it clears it. Its subject
carries a transform **and** a camera, because a transform alone is an "empty entity" the validator
already reports — a subject that started marked could not show the mark appearing, which is how the
first draft of it failed. Checked by causing two failures — the last issue winning instead of the
worst, and a cache that never refreshes after the first frame. Each fails by name.

### `STUDIO-13012` — Every mutation goes through a command

**Acceptance.** No panel writes to the document directly, and something other than review keeps it
that way.

**It was already true, and that is exactly when a guard is worth adding.** Nothing in
`src/shell-panels`, `src/ui-core` or `src/viewport` touches `SceneDocument`'s mutating API — the
audit that opened this task found no violation to fix. A rule with no violations is cheap to
enforce and expensive to restore once it has one, and this is the rule the undo stack rests on: a
panel that wrote to the document directly would make one change Ctrl+Z cannot reach, and a user who
finds *one* such change stops trusting undo for all of them.

**A closed allow-list, every entry carrying its reason.** Eight files may call the mutators, and
each says why: the document's own implementation, the three files where the commands live,
`CreatePrefabCommand` and its undo, the default camera of a brand-new scene (before there is a
document to undo into), the benchmark's scenario setup (routing it through the history would be
measuring the history), and the player's own runtime scene, which is not the document being edited.
Adding to that list is a decision somebody makes in review rather than a pattern that quietly stops
complaining.

**The list is checked against reality in both directions.** A file that mutates and is not on it
fails; an entry whose file no longer mutates anything also fails, because an exemption nobody needs
is how a list stops being read.

**`clear()` is deliberately not scanned for.** It is too common a method name to match textually,
and a document cleared outside a command is a whole-file operation — opening or closing a scene —
rather than an edit, which is not what undo is for. Said here rather than left as a gap somebody
finds later and mistakes for an oversight.

**Verification.** `tests/ArchitectureGuardTests.cpp`: `OnlyCommandsChangeTheEditedScene`. Checked by
causing both failures — a `findEntityForEdit` planted in the Outliner's menu dispatch, which the
guard reported with its file, line and the rule; and a stale entry added to the allow-list, which
the second assertion caught. Each fails by name.

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

