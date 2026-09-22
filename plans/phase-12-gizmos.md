# Phase 12 — Selection and gizmos 2

> **ARCHIVED — historical record. This file authorises no work.** It belongs to the
> [archived programme roadmap](../docs/ROADMAP-ARCHIVE.md), whose scope was retired on 2026-09-22
> by [ADR-001](../docs/ADR-001-SCOPE-REDUCTION.md). **A ⬜ below means *not built*. It no longer
> means *planned*.** The one authoritative active roadmap is [`plan.md`](../plan.md).
>
> **Disposition of this phase:** Complete.
>
> Ids in this phase are `STUDIO-12001` … `STUDIO-12999` and are never reused. Every id here still resolves, so a commit, test or code comment that cites
> one keeps its meaning.

**Purpose.** Production-quality transform manipulation.

**Exit criteria.** Transforming objects feels precise and predictable, and every drag is exactly one undo entry.

**Progress:** 11 of 11 complete `████████████`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-12001` | Translate gizmo | ✅ | `STUDIO-11006` |
| `STUDIO-12002` | Rotate gizmo | ✅ | `STUDIO-11006` |
| `STUDIO-12003` | Scale gizmo | ✅ | `STUDIO-11006` |
| `STUDIO-12004` | Local and world transform spaces | ✅ | `STUDIO-12001` |
| `STUDIO-12005` | Multi-selection transforms about a shared pivot | ✅ | `STUDIO-12001` |
| `STUDIO-12006` | Pivot editing | ✅ | `STUDIO-12005` |
| `STUDIO-12007` | Snapping: grid, angle and scale increments | ✅ | `STUDIO-12001` |
| `STUDIO-12008` | One undo entry per drag, returning exactly to the drag start | ✅ | `STUDIO-12001` |
| `STUDIO-12009` | Box selection | ✅ | `STUDIO-11006` |
| `STUDIO-12010` | Duplicate, delete, parent and reparent from the viewport | ✅ | `STUDIO-12005` |
| `STUDIO-12011` | Drag and drop placement from the Content Browser into the scene | ✅ | `STUDIO-09008` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-12001` — Translate gizmo

**Acceptance.** An entity can be moved along one axis or in one plane, in either space, snapped or
free, as one entity or as a selection — and where the manipulator is drawn is where it is grabbed.

**Most of it was already there and one thing was not.** The 2D gizmo has had two arms and a `Both`
centre handle since ED-401; the 3D one has had three arms, screen-space sizing, local and world
spaces, a shared pivot for multi-selections and snapping since the prototype. What it did not have
was **plane handles** — it was three bare lines — so sliding an object across a floor took two drags
along two arms and landed wherever the second one stopped. That is the commonest 3D move there is,
and the 2D gizmo's centre handle is the same idea with only one plane to choose from.

**Three squares, not one.** A single "screen plane" handle is simpler and slides an object along
whatever the camera happens to be looking at, which is not a direction anything in the scene is laid
out along. Which plane the user wants is the question they answer by reaching for one of three.

**Each square is drawn in the colour of the arm it leaves out** — the XY square is blue, for Z.
That is the only labelling a square between two arms can carry, and it says the useful thing: which
axis the drag will not touch.

**The squares start a quarter of the way out**, and that is what stops the arms and the planes
fighting over a press: every pixel of an arm's own length is still the arm's. The hit test tries the
arms first as well, because an arm is a line and a plane is an area, so a press within a few pixels
of an arm is far more likely to be aimed at it. The case that would catch getting this backwards
asserts a press halfway along the X arm is still X.

**The third axis is the assertion that matters for the drag.** A plane drag solved as "wherever the
ray happens to be" moves the entity off the plane as soon as the cursor leaves the square — the
failure that makes a plane handle worse than two axis drags rather than better. `intersectRayWithPlane`
is the plane counterpart of `closestPointOnAxis` and refuses in the same two ways: a plane seen
exactly edge-on has no answer, and a ray pointed away from a plane meets it at a negative distance,
which would drag the object to a mirror image of where the cursor is.

**Snapping rounds both in-plane axes and neither other.** The *result* rather than the movement, as
the axis form already does, so the entity lands on the grid rather than a grid-sized distance from
where it happened to start — and the axis the plane leaves out keeps whatever the entity had.

