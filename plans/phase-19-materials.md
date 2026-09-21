# Phase 19 — Materials

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-19001` … `STUDIO-19999` and are never reused.

**Purpose.** Make Studio the primary authoring environment for CNA's modern rendering.

**Exit criteria.** A property-based material editor good enough that a node graph is an addition rather than a rescue.

**Progress:** 2 of 9 complete `██░░░░░░░░░░`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-19001` | PBR material asset model | ✅ | — |
| `STUDIO-19002` | Texture slots: base colour, normal, roughness, metalness, emissive, occlusion | ⬜ | `STUDIO-19001` |
| `STUDIO-19003` | Scalar and vector material parameters | ⬜ | `STUDIO-19001` |
| `STUDIO-19004` | Transparency modes | ⬜ | `STUDIO-19001` |
| `STUDIO-19005` | Material instances and parameter overrides | ⬜ | `STUDIO-19001` |
| `STUDIO-19006` | Live material preview in the viewport | ⬜ | `STUDIO-11011` |
| `STUDIO-19007` | Material preview thumbnail rendering | ⬜ | `STUDIO-09003` |
| `STUDIO-19008` | Renderer capability diagnostics for materials | ⬜ | `STUDIO-02021` |
| `STUDIO-19009` | Material assignment to mesh entities | ✅ | `STUDIO-19001` |

## Acceptance and verification

Tasks whose completion condition is not obvious from the title.

### `STUDIO-19001` — PBR material asset model

**Acceptance.** A material somebody authored is a file with an id: metallic-roughness parameters,
texture references that survive a rename, a version it declares, and one reader that the editor and
the renderer both go through.

**The dependency this row declared was a cycle, and correcting it is part of the task.** It said
`STUDIO-10007`, and `STUDIO-10007` said `STUDIO-19001`. Honoured literally, neither could ever have
been ticked. The direction is one way and it is the other one: `MaterialDocument` is a struct, a
JSON shape and a conversion — it needs no pipeline, no database and no importer to exist, which is
why its row now reads `—`. `STUDIO-10007` needs it, because a pipeline with no document to read
has nothing to recognise. The cycle is recorded rather than quietly deleted because it is the kind
of defect that survives being noticed: nothing in the repository enforces the dependency column, so
the only thing that catches a bad edge is somebody reading it.

**The model is `MeshMaterial`'s fields, deliberately.** An asset that could express more than an
imported material would be a second thing for the model pass to handle, bounded by the same
`BasicEffect`/`PbrEffect` pair either way. So `toMeshMaterial` converts and the pass stays one code
path. What the asset adds is what only an asset needs: a name a person chose, and textures by
`Uuid` rather than by a path relative to some model file.

**Metallic-roughness is stored and Blinn-Phong is derived**, never both. Which effect draws is a
property of the build (gap G-05), so a document that stored two descriptions of its own surface
would render as two different materials depending on the renderer, and the two would drift the
first time somebody edited one of them. `toMeshMaterial` computes the specular pair from
`metallic` and `roughness` at the point of use: a metal reflects its own colour, a dielectric
reflects white.

**Textures are ids, and the conversion leaves the paths empty.** `MeshData`'s materials carry paths
because the glTF importer has no asset database and must not pretend to; this side has one, so it
carries ids, and an id survives the texture being renamed or moved (D-08). Filling a path in during
the conversion would invent a second way for a texture to be named. Resolving an id to a file
belongs to whoever holds the database, which is `StudioContext::makeMaterialProvider`.

**The document carries no id of its own**, which is worth stating because the first version did.
Identity is the `Uuid` in the `.cnaasset` sidecar — that is what D-08 means — and a second id inside
the file would be a second answer free to disagree with the first. The disagreement would surface
as a `ModelRenderer` pointing at a material the database can find and the file denies being.

**Absence is a default; a future version is a refusal.** `loadFromJson` keeps its defaults for every
field that is not there, so a material written by a later editor with three more fields still loads
as the material it mostly is. The single hard failure is a `formatVersion` this build cannot read,
because that is the one case where loading would mean silently discarding somebody's work on the
next save. `loadMaterialDocument` distinguishes three failures — not a material, unreadable, and a
format this build is too old for — because only the last of them means "do not offer to overwrite
it".

**What this row is not.** The four texture slots here are base colour, normal, metallic-roughness
and emissive: there is no occlusion slot, and metalness and roughness share one texture rather than
two. That is `STUDIO-19002`, which stays open. `alpha` is a number and not a transparency *mode*
(`STUDIO-19004`); there are no instances or overrides (`STUDIO-19005`); and nothing assigns a
material to a mesh entity through the Inspector yet (`STUDIO-19009`). This row is the model, and
claiming its neighbours would make the phase's progress bar a worse description of the editor than
no bar at all.

