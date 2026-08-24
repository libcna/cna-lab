# Myra-CNA continuity

## Current state

- Active branch: `develop`.
- Pushed commit `ba98d62` (`Implement ToggleButton interaction core`) completes
  P6-005a. The repository HEAD described by this handoff also completes P6-006a's
  style-independent abstract `CheckButtonBase`, P6-007a's concrete
  `CheckButton`, P6-008a's direct-sibling `RadioButton` behavior, P6-018a's
  style-independent `ScrollViewer` core, P6-019a's split-pane core, P6-022a's
  style-independent `Slider` core, P7-001/P7-002's generic selector contracts
  and collection-backed selection core, P7-003's `ListViewButton` grouping,
  P7-004a's collection-backed `ListView` core, P7-005a's retained ComboView
  core, P7-006/P7-008's menu-item contracts and separator state, P7-007a's
  MenuItem data core, P7-009a's retained menu/navigation core, P7-010a/P7-011a's
  horizontal/vertical menu navigation, P7-012a's TabItem data core, P7-013a's
  TabControl core, P7-014's tree-node contract, P7-015a/P7-016a's TreeView node
  and tree cores, P5-012's complete Container contract, and P0-023's
  compatibility with current modular
  CNA/sharp-runtime checkouts. `CheckButton` now carries its `IsChecked` and
  `PressedChanged` event aliases, exact clone factory, and MML registration;
  stylesheet construction remains deliberately deferred.
- The authoritative upstream reference remains Myra revision
  `0d79b939310bfe1d00b21803fe15e291caf60aa1` at `/tmp/myra-upstream`.
- Current linked validation pins sibling CNA
  `1bb2145d99ed572dd4eb15009c34e2e5f410fcf0` and sharp-runtime
  `54578590b328aa9612fe38bfddca9fd8ca795144`; both sibling worktrees remained
  unmodified.
- The font audit additionally pinned FontStashSharp 1.5.6 at
  `24f3dc46d59dcda0dddb59754a99aeaefc9bd369`, FontStashSharp.Base 1.2.3 at
  `670ecb8c7a323fcce6d9c176fa3692b4dd28b959`, and XNAssets 0.8.5 at
  `928a178da1b3fd83a10536a455390f8d0ff7477b`; exact links and findings live in
  `docs/font-audit.md` rather than relying on the temporary `/tmp` checkouts.

## Completed foundation

The project has a provenance-checked C++23/CMake library with direct CNA and
sharp-runtime integration modes. Every translated source has Myra attribution
and a manifest entry. The current ported surface includes:

- build integration for both legacy flat and current modular CNA/sharp-runtime
  layouts. Header-only mode discovers module include/source trees. Because the
  current CNA source-partition guard requires CNA to remain top-level, linked
  and sanitizer builds use `cmake/AddMyraCnaToCnaBuild.cmake` through
  `CMAKE_PROJECT_CNA_INCLUDE`; Myra explicitly links `SharpRuntime::Xml` and
  current renderer test seams. `MYRA_CNA_LINK_CNA=ON` now gives a focused
  diagnostic for modular checkouts instead of entering an invalid child build;
- a shared strict-warning policy for all project-owned targets, with warnings
  as errors by default, a repository `.clang-format`, conditional non-mutating
  check/mutating format targets, and opt-in fail-fast include-what-you-use
  integration documented in `DEVELOPING.md`;
- reusable GoogleTest numeric/rectangle/color predicates with component-level
  failure diagnostics, plus a deterministic move-only temporary-asset tree
  isolated by build working directory and test name. The latter rejects path
  escapes, clears stale content, supports nested text fixtures, and owns cleanup;
- a conditional SDL_RENDERER smoke executable/CTest that creates a real CNA
  window and device, renders a red Myra `SolidBrush` over a green clear, reads
  both pixels back, and exits after one frame. CTest records its X11 display,
  labels, resource lock, and timeout explicitly;
- a completed FontStashSharp/FNA API, provenance, licence, Unicode-indexing,
  and native-alternative audit. The full zlib notice is preserved, the inactive
  platform-agnostic asset extension is manifested, and no dependency/source was
  incorporated pending `needs_human` P3-004;
- a complete lineage classification of all 189 pinned production C# files,
  including the ten non-default exceptions, selected FNA source set, exact
  inventory hash, and deliberate CNA replacements in
  `docs/source-lineage-audit.md`;
- a complete default-resource audit with hashes for all seven Myra files.
  Inter 3.012 resolves to exact official source commit
  `06b166889e335a2454c0767734a05b27f6403098` under OFL 1.1 and its full notice
  is preserved. The skin remains unbundled: 202/207 raw rasters byte-match
  VisUI, whose NOTICE/icon terms require the human decision P0-015b;
- a complete P0-016 inventory of all 35 pinned Myra.Tests assets (906457
  bytes; sorted manifest SHA-256
  `8d6ae480b533b3dc1892fc2ff386db7a9056e5127e9d8c40455df6d58cb7291f`).
  Myra-authored XML is reusable with attribution and the libGDX skin has exact
  Apache-2.0 lineage. The MonoGame logo, Arial/C64 bitmap assets, unmatched
  DroidSans build, and VisUI-derived default skin must be replaced/deferred;
  no upstream test binary was copied;
- event arguments, multicast handlers, a propagation manager whose capture and
  bubble queues retain processors through dispatch/filtering, and invocation
  helpers (`DEV-039`);
- `Thickness`, `Transform`, `Mathematics`, `ColorHSV`, and string helpers;
- MML `BaseObject`, `IItemWithId`, and `IHasColor`;
- UI contracts, `Widget`'s layout/transform kernel and complete child-tree API,
  abstract `ContentControl`, `Container`, `Panel`, `Proportion`, and
  `SingleItemLayout<T>`.
- `Container` now completes its selected-upstream surface: stretch defaults,
  the explicit owned widget facade, reentrant-safe deep cloning, and input that
  falls through exactly when no background brush is assigned.
- Registry metadata attributes (`Content`, `Range`, XML/style/file-path, and
  skip/designer markers) plus `FileDialogMode`.
- An explicit, non-reflective MML `TypeRegistry`, with factories, typed
  property getter/setter adapters, inherited properties, defaults, XML names,
  and the previously ported attribute metadata. Registration rejects ambiguous
  C++-name/XML-alias cross-collisions, including a derived descriptor that would
  merge two inherited property identities, while retaining intentional derived
  overrides. Property/complex-adapter callbacks reject null owners before user
  code runs, and factories must return a non-null object.
