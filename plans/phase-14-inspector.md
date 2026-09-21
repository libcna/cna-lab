# Phase 14 — Details Inspector 2

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-14001` … `STUDIO-14999` and are never reused.

**Purpose.** A first-class production property editor driven by the descriptor model.

**Exit criteria.** Every property type a component can declare is editable, validated and undoable.

**Progress:** 2 of 18 complete `█░░░░░░░░░░░`

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
| `STUDIO-14012` | Reset to default | ⬜ | `STUDIO-14001` |
| `STUDIO-14013` | Revert and apply prefab overrides | ⬜ | `STUDIO-14012` |
| `STUDIO-14014` | Copy and paste property values | ⬜ | `STUDIO-03025` |
| `STUDIO-14015` | Validation warnings shown inline | ⬜ | `STUDIO-14001` |
| `STUDIO-14016` | Tooltips and documentation from descriptor metadata | ⬜ | `STUDIO-03021` |
| `STUDIO-14017` | Multi-selection editing where the semantics are unambiguous | ⬜ | `STUDIO-14001` |
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

