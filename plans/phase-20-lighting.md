# Phase 20 — Lighting

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-20001` … `STUDIO-20999` and are never reused.

**Purpose.** Lighting authoring that matches what the runtime can actually execute.

**Exit criteria.** The viewport and the game preview agree, and no light type exists in Studio that the runtime cannot render.

**Progress:** 5 of 8 complete `███████░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-20001` | Directional light authoring | ✅ | `STUDIO-19001` |
| `STUDIO-20002` | Point light authoring | ✅ | `STUDIO-20001` |
| `STUDIO-20003` | Spot light authoring | ✅ | `STUDIO-20001` |
| `STUDIO-20004` | Ambient and environment lighting | ✅ | `STUDIO-20001` |
| `STUDIO-20005` | Sky and environment map authoring | ⬜ | `STUDIO-10010` |
| `STUDIO-20006` | Shadow configuration | ⬜ | `STUDIO-20001` |
| `STUDIO-20007` | Viewport lighting matches the game preview as closely as the runtime allows | ⬜ | `STUDIO-20001` |
| `STUDIO-20008` | No light type is offered that the runtime cannot render | ✅ | `STUDIO-20001` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-20001` — Directional light authoring

**Acceptance.** A user can add a directional light, aim it, set its colour and intensity, see it
light the models in the viewport — and the editor does not offer them a field that does nothing.

**Most of it was already built, and nobody could reach any of it.** `SceneLighting` has read
`CNA.Light` and reduced a scene's lights to the three directional slots `IEffectLights` offers
since ED-404; `SceneWireframe` draws a light's aim and its badge; the Outliner gives it an icon;
the model pass applies the result per entity. What was missing was both ends: **a light could not
be created** — `STUDIO-13013`, which this row waited on in practice if not on paper — and **the
Inspector offered a field that could not work**.

**`Range` was editable on a directional light and did nothing.** A directional light has no
position, so it has nothing for a falloff distance to fall off *from*: `falloffAt` returns 1 for
one whatever its range says. The descriptor's tooltip had said "Point and Spot only." since it was
written and nothing acted on it, so a user could type a number into the field, watch the viewport
not change, and have no way to tell a field that was never theirs to set from a renderer that is
broken. That is the state `STUDIO-12004` took the gizmo space toggle out of, one layer up.

**The fix is a rule, not a special case.** `PropertyDescriptor::appliesWhen` names a sibling
property and the values of it that make this one live; `CNA.Light`'s `range` applies when `kind` is
Point or Spot. The alternative — the Details panel checking for `CNA.Light` by name — is a rule
that only ever helps the one component somebody remembered to write it for, and `STUDIO-20003`'s
spot cone angle would be the second.

**Greyed rather than hidden**, which is the rule this shell already follows for the 3D-only
commands and for the space toggle under Scale: somebody who goes looking for a field should find it
and see why it is dead. The tooltip says which sibling is stopping it and what that sibling is
currently set to — "Inactive while Type is Directional." — built from the condition and the live
value rather than written into the descriptor as a second string that could disagree with it.
Reset and Paste are withdrawn from the row for the same reason the editor is: both would write a
value the user cannot see the effect of.

**A condition fails open and is checked at rest.** A condition naming a property that is not there
applies rather than not, because greying a field a user needs with no explanation is the more
expensive way to be wrong. The mistake itself is caught by reading the descriptors:
`EveryConditionalPropertyNamesASiblingThatCanSatisfyIt` fails when a condition names a missing
sibling, a sibling of a kind a condition cannot be written against, or a value that sibling's
enumeration does not offer — the last of which would grey a field on *every* kind and explain it
with a value the user cannot select.

