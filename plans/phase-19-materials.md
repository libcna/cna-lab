# Phase 19 — Materials

> Part of the [CNA Studio master plan](../plan.md). Ids in this phase are `STUDIO-19001` … `STUDIO-19999` and are never reused.

**Purpose.** Make Studio the primary authoring environment for CNA's modern rendering.

**Exit criteria.** A property-based material editor good enough that a node graph is an addition rather than a rescue.

**Progress:** 9 of 9 complete `████████████`

| Id | Task | Status | Depends on |
|----|------|:------:|------------|
| `STUDIO-19001` | PBR material asset model | ✅ | — |
| `STUDIO-19002` | Texture slots: base colour, normal, roughness, metalness, emissive, occlusion | ✅ | `STUDIO-19001` |
| `STUDIO-19003` | Scalar and vector material parameters | ✅ | `STUDIO-19001` |
| `STUDIO-19004` | Transparency modes | ✅ | `STUDIO-19001` |
| `STUDIO-19005` | Material instances and parameter overrides | ✅ | `STUDIO-19001` |
| `STUDIO-19006` | Live material preview in the viewport | ✅ | `STUDIO-11011` |
| `STUDIO-19007` | Material preview thumbnail rendering | ✅ | `STUDIO-09003` |
| `STUDIO-19008` | Renderer capability diagnostics for materials | ✅ | `STUDIO-02021` |
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

### `STUDIO-19002` — Texture slots: base colour, normal, roughness, metalness, emissive, occlusion

**Acceptance.** Every map a `.cnamaterial` can carry has a slot in the editor, is a tracked
reference like any other, and reaches the renderer as a path it can open.

**`MaterialDocument` carried four texture ids and the editor could set none of them.** The editor
drew six values — name, base colour, emissive, metallic, roughness, alpha — and no slots at all, so
the only way to give a material a map was to write the JSON by hand. That is the same state
`STUDIO-10007` found the *file* in, one layer up, and it is why both rows stayed open while the
document, the reader, the provider and the renderer were all finished.

**Five slots rather than six, and the difference is CNA's rather than an omission.** The row names
roughness and metalness separately; glTF packs them into one image — occlusion in R, roughness in
G, metallic in B — and `PbrEffect` takes one metallic-roughness texture. Two slots would be a
picture of a general PBR editor rather than of what this renderer draws, and the second would have
nowhere to go. So the slots are Base Colour, Normal, Metallic-Roughness, Emissive and Occlusion.

**Occlusion is new, and it is a slot of its own rather than a second name for the packed map.**
glTF permits a dedicated occlusion image and `PbrEffect` has `setOcclusionMapProperty`; Studio
simply never carried one. `MaterialDocument::occlusionTexture` and
`MeshMaterial::occlusionTexturePath` are the two ends of it, and the model pass binds it.

**Additive at `formatVersion` 1, and the cost is stated rather than hidden.** Nothing already
written changes meaning and `loadFromJson` keeps its defaults for anything absent, so an older
material loads unchanged. A bump would have made every material this build writes *unreadable* by
the previous one for the sake of one optional texture. What the choice costs: an older Studio
opening a material with an occlusion map ignores the field, and drops it if the user then saves.
That is the trade every additive field in this project makes, and it is worth writing down because
the alternative looks safer and is not.

**Each slot is an ordinary typed asset slot.** `STUDIO-19009` had just made those mean something,
so a material's maps pick, filter and take a drop exactly as a component's reference does, rather
than growing a second kind of picker inside the material editor. Five slots each offering `(none)`
and the project's textures — and not the material itself, which an untyped slot would list.

**What the current build actually draws, said plainly.** `kPreferPbrEffect` is false (CNA gap
G-05), so the model pass runs through `BasicEffect`, which takes a diffuse texture and no others.
The normal, metallic-roughness, emissive and occlusion maps are carried end to end — written,
tracked, resolved to paths, and handed to the effect — and only the first of them reaches the
screen in this build. The editor already says which effect a build got, for exactly this reason.
Recorded, not acted on, is the same bargain `STUDIO-10006` struck for the font settings.