**Verification.** `tests/SceneTests.cpp`: three visible squares whose normals are the arms they
leave out; the middle of each is a grab on that plane; a press along an arm is still the arm and the
origin belongs to no plane; the gizmo draws three arms and three squares. And the drag: two axes
move, the third does not, snapping rounds the two and leaves the third, an edge-on plane and a ray
pointed away both refuse, and the ordinary intersection is where the geometry says. Checked by
causing both: planes tested before the arms makes the X drag unreachable, and a drag that leaves the
plane fails three assertions including the one that names the third axis.

### `STUDIO-12002` — Rotate gizmo

**Acceptance.** An entity can be turned about any of the three axes, in either space, snapped or
free, as one entity or as a selection — and the rings read as a ball rather than as three ellipses
drawn over each other.

**Most of it was already there and one thing was not.** Three rings, sampled and projected so that
what is drawn is exactly what can be grabbed; an angle measured on the ring's own plane from the
press rather than accumulated frame to frame; the delta wrapped into (-π, π] so dragging across the
seam does not spin the entity; world-space turns stored in the parent's frame; a shared axis for a
whole selection so twenty entities cannot drift apart. What was missing is that every ring drew
**all** of itself.

**The far half of a ring turns the opposite way on screen from the near half**, because it is the
same circle seen from behind. So a press that landed on the back of a ring read as the gizmo working
backwards — and three full circles over each other made landing there easy, because at any angle
worth working at, two of them cross the third twice.

**Hidden against the gizmo's centre, not against the eye's distance to each point.** That is the
choice that makes a ring seen *face-on* keep the whole of itself: all of its samples are then level
with the centre, none of them is behind it, and a rule written against raw depth would cut the one
ring the user can see best arbitrarily in half. The comparison carries a small negative tolerance
for exactly that case, where the dot product is zero up to rounding.

**The visible set is one contiguous arc in cyclic order, and it can straddle the seam** of the
sampling, so the run is found on the circle rather than on the array. A face-on ring keeps the
closing sample and stays a closed circle; a tilted one becomes an open arc and is drawn and
hit-tested as one.

**A ring seen edge-on is still dropped entirely**, as it was: it would project to a line through the
centre, overlap the other two, and its plane is then nearly parallel to the cursor ray, so a drag on
it has no angle to measure.

**Verification.** `tests/SceneTests.cpp`: a tilted ring keeps an arc rather than a circle; the point
on it nearest the eye is a grab on that ring and the point furthest is not — asserted as "not this
ring" rather than "nothing", because a ring's hidden half can pass close to another ring's visible
one and what matters is that it has stopped being a handle for its own. And a face-on ring keeps
every sample and stays closed, while the two edge-on ones are absent. Checked by causing both:
keeping the back half fails the arc and the grab assertions on all three rings, and a tolerance of
the wrong sign empties the face-on ring entirely.

### `STUDIO-12003` — Scale gizmo

**Acceptance.** An entity can be resized along one axis, on two at once by one factor, or on all
three — snapped or free, as one entity or as a selection — and a drag can flip it without ever
landing on zero.

**Most of it was already there and one claim in it was wrong.** Three arms ending in handles, a
centre handle for the uniform case, a factor that is a *ratio* of screen distances rather than a
difference (the only measure of a unitless quantity that means the same at every camera distance),
snapping applied to the shared factor so a selection stays in proportion, `keepScalable` so a drag
through the origin flips the entity — which XNA's negative scale supports and a user may well mean
— without landing on zero, where it would be invisible *and* unclickable. Arms are never dropped
and never refuse, because a scale is a ratio of screen distances and the screen always has one; a
foreshortened arm shortens to a floor and fades instead, which keeps it separately grabbable from
the centre handle.

**What was missing is a plane handle, and the comment claiming it was not needed was mine and was
wrong.** `STUDIO-12001` left a note saying scaling "in a plane" is two independent factors and the
two arms already say so. Two arm drags give two factors that are independent *and unequal* — each is
a ratio of how far along its own arm the cursor went, and getting two of them to agree by eye is not
something anybody does. "Twice as wide and twice as deep, same height" is one operation, and it had
no handle. The note is corrected in place rather than left standing.