- Myra's typed `AttachedPropertyInfo<T>`, global attached-property registry,
  base-type enumeration via `TypeRegistry`, change notifications, and widget
  measure/arrange invalidation options. Global ID/storage access is synchronized,
  returned heap-backed descriptors stay address-stable, and duplicate exact
  owner/name declarations are rejected.
- `GridLayout` plus the layout/attached-property subset of `Grid`, including
  Part/Auto/Fill/Pixels proportions, spacing, spans, cell geometry, child
  placement, observable proportion collections, and safe retained-proportion
  change subscriptions.
- `StackPanelLayout`, `StackPanel`, `HorizontalStackPanel`, and
  `VerticalStackPanel` layout subsets, including spacing, attached
  proportions, and explicit observable proportion collections.
- MML `ITypeSerializer`/`TypeSerializer<T>` and invariant Vector2, Thickness,
  and Rectangle serializers.
- An MML `ValueCodecRegistry` for invariant primitive values, C++ optionals,
  finite explicitly named enums, flags, and the audited geometry serializers
  when CNA is linked. Deferred-linkage consumers can opt into geometry after
  supplying CNA symbols.
- Registry-backed MML `BaseContext` property classification, including inherited
  descriptors, simple/complex splitting, explicit external-asset metadata, and
  the distinct upstream load/save skip rules.
- Registry-backed MML scalar loading and saving with factories, legacy class and
  property names, XML property names, defaults/null omission, empty-string
  overrides, load/save filters, explicit external-asset callbacks, and hard
  diagnostics for unsupported complex content.
- Attached-property XML load/save and `_`-prefixed `BaseObject` user-data loading
  through explicit `TypeDescriptor` BaseObject accessors. Attached values retain
  the upstream `OwnerType.Property` spelling and default omission behavior.
- Explicit complex-property adapters for read-only/writable single objects,
  sequences, dictionaries, and implicit content, including recursive save/load,
  derived-item validation, direct-base pointer adjustment, dictionary Id keys,
  namespace-prefixed property elements, and custom save filtering.
- `LoadContext` conversion/type/property failures now name the offending XML
  element or attribute. Tests lock down malformed DOM input, intentionally
  ignored unknown scalar attributes, rejected unknown roots/complex properties,
  incompatible nested types, and duplicate dictionary Id keys. Each mapping
  records its exact registered type; Project loading additionally rejects
  malformed/non-Project documents and duplicate non-empty, case-sensitive IDs
  on distinct mapped `BaseObject` instances (`DEV-044`).
- Failed root or nested loads transactionally remove every `ObjectsNodes`
  entry added by that attempt. Successful context-created mappings retain their
  objects, and `LoadedDocument` owns the parsed DOM plus the transferred mapping
  suffix. Ignored-but-successfully-loaded content therefore remains alive and
  node pointers outlive the loader (`DEV-028`, `DEV-042`).
- A central `RegisterMyraTypes` table for all currently ported MML objects:
  `BaseObject`, `Widget`, abstract content/container/stack bases, `Panel`,
  Grid, horizontal/vertical stack panels, `Image`, the abstract/concrete
  separator and ProgressBar hierarchies, `ButtonBase`, concrete `Button` and
  `ToggleButton`, `Proportion`, `ExportOptions`, and `Project`. It records
  inherited default overrides, content/proportion adapters, and eagerly
  initializes the Grid/StackPanel attached-property declarations.
- The dependency-safe P4-020a `Project` core preserves the upstream
  `Project.ExportOptions` plus implicit root XML shape, path metadata, built-in
  legacy container aliases, and special Grid/StackPanel default-proportion save
  filtering. Built-in and explicit registry/codec overloads load and save the
  current widget catalog. Loaded projects own their source DOM and mapping
  handles, but clear the root factory handle from their own mapping vector to
  avoid a `shared_ptr` self-cycle (`DEV-043`). Stylesheet application, asset
  resolution, stylesheet-aware object construction, and code generation remain
  pending. P4-020b additionally ports single-object editor semantics: owned
  loads resolve legacy/proportion-property tags, and scalar-only saves preserve
  tag overrides plus parent attached-property context.
- `Utility::UIUtils` visible depth-first traversal and exact stable upstream
  bubble-sort ordering; `Widget` Z-index snapshots now consume the shared helper.
- P5-020 layout coverage now spans margins, borders, padding, explicit and
  min/max sizing, horizontal/vertical alignment, invisible subtrees, opacity,
  stable Z-order, and Widget-level translation/scale/rotation/origin transforms.
  The transform integration test also verifies inverse conversion and cache
  invalidation after a property change.
- `Utility::PathUtils` lexical absolute-to-relative filesystem conversion, with
  original-input fallback and no requirement that target paths already exist.
- Header-only `Utility::Rest` table cloning and column sorting, mapped to
  type-safe homogeneous C++ rows. Pointer-like values retain upstream's shallow
  element-copy behavior, and invalid jagged columns are rejected before sorting
  can mutate the table (`DEV-029`).
- `Utility::CrossEngineStuff` maps upstream color multiplication directly to
  CNA `Color::Multiply`, reads view size through the checked environment device,
  creates live-device textures, and converts validated row-major RGBA8 regions
  to CNA's typed `Color` upload API (`DEV-031`). CNA-dependent definitions are
  linked only when the sibling CNA target is available.
- CNA-linked builds now expose `MyraEnvironment`'s checked non-owning `Game` and
  `GraphicsDevice` properties. Replacement removes old event subscriptions;
  ordinary explicit disposal and RAII game destruction clear the environment,
  and custom lifecycle arrangements have an explicit `ClearGame()` fallback
  (`DEV-030`). CNA-dependent definitions live in a separate linked-only source
  so the headers-only compile-check configuration keeps its existing link surface.
- `MyraEnvironment` also carries the upstream event model, four debug-frame
  flags, clipping override, widget-driven/default/current cursor settings, and
  all twelve `MouseCursorType` mappings through CNA `Mouse::SetCursor`. Pure
  settings remain available in deferred-linkage builds; cursor application is
  isolated with the CNA lifecycle definitions.
- `Utility::InputExtension` maps CNA `Keys` to the selected upstream US-keyboard
  character set, including shifted digits/OEM punctuation, number-pad keys, and
  control characters, with the required MonoGame.Extended dual attribution.
