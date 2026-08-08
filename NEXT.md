# Myra-CNA continuity

## Current state

- Active branch: `develop`.
- Last validated commit: `3b96e86` (`Port Myra stack panel layout subset`).
- The worktree was clean before the current TypeSerializers implementation.
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
- UI contracts, `Widget`'s layout/transform/ownership kernel, `Container`,
  `Panel`, `Proportion`, and `SingleItemLayout<T>`.
- Registry metadata attributes (`Content`, `Range`, XML/style/file-path, and
  skip/designer markers) plus `FileDialogMode`.
- An explicit, non-reflective MML `TypeRegistry`, with factories, typed
  property getter/setter adapters, inherited properties, defaults, XML names,
  and the previously ported attribute metadata.
- Myra's typed `AttachedPropertyInfo<T>`, global attached-property registry,
  base-type enumeration via `TypeRegistry`, change notifications, and widget
  measure/arrange invalidation options.
- `GridLayout` plus the layout/attached-property subset of `Grid`, including
  Part/Auto/Fill/Pixels proportions, spacing, spans, cell geometry, and child
  placement.
- `StackPanelLayout`, `StackPanel`, `HorizontalStackPanel`, and
  `VerticalStackPanel` layout subsets, including spacing and attached
  proportions.
- MML `ITypeSerializer`/`TypeSerializer<T>` and invariant Vector2, Thickness,
  and Rectangle serializers.

The widget work is deliberately partial: no drawing traversal, desktop
propagation, style application, hit testing, or input dispatch has been
claimed as complete. `UPSTREAM_MANIFEST.md` records this per source.

## Latest validation

Both configurations are green with the uncommitted TypeSerializers change:

```bash
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure --parallel 3
# 31/31 tests passed

CCACHE_DIR=/tmp/myra-cna-ccache cmake --build build-cna --parallel 3
ctest --test-dir build-cna --output-on-failure --parallel 3
# 56/56 tests passed, with MYRA_CNA_LINK_CNA=ON and SOFTWARE backend
```

All compilation commands must continue to use at most three workers.

## Current autonomous priority

1. Keep `README.md`, `plan.md`, and this file synchronized with the actual
   partial-widget implementation.
2. Implement MML value codecs before XML loading, now that foundational layout
   containers can be registered later.
3. Continue layout work only with a coherent next dependency. Do not represent partial
   renderer, font, desktop, input, or Grid selection/style support as complete.

## Known limitations and decisions

- `sharp-runtime` reflection remains unusable by design. MML needs the
  project-local explicit type/property registry described in `plan.md`.
- Dynamic TTF/rich-text support is not selected. Do not add a third-party font
  dependency without a licence/behavior audit and a new explicit dependency
  decision; work on independent non-text tasks meanwhile.
- The MML Color serializer is deliberately absent: upstream delegates named
  color handling to FontStashSharp `ColorStorage`, whose implementation and
  licence have not yet passed the dependency audit. The audited serializers
  still cover Vector2, Thickness, and Rectangle.
- Default skin assets and `Inter-Regular.ttf` must not be copied until their
  redistribution provenance is resolved.
- Do not edit sibling `cna`, `cna-extended`, or `sharp-runtime` repositories.
  If one proves to need a modification, record the need here and request a
  human decision rather than changing it.
- `Widget::AddChild` deliberately reparents a child to preserve one owning
  parent. This necessary C++ difference is `DEV-011` and is covered by tests.
- Grid and StackPanel are layout subsets. Their proportion collections use
  vectors rather than sharp-runtime `ObservableCollection`; see `DEV-014` and
  the remaining P6-013/P6-016 tasks before claiming full parity.

## Recommended next starting point

Read this file first, then inspect `plan.md` against the current source. The
next implementation task is the remaining MML value-codec registry for
primitive, optional, and explicitly mapped enum values. It is independent of
renderer and font work; Color remains gated by its separate provenance audit.
