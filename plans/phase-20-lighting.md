# Phase 20 — Lighting

> **ARCHIVED — historical record. This file authorises no work.** It belongs to the
> [archived programme roadmap](../docs/ROADMAP-ARCHIVE.md), whose scope was retired on 2026-09-22
> by [ADR-001](../docs/ADR-001-SCOPE-REDUCTION.md). **A ⬜ below means *not built*. It no longer
> means *planned*.** The one authoritative active roadmap is [`plan.md`](../plan.md).
>
> **Disposition of this phase:** Complete.
>
> Ids in this phase are `STUDIO-20001` … `STUDIO-20999` and are never reused. Every id here still resolves, so a commit, test or code comment that cites
> one keeps its meaning.

**Purpose.** Lighting authoring that matches what the runtime can actually execute.

**Exit criteria.** The viewport and the game preview agree, and no light type exists in Studio that the runtime cannot render.

**Progress:** 8 of 8 complete `████████████`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-20001` | Directional light authoring | ✅ | `STUDIO-19001` |
| `STUDIO-20002` | Point light authoring | ✅ | `STUDIO-20001` |
| `STUDIO-20003` | Spot light authoring | ✅ | `STUDIO-20001` |
| `STUDIO-20004` | Ambient and environment lighting | ✅ | `STUDIO-20001` |
| `STUDIO-20005` | Sky and environment map authoring | ✅ | `STUDIO-10010` |
| `STUDIO-20006` | Shadow configuration | ✅ | `STUDIO-20001` |
| `STUDIO-20007` | Viewport lighting matches the game preview as closely as the runtime allows | ✅ | `STUDIO-20001` |
| `STUDIO-20008` | No light type is offered that the runtime cannot render | ✅ | `STUDIO-20001` |

## A correction, and it affects four rows below

**`STUDIO-20002`, `STUDIO-20003`, `STUDIO-20006` and `STUDIO-20008` were written against a false
belief about what CNA can do.** Three were reopened, and all three have since been closed again by
doing the work the correction revealed: a real punctual light in the effect, a drawn spot cone, and
a shadow pass in the viewport.

The belief cost a second defect on the way out, which is worth recording beside the first. The
punctual light `STUDIO-20003` added was sent through `PbrEffect`, because `PbrEffect` was the
header that had been read — and `BasicEffect` implements the same `IShadowReceiverEXT` interface
while being the effect this build actually draws with. The feature shipped switched off on the only
path anybody runs. **Reading one type's header is not reading the API**, in either direction.

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
from it.

Both validation rules the mistake produced are gone. `spot-light-cone-not-rendered` went first:
the cone is drawn now, and where it is not the effect says so through
`studioLightCapabilityIssues` rather than a document report that cannot know which effect a build
uses. `shadows-not-rendered` went with `STUDIO-20006` — the editor draws shadows, so a rule saying
it does not was simply false. What replaced it is narrower and still true:
`shadows-need-a-directional-light`, because the pass renders from a sun and Studio does not drive
CNA's cube and spot maps yet.

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

**What this row is not.** No shadow casting — that is `STUDIO-20006`, which has since landed —
no sun-angle or time-of-day widget,
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

**This row was completed twice, and the first time was on a false premise.** The first version said
a spot light's cone "cannot be" approximated because `IEffectLights` has nowhere to put an angle —
true of `IEffectLights`, false of CNA, which takes one real punctual light per draw through
`PbrEffect::setPunctualLightEXT`, with a position, a range and inner and outer cone angles. A CNA
gap was filed over that misreading and has been withdrawn (`G-13`). What follows is the work done
once the API was actually read.

**The reduction gained a second output.** `EffectLighting` carried three directional slots and now
carries an `EffectPunctualLight` beside them, filled per drawn object with the **brightest point or
spot light there** — one, because that is CNA's budget and its own documentation calls the ceiling
deliberate: each shadowed punctual light is another generation pass, six of them for a point light.

**The light that takes the slot is removed from the directional candidates**, and that ordering is
the whole of the correctness here: the same lamp sent both ways is a lamp at twice its brightness,
with nothing on screen to explain it. Every *other* point or spot light still gets the old
approximation, which is why that behaviour is still tested rather than deleted — it is now what the
runners-up get instead of what everything got.

**Its colour carries intensity and not falloff.** The directional approximation has no position to
measure from, so its falloff is baked into its colour; the punctual slot has one, so the effect
measures distance per *pixel*. That is the whole reason it is worth sending: a lamp beside a large
model now lights the near end of it and not the far one, which no amount of per-object dimming can
do.

**`CNA.Light` gained the two cone angles**, authored in degrees and carried in radians — the same
split `CNA.Camera`'s field of view uses, because a person types 35 and every renderer wants
radians. Both are `appliesWhen` Spot, so they are greyed on the other two kinds; the inner angle is
clamped to the outer one on read, because a hand-edited scene where it exceeds it would divide by a
negative band and light the *outside* of the cone, which looks like a renderer fault rather than a
bad number.

**The viewport overlay draws the cone** — four rays and a rim ring at the outer angle, since that
is the edge a user aims. Four rather than one per sample: twenty-four lines from a point is a
starburst that hides the geometry behind it.

**Where it still cannot be drawn, the effect says so rather than the scene report.** A
`BasicEffect` build has only directional lights, so both punctual kinds are flattened; that is now
`studioLightCapabilityIssues`, beside the material one, keyed on the effect's own name and drawn on
the light in the Inspector. It replaced a `validateScene` rule that said the same thing
unconditionally — wrong once because CNA can draw a cone, and wrong again because a document report
cannot know which effect a build uses.

**Verification.** `tests/SceneTests.cpp` —
`APointLightIsSentWholeAndTheRunnersUpAreStillApproximated` (whole into the punctual slot, *not*
also a directional one, colour undimmed by distance, out of range means unlit, and the loser of two
lamps still approximated and still aimed at what it lights),
`ASpotLightCarriesItsConeToTheEffect` (both angles, its own axis rather than one aimed at the
target, and a directional light never reaching the slot), and
`ASpotLightsConeIsReportedByTheEffectRatherThanByTheSceneReport`.
`tests/StudioDetailsPanelTests.cpp` — the greyed rows per kind, and
`ALightSaysWhatTheBuildsEffectCannotDrawAboutIt`. The model pass's call is a device call, so it is
held by the wiring scan that already covers the rest of `EffectLighting`.

Checked by causing each: the punctual slot never filled, the light counted twice, its colour
pre-dimmed by falloff, the cone angles dropped, the model pass not applying it, and the capability
report going silent all fail by name.

### `STUDIO-20008` — No light type is offered that the runtime cannot render

**Acceptance.** Where the editor offers a light this renderer cannot draw faithfully, it says so
on the light, in the report a user already reads — not in a header.

**Closed by making it true rather than by disclosing it.** This row was completed once as a pair of
warnings, on the belief that CNA could not draw a point light properly or a spot light's cone at
all. It can, and `STUDIO-20003` now does: every kind `CNA.Light` offers is a kind the default
effect renders. Nothing is left to warn about on a `PbrEffect` build, and the rule that used to
warn unconditionally is gone.

What remains is a build drawing through `BasicEffect`, which has only directional lights — and
that is reported by `studioLightCapabilityIssues`, keyed on the effect's own name, rather than by
a document rule that cannot know which effect is in use. The `more-directional-lights-than-slots`
rule stays exactly as it was: three slots is a real ceiling on every effect, and a fourth
directional light genuinely reaches nothing.

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

### `STUDIO-20005` — Sky and environment map authoring

**Acceptance.** A scene can name a sky, the viewport draws it, and where the build cannot light
from it the editor says so rather than leaving the difference to be found.

**✅ Done.** `STUDIO-10010` made an environment map an asset that could be created, edited and
costed; this is the row with a consumer. The scene names one, `CNA::Graphics::Skybox` draws it, and
`EnvironmentProcessor` turns the panorama into the cube behind the scene and — where anything can
sample them — the three products of the split sum.

**Five ways for the chain to end in no sky, and the editor names which.** A scene references an
environment map, the environment map references a panorama, and the panorama is a texture: three
links, each of which can be absent, deleted, unreadable or the wrong kind. Every break looks
*identical* in the viewport, so `planSceneSky` returns which one it is and the Inspector prints a
sentence a user can act on. The walk is CNA-free and tested against documents, and the provider it
takes is four plain fields rather than the document type — `cna-studio-scene` links
`cna-studio-core` and nothing else, and inverting that for one struct would have been the wrong
trade.

**Drawing and lighting are two answers, because CNA gives them separately** — `Skybox::draw` puts
the cube on screen and `setImageBasedLightEXT` makes it light things — and each is worth wanting
alone: a backdrop that must not tint the scene, or image-based lighting in a room whose windows
show no sky. Two switches on the scene, two booleans on the plan, and a scene with both off reports
`problem == None`, because what a *problem* means is that the editor cannot do what was asked, not
that the user chose less than the maximum.

**And here the two effects are genuinely not interchangeable, which is the finding this row turned
on.** `STUDIO-20006` established that `BasicEffect` implements `IShadowReceiverEXT` exactly as
`PbrEffect` does, so shadows and punctual lights reach both. **Image-based lighting does not.**
`setImageBasedLightEXT` is declared on `PbrEffect` and `SkinnedPbrEffect` and on nothing else — it
is not on that interface — and `kPreferPbrEffect` is **false** because `PbrEffect` draws nothing on
the one backend this repository can photograph (`G-05`). So a sky authored in the build every user
runs is *drawn* and lights nothing.

That was checked before the code was written rather than discovered after, which is the G-13/G-14
lesson applied in the other direction: it is not enough for an API to exist, it has to be reachable
on the path that actually runs. The consequences are three:

1. The cube is generated whenever the scene asks for a sky, because drawing one needs no effect
   support at all.
2. The irradiance, prefiltered specular and BRDF table are generated **only when the effect can
   take them**. Convolving a hemisphere per texel for a light nothing can sample would be a second
   of CPU work per sky for no pixels.
3. `studioEnvironmentCapabilityIssues` says so in the scene Inspector, beside the settings that
   caused it — the same disclosure `STUDIO-20003` uses for a point light on `BasicEffect`, and for
   the same reason: a feature that silently does nothing is worse than one that refuses.

The architecture guard that refuses `pbr->set…EXT(` gained its first exemption here, and the
exemption is the fact rather than a hole: `setImageBasedLightEXT` is named in the allow-list with
the reason, so the next person reaching for the concrete pointer still has to justify it.

**Everything is cached against the environment map's asset id *and* the panorama's.** Keyed on the
first alone, repointing a sky at a different image would serve the old cube under the new name —
which looks like a sky that refuses to change, and is the kind of defect that gets blamed on the
importer.

**Verification.** `tests/SceneTests.cpp` —
`EveryBrokenLinkInTheSkysChainIsNamedRatherThanReportedAsNoSky` (all five states, and that four of
them have a sentence while "no environment map" deliberately does not),
`ASkyThatIsDeliberatelySwitchedOffIsNotAProblem` (including degrees in, radians out, and a negative
intensity clamped), `TheModelBatchCarriesTheScenesSky` (and that a host which forgot the provider
reports `EnvironmentMapMissing` rather than looking like a scene with no sky),
`TheScenesSkyRoundTripsAndAnAbsentOneWritesNothing`, and
`ASkyOnABasicEffectBuildIsDrawnAndSaysItLightsNothing`.

The device half is a source scan honest about being one, in
`TheShadowPlanTheBatchCarriesIsWhatTheDeviceHalfRenders` — which now covers the sky as well as the
shadow, including the ordering: the sky is drawn **inside** the scene's target and **before** the
models, which is the opposite constraint to the shadow map's. `Skybox` draws a fullscreen triangle
with no depth configuration, so drawn afterwards it would erase them.

Checked by causing each: the chain collapsing three problems into one, the batch never carrying the
sky, the sky drawn after the models, and a `BasicEffect` build that stops saying it lights nothing
— all four fail by name.

**What this row is not.** There is no procedural sky: CNA ships `AtmosphericSky` and Studio does not
drive it, so a project wanting a time of day authors a panorama. There is no per-scene override of
the environment map's *processing* — the sizes and sample counts belong to the asset, which is what
lets two scenes share one sky without processing it twice. And the sky is not in the game view's
`renderGame3D` path by a separate route: it is on the batch, so both views get it from the same
place and cannot disagree.

### `STUDIO-20006` — Shadow configuration

**Acceptance.** A user is not offered shadow settings that nothing honours — or, where the settings
already exist and are carried through to a game, they are told plainly what the editor draws.

**✅ Done. The viewport draws shadows, and `castShadows` / `receiveShadows` finally decide
something.** Both have been editable since Phase 1, both default to true, both are serialised, and
until this row neither changed a pixel. A user has been able to turn a model's shadow off for the
whole of the editor's existence and watch nothing happen.

**The row was reopened once, because the CNA gap it rested on was filed in error.** The original
entry said neither effect takes a shadow map and there is no seam for one that could. `PbrEffect`
implements `IShadowReceiverEXT` — `setShadowMapEXT`, `setLightViewProjectionEXT`,
`setShadowsEnabledEXT`, a depth bias, a filter radius — and the CNAEXT engine layer ships
`CNA::Graphics::ShadowMap`, which renders from a directional light, fits an orthographic volume to
the scene bounds, hands back rigid and skinned caster effects, and reports `isSupported()` where a
renderer cannot manage it. `G-14` is withdrawn, and the lesson is recorded there: **check what the
API offers before writing down what it lacks.**

**And the same mistake, found a second time while building the fix.** `applyLighting` reached for
`PbrEffect` to send the punctual light `STUDIO-20003` added, because `PbrEffect` was the header
that had been read. `BasicEffect` implements `IShadowReceiverEXT` too — and `kPreferPbrEffect` is
**false**, so `BasicEffect` is the path every build actually takes. The feature had shipped
switched off on the only path anybody runs, and nothing said so. Both the punctual light and the
shadow map now go through `Impl::shadowReceiver()`, and a guard test refuses any `pbr->set…EXT(`
in the file by name.

**The decision is a CNA-free function and the device call is wiring**, as everywhere else:

- `SceneShadows.hpp` / `.cpp` compute a `SceneShadowPlan` — which light, which draws, and the world
  bounds to fit the projection to. The brightest enabled **directional** light wins, by the same
  rule and for the same reason `computeEffectLighting` fills its three slots.
- The bounds are the **casters'**, not the scene's. A volume twice the size it needs is a quarter
  of the effective resolution, so a distant skybox entity would otherwise halve the shadow quality
  of everything a user can see.
- The plan is a field of `SceneModelBatch`, filled by `buildSceneModelBatch`, **not** recomputed by
  the viewport. Two places working out which entity casts is two answers that can disagree, and
  this way a test asserts that a *document* produces a shadow plan with no device in the room.
- `CnaModelPass::renderShadowMap` takes the batch alone and reads `batch.shadows`. A plan passed
  beside a batch is a plan a caller can get out of step with it.

**Three device facts that are not obvious and are why the pass is shaped as it is.**

1. **`ShadowMap::end` restores the back buffer**, not whatever was bound before it. So the pass
   runs *before* `CnaSceneRenderer` binds the scene's target. Called after, the rest of the frame
   would draw into the window — a defect whose symptom is the editor's chrome flickering, nowhere
   near the shadow code. A guard test compares the two positions in the source.
2. **Generating a map and sampling one are different capabilities.** `ShadowMap::isSupported()`
   answers the first; `GraphicsDevice::SupportsShadowSamplingEXT()` answers the second, and CNA's
   own shadow example checks both because on Vulkan the first is true and the second is false —
   where applying a shadow-sampling effect does not draw an unshadowed picture, it crashes
   mid-draw. A refusal is latched, because `ShadowMap`'s constructor allocates a target of up to
   2048 square *before* `isSupported` can be asked.
3. **Casters are drawn with `CullMode::None`.** The scene pass culls counter-clockwise because the
   *camera's* projection mirrors Y; the light's view-projection is CNA's own and mirrors nothing,
   so that reasoning does not carry over — and a caster that is a single unclosed surface, which
   plenty of imported geometry is, casts nothing when either side is culled.

**The world matrix goes up through `uWorld`.** `getCasterEffect()` returns a raw `ShaderEffect`
precisely so an app with its own transforms can place its casters, and `applyCaster()` resets that
uniform to identity — so the world is uploaded *after* it, not before. This is the documented
pattern; CNA's own `cnaext_shadowmap_test` uses it.

**The validation rule narrowed rather than disappeared.** `shadows-not-rendered` said the editor
drew no shadows, which stopped being true. What survived is `shadows-need-a-directional-light`: the
pass renders from a sun, and a room lit only by lamps has casters, receivers and no shadow. CNA
ships `CubeShadowMap` and `SpotShadowMap` for a point light's cube and a spot's frustum; Studio
does not drive them yet, and that is a Studio row rather than a CNA gap. Still once for the scene
rather than once per model — the flags default to on, and a per-entity rule would fire on every
model in every project forever.

**Verification.** `tests/SceneTests.cpp` — `TheShadowPlanFollowsTheCastAndReceiveFlags` (both flags
read off `buildSceneModelBatch(...).shadows`, so the wiring is covered as well as the arithmetic;
unticking Cast Shadows empties the map, unticking Receive Shadows does not),
`TheShadowPlanPicksTheBrightestDirectionalLightAndIgnoresTheOthers` (a point light plans nothing, a
dim sun does, a brighter one takes over, the direction is unit length), and
`AShadowWantedFromALampIsReportedBecauseThePassNeedsASun`.

The device half cannot be asserted headlessly, so it is guarded by a source scan honest about being
one: `TheShadowPlanTheBatchCarriesIsWhatTheDeviceHalfRenders` in
`tests/ArchitectureGuardTests.cpp`, covering generation, attachment, the `pbr->set…EXT(` refusal,
and the ordering against the render target bind.

Checked by causing each: the builder not attaching the plan, the renderer never calling the pass,
the pass called after the target bind, and an EXT setter reached through `PbrEffect` — all four
fail by name, as does the validation rule losing its "and no sun" condition.

**What this row is not.** There is no shadow *quality* setting: the map is Medium (1024 square),
which is the size CNA's default depth bias is documented as tuned for, and a control that traded
acne against peter-panning on a user's behalf without offering them the choice would be worse than
the constant. When a setting arrives it belongs in the project or the scene environment rather than
on every light, and `kShadowQuality` is the one line it replaces. There are no cascades, no cube
or spot shadows, and no shadow from a blended surface — the map stores one distance per texel and
has nowhere to put "half blocked", so a window casts a solid shadow rather than none.

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