- `Graphics2D::IBrush` and `IImage` establish the rendering contracts without
  owning the caller's `RenderContext`; the image contract preserves a by-value
  CNA `Point` size and the brush helper supplies upstream's white tint.
- CNA-linked builds now have a graphics-only `RenderContext` over `SpriteBatch`:
  checked Begin/End/Flush and Dispose state, viewport-relative scissor updates,
  explicit scissor-enabled rasterizer application, nearest/linear/anisotropic
  batch boundaries, all texture Draw overloads, transforms, opacity, rotation,
  scale, and depth. Its dual-attributed shape surface now covers filled/outlined
  rectangles, polygons, lines, points, circles, and arcs with selected-upstream
  draw order and quirks plus deterministic numeric checks (`DEV-038`).
  Text/rich-text overloads remain correctly gated on Phase 3.
- `TextureRegion` retains its exact CNA texture object through `shared_ptr`,
  supports whole/relative bounds and nullable names, and draws through the
  context (`DEV-032`). `ColoredRegion` similarly retains a non-null mutable
  region and preserves upstream channel tinting (`DEV-033`).
- The dependency-free `TintedRegion` core retains an immutable region, exposes
  size/color, preserves reference-identity equality and exact channel tinting,
  and has deterministic hash coverage (`DEV-034`). Its `ToString()` remains
  gated only on the audited `ColorStorage` contract. `NinePatchRegion` now
  constructs/draws all available slices, preserves upstream's small-destination
  positioning, and rejects undefined C++ integer overflow (`DEV-035`).
- `TextureRegionAtlas` retains its texture and region objects, implements the
  upstream indexer/lookup and `atlas:region` split rules, and round-trips regular
  and nine-patch entries through sharp-runtime XML. Duplicate IDs and unknown
  element names preserve selected-upstream replacement/regular-region behavior;
  malformed attributes and null handles fail deterministically (`DEV-036`).
- The font-independent `DefaultAssets` core creates and retains a 1x1 opaque
  white CNA texture, recreates an externally disposed texture, and clears its
  cache on Game replacement/disposal without invalidating external owners
  (`DEV-037`). This also enables `SolidBrush`'s color/property and exact tinting
  core; its ColorStorage string surface and stylesheet-selected white region
  remain explicitly deferred.
- The complete `Widget.Children.cs` surface: stable snapshot traversal, recursive
  typed/untyped search, ID lookup, filtering, visibility-aware descendant counts,
  one-parent reparenting, and explicit ancestor-cycle rejection. The visible-count
  implementation corrects the pinned upstream stale-cache/invisible-child defect.
- Widget now retains all five background/border visual states and render
  callbacks, draws its box decoration, traverses a locally retained child
  snapshot with composed transforms/opacity, and applies upstream's unrotated
  culling/clipping behavior. Caller transform, opacity, and scissor state are
  restored even when user rendering throws (`DEV-045`); dependency-deferred
  builds provide explicit throwing renderer stubs rather than unresolved vtables.
- The FNA-selected `Image` core retains all five `IImage` visual states,
  measures their maximum dimensions, draws the current state with tint and
  resize behavior, and clones the exact type while sharing managed renderable
  handles. Its selected-upstream aspect formula is preserved, with deterministic
  zero-height/overflow diagnostics (`DEV-046`). Resize and external-image MML
  metadata are registered; Color text and ImageStyle remain dependency-gated.
- `SeparatorWidget`, `HorizontalSeparator`, and `VerticalSeparator` now provide
  orientation-specific thickness measurement, upstream alignment defaults,
  inherited `Image` rendering, exact-type clones, and abstract/concrete MML
  metadata. A dynamic thickness change invalidates cached measurement instead
  of retaining the selected upstream's stale result (`DEV-047`); stylesheet
  construction/application remains P6-009–P6-011/P8-004.
- `ProgressBar`, `HorizontalProgressBar`, and `VerticalProgressBar` now provide
  upstream epsilon/event state, range clamping, alignment defaults, exact-type
  clones, orientation rendering, and external-filler MML metadata. Fillers are
  retained across reentrant drawing and unsafe float-to-int extents fail
  deterministically (`DEV-048`). The pinned vertical implementation fills from
  the top despite its bottom-to-top comment; stylesheet integration remains
  P6-021/P8-005. `Myra.hpp` now exports Image, separator, and ProgressBar types.
- Versioned Widget dirty state preserves measure/arrange invalidations raised
  during virtual layout callbacks or `ArrangeUpdated`; enabled-state propagation
  holds a local child snapshot so reentrant tree refresh cannot invalidate C++
  iteration.
- Widget, Panel, StackPanel, and `UIUtils` retain local child snapshots across
  virtual layout/traversal callbacks. Add/remove callbacks see an already-dirty
  tree cache, callback arguments stay alive, and `ClearChildren` safely handles
  reentrant removal or addition without invalidating C++ iterators.
- The dependency-free Widget API also includes throwing `EnsureWidgetById` and
  the protected `SuppressInvalidateMeasure` batching contract.
- Widget's dependency-free behavior-property subset now covers drag direction,
  draggable state, recursively inherited optional mouse cursors, optional
  tooltips, modal/pressed/clipping/focus-acceptance flags, and cancelable
  user-driven pressed changes. The internally controlled keyboard-focus state
  raises the upstream change event and is ready for future Desktop integration.
  The XML registry exposes the upstream-visible drag, cursor, tooltip, and
  clipping properties; Desktop-, style-, brush-, and input-dependent properties
  remain deferred.
- Widget also retains nullable `StyleName` values for future stylesheet lookup,
  exposes the upstream self-defaulting non-owning `DragHandle`, maps arbitrary
  `Tag` data to `std::any`, and provides border/background box bounds needed by
  future rendering. Named styles already survive MML round trips even though
  applying them remains Phase 8 work (`DEV-027`).
- Abstract `ContentControl` now carries the `Widget` + `IContent` inheritance
  contract, inherited single-widget MML content adapter, and null-safe deep-copy
  override (`DEV-041`).
- No-argument `Widget::Clone()` constructs through a protected virtual factory,
  requires a new non-null exact dynamic type, copies the ported base/attached
  state, and remaps a self drag handle to the clone (`DEV-040`). Current concrete
  containers supply factories; `Container` deep-clones stable child snapshots,
  while Grid and stack panels copy their currently ported type-specific state.