**Verification.** `tests/StudioMaterialEditorTests.cpp` — the occlusion map round-tripping at
version 1 with an older material still loading and an unset slot absent from the file; a texture
dropped on the Base Colour slot reaching the file and undoing; the occlusion slot writing its own
field and leaving the packed map alone; five slots offering ten rows over a project of one texture
rather than fifteen; and every map resolving to the record's path, with a slot pointing at an asset
that has gone resolving to no path rather than a stale one.
`tests/AssetDependencyTests.cpp` — occlusion counted as a reference, so a dependency view cannot
call the texture unused and offer to delete it.
Checked by causing each: occlusion not serialised, the occlusion slot writing the packed field, the
slots left untyped, the provider dropping the occlusion path, and the dependency walk skipping it
each fail by name. The last two produced **no** failure at first, which is why both gates exist:
the provider's path resolution and the material's dependency edges had been covered for the four
older maps and for neither of the new ones.

**What this row is not.** Nothing previews a map before it is assigned or shows a thumbnail in the
picker (`STUDIO-19007`). The glTF importer still reads only the packed occlusion form and warns
about a separate one — reading it belongs to `STUDIO-10004`; what this row makes possible is an
*authored* material naming an occlusion map of its own.

### `STUDIO-19003` — Scalar and vector material parameters

**Acceptance.** A material's numbers are edited as the numbers they are: the normalised ones are
bounded, a drag sets them and the field beside it means the same thing the drag does.

**The vectors were already right and the scalars were text boxes.** Base colour and emissive have
been colour rows since the editor existed; metallic, roughness and alpha were fields a user could
type 400 into, and did not have to — `metallic = 4` is what a slipped decimal point produces, it
is written to the file, and the only symptom is a model that looks wrong.

**`PropertyDescriptor` had the answer and nothing read it.** `minimum` and `maximum` have existed
since descriptors did, and the field's own comment said "the inspector may present a slider instead
of a text field". Eight built-in properties declare a range — a sound's volume and pan, a camera's
field of view, a tile map's columns and rows, a sprite's layer depth — and **both fields were read
by nothing**. This is the third of these found in a row, after `assetType` (`STUDIO-19009`) and the
null-context fallback: a rule written down, believed, and absent.

**So there is a slider, and it is a widget rather than a material feature.** `studioSlider` lives
in `cna-studio-ui-core` beside the button and the checkbox, so every ranged property in the editor
gets one — the material's three, the sound's volume, the camera's field of view — rather than the
material editor growing a control of its own that nothing else can use.

**Clamped, never refused.** A value outside the range arrives from a hand-edited file and from an
older build. A control that refused to show it would leave the user unable to see what is wrong,
let alone fix it, so the thumb pins to whichever end it is past and the value is left alone until
they move it. The *document* still does not enforce the range, which is exactly what
`PropertyDescriptor` promises.

**Clicking the track jumps there.** A slider is a position, and the gesture that says "put it here"
should put it there; a control that stepped towards the click would take five presses to cross its
own track. Nudging belongs to the arrow keys — by the declared step, or by a hundredth of the range
where there is none — with Home and End for the ends.

**The number stays.** A slider is the gesture and the field is the precision: a control that
offered only the first would make "exactly 0.25" something a user has to aim for. The field clamps
to the same range, because a number beside a slider has to mean what the slider means.

**A drag through the slider is one undo entry.** It reports `dragging` exactly as a scrub does, so
the caller merges it — otherwise dragging a volume across the panel is forty presses of Ctrl+Z.

**Verification.** `tests/UiCoreTests.cpp` — a click jumping to its point at both ends and the
middle; an out-of-range value shown rather than corrected, and a drag that leaves the widget
entirely still unable to write outside the range; a step landing on stops and never past the end;
a range with no width drawn and inert; and the arrows nudging by a hundredth with Home and End at
the ends.
`tests/StudioDetailsPanelTests.cpp` — a property declaring a range getting a slider in the real
component grid, clamped, and undoable.
Checked by causing each: the grid dropping the descriptor's range, the slider not clamping, the
track taking no pointer input, and the step rounding not re-clamped each fail by name.

**Two of those breaks passed at first, and the cases were rewritten rather than the breaks
explained away.** The clamp inside the commit is only load-bearing on the *keyboard* path — a
drag's value is already derived from a clamped fraction — so the pointer-only cases never reached
it; the arrow-key case exists for that reason. And a step of 0.3 over 0..1 rounds *down* to 0.9 and
never overshoots, so the step case now uses 0.4, where rounding 1.0 gives 1.2 and the re-clamp is
the only thing between that and a 0..1 property holding 1.2.