**One factor on both of the plane's axes.** That is the whole content of the handle and the
assertion that matters: the case checks the two results are equal *to each other*, not merely that
both grew, because an implementation measuring each axis separately passes everything else.

**The same three squares, in the same places as the translate gizmo's**, built by one shared
function — a user who has learnt where the XY handle is under W finds it there under R. What a drag
on one *means* differs, and that is the only difference: translate slides in the plane, scale
multiplies both of its axes.

**Built from the unfloored arm length.** The arms are redrawn to a minimum so their handles stay
clear of the centre one; a square pulled in with a foreshortened arm would be saying the plane is
somewhere it is not.

**The ordering guard is a guard, not a gate.** The planes are tried after the arms, as on the
translate gizmo. There it fires: those arms are bare lines and a square does project over one. Here
the end handles are checked before anything else and the squares start a quarter of the way out on
*both* their axes, so no camera angle tried puts a square over an arm — the guard is kept because it
is cheap and because the day a handle size changes is the day it starts mattering, and the plan says
so rather than the test claiming a gate it does not have.

**Verification.** `tests/SceneTests.cpp`: three squares in the same screen positions as the
translate gizmo's; each is a grab on its own plane; the centre and the arm handles still win where
they are. And the drag: not-moved is no edit; twice out along the diagonal doubles *both* of the
plane's axes and leaves the third at one; the two results are equal to each other; the YZ handle
leaves X alone; and the gizmo draws thirty-one segments. Checked by causing it — one axis of the
pair scaled by a different factor fails three assertions, including the one that names the claim.

### `STUDIO-12009` — Box selection

**Acceptance.** A drag over empty space sweeps a rubber band and selects what it covers, in both
viewports, without changing what a click does.

**Overlap, not enclosure**, and this is the decision the task turns on. Requiring an entity to be
wholly inside the band is the tidier rule and the wrong one: a level's backdrop is larger than the
viewport, so nothing could ever band it, and a user would learn that the rubber band works on small
things only. The cost is real and worth naming — that same backdrop is caught by every band drawn
over it — and it is the lesser of the two, because a selection with one thing too many in it can be
seen and corrected while one that silently cannot include an object cannot.

**The threshold is what makes this safe to add at all.** Both viewports already treat a press as a
selection, and a click on empty space is how a user deselects. Without a minimum travel every click
becomes a band a fraction of a pixel wide, and that deselecting click becomes a band that selects
whatever happens to sit under the pixel. Four pixels: below a hand's own tremor on a press, above
nothing. The case that proves it is the one that turns the manipulator off and Ctrl-clicks a
selected entity — a *click* toggles and removes it, a *band* adds as a union and would leave it
selected, so a press with no movement that became a band shows up immediately.

**Adding is a union and never a toggle**, which is the one place a band differs from a Ctrl-click.
A band sweeps an area, and an area that happens to cover something already selected should not
remove it: a user widening a band would watch entities drop out of the selection as the band grew
over them. An empty band with no modifier still replaces, so sweeping nothing clears — the same
thing a click on nothing does.

**An icon is boxed at the icon's own size.** An entity with no geometry has no world bounds, so a
band written only against bounds would sweep straight past every camera, light and marker in the
scene — the entities a user most often wants to gather up, and exactly the ones the click picker
already finds by their icons.

**A box in the world is not a box on the screen**, so the 3D form takes the screen extent of all
eight projected corners. Two of them is the fast wrong answer, invisible until something is viewed
off-axis; the case that catches it puts an isometric camera on a cube and bands a one-pixel strip
beyond where `min` and `max` project. A corner behind the eye is dropped rather than clamped, so an
entity the camera is standing inside is measured by the part of it the user can actually see.

**The band goes where a left drag already means "select".** Maya's scheme puts every gesture behind
Alt and Blender's puts them on the middle button, so under both an unmodified left drag is a
selection and the band needs nothing moved to make room. Studio's own scheme spends the plain left
drag on the orbit, so the band goes on **Ctrl+left** there — consistent rather than invented,
because Ctrl already means "add to what is selected" on a click, and Ctrl is no part of Studio's
navigation vocabulary, so nothing was taken away to make room.

