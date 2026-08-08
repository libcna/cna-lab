# Myra-CNA — faithful C++23/CNA port plan

## 1. Decision, scope, and status

**Status:** planning complete; implementation has not begun.

**Product:** `myra-cna` is one standalone C++23 library: a faithful port of the
Myra UI library to C++ for CNA.  It is not a new generic GUI framework, a CNA
core module, a rewrite with a different widget model, or a port of MyraPad.

**Primary source of truth:** the upstream `Myra.FNA.Core` build selection from
MyraUI/Myra revision
`0d79b939310bfe1d00b21803fe15e291caf60aa1` (`2026-08-08`, `build fix`),
checked out for this analysis at `/tmp/myra-upstream`.

**Integration baselines inspected:** CNA `ac3aaaeb2`, cna-extended `2ff3cff`,
and sharp-runtime `b797928f`.

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
   The bundled `Inter-Regular.ttf` has no separate licence notice in the
   inspected Myra tree, and the README says the skin originates from VisUI.
   Asset provenance must be verified before either asset is redistributed.
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
- [ ] P0-003 Add C++23, warning-as-error, formatting, and include-what-you-use policy.
- [x] P0-004 Implement parent-CNA / opt-in sibling-CNA / headers-only CMake modes.
- [x] P0-005 Add `MYRA_CNA_BUILD_TESTS` and GoogleTest integration without building unrelated sibling tests.
- [x] P0-006 Add `MYRA_CNA_BUILD_EXAMPLES` and a minimal executable target.
- [ ] P0-007 Add an explicit `--parallel 3` build/test command to README and CI scripts.
- [x] P0-008 Create root `LICENSE` for original Myra-CNA work.
- [x] P0-009 Create `NOTICE.md` with complete Myra MIT notice and official source URL/revision.
- [x] P0-010 Create `THIRD_PARTY_NOTICES.md` with Myra, MonoGame.Extended, and TextCopy notices.
- [x] P0-011 Create `UPSTREAM_MANIFEST.md` schema and source-header template.
- [x] P0-012 Add automated test/lint that every ported `.hpp`/`.cpp` has provenance metadata.
- [x] P0-013 Add automated test/lint that every source header points to a manifest row.
- [ ] P0-014 Audit every upstream `src/Myra` file for a non-Myra lineage or copied header.
- [ ] P0-015 Audit the default skin and `Inter-Regular.ttf` redistribution provenance.
- [ ] P0-016 Audit all Myra.Tests assets before copying them.
- [x] P0-017 Document named-source and asset exclusions until their licences are resolved.
- [x] P0-018 Add `docs/cpp-deviations.md` with an empty, reviewed deviation-table template.
- [ ] P0-019 Add test helper for numeric/rectangle/color comparisons.
- [ ] P0-020 Add deterministic temporary asset directory helper.
- [x] P0-021 Add a headless test executable and register it in CTest.
- [ ] P0-022 Add a CNA SDL_RENDERER smoke executable and register its display requirement.

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
- [ ] P1-013 Port `Utility/CrossEngineStuff.cs` directly to CNA colors/matrices.
- [x] P1-014 Port `Utility/Mathematics.cs` and document C++ numeric differences.
- [ ] P1-015 Port `Utility/PathUtils.cs` over sharp-runtime/filesystem APIs.
- [ ] P1-016 Port `Utility/Rest.cs`.
- [x] P1-017 Port `Utility/StringUtils.cs`.
- [ ] P1-018 Port `Utility/UIUtils.cs`.
- [x] P1-019 Port `Utility/EventsExtensions.cs` with removable subscriptions.
- [ ] P1-020 Port `Utility/CurrentPlatform.cs` only to the extent required by FileDialog/clipboard.
- [ ] P1-021 Port `Utility/InputExtension.cs` with MonoGame.Extended dual attribution.
- [ ] P1-022 Translate upstream unit tests for all Phase 1 types.

### Phase 2 — direct CNA environment and graphics primitives