**Verification.** `tests/StudioDetailsPanelTests.cpp` —
`ALightsRangeIsDeadOnADirectionalOneAndLiveOnAPointOne` (one inactive row on a directional light
and none on either of the others),
`APropertysConditionIsMetByItsSiblingsValueAndFailsOpenWithoutOne` (the rule itself, including the
two spellings a condition understands and the silence for everything else), and the descriptor
guard. `tests/SceneTests.cpp` —
`ADirectionalLightCreatedFromTheEditorLightsTheModelsInTheScene`, which is end to end on purpose:
every other lighting case builds its `CNA.Light` by hand and so cannot tell whether the light a
*user* gets is a light at all. It asserts the scene goes from `useDefaultLighting` to one white
light at full intensity, that rotating the entity turns it, and that moving it a hundred units
changes nothing — which is the same fact the greyed Range is about.

Checked by causing each: the condition removed from the descriptor, the panel ignoring it, the
condition failing closed instead of open, a condition naming a sibling that does not exist, one
asking for a value the sibling cannot hold, and the archetype arriving with no `CNA.Light` on it
all fail by name.

**What this row is not.** No shadow casting (`STUDIO-20006`), no sun-angle or time-of-day widget,
and no light gizmo beyond the aim line and badge the wireframe already draws — a directional light
has no handle to drag that the rotate gizmo does not already give it. The three-light cap that
`IEffectLights` imposes is reported to nobody yet; that is `STUDIO-20008`.

### `STUDIO-20002` — Point light authoring

**Acceptance.** A user can add a point light, move it, set its colour, intensity and range, see it
light the models near it and not the ones outside its reach — and the viewport does not draw them
an indicator of something a point light does not have.

**A point light could not be created, because a kind is not a component.** All three of
`CNA.Light`'s kinds are the *same* component told apart by an enumeration, so the archetype table
`STUDIO-13013` introduced — which says a kind is a name and a list of component type ids — had no
way to say "a point light" at all. `StudioEntityArchetype::presets` is that way: a component type
id, a property name and a value, applied over the descriptor's defaults rather than instead of
them. The Point Light row sets `kind` and nothing else, so its colour, intensity and range are
still whatever `CNA.Light` says they are.

**Three components mapping onto one would be three ways to write the same scene file.** The
alternative — `CNA.PointLight`, `CNA.SpotLight` — is a runtime change wearing an editor's clothes,
and the constraint on this whole programme is that Studio produces CNA games rather than CNA Studio
games. What the runtime reads is `CNA.Light`, and that is what the editor writes.

**A preset is checked against the descriptor it claims**, because every way of getting one wrong is
silent: naming a component the archetype does not build, a property the descriptor does not
declare, a value of the wrong type, or an enumeration option that does not exist all produce an
entity that looks right in the Outliner and is not the kind the menu row promised. Get the Point
Light's preset wrong and the user is handed a *directional* light called "Point Light".

**The overlay drew an aim arrow on a point light.** A point light shines equally in every
direction, so its rotation is nothing to it — and turning one swung a line about the viewport and
changed how the scene was lit by exactly nothing. That is an indicator of a fact that does not
exist, and it is the same defect `STUDIO-20001` took out of the Inspector one layer up. The arrow
is now drawn for the two kinds that have a direction, and the range ring for the two that have a
range.

**Which fields a kind uses is now three predicates rather than a condition written out wherever it
is needed.** `sceneLightUsesDirection`, `sceneLightUsesPosition` and `sceneLightUsesRange` live
beside the reduction that consumes them, and
`TheLightInspectorAndTheLightOverlayAgreeAboutWhatEachKindUses` holds `CNA.Light`'s `appliesWhen`
to them: the Inspector deciding a field is editable and the renderer deciding it changes nothing
are two descriptions of one fact, and a disagreement is a field a user can set and cannot see.

**Verification.** `tests/SceneTests.cpp` —
`APointLightCreatedFromTheEditorLightsFromWhereItIsAndStopsAtItsRange` (the preset lands, the rest
of the light is still the descriptor's, the light arrives from where the lamp is rather than along
its axis, moving the lamp moves where the light comes from, and outside its range the scene falls
back to the default rather than drawing the crate black),
`TheLightOverlayDrawsAnArrowOnlyWhereDirectionMeansSomething` (a spot light's segment count is a
point light's plus a directional one's, and a point light with no range draws nothing at all), and
the agreement guard. `tests/StudioShellActionTests.cpp` —
`EveryArchetypePresetNamesAPropertyThatExistsAndFits`.