- Standalone input propagation tests cover capture FIFO, bubble LIFO, and
  stop-propagation filtering in both modes, including the selected upstream's
  second reversal of surviving bubble-stack entries during a rebuild. Queue
  entries own non-null processor handles until dispatch or filtering so tree
  mutation cannot leave a dangling event target (`DEV-039`).
- Widget now publicly exposes the selected upstream mouse/touch virtual hooks
  and events, plus direct `OnKeyDown`/`OnKeyUp` forwarding through typed CNA
  `Keys` arguments. Queued pointer dispatch preserves hook-before-event order
  and retains the widget through reentrant delivery via the existing DEV-039
  ownership contract. Desktop hit testing/positions, wheel state,
  tooltip/cursor/hover behavior, double-click detection, drag tracking, and
  character input remain in their dependency tasks.
- `ButtonBase` now supplies the abstract style-independent press/click state
  machine used by future button controls: `ReadOnly`, `DoClick`, internal touch
  hooks, `Click`, pressed-state cloning, and MML metadata. Clones retain
  `ReadOnly` instead of silently becoming interactive as in the selected
  upstream omission (`DEV-049`); the armed-click flag remains transient.
- Concrete `Button` now owns a `SingleItemLayout<Widget>`, drives pressed state
  on touch down/up/left, performs upstream Space-key activation after the
  ordinary key event, deep-clones its content through the existing
  ContentControl path, and is a concrete MML type. Its internal
  `ReleaseOnTouchLeft` default is retained for future Slider/SplitPane use.
- Concrete `ToggleButton` now owns the same single-item content layout while
  mapping `IsToggled` directly to the inherited pressed state and exposing
  `IsToggledChanged` as a true reference alias of `PressedChanged`. Touch-down
  and Space toggle through the cancelable user-change path, touch-up still
  clicks, cloning is exact and deep, and MML round trips the concrete type.
  Tests deliberately pin the selected upstream's keyboard quirk: Space can
  toggle a read-only control, while disabled controls remain guarded.
- Abstract `CheckButtonBase` now owns a horizontal check/content layout with
  configurable order and spacing, retained checked/unchecked images, a
  read-only nested check widget, touch/Space toggling, exact deep clone state,
  a `CheckPosition` codec, and MML metadata. Direct images refresh with pressed
  state and survive cloning (`DEV-050`), and spacing changes invalidate cached
  measurement (`DEV-051`). The selected upstream quirk allowing Space while
  read-only remains covered; hover-derived visual selection and styles remain
  explicitly deferred.
- Checked float-to-integer conversion for layout/transform values and checked Grid
  spacing/size/location accumulation. Grid now rejects non-finite/out-of-range
  proportions and non-positive spans deterministically, while restoring upstream's
  epsilon-zero rule for tiny aggregate `Part` weights.
- Checked integer arithmetic across `Thickness`, `LayoutUtils`, and Widget's
  measure/arrange/transform path. Overflow now fails deterministically instead of
  invoking signed-integer undefined behavior, point containment widens edge sums,
  and Widget opacity rejects NaN as well as out-of-range finite values.

The widget work is deliberately partial: drawing traversal is complete, while
desktop propagation, hover/tooltip integration, style application, hit testing,
and input dispatch remain open. `UPSTREAM_MANIFEST.md` records this per source.

## Whole-port progress estimate

As of 2026-08-24 after P5-012, `plan.md` has **169/313 checked tasks
(54.0%)**. Equal checkbox counting overstates
end-user parity because the
largest remaining workstreams are font/rich text, Desktop/input, most controls,
styles/default assets, selectors/windows/dialogs, DataGrid/PropertyGrid, and the
exhaustive release gate. The feature-weighted estimate is therefore **about
30–35% of the complete Myra-CNA port**.

All currently known technical work through P10-028 is estimated at
**1,106–1,952 focused implementation/validation hours remaining**; use about
**1,529 hours** as the planning midpoint or **1,100–2,000 hours** as the rounded
range. This includes code, tests, documentation, integration, and the known
project-owned test-fixture work. It assumes P3-004 and P0-015b receive prompt
human decisions and excludes idle waiting/legal-review time. Choosing wholly
original default-skin artwork under P0-015b would add approximately **80–200
specialist art hours**. The phase-by-phase derivation and assumptions are in
`plan.md` section 1.4; recalculate after major phases or blocker decisions.
These are human-equivalent focused engineering hours. For quota/session
planning, the separate rough estimate remains **100–200 active Codex hours**
(midpoint about 150) to finish the technical port, with high uncertainty from
build/test latency, context turnover, and future compatibility discoveries.

## Latest validation

The default and linked SOFTWARE configurations are broadly green with the
current worktree. The latest sanitizer checkpoint is also broadly green for
all tests that existed at that checkpoint:

```bash
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure --parallel 3
# 64/64 tests passed

cmake -S ../cna -B build-cna-parent \
  -DCMAKE_PROJECT_CNA_INCLUDE="$PWD/cmake/AddMyraCnaToCnaBuild.cmake" \
  -DCNA_BUILD_TESTS=OFF -DCNA_BUILD_EXAMPLES=OFF \
  -DCNA_GRAPHICS_RENDERER=SOFTWARE \
  -DMYRA_CNA_BUILD_TESTS=ON -DMYRA_CNA_BUILD_EXAMPLES=ON
CCACHE_DISABLE=1 cmake --build build-cna-parent --parallel 3
ctest --test-dir build-cna-parent/_myra_cna \
  --output-on-failure --parallel 3
# 294/294 tests passed with current modular CNA/sharp-runtime

ASAN_OPTIONS=detect_leaks=0 CCACHE_DISABLE=1 \
  cmake --build build-sanitize-parent --parallel 3
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 \
  ctest --test-dir build-sanitize-parent/_myra_cna \
  --output-on-failure --parallel 3
# 250/250 tests passed with ASan address checks and UBSan

# focused CheckButtonBase/ButtonBase/registry/codec validation: 19/19 passed

# P5-012 Container completion: focused linked Panel 4/4, broad default 64/64,
# and broad linked SOFTWARE 294/294 passed

# focused UIUtils validation after that broad run: 3/3 passed
# focused PathUtils validation after that broad run: 3/3 passed
# focused Widget validation after the child-tree completion: 9/9 passed
# focused numeric/Grid/transform validation after P5-020a: 15/15 passed
# focused headers-only Mathematics validation after P5-020a: 3/3 passed
# focused headers-only TypeRegistry validation after P4-012b: 6/6 passed
# focused linked registry/BaseContext/XML validation after P4-012b: 16/16 passed
# focused Widget validation after P5-008b/P5-008c: 13/13 passed
# focused headers-only attached-registry validation after P4-014b: 3/3 passed
# focused linked attached-registry/MML validation after P4-014b: 11/11 passed
# focused linked Thickness/LayoutUtils/Widget validation after P5-020b: 21/21 passed
# focused headers-only InputEventsManager validation after P5-021a: 4/4 passed
# focused linked reentrant Widget/Panel/StackPanel/UIUtils validation after P5-008d: 26/26 passed
# focused linked Widget behavior/MML validation after P5-008e: 3/3 passed
# focused linked Widget keyboard-focus validation after P5-008f: 1/1 passed
# focused ContentControl contract/metadata validation: default 2/2, linked 3/3 passed
# focused linked Widget data/bounds/MML validation after P5-008g: 4/4 passed
# focused linked MML parser/diagnostic validation after P4-021a: 1/1 passed
# focused linked MML transactional-mapping validation after P4-017d: 7/7 passed
# focused Rest validation after P1-016: default 3/3, linked 3/3 passed
# focused linked CrossEngineStuff color validation after P1-013a: 2/2 passed
# focused linked ASan+UBSan destruction/reparenting validation: 7/7 passed
# focused linked MyraEnvironment validation after P2-001/P2-002: 5/5 passed
# focused linked CrossEngineStuff validation after P1-013: 6/6 passed
# focused MyraEnvironment settings after P2-004: default 2/2, linked 2/2 passed
# focused linked MyraEnvironment lifecycle/cursor validation after P2-003: 7/7 passed
# focused graphics-contract validation after P2-006/P2-007: default 1/1, linked 1/1 passed
# focused linked RenderContext validation after P2-016–P2-019: 8/8 passed
# focused linked TextureRegion validation after P2-009: 4/4 passed
# focused linked ColoredRegion validation after P2-010: 3/3 passed
# focused linked TintedRegion core validation after P2-011a: 4/4 passed
# focused linked NinePatchRegion validation after P2-012: 4/4 passed
# focused linked TextureRegionAtlas validation after P2-013: 5/5 passed
# default build/header compile after P2-013: passed
# focused linked DefaultAssets/MyraEnvironment validation after P2-015: 12/12 passed
# focused linked SolidBrush core validation after P2-008a: 3/3 passed
# default build/header compile after P2-008a/P2-015: passed
# focused linked RenderContext.Shapes validation after P2-020: 7/7 passed
# default build/header compile after P2-020: passed
# focused linked RenderContext validation after P2-021: 9/9 passed
# linked HEADLESS graphics subset after P2-022: 44/44 passed
# linked HEADLESS full suite after P2-022: 172/172 passed
# linked SDL_RENDERER/Xvfb graphics subset after P2-022: 44/44 passed
# linked SDL_RENDERER/Xvfb full suite after P2-022: 172/172 passed
# focused retained InputEventsManager validation after P5-007b:
# default 7/7, linked SOFTWARE 7/7, linked ASan+UBSan 7/7 passed
# focused Widget clone/deep-copy validation after P5-008h:
# linked SOFTWARE 6/6, linked ASan+UBSan 6/6 passed; default build passed
# focused owned-document/retained-mapping validation after P4-017c: 9/9 passed
# focused Project/MML/registry validation after P4-020a/P4-021:
# linked SOFTWARE 18/18, linked ASan+UBSan (leaks disabled) 18/18 passed;
# default/header build passed
# focused Project validation after P4-020b: linked SOFTWARE 9/9 passed;
# default/header build passed
# focused Widget/Transform/LayoutUtils/UIUtils validation after P5-020:
# linked SOFTWARE 24/24, linked ASan+UBSan (leaks disabled) 24/24 passed
# P0-003 default configure/build and 61/61 tests passed; linked SOFTWARE and
# ASan+UBSan broad suites passed 193/193. clang-format/IWYU are not installed on
# this host; default missing-tool handling and IWYU's requested-mode fatal
# diagnostic were exercised.
# focused comparison-helper validation after P0-019: linked SOFTWARE 6/6 and
# linked ASan+UBSan (leaks disabled) 6/6 passed
# focused temporary-asset helper validation after P0-020: default, linked
# SOFTWARE, and linked ASan+UBSan (leaks disabled) 3/3 passed in each;
# broad suites subsequently passed 64/64, 198/198, and 198/198
# P0-022 SDL_RENDERER smoke on Xvfb :106: 1/1 passed with exact red/green
# readback; the fully rebuilt current SDL_RENDERER suite passed 199/199
# P3-001/P3-002/P3-003/P3-018 are documentation/provenance-only changes;
# git diff --check and local Markdown/link/path consistency checks passed
# P0-014/P1-020 source-lineage audit: 189 files, inventory SHA-256
# 11bfaad70b94bce29929ed31cd064992ef5479a33ab43eb82b1b873ab98c1850;
# provenance CTest 1/1 and git diff --check passed
# P0-015 asset audit: all seven resource hashes verified; Inter embedded source
# commit and OFL text verified; VisUI comparison was 202/207 exact raw rasters
# P0-016 test-asset audit: 35/35 paths and 906457 bytes classified; sorted
# manifest SHA-256 8d6ae480b533b3dc1892fc2ff386db7a9056e5127e9d8c40455df6d58cb7291f;
# pinned libGDX PNG and atlas matched exactly, normalized BMFont differed only
# by Myra's atlas-page reference; AOSP DroidSans comparison exposed 33 differing
# bytes/build 112 vs 113, so the test binary is replacement-only
# P5-008i Widget renderer: focused linked SOFTWARE 7/7; broad default 64/64,
# linked SOFTWARE 205/205, and ASan+UBSan 205/205 passed. Sanitizer configure-time
# test discovery requires ASAN_OPTIONS=detect_leaks=0 on this ptrace host too.
# P6-001a Image core: focused Image/registry/codec 15/15; broad default 64/64,
# linked SOFTWARE 212/212, and ASan+UBSan 212/212 passed
# P6-009a separator core: focused separator/Image/registry/codec 20/20;
# broad default 64/64, linked SOFTWARE 217/217, and ASan+UBSan 217/217 passed
# P6-021a ProgressBar core: focused ProgressBar/registry 10/10; broad default
# 64/64, linked SOFTWARE 225/225, and ASan+UBSan 225/225 passed
# P5-010a Widget input hooks/events: focused WidgetInput/InputEventsManager/Widget
# linked SOFTWARE 28/28 and ASan+UBSan 28/28; broad default 64/64, linked
# SOFTWARE 229/229, and ASan+UBSan 229/229 passed
# P6-003a ButtonBase core: focused ButtonBase/WidgetInput/registry 10/10 in
# linked SOFTWARE and ASan+UBSan; broad default 64/64, linked SOFTWARE 233/233,
# and ASan+UBSan 233/233 passed
# P6-004a Button core: focused Button/ButtonBase/registry 11/11 in linked
# SOFTWARE; broad default 64/64, linked SOFTWARE 238/238, and ASan+UBSan
# 238/238 passed
# P6-005a ToggleButton core: focused ToggleButton/ButtonBase/registry 13/13 in
# linked SOFTWARE; broad default 64/64, linked SOFTWARE 244/244, and ASan+UBSan
# 244/244 passed
# P0-023/P6-006a: focused CheckButtonBase/ButtonBase/registry/codec 19/19;
# broad default 64/64, modular linked SOFTWARE 250/250, and modular ASan+UBSan
# 250/250 passed against CNA fb3728267 / sharp-runtime 81624983
# MYRA_CNA_LINK_CNA=ON against that modular checkout fails immediately with
# the documented CMAKE_PROJECT_CNA_INCLUDE migration diagnostic
# implementation checkpoint 6ff6fa1 revalidated: default 64/64 and
# ASan+UBSan 198/198 passed before commit
```