**Drawn in the UI layer rather than by the renderer.** A rubber band is editor chrome over the
viewport image, like the toolbar, so it is a `fillRect` and a `strokeRect` on the frame's draw list
— which means where it goes is testable in a headless build, and no CNA-side pass was needed for it
at all. The band is held in *panel* pixels so a wheel notch mid-drag does not stretch it, and so a
docked panel that moves carries its band with it.

**Verification.** `tests/ViewportTests.cpp`: a band takes what it overlaps in either drag direction,
takes a sprite it merely clips, takes nothing over empty space, passes over a disabled entity, and
takes a camera by its icon while a band clear of that icon takes nothing.
`tests/SceneTests.cpp`: the 3D form sweeps what the camera can see, and catches a cube beyond where
two corners project. `tests/StudioViewportPanelTests.cpp`: a drag bands and a press that does not
move is still a click, including the Ctrl case that tells the two apart; and the rectangle is not
drawn until the press has become a band, comes out the right way round dragged backwards, and moves
with the panel. `tests/StudioViewport3DTests.cpp`: the band is the left drag under Maya's scheme and
Ctrl+left under Studio's, where the plain drag still orbits. Checked by causing four: enclosure
instead of overlap, two projected corners instead of eight, no threshold, and Ctrl+left orbiting in
Studio's scheme. Each fails by name.

### `STUDIO-12004` — Local and world transform spaces

**Acceptance.** The translate and rotate gizmos work in either space, the user can switch between
them, and the one gizmo that has no space says so rather than pretending.

**The substance was there and one thing lied.** `GizmoSpace` runs through both translate gizmos and
both rotate ones: in World the arms are the world's axes and a drag along X moves the entity right
whatever it is doing; in Local they follow the entity's own, which is what somebody placing
something inside a rotated rig wants. `X` toggles it, the toggle is checkable so a toolbar can show
which space is live, and any drag in flight ends with the space that owned it -- a translate
half-finished in world space would otherwise keep writing world deltas into a local transform.

**Scale has no space, and the toggle used to accept the press anyway.** A non-uniform scale in world
space needs a shear, which a position/rotation/scale transform cannot express, so *neither* scale
gizmo takes a `GizmoSpace` at all -- both say so in their own documentation. Pressing X while
scaling flipped the check and changed nothing on screen. A control that responds and does nothing is
worse than one that refuses, because the user has no way to tell which of the two just happened.

**Disabled rather than hidden**, which is the rule the rest of this shell already follows for the
3D-only commands and the ground-plane toggle: somebody who went looking for it should find it and
see why it is greyed out.

**The chosen space survives the trip through Scale**, so switching to Scale and back does not
quietly reset it -- the setting belongs to the user, not to the mode that happened to be active.

**Verification.** `tests/StudioViewport3DTests.cpp`: the toggle is live and works under Translate,
stays live under Rotate, refuses under Scale, and the space the user chose is still theirs on the
way back. `tests/ViewportTests.cpp` already pins the arithmetic -- a local layout's arms follow the
entity's rotation and a world layout's do not. Checked by causing it: a toggle enabled everywhere
fails by name.

### `STUDIO-12005` — Multi-selection transforms about a shared pivot

**Acceptance.** A selection of several entities drags, turns and resizes as one arrangement about
their shared pivot, in one undo entry — and every handle that works on one entity works on many.

**The mechanism was there and one third of it was dead.** `MultiTransform3D` captures the
selection's *roots* at the press — a child carried by a selected parent would otherwise be
transformed twice — holds each one's start transform so every frame is measured from the press
rather than the last, and turns a gesture into a list of edits applied as a single command. The
gizmos hand it the *gesture* rather than their own answer: `getWorldDelta`, `getDeltaAngle`,
`getFactor` exist so that twenty entities cannot each solve the cursor against their own handle and
drift apart.

**The uniform scale handle did nothing on a multi-selection.** The multi path built its per-axis
factors with three conditions — is this X, is this Y, is this Z — so the centre handle, which is
neither, produced a factor of one on every axis. The drag ran, changed nothing, and reported
nothing. That is the quietest way for a manipulator to be broken: no error, no movement, and a user
who concludes the selection is somehow locked. It is also the commonest scale there is.