**A third thing worth recording.** The first draft of the clamp case pressed at `bounds.right()`
and at points 500 pixels outside the widget. A rectangle's right edge is *outside* it, so those
presses hovered nothing: the case passed by never reaching the slider at all. It drives a real
drag now, which is both the honest gesture and the one that can fail.

### `STUDIO-19004` — Transparency modes

**Acceptance.** A material says how its alpha is meant to be read, the editor lets a user say it,
an imported material keeps what its file said, and the renderer draws the result in the right order
rather than solid.

**Nothing in Studio had a transparency model at all.** `MeshMaterial` carried an `alpha` factor and
the model pass set `BlendState::Opaque` once for the whole batch, so a material with alpha 0.5 was
written, edited, resolved, handed to the effect — and drawn fully solid. The inspector said 0.5 and
the screen said 1.0, with nothing anywhere to explain the difference. That is the one failure a
material can have that looks like a renderer bug.

**glTF's three modes, spelt as glTF spells them.** `Opaque`, `Mask` and `Blend`, because that is
where every imported material's answer comes from and inventing a fourth would be Studio deciding
something the format has already settled. CNA's `PbrEffect` takes exactly these through
`AlphaModeEXT`, so there is no translation to get wrong beyond naming; `MeshAlphaMode` is Studio's
own enumeration so that `cna-studio-core` keeps needing no CNA at all.

**A mode rather than a guess from the alpha factor.** A material whose base-colour *texture* is
partly transparent has an alpha factor of 1 and is still transparent, so deriving the mode from the
factor would draw every one of those solid — which is most of the leaves, decals and fences in any
project.

**The importer read is here rather than in `STUDIO-10004`, and the plan says so.** `cgltf` parses
`alpha_mode` and `alpha_cutoff` and the importer dropped both. Leaving that to the model-import row
would have made this one a feature for hand-authored materials and nothing else, which is not what
"transparency modes" means to anybody with a glTF in their project. `STUDIO-10004` stays ✅; what
changed is one switch in `convertMaterial`.

**Ordering is the whole of the renderer change, and it is a CNA-free function.** Blending is not
commutative with depth: a transparent pane drawn before what is behind it blends against the
background instead, and the result is a window with a hole in it. So `orderSceneModelDraws` returns
two passes — the opaque draws, then the blended ones furthest first — and the model pass walks
them. The opaque pass keeps `DepthStencilState::Default`; the blended one uses **`DepthRead`**,
because a pane must still be hidden by the wall in front of it and must not write a depth that
stops the pane behind it from drawing.

**Sorted per draw, not per part or per triangle, and that is stated rather than implied.** A
fixed-function pass cannot sort triangles; a per-part centroid would cost a walk of every vertex
every frame. An entity's own origin is the answer every editor of this kind gives, and two
transparent panes belonging to one model will sort by the model. A draw appears in *both* passes
when it has parts of both kinds, which is the ordinary case for anything with glass in it.

**Masked parts stay in the opaque pass.** A cut-out is a hard edge: it writes depth, it needs no
ordering, and sorting it would be paying for something it does not use. On the `PbrEffect` path the
cut is the effect's own (`setAlphaModeEXTProperty`); with `kPreferPbrEffect` false (gap G-05) the
build draws through `BasicEffect`, which has no alpha test, so a masked material draws solid there.
Said here rather than discovered: the editor already names the effect a build got, for exactly this
class of difference.

**`applyMaterial` left the renderer.** The pass resolved a part's material itself — the per-part
list, then the model override, then the part's own — and this task needed the same answer *before*
the draw, to decide which pass a part belongs in. The rule moved to `resolveMeshPartMaterial` in
the CNA-free scene module and the copy in the pass went with it. Two copies would have been two
chances to put a part in one pass and draw it with the other's material.

**Additive at `formatVersion` 1**, like the occlusion map, and written only when it is not glTF's
default so a material's diff stays about what changed. A mode this build does not recognise reads
as `Opaque` — glTF's own rule — so a file from a later editor draws solid rather than not at all.

