# Myra-CNA — faithful C++23/CNA port plan

## 1. Decision, scope, and status

**Status:** active implementation. The shared-runtime, provenance, and an
initial retained-mode layout kernel are complete enough for tested partial
widgets; the explicit MML metadata and scalar-codec foundations are also in
place. Renderer, Desktop/input, XML loading, fonts, styles, and advanced
controls remain in progress.

**Product:** `myra-cna` is one standalone C++23 library: a faithful port of the
Myra UI library to C++ for CNA.  It is not a new generic GUI framework, a CNA
core module, a rewrite with a different widget model, or a port of MyraPad.

**Primary source of truth:** the upstream `Myra.FNA.Core` build selection from
MyraUI/Myra revision
`0d79b939310bfe1d00b21803fe15e291caf60aa1` (`2026-08-08`, `build fix`),
checked out for this analysis at `/tmp/myra-upstream`.

**Integration baselines inspected:** initial CNA `ac3aaaeb2`, cna-extended
`2ff3cff`, and sharp-runtime `b797928f`; the current modular integration is
validated against CNA `1bb2145d99ed572dd4eb15009c34e2e5f410fcf0` and sharp-runtime
`54578590b328aa9612fe38bfddca9fd8ca795144`.

### 1.1 In scope

- Every feature that is part of upstream `Myra.FNA.Core`: MML project/style
  loading and saving, all widget families, default assets, input, rich text,
  texture atlases, styles, dialogs, and the property/data-grid facilities.
  The source inventory contains 189 C# files / 35,934 lines; the seven
  `Platform/**` files are explicitly removed by the upstream FNA project and
  therefore are not part of the CNA target's compatibility surface.