**The plane handles inherited the same hole the day they were added** (`STUDIO-12003`), which is the
argument for the fix being one shared function rather than a fourth and fifth condition.
`gizmoScaleFactors` maps a handle and a factor to the three numbers, and both the single-entity path
and the multi-selection path now go through it — so the next handle added cannot be dead on one side
and alive on the other.

**A group resizes about the pivot rather than in place**, which is what the case asserts alongside
the sizes: two entities that grew without moving apart would overlap each other, and the offsets
have to grow along the same axes the sizes do.

**Verification.** `tests/StudioViewport3DTests.cpp`: the translate and rotate halves were already
pinned — both entities move by the same amount, turn about the shared pivot, and land in one undo
entry. Added: a uniform drag grows both entities on all three axes by the same factor and pushes
them apart from the pivot, and a plane drag grows two axes by one factor across the selection while
the third stays at one. Checked by causing it — the three-condition mapping restored fails six
assertions across both halves, by name.

### `STUDIO-12010` — Duplicate, delete, parent and reparent from the viewport

**Acceptance.** An entity picked in the viewport can be duplicated, deleted, parented and unparented
without leaving it — and a reparent leaves the object where it is.

**Two of the four already worked, and finding that out was the task's first half.** Ctrl+D and
Delete are shell commands that act on the *selection*, and the viewport is one of the things that
sets it: both take the selection's roots, batch into one undo entry, and act wherever the user
picked. Nothing was needed for those but to check they were true.

**A reparent moved the object, which is the defect this task existed to find.** An entity's stored
position, rotation and scale are relative to its parent, so moving it under a different one and
leaving the numbers alone moves the *object* — across the level, if the new parent is somewhere
else. Dropping a prop onto a moved rig in the Outliner teleported it. The numbers are the
implementation and the object is what the user is looking at, so the object is what stays still:
`ReparentEntityCommand` now captures the world transform at the press and rewrites the local one on
both execute and undo.

**`localTransformUnder` is the inverse of the composition `computeWorldTransform` performs**, in the
reverse order — undo the parent's translation, then its rotation, then its scale. Exact when the
parent's scale is uniform, and the closest a position/rotation/scale transform can come when it is
not: a rotated child under a non-uniformly scaled parent is a shear, and such a transform cannot
hold one. The same limitation the scale gizmo documents, because it is the same limitation.

**A parent with a zero scale flattens the space its child lives in.** There is then no local number
that puts the child back, because every local number multiplies to the same place. Keeping the world
value on that axis is the closest thing to "where it was" that exists — and it is what the child
gets back the moment the parent is given a size again, which an infinity or a NaN would not be.

**Attach and Detach are the viewport's half of parenting.** The *last* selected entity is the
parent, which is the rule every editor with this command uses: a user builds the group by clicking
the pieces and finishes on the thing they all belong to. Both are one undo entry, because
reorganising a hierarchy one entity at a time on Ctrl+Z would put the scene through arrangements it
was never actually in. An entity that is already there is skipped, and one that is an *ancestor* of
the target is refused and said so: the document rejects the cycle anyway and leaves the scene
untouched, so pushing the command would be harmless and would leave an undo entry that undoes
nothing.

**Both are unbound**, for the reason the six standard views are: Unreal's Ctrl+P and Shift+P are the
nearest thing to an agreed chord and neither key is in this editor's vocabulary at all, so a menu
entry somebody can bind for themselves beats a third scheme nobody knows.

**Verification.** `tests/CommandTests.cpp`: a reparent under a parent that is moved, turned *and*
scaled leaves the world transform unchanged while the stored numbers do change — the second half
matters, because an implementation that left them alone would pass the first against an identity
parent; and undo restores the parent and the numbers, so a file saved after an undo matches one
saved before. Plus the flattened parent staying finite. `tests/StudioShellActionTests.cpp`: Attach
greys out under one selection, parents to the last selected, undoes and redoes as one entry; Detach
greys out until something is parented and takes them back; and an attach that would make an entity
its own descendant moves nothing and records nothing. Checked by causing four: no rewrite, a
division by a zero scale, the first selected as parent, and the cycle check dropped. Each fails by
name.

