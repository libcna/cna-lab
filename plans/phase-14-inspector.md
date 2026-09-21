# Phase 14 — Details Inspector 2

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-14001` … `STUDIO-14999` and are never reused.

**Purpose.** A first-class production property editor driven by the descriptor model.

**Exit criteria.** Every property type a component can declare is editable, validated and undoable.

**Progress:** 6 of 18 complete `████░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-14001` | Component sections with collapse and expand | ✅ | `STUDIO-07007` |
| `STUDIO-14002` | Add and remove component | ✅ | `STUDIO-14001` |
| `STUDIO-14003` | Numeric fields: typed entry and drag-to-change | ⬜ | `STUDIO-14001` |
| `STUDIO-14004` | Vector and rotation editors | ⬜ | `STUDIO-14003` |
| `STUDIO-14005` | Colour editor | ⬜ | `STUDIO-14001` |
| `STUDIO-14006` | Enum, boolean and string editors | ⬜ | `STUDIO-14001` |
| `STUDIO-14007` | Asset reference field with drag-and-drop and a picker | ⬜ | `STUDIO-09008` |
| `STUDIO-14008` | Entity reference field | ⬜ | `STUDIO-14007` |
| `STUDIO-14009` | List properties: add, remove, reorder — each its own undo entry | ⬜ | `STUDIO-14001` |
| `STUDIO-14010` | Nested structure editing | ⬜ | `STUDIO-14009` |
| `STUDIO-14011` | Read-only data display | ⬜ | `STUDIO-14001` |
| `STUDIO-14012` | Reset to default | ✅ | `STUDIO-14001` |
| `STUDIO-14013` | Revert and apply prefab overrides | ⬜ | `STUDIO-14012` |
| `STUDIO-14014` | Copy and paste property values | ✅ | `STUDIO-03025` |
| `STUDIO-14015` | Validation warnings shown inline | ✅ | `STUDIO-14001` |
| `STUDIO-14016` | Tooltips and documentation from descriptor metadata | ⬜ | `STUDIO-03021` |
| `STUDIO-14017` | Multi-selection editing where the semantics are unambiguous | ✅ | `STUDIO-14001` |
| `STUDIO-14018` | Responsive with very large property counts | ⬜ | `STUDIO-30010` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-14002` — Add and remove component

**Acceptance.** A component can be chosen and added, and taken off again, both through the history.

**Mostly built, under `STUDIO-07040`** — the gap that stopped Dear ImGui being deleted. Add is a
dropdown of every addable type, grouped by category, with an Add button beside it; Remove is a
button on each component's own header, asked of the command rather than decided in the panel so a
descriptor that marks a component required is honoured in one place. Both go through the history,
both are tested, and the required-component refusal is tested too.

**The list itself was not gated**, and that is the half that makes the feature a feature: an
Inspector that could only ever add the first entry of the registry would be one a user works around
rather than uses. The list is now `studioAddComponentChoices`, a CNA-free function of the registry
and the entity, so what is offered can be asserted without a frame, a dropdown or a popup — the
widget shows it and the button adds what it names, which is wiring. The choice is carried as a
*type id* rather than an index, because the list shortens the moment a unique component is added and
a remembered index would then point at a different type.

**A first attempt at gating it drove the dropdown through the shell and passed against an
implementation that ignored the choice entirely.** The list shortens as unique components are added,
so the entry at index zero changes on its own, and "a different component arrived" was true either
way. The test was removed rather than patched: it was asserting a coincidence. Recorded here and in
the replacement's comment, because that shape of false pass is easy to write again.

**Verification.** `tests/StudioDetailsPanelTests.cpp`:
`AComponentCanBeAddedToTheSelectedEntityAndUndone` and
`AComponentIsRemovedFromItsOwnHeaderAndUndone` were already there.
`TheAddComponentListLeavesOutWhatTheEntityAlreadyHas` pins the list: a unique component the entity
carries is not offered, every entry names a registered type and carries the id it will add, the
label is the category and the display name with the fallback when there is no category, and an
entity with nothing on it is offered exactly one more. Checked by causing the unique-exclusion to
stop happening; it fails by name.

### `STUDIO-14014` — Copy and paste property values

