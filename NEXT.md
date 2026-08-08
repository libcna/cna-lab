# Myra-CNA continuity

## Current state

- Active branch: `develop`.
- Last validated repository commit before this session: `490b233`
  (`Record observable proportion handoff`).
- This handoff contains coherent, focused-test-validated P4-013a/P4-015
  value-codec/context foundations and complete registry-adapted P4-017/P4-018
  XML contexts described below.
- The authoritative upstream reference remains Myra revision
  `0d79b939310bfe1d00b21803fe15e291caf60aa1` at `/tmp/myra-upstream`.

## Completed foundation

The project has a provenance-checked C++23/CMake library with direct CNA and
sharp-runtime integration modes. Every translated source has Myra attribution
and a manifest entry. The current ported surface includes:

- event arguments, multicast handlers, propagation manager, and invocation
  helpers;
- `Thickness`, `Transform`, `Mathematics`, `ColorHSV`, and string helpers;
- MML `BaseObject`, `IItemWithId`, and `IHasColor`;
- UI contracts, `Widget`'s layout/transform kernel and complete child-tree API,
  abstract `ContentControl`, `Container`, `Panel`, `Proportion`, and
  `SingleItemLayout<T>`.
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
  incompatible nested types, and duplicate dictionary Id keys; Project-wide
  duplicate object-Id validation remains with P4-020/P4-021.
- Failed root or nested loads transactionally remove every `ObjectsNodes`
  entry added by that attempt. A reusable C++ context therefore cannot retain
  raw pointers to objects destroyed while unwinding (`DEV-028`).
- A central `RegisterMyraTypes` table for all currently ported MML objects:
  `BaseObject`, `Widget`, abstract content/container/stack bases, `Panel`,
  Grid, horizontal/vertical stack panels, and `Proportion`. It records inherited
  default overrides, content/proportion adapters, and eagerly initializes the
  Grid/StackPanel attached-property declarations.
- `Utility::UIUtils` visible depth-first traversal and exact stable upstream
  bubble-sort ordering; `Widget` Z-index snapshots now consume the shared helper.
- `Utility::PathUtils` lexical absolute-to-relative filesystem conversion, with
  original-input fallback and no requirement that target paths already exist.
- Header-only `Utility::Rest` table cloning and column sorting, mapped to
  type-safe homogeneous C++ rows. Pointer-like values retain upstream's shallow
  element-copy behavior, and invalid jagged columns are rejected before sorting
  can mutate the table (`DEV-029`).
- The dependency-free portion of `Utility::CrossEngineStuff` now maps upstream
  color multiplication directly to CNA `Color::Multiply`. View-size and texture
  operations remain explicitly dependent on P2's GraphicsDevice lifetime contract.
- `Utility::InputExtension` maps CNA `Keys` to the selected upstream US-keyboard
  character set, including shifted digits/OEM punctuation, number-pad keys, and
  control characters, with the required MonoGame.Extended dual attribution.
- The complete `Widget.Children.cs` surface: stable snapshot traversal, recursive
  typed/untyped search, ID lookup, filtering, visibility-aware descendant counts,
  one-parent reparenting, and explicit ancestor-cycle rejection. The visible-count
  implementation corrects the pinned upstream stale-cache/invisible-child defect.
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
  contract and an inherited single-widget MML content adapter. Its upstream
  deep-copy override remains deferred until Widget cloning has an explicit C++
  factory contract.
- Standalone input propagation tests cover capture FIFO, bubble LIFO, and
  stop-propagation filtering in both modes, including the selected upstream's
  second reversal of surviving bubble-stack entries during a rebuild.
- Checked float-to-integer conversion for layout/transform values and checked Grid
  spacing/size/location accumulation. Grid now rejects non-finite/out-of-range
  proportions and non-positive spans deterministically, while restoring upstream's
  epsilon-zero rule for tiny aggregate `Part` weights.
- Checked integer arithmetic across `Thickness`, `LayoutUtils`, and Widget's
  measure/arrange/transform path. Overflow now fails deterministically instead of
  invoking signed-integer undefined behavior, point containment widens edge sums,
  and Widget opacity rejects NaN as well as out-of-range finite values.

The widget work is deliberately partial: no drawing traversal, desktop
propagation, style application, hit testing, or input dispatch has been
claimed as complete. `UPSTREAM_MANIFEST.md` records this per source.

