# Phase 20 — Lighting

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-20001` … `STUDIO-20999` and are never reused.

**Purpose.** Lighting authoring that matches what the runtime can actually execute.

**Exit criteria.** The viewport and the game preview agree, and no light type exists in Studio that the runtime cannot render.

**Progress:** 4 of 8 complete `██████░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-20001` | Directional light authoring | ✅ | `STUDIO-19001` |
| `STUDIO-20002` | Point light authoring | ✅ | `STUDIO-20001` |
| `STUDIO-20003` | Spot light authoring | 🔄 | `STUDIO-20001` |
| `STUDIO-20004` | Ambient and environment lighting | ✅ | `STUDIO-20001` |
| `STUDIO-20005` | Sky and environment map authoring | ⬜ | `STUDIO-10010` |
| `STUDIO-20006` | Shadow configuration | 🔄 | `STUDIO-20001` |
| `STUDIO-20007` | Viewport lighting matches the game preview as closely as the runtime allows | ✅ | `STUDIO-20001` |
| `STUDIO-20008` | No light type is offered that the runtime cannot render | 🔄 | `STUDIO-20001` |

## A correction, and it affects four rows below

**`STUDIO-20002`, `STUDIO-20003`, `STUDIO-20006` and `STUDIO-20008` were written against a false
belief about what CNA can do, and three of them have been reopened.**

The belief was that `IEffectLights` — an ambient colour and three directional lights — is the whole
of CNA's lighting, so that a point light must be approximated, a spot light's cone cannot exist,
and shadows are impossible. Everything in that sentence is true of `IEffectLights`, which is the
*XNA 4.0* interface, and Studio's model pass uses nothing else. It is false of **CNA**, whose own
additions live in the CNAEXT layer:

| What was believed impossible | What CNA actually offers |
|---|---|
| A point or spot light | `PbrEffect::setPunctualLightEXT`, a `PunctualLightEXT` with position, direction, range and **inner and outer cone angles** — one shadowed punctual light per draw, beside the three directional slots |
| A shadow | `PbrEffect` implements `IShadowReceiverEXT`; the CNAEXT engine layer ships `CNA::Graphics::ShadowMap`, with cascades, quality levels and rigid and skinned caster effects |
| Environment lighting | `PbrEffect::setImageBasedLightEXT`, taking irradiance and prefiltered specular cubes and a BRDF LUT |

**Two CNA gaps were filed in error and are withdrawn** — `G-13` and `G-14` in
[`docs/CNA-GAPS.md`](../docs/CNA-GAPS.md), kept and marked rather than deleted. The error was
reading Studio's own code to decide what CNA lacks. The register exists to report CNA's rough
edges; filing a gap against a feature that exists costs its maintainers time and tells Studio's own
readers something false.

**What survives and what does not.** The editor-side work in those rows stands: the archetype
presets, the Point and Spot rows, the overlay that stopped drawing an aim arrow on a point light,
the conditional-property mechanism, and the validation plumbing are all correct and all still
wanted. What was wrong is the *justification* — "the renderer cannot" — and the conclusions drawn
from it. The two validation rules now say what Studio does rather than what CNA cannot, and they
are temporary by construction: each becomes silent when the work below lands.

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
row did not change it — and no per-light shadow settings (`STUDIO-20006`).

**And a correction.** A point light is approximated here as a directional light aimed at what is
being drawn, which this row described as forced by the API. It is forced by *Studio's use* of the
API: CNA's `PbrEffect::setPunctualLightEXT` takes a real point light. Drawing one is
`STUDIO-20003`'s reopened work, and this row's authoring half is unaffected by the correction.

### `STUDIO-20003` — Spot light authoring

**Acceptance.** A user can add a spot light, place it, aim it and bound it — and is told, where it
costs them something, that this build does not draw its cone.

**Everything a spot light has except the cone already worked.** It is the only kind that uses all
three of position, direction and range, and `SceneLighting`'s reduction resolves the first and
third exactly as it does for a point light while the second follows the entity's forward axis. The
archetype is one preset — `kind` set to `Spot` — because the three kinds are one component
(`STUDIO-20002`).

**🔄 Reopened. The paragraph that used to sit here was wrong.** It said the cone "cannot be"
approximated because `IEffectLights` has nowhere to put an angle. `IEffectLights` does not, and
**CNA does**: `PbrEffect::setPunctualLightEXT` takes a `PunctualLightEXT` carrying position,
direction, range, `InnerAngle`, `OuterAngle` and a shadow map of its own — one such light per draw,
beside the three directional slots. The gap filed against CNA over this (`G-13`) is withdrawn.

**What shipped is still right as far as it goes**, and it is the authoring half: a Spot Light row
in the Entity menu, the preset mechanism that makes a kind expressible, and an overlay and
Inspector that tell the truth about which of a light's fields this *viewport* currently uses.

**What remains is the renderer half**, and it is Studio's work rather than CNA's:

1. `SceneLighting` reduces every light to three directional slots. It needs a second output — the
   one punctual light, chosen per drawn object the way the three brightest already are — so that a
   point light stops being a directional light aimed at its target and a spot light gets its cone.
2. `CNA.Light` then needs the two cone angles it has never had, because at that point they are
   values the renderer reads rather than a field nobody honours.
3. `CnaModelPass` applies it through `setPunctualLightEXT`, which it already has a `PbrEffect*` to
   call. `BasicEffect` does not implement the extension, so the capability report
   (`STUDIO-19008`'s) is where a build drawing through it says so.

Until that lands, `validateScene` reports `spot-light-cone-not-rendered`, and the rule now says
what Studio draws rather than what CNA cannot.

### `STUDIO-20008` — No light type is offered that the runtime cannot render

**Acceptance.** Where the editor offers a light this renderer cannot draw faithfully, it says so
on the light, in the report a user already reads — not in a header.

**🔄 Reopened, because the row's own title is now reachable rather than aspirational.** Both
limits below are Studio's rather than CNA's: `PbrEffect` takes a punctual light with a cone
(`STUDIO-20003`) and can sample a shadow (`STUDIO-20006`), so "no light type is offered that the
runtime cannot render" stops being a disclosure exercise and becomes a statement Studio can simply
make true. The rules stay until it is, and they now say what Studio draws rather than what CNA
cannot — but this row closes when there is nothing left to disclose about the spot light, not
before.

**Read as "say so" rather than as "remove it", and that part holds.** The literal reading of the
row is that `Spot` should come out of `CNA.Light`'s kinds. That was rejected: the kind has been in
the descriptor since Phase 1, scenes already hold spot lights, and a game reading the loader's
carried components can implement a cone for itself. Deleting a kind would break those scenes and
forbid something Studio has no business forbidding. `SceneLighting.hpp` had already written down
where the answer belongs — "the Validation panel is where that should be said to a user rather than
here" — and this is that.

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

### `STUDIO-20006` — Shadow configuration

**Acceptance.** A user is not offered shadow settings that nothing honours — or, where the settings
already exist and are carried through to a game, they are told plainly that the editor draws none.

**🔄 Reopened. The two paragraphs that used to follow were wrong**, and so was the CNA gap they
rested on. They said neither effect takes a shadow map and there is no seam for one that could.
`PbrEffect` implements `IShadowReceiverEXT` — `setShadowMapEXT`, `setLightViewProjectionEXT`,
`setShadowsEnabledEXT`, a depth bias — and the CNAEXT engine layer ships
`CNA::Graphics::ShadowMap`, which renders from a directional light, fits an orthographic volume to
the scene bounds, hands back rigid and skinned caster effects, and reports `isSupported()` where a
renderer cannot manage it. There are cascades and quality levels. `G-14` is withdrawn.

**Studio has offered shadow configuration since Phase 1 and has never drawn a shadow**, which is
the part that was true. `CNA.ModelRenderer` carries `castShadows` and `receiveShadows`, both
defaulting to true, both editable, and read by nothing **in Studio**. A user has been able to turn
a model's shadow off since the component existed and no picture has ever changed.

**The real work, now that the API is known:**

1. A shadow pass in `cna-studio-viewport`, around the model batch it already builds: `begin` with
   the scene's brightest directional light and the batch's world bounds, draw the casters with the
   caster effect, `end`.
2. `castShadows` decides what goes into that pass and `receiveShadows` decides whether a draw gets
   `setShadowsEnabledEXT(true)` — at which point both flags mean what they say.
3. `isSupported()` and the `BasicEffect` path are where a build that cannot do it says so, through
   the capability report rather than by drawing something misleading.
4. Only then is *configuration* worth adding — a quality level belongs in the project or the scene
   environment, not on every light, and it should arrive with the pass that honours it.

Until that lands, `validateScene` reports `shadows-not-rendered`, and the rule now says what Studio
draws rather than what CNA cannot.

**What is left is to say so, once, where it costs something.** `validateScene` reports
`shadows-not-rendered` for a scene that is actually set up to want a shadow: an enabled light and
an enabled model renderer that says it casts. A scene with no light casts none on any renderer and
a scene with no model has nothing to cast one, so neither says anything. And it is reported **once
for the scene rather than once per model**, because the flags default to on: a per-entity rule would
fire on every model in every project forever, which is the shape of a rule people configure their
way out of and then stop reading.

**Verification.** `tests/SceneTests.cpp` —
`AConfiguredShadowIsReportedBecauseThisBuildDrawsNone`: one issue for a scene that wants a shadow,
still one for a scene with five models, and nothing for a scene with no light, no model, or a model
that says it does not cast. A Warning rather than an Error, because the scene is legal and the flags
reach a game that may well honour them — what is wrong is only what the editor shows.

Checked by causing each: the rule firing more than once, and the rule dropping its "something
actually casts" condition, both fail by name.

**What this row is not, yet.** There is no shadow in the viewport. There is no per-light shadow
setting, no bias, no resolution and no cascade — and none of those should arrive before the pass
that honours them, which is the one thing the original version of this entry got right for the
wrong reason.

### `STUDIO-20007` — Viewport lighting matches the game preview as closely as the runtime allows

**Acceptance.** Where an approximation is unavoidable, it is documented rather than hidden — and
where the two views simply disagreed, they stop.

**They did not match. The game preview had no lighting at all, because it had no models.**
`CNA.Camera` has carried a `projection` — Orthographic or Perspective — since Phase 1, and
`computeGameView` read the clear colour, the orthographic size and the position and ignored that
one property. So the game view ran the **2D sprite pass** whatever the camera said. A project
authored in 3D, which is what a new CNA-native project opens into (`STUDIO-11014`), previewed as
its clear colour and its sprites: no models, no materials, no lighting, none of the work. "What
will a player see" was answered with a picture of almost none of the scene.

That is not an approximation to document. It is two views of one scene that were never connected,
and the row is about them agreeing.

**A game camera looks along its entity's own forward axis: +Z rotated by its rotation.** Nothing
in the repository had ever used a camera entity's *rotation* — the 2D path reads x and y and
stops — so the convention had to be chosen rather than looked up. It is `CNA.Light`'s, which
`SceneLighting.hpp` states and tests: the axis that points into a Y-down, XY-plane world. A camera
answering differently would mean Studio had two answers to "which way is this entity facing" about
the same transform. It is deliberately *not* the editor orbit camera's zero pose, which is user
state rather than an entity's rotation and is free to start wherever suits an orbit.

**The orbit camera is positioned by putting its pivot in front of the entity.** `StudioCamera3D` is
an orbit — pivot, distance, yaw, pitch — and its eye is `pivot - forward * distance`. A game camera
has no orbit, so the pivot goes `distance` ahead and the eye lands exactly on the entity. For a
perspective view the distance is arbitrary and says so; for an orthographic one it is *not*, since
the visible height is derived from it, so there it comes from `orthographicSize` and the two
projections stay consistent with the 2D path.

**What the two views deliberately do not share.** The game view is given no wireframe, no
selection and **no debug view**. A player never sees a roughness view, so a preview that showed one
would answer a different question than the one it is asked. That is a difference by construction —
the editor passes are not run — rather than a filter, which is the same guarantee that keeps Studio
chrome out of a shipped game.

**Lighting agrees because both build the same batch**, and the test asserts it anyway. "They call
the same function" is a claim about today's code; what a user sees is the claim worth pinning, and
it is exactly the one that was false.

**Verification.** `tests/ViewportTests.cpp` — `APerspectiveCameraGivesAPerspectiveGameView` (the
projection is honoured, every field the component states reaches the camera, the eye is *on* the
entity rather than ten units behind it, the aim follows the entity's rotation, rotating does not
move it, and a scene with no camera keeps the flat fallback it always had) and
`TheGameViewAndTheEditorViewAgreeAboutLighting`, which builds both batches over one scene from two
very different cameras and compares the `EffectLighting` for the same entity.

The host's branch is a call into a device and cannot be asserted on headlessly, so it is guarded
the way `STUDIO-20004`'s pass is: `TheModelPassReadsEveryFieldTheLightingReductionFillsIn` now also
checks that `CnaStudioShellHost.cpp` mentions `perspective` and `renderGame3D` at all. A source
scan cannot say the branch is *right*; it can say the projection is not decided over the document
and then drawn by nothing, which is precisely the defect this row found.

Checked by causing each: the projection ignored, the pivot left on the entity so the eye sits
behind it, the forward convention flipped to −Z, and the host's branch removed all fail by name.

**What this row is not.** There is no camera frustum drawn in the editor viewport — a camera is
still a badge, and `EntityArchetypes.cpp` claimed otherwise until this row corrected it. The
`orthographicSize` and `fieldOfView` fields are still both editable whatever the projection says,
which is `STUDIO-20001`'s `appliesWhen` waiting to be applied one component over. And the game view
is still a preview rather than the player: `cna-player` runs the scene, and this draws it.