**Verification.** `AMaterialAssetRoundTripsAndDerivesItsBlinnPhongHalf` and
`AMaterialFromAFutureFormatVersionIsRefused` in `tests/ProjectAndAssetTests.cpp` pin the format and
the conversion, including an unset texture being absent from the file rather than written as a nil
id. `tests/StudioMaterialEditorTests.cpp` carries ten cases over the editor and the reader, among
them `TheMaterialProviderAndTheEditorReadTheSameFileTheSameWay` — the one that matters most here,
because two readers of one format is how a renderer and an inspector come to disagree about what a
user is looking at — and `LoadingSaysWhichOfTheThreeFailuresItWas`.
`tests/AssetDependencyTests.cpp` pins a material's texture ids being real references, so renaming a
texture cannot break a material and the dependency view can answer "what uses this".

### `STUDIO-19009` — Material assignment to mesh entities

**Acceptance.** A user can point a `ModelRenderer` at a material asset and see the model drawn with
it, and the slot they point is a slot that means "material" rather than a list of the project.

**The reference has existed since Phase 1 and the slot did not mean anything.**
`CNA.ModelRenderer` declared `material` before there was a material to point it at; ED-403 gave
materials a file, `STUDIO-10007` gave a user a way to make one, and `STUDIO-19001` settled the
model. What was left was the act of assignment, and the act had two holes in it.

**`PropertyDescriptor::assetType` was written by seven components and read by nothing.** Its own
doc comment said "the inspector uses it to refuse a drop of the wrong kind", which is the worst
state a rule can be in: written down, believed, and absent. The material slot listed every sound,
font, texture and model in the project, and took a drop of any of them — writing a reference to a
`.wav` into the scene, which the Problems panel then reported a frame later. That is a long way
round to tell a user their drag went somewhere it should not have.

**So the slot is typed now, at both ends.** The picker offers only the declared kind, and a drop is
checked against the in-flight payload rather than only on release — a target that lights up and
then swallows the drop is worse than one that never lit up, so the ring is drawn in the error
colour while a refusal is being carried. A refused drop is counted and reported rather than
swallowed: a drop that changes nothing and says nothing is indistinguishable from a drop the editor
missed, and the user's next move is to try it again harder.

**Two escapes are deliberate rather than accidental.** An **undeclared** slot takes anything, which
is what most slots in the editor still are and what every one of them was before this. A kind this
build **cannot parse** takes anything too: the field is a string precisely so a plugin can name an
asset kind the editor was never compiled against, and filtering on a name that resolves to nothing
would leave that plugin's slot offering an empty list and refusing every drop — worse than the
unfiltered behaviour it replaced. A declared slot with an unparseable *payload* is the opposite
case and is refused, because the slot promises a kind and an id the database does not know cannot
be checked against it.

**A promise made real on the way past.** `StudioPropertyEditContext`'s header has said since
`STUDIO-07045` that a null context "offers neither and falls back to showing the id". Both picker
loops dereferenced it. Unreachable from all four callers, which all pass a document, so this is a
documented behaviour given an implementation rather than a crash fixed — and it is the same class
of defect as the `assetType` field above, found by looking for more of them.

**What the panel reports, and why it has to.** The picker's list is built inside the editor and
handed to a dropdown, so it leaves no trace a test can read — and "the picker filters" is exactly
the claim that would pass a test of the filtering *rule* while the picker ignored it. So the result
carries how many rows every asset picker offered. `CNA.ModelRenderer` has two typed slots, each
offering `(none)` and the project's one asset of its kind: four rows over the two, against ten
unfiltered.

**Verification.** `tests/StudioDetailsPanelTests.cpp` — the rule itself, including both escapes;
the panel offering four rows rather than ten over a project of four assets; and a sound refused by
a typed slot while the material is taken, undoably.
`tests/SceneTests.cpp` — the assigned material reaching the batch as a whole-model override on
every part, with a material that has not loaded falling back to the model's own rather than drawing
nothing.
Checked by causing each: the picker ignoring the declared kind, the drop taking anything, an
unparseable kind filtering everything out, and the assignment never reaching the batch each fail by
name.

**What this row is not.** There is no per-part assignment *UI*: `ED-410`'s list is a real property
and the grid edits it as a list of structures, which is not the same as picking a material for a
part off the model in the viewport. Nothing here previews the assignment before it is made
(`STUDIO-19006`), and nothing draws a material's thumbnail in the picker (`STUDIO-19007`).

### `STUDIO-19008` — Renderer capability diagnostics for materials

**Acceptance.** A material using a feature the target renderer lacks is reported at authoring time