### `STUDIO-12011` — Drag and drop placement from the Content Browser into the scene

**Acceptance.** An asset dragged out of the Content Browser and let go over either viewport becomes
an entity there.

**Half of it shipped with `STUDIO-09008` and the other half did nothing.** The 2D view has accepted
drops since then — it highlights while a drag is over it, reports where the pointer was in world
units, and the shell turns that into an entity. The 3D view accepted *nothing*: no highlight, no
placement, no explanation. The gesture simply died, which reads as the Content Browser being broken
rather than as the view not supporting it.

**A drop lands on the grid's own plane**, whichever of the two the user has chosen. The grid is the
only landmark a 3D view has, so a thing dropped onto it is a thing standing somewhere the user can
see. A fixed depth would put the entity in mid-air at a distance nobody chose, and the origin would
put it under whatever is already there — which is precisely the complaint the 2D case was written
against.

**When the ray does not reach that plane, the drop lands in front of the camera**, at the distance
it is orbiting at. That happens looking edge-on to the plane or away from it, and the alternative is
refusing a drop the user has already committed to. Somewhere they can see it and drag it from is all
a drop has to promise.

**The viewport reports and the shell acts**, exactly as the 2D path does: the panel does not know
what an asset becomes, and a panel that created entities would be one that had to know about
textures, models, prefabs and whatever comes next.

**Verification.** `tests/StudioViewport3DTests.cpp`: the landing point is on z = 0 for the scene
plane and y = 0 for the ground one, tracks the cursor rather than merely lying on the plane, and
stays finite and in front of the eye when the ray never reaches it. And the panel end to end — a
drag begun over it is reported with the asset's id and the panel's own landing point, and nothing
reaches the scene from the panel. Checked by causing both: a panel that ignores the drop, and a
landing point that always takes the fallback. Each fails by name.

### `STUDIO-12006` — Pivot editing

**Acceptance.** A user chooses what a multi-selection turns and resizes about, and both viewports
answer the same way.

**Two modes, because they are two intentions and neither is a rounding of the other.** "Arrange
these relative to each other" wants the middle of them; "put these where that one is" wants that
one. `Center` is the average of the selection's world positions and is what the editor did when
there was no choice, so it stays the default and every existing caller keeps meaning what it meant.
`Active` is the last-selected entity's own origin — Unity's Pivot, Blender's Active Element.

**`Active` is the only mode under which a rotation leaves something exactly where it was**, which is
what makes it the one to reach for when placing a part against a fixed feature. About the middle,
*every* member moves.

**The last that has a transform, walked backwards**, so a selection ending on something
untransformable still has a pivot rather than the gizmo vanishing because the user happened to
finish on a node with nothing to place.

**The centre of the bounding box is deliberately not a third mode.** That reasoning predates this
task and is left standing: the bounds centre moves when an entity is merely *rotated*, with nothing
having been asked to move, so a user turning one piece of a group would watch the group's pivot
drift under them. A mode whose own documentation has to warn about that is a feature with a
footnote.

**Live in every view, and exclusive.** A mode is a preference rather than a property of the current
selection, so the commands do not flicker as the user picks and unpicks things; and a drag in flight
ends with the pivot that owned it, for the same reason a space change ends one — a turn
half-finished about the centre would keep writing angles measured from a point the gizmo has left.

**Verification.** `tests/StudioViewport3DTests.cpp`: the two modes give x = 20 and x = 40 for the
same pair of entities, `Active` follows the selection order while `Center` does not, and `Center` is
what an unqualified call returns. And through the panel: a turn under `Active` leaves the
last-selected entity exactly where it is while the other moves. That case asserts the drag
*happened* before asserting what it did — a panel ignoring the mode puts its gizmo at the middle,
the press on the ring drawn about the active entity misses entirely, and everything else would pass
by nothing having moved. Checked by causing it: the panel dropping the mode fails both, by name.

### `STUDIO-12007` — Snapping: grid, angle and scale increments

**Acceptance.** A drag rounds to the project's own grid, angle and scale steps, and snapping is a
state a user can see rather than a key they hold all day.