**Verification.** `tests/SceneTests.cpp` — blended draws following the opaque ones and sorted
furthest first, with the panes placed along the camera's own forward vector so the case asserts
"further is drawn first" rather than re-stating the view matrix's sign; a model with both kinds of
part appearing in both passes; a masked part staying opaque; and an assigned material's mode
winning over the model's own.
`tests/StudioMaterialEditorTests.cpp` — the mode round-tripping, absent at the default, spelt as
glTF spells it, unknown values reading as Opaque, reaching `toMeshMaterial`, and the cutoff row
drawn only for a masked material.
`tests/ModelImportTests.cpp` — an imported material keeping `BLEND` and `MASK` with its cutoff,
and the base-colour factor's alpha still carried separately.
Checked by causing each: the blended pass left unsorted, masked parts sorted as blended, the
importer dropping `BLEND`, `toMeshMaterial` dropping the mode, and the cutoff row drawn
unconditionally each fail by name.

**What this row is not.** There is no per-object transparency *sort quality* beyond the per-draw
origin, no order-independent transparency, and no double-sided flag — `PbrEffect` has one and
nothing in Studio sets it, which is the next thing anybody authoring glass will ask for.

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

### `STUDIO-19006` — Live material preview in the viewport

**Acceptance.** Editing a material changes the scene on the next frame, and does not cost a file
read per model to do it.

**Both halves, because either alone is the wrong thing.** The preview *was* live:
`makeMaterialProvider` read the `.cnamaterial` off disk on every call, so an edit was visible
immediately. It was also called once per model entity per **frame**, so a scene of two hundred
models opened and parsed two hundred files sixty times a second — live, and paid for with the
frame it was previewing. Caching it without invalidating is the opposite mistake, and a worse one:
a material the user is editing *while looking at it*, showing the version before their own edit.

**The document cache moved from the Inspector to the context.** It started on
`StudioShellPanels`, which is where the Details panel needed it; the renderer needs the same
documents and needs them per frame. Two caches would be two copies of one file and two chances to
disagree about a material somebody is editing — so there is one, on `StudioContext`, and both read
through it.

**The invalidation went in `announceCommand`, and that is the defect this task found.** The
obvious place is `execute`, and a first pass put it there. `announceCommand`'s own header has said
since it was written that *undo and redo bypass `execute`* — they act on the history directly,
because there is no new command to run. Invalidating in `execute` would therefore have dropped the
cache on a material edit and **not** on its reversal: the file back as it was, and the viewport
still drawing the version the user had just taken back. Every document change passes through
`announceCommand`; nothing else does.

**The case had to be written not to hide it.** Its first draft called `invalidate()` by hand after
the undo, which is what a test does when it is describing the implementation rather than the
behaviour — and it passed with the defect in place. It announces the entry instead, exactly as the
Undo action does.

**Coarse invalidation, deliberately.** Every cached document is dropped rather than the one that
changed. The cost of dropping too much is one reload of what is on screen; the cost of dropping
too little is an editor showing a file it has already overwritten.

**A note for anyone writing a test here.** The cache reloads when the record's *stamp* moves or
when something invalidates it. A test that rewrites an asset file behind the database's back gets
neither, and must say so itself —
`EveryMapAMaterialNamesReachesTheRendererAsAPath` now does. In the editor a command invalidates and
an external edit moves the stamp through the asset watcher, so neither case is reachable by a user.

**Verification.** `AMaterialEditIsVisibleAtOnceAndCostsOneFileReadRatherThanOnePerDraw` in
`tests/StudioMaterialEditorTests.cpp` — ten resolutions costing one file read rather than ten, an
edit through a command visible on the very next resolution with its derived Blinn-Phong half
following, and an undo taking the viewport back with it.
Checked by causing each: the provider reading the file every call, nothing invalidating at all,
and invalidating in `execute` only, each fail by name. The third is the defect itself.

**What this row is not.** There is no dedicated preview *scene* — no floating sphere shown when a
material is selected with nothing in the level using it. `STUDIO-19007`'s thumbnail is what a
material looks like on its own; this row is what it looks like on the models that use it.

### `STUDIO-19007` — Material preview thumbnail rendering

**Acceptance.** A `.cnamaterial` in the Content Browser looks like the material, not like a file
icon.