All compilation commands must continue to use at most three workers.
The sanitizer build used GCC 14 Debug instrumentation with
`-fsanitize=address,undefined -fno-omit-frame-pointer`. Leak detection alone
was disabled because LeakSanitizer cannot run under this environment's
`ptrace`; ASan address checks and UBSan remained active.

## Current autonomous priority

1. Treat P3-004 as `needs_human`: do not add FontStashSharp, stb, FreeType,
   HarfBuzz, or another font source until the layout/rasterizer, untrusted-font,
   and shaping choices in `docs/font-audit.md` are explicitly approved.
2. Treat P0-015b as `needs_human`: do not copy the VisUI-derived skin atlas or
   raw artwork. Inter itself is provenance-cleared under OFL but packaging waits
   for P3-004/P3-020.
3. P0-016 is complete. Keep every upstream test binary unbundled; execute
   P0-016a's original PNG/BMFont/stylesheet replacements only when their
   affected asset tests become implementable.
4. P0-023, P5-008i, P5-010a, P5-012, P6-001a, P6-003a, P6-004a, P6-005a,
   P6-006a, P6-007a, P6-008a, P6-009a, P6-018a, P6-019a, P6-021a, P6-022a, P7-001–P7-005a, P7-006, P7-008, and P7-012a–P7-016a are complete. Keep full
   P6-006/P6-007/P6-008 hover/style work in P5-010/P8-003, and keep Slider's
   Desktop drag/wheel/style work in P5-016/P5-019/P8-005; do not reopen P6-005's Label/style-
   dependent remainder before P6-002/P8-003. TreeView now owns a reciprocal node
   registry and selection/key core; Desktop mouse/touch hit testing, hover/brush
   rendering, styles, and MML registration remain P5-016/P7-016/P8-004. Choose
   the next dependency-safe widget milestone only after an upstream/source and
   registry audit. TabItem currently stops before Color/Label/Button wiring;
   those paths remain P3-004/P6-002/P7-012/P7-013/P8-004.
5. Keep P4-019 open for types added by future Phase 5–9 work; every MML-capable
   type currently in the repository is registered and round-trip tested.
6. Continue layout work only with a coherent next dependency. Do not represent partial
   renderer, font, desktop, input, or Grid selection/style support as complete.

## Known limitations and decisions

- `sharp-runtime` reflection remains unusable by design. MML needs the
  project-local explicit type/property registry described in `plan.md`.
- `InputEventsManager` now requires non-null shared processor ownership and
  retains every event target through dispatch or filtering (`DEV-039`). Future
  Widget input code must enqueue its owning handle; the separate widget-to-
  Desktop link remains non-owning as required by the project ownership model.
- `CreateAndLoadDocument` owns sharp-runtime's non-movable DOM and transfers its
  load mappings into the returned result; mappings retain every context-created
  object (`DEV-042`). The element-based `CreateAndLoad` and raw `Load` overloads
  deliberately remain low-level: callers must keep their DOM/external target
  alive. Failed mapping suffixes roll back transactionally (`DEV-028`).
- `Project::LoadFromXml` adopts that owned document/mapping suffix and clears
  the retained root-Project handle inside its own mappings, preventing a
  permanent ownership cycle while retaining every nested created object
  (`DEV-043`). `StylesheetPath` and `DesignerRtfAssetsPath` are currently
  round-tripped metadata only; they do not imply unported style/asset behavior.
- Widget deep cloning now keeps public `Clone()` no-argument and delegates
  construction to an exact-type virtual factory (`DEV-040`). Every future
  concrete widget must override `CreateCloneInstance()`; omission or an invalid
  result fails deterministically instead of slicing. ContentControl and
  Container own deep clones of their present content/children (`DEV-041`).
- Dynamic TTF/rich-text support is not selected. The completed
  `docs/font-audit.md` records the active FNA API, exact zlib-licensed upstream
  revisions, alternatives, and recommendation. P3-004 remains `needs_human`;
  do not incorporate a font/layout/rasterizer source until that explicit choice.
- FontStashSharp 1.5.6 mixes UTF-16 code-unit substring positions with
  code-point counts, while Myra TextBox also uses C# string indices. P3-006 must
  define explicit UTF-8 byte/code-point/grapheme mapping and astral-plane tests;
  silently changing the public cursor model to graphemes is not authorized.
- P6-024 is also downstream of that index-domain decision: `UndoRedoStack`
  captures deleted/replaced text with C# `Substring(where, length)`. Do not
  translate those positions to `std::string` byte offsets before P3-006.
- P2-014 is intentionally coupled to P3-016: upstream AssetManagementBase has a
  strong, recursive, settings-aware cache, whereas CNA ContentManager's
  Texture2D cache deliberately retains only weak backend handles. Do not add a
  competing Myra-only texture cache before the complete asset-manager adapter
  defines object identity and recursive-load behavior.
