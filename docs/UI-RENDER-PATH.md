# CNA Studio — the UI render path

> What actually reaches the GPU when the native Studio shell draws a frame, traced from the widget
> call to the CNA graphics call, and the four distinct layers that the phrase "the Studio renderer"
> has so far been used for all four of.
>
> Audit of `STUDIO-04021`. Read with [`ARCHITECTURE.md`](ARCHITECTURE.md) §3, §4 and §5.

## Why this document exists

"CNA Studio requires a renderer capable of the modern CNAEXT Graphics API" has been an architectural
statement in this repository since Phase 2. This document is the result of checking whether it is
true, by reading the calls rather than the names.

**It was not true when this was written, and it is true now.** The audit's finding and the
migration that answered it are both recorded here; §"Where it stands" has the current state. What
follows describes what the audit found.

The native Studio UI reached the GPU through the classic XNA-compatible
pipeline it inherited from the Dear ImGui prototype. Nothing was mis-stated deliberately: the seam
between the UI and the renderer is genuinely toolkit-independent, the native UI genuinely inherited
a working renderer across it, and the phase named "CNAEXT UI renderer" genuinely contains fifteen
completed tasks. What none of that establishes is which CNA API the pixels come out of.

## The four layers

These are four separate things. Conflating any two of them is how a claim like the one above
survives for six phases without anybody being wrong on purpose.

| # | Layer | What it is | Where it lives | Links CNA |
|---|-------|------------|----------------|:---------:|
| 1 | **UI framework** | Widgets, identity, retained state, layout, input routing, focus, theming, docking. Produces geometry; owns no device resource. | `cna-studio-ui-core` | **no** |
| 2 | **UI GPU renderer** | Turns that geometry into graphics calls. Owns textures, buffers and effects. | `cna-studio-viewport` today; `cna-studio-ui-renderer` is where it belongs | yes |
| 3 | **CNA host renderer** | The CNA renderer the `cna-studio` process was compiled against. Draws layers 1–2 and the scene viewport. | chosen by `CNA_GRAPHICS_RENDERER` at Studio's configure time | — |
| 4 | **Game target renderer** | The CNA renderer the *user's game* ships on. Runs in `cna-player` or in the packaged build. | chosen by the project's target profile | — |

Layers 3 and 4 are independent and §3 of `ARCHITECTURE.md` says why at length. Layers 1 and 2 are
what this document adds: the host capability contract is a statement about what layer 2 *asks of*
layer 3, and it can only be enforced once layer 2 is a thing with a name.

## The traced path

```
  StudioShell::describeFrame…            src/ui-core/StudioShell.cpp        │ layer 1
    StudioFrame::…                       src/ui-core/StudioFrame.cpp        │ CNA-free
      StudioDrawList::addQuad/addGlyph…  src/ui-core/StudioDrawList.cpp     │
        ▼ fills
  UiDrawData                             include/CNA/Studio/Ui/UiDrawData.hpp
    ├─ lists[].vertices  : UiVertex  { x y u v rgba }
    ├─ lists[].indices   : uint16
    ├─ lists[].commands  : { texture, clipRect, indexOffset, indexCount, vertexOffset }
    └─ textureRequests   : { Create | Update | Destroy, pixels, region }
        ▼ consumed by
  CnaStudioShellHost::Draw               src/viewport/CnaStudioShellHost.cpp:524   ┐
    CnaUiRenderer::applyTextureRequests   src/viewport/CnaUiRenderer.cpp           │ layer 2
    CnaUiRenderer::renderGeometry         src/viewport/CnaUiRenderer.cpp           ┘
        ▼ calls
  Microsoft::Xna::Framework::Graphics     ← classic XNA 4.0 surface, layer 3
```

### What layer 2 actually calls

Every graphics call the native Studio UI makes today, read out of
`src/viewport/CnaUiRenderer.cpp`:

