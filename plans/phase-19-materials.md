# Phase 19 — Materials

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-19001` … `STUDIO-19999` and are never reused.

**Purpose.** Make Studio the primary authoring environment for CNA's modern rendering.

**Exit criteria.** A property-based material editor good enough that a node graph is an addition rather than a rescue.

**Progress:** 1 of 9 complete `█░░░░░░░░░░░`

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
| `STUDIO-19009` | Material assignment to mesh entities | ⬜ | `STUDIO-19001` |

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

### `STUDIO-19008` — Renderer capability diagnostics for materials

**Acceptance.** A material using a feature the target renderer lacks is reported at authoring time