**Forty materials were forty identical rows.** The thumbnail cache's gate was
`studioCanDecodeImageExtension`, so a material was skipped and told apart only by its name — which
is the one thing about a material a user did not derive from how it looks.

**A sphere, because it shows every parameter this format has except the maps.** Base colour,
roughness, metalness and emission are all legible on a lit ball at 128 pixels, and none of them is
legible on a flat swatch.

**On the CPU, and that is a decision rather than a shortcut.** The thumbnail cache does its work on
a *worker thread*, where there is no graphics device and must not be one; a GPU preview would need
a device, a render target and a pass, and would simply be absent in the headless build. Studio
already rasterises its entire UI in software in that build, so a shaded sphere is well inside what
this project does on a worker — and the preview is then identical in every configuration.

**The maps are not sampled, and the plan says so rather than implying otherwise.** A material whose
appearance is mostly its base-colour texture previews as a plain sphere of its factor colour.
Sampling would mean decoding a second image on the worker and resolving its path through a database
this function deliberately cannot see; what it buys is a better thumbnail for textured materials
and nothing at all for the untextured ones — which are the ones a user is most likely to have
several of and least able to tell apart. It is the obvious next step and it is not this row.

**Transparent corners rather than a background colour.** The browser's card is whatever the theme
says it is; a thumbnail with its own grey corners is a grey square in the light theme and a
different grey square in the dark one. And a `Blend` material is drawn *over* that transparency, so
a glass material previews as a faint sphere — the one property of a material a flat swatch cannot
show at all.

**The shading needed a normalisation term, which the first version did not have, and the case that
found it is the reason it exists.** A dielectric's reflectance is 0.04, so without normalising the
Blinn-Phong lobe the difference between a mirror and a matte surface was about ten levels out of
255: the preview could not show roughness, which is half of what it is for. Worse, it had the sign
of the effect backwards — a sharper highlight spreads the *same* reflectance over fewer pixels, so
a smooth material rendered as a dimmer picture than a rough one.

**The normalisation is softened and capped, and both are preview decisions stated as such.** The
exact factor for a mirror-smooth material is around a hundred, which does not brighten the
highlight so much as saturate every channel across a quarter of the sphere — and a sphere whose
highlight is white to the edges tells a user nothing about its colour, which is what they came to
the thumbnail for. The square root keeps roughness's *ordering* intact while leaving the hue
readable, and the cap holds the sharpest materials to a bright spot rather than a bleached one. A
thumbnail is a picture rather than a render, and this is the one place that difference is spent.

**`loadMaterialFile` was split out of `loadMaterialDocument`** so the worker has a reader that
needs no database: the asset database and the document cache belong to the main thread, and a
worker reaching into either is the crossing that whole class is written to avoid. Still one reader
— the database's half is resolving an id to a path, and it then calls this.

**Verification.** `tests/StudioMaterialEditorTests.cpp` — a sphere with transparent corners, an
opaque centre in the material's own colour, and an antialiased rim; a smooth material's highlight
brighter than a matte one's, a metal's brighter than a dielectric's, and emission lifting the
unlit side; and `Blend` previewing see-through while `Opaque` and `Mask` with the same alpha
factor do not.
`tests/ThumbnailCacheTests.cpp` — a material producing a square thumbnail through the real cache,
not made twice, and an unreadable material becoming a cached failure rather than a parse on every
pump.
Checked by causing each: materials skipped by the cache again, the coverage term dropped, and the
alpha mode ignored each fail by name.

**Two of the breaks needed the cases strengthened first, and that is worth recording.** Removing
the silhouette's early-out changed nothing, because the coverage term already zeroes those pixels;
removing the coverage term changed nothing either, because the early-out already skips them. The
two were redundant *with each other*, so the corner assertions gated the pair and neither alone —
which is why there is now a case for the antialiased rim, a thing only the coverage term can
produce. The early-out is labelled in the source as the optimisation it is.

**And the metal case was wrong twice before it was right.** Its first draft read the blue channel
at the brightest pixel and had the sign backwards — a red metal reflects *more* blue than a
dielectric's near-colourless 0.04, not less — and its second still read a single pixel, whose core
saturates on both. What a metal actually looks like is a far brighter reflection, and that is what
it asserts.