Checked by causing each: presets never applied, the arrow drawn on every kind, the renderer
disagreeing with the Inspector about a spot light's range, a preset naming a property that does not
exist, and one naming an enumeration value that does not exist all fail by name.

**What this row is not.** There is no falloff *curve* — `falloffAt` is what ED-404 wrote and this
row did not change it — no per-light shadow settings (`STUDIO-20006`), and no warning yet that
`IEffectLights` takes only three lights at a time, which is `STUDIO-20008`.

### `STUDIO-20003` — Spot light authoring

**Acceptance.** A user can add a spot light, place it, aim it and bound it — and is told, where it
costs them something, that this build does not draw its cone.

**Everything a spot light has except the cone already worked.** It is the only kind that uses all
three of position, direction and range, and `SceneLighting`'s reduction resolves the first and
third exactly as it does for a point light while the second follows the entity's forward axis. The
archetype is one preset — `kind` set to `Spot` — because the three kinds are one component
(`STUDIO-20002`).

**The cone is not approximated, because it cannot be.** `IEffectLights` has an ambient colour and
three *directional* slots and nowhere to put an angle; there is no seam for an effect of one's own
(`G-12`); so a spot light is drawn exactly as a point light is. Recorded as **CNA gap G-13**.

**No `coneAngle` field was added, and that is the decision this row turns on.** Studio could
declare one on `CNA.Light` — the scene loader carries the component's properties through to a game
untouched, so a game *could* read it. What Studio could not do is draw it, and a field the editor
offers, cannot show, and nothing in CNA reads is the shape this session has spent three tasks
removing: a rule written down and honoured by nobody. A game that wants a cone has a better
answer already, which is a component of its own that Studio carries the same way.

### `STUDIO-20008` — No light type is offered that the runtime cannot render

**Acceptance.** Where the editor offers a light this renderer cannot draw faithfully, it says so
on the light, in the report a user already reads — not in a header.

**Read as "say so" rather than as "remove it".** The literal reading of the row is that `Spot`
should come out of `CNA.Light`'s kinds. That was rejected: the kind has been in the descriptor
since Phase 1, scenes already hold spot lights, and a game reading the loader's carried components
can implement a cone for itself. Deleting a kind over a limitation of *one renderer* would break
those scenes and forbid something Studio has no business forbidding. `SceneLighting.hpp` had
already written down where the answer belongs — "the Validation panel is where that should be said
to a user rather than here" — and this is that.

**Two rules, and the second is the one that took thought.**

`spot-light-cone-not-rendered` fires on every enabled spot light: "This build draws a spot light as
a point light: the effect has no cone." Per light rather than once per scene, because the answer is
per light — one may be standing in for a lamp and be perfectly fine as a point light, and another
may be the spotlight the level is built around.

`more-directional-lights-than-slots` fires when a scene holds more than three enabled *directional*
lights. The obvious rule — "more than three lights anywhere" — would fire on every real level and
be wrong to fire: `computeEffectLighting` picks the three brightest *where the object is*, so
twenty lamps spread across a level is twenty lamps working correctly. A directional light is
different in kind: it has no position, so it applies everywhere, and a fourth one can never reach
anything however the level is laid out. That is the case worth a warning, and confining the rule to
it is what keeps it quiet.

Both are Warnings. The scene is legal, it runs, and what it draws is a reasonable light — it is
simply not the light the user asked for, which is the definition this file gives for the severity.

**Verification.** `tests/SceneTests.cpp` — `ASpotLightIsReportedBecauseThisRendererHasNoCone` (one
issue per spot light, named on the light so the report's row selects it, silent for the other two
kinds, and resolved by switching the light off),
`MoreDirectionalLightsThanTheEffectHasSlotsIsReported` (three say nothing, four are each reported,
and four *point* lights say nothing at all), and
`ASpotLightCreatedFromTheEditorUsesItsPositionDirectionAndRange`, which pins the three predicates
against all three kinds.