- [ ] P2-001 Port `MyraEnvironment.cs` as direct CNA environment/configuration.
- [ ] P2-002 Define GraphicsDevice lifetime/initialisation contract and diagnostic errors.
- [ ] P2-003 Implement default cursor mapping through CNA `MouseCursor`/`Mouse::SetCursor`.
- [ ] P2-004 Implement `DisableClipping`, debug-frame, and event-model settings.
- [ ] P2-005 Define explicit per-game/per-desktop cleanup in lieu of GC finalization.
- [ ] P2-006 Port `Graphics2D/IBrush.cs`.
- [ ] P2-007 Port `Graphics2D/IImage.cs`.
- [ ] P2-008 Port `Graphics2D/Brushes/SolidBrush.cs`.
- [ ] P2-009 Port `Graphics2D/TextureAtlases/TextureRegion.cs`.
- [ ] P2-010 Port `Graphics2D/TextureAtlases/ColoredRegion.cs`.
- [ ] P2-011 Port `Graphics2D/TextureAtlases/TintedRegion.cs`.
- [ ] P2-012 Port `Graphics2D/TextureAtlases/NinePatchRegion.cs` including degenerate dimensions.
- [ ] P2-013 Port `Graphics2D/TextureAtlases/TextureRegionAtlas.cs`.
- [ ] P2-014 Implement texture loading/caching ownership against CNA `Texture2D`.
- [ ] P2-015 Implement white-region creation with CNA texture lifetime tests.
- [ ] P2-016 Port `Graphics2D/RenderContext.cs` begin/end/flush state machine.
- [ ] P2-017 Map nearest/linear/anisotropic filtering to CNA sampler states.
- [ ] P2-018 Map clipping/scissor changes to CNA rasterizer/device state and flush ordering.
- [ ] P2-019 Port all `RenderContext::Draw` overloads.
- [ ] P2-020 Port `RenderContext.Shapes.cs` with MonoGame.Extended dual attribution.
- [ ] P2-021 Test sprite source rectangles, rotation, scale, opacity, clipping, and nested clips.
- [ ] P2-022 Test render context under SOFTWARE/HEADLESS where possible and SDL_RENDERER on screen.

### Phase 3 — font, rich text, assets, and asset manager

- [ ] P3-001 Inventory every FontStashSharp type/method called by upstream `src/Myra`.
- [ ] P3-002 Verify FontStashSharp licence and record it before translating any implementation.
- [ ] P3-003 Compare a direct required-subset port with existing permissible C/C++ alternatives.
- [ ] P3-004 Obtain an explicit dependency decision before adding any new third-party source.
- [ ] P3-005 Define Myra text abstraction without changing upstream widget semantics.
- [ ] P3-006 Implement UTF-8/Unicode code-point iteration and grapheme-boundary policy.
- [ ] P3-007 Implement dynamic TTF/OTF font loading.
- [ ] P3-008 Implement glyph rasterisation and CPU/GPU atlas allocation.
- [ ] P3-009 Implement atlas growth, invalidation, cache lifetime, and device-loss behavior.
- [ ] P3-010 Implement glyph metrics, kerning, line height, baseline, and measurement.
- [ ] P3-011 Implement rich-text tokenisation/layout used by Label/TextBox.
- [ ] P3-012 Implement wrapping, alignment, clipping, and selection-rectangle layout.
- [ ] P3-013 Implement RTL/bidirectional behavior to the documented upstream level.
- [ ] P3-014 Render glyphs through `RenderContext`/CNA SpriteBatch.
- [ ] P3-015 Test text pixel baselines, measurement, wrapping, missing glyphs, and atlas reuse.
- [ ] P3-016 Port `MyraAssetManagerExtensions.cs`.
- [ ] P3-017 Port `MyraAssetManagerExtensions.Stylesheet.cs`.
- [ ] P3-018 Record `MyraAssetManagerExtensions.PlatformAgnostic.cs` as an inactive conditional source in the FNA/CNA manifest; do not create a parallel platform-agnostic API.
- [ ] P3-019 Port `DefaultAssets.cs` after the default-asset licence gate passes.
- [ ] P3-020 Copy/package licensed default skin resources and verify all paths case-sensitively.
- [ ] P3-021 Provide a documented no-default-assets configuration for unresolved asset cases.

### Phase 4 — MML metadata and XML foundation

- [ ] P4-001 Port `Attributes/ContentAttribute.cs` as registry metadata.
- [ ] P4-002 Port `Attributes/DesignerFoldedAttribute.cs` as registry metadata.
- [ ] P4-003 Port `Attributes/FilePathAttribute.cs` as registry metadata.
- [ ] P4-004 Port `Attributes/RangeAttribute.cs` as registry metadata.
- [ ] P4-005 Port `Attributes/SkipLoadAttribute.cs` as registry metadata.
- [ ] P4-006 Port `Attributes/SkipSaveAttribute.cs` as registry metadata.
- [ ] P4-007 Port `Attributes/StylePropertyPathAttribute.cs` as registry metadata.
- [ ] P4-008 Port `Attributes/XmlNameAttribute.cs` as registry metadata.
- [x] P4-009 Port `MML/BaseObject.cs`.
- [x] P4-010 Port `MML/IItemWithId.cs`.
- [ ] P4-011 Port `MML/IHasColor.cs` with CNA `Color`.
- [ ] P4-012 Design `TypeRegistry`, `TypeDescriptor`, property descriptor, and factory contracts.
- [ ] P4-013 Implement value codecs for primitive, optional, enum, color, vector, rectangle, thickness, image, and font values.
- [ ] P4-014 Implement attached-property descriptors and `AttachedPropertiesRegistry.cs` semantics.
- [ ] P4-015 Port `MML/BaseContext.cs` over `System::Xml`.
- [ ] P4-016 Port `MML/TypeSerializers.cs` over the explicit codecs.
- [ ] P4-017 Port `MML/LoadContext.cs`, including collection/content-property rules.
- [ ] P4-018 Port `MML/SaveContext.cs`, including default/skip-save rules.
- [ ] P4-019 Register every Phase 5–9 public MML type/property explicitly.
- [ ] P4-020 Port `Graphics2D/UI/Project.cs` load/save/clone/export behavior.
- [ ] P4-021 Add malformed XML, unknown type/property, duplicate id, and conversion-error tests.
- [ ] P4-022 Translate `MMLTests.cs`, `AssetLoadingTests.cs`, and XML round-trip fixtures.