**The rounding worked and two thirds of the task did not exist.** `GizmoSnap` carries all three
steps and every drag path applies them — the translate result is snapped rather than the movement,
the rotate delta is snapped as an angle, the scale factor is snapped as the quantity a whole
selection shares. What was missing: only the *grid* step was a project setting, and snapping
happened only while Ctrl was held.

**Fifteen degrees and tenths were constants in the editor.** That suits most projects and suits an
isometric one badly: a game laid out on thirty-degree facings, or built out of pieces that double,
wants to say so once, exactly as one laid out on a sixteen-pixel tile grid already could.
`angleSnap` and `scaleSnap` join `gridSnap` and take its shape in every respect — zero means the
editor's default, negatives are refused rather than clamped, and the keys stay out of the file until
they are set, so the first save of every existing project is not a diff that says nothing.

**Both viewports read the same settings**, because how a project is authored is not a property of
which viewport it is being authored in. The one difference is the fallback the 3D view already had:
it has no on-screen grid to borrow a step from, so an undeclared project lands on one world unit.

**Snapping is a state, and the modifier inverts it.** Ctrl was the only way to snap, so a user
laying out a level on a grid held it for every drag of the day. With the toggle off, Ctrl still
snaps, exactly as before. With it on, Ctrl is the *momentary escape* — the one placement that has to
sit off the grid — which is the part worth a case: a modifier that merely repeated the setting would
leave no way to make that placement but to turn snapping off and remember to turn it back on.

**Checkable, and not disabled in any view.** A viewport where a drag rounds and one where it does
not look identical until the drag happens, so a user who cannot see which they are in finds out by
placing something wrong. And the setting survives switching views, because both gizmos round.

**Verification.** `tests/ProjectAndAssetTests.cpp`: the two new settings round-trip, refuse
negatives, and stay out of the file until set. `tests/StudioViewport3DTests.cpp`: the four cases of
toggle-against-modifier; the same gesture lands on an awkward number free and on a multiple of five
snapped, with five chosen precisely because one is the undeclared fallback and a case using it would
pass whether the setting was read or not; and the command is checked, unchecked and survives a view
change. Checked by causing both: a modifier that repeats the toggle instead of inverting it, and a
3D snap that ignores the project's step. Each fails by name.

### `STUDIO-12008` — One undo entry per drag, returning exactly to the drag start

**Acceptance.** Carried forward from the prototype and retested through the Studio UI.

**The mechanism was there and one of its two gates was not.** A drag's first edit is a `NewEntry`
and every one after it is a `MergeWithPrevious`; `SetPropertyCommand::mergeWith` adopts the newer
command's final value and **keeps its own original** as the undo target; and an edit that would
write the value the document already holds writes nothing, so grabbing a handle and thinking better
of it leaves no entry to undo.

**What was missing is a case that could see any of it.** The three 3D cases that said "as one undo
entry" drove the pointer from the grab to the target in a *single jump*, so the drag committed
exactly once — one entry out of one edit, with the merge never running. That says nothing about the
mechanism that turns sixty edits into one, and sixty entries is what a user gets when it breaks. The
2D side already had a proper case; the 3D side did not, and its comment claimed otherwise.

**The half a merge gets wrong is the undo target.** Adopting the newer command's *old* value as well
as its new one gives an undo that goes back to the previous frame. On a single jump that is
indistinguishable from correct; on a real drag it leaves the entity a pixel from where it started,
with nothing left on the stack to fix it.

**Undone is not gone.** The case asserts the history's *cursor* rather than its entry count: an
undone entry stays on the stack as a redo target, so counting entries would say the undo had not
happened. Redo puts the whole drag back in one step, which is the other half of "one entry" and the
half a per-frame stack makes unusable.

**Verification.** `tests/StudioViewport3DTests.cpp`: a `dragVia` helper that presses, moves through
five intermediate points and releases; the drag is one entry, undo returns to exactly where it
began, redo restores it; and a press on a handle that goes nowhere leaves the stack untouched.
Checked by causing both halves: dropping the merge gives five entries and an undo that lands
mid-drag, and a merge that adopts the newer old value leaves the entity short of its start.