| Call | Header | Era |
|------|--------|-----|
| `BasicEffect(device)`, `Apply()` | `…/Graphics/BasicEffect.hpp` | XNA 4.0 |
| `setProjectionProperty`, `setWorldProperty`, `setViewProperty` | `IEffectMatrices` | XNA 4.0 |
| `GraphicsDevice::DrawUserIndexedPrimitives` | `…/Graphics/GraphicsDevice.hpp` | XNA 4.0 |
| `Texture2D(device, w, h)`, `SetData`, `SetData(region)` | `…/Graphics/Texture2D.hpp` | XNA 4.0 |
| `BlendState::NonPremultiplied` | `…/Graphics/BlendState.hpp` | XNA 4.0 |
| `DepthStencilState::None` | `…/Graphics/DepthStencilState.hpp` | XNA 4.0 |
| `RasterizerState::CullNone` + `ScissorTestEnable` | `…/Graphics/RasterizerState.hpp` | XNA 4.0 |
| `GraphicsDevice::ScissorRectangle` | `…/Graphics/GraphicsDevice.hpp` | XNA 4.0 |
| `SamplerState::LinearClamp` | `…/Graphics/SamplerState.hpp` | XNA 4.0 |

Nothing from `CNA::Graphics`. No `ShaderEffect`, no `ConstantBuffer`, no `VertexBuffer`, no
`IndexBuffer`, no `ShaderPackageEXT`. `CnaUiRenderer.cpp` includes no CNAEXT header and would
compile against a CNA built with `-DCNA_CNAEXT=OFF`.

That is a *correct* renderer and a *portable* one — it is the reason the native shell drew real text
on a real device the week the shell existed. It is not the modern one.

### Where the claim came from

`CnaUiRenderer.hpp`'s own header comment is the fossil. Its table is headed **"ImGui needs"** and
maps Dear ImGui's requirements onto CNA calls; it was written for `STUDIO-00xxx`-era work, when the
question was whether an ImGui editor could be drawn with a game's API at all. The answer was yes,
using `BasicEffect`, and the file has been right about that ever since.

The native UI then adopted the same seam — deliberately, and it was the right call: a strangler
migration that also rewrote the GPU layer would have had no working state to fall back to. But
`UiDrawData` is the seam's *name*, and inheriting the seam inherited the implementation behind it.
Phase 4 was titled "CNAEXT UI renderer" and its tasks were ticked as the UI gained the behaviours
they name — batching, clipping, atlases, readback — every one of which was implemented on the
classic path. §"Ledger" below reconciles that.

## What "the modern CNAEXT Graphics API" means here

CNA's modern surface, for a UI renderer's purposes, is:

| Facility | Type | Gate |
|----------|------|------|
| Programmable effects from source | `Microsoft::…::Graphics::ShaderEffect` | always compiled; marked `CNAEXT` |
| Typed shader payloads and packages | `CNA::Graphics::ShaderCodeEXT`, `ShaderPackageEXT` | `CNA_CNAEXT` |
| Shared compilation cache | `CNA::Graphics::ShaderEffectFactory` | `CNA_CNAEXT` |
| std140 uniform blocks | `ShaderEffect::DeclareUniformBlockEXT` | `CNA_CNAEXT` |
| Engine-layer revision | `CNA::Graphics::getEngineLayerVersion()` | `CNA_CNAEXT` |
| Capability profile | `GraphicsDevice::GetRendererCapabilityProfileEXT()` | always compiled; marked `CNAEXT` |
| Persistent GPU buffers | `VertexBuffer` / `DynamicVertexBuffer`, `IndexBuffer` / `DynamicIndexBuffer` | XNA 4.0 shapes, used by the modern path |

The renderer-side features a device must classify for the modern path are
`ShaderEffects`, `ShaderEffectSourceExecution` and at least one `ShaderDialect*`.

`BasicEffect` is not on that list, and that is the whole distinction: a renderer can implement
`BasicEffect` — fixed-function or otherwise — without being able to execute a shader anybody wrote.
A Studio host that can only do the former can draw today's UI and can never draw a material preview,
a shader-graph node, a post-process or anything else Phases 19–23 exist for.

## Consequences that were live before this audit

