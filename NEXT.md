# Myra-CNA continuity

## Current state

- Active branch: `develop`.
- Last validated commit at the start of this autonomous session:
  `140d1f9` (`Port Myra attached properties registry`).
- The worktree was clean before the current GridLayout implementation.
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

The widget work is deliberately partial: no drawing traversal, desktop
propagation, style application, hit testing, or input dispatch has been
claimed as complete. `UPSTREAM_MANIFEST.md` records this per source.

## Latest validation

Both configurations are green with the uncommitted GridLayout change:

```bash
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure --parallel 3
# 31/31 tests passed

CCACHE_DIR=/tmp/myra-cna-ccache cmake --build build-cna --parallel 3
ctest --test-dir build-cna --output-on-failure --parallel 3
# 52/52 tests passed, with MYRA_CNA_LINK_CNA=ON and SOFTWARE backend
```

All compilation commands must continue to use at most three workers.

## Current autonomous priority

1. Keep `README.md`, `plan.md`, and this file synchronized with the actual
   partial-widget implementation.
2. Continue layout work with `StackPanelLayout`, which is now unblocked by
   `GridLayout`, then its `StackPanel` container subset.
3. Implement MML value codecs before XML loading. Do not represent partial
   renderer, font, desktop, input, or Grid selection/style support as complete.

## Known limitations and decisions

- `sharp-runtime` reflection remains unusable by design. MML needs the
  project-local explicit type/property registry described in `plan.md`.
- Dynamic TTF/rich-text support is not selected. Do not add a third-party font
  dependency without a licence/behavior audit and a new explicit dependency
  decision; work on independent non-text tasks meanwhile.
- Default skin assets and `Inter-Regular.ttf` must not be copied until their
  redistribution provenance is resolved.
- Do not edit sibling `cna`, `cna-extended`, or `sharp-runtime` repositories.
  If one proves to need a modification, record the need here and request a
  human decision rather than changing it.
- `Widget::AddChild` deliberately reparents a child to preserve one owning
  parent. This necessary C++ difference is `DEV-011` and is covered by tests.
- `Grid` is only a layout subset. Its proportion collections currently use
  vectors rather than sharp-runtime `ObservableCollection`; see `DEV-014` and
  the remaining P6-016 task before claiming complete Grid parity.

## Recommended next starting point

Read this file first, then inspect `plan.md` against the current source. The
next implementation task is `StackPanelLayout`, followed by the corresponding
layout subset of `StackPanel`. MML value codecs remain the next independent MML
foundation task.