**Acceptance.** A property's value can be taken to the clipboard and put into another, and a paste
of the wrong thing is refused rather than coerced.

**Nothing existed.** The clipboard seam has been there since `STUDIO-03025` and the text fields use
it for their own text; no property value had ever been on it.

**The clipboard text is the same JSON a scene file holds.** That is already the one written-down
definition of what a `PropertyValue` looks like, and it is the one that has to survive a release — a
second encoding invented for the clipboard would be a second thing to keep in step with the first.
Compact rather than pretty, because a clipboard is a line in a chat window as often as a paste into
an editor, and readable on purpose: a user who copies a position into a bug report should get
something they can read, and one who pastes a readable thing back should be understood.

**The round trip is the whole of the check, and it has to be.** `PropertyValue::fromJson` is
forgiving by design — a scene file holding a number where a vector belongs should load rather than
refuse, so it hands back the type's own zero — which means a colour pasted into a float would arrive
as a perfectly valid *nothing*. Comparing the result's type against the expected one would be a
check that can never fire, because `fromJson` always answers in the type it was asked for; a first
version had exactly that line, and breaking it changed no test. It was removed rather than left
looking like a safeguard.

**The menu is on the label, not the row.** The row is mostly editor, and a target covering it would
take the press before the fields inside it got one — the router gives a press to the first widget
described under the pointer, which is the defect the World Outliner's rows had (`STUDIO-13005`). The
label is the one part of a property row that is not already a control.

**A menu rather than more buttons.** Reset earns its place in the label column because its
*presence* means something; three ghost buttons on every row would say nothing and cost the width
that makes labels readable. Reset is in the menu as well, so a user who learns one surface has both.

**Paste is greyed when the clipboard holds nothing that fits**, which is the same rule every other
menu in the editor follows: a row that is absent tells the user less than one that is present and
unavailable.

**Verification.** `tests/StudioDetailsPanelTests.cpp`:
`APropertyValueSurvivesBeingCopiedAndPastedBack` round-trips nine property types and pins the text
as single-line and readable. `PastingSomethingThatIsNotThePropertysKindIsRefused` pins prose, empty
text, malformed JSON, a valid value of the wrong kind, a number where a vector belongs, and a
property with no declared type. `APropertyValueIsCopiedAndPastedThroughTheRowsMenu` drives the real
menu through the shell, copies, changes the value underneath, pastes it back and undoes that. It
re-seeds the clipboard before each paste attempt, because the sweep passes over Copy on its way to
Paste — without that it would copy the *new* value and then paste it, and pass while proving
nothing. Checked by causing the round-trip check to stop happening; it fails by name.

### `STUDIO-14015` — Validation warnings shown inline

**Acceptance.** What is wrong with a component is said on the component, where the fix is made.

**The Details panel showed no validation at all.** The only way to learn an entity was broken was to
open the Problems panel and find it in a list — a panel away from the one carrying the control that
fixes it.

**Above the properties, not below.** A message under forty rows of properties is one the user
scrolls past on their way to the thing it is about.

**The message, not a count.** The Outliner's row says how *many* because it has a column
(`STUDIO-13010`); this panel has the width to say *what*, and "what" is the thing that tells a user
which control to reach for.

**An issue naming no component belongs to the entity**, not to whichever section happens to be
first, and scene-wide issues are left out entirely: hanging "two primary cameras" on whichever
camera happens to be selected would name a culprit the rule does not have.