1. **The host contract did not enforce its headline requirement.** `StudioHostEvaluation` carried
   `modernApiAvailable` into its report and no requirement consulted it, so a renderer with no
   modern API at all passed `canHostStudio` on the strength of `ThreeDimensionalPipeline`,
   `DepthStencilBuffer` and a 2048-pixel texture limit. Closed by `STUDIO-02070`.

2. **`modernApiAvailable` was a literal `true`.** Both hosts passed
   `/*modernApiAvailable=*/true` to `captureStudioCapabilitySnapshot`. The field therefore reported
   what the call site asserted rather than what the build had. Closed by `STUDIO-02071`.

3. **Phase 4's ledger described a module that does not exist.** `STUDIO-04001` — create
   `cna-studio-ui-renderer` — is ⬜, and eight of its dependants are ✅. Reconciled by
   `STUDIO-04022`.

## The staged migration

Deleting a working renderer to install an unproven one is how a tool loses a week. The sequence is:

| Stage | State | Task | |
|-------|-------|------|---|
| 0 | `CnaUiRenderer` draws every frame. | — | where the audit found things |
| 1 | The contract enforces the modern requirement; availability is read from the build. | `STUDIO-02070`, `STUDIO-02071` | ✅ |
| 2 | `cna-studio-ui-renderer` exists as a module with `CnaUiRenderer` moved into it unchanged, behind `StudioUiRenderBackend`. | `STUDIO-04001` | ✅ |
| 3 | `StudioModernUiRenderer` implements the same interface over `ShaderEffect` and GPU buffers. | `STUDIO-04023`, `STUDIO-04024` | ✅ |
| 4 | A/B verification: both backends draw the same frame and the captures are compared. | `STUDIO-04025` | ✅ |
| 5 | The native host defaults to the modern backend. | `STUDIO-04026` | ✅ |
| 6 | The classic backend is removed, or retained only where a justification is written down. | `STUDIO-04027` | ✅ |

No stage removed a working path before the one after it had passed.

### Where it stands