### `STUDIO-19008` — Renderer capability diagnostics for materials

**Acceptance.** A material using a feature the target renderer lacks is reported at authoring time,
by name, with what happens instead.

**This closes a gap the plan kept writing down and the editor never said.** Studio carries five
texture slots and three alpha modes end to end — written, tracked, resolved to paths, handed to the
effect. The build draws through `BasicEffect` (gap G-05), which samples one texture and has no
alpha test, so four of those maps and one of those modes reach everything except the screen.
`STUDIO-19002` and `STUDIO-19004` both recorded that in their acceptance entries, which is the
right place for a decision and the wrong place for a warning: nobody authoring a material is
reading the plan.

**Named per feature rather than as one sentence.** "Some of this material will not draw" is a line
a user cannot act on. "The normal map is not sampled; the surface is drawn with the geometry's own
normals, which is flatter rather than wrong" is one they can — either they fill a different slot,
or they know why the model looks flat and stop looking for a bug that is not there.

**What is deliberately *not* reported is as much of the rule as what is.** The base-colour map
draws, so warning about it would be telling a user to change something that works. `Blend` draws,
because blending is the device's state rather than the effect's, and only `Mask` needs a shader.
The metallic and roughness *factors* still shape the highlight through the Blinn-Phong derivation,
so the metallic-roughness **map** is reported and the numbers beside it are not — and the warning
says which, because "metallic does not work here" would be false.

**An effect this build does not recognise reports nothing.** A headless preview has no device and
no effect name, and a CNA that grows a third effect should make Studio quiet rather than wrong: a
diagnostic that fires on an effect it has never heard of is one people learn to ignore. `"none"`,
the empty string and any future name all report nothing.

**CNA-free, and its own module rather than a branch inside the panel.** `MaterialCapabilities` is a
function from an effect name and a document to a list, so validation can use it later without going
through the Inspector — and so the rule can be asserted without a frame.

**The scroll extent reserves both variable parts at their maximum.** The Mask-only cutoff row is
reserved as though it is always there, and the warnings at five, which is all of them. An extent
that shrank as a user switched alpha modes would move the rows under their pointer, and one that
grew would leave a material's last warning unreachable. A few empty rows at the bottom cost a gap;
the alternative is reading the material a second time every frame purely to count its problems.

**Verification.** `tests/StudioMaterialEditorTests.cpp` — a full material reporting five issues on
`BasicEffect` and none on `PbrEffect`; the base-colour map and `Blend` absent from the list; every
issue carrying a detail longer than its name; and an unrecognised, empty or `"none"` effect
reporting nothing. Then the panel itself: silent with no effect named, silent on `PbrEffect`, three
rows on `BasicEffect` for a material with two of the maps and a mask, and three rows taller for it.
Checked by causing each: warning about a map that does draw, treating an unknown effect as
`BasicEffect`, and the panel computing the issues and printing none each fail by name.

**What this row is not.** The *target* renderer is the one this build has, not a build profile's:
Studio cannot yet tell a user that the material is fine here and will not draw on the console they
are shipping to. That needs a capability table per target profile, which `STUDIO-17xxx` owns and
this row does not pretend to.


### `STUDIO-19005` — Material instances and parameter overrides

**Acceptance.** A material can name another as its parent, state only the parameters it changes,
and follow that parent for the rest — visibly, in the editor and in the viewport, with a way to
take an override back.

**The override set is the file's keys, and this is what `ED-300` asks for rather than a breach of
it.** `ED-300` forbids storing overrides *beside* the values they describe, because two
descriptions of the same fact are free to disagree: a list saying "roughness is overridden" and a
roughness field holding the parent's number is a property that reverts to something the user never
chose. Here a parameter an instance does not state is **absent from its file** — `toJson` writes
`parent`, `name` and the stated keys, and nothing else — so there is no second description to
drift. `loadFromJson` fills `overridden` from `json.contains(key)` over the twelve parameter keys,
which makes it a *reading* of the file rather than a record kept next to one.