### Phase 5 — Widget, desktop, layout, and input kernel

- [x] P5-001 Port `Graphics2D/UI/Enums.cs`.
- [x] P5-002 Port `Graphics2D/IContent.cs`.
- [x] P5-003 Port `Graphics2D/UI/ILayout.cs`.
- [x] P5-004 Port `Graphics2D/UI/ITransformable.cs`.
- [x] P5-005 Port `Graphics2D/UI/LayoutUtils.cs`.
- [x] P5-006 Port `Graphics2D/UI/InputContext.cs`.
- [x] P5-007 Port `Graphics2D/UI/InputEventsManager.cs`.
- [x] P5-008a Implement the layout, transform, property/event, and invalidation kernel of `Widget.cs`.
- [ ] P5-008 Port `Graphics2D/UI/Widget.cs` properties/defaults/invalidation/render traversal.
- [x] P5-009a Implement explicit child ownership, reparenting, and stable Z-index snapshots from `Widget.Children.cs`.
- [ ] P5-009 Port `Graphics2D/UI/Widget.Children.cs` with explicit ownership and reparenting.
- [ ] P5-010 Port `Graphics2D/UI/Widget.Input.cs` bubbling/capturing/hover/drag semantics.
- [ ] P5-011 Port `Graphics2D/UI/ContentControl.cs`.
- [x] P5-012a Implement `Container` stretch defaults and its explicit child-ownership facade.
- [ ] P5-012 Port `Graphics2D/UI/Container.cs`.
- [ ] P5-013 Port `Graphics2D/UI/Layouts/SingleItemLayout.cs`.
- [ ] P5-014 Port `Graphics2D/UI/Layouts/StackPanelLayout.cs`.
- [ ] P5-015 Port `Graphics2D/UI/Layouts/GridLayout.cs`.
- [ ] P5-016 Port `Graphics2D/UI/Desktop.cs` widget ordering, layout, focus, menus, tooltip, dispose.
- [ ] P5-017 Port `Graphics2D/UI/Desktop.Input.cs` against CNA keyboard/mouse/touch snapshots.
- [ ] P5-018 Add CNA `TextInputEXT` subscription lifecycle to Desktop/TextBox focus transitions.
- [ ] P5-019 Implement correct wheel deltas and pointer capture using previous input states.
- [ ] P5-020 Test arrange/measure margins, padding, min/max, alignments, transforms, visibility, opacity, and z-order.
- [ ] P5-021 Test focus changes, keyboard navigation, capture/bubble ordering, hover, drag, tooltip, and context-menu lifetimes.
- [ ] P5-022 Test destruction/reparenting under AddressSanitizer and UndefinedBehaviorSanitizer.

### Phase 6 — simple widgets, containers, range controls, and text editing