**One walk feeds both panels.** The shell already revalidated on the command history's cursor for
the Outliner's marks; the Inspector reads the same result. Two walks would be two answers that can
disagree, which is the worst way for a user to learn their scene is broken — and validating per
frame would make every panel showing an entity pay for every entity in the scene
(`STUDIO-13011`'s budget).

**Counted in the pre-pass and drawn in the same words.** The scroll view is sized from the measure,
and the two are separately load-bearing — the same pair of `continue`s `STUDIO-14001` had to gate
twice, and gated twice again here.

**Verification.** `tests/StudioDetailsPanelTests.cpp`:
`AnIssueIsPickedApartByTheEntityAndTheComponentItNames` pins the split — this entity's issues and
not another's, the component's and not the entity's, the scene-wide one dropped, and an entity
nothing names showing nothing.
`TheInspectorDrawsAComponentsIssuesAboveItsProperties` measures a clean panel first and pins two
extra rows drawn *and* measured, and that an issue naming a component the entity does not carry is
nobody's row. Checked by causing both failures — the measure not counting them, and the draw not
drawing them. Each fails by name.

### `STUDIO-14017` — Multi-selection editing where the semantics are unambiguous

**Acceptance.** An edit made with several entities selected reaches all of them, as one undo entry,
and is only offered where it means one thing.

**The Inspector showed the last selected entity and edited only it**, saying nothing about the rest.
A user who selected five crates and set their scale changed one and found out later.

**The components shown are the intersection, not the union.** A component only some of them carry is
exactly the case the task's own title excludes: there is no unambiguous answer to what editing it
should do, and picking one silently is how a user loses work they did not know they were doing. In
the last selected entity's order, because that is the entity the panel is built around and a list
that reordered itself as the selection grew would be one a user cannot learn. A selection of one is
that entity's own components, which is what makes this the only path rather than a second one for
the multi case.

**A value is only shared when they agree**, through the descriptor's default — an entity that never
wrote the property and one that wrote the default agree, because they do as far as the game is
concerned, and reporting a difference nothing can see would be worse than useless.

**One edit, one undo entry, for the whole selection.** A command per entity would be five presses of
Ctrl+Z to undo one keystroke and, worse, would undo them one at a time, leaving the scene in
arrangements that never existed — the same bargain `TransformEntitiesCommand` strikes for a gizmo.

#### `CompositeCommand` gained a merge key, and the scrub case gained the assertion that needed it

Wrapping the edit in a batch quietly removed the fold a scrub depends on (`STUDIO-07055`): a
`CompositeCommand` had no merge key, so every frame of a drag would have been its own undo entry.
**Nothing caught it.** `DraggingANumericFieldScrubsTheValueWithoutTypingIntoIt` pinned "something is
undoable" and one `undo()`, which passes just as well for forty entries.

Adding the count assertion was not enough either: the harness delivers a drag in *one frame* on
purpose, so it produces a single command and an implementation with no merge key at all passes.
The case now drags over eight frames — and against that, removing the key gives twelve entries where
there should be one. A merge is not testable by a gesture that never needs one.

The composite's merge is all-or-nothing and checked before anything changes: a batch half-merged
with the one after it is an undo entry that reverses some of a gesture and not the rest, which is
worse than an entry per frame because it looks like it worked.

**Not done: a "multiple values" indicator.** Where the entities disagree the editor still shows the
primary's value. `studioSharedPropertyValue` answers the question and is tested; what is missing is
a mixed state in the editors themselves, which is a change to every property editor rather than to
the panel. Recorded rather than left for the next reader to find as a defect.

**Verification.** `tests/StudioDetailsPanelTests.cpp`:
`TheInspectorShowsOnlyTheComponentsEveryoneSelectedHas` pins the intersection, the order, the
single-entity case, an empty selection and a stale id.
`ASharedPropertyValueIsNothingWhenTheEntitiesDisagree` pins agreement, disagreement, the
unwritten-versus-explicit-default case and an entity without the component at all.
`EditingAPropertyOverASelectionChangesAllOfThemInOneStep` drives a real scrub over three entities
and pins that all three move and one Ctrl+Z takes all three back. Checked by causing three failures
— the edit reaching only the primary, the component list taking the union, and the value ignoring
disagreement — plus the merge-key break above. Each fails by name.

### `STUDIO-14012` — Reset to default

**Acceptance.** A property that differs from what its descriptor declares can be put back, through
the history.

**Reset existed only for importer settings**, in the asset inspector. A *component* property had no
way back at all: a user who scrubbed a scale and wanted 1 again had to know the default and retype
it, and for a colour or a quaternion they would have had to guess.

**Offered only where it would do something.** A column of Reset buttons dead on every untouched row
is a column of noise — and the button being present is also the only indication the panel gives that
a property has been changed from what the component was born with, which it could not be if it were
always there. A component the registry does not know has no default to go back to, and a read-only
property is not an override, so neither gets one.

**It comes off the label column, never the control's.** Narrowing the control would move every field
inside it the moment a property became overridden — a user typing into the first of three angle
boxes would find the boxes slide out from under the pointer as soon as that first one committed.
That is not hypothetical: the first version took the space from the control and broke
`EachTypedAngleFieldCommitsThroughTheHistoryAsItsOwnEntry`, which types into the second and third
fields at coordinates derived from the row. The label is truncated text and already shortens for a
hundred other reasons.

**Taken before the label is drawn**, or the button lands on top of the text.

**Through the history, like every other edit**, and never merged into a scrub in flight: a reset is
still a change, and a user who clicks it by mistake needs the same way out as one who typed by
mistake. Reported as `propertiesReset` rather than folded into `edited`, because a reset is the one
edit a user makes to undo their own work and a shell that could not tell them apart would say
"edited" about a click that put something back.

**Verification.** `tests/StudioDetailsPanelTests.cpp`:
`AnOverriddenComponentPropertyCanBeResetToItsDefault` sweeps the label column of a panel whose
position has been moved, finds the button, and pins the value going back to the descriptor's default
and one Ctrl+Z bringing the edit back. `APropertyThatMatchesItsDefaultOffersNoReset` runs the same
sweep over an entity built with `applyDefaults` and pins that nothing resets and nothing is edited
by trying. Checked by causing both failures — the button offered whether or not the value differs,
and the button doing nothing when pressed. Each fails by name.

### `STUDIO-14001` — Component sections with collapse and expand

**Acceptance.** A component's properties can be folded away and brought back, and the fold survives
the frame.

**Nothing folded and there was nowhere to remember one.** Every component was drawn fully expanded
with a header carrying a name and a Remove button and no disclosure at all, so an entity with eight
components was a wall a user had to scroll past to reach the ninth. The panel took no caller-owned
state of any kind.

**The state is caller-owned**, like the World Outliner's `StudioTreeState` and for the same reason:
the panel is rebuilt from scratch every frame, so anything it must remember between frames belongs
to whoever outlives one.

**It holds the *collapsed* set**, so the default is open. A panel that started every section closed
would show a user a column of headings and make them work to discover their entity has anything on
it — the same bargain the tree strikes.

**Keyed by component *type*, not by entity.** A user who closes Transform means "I am not working on
transforms", not "not on this one's", so it stays closed as they click through a scene — which is
the only way the gesture saves them anything. Two components of the same type on one entity
therefore close together: rare, visible, and a better trade than a fold that springs open on every
selection change.

**A null state means every section is open**, which is what the panel did before folding existed.
That is what keeps the headless paths and the many cases that care about a property rather than
about folding meaning what they meant, and it is asserted rather than assumed.

**The triangle is its own widget, not the whole header.** A header that toggled on any click would
close a section every time a user aimed at Remove and missed.

**Two `continue`s over one predicate, and each needs its own gate.** The row-count pre-pass and the
draw skip a closed section in the same words, and they are separately load-bearing: the scroll view
is sized from the measure, so a pre-pass that kept counting rows the draw had stopped drawing would
let the panel scroll past its own last control — and nothing about the drawn rows would show it.
`StudioDetailsResult::rowsMeasured` is reported for exactly that reason. The two counts are *not*
equal by design (the measure reserves the optional sections at their maximum, because it runs before
the comparisons that decide whether they appear), so what is pinned is that folding moves both by
the same amount.

**Verification.** `tests/StudioDetailsPanelTests.cpp`:
`FoldingAComponentSectionHidesItsPropertiesAndKeepsItsHeading` clicks the triangle through a real
frame — swept, so a spacing change cannot turn it into a case that clicks empty space — and pins the
heading surviving, the properties going, the measure moving with the draw, and the section reopening
to exactly what it was. `APanelWithNoFoldStateDrawsEverySectionOpen` pins the defaulted behaviour
against the folding one. Checked by causing both failures — the draw ignoring the fold, and the
pre-pass ignoring it. The first attempt at the draw break silently edited the pre-pass instead,
because the two lines are identical; that is how the second gate came to be written.


### `STUDIO-14004` — Vector and rotation editors

**Acceptance.** Rotation is edited as Euler angles that round-trip through the stored quaternion without drift

### `STUDIO-14017` — Multi-selection editing where the semantics are unambiguous

**Acceptance.** Mixed values are shown as mixed, not as the first value