**All six stages are done.** On a host that meets the modern profile, `cna-studio` with no flags
draws its entire UI through a `ShaderEffect` Studio compiled from a `ShaderPackageEXT`, over a
`DynamicVertexBuffer` and a `DynamicIndexBuffer` written once per draw list with
`SetDataOptions::Discard`. Verified on `OPENGL4` under Xvfb on Mesa's llvmpipe: `Modern graphics
API: CNA engine layer 18`, `UI renderer: modern`, 1920x1080, 90 draw calls, 16140 triangles, with
text, icons, the composited scene and tinted log rows all drawn by it.

**Stage 6 waited on two things and both resolved in order.** For a while the classic backend was
retained *with the justification written down*: CNA's `SOFTWARE` renderer cannot execute a shader,
and until `STUDIO-04029` it was the only renderer this project's CI could build at all — deleting
the compatibility backend then would have left Studio's only dependency-free automated
configuration unable to start it at all. `STUDIO-04029` put `OPENGL4` under Xvfb into CI as a real,
running leg, which is the condition stage 6 was waiting on. `STUDIO-02074` then retired the
compatibility *profile* one layer up: `StudioHostProfile` has one member, `resolveStudioUiBackend`
no longer has a second backend to fall back to, and a host that cannot meet the modern profile
refuses to start rather than falling back — the SOFTWARE CI leg now builds `cna-studio` and
`cna-player-software` and asserts that exact refusal instead of hosting Studio. `STUDIO-04027`
then deleted `CnaUiRenderer` itself: `include/CNA/Studio/UiRenderer/CnaUiRenderer.hpp` and
`src/ui-renderer/CnaUiRenderer.cpp` are gone, `cna-studio-ui-renderer` keeps its module boundary
with one backend in it instead of two, and every classic GPU call this document traced is deleted
with the file that made it. `studioUiGpuVertexStrideMatches()` — the one piece of that file
genuinely unrelated to which backend draws, a cost-model sanity check `STUDIO-04028`'s benchmark
still needs — moved to `CnaStudioShellHost.cpp`, its only caller.

### The A/B result, which was not the expected one

The task was written expecting a tolerance: a fixed-function path and a fragment shader resolve the
same triangle's edge pixels differently. That is true *across renderers*. Within one renderer it is
not — both backends submit the same geometry, in the same order, with the same blend, sampler and
scissor state, and only the program and the buffer route differ. Neither changes where a triangle
lands or what colour it is.

So `CnaStudioUiRenderBackendsAgree` asserted **byte equality**, which is a far stronger contract than
any threshold would have been, and it held for as long as there were two backends to compare. It also
refused to become a tautology: before comparing anything it checked that each run used the backend it
asked for, because a host that quietly ran the same backend twice is the single most likely way for
an A/B comparison to pass while proving nothing. `STUDIO-02074` retired the test along with the
compatibility backend it compared against — there is no longer a second route through this host for
a frame to take, so there is nothing left for it to disagree with.

### Three defects the migration produced, and the one assertion that caught all three

Two of the three produced exactly the same symptom — a completely blank frame with every draw call
reported — and neither would have been caught by a counts assertion. `--screenshot-min-colors`,
added by `STUDIO-04015` for precisely this class of failure, caught both; the third was a compile
error and is listed because it is the same misunderstanding of the same API:

1. **Uniforms set before the program was bound.** CNA's uniform setters write to the currently bound
   program, and binding is what `Apply()` does. The projection went to whatever program the previous
   caller had left bound.
2. **The projection passed row-major.** CNA hands the array to the graphics API untransposed and
   XNA's `Matrix` is row-major. Every vertex landed outside the clip volume.
3. **A `SetData` overload that takes no byte offset.** `DynamicVertexBuffer::SetData` is
   `(data, startIndex, elementCount, options)`, where `startIndex` indexes the *source array* — XNA's
   five-argument form with a destination byte offset is not among the overloads it declares. This
   one was a compile error, which is the only reason it is not a fourth blank frame.

The first two are worth stating plainly: a UI renderer that is *completely* wrong looks exactly like
a UI renderer that was never called.

**No backend-specific code at any stage.** Studio calls Vulkan, D3D, GL, Metal and WebGPU through
exactly zero lines of its own; `STUDIO-02033`'s guard test fails the build on a direct backend call
and it applies to the new module exactly as it applies to the old one.

---

## The interactive target (`STUDIO-30030`)

Sixty frames a second is **16 667 µs** for everything a frame does. Describing the UI is one of four
things inside that — describing it, submitting the geometry, rendering the viewport's own scene, and
presenting — so it gets a quarter: **4 167 µs**. `--ui-benchmark` prints that budget beside every
scenario, marks each row `ok` or `OVER`, and **exits 3** when any row is over, so a performance job
can act on it rather than relying on somebody reading the table.

**The budget is judged against the cheapest frame of the run** (`us(min)`), not the median — see
[Which statistic the gate reads](#which-statistic-the-gate-reads) below for the measurements that
decided that.

It is a *description* budget, not a frame time. A row at 4 167 µs is not a Studio running at sixty
frames a second; it is a Studio whose UI has spent its entire share.

**Deliberately extreme scenarios get 8 333 µs instead** — the 20 000-entity outliner, the two
thumbnail rows — and promise something different by it. At those sizes Studio does not claim sixty
frames a second, it claims that nothing collapses. Holding them to the interactive budget would have
made them fail on the day they were written, and a budget that is over from birth is one nobody
reads.

**The 100 000-asset browser used to be on that list and is not any more** (`STUDIO-33027`). It was
there because it measured 6 000–7 800 µs, and that turned out to be the benchmark's own fixture
rather than the Content Browser: `fillAssets` built its records through `AssetDatabase::add()`, the
one path that leaves a record unstamped, so the asset watcher's first poll handed all hundred
thousand of them to the reload path and ninety of the hundred and twenty frames measured a mass
re-import. Stamped the way `scan()` stamps what it walks, the three `-100k` rows read **542, 165 and
184 µs** — the windowing they exist to defend had been working the whole time. They now carry the
ordinary interactive budget, with roughly sevenfold headroom. A stress ceiling of 8 333 µs over a
550 µs frame is not a loose gate, it is an absent one.

### Which statistic the gate reads

Absolute microseconds are not comparable between machines, or between the same machine busy and
idle. This project has the receipt several times over: five validation runs of *unchanged* binaries
failed the gate, with every counted column — draw calls, vertices, bytes on the bus — identical byte
for byte across the red and green runs. Only wall-clock moved.

Each row therefore reports three frame times — `us(min)`, `us(med)` and `us(max)` — and its cost as
a multiple of the idle shell, `xbase`. **The gate reads `us(min)`.** That was settled by measuring
rather than by argument: one binary, run on an idle four-core machine and then against four
busy-loops on it, with the counted columns identical across the pair so that anything that moved was
the machine. Over the sixteen non-baseline scenarios:

| statistic | mean drift | worst drift | scenarios pushed over budget |
|-----------|-----------:|------------:|------------------------------|
| `us(min)` | **2.3 %**  | **7.1 %**   | **none**                     |
| `us(med)` | 10.6 %     | 26.1 %      | two                          |
| `xbase`   | 35.3 %     | 47.8 %      | — (not a gate)               |

**`xbase` is the worst of the three**, which is the opposite of what this document used to say: it
claimed the ratio "cancels the machine". It cancels a machine that is uniformly slower by a factor.
Contention is not that — it adds a roughly *constant* cost per frame, and the baseline is the
smallest number in the table, so it takes the largest relative hit and drags every ratio down with
it. The busy-loops moved the baseline frame 63 % and a 7 800 µs scenario 11 %. The correction is
larger than the error it corrects.

`xbase` stays in the report because the shape of a profile is worth seeing at a glance — an outliner
row at 50× the idle shell and a resize row at 3× say something two columns of microseconds make the
reader work out. It is not a figure to compare across runs, and not what the gate reads.

**What the minimum cannot see**, said plainly: a regression that happens on some frames and not all.
Neither could the median — three dear frames out of a hundred and twenty move neither. `us(max)` is
reported beside them so a stall is at least visible, and it found one immediately: at a hundred
thousand assets the asset watcher's poll cost about half a second and ran twice a second — fixed
under `STUDIO-30031`, which spread the sweep across polls and cut those rows' worst frame from
552 000 µs to 1 336. For the two thumbnail rows the minimum is a frame in which no thumbnail work
happened (322 µs against a 2 949 µs median); what guards those is counted rather than timed — the
generated/shared/cancelled totals printed under each row, and `ThumbnailCacheTests`, which waits on
completions rather than on frames.

This is the same instinct as the test suite's: it counts work done rather than time taken, because a
wall-clock assertion on a shared CI machine fails for reasons that have nothing to do with the code.
The budget gate lives in the benchmark — run deliberately, by somebody who wants a verdict — and not
in `ctest`. Its verdict is `studioUiFrameTimesExceedBudget` in `StudioUiBenchmark.hpp`, in the
library rather than in `Main.cpp`, so the suite can reach it: a gate whose logic lives where no test
can see it is the same fault in a different place.

---

## What the two backends actually cost (`STUDIO-04028`)

"The modern renderer is not slower" was an assumption repeated for four tasks. It is a number now.

`cna-studio --ui-benchmark` runs eight representative frame shapes and reports what each backend
asks the device to do. It needs no CNA, no GPU and no window — every figure is computed from the
`UiDrawData` both backends are handed — which is what makes it runnable on every push rather than
only in the hour-long CNA job.

At 1920×1080, sixty frames per scenario, with the example project open:

| Scenario | µs/frame (median) | draw calls | vertices | classic KB | modern KB | ratio |
|----------|------------------:|-----------:|---------:|-----------:|----------:|------:|
| baseline (shell as it opens) | 1 857 | 19 | 5 938 | 2 659 | 154 | **17.3×** |
| 2 000-entity outliner | 308 960 | 20 | 7 317 | 3 449 | 191 | **18.1×** |
| … the same, scrolling | 308 055 | 20 | 7 755 | 3 656 | 202 | **18.1×** |
| 1 500-asset content grid | 23 291 | 19 | 6 150 | 2 754 | 160 | **17.3×** |
| eight-component Details panel | 2 645 | 20 | 8 202 | 3 865 | 213 | **18.2×** |
| a keystroke a frame | 2 091 | 19 | 7 686 | 3 442 | 200 | **17.2×** |
| atlas growth | 5 240 | 20 | 10 169 | 4 795 | 266 | **18.0×** |
| resize every frame | 11 235 | 20 | 6 638 | 3 129 | 173 | **18.1×** |

**The classic backend puts seventeen to eighteen times as much geometry on the bus, per frame, on
every shape of frame measured.** The reason is structural rather than incidental:
`DrawUserIndexedPrimitives` takes *user arrays*, so the driver copies them on every call — and the
array a command is given runs from its base vertex to the end of its list, which in any list the
toolkit has not had to split past 65 535 vertices is the whole array. Nineteen draw calls over one
list of six thousand vertices is that list copied nineteen times. The modern backend uploads it
once into a persistent `DynamicVertexBuffer` and then issues nineteen `DrawIndexedPrimitives`
against it. The ratio is therefore roughly the draw-call count, and it grows as the UI is batched
more finely — which is the direction every added panel pushes it.

**This does not measure GPU time**, and deliberately so: a driver's answer to the same submission
varies by vendor, and the question `STUDIO-04027` has to settle is whether one backend submits more
work than the other, which is a property of Studio rather than of Mesa.

**The cost model is checked against the renderers it models.** It is a second implementation of
both inner loops, so it can be quietly wrong. `CnaStudioShellHost` therefore recomputes it against
the backend's own counters on every frame of every run that has a frame limit — which is every
capture and every CTest smoke test, on both CNA legs of CI, and none of an interactive session's —
and a disagreement fails the process with the field, both numbers and the frame named. Verified by
making the model wrong on purpose: it reported `draw calls: the cost model says 36, the
compatibility backend reported 18 (frame 1)` and exited 6.

**Two numbers, because two things are being counted.** The table is what reaches the bus. Studio
hands CNA 2.3× that, because `VertexPositionColorTexture` is 56 bytes for 20 bytes of data and CNA
repacks it to a 24-byte stream before uploading. That is `docs/CNA-GAPS.md` G-11; it is the same
factor for both backends and so changes the absolute figures above without touching the ratio.
`--ui-benchmark` prints both, per scenario.

**The classic backend had been reporting zero.** `UiRenderStats::geometryBytesUploaded` existed,
the modern backend filled it in, and `CnaUiRenderer` never touched it — so every comparison of the
two on upload bytes was a number against a zero, which reads as "the classic path uploads nothing"
rather than as "nobody counted". Fixed with this task, which is how the 17× was measurable at all.

### What the benchmark found that it was not looking for

Two, both recorded rather than fixed here:

- **The World Outliner is O(n²) in scene size.** 250 entities cost 9 ms a frame, 500 cost 25 ms,
  1 000 cost 84 ms and 2 000 cost 309 ms — four times the cost for twice the entities, which is the
  signature. `SceneDocument::getChildren` scans every entity in the document, and the outliner's
  flatten calls it once per row. Rows are virtualised, so the *drawing* is flat; the walk is not.
  `STUDIO-30013`.
- **The Content Browser stat-s every asset, every frame.** 1 500 assets cost it 21.5 ms against
  1.9 ms for the shell around it — linear in asset count (about 1.8× per doubling), with a large
  constant: `isMissing` is a `std::filesystem::exists()` called once per row, and the missing
  *count* is a second full pass with another `exists()` per asset. About 3 000 synchronous stat
  calls a frame. Attributed by measurement rather than by reading: with both calls stubbed the same
  scenario falls to 8.3 ms, so 61% of the panel's frame cost is the filesystem. `STUDIO-30014`
  measured it; `STUDIO-30015` fixes it, and waits on `STUDIO-30012` because *when* a file deleted
  outside the editor should be noticed is a design decision rather than an optimisation.

Neither is a rendering problem and neither would have been visible in a screenshot, which is the
argument for having a benchmark that reports CPU time beside the counts.