- Every upstream test in `src/Myra.Tests` (15 C# files) as a behavioral
  specification, plus its 35 test assets where their licences permit use.
- CNA rendering, input, clipboard, cursor, file-system, and text-input
  integration.
- Direct runtime dependencies on `CNA`, `SHARP_RUNTIME`, and, only where a
  completed feature benefits from it, `CNA_EXTENDED`.
- A CMake library, CTest/GoogleTest suite, executable examples, documentation,
  source-provenance manifest, and complete redistribution notices.

### 1.2 Explicitly out of scope for the library port

- `src/MyraPad`, `src/MyraViewer`, `src/Myra.GdxSkinImport`, and the seven
  `src/Myra/Platform/**` files; the latter are deliberately absent from
  upstream `Myra.FNA.Core.csproj` and would create a second platform-agnostic
  product rather than improve the CNA port.  These are separate
  tools/importers or a distinct upstream target and are not silently included
  in a library port.
- Porting C# sample applications verbatim.  Their UI layouts and interactions
  are test/reference material; selected scenarios become C++ CNA examples.
- A replacement for a .NET CLR, automatic runtime reflection, a C# compiler,
  XNB content-pipeline support, or a MyraPad-equivalent designer.
- Any edit to `../cna`, `../cna-extended`, or `../sharp-runtime` unless it is
  separately requested.

### 1.3 Definition of “faithful port”

Faithful means that public type names, namespace hierarchy, default values,
layout and draw ordering, MML syntax, styles, widget interaction semantics,
errors where practical, and observable behavior follow the selected upstream
revision.  C++ substitutions are allowed only where language/runtime semantics
make a literal translation impossible, and each substitution must be documented
in `docs/cpp-deviations.md` with a test.

Expected namespace/file shape:

```text
include/Myra/Graphics2D/UI/Widget.hpp
src/Myra/Graphics2D/UI/Widget.cpp
include/Myra/MML/LoadContext.hpp
src/Myra/MML/LoadContext.cpp
```

The library target is `MYRA_CNA`, with alias `Myra::CNA`.  `Myra` remains the
source-facing namespace so that the C++ port is visibly and structurally a
port of Myra.  C# properties become the existing CNA/sharp-runtime convention
`getXProperty()` / `setXProperty()`; this is a surface syntax change, not a
behavioral redesign.

### 1.4 Whole-port completion and remaining-effort estimate

Checkpoint: 2026-08-24, after P5-016c. The mechanical backlog count is
**193/337 checked tasks (57.3%)**. The count is useful for
auditing plan state but is
not a parity percentage: small foundation subtasks and large end-to-end
features each count once, while the remaining font/text, Desktop/input,
advanced-widget, style, data/property-grid, asset, and release-parity work is
substantially heavier. The current feature-weighted engineering estimate is
therefore **about 30–35% of the complete port**.

The following ranges estimate focused implementation, review, documentation,
build, and validation time for every currently open task through P10-028. They
are planning ranges rather than a delivery promise.

| Phase | Checked tasks | Estimated focused hours remaining |
| --- | ---: | ---: |
| Phase 0 — legal/assets/harness | 23/26 | 24–80 |
| Phase 1 — shared runtime | 22/23 | 16–32 |
| Phase 2 — CNA graphics | 20/24 | 24–48 |
| Phase 3 — font/text/assets | 4/21 | 180–300 |
| Phase 4 — MML/XML | 31/36 | 40–80 |
| Phase 5 — Widget/Desktop/input | 42/49 | 0–0 |
| Phase 6 — controls/editing | 24/51 | 96–158 |
| Phase 7 — selectors/windows/dialogs | 26/44 | 142–232 |
| Phase 8 — styles/default skin | 0/14 | 140–240 |
| Phase 9 — file/data/property grids | 1/21 | 180–320 |
| Phase 10 — parity/release | 0/28 | 140–240 |
| **Whole remaining technical port** | **193/337 complete** | **982–1,730** |

For scheduling, use **about 1,356 focused hours remaining** as the midpoint,
with **1,000–2,000 hours** as the sensible rounded range. This assumes prompt
human decisions for P3-004 and P0-015b, no newly discovered upstream/CNA
architectural blocker, and continued reuse of the existing tested foundations.
It excludes idle time waiting for decisions or legal review. If P0-015b is
resolved by commissioning an original default skin rather than clearing an
existing one, budget an additional **80–200 specialist art hours** outside the
technical-port estimate. Recalculate this section after each major phase or
when either blocker is resolved.

## 2. Evidence-based architecture assessment

### 2.1 What makes CNA a good fit

The upstream FNA path uses the same conceptual primitives CNA exposes:
`Vector2`, `Point`, `Rectangle`, `Color`, `Texture2D`, `GraphicsDevice`,
`SpriteBatch`, `RasterizerState`, `SamplerState`, `Keyboard`, `Mouse`, and
`TouchPanel`.  CNA also has `TextInputEXT`, `MouseCursor`, and `Clipboard`
extensions, which cover Myra's editable-text and cursor requirements.

`RenderContext` is the rendering spine in upstream Myra.  On the CNA path it
will own/use a CNA `SpriteBatch`, select filtering/state at `Begin`, flush when
the scissor changes, and draw `Texture2D` regions directly.  This is one CNA
integration layer; it does **not** require a renderer implementation per CNA
backend.  Backend correctness is obtained through CNA's `SpriteBatch`.

The primary library path is deliberately direct CNA integration, not the
over-generalised `MyraCpp + MyraCNA` split.  The upstream platform-agnostic
interfaces are still useful reference material, but they are not a second
public product to build first.

### 2.2 Reuse from sibling projects

| Need | Existing capability | Intended use |
| --- | --- | --- |
| .NET-shaped collections, event helpers, strings, streams, filesystem | `sharp-runtime` | Reuse where behavior fits; do not reimplement BCL-like types. |
| XML DOM backed by tinyxml2 | `sharp-runtime::System::Xml` | Hand-write MML/style XML readers/writers over this DOM. |
| `SpriteBatch`, scissor, textures, input, cursors, clipboard, IME text | CNA | Required public runtime dependency. |
| bitmap `.fnt` fonts, texture-backed glyph drawing | `CNA::Extended::BitmapFonts` | Optional reusable path and test oracle; not sufficient by itself for Myra's dynamic TTF default skin. |
| MonoGame.Extended porting and provenance practice | `cna-extended` | Follow the tested CMake/attribution/manual-XML precedents. |

`cna-extended` should not become an unconditional dependency merely because two
upstream Myra files cite MonoGame.Extended.  `InputExtension.cs` and
`RenderContext.Shapes.cs` are small direct translations whose behavior must be
preserved and whose dual attribution must be retained.  A dependency is added
only if a concrete public capability is reused.

### 2.3 Hard constraints discovered

1. **Reflection:** sharp-runtime intentionally treats `System::Type`,
   `Activator`, and reflection as permanent stubs.  Upstream MML's
   `LoadContext`/`SaveContext`, `PropertyGrid`, and `DataGrid` rely on C#
   reflection and attributes.  A project-local, explicit registry is required;
   it must not pretend that `System::Type` reflection works.
2. **Fonts/text:** upstream Myra relies on FontStashSharp for dynamic TTF/OTF
   glyph-atlas generation, rich text, measurement, RTL behavior, cursor
   placement, and selection.  CNA's `SpriteFont` and cna-extended's bitmap
   font module are useful but do not alone provide this behavior.  This is the
   largest technical workstream and must precede a claim of full parity.
3. **Ownership:** C# relies on GC for cycles through Desktop, parents,
   children, style objects, and event subscriptions.  The port must make the
   hierarchy explicit: parents own children; child-to-parent and
   widget-to-desktop links are non-owning; subscriptions that cross ownership
   boundaries carry removable tokens and are removed during detach/destruction.
4. **Assets:** default skin textures and XML are part of expected UI output.
   The bundled `Inter-Regular.ttf` is traced to an exact OFL-1.1 Inter source
   commit, but the VisUI-derived skin includes artwork governed by additional
   NOTICE/icon terms. The font can be packaged later with its full OFL notice;
   the current skin remains blocked by `needs_human` P0-015b.
5. **Text input:** direct key-to-character conversion is only a fallback.
   Real text entry must subscribe to CNA `TextInputEXT` so UTF-8/IME input is
   not reduced to an English keyboard mapping.

## 3. Non-negotiable compatibility design

### 3.1 Object, collection, and event model

- Public polymorphic widgets are held as `std::shared_ptr<Widget>` where the
  upstream API exposes a long-lived mutable collection.  Containers own their
  child pointers; `parent_` and `desktop_` are checked non-owning pointers.
- C# `ObservableCollection<T>` maps first to
  `System::Collections::ObjectModel::ObservableCollection<T>` when it supplies
  the needed behavior.  Any missing mutation notification behavior is wrapped
  in a Myra-local adapter with regression tests, not forked into sharp-runtime.
- C# events use `System::MulticastAction` or the matching sharp-runtime event
  type.  A stored subscription token is mandatory for subscriptions removed on
  reparenting, closing a window, or destruction.
- `std::optional<T>` represents nullable value types; non-owning pointers
  represent nullable references; `std::unique_ptr<T>` represents exclusive
  internal ownership.  No ownership cycle may be introduced to imitate GC.

### 3.2 Explicit Myra metadata registry

`Myra::MML::TypeRegistry` will be a deliberately small, compile-time-authored
replacement for the reflection subset Myra actually consumes.  For every
registered type it records:

- MML/XML type name and factory;
- base-type/category information needed by MML and `PropertyGrid`;
- property name, C++ getter/setter adapters, value codec, default value, and
  read/write/visibility flags;
- the semantics represented upstream by `Content`, `XmlName`, `SkipLoad`,
  `SkipSave`, `StylePropertyPath`, `Range`, `FilePath`, and
  `DesignerFolded` attributes;
- child/content collection adapters and attached-property accessors.

Registration is explicit near the ported type (or in a generated-looking
`RegisterMyraTypes.cpp` table) and is tested against every supported MML
element/property.  It is not a new general reflection subsystem and does not
require changes to sharp-runtime.

### 3.3 Text interface boundary

Before a widget renders text, it sees a Myra-owned abstraction with operations
for load/create font, glyph caching, measure, wrap, hit test, draw rich text,
selection rectangles, line navigation, RTL, and disposal.  This keeps the
faithful widget port close to FontStashSharp call sites while containing the
unavoidable C++ implementation choice.

The decision gate is:

- first verify FontStashSharp's licence, public behavior, and rendering model;
- compare a narrow direct C++ port of its required subset with an existing
  C/C++ implementation only if its licence and behavior are acceptable;
- obtain explicit approval before adding a new third-party dependency;
- preserve FontStashSharp attribution if any of its implementation is ported;
- prove the selected implementation with Myra text/reference tests before
  porting advanced text widgets.

### 3.4 Build and platform policy

- CMake 3.20+, C++23, warnings as errors (`-Wall -Wextra -Werror` or
  `/W4 /WX`), `MYRA_CNA_BUILD_TESTS`, `MYRA_CNA_BUILD_EXAMPLES`, and a
  documented `MYRA_CNA_LINK_CNA` sibling fallback.
- Follow cna-extended's three modes: use parent-provided `CNA` target, opt-in
  sibling `add_subdirectory(../cna)`, or compile-check headers-only mode where
  possible.  Do not introduce FetchContent/submodules for CNA without a new
  decision.
- Every build command uses at most three workers:
  `cmake --build <build-dir> --parallel 3`.
- Initial correctness backends: `SOFTWARE`/`HEADLESS` for non-visual tests and
  `SDL_RENDERER` for real 2D integration.  Expand only after this baseline is
  green; the final integration matrix includes CNA's practical 2D backends.

## 4. Licensing, attribution, and provenance policy

The upstream Myra root licence is MIT, copyright `(c) 2017-2020 The Myra Team`.
Upstream also ships MIT notices for MonoGame.Extended (Dylan Wilson, 2015) and
TextCopy (Simon Cropp, 2018).  This project must preserve all applicable
notices.  A root MIT licence for original Myra-CNA work does not replace them.

Before any ported source is accepted, all of the following must exist:

1. `LICENSE` — Myra-CNA's MIT licence for original work.
2. `NOTICE.md` — full Myra MIT text, the exact pinned upstream URL/revision,
   an explicit “independent, unofficial C++ port for CNA” statement, and full
   notices for every direct upstream lineage.
3. `THIRD_PARTY_NOTICES.md` — complete text/provenance for Myra, MonoGame.
   Extended-derived files, TextCopy-derived files, FontStashSharp if used,
   fonts/skin assets, and every future third-party dependency.
4. `UPSTREAM_MANIFEST.md` — one row for every ported source or copied asset:
   destination, upstream path, pinned commit, upstream licence, extra lineage,
   port status, and test(s) that exercise it.
5. A header in every direct source translation, for example:

```cpp
// SPDX-License-Identifier: MIT
// Copyright (c) Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
```

Files based on `InputExtension.cs` or `RenderContext.Shapes.cs` additionally
name MonoGame.Extended and point to its full notice.  Files based on
`TextCopy/*` additionally name TextCopy.  The precise upstream header is
preserved if it names another original author.  The same audit applies to
assets; an unresolved asset licence blocks copying that asset, not the rest of
the codebase.

## 5. Complete implementation backlog

Checkboxes are intentionally granular.  A task is complete only when its
source provenance is recorded, documentation is updated, the relevant tests
are green, and the build uses no more than three parallel jobs.

### Phase 0 — repository, legal baseline, and test harness

- [x] P0-001 Create CMake project, `MYRA_CNA` static library, and `Myra::CNA` alias.
- [x] P0-002 Add `include/`, `src/`, `tests/`, `examples/`, `assets/`, `docs/`, and `cmake/` layout.
- [x] P0-003 Add C++23, warning-as-error, formatting, and include-what-you-use policy. All project-owned targets share C++23/no-extension and strict warning settings; `.clang-format`, conditional format/check targets, and an opt-in fail-fast IWYU audit are documented in `DEVELOPING.md`. Ordinary consumers do not require either developer executable.
- [x] P0-004 Implement parent-CNA / opt-in sibling-CNA / headers-only CMake modes.
- [x] P0-005 Add `MYRA_CNA_BUILD_TESTS` and GoogleTest integration without building unrelated sibling tests.
- [x] P0-006 Add `MYRA_CNA_BUILD_EXAMPLES` and a minimal executable target.
- [x] P0-007a Document `--parallel 3` build and test commands in README.
- [ ] P0-007 Add the same worker limit to CI scripts when CI is introduced by P10-022.
- [x] P0-008 Create root `LICENSE` for original Myra-CNA work.
- [x] P0-009 Create `NOTICE.md` with complete Myra MIT notice and official source URL/revision.
- [x] P0-010 Create `THIRD_PARTY_NOTICES.md` with Myra, MonoGame.Extended, and TextCopy notices.
- [x] P0-011 Create `UPSTREAM_MANIFEST.md` schema and source-header template.
- [x] P0-012 Add automated test/lint that every ported `.hpp`/`.cpp` has provenance metadata.
- [x] P0-013 Add automated test/lint that every source header points to a manifest row.
- [x] P0-014 Audit all 189 upstream `src/Myra` production files for non-Myra lineage or copied headers. `docs/source-lineage-audit.md` records the complete default classification, ten exceptions, FNA selection, exact inventory hash, licences, and do-not-translate decisions.
- [x] P0-015 Audit the default skin and `Inter-Regular.ttf` redistribution provenance. `docs/default-assets-audit.md` pins all seven resource hashes, traces Inter to its embedded OFL-1.1 source commit, and proves that 202/207 raw raster assets match VisUI.
- [ ] **needs_human — P0-015b:** Choose and document either authoritative permission/legal clearance for the VisUI-derived atlas (including all Apache/NOTICE/icon obligations) or replacement with original/clearly licensed skin artwork. Do not copy the atlas or its visual sources before this decision.
- [x] P0-016 Audit all 35 Myra.Tests assets before copying them. `docs/test-assets-audit.md` records every path, size, SHA-256, source/licence finding, and copy/defer/replace disposition under a reproducible inventory hash.
- [ ] P0-016a Before porting upstream asset-loading/stylesheet tests, create project-owned replacements for the MonoGame logo, Arial BMFont, Commodore64 bundle, and unmatched DroidSans binary; rewrite dependent MIT XML paths. Keep the VisUI-derived default skin blocked, and add the complete Apache-2.0 notice before copying the provenance-cleared libGDX bundle.
- [x] P0-017 Document named-source and asset exclusions until their licences are resolved.
- [x] P0-018 Add `docs/cpp-deviations.md` with an empty, reviewed deviation-table template.
- [x] P0-019 Add test helper for numeric/rectangle/color comparisons. `ComparisonHelpers.hpp` supplies reusable GoogleTest predicate-format assertions with component-level diagnostics and explicit non-finite/tolerance behavior; helper tests plus Widget/SolidBrush consumers pass in linked and sanitised builds.
- [x] P0-020 Add deterministic temporary asset directory helper. The move-only RAII helper isolates paths by build working directory and current GoogleTest name, rejects traversal/absolute paths, removes stale input before a test, writes nested text assets, and cleans its exact owned tree afterward.
- [x] P0-021 Add a headless test executable and register it in CTest.
- [x] P0-022 Add a CNA SDL_RENDERER smoke executable and register its display requirement. The conditional CTest target creates a real CNA Game/device on X11, draws a Myra `SolidBrush`, verifies inside/outside backbuffer pixels, and exits after one frame; its display environment, labels, resource lock, and timeout are explicit. The full current SDL_RENDERER suite passes 199/199 on Xvfb.
- [x] P0-023 Restore build compatibility with current modular CNA/sharp-runtime checkouts. Header-only mode discovers module include/source trees; linked and sanitised validation drives CNA as the top-level project through `CMAKE_PROJECT_CNA_INCLUDE`. Its driver defers enabling and linking Myra's narrow `SharpRuntime::Xml` component until sharp-runtime has registered modular components; linked test fakes no longer rely on CNA's removed `SDL_Texture` accessor and windowless cursor tests tolerate CNA's explicit platform refusal. The legacy `MYRA_CNA_LINK_CNA` child mode now diagnoses modular checkouts and remains available for compatible older layouts. Validated against CNA `1bb2145d` and sharp-runtime `54578590` without editing either sibling.

### Phase 1 — shared runtime translation layer

- [x] P1-001 Port `Events/CancellableEventArgs.cs`.
- [x] P1-002 Port `Events/CancellableEventArgs{T}.cs`.
- [x] P1-003 Port `Events/EventHandlingStrategy.cs`.
- [x] P1-004 Port `Events/GenericEventArgs.cs`.
- [x] P1-005 Port `Events/MyraEventArgs.cs`.
- [x] P1-006 Port `Events/MyraEventHandler.cs` using sharp-runtime event facilities.
- [x] P1-007 Port `Events/TextDeletedEventArgs.cs`.
- [x] P1-008 Port `Events/ValueChangedEventArgs.cs`.
- [x] P1-009 Port `Events/ValueChangingEventArgs.cs`.
- [x] P1-010 Port `Graphics2D/Thickness.cs`, equality, parsing, and arithmetic.
- [x] P1-011 Port `Graphics2D/Transform.cs` using CNA matrices/vectors.
- [x] P1-012 Port `Utility/ColorHSV.cs` and conversion edge cases.
- [x] P1-013a Port the dependency-free `CrossEngineStuff.MultiplyColor` path directly to CNA `Color::Multiply`.
- [x] P1-013 Complete `Utility/CrossEngineStuff.cs` view-size and texture helpers on the checked P2-001/P2-002 GraphicsDevice lifetime contract, including validated RGBA-region conversion to CNA's typed `Color` upload API.
- [x] P1-014 Port `Utility/Mathematics.cs` and document C++ numeric differences.
- [x] P1-015 Port `Utility/PathUtils.cs` over lexical `std::filesystem` paths.
- [x] P1-016 Port `Utility/Rest.cs` as type-safe generic table helpers, preserving shallow element-copy and column-sort behavior while validating jagged rows before mutation.
- [x] P1-017 Port `Utility/StringUtils.cs`.
- [x] P1-018 Port `Utility/UIUtils.cs` and use its stable upstream Z-index ordering for widget snapshots.
- [x] P1-019 Port `Utility/EventsExtensions.cs` with removable subscriptions.
- [x] P1-020 Resolve internal `Utility/CurrentPlatform.cs` without translation: FileDialog will use CNA `getCurrentPlatform()`/`getCurrentDesktopOS()` and TextBox will use CNA Clipboard, avoiding an unnecessary MonoGame-derived platform shim.
- [x] P1-021 Port `Utility/InputExtension.cs` with MonoGame.Extended dual attribution.
- [ ] P1-022 Translate upstream unit tests for all Phase 1 types.

### Phase 2 — direct CNA environment and graphics primitives

- [x] P2-001 Port the direct CNA Game/GraphicsDevice portion of `MyraEnvironment.cs`; cursor, input delegate, asset, and rendering settings remain in their dedicated tasks below.
- [x] P2-002 Define and test a checked non-owning GraphicsDevice lifetime/initialisation contract, normal CNA disposal/destruction detachment, explicit custom-lifecycle clearing, and diagnostic errors.
- [x] P2-003 Implement all twelve default cursor mappings through CNA `MouseCursor`/`Mouse::SetCursor`, preserving upstream no-op and invalid-enum assignment ordering.
- [x] P2-004 Implement `DisableClipping`, all four debug-frame flags, and event-model settings with upstream defaults.
- [ ] P2-005 Define explicit per-game/per-desktop cleanup in lieu of GC finalization. Per-game cleanup is complete in P2-002; retain this task for the unported Desktop caches/lifecycle.
- [x] P2-006 Port `Graphics2D/IBrush.cs` with a non-owning `RenderContext&` draw contract and C++ white-tint extension helper.
- [x] P2-007 Port `Graphics2D/IImage.cs` as the sized `IBrush` contract.
- [x] P2-008a Port `SolidBrush`'s CNA-color property and exact tint/draw core through the font-independent default white region.
- [ ] P2-008 Complete the string constructor and `ToString()` after P3-004 authorizes the audited `ColorStorage` compatibility implementation and P4-013 supplies it, then select `Stylesheet.Current.WhiteRegion` after the Phase 8 stylesheet core exists.
- [x] P2-009 Port `Graphics2D/TextureAtlases/TextureRegion.cs` with retained CNA texture handles, nullable names, checked relative offsets, and RenderContext drawing.
- [x] P2-010 Port `Graphics2D/TextureAtlases/ColoredRegion.cs` with retained non-null regions and exact upstream per-channel tint truncation.
- [x] P2-011a Port the retained region, tinting, size, equality, and hashing core of `Graphics2D/TextureAtlases/TintedRegion.cs`.
- [ ] P2-011 Complete `TintedRegion.ToString()` after P3-004 authorizes the audited `ColorStorage` compatibility implementation and P4-013 supplies the selected-upstream color-string contract.
- [x] P2-012 Port `Graphics2D/TextureAtlases/NinePatchRegion.cs`, including selected-upstream degenerate-destination positioning and checked C++ geometry.
- [x] P2-013 Port `Graphics2D/TextureAtlases/TextureRegionAtlas.cs` with retained texture/region handles, Myra XML round trips, selected-upstream duplicate/unknown-entry behavior, atlas-region reference splitting, and checked diagnostics.
- [ ] P2-014 Implement texture loading/caching ownership against CNA `Texture2D` as part of P3-016's `AssetManagementBase`-compatible layer; CNA's weak backend cache alone does not preserve upstream AssetManager object identity.
- [x] P2-015 Implement font-independent white-region creation with retained CNA texture handles, stale-texture recovery, and per-game cache cleanup/lifetime tests.
- [x] P2-016 Port the graphics-only `Graphics2D/RenderContext.cs` begin/end/flush and RAII/Dispose state machine; font/rich-text overloads remain with Phase 3.
- [x] P2-017 Map nearest/linear/anisotropic filtering to CNA clamp sampler states and test flush-before-sampler-change ordering.
- [x] P2-018 Map clipping/scissor changes to CNA rasterizer/device state, viewport offsets, `DisableClipping`, and flush ordering.
- [x] P2-019 Port all texture `RenderContext::Draw` overloads with transform, scale, rotation, depth, and opacity forwarding; `DrawString`/rich text remain font-gated Phase 3 work.
- [x] P2-020 Port `RenderContext.Shapes.cs` with MonoGame.Extended dual attribution, exact selected-upstream edge ordering/closed polygons/opacity quirks, and checked C++ numeric geometry.
- [x] P2-021 Test nontrivial sprite source rectangles, rotation, scale, opacity, clipping, and caller-managed nested clip intersection/restoration. Automatic Widget clip traversal remains explicitly in P5-008.
- [x] P2-022 Test the graphics-only RenderContext and dependent primitives under SOFTWARE and HEADLESS, plus SDL_RENDERER on an Xvfb X11 screen; all three linked backend configurations pass their graphics subsets and full Myra-CNA suites.

### Phase 3 — font, rich text, assets, and asset manager

- [x] P3-001 Inventory every FontStashSharp type/method called by upstream `src/Myra`; the active FNA surface and behavioral closure are recorded in `docs/font-audit.md`.
- [x] P3-002 Verify FontStashSharp 1.5.6/Base 1.2.3 provenance and zlib licence, pin exact commits, and preserve the complete notice before translating any implementation.
- [x] P3-003 Compare a direct required-subset port with CNA SpriteFont, cna-extended BitmapFont, FreeType/HarfBuzz, SDL_ttf, and an independent stb-based layout in `docs/font-audit.md`.
- [ ] **needs_human — P3-004:** Select and explicitly approve the font layout/rasterizer sources, untrusted-font policy, and optional-shaping scope before adding any new third-party source or implementing P3-005 onward.
- [ ] P3-005 Define Myra text abstraction without changing upstream widget semantics.
- [ ] P3-006 Implement UTF-8/Unicode iteration and an explicit index-domain/grapheme policy that reconciles the audited upstream mix of UTF-16 code-unit positions and code-point counts.
- [ ] P3-007 Implement dynamic TTF/OTF font loading.
- [ ] P3-008 Implement glyph rasterisation and CPU/GPU atlas allocation.
- [ ] P3-009 Implement atlas growth, invalidation, cache lifetime, and device-loss behavior.
- [ ] P3-010 Implement glyph metrics, kerning, line height, baseline, and measurement.
- [ ] P3-011 Implement rich-text tokenisation/layout used by Label/TextBox.
- [ ] P3-012 Implement wrapping, alignment, clipping, and selection-rectangle layout.
- [ ] P3-013 Implement RTL/bidirectional behavior to the documented selected-upstream level; optional HarfBuzz shaping is off by default upstream and remains a separate P3-004 decision.
- [ ] P3-014 Render glyphs through `RenderContext`/CNA SpriteBatch.
- [ ] P3-015 Test text pixel baselines, measurement, wrapping, missing glyphs, and atlas reuse.
- [ ] P3-016 Port `MyraAssetManagerExtensions.cs`.
- [ ] P3-017 Port `MyraAssetManagerExtensions.Stylesheet.cs`.
- [x] P3-018 Record `MyraAssetManagerExtensions.PlatformAgnostic.cs` as an inactive conditional source in the FNA/CNA manifest; do not create a parallel platform-agnostic API.
- [ ] P3-019 Complete `DefaultAssets.cs`: the font-independent white-region cache is already P2-015; port stylesheet selection only after the style core exists, and do not bind it to the blocked VisUI atlas before P0-015b.
- [ ] P3-020 Package the audited Inter font only after P3-004 selects the font implementation and preserve its full OFL notice; package a default skin only after P0-015b, then verify every path case-sensitively.
- [ ] P3-021 Provide and test a documented no-default-skin configuration while P0-015b remains unresolved; font-independent primitives must continue to work without bundled visual assets.

### Phase 4 — MML metadata and XML foundation

- [x] P4-001 Port `Attributes/ContentAttribute.cs` as registry metadata.
- [x] P4-002 Port `Attributes/DesignerFoldedAttribute.cs` as registry metadata.
- [x] P4-003 Port `Attributes/FilePathAttribute.cs` as registry metadata.
- [x] P4-004 Port `Attributes/RangeAttribute.cs` as registry metadata.
- [x] P4-005 Port `Attributes/SkipLoadAttribute.cs` as registry metadata.
- [x] P4-006 Port `Attributes/SkipSaveAttribute.cs` as registry metadata.
- [x] P4-007 Port `Attributes/StylePropertyPathAttribute.cs` as registry metadata.
- [x] P4-008 Port `Attributes/XmlNameAttribute.cs` as registry metadata.
- [x] P4-009 Port `MML/BaseObject.cs`.
- [x] P4-010 Port `MML/IItemWithId.cs`.
- [x] P4-011 Port `MML/IHasColor.cs` with CNA `Color`.
- [x] P4-012 Design and implement `TypeRegistry`, `TypeDescriptor`, property descriptor, and factory contracts.
- [x] P4-012b Enforce one unambiguous effective inherited property namespace and reject null registry callback owners/factory results.
- [x] P4-013a Implement invariant codecs for primitive and optional values plus finite explicit enum/flags mappings; include audited geometry automatically in CNA-linked builds.
- [ ] P4-013 Implement Color only after P3-004 authorizes the audited `ColorStorage` compatibility source, and add external image/font value handling after their dependency and provenance gates pass.
- [x] P4-014 Implement attached-property descriptors and `AttachedPropertiesRegistry.cs` semantics.
- [x] P4-014b Make global attached-property ID/collection access thread-safe and reject duplicate owner/name declarations.
- [x] P4-015 Port `MML/BaseContext.cs` property classification onto `TypeRegistry` and `ValueCodecRegistry` (the upstream class itself does not access XML).
- [x] P4-016a Port the audited Vector2, Thickness, and Rectangle subset of `MML/TypeSerializers.cs`.
- [ ] P4-016 Complete `MML/TypeSerializers.cs` over explicit codecs, including Color after P3-004 authorizes the audited `ColorStorage` compatibility implementation.
- [x] P4-017a Implement registry-backed object creation, scalar attribute loading, legacy-name mapping, explicit external-asset adapters, and conversion diagnostics.
- [x] P4-017b Implement attached-property attributes and `_`-prefixed `BaseObject` user-data loading through explicit type adapters.
- [x] P4-017 Port `MML/LoadContext.cs` onto explicit scalar, attached, single-object, sequence, dictionary, content, and external-asset adapters.
- [x] P4-017d Roll back all `ObjectsNodes` entries added by a failed root or nested load so reusable C++ contexts cannot retain dangling object pointers.
- [x] P4-017c Make the future Project/load result own its parsed XML document and retain created object handles where their public types permit; this human-selected contract replaces the current caller-owned `XmlDocument` mapping before P4-020 exposes it through `Project`. `LoadedDocument` now owns the non-movable DOM and transferred mapping suffix, while each context-created mapping retains its type-erased owner; low-level `Load` remains explicitly caller-owned (`DEV-042`).
- [x] P4-018a Implement scalar property saving with XML names, explicit external-asset adapters, null/default omission, empty-value overrides, and load/save skip metadata.
- [x] P4-018b Implement non-default attached-property saving with owner-qualified upstream names.
- [x] P4-018 Port `MML/SaveContext.cs`, including default/skip-save, attached, recursive complex/content, namespace-prefix, and external-asset adapter rules.
- [x] P4-019a Add the central registration table for every currently ported MML type/property (`BaseObject`, `Widget`, `ContentControl`, container bases, `Panel`, Grid, stack panels, `Proportion`, `ExportOptions`, and `Project`) with real-type XML round trips.
- [ ] P4-019 Register every Phase 5–9 public MML type/property explicitly.
- [x] P4-020a Port the dependency-safe `ExportOptions` and `Project` root/load/save core. Built-in and caller-supplied registry overloads preserve the upstream XML shape and legacy container names; a loaded Project owns its DOM/mappings without retaining itself, while default Grid/StackPanel proportions are filtered like upstream (`DEV-043`).
- [x] P4-020b Port the dependency-safe internal single-object load/save semantics as public C++ integration helpers. Loads return an owning `LoadedDocument`, resolve legacy widgets and proportion-property tags, and retain the root; saves preserve upstream `skipComplex`, tag override, parent attached-property context, and Project filtering.
- [ ] P4-020 Complete `Graphics2D/UI/Project.cs`: add stylesheet-aware construction and asset-manager integration after their dependencies exist. The earlier “clone/export” wording was stale: selected upstream has no Project clone or code exporter; it only persists `ExportOptions`, while code generation lives in MyraPad.
- [x] P4-021a Add parser-boundary and `LoadContext` tests for malformed XML, ignored unknown scalar attributes, rejected unknown root/complex names, incompatible nested types, duplicate dictionary Id keys, and contextual conversion diagnostics.
- [x] P4-021 Complete Project-level malformed-document and duplicate object-Id validation. Each load mapping now records its exact registered type, and Project load rejects malformed/non-Project documents plus duplicate non-empty, case-sensitive IDs across distinct mapped `BaseObject` instances while preserving missing/empty IDs (`DEV-044`).
- [ ] P4-022 Translate `MMLTests.cs`, `AssetLoadingTests.cs`, and XML round-trip fixtures.

### Phase 5 — Widget, desktop, layout, and input kernel

- [x] P5-001 Port `Graphics2D/UI/Enums.cs`.
- [x] P5-002 Port `Graphics2D/IContent.cs`.
- [x] P5-003 Port `Graphics2D/UI/ILayout.cs`.
- [x] P5-004 Port `Graphics2D/UI/ITransformable.cs`.
- [x] P5-005 Port `Graphics2D/UI/LayoutUtils.cs`.
- [x] P5-006 Port `Graphics2D/UI/InputContext.cs`.
- [x] P5-007 Port `Graphics2D/UI/InputEventsManager.cs`.
- [x] P5-007b Replace the current non-owning input queue with the human-selected lifetime contract: queued widget processors are retained, while the eventual Desktop link remains non-owning. `Queue` now requires a non-null `shared_ptr<IInputEventsProcessor>` and each capture/bubble entry retains it until dispatch or propagation filtering (`DEV-039`).
- [x] P5-008a Implement the layout, transform, property/event, and invalidation kernel of `Widget.cs`.
- [x] P5-008b Preserve measure/arrange invalidations raised reentrantly from virtual callbacks or events, and use stable child snapshots during enabled-state propagation.
- [x] P5-008c Port `EnsureWidgetById` and protected `SuppressInvalidateMeasure` batch-update behavior.
- [x] P5-008d Retain local child snapshots across virtual layout/traversal callbacks and make add/remove/clear callbacks reentrant-lifetime safe.
- [x] P5-008e Port the dependency-free Widget behavior properties/defaults for dragging, recursive mouse cursors, tooltips, modal/pressed state, clipping, and keyboard-focus acceptance, including pressed change/cancellation events and current-type MML metadata.
- [x] P5-008f Port Widget's internally controlled keyboard-focus state, change event, and public got/lost focus callbacks for future Desktop integration.
- [x] P5-008g Port dependency-free `StyleName`, non-owning `DragHandle`, arbitrary `Tag`, and border/background box bounds, including `StyleName` MML round trips.
- [x] P5-008h Port no-argument `Widget.Clone`/`CopyFrom` using the human-selected virtual per-type construction factory, then complete dependent ContentControl/Container copy overrides and custom-widget tests. Clone factories must return a new non-null exact dynamic type, current concrete widgets provide factories, containers deep-clone their owned children, and custom non-default-constructible widgets are covered (`DEV-040`, `DEV-041`).
- [x] P5-008i Port Widget's retained brush visual states, pre/post-render callbacks, background/border rendering, transformed child traversal, opacity, culling, clipping, and exception-safe context restoration (`DEV-045`). Tooltip timing/lifecycle arrived in P5-016c.
- [ ] P5-008 Port `Graphics2D/UI/Widget.cs` properties/defaults/invalidation/render traversal.
- [x] P5-009a Implement explicit child ownership, reparenting, and stable Z-index snapshots from `Widget.Children.cs`.
- [x] P5-009 Port `Graphics2D/UI/Widget.Children.cs` with explicit ownership, reparenting, recursive queries, typed ID lookup, visibility-aware counts, and cycle rejection.
- [x] P5-010a Port the Desktop-independent Widget input hook/event surface needed by controls: virtual mouse/touch/key callbacks and CNA `Keys` event forwarding. Pointer dispatch preserves hook-before-event ordering, while direct key callbacks raise typed events. Hit testing, queued position transitions, wheel targeting, basic hover visuals, double-click timing, Widget-driven cursor routing, generic Widget drag capture, and tooltip timing arrived in P5-010b/P5-010c/P5-010d/P5-019a/P5-016c; keep remaining consumer-specific capture in P5-019 and character-index semantics gated on P3-006.
- [x] P5-010b Port Widget local mouse/touch positions and Desktop hit-test propagation: parent-clipped reverse-Z traversal, capture/bubble-compatible queued enter/leave/move/down/up transitions, transparent-container fall-through, modal blocking, hover visual selection, touch focus, and deepest accepting wheel targeting. Exact retained widget targets keep queued callbacks safe across reentrant tree removal, and `ProcessWidgetInput` exposes the pre-render staging seam until Desktop rendering lands (`DEV-065`). Double-click timing, cursor routing, generic drag capture, context-menu interaction, and tooltip timing/lifecycle arrived in P5-010c/P5-010d/P5-019a/P5-016b/P5-016c; keep consumer-specific capture and character input in their existing tasks.
- [x] P5-010c Port Widget double-click timing and the `MyraEnvironment` interval/radius settings: preserve the selected upstream's 500 ms and two-pixel defaults, strict interval comparison, inclusive per-axis radius, TouchDown-before-TouchDoubleClick order, pair reset, and failed-tap origin replacement. Use a monotonic C++ clock and widened coordinate differences so wall-clock changes and signed overflow cannot corrupt recognition (`DEV-066`).
- [x] P5-010d Port Widget-driven mouse cursor routing: apply the entered widget's cursor before its hook/event, restore the nearest still-hovered ancestor cursor on leave, otherwise restore `DefaultMouseCursorType`, and honor `SetMouseCursorFromWidget`. Linked CNA builds apply all twelve native cursor shapes; deferred-linkage builds validate/store the same enum without a native backend, and automatic routing tolerates a backend that reports cursor shapes unsupported while retaining the selected backing-state order (`DEV-073`).
- [ ] P5-010 Port `Graphics2D/UI/Widget.Input.cs` bubbling/capturing/hover/drag semantics.
- [x] P5-011a Port `ContentControl`'s abstract `IContent` contract and inherited MML content adapter; its former deep-copy dependency is now completed by P5-011/P5-008h.
- [x] P5-011 Port `Graphics2D/UI/ContentControl.cs`, including its deep-copy behavior after Widget cloning exists. Nullable content is preserved safely during cloning (`DEV-041`).
- [x] P5-012a Implement `Container` stretch defaults and its explicit child-ownership facade.
- [x] P5-012 Port `Graphics2D/UI/Container.cs`, including the background-dependent input fall-through contract.
- [x] P5-013 Port `Graphics2D/UI/Layouts/SingleItemLayout.cs`.
- [x] P5-014 Port `Graphics2D/UI/Layouts/StackPanelLayout.cs`.
- [x] P5-015 Port `Graphics2D/UI/Layouts/GridLayout.cs`.
- [x] P5-016a Port the dependency-safe `Desktop` retained-root core: validated observable ownership and cross-parent/Desktop transfer, recursive placed-state propagation, stable root Z-order, bounds/layout and menu discovery, traversal/find/count/modal queries, composed transforms, and cancellable focus changes with forced detach cleanup (`DEV-011`, `DEV-061`, `DEV-062`). The unlinked bootstrap supplies an explicit default-bounds diagnostic (`DEV-063`). Context-menu and tooltip lifecycles arrived in P5-016b/P5-016c; keep rendering/disposal resources, input polling/routing, and style defaults in P5-016/P5-017/P5-018/P8-002.
- [x] P5-016b Port the `Desktop` context-menu lifecycle: retain the active menu as a root, convert global to local coordinates with the selected right/bottom-only fit, manage visibility and focus capture/restoration, support cancellable outside touch and `Closing`/`Closed` events, and process Escape after key routing. Reentrant replacement, direct collection removal, and checked positioning remain ownership- and numeric-safe (`DEV-074`).
- [x] P5-016c Port the dependency-safe tooltip lifecycle: expose upstream delay/offset and an injectable widget creator, start/reset a monotonic stationary-hover timer on Widget enter/move/leave, show a retained right/bottom-fitted Desktop overlay, and hide it on owner leave/detach, touch, replacement, or direct overlay removal. Track the exact owner weakly instead of trusting arbitrary `Tag` payloads (`DEV-079`); keep only the default Label/style-backed creator in P6-002/P8-002.
- [ ] P5-016 Port `Graphics2D/UI/Desktop.cs` widget ordering, layout, focus, menus, dispose, and the Label-backed default tooltip creator.
- [x] P5-017a Port the dependency-safe Desktop input snapshot and keyboard-routing core: injectable CNA/default mouse and fixed key-state providers, previous/current pointer state, mouse-emulated touch transitions, cumulative wheel deltas, retained global event queueing, key up/down/repeat, Tab focus traversal, and menu/focused-widget routing. A detachable retained queue proxy keeps stack-owned Desktop lifetimes safe (`DEV-064`). Widget hit testing/position propagation and basic wheel targeting arrived in P5-010b, Widget-local double-click recognition in P5-010c, Widget-driven cursor routing in P5-010d, generic Widget drag capture in P5-019a, context-menu interaction in P5-016b, and touch-driven tooltip dismissal in P5-016c; keep remaining consumer-specific capture in P5-019/P5-021.
- [ ] P5-017 Port `Graphics2D/UI/Desktop.Input.cs` against CNA keyboard/mouse/touch snapshots.
- [ ] P5-018 Add CNA `TextInputEXT` subscription lifecycle to Desktop/TextBox focus transitions.
- [x] P5-019a Port the generic Widget drag-capture core: arm only through the retained in-tree `DragHandle`, subscribe draggable roots/children to Desktop/parent movement and release events, preserve horizontal/vertical flags and upstream bounds-clamp order, and reset capture on release/detach. Tokenized subscriptions retain exact callback targets and detach before native owner links change (`DEV-067`); checked float conversion and widened coordinate arithmetic reject C++ overflow without changing valid movement (`DEV-068`).
- [x] P5-019b Port the first consumer-specific capture/wheel slice for `Button` and `Slider`: non-releasing buttons reset pressed state on Desktop-wide touch-up, sliders track a pressed knob through Desktop movement beyond local bounds, touch-to-value mapping preserves both orientations and the selected event order, and deepest-target wheel routing honors `WheelAdjustment`/`WheelStep`. Exact retained callback targets, tokenized detach, and clone preservation keep reentrant removal safe (`DEV-069`); widened hint arithmetic rejects native overflow without changing representable geometry (`DEV-070`). Preserve the selected upstream's duplicate general `ValueChanged` notification for a successful wheel adjustment before its single `ValueChangedByUser` notification.
- [x] P5-019c Port the `ScrollViewer` consumer-capture slice: both image thumbs arm only on their current geometry, track Desktop movement outside local bounds, clamp through the selected incremental delta mapping, and stop on local/global release or detach. Transparent content falls through except over active scrollbar frames. Exact retained callback targets and tokenized cleanup make reentrant move/release removal safe (`DEV-071`); widened, checked extent/frame/thumb/wheel/drag arithmetic rejects native overflow without partial scroll-position mutation (`DEV-072`).
- [ ] P5-019 Complete consumer-specific pointer capture and wheel integration for DataGrid resizing and other downstream controls. The snapshot wheel-delta core, generic Widget drag capture, Button/Slider slice, and ScrollViewer thumb slice are complete in P5-017a/P5-010b/P5-019a/P5-019b/P5-019c.
- [x] P5-020 Test arrange/measure margins, padding, min/max, alignments, transforms, visibility, opacity, and z-order. Existing Widget/LayoutUtils/UIUtils coverage exercises every listed behavior; the final integration case locks down Widget scale, rotation, fractional transform origin, inverse conversion, and transform-cache invalidation under linked and sanitised builds.
- [x] P5-020a Harden Grid numeric conversions/accumulation and restore the upstream epsilon-zero `Part` distribution rule, with invalid-span and overflow tests.
- [x] P5-020b Remove signed-overflow UB from the ported `Thickness`/`LayoutUtils`/Widget measure-arrange-transform chain and reject NaN opacity.
- [ ] P5-021 Test focus changes, keyboard navigation, capture/bubble ordering, hover, drag, tooltip, and context-menu lifetimes. Focus/navigation, propagation/basic hover, generic drag capture/lifetime, context-menu lifetime, and the tooltip timer/owner/touch/removal core are covered; keep downstream control-specific capture matrices open.
- [x] P5-021a Lock down the standalone `InputEventsManager` capture/bubble/stop-propagation ordering, including upstream's bubbling stack-rebuild reversal.
- [x] P5-022 Test destruction/reparenting and the broad linked suite under AddressSanitizer and UndefinedBehaviorSanitizer.

### Phase 6 — simple widgets, containers, range controls, and text editing

- [x] P6-001a Port the FNA-selected dependency-safe `Simple/Image.cs` core: retained visual-state images, maximum-state measurement, tint/resize rendering, exact-type cloning, resize enum codec, and external-image MML metadata (`DEV-046`). Keep Color MML text and style application in P4-013/P6-001/P8-002.
- [ ] P6-001 Port `Simple/Image.cs`, including `ImageStyle` application after P8-002.
- [ ] P6-002 Port `Simple/Label.cs`.
- [x] P6-003a Port the style-independent `ButtonBase` core: `ReadOnly`, touch down/up click state, `DoClick`, the `Click` event, abstract internal hooks, clone state, and MML metadata. Cloning preserves `ReadOnly`, correcting the selected upstream omission (`DEV-049`). Keep button/image-button style application in P6-003/P8-003.
- [ ] P6-003 Port `Simple/ButtonBase.cs`.
- [x] P6-004a Port the style-independent concrete `Button` core: single-content layout, touch press/release and touch-left state, Space-key activation, exact-type cloning, and MML registration/round trips. Preserve the internal `ReleaseOnTouchLeft` default for Slider/SplitPane use; its ownership-safe Desktop-wide touch-up subscription arrived in P5-019b. Keep style construction/dictionary lookup in P6-004/P8-003 and `CreateTextButton` after Label in P6-002/P6-004.
- [ ] P6-004 Port `Simple/Button.cs`.
- [x] P6-005a Port the style-independent `ToggleButton` core: `IsToggled`, the exact `IsToggledChanged` alias of `PressedChanged`, single-content layout, persistent touch toggling/click arming, Space-key toggling, cancelable user changes, exact-type cloning, and MML registration/round trips. Preserve the selected upstream behavior that Space can toggle while `ReadOnly` (but not while disabled); keep stylesheet construction/dictionary lookup and Label-dependent `CreateTextButton` in P6-002/P6-005/P8-003.
- [ ] P6-005 Port `Simple/ToggleButton.cs`.
- [x] P6-006a Port the style-independent `CheckButtonBase` core: check-position/content layout, spacing, retained checked/unchecked renderables, read-only check image, touch/Space toggling, exact clone state, `CheckPosition` codec, and abstract MML metadata. Preserve the selected upstream read-only keyboard quirk, while correcting omitted visual-state refresh/clone backing handles (`DEV-050`) and stale measurement after spacing changes (`DEV-051`). The internal image's parent-hover selection arrived in P6-006b; keep stylesheet application for P6-006/P8-003.
- [x] P6-006b Port `CheckImageInternal.UseOverBackground`: while attached, the check image selects its over visual from the parent check button's hover state rather than its smaller own bounds; when detached, it falls back to its own hover. Preserve Widget visual precedence so pressed and disabled images still override hover, and retain the dynamic behavior through CheckButtonBase cloning.
- [ ] P6-006 Port `Simple/CheckButtonBase.cs`.
- [x] P6-007a Port the style-independent concrete `CheckButton` core: `IsChecked`, its exact `PressedChanged` event alias, exact-type cloning, and MML metadata/round trips. It inherits the P6-006b parent-hover check image; keep stylesheet construction and dictionary lookup for P6-007/P8-003.
- [ ] P6-007 Port `Simple/CheckButton.cs`.
- [x] P6-008a Port the style-independent `RadioButton` core: direct-sibling exclusive selection, last-selected preservation, exact-type cloning, and MML type/content round trips. Preserve upstream's XML-ignored `IsPressed`; it inherits the P6-006b parent-hover check image, while stylesheet construction and dictionary lookup remain P6-008/P8-003.
- [ ] P6-008 Port `Simple/RadioButton.cs`.
- [x] P6-009a Port the style-independent separator hierarchy core: thickness/orientation measurement, concrete alignment defaults, inherited image rendering, exact-type cloning, and MML metadata. Dynamic `Thickness` changes invalidate the cached measurement (`DEV-047`). Keep stylesheet constructors/application in P6-009–P6-011/P8-004.
- [ ] P6-009 Port `Simple/SeparatorWidget.cs`.
- [ ] P6-010 Port `Simple/HorizontalSeparator.cs`.
- [ ] P6-011 Port `Simple/VerticalSeparator.cs`.
- [x] P6-012a Implement `Panel` measuring and arranging behavior without style constructors.
- [ ] P6-012 Port `Containers/Panel.cs`.
- [x] P6-013a Implement the layout and attached-proportion subset of `Containers/StackPanel.cs`.
- [x] P6-013b Restore `ObservableCollection<Proportion>` semantics for the layout subset, using retained C++ proportion ownership and sharp-runtime collection notifications.
- [ ] P6-013 Port `Containers/StackPanel.cs` styles and debug rendering.
- [x] P6-014a Implement the layout subset of `Containers/HorizontalStackPanel.cs`.
- [ ] P6-014 Port `Containers/HorizontalStackPanel.cs` style integration.
- [x] P6-015a Implement the layout subset of `Containers/VerticalStackPanel.cs`.
- [ ] P6-015 Port `Containers/VerticalStackPanel.cs` style integration.
- [x] P6-016a Implement `Grid` layout and attached row/column/span property subset.
- [x] P6-016b Restore Grid's observable proportion collection and retained-proportion invalidation semantics.
- [ ] P6-016 Port `Containers/Grid.cs` selection, style, debug render, and input.
- [x] P6-017 Port `Containers/Proportion.cs`.
- [x] P6-018a Port the style-independent `ScrollViewer` core: retained single content, scrollbar geometry, scrolling/clamping, direct wheel scrolling, optional image thumbs, exact-type cloning, and render traversal. Desktop thumb drag capture and scrollbar input fall-through arrived in P5-019c and MML metadata in P6-018b; keep stylesheet application in P6-018/P8-004.
- [x] P6-018b Register the concrete `ScrollViewer` MML surface: inherited implicit content, four nullable external scrollbar images, scroll multiplier and visibility flags, stretch/clipping default overrides, and XML-ignored maximum/current positions. Round-trip caller-provided image handles and nested widget content without styles, and activate Project's existing legacy `ScrollPane` alias.
- [ ] P6-018 Port `Containers/ScrollViewer.cs`.
- [x] P6-019a Port the style-independent `SplitPane` core and both orientations: retained logical widget collection, grid-separated handles, proportions/split positions, reset events, removal, and exact-type deep cloning. Desktop drag/cursor control arrived in P6-019b and MML/container fidelity in P6-019c; keep handle-style dimensions/visuals in P6-019/P6-020/P8-004.
- [x] P6-019b Port the style-independent `SplitPane` interaction slice: horizontal/vertical handles resize their adjacent proportions, preserve the selected multi-handle cell-offset mapping, track movement through Desktop outside local bounds, switch/restore the cursor, stop on global release or detach, and ignore invalid zero/negative available extents. Retained Desktop callbacks plus detachable tokenized handle callbacks keep reentrant removal, reset, and externally retained handles safe; widened geometry and epsilon-deduplicated dual routing are recorded in `DEV-078`. Keep `HandleStyle` dimensions/visuals in P6-019/P6-020/P8-004.
- [x] P6-019c Restore the selected upstream `SplitPane : Container` hierarchy so both orientations inherit stretch defaults and background-dependent input fall-through, then register the abstract/concrete MML hierarchy with XML-ignored orientation and inherited implicit `Widgets` content. Nested horizontal/vertical split panes round-trip without styles.
- [ ] P6-019 Port `Containers/SplitPane.cs`.
- [ ] P6-020 Port horizontal and vertical split-pane specialisations.
- [x] P6-021a Port the style-independent `ProgressBar` hierarchy core: retained filler ownership, minimum/maximum/value state and event behavior, orientation rendering, exact-type cloning, alignment defaults, and MML metadata. Invalid floating fill conversions fail deterministically (`DEV-048`). Keep stylesheet construction/application in P6-021/P8-005.
- [ ] P6-021 Port `Range/ProgressBar.cs` and both orientations.
- [x] P6-022a Port the style-independent `Slider` hierarchy core: range clamping, typed value event, retained button/image knob, safe knob synchronization, orientation defaults, exact-type cloning, and MML metadata. Preserve upstream's sequential clamp behavior for inverted ranges; Desktop drag/wheel input arrived in P5-019b, while stylesheet integration remains P6-022/P8-005.
- [ ] P6-022 Port `Range/Slider.cs` and both orientations.
- [ ] P6-023 Port `Range/SpinButton.cs`.
- [ ] P6-024 Port `TextEdit/UndoRedoRecord.cs` and `UndoRedoStack.cs` after P3-006 selects the C++ text index domain: their `Substring(where, length)` behavior currently uses C# UTF-16 code-unit indices and must not be silently mapped to UTF-8 byte offsets.
- [ ] P6-025 Port `Simple/TextBox.cs` keyboard, IME, clipboard, selection, undo/redo, scrolling, and rich text.
- [ ] P6-026 Replace upstream TextCopy calls with CNA Clipboard while preserving user-visible behavior.
- [ ] P6-027 Translate `SimpleWidgetsTests.cs`, `LabelTests.cs`, `GridTests.cs`, `SliderTests.cs`, and `StackPanelTests.cs`.
- [ ] P6-028 Add mouse/touch/keyboard/IME/clipboard regression tests for every interactive control.

### Phase 7 — selectors, menus, trees, windows, dialogs, and colour picker

- [x] P7-001 Port selector interfaces: `ISelector.cs` and `ISelectorItem.cs`.
- [x] P7-002 Port `Selectors/Selector.cs` generic selection behavior.
- [x] P7-003 Port `Selectors/ListViewButton.cs`.
- [x] P7-004a Port the style-independent `ListView` core: retained ScrollViewer/stack layout, widget wrapping and collection mutation, single/multiple selection state, click selection, scroll forwarding, and exact-type cloning. Desktop dropdown closure and keyboard navigation arrived in P7-004b and MML metadata in P7-004c; keep styles in P7-004/P8-004.
- [x] P7-004b Port `ListView` dropdown interaction: pressed items close only an active matching Desktop context menu, Up/Down navigation skips separators and scrolls the selection into view, and Enter closes the dropdown. Tokenized wrapper callbacks and a detachable callback-state proxy prevent dangling dispatch after rebuild, destruction, or reentrant owner removal (`DEV-075`).
- [x] P7-004c Register the style-independent `ListView` MML surface: implicit logical `Widgets` content, `SelectionMode` codec/default, and XML-ignored internal ScrollViewer/selection state. Round trips preserve concrete logical children without exposing the internal `ListViewButton` wrappers.
- [ ] P7-004 Port `Selectors/ListView.cs` collection adapters and virtual behavior.
- [x] P7-005a Port the style-independent `ComboView` core: retained toggle/list ownership, delegated item and selection APIs, selection-event forwarding, initial selection on expansion, selected-content cloning, measure/arrange sizing, and exact-type cloning. Desktop dropdown display/closure and keyboard delegation arrived in P7-005b and MML metadata in P7-005c; keep the Label placeholder/style in P6-002/P7-005/P8-004.
- [x] P7-005b Port `ComboView` Desktop dropdown integration: size and open the retained ListView below the border box, delegate keys, unpress on an outside context close, and unsubscribe exactly across Desktop transfer/destruction. Retained external subscriptions, tokenized internal callbacks, and callback-state invalidation keep reentrant selection removal and externally retained children safe (`DEV-075`).
- [x] P7-005c Register the style-independent `ComboView` MML surface: nullable `DropdownMaximumHeight`, `SelectionMode`, implicit logical `Widgets` content, and XML-ignored expanded/list/selection state. Round trips preserve concrete logical children without exposing the internal ListView or its button wrappers.
- [ ] P7-005 Port `Selectors/ComboView.cs`.
- [x] P7-006 Port `Selectors/IMenuItem.cs`.
- [x] P7-007a Port the style-independent `MenuItem` data core: text/mnemonic and marker-free display state, image/shortcut/tag metadata, enabled/index/owner state, constructors, and change/selection events. MML metadata arrived in P7-007b; keep retained Label/Image widgets, rich-text mnemonic colour, and stylesheet integration in P3-004/P6-002/P7-007/P8-006.
- [x] P7-007b Register the dependency-safe menu-item MML data surface: abstract `IMenuItem` identity/runtime metadata, concrete `MenuItem` text/shortcut/external-image properties and implicit heterogeneous nested items, and concrete `MenuSeparator` with upstream-ignored identity. Flatten `MenuItem`'s BaseObject identity onto the interface branch needed for exact C++ pointer adjustment; keep blocked Color/ShortcutColor and internal submenu/separator visuals absent.
- [ ] P7-007 Port `Selectors/MenuItem.cs`.
- [x] P7-008 Port `Selectors/MenuSeparator.cs`.
- [x] P7-009a Port the style-independent `Menu` core: observable retained item collection, owner/index synchronization, nested `VerticalMenu` ownership, recursive id lookup, logical hover/selection/open state, close/click behavior, mnemonic/Enter/Space handling, and separator-skipping navigation. MML metadata arrived in P7-009b; keep Grid widget composition, measured cells, Label/Image visual wiring, selection brushes, styles, and Desktop context menus in P5-016/P6-002/P6-016/P7-009/P8-006.
- [x] P7-009b Register the dependency-safe abstract `Menu` and concrete `HorizontalMenu`/`VerticalMenu` MML hierarchy: implicit heterogeneous logical items, `HoverIndexCanBeNull`, exact concrete alignment defaults, and XML-ignored orientation/open/hover/selection state. Keep the unported font/color/selection-brush/label-alignment/style properties and internal Grid/submenus absent.
- [ ] P7-009 Port `Selectors/Menu.cs`.
- [x] P7-010a Port `HorizontalMenu`'s style-independent orientation, alignment defaults, and Left/Right navigation. Its concrete MML registration arrived in P7-009b.
- [ ] P7-010 Port `Selectors/HorizontalMenu.cs`.
- [x] P7-011a Port `VerticalMenu`'s style-independent orientation, alignment defaults, and Up/Down navigation. Its concrete MML registration arrived in P7-009b.
- [ ] P7-011 Port `Selectors/VerticalMenu.cs`.
- [x] P7-012a Port the style-independent `TabItem` data core: retained optional text/content/image/tag/height state, identifier/change and selection events, `ToString`, and clone behavior. MML metadata arrived in P7-012b; keep text color and ListViewButton/Label visual wiring in P3-004/P6-002/P7-012/P7-013/P8-004.
- [x] P7-012b Register the dependency-safe `TabItem` MML surface over `BaseObject`: optional text/height, implicit widget content, and XML-ignored tag/image/spacing/selection state. Keep `Color` absent until the P3 font/text type is approved and ported.
- [ ] P7-012 Port `Selectors/TabItem.cs`.
- [x] P7-013a Port the style-independent `TabControl` core: retained item/button/content ownership, first-item and click selection, selected-content replacement, TabItem content-change subscriptions, four selector positions, removal, and deep cloning. MML metadata arrived in P7-013c; keep Label/text/color visuals, close-button styling, and stylesheet integration in P3-004/P6-002/P7-013/P8-004.
- [x] P7-013b Port `TabControl.CloseableTabs` structure and native callback safety: wrap selector buttons with a close button, remove the exact item, preserve closeable headers through cloning, invalidate layout on item changes, and detach every rebuilt/destroyed selector, close, and item callback. A token ledger plus detachable callback-state proxy keeps externally retained buttons and already-snapshotted handlers inert (`DEV-077`). MML metadata arrived in P7-013c; keep Label/text/color visuals, close-button styling, and stylesheet integration in P3-004/P6-002/P7-013/P8-004.
- [x] P7-013c Register the dependency-safe `TabControl` MML surface: exact selector-position codec, logical implicit `TabItem` sequence, closeable/position/alignment/clip defaults, and XML-ignored selection state. Flatten the unregistered generic `Selector<Grid, TabItem>` metadata onto the concrete `Widget`-derived descriptor and keep internal grids/buttons plus the blocked style property absent.
- [ ] P7-013 Port `Selectors/TabControl.cs`.
- [x] P7-014 Port `Misc/ITreeViewNode.cs`.
- [x] P7-015a Port the style-independent `TreeViewNode` core: retained content and ordered child-node hierarchy, parent links, expand/collapse mark visibility, grid/stack layout, removal, and deep exact-type cloning. Keep stylesheet mark application and MML metadata in P7-015/P8-004.
- [ ] P7-015 Port `Misc/TreeViewNode.cs`.
- [x] P7-016a Port the style-independent `TreeView` core: retained top-level/all-node registry, selection events, expand-path/traversal/find APIs, parent-child keyboard navigation, subtree removal, row-visibility maintenance, and deep tree cloning. Desktop mouse/touch row interaction arrived in P7-016b; keep hover/selection brush rendering, styles, and MML metadata in P7-016/P8-004.
- [x] P7-016b Port `TreeView` pointer interaction: derive visible row rectangles through composed transforms, update/clear mouse hover, select touched rows, and preserve the selected upstream's inverted expand-mark double-click condition. A detachable tokenized mark callback keeps publicly retained expand buttons safe after node destruction (`DEV-076`).
- [ ] P7-016 Port `Misc/TreeView.cs`.
- [ ] P7-017 Port `Misc/Window.cs`.
- [ ] P7-018 Port `Misc/Dialog.cs` and modal focus/callback lifetime.
- [ ] P7-019 Port `ColorPicker/ColorPickerPanel.cs`.
- [ ] P7-020 Port `ColorPicker/ColorPickerPanel.Generated.cs` as reviewed generated source.
- [ ] P7-021 Port `ColorPicker/ColorPickerDialog.cs`.
- [ ] P7-022 Port `DebugOptionsWindow.cs` and its generated companion.
- [ ] P7-023 Translate `SelectorsTests.cs`, event tests, and all selector/window MML fixtures.
- [ ] P7-024 Add modal-window, submenu, tab, tree expansion, and colour-picker pixel/input tests.

### Phase 8 — styles, default skin, and stylesheet behavior

- [ ] P8-001 Port `WidgetStyle.cs`.
- [ ] P8-002 Port `DesktopStyle.cs`, `LabelStyle.cs`, and `ImageStyle.cs`.
- [ ] P8-003 Port button/check/image-text/combo/list-box styles.
- [ ] P8-004 Port grid, scroll-viewer, split-pane, and separator styles.
- [ ] P8-005 Port progress-bar, slider, and spin-button styles.
- [ ] P8-006 Port menu, tab-control, tree, window, dialog, and file-dialog styles.
- [ ] P8-007 Port `DataGridStyle.cs` and `DataGridHeaderStyle.cs`.
- [ ] P8-008 Port `StylesheetFont.cs` and font cache/error behavior.
- [ ] P8-009 Port `StylesheetFontsCollection.cs`.
- [ ] P8-010 Port `Stylesheet.cs`, all style collections, current-style scope, and apply behavior.
- [ ] P8-011 Load default normal and 2x skins after asset attribution passes.
- [ ] P8-012 Translate `StylesheetTests.cs`, including existing-atlas font cases.
- [ ] P8-013 Test all licensed upstream stylesheets (default, C64, LibGDX) and document any omitted asset.
- [ ] P8-014 Add screenshot/reference tests for default-skin controls.

### Phase 9 — file dialog, data grid, property grid, and remaining advanced API

- [x] P9-001 Port `File/FileDialogMode.cs`.
- [ ] P9-002 Port `File/FileDialog.cs`.
- [ ] P9-003 Port `File/FileDialog.Util.cs`.
- [ ] P9-004 Port `File/FileDialog.PlatformDependent.cs` using C++ filesystem/CNA facilities.
- [ ] P9-005 Port `File/FileDialog.Generated.cs` as reviewed generated source.
- [ ] P9-006 Test path normalisation, roots, filters, permissions/errors, and modal lifecycle.
- [ ] P9-007 Port `Data/DataGridColumnBase.cs`.
- [ ] P9-008 Port text, image, and check-box data-grid columns.
- [ ] P9-009 Port `Data/DataGrid.cs` layout, filtering, sorting, resizing, selection, and editing.
- [ ] P9-010 Define explicit `DataGrid` data-item descriptors; do not use nonfunctional System::Type reflection.
- [ ] P9-011 Test typed data grids with registered object descriptors and unsupported-data diagnostics.
- [ ] P9-012 Port `Properties/Record.cs` and `ReflectionRecord.cs` onto explicit descriptors.
- [ ] P9-013 Port property and field record adapters.
- [ ] P9-014 Port `CustomValues.cs`.
- [ ] P9-015 Port `PropertyGridSettings.cs`.
- [ ] P9-016 Port `CollectionEditor.cs`.
- [ ] P9-017 Port `PropertyGrid.cs` onto TypeRegistry and collection adapters.
- [ ] P9-018 Port `MML/AttachedPropertiesRegistry.cs` tests into PropertyGrid/MML tests.
- [ ] P9-019 Document the finite registry requirement as a C++ deviation only where user-defined runtime reflection was possible in C#.
- [ ] P9-020 Add tests for every PropertyGrid editor, custom value, collection editor, and registered property type.
- [ ] P9-021 Translate `DataGridTests.cs` and `CustomTests.cs`.

### Phase 10 — exhaustive parity, examples, packaging, and release gate

- [ ] P10-001 Complete `UPSTREAM_MANIFEST.md` rows for all 189 upstream production source files, marking FNA-excluded/conditional files explicitly.
- [ ] P10-002 Complete manifest rows for all copied tests/assets and all new Myra-CNA-only files.
- [ ] P10-003 Verify no upstream `Myra.FNA.Core` public type is missing from the compatibility inventory.
- [ ] P10-004 Verify no upstream public property/event/enum value is silently omitted.
- [ ] P10-005 Add a machine-readable API inventory and compare it with the pinned upstream inventory.
- [ ] P10-006 Add headless unit tests for each ported class/function.
- [ ] P10-007 Add MML fixture tests for each MML-instantiable widget/style.
- [ ] P10-008 Add reference screenshots for basic/simple/container/range widgets.
- [ ] P10-009 Add reference screenshots for selectors/windows/colour-picker/data-grid widgets.
- [ ] P10-010 Establish tolerances and per-backend expected differences before accepting screenshots.
- [ ] P10-011 Run SDL_RENDERER integration tests on a real/virtual display.
- [ ] P10-012 Run SOFTWARE backend tests for deterministic CPU coverage.
- [ ] P10-013 Run an additional mature CNA GPU backend for SpriteBatch/scissor validation.
- [ ] P10-014 Run ASan/UBSan test configurations and fix all Myra-CNA findings.
- [ ] P10-015 Add an all-widgets CNA demo based on upstream behavior, not copied C# application code.
- [ ] P10-016 Add a text-edit/IME/clipboard CNA example.
- [ ] P10-017 Add an MML/default-style loading CNA example.
- [ ] P10-018 Add a file-dialog and property-grid example after their registry work is complete.
- [ ] P10-019 Write README: purpose, non-affiliation, licence/attribution, prerequisites, build, and minimal usage.
- [ ] P10-020 Write migration guide from Myra C# / FNA code to Myra-CNA conventions.
- [ ] P10-021 Document every C++ deviation and every unsupported/blocked source feature.
- [ ] P10-022 Add CI with compilation, headless tests, provenance lint, and licence manifest validation.
- [ ] P10-023 Ensure CI and docs never invoke more than three compilation workers.
- [ ] P10-024 Verify clean consumer integration through `add_subdirectory` with a parent CNA target.
- [ ] P10-025 Verify standalone opt-in sibling-CNA configuration.
- [ ] P10-026 Verify documented headers-only compile-check behavior and its limitations.
- [ ] P10-027 Perform a manual licence/provenance release audit.
- [ ] P10-028 Tag the exact upstream revision and compatibility level in the first release notes.

## 6. Source-family coverage map

This map prevents source families from disappearing into broad phase labels.  Each
individual source file named below receives its own manifest row and focused
implementation/test task during the phase shown.

| Upstream source family | Files/features | Backlog |
| --- | --- | --- |
| `Attributes/` | Content, DesignerFolded, FilePath, Range, SkipLoad, SkipSave, StylePropertyPath, XmlName | P4-001–P4-008 |
| `Events/` | all nine event argument/handler files | P1-001–P1-009 |
| `Graphics2D/` | brushes, images, RenderContext, Shapes, Thickness, Transform | P1-010–P1-011, P2-006–P2-022 |
| `Graphics2D/TextureAtlases/` | ColoredRegion, NinePatchRegion, TextureRegion, TextureRegionAtlas, TintedRegion | P2-009–P2-015 |
| `Graphics2D/UI/` kernel | Widget (three partials), Desktop (two partials), Container, ContentControl, Project, layouts, input, enums/interfaces | P4-020, P5-001–P5-022 |
| `Graphics2D/UI/Simple/` | Button, ButtonBase, CheckButton*, Image, Label, RadioButton, separators, TextBox, ToggleButton | P6-001–P6-011, P6-024–P6-028 |
| `Graphics2D/UI/Containers/` | Grid, panels/stacks, proportion, scroll, split panes | P6-012–P6-020 |
| `Graphics2D/UI/Range/` | progress bars, sliders, spin button | P6-021–P6-023 |
| `Graphics2D/UI/Selectors/` | ComboView, ListView, Menu*, Selector, Tab* | P7-001–P7-013 |
| `Graphics2D/UI/Misc/` | Dialog, TreeView/Node, Window | P7-014–P7-018 |
| `Graphics2D/UI/ColorPicker/` | panel, generated panel, dialog | P7-019–P7-021 |
| `Graphics2D/UI/Styles/` | all 25 style and stylesheet/font files | P8-001–P8-014 |
| `Graphics2D/UI/File/` | FileDialog and generated/platform/util companions | P9-001–P9-006 |
| `Graphics2D/UI/Data/` | DataGrid and all columns | P9-007–P9-011 |
| `Graphics2D/UI/Properties/` | records, collection editor, custom values, PropertyGrid | P9-012–P9-020 |
| `MML/` | attached properties, base/load/save contexts, serializers, base interfaces | P4-009–P4-022, P9-018–P9-019 |
| root `Myra/*.cs` | DefaultAssets, MyraEnvironment, three AssetManager extension files | P2-001–P2-005, P3-016–P3-020 |
| `Platform/` | renderer/platform interfaces, FontStash renderer adapters, keys, touch types | Explicitly excluded: upstream `Myra.FNA.Core.csproj` removes this whole directory.  Seven manifest rows record this target-specific decision. |
| `TextCopy/` | BashRunner, Clipboard, Linux/OSX/Windows clipboard | P6-026; attribution retained, implementation replaced by CNA Clipboard where behavior permits |
| `Utility/` | ColorHSV, platform/input/math/path/reflection/serialization helpers | P1-012–P1-021, P4/P9 descriptor work |
| `Resources/` | Inter TTF, normal/2x skin PNG/XMAT/XMMS | P0-015–P0-017, P3-019–P3-021, P8-011–P8-014 |
| `Myra.Tests/` | 15 C# test files and assets | P0-016, P1/P4/P5/P6/P7/P8/P9 test tasks, P10-006–P10-013 |

## 7. Sequencing and acceptance gates

1. **Do not claim a rendered or text-capable widget before the P0/P1/P2/P3
   gates:** a headless layout/ownership kernel may be translated and tested
   earlier, but visual controls require rendering primitives and a real font
   strategy.
2. **Do not claim MML support before P4:** XML parsing without a type/property
   registry is not MML compatibility.
3. **Do not claim editable text support before P3 + P5-018 + P6-025:** keyboard
   polling alone does not cover Unicode/IME, selection, clipboard, or undo.
4. **Do not claim full Myra parity before P9 and P10:** PropertyGrid/DataGrid
   are precisely where C# reflection behavior becomes visible.
5. **Do not redistribute a copied test asset before P0-016 passes, or any
   VisUI-derived skin asset before `needs_human` P0-015b is resolved.**
6. **Do not mark a port task complete without upstream-source comparison, a
   regression test, a manifest entry, clean warnings, and a `--parallel 3`
   build.**

## 8. Initial risks and decision log

| ID | Risk/decision | Current conclusion | Required next action |
| --- | --- | --- | --- |
| R1 | Project shape | One `myra-cna` C++/CNA library; no preliminary two-library split. | P0-001 onward. |
| R2 | Reflection | Cannot rely on sharp-runtime reflection; it is intentionally stubbed. | Implement P4 explicit registry. |
| R3 | Font stack | The completed `docs/font-audit.md` confirms that current siblings lack equivalent dynamic TTF/RichText behavior and documents a required-subset FontStashSharp plus native-rasterizer recommendation. | Resolve `needs_human` P3-004, then execute P3-005–P3-015 before advanced text widgets. |
| R4 | CNA Extended | Valuable existing code and proven practices, but no blanket linkage. | Reuse only after per-capability comparison. |
| R5 | MonoGame.Extended lineage | Two Myra files directly acknowledge it; existing C++ port is a useful reference. | Preserve dual notices in P0/P1/P2. |
| R6 | TextCopy | CNA already has Clipboard API. | Keep TextCopy notice if semantics/source are ported; otherwise document replacement and compare behavior. |
| R7 | Default/test assets | P0-015 traced Inter 3.012 to an exact OFL-1.1 source commit and preserved its full notice. The skin is demonstrably VisUI-derived and carries additional NOTICE/icon terms that this project will not reinterpret autonomously. P0-016 classified every one of the 35 test assets and found additional logo/font fixtures that must be replaced. | Resolve `needs_human` P0-015b or create original/clearly licensed artwork; execute P0-016a's project-owned test-fixture replacements before porting the affected tests. |
| R8 | Backend scope | Myra should use CNA SpriteBatch once, then inherit CNA backend support. | Establish SDL_RENDERER/SOFTWARE baseline first. |
| R9 | CPU use | User requires at most three compilation workers. | Enforce P0-007 and P10-023. |