- [ ] P6-001 Port `Simple/Image.cs`.
- [ ] P6-002 Port `Simple/Label.cs`.
- [ ] P6-003 Port `Simple/ButtonBase.cs`.
- [ ] P6-004 Port `Simple/Button.cs`.
- [ ] P6-005 Port `Simple/ToggleButton.cs`.
- [ ] P6-006 Port `Simple/CheckButtonBase.cs`.
- [ ] P6-007 Port `Simple/CheckButton.cs`.
- [ ] P6-008 Port `Simple/RadioButton.cs`.
- [ ] P6-009 Port `Simple/SeparatorWidget.cs`.
- [ ] P6-010 Port `Simple/HorizontalSeparator.cs`.
- [ ] P6-011 Port `Simple/VerticalSeparator.cs`.
- [x] P6-012a Implement `Panel` measuring and arranging behavior without style constructors.
- [ ] P6-012 Port `Containers/Panel.cs`.
- [ ] P6-013 Port `Containers/StackPanel.cs`.
- [ ] P6-014 Port `Containers/HorizontalStackPanel.cs`.
- [ ] P6-015 Port `Containers/VerticalStackPanel.cs`.
- [ ] P6-016 Port `Containers/Grid.cs`.
- [ ] P6-017 Port `Containers/Proportion.cs`.
- [ ] P6-018 Port `Containers/ScrollViewer.cs`.
- [ ] P6-019 Port `Containers/SplitPane.cs`.
- [ ] P6-020 Port horizontal and vertical split-pane specialisations.
- [ ] P6-021 Port `Range/ProgressBar.cs` and both orientations.
- [ ] P6-022 Port `Range/Slider.cs` and both orientations.
- [ ] P6-023 Port `Range/SpinButton.cs`.
- [ ] P6-024 Port `TextEdit/UndoRedoRecord.cs` and `UndoRedoStack.cs`.
- [ ] P6-025 Port `Simple/TextBox.cs` keyboard, IME, clipboard, selection, undo/redo, scrolling, and rich text.
- [ ] P6-026 Replace upstream TextCopy calls with CNA Clipboard while preserving user-visible behavior.
- [ ] P6-027 Translate `SimpleWidgetsTests.cs`, `LabelTests.cs`, `GridTests.cs`, `SliderTests.cs`, and `StackPanelTests.cs`.
- [ ] P6-028 Add mouse/touch/keyboard/IME/clipboard regression tests for every interactive control.

### Phase 7 — selectors, menus, trees, windows, dialogs, and colour picker

- [ ] P7-001 Port selector interfaces: `ISelector.cs` and `ISelectorItem.cs`.
- [ ] P7-002 Port `Selectors/Selector.cs` generic selection behavior.
- [ ] P7-003 Port `Selectors/ListViewButton.cs`.
- [ ] P7-004 Port `Selectors/ListView.cs` collection adapters and virtual behavior.
- [ ] P7-005 Port `Selectors/ComboView.cs`.
- [ ] P7-006 Port `Selectors/IMenuItem.cs`.
- [ ] P7-007 Port `Selectors/MenuItem.cs`.
- [ ] P7-008 Port `Selectors/MenuSeparator.cs`.
- [ ] P7-009 Port `Selectors/Menu.cs`.
- [ ] P7-010 Port `Selectors/HorizontalMenu.cs`.
- [ ] P7-011 Port `Selectors/VerticalMenu.cs`.
- [ ] P7-012 Port `Selectors/TabItem.cs`.
- [ ] P7-013 Port `Selectors/TabControl.cs`.
- [ ] P7-014 Port `Misc/ITreeViewNode.cs`.
- [ ] P7-015 Port `Misc/TreeViewNode.cs`.
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

- [ ] P9-001 Port `File/FileDialogMode.cs`.
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

1. **Do not port widgets before P0/P1/P2/P3 decision gate:** the project needs
   provenance, rendering primitives, and a real font strategy first.
2. **Do not claim MML support before P4:** XML parsing without a type/property
   registry is not MML compatibility.
3. **Do not claim editable text support before P3 + P5-018 + P6-025:** keyboard
   polling alone does not cover Unicode/IME, selection, clipboard, or undo.
4. **Do not claim full Myra parity before P9 and P10:** PropertyGrid/DataGrid
   are precisely where C# reflection behavior becomes visible.
5. **Do not redistribute a copied asset before P0-015/P0-016 passes.**
6. **Do not mark a port task complete without upstream-source comparison, a
   regression test, a manifest entry, clean warnings, and a `--parallel 3`
   build.**

## 8. Initial risks and decision log

| ID | Risk/decision | Current conclusion | Required next action |
| --- | --- | --- | --- |
| R1 | Project shape | One `myra-cna` C++/CNA library; no preliminary two-library split. | P0-001 onward. |
| R2 | Reflection | Cannot rely on sharp-runtime reflection; it is intentionally stubbed. | Implement P4 explicit registry. |
| R3 | Font stack | Highest-risk parity area; current siblings lack equivalent dynamic TTF/RichText behavior. | Execute P3-001–P3-015 before advanced widgets. |
| R4 | CNA Extended | Valuable existing code and proven practices, but no blanket linkage. | Reuse only after per-capability comparison. |
| R5 | MonoGame.Extended lineage | Two Myra files directly acknowledge it; existing C++ port is a useful reference. | Preserve dual notices in P0/P1/P2. |
| R6 | TextCopy | CNA already has Clipboard API. | Keep TextCopy notice if semantics/source are ported; otherwise document replacement and compare behavior. |
| R7 | Default assets | Legal provenance needs verification, especially Inter and VisUI-derived skin. | P0-015/P0-016 gate. |
| R8 | Backend scope | Myra should use CNA SpriteBatch once, then inherit CNA backend support. | Establish SDL_RENDERER/SOFTWARE baseline first. |
| R9 | CPU use | User requires at most three compilation workers. | Enforce P0-007 and P10-023. |
