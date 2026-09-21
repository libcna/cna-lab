# Phase 20 — Lighting

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-20001` … `STUDIO-20999` and are never reused.

**Purpose.** Lighting authoring that matches what the runtime can actually execute.

**Exit criteria.** The viewport and the game preview agree, and no light type exists in Studio that the runtime cannot render.

**Progress:** 1 of 8 complete `█░░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-20001` | Directional light authoring | ✅ | `STUDIO-19001` |
| `STUDIO-20002` | Point light authoring | ⬜ | `STUDIO-20001` |
| `STUDIO-20003` | Spot light authoring | ⬜ | `STUDIO-20001` |
| `STUDIO-20004` | Ambient and environment lighting | ⬜ | `STUDIO-20001` |
| `STUDIO-20005` | Sky and environment map authoring | ⬜ | `STUDIO-10010` |
| `STUDIO-20006` | Shadow configuration | ⬜ | `STUDIO-20001` |
| `STUDIO-20007` | Viewport lighting matches the game preview as closely as the runtime allows | ⬜ | `STUDIO-20001` |
| `STUDIO-20008` | No light type is offered that the runtime cannot render | ⬜ | `STUDIO-20001` |

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

### `STUDIO-20007` — Viewport lighting matches the game preview as closely as the runtime allows

**Acceptance.** Where an approximation is unavoidable, it is documented rather than hidden