Checked by causing each: the spot warning removed, the slot cap firing one light early, the rule
ignoring the enabled flag, point lights counted towards the directional cap, and the spot
archetype presetting the wrong kind all fail by name.

**What these rows are not.** There is no suppression list, so a user who has read the spot-light
warning reads it again every time the report is built — `STUDIO-33xxx` owns per-rule suppression
and this row does not pretend to. And the three-light cap is reported only for the case that is
unconditionally wrong; a scene where four *point* lights genuinely overlap will still silently drop
one, because saying so would need the check to run per object and per frame, which a structural
rule over a document cannot do.

### `STUDIO-20004` — Ambient and environment lighting

**Acceptance.** A scene's ambient light is authored in one place, belongs to the level rather than
to anything standing in it, and reaches every model drawn — including the ones in a scene that has
no lights at all.

**Most of it was ED-407's and one case was silently dead.** `SceneEnvironment` has held the
ambient since then, Scene Settings edits it through a command that merges like any other, and
`buildSceneModelBatch` writes it over `EffectLighting`'s own default for every draw. What nobody
had followed through was the *unlit* path: a scene with no enabled light is drawn through XNA's
`EnableDefaultLighting()`, which sets that rig's own ambient and then the pass **returned**. The
scene's ambient was computed correctly, written into the draw, and dropped on the floor.

**So darkening a scene did nothing until you put a lamp in it** — in exactly the scene the default
rig exists for, which is the one somebody has just dropped a model into. A user would set the
ambient, see no change, set it darker, see no change, and conclude the setting was broken. It was
not broken; it was unreachable.

**The fix is conditional, and the condition is the design.** `EffectLighting::
ambientOverridesDefault` is set when the scene states an ambient that is not the default one, and
the pass applies the ambient *after* `EnableDefaultLighting()` only then. A scene nobody has
touched keeps XNA's rig exactly — which is the promise `EnableDefaultLighting` is called for, that
a CNA scene and an XNA one with no lights look the same — and a scene somebody has deliberately
changed gets what they asked for. Always overriding would have broken the first promise to keep
the second; never overriding is the defect.

Setting the ambient *back* to the default puts the scene back under XNA's rig rather than pinning
it to a value that happens to equal it. The two are the same picture today and only one of them
follows the framework if its rig ever changes.

**Verification.** `tests/SceneTests.cpp` —
`TheScenesAmbientIsAppliedEvenWhenNothingLightsTheScene`, which asserts on the *flag* rather than
on the colour: the colour was already correct and already ignored, so a case that checked it would
have passed against the defect. It covers an untouched scene, a darkened one, a brightened one (the
rule is "the user said something", not "the user said something dark"), a scene with a light where
the flag is beside the point, and a scene set back to the default.

Applying it needs a device, so the pass's half is guarded by
`TheModelPassReadsEveryFieldTheLightingReductionFillsIn` in `tests/ArchitectureGuardTests.cpp` — a
source scan, honest about being one. It cannot say the field is applied *correctly*, only that
`CnaModelPass` mentions it at all; that is the difference between a field nobody wired up and a
field wired up wrongly, and only the first is invisible to every other test in the suite. It is
also precisely the failure this row found.

Checked by causing each: the flag never set (the defect as it was), the flag always set (which
breaks the XNA-parity promise), and the pass dropping the override all fail by name.

**What this row is not.** There is no image-based or environment *lighting* — a sky that lights
the scene is `STUDIO-20005`, and needs a cube map the effects have no slot for. There is no
ambient *occlusion*, which is a material's business (`STUDIO-19002` gave it a texture slot). And
fog, the environment's other half, is ED-407's and unchanged here.

### `STUDIO-20007` — Viewport lighting matches the game preview as closely as the runtime allows

**Acceptance.** Where an approximation is unavoidable, it is documented rather than hidden