**Prefabs answer the same question by comparison, and a material must not.** `ED-300` computes a
prefab instance's overrides by diffing it against its source, because a prefab instance has to be
indistinguishable from a hand-authored entity — an entity that dropped a component would otherwise
be a component the scene cannot express. A material instance has the opposite requirement: it has
to *follow* its parent, and comparison cannot express "inherited, and equal by coincidence". A
variant that happens to share its parent's roughness today would silently stop following it the
first time the parent moved, which is exactly the bug instances exist to prevent. So the rule is
the *same* rule — never two records of one fact — and the two answers differ because the facts do.

**Nil and absent mean different things, for an instance only.** A material of its own writes every
key and an unset texture is omitted, which is what `STUDIO-19001` pinned and what every material
already in a project round-trips through. An instance that overrides a slot *to nothing* — "this
variant has no normal map" — writes the key as an empty string, because omitting it would be
indistinguishable from not overriding it at all and the map would come back from the parent.

**Resolution walks through the document cache at every level.** `AssetDocumentCache::
resolvedMaterial` asks the cache for each material in the chain rather than reading files, so an
edit to a root material is visible in every instance of it on the next frame (`STUDIO-19006`). It
deliberately does **not** cache the resolved form: a cache entry's stamp is its own file's, and a
resolved document depends on files it has no stamp for — the entry would go stale on a parent edit
with nothing able to notice. Resolving a two-level instance therefore costs two cache lookups and
no file reads, which is the right price.

**A loop is refused at the gesture and survived at the reader.** `studioMaterialChainWouldLoop`
runs before the parent is written, so the editor never creates one; the message says "That material
already inherits from this one." rather than reporting a chain the user cannot see.
`resolveMaterialDocument` still carries a visited set *and* a depth limit of 32, because a file
somebody hand-edited is not bound by the editor's rules, and it returns what it resolved as far as
the repeat — a user with a loop needs to see the material while they fix it. The panel says the
chain is circular rather than showing the instance as a material of its own, since those look
identical and only one of them is the user's mistake.

**The marker is a control, not a dot.** An override a user cannot take back is a one-way door: the
nearest thing to "follow the parent again" would be typing the parent's current number in by hand,
which states it just as hard. So the marker is an `Undo` button that reverts, drawn only for an
instance — on a material of its own every parameter is stated and a column of identical markers
would report the one fact that never changes. It is taken from the **label** column rather than the
control one, for the reason `STUDIO-14012`'s reset button was: a marker in the control column moves
every editor sideways, and the cases that type into those editors would find the marker instead.

**Verification.** `tests/StudioMaterialEditorTests.cpp` — the model half:
`AnInstanceWritesOnlyTheParametersItStates` (the keys are the override set, a material of its own
unchanged, a slot cleared to nothing written empty),
`AnInstanceResolvesToItsParentsValuesWithItsOwnOverTheTop` (a three-level chain, the leaf keeping
its own identity, and an edit to the root moving what the leaf does not state) and
`ACircularParentChainIsReportedAndStillDrawsSomething`. Then the editor half:
`EditingOneParameterOfAnInstanceLeavesTheRestInherited` reads the file as *text* — because a
document that states everything and one that states one thing resolve identically, and only the
bytes tell them apart; `TheOverrideMarkerPutsTheParentsValueBack`;
`AMaterialOfItsOwnShowsNoOverrideMarkers`; and
`TheParentSlotRefusesAMaterialThatAlreadyInheritsFromThisOne`, with a non-cycling drop as the
positive control so a slot that ignored every drop could not pass it.

Checked by causing each: a scalar edit writing the resolved document without stating its key, the
marker never reverting, the cycle check removed from the parent slot, the marker's `isInstance`
guard removed, and `resolveMaterialDocument` dropping the leaf's override set all fail by name —
the last in three places, which is what makes it safe for the panel to rely on.

**`materialOverridesShown` is reported because the damage is invisible on the frame it happens.**
An instance whose whole parameter set quietly became overridden draws every value correctly; what
is broken is the *next* edit to its parent, which no longer reaches it. A count says so now.

**What this row is not.** There is no editor gesture for "make this an instance of that" beyond
filling the Parent slot — no "Create Variant" on a material in the Content Browser, and no way to
see a material's children. There is no multi-select revert, and no indication in the Content
Browser that a material is an instance at all. A parent chain is also not *validated* by the
project: a `.cnamaterial` hand-edited into a loop is caught when something resolves it, not when
the project is scanned.