## Latest validation

Both supported configurations are broadly green with the complete handoff
worktree:

```bash
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure --parallel 3
# 55/55 tests passed

CCACHE_DIR=/tmp/myra-cna-ccache cmake --build build-cna --parallel 3
ctest --test-dir build-cna --output-on-failure --parallel 3
# 114/114 tests passed, with MYRA_CNA_LINK_CNA=ON and SOFTWARE backend

ASAN_OPTIONS=detect_leaks=0 CCACHE_DIR=/tmp/myra-cna-ccache \
  cmake --build build-sanitize --parallel 3
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 \
  ctest --test-dir build-sanitize --output-on-failure --parallel 3
# 114/114 tests passed with ASan address checks and UBSan

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
```

All compilation commands must continue to use at most three workers.
The sanitizer build used GCC 14 Debug instrumentation with
`-fsanitize=address,undefined -fno-omit-frame-pointer`. Leak detection alone
was disabled because LeakSanitizer cannot run under this environment's
`ptrace`; ASan address checks and UBSan remained active.

## Current autonomous priority

1. Start the next context by auditing CNA `Game`/`GraphicsDevice` destruction
   and completing P2-001/P2-002's explicit non-owning environment contract.
   Then finish `CrossEngineStuff` view-size/texture helpers (P1-013) before the
   RenderContext spine. The already-ported color path is P1-013a.
2. Keep `README.md`, `plan.md`, and this file synchronized with the actual
   partial-widget implementation.
3. Keep P4-019 open for types added by future Phase 5–9 work; every MML-capable
   type currently in the repository is registered and round-trip tested.
4. Continue layout work only with a coherent next dependency. Do not represent partial
   renderer, font, desktop, input, or Grid selection/style support as complete.

## Known limitations and decisions

- `sharp-runtime` reflection remains unusable by design. MML needs the
  project-local explicit type/property registry described in `plan.md`.
- `InputEventsManager` currently queues non-owning processor pointers, unlike
  the managed queue's retained references. No ported widget implements that
  processor interface yet. P5-007b is `needs_human`: settle the public lifetime
  contract before P5-010 connects widgets to the queue.
- `LoadContext::ObjectsNodes` currently exposes non-owning pointers to elements
  in the caller's non-movable sharp-runtime `XmlDocument`. P4-017c is
  `needs_human`: select an owning snapshot/serialized-source/caller-retention
  contract before P4-020 transfers this mapping into a longer-lived `Project`.
  Failed loads already roll back object/node entries transactionally, so this
  unresolved success-path document lifetime does not leave failure-path dangling
  object pointers (`DEV-028`).
- Widget deep cloning is not yet safe to expose: upstream uses reflection to
  invoke a concrete widget constructor, while C++ must also support future
  custom widget types. P5-008h is `needs_human`: select a virtual per-type
  factory, explicit `TypeRegistry` argument, or another public construction
  contract before implementing `Clone`/`CopyFrom` and ContentControl/Container
  deep-copy overrides.
- Dynamic TTF/rich-text support is not selected. Do not add a third-party font
  dependency without a licence/behavior audit and a new explicit dependency
  decision; work on independent non-text tasks meanwhile.
- The MML Color serializer is deliberately absent: upstream delegates named
  color handling to FontStashSharp `ColorStorage`, whose implementation and
  licence have not yet passed the dependency audit. The audited serializers
  still cover Vector2, Thickness, and Rectangle.
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
- Default skin assets and `Inter-Regular.ttf` must not be copied until their
  redistribution provenance is resolved.
- Do not edit sibling `cna`, `cna-extended`, or `sharp-runtime` repositories.
  If one proves to need a modification, record the need here and request a
  human decision rather than changing it.
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

Read this file first, then inspect `plan.md` against the current source. The
handoff is broadly validated in headers-only, linked SOFTWARE-CNA, and linked
ASan+UBSan configurations. Begin with P2-001/P2-002: inspect CNA `Game` and
`GraphicsDevice` ownership/disposal, define Myra's checked non-owning global
environment contract and diagnostics, and then finish P1-013's view-size and
texture operations. Do not begin `RenderContext` or Desktop rendering while
that device contract is implicit. P4-019 remains open only for future widget
types; caller-provided external-asset callbacks are already usable.