- The MML Color serializer is deliberately absent: upstream delegates named
  color handling to FontStashSharp `ColorStorage`. Its implementation and zlib
  licence are now audited, but incorporating/translation remains blocked on the
  explicit P3-004 source decision. Existing serializers still cover Vector2,
  Thickness, and Rectangle.
- A disengaged `std::optional<T>` has no scalar XML representation and is
  omitted by a save context; direct codec serialization rejects it. Any present
  XML attribute deserializes to an engaged optional. Enum names remain
  case-sensitive like upstream `Enum.Parse`, with numeric fallback retained.
- XML contexts reject complex properties that lack an explicit adapter. Unknown
  regular scalar attributes remain ignored like upstream; malformed, unresolved,
  or type-incompatible complex/attached values fail explicitly. `BaseObject`
  user data is load-only because the selected upstream `SaveContext` does not
  serialize it. Adapter enumeration must report the exact dynamic object address;
  the tests cover a multiple-inheritance derived item with nonzero base offset.
- `Inter-Regular.ttf` is traced to exact Inter source commit
  `06b166889e335a2454c0767734a05b27f6403098`, and the complete OFL 1.1 notice
  is in `THIRD_PARTY_NOTICES.md`; no binary is copied yet because P3-004/P3-020
  still determine font integration and packaging.
- The default atlas is a separate blocker. Myra acknowledges VisUI, and 202 of
  207 pinned raw rasters are byte-identical to the audited VisUI revision.
  VisUI's Apache licence plus additional NOTICE/icon terms are not interpreted
  autonomously. P0-015b requires documented permission/compliance or original,
  clearly licensed replacement artwork. The current white-region primitive is
  independently created and does not use this atlas.
- All 35 pinned Myra.Tests assets are now classified in
  `docs/test-assets-audit.md`. Do not copy `MonoGameLogo.png`, the Arial BMFont
  pair, the Commodore64 bundle, or the unmatched DroidSans build; their tests
  must use P0-016a replacements. The exact libGDX bundle is Apache-2.0-derived
  and may be copied only after its complete notice is added. General Myra XML
  may be reused after blocked asset paths are rewritten.
- Do not edit sibling `cna`, `cna-extended`, or `sharp-runtime` repositories.
  If one proves to need a modification, record the need here and request a
  human decision rather than changing it.
- CNA `fb3728267` uses top-level `CMAKE_SOURCE_DIR` paths and a root
  source-partition guard, so it cannot safely be embedded through the legacy
  `MYRA_CNA_LINK_CNA=ON` child path. Use the documented
  `CMAKE_PROJECT_CNA_INCLUDE` driver; CTest must run from the generated
  `_myra_cna` directory when `CNA_BUILD_TESTS=OFF`. This is a build-topology
  constraint, not a request to modify the sibling repository.
- `MyraEnvironment` is deliberately non-owning. Normal CNA `Game::Disposed`
  and `GraphicsDevice::Disposing` paths clear it, including ordinary stack/RAII
  destruction. A custom graphics-device service that outlives a game destroyed
  without raising either event must call `ClearGame()` first (`DEV-030`).
- `CrossEngineStuff::SetTextureData` validates region arithmetic and RGBA byte
  length before converting to CNA's typed `Color` upload (`DEV-031`). It ignores
  trailing bytes like upstream's explicit element count.
- `Widget::AddChild` deliberately reparents a child to preserve one owning
  parent. This necessary C++ difference is `DEV-011` and is covered by tests.
- Widget `DragHandle` is a checked-by-convention non-owning pointer and `Tag` is
  `std::any` (`DEV-027`), following the repository-wide ownership boundary;
  normal handles are self or another widget retained by the same tree.
- Widget insertion also rejects ancestor cycles (`DEV-017`), and visible-only
  descendant counting corrects the selected upstream cache/count defect
  (`DEV-018`). Recursive query APIs retain local snapshots so callbacks may
  mutate or refresh the live child list safely.
- A child's `ZIndex` change now invalidates its parent's cached ordering
  (`DEV-019`); the pinned upstream otherwise retains stale traversal/render order
  after the first snapshot until an unrelated collection mutation.
- `Widget::getBoundsProperty()` now matches upstream's zero-based bounds instead
  of exposing the internal parent-positioned layout rectangle; parent placement
  remains available through `ContainerBounds` and coordinate transforms.
- Widget measurement caches now key on the caller-supplied available size rather
  than the internally reduced content size, correcting the pinned upstream
  false-miss/false-hit defect (`DEV-020`).
- Widget measure/arrange passes now retain invalidations raised reentrantly from
  virtual callbacks or events, and enabled propagation owns a stable temporary
  child snapshot (`DEV-023`).
- Widget/layout/tree traversal now also owns child snapshots across virtual
  callbacks, and child collection callbacks update cache state before invoking
  user code while retaining callback arguments (`DEV-026`).
- Grid and StackPanel are still layout subsets: styles, debug rendering,
  selection, and input remain unported. Their `ProportionCollection` now maps
  the upstream observable reference collection to sharp-runtime
  `ObservableCollection<std::shared_ptr<Proportion>>`; see accepted `DEV-014`.
- The separator hierarchy is complete only for its dependency-safe core. Its
  style constructors, style dictionary selection, and `SeparatorStyle`
  application remain deferred to P6-009–P6-011/P8-004.
- The ProgressBar hierarchy likewise has no style constructor, style dictionary
  selection, or `ProgressBarStyle` application yet; those remain
  P6-021/P8-005. Its retained `Filler` is already usable directly and through
  caller-provided MML external-asset callbacks.
- `Button` is complete only for P6-004a's style-independent core. Stylesheet
  constructors/dictionary lookup and Label-dependent `CreateTextButton` remain
  P6-002/P6-004/P8-003. The Desktop-wide touch-up subscription used when the
  internal `ReleaseOnTouchLeft` flag is false remains P5-016/P6-004; the flag is
  currently private to future Slider and SplitPane friends and defaults true.
- `ToggleButton` is complete only for P6-005a's style-independent core.
  Stylesheet construction/dictionary lookup and Label-dependent convenience
  creation remain P6-002/P6-005/P8-003. The selected upstream behavior is
  intentionally preserved: touch interaction honors `ReadOnly`, but Space
  still toggles a read-only control; disabled controls reject both paths.
- `CheckButtonBase` and concrete `CheckButton` are complete only for their
  P6-006a/P6-007a style-independent cores.
  Its public direct images, layout, input, cloning, enum codec, and MML metadata
  are usable, but `CheckImageInternal` cannot inherit the parent's hover state
  until P5-010 adds `IsMouseInside`; style construction/application remains
  P6-006/P6-007/P6-008/P8-003. `RadioButton` now has its style-independent
  direct-sibling selection core, cloning, and MML registration; its stylesheet
  construction/application remains deferred.
- Grid layout rejects active null proportions and invalid
  negative/out-of-measured-range coordinates or spans before they can become
  C++ indexing/dereference UB; StackPanel's intentional null-default fallback
  remains supported while null explicit entries fail deterministically. This
  managed-exception mapping is `DEV-021`.
- Float-to-integer layout and transform conversions reject non-finite or
  out-of-range values, and Grid's integer accumulation rejects overflow instead
  of relying on undefined C++ arithmetic (`DEV-022`). Valid conversions retain
  upstream truncation or midpoint-to-even rounding as appropriate.
- Thickness dimensions/rectangle subtraction, alignment offsets, and Widget's
  box-model and transform sums also reject integer overflow; containment widens
  edge sums and opacity rejects NaN (`DEV-025`).
- The global attached-property registry serializes concurrent creation/query
  access and rejects a duplicate exact owner/name declaration (`DEV-024`),
  preventing C++ data races and ambiguous XML lookup.

## Recommended next starting point

Read this file first, then inspect `plan.md` against the current source. This
handoff is broadly validated in headers-only, current modular linked
SOFTWARE-CNA, and current modular linked ASan+UBSan configurations. The earlier
P2-001/P2-002 contract
has focused linked validation, P1-013's full viewport/texture bridge passes all
six focused tests, P2-003/P2-004 cursor/configuration coverage passes in its
applicable linked and headers-only modes, and P2-006/P2-007 contracts pass in
both modes. The graphics-only P2-016–P2-019 RenderContext spine passes 8/8
focused tests; TextureRegion, ColoredRegion, the TintedRegion core,
NinePatchRegion, and TextureRegionAtlas pass 4/4, 3/3, 4/4, 4/4, and 5/5
respectively. DefaultAssets plus the environment pass 12/12 focused lifecycle
tests, the SolidBrush core passes 3/3, P2-020 shape rendering passes 7/7, and
the expanded P2-021 RenderContext suite passes 9/9 including nontrivial source
rectangles and caller-managed nested clip restoration. P2-022 is also complete:
HEADLESS and SDL_RENDERER-on-Xvfb each passed the 44/44 graphics subset and full
172/172 suite at the P2-022 milestone. The current SDL_RENDERER tree, including
the explicit display smoke and all work since then, passes 199/199 on Xvfb.
The current default, linked SOFTWARE, and ASan+UBSan suites pass
64/64, 250/250, and 250/250 respectively. P6-006a passes all 19/19 focused
CheckButtonBase/ButtonBase/registry/codec tests in the linked build; those paths
are also covered by the complete sanitised run. P6-005a previously passed all
13/13 focused ToggleButton/ButtonBase/registry tests. P6-004a previously passed all
11/11 focused Button/ButtonBase/registry tests in the linked build and complete
sanitised run. P6-003a previously passed all 10/10
focused ButtonBase/WidgetInput/registry tests in both linked and sanitised builds.
P5-010a passes all 28/28 focused WidgetInput/InputEventsManager/Widget tests in
both linked and sanitised builds.
P6-021a passes all 10/10 focused ProgressBar/registry tests, P6-009a passes its 20/20 focused
separator/Image/registry/codec set, P6-001a passes its 15/15 focused subset, and
P5-008i passes all 7/7 focused Widget renderer tests. P5-007b passed 7/7
focused tests in default, linked SOFTWARE, and linked ASan+UBSan
configurations. P5-008h then passed 6/6 focused linked SOFTWARE and ASan+UBSan
tests, and the default build passed. P4-017c subsequently passed all 9/9 focused
MML context tests. P4-020a/P4-021 then passed 18/18 focused
Project/MML/registry tests in linked SOFTWARE and ASan+UBSan configurations
(LeakSanitizer disabled for the host `ptrace` limitation), and the
default/header build passed. P4-020b's single-object helpers subsequently passed
9/9 focused linked SOFTWARE tests and the default/header build. P5-020 then
closed the listed layout/transform behavior matrix with 24/24 focused tests in
both linked SOFTWARE and ASan+UBSan builds. P0-003 subsequently centralized
C++23/strict-warning settings and added documented optional formatting/IWYU
audits. P0-019/P0-020 then added reusable comparison diagnostics and isolated
temporary asset fixtures, with all three broad configurations still green.
P0-022 then added and validated the real SDL_RENDERER/Xvfb pixel smoke. The
subsequent P3-001/P3-002/P3-003 audit is recorded in `docs/font-audit.md`, with
P3-018 manifested and P3-004 explicitly `needs_human`; it changed no compiled
source. P0-014 then completed the all-production-source lineage audit, and
P0-015 completed the default-resource provenance audit. The exact Inter font is
OFL-cleared but still unbundled; the VisUI-derived atlas is explicitly
`needs_human` P0-015b. P0-016 subsequently classified all 35 test assets without
copying them and opened P0-016a for behavior-equivalent project-owned fixtures.
P6-006a/P6-007a/P6-008a are complete; do not start their hover-dependent
`CheckImageInternal` override before P5-010 provides `IsMouseInside`, and keep
their styles in P8. Do not reopen P6-005's Label/style-dependent remainder
until P6-002/P8-003 are implementable. Defer the Desktop-driven hit-test/
hover/drag path and replacement fixtures likewise. If P3-004 is later
approved, begin with P3-005's narrow abstraction and P3-006's explicit index-domain contract
before introducing rasterizer code. P4-019 remains open only for future widget types,
while caller-provided external-asset callbacks are already usable. P5-012 is
complete. The P7-017 audit confirmed that a faithful Window needs Label and the
Desktop show/focus/removal lifecycle; the next coherent dependency-safe target
is therefore a narrowly audited P5-016 Desktop ownership/layout/focus core,
with input polling, text input, styles, menus, tooltips, and drag behavior kept
in their existing downstream tasks.
