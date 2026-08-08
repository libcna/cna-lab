# Myra-CNA

Myra-CNA is an independent C++23 port of [Myra](https://github.com/MyraUI/Myra)
for [CNA](https://github.com/openeggbert/cna). It targets the behavior of
upstream Myra's FNA/XNA-oriented build directly through CNA; it does not depend
on FNA or .NET at runtime.

The project is in an early, validated retained-mode layout stage. In addition
to build, attribution, provenance, events, and utility foundations, it has
partial ports of `Widget`, `Container`, `Panel`, `Proportion`, and
`SingleItemLayout<T>`, `GridLayout`, and layout-only `Grid`, along with MML
type and attached-property registries. They support layout
measurement/arrangement, transforms, explicit child ownership, and registered
metadata; rendering, Desktop integration, styles, MML loading, rich text, Grid
selection/input, and interactive controls are not implemented yet. See
[NEXT.md](NEXT.md) for the current hand-off state and
[plan.md](plan.md) for the full compatibility backlog.

## Build

The basic bootstrap configuration only needs the sibling CNA checkout for its
GoogleTest vendoring path:

```bash
cmake -S . -B build
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure --parallel 3
```

To make this project add and link sibling CNA itself, select one backend:

```bash
cmake -S . -B build-cna \
  -DMYRA_CNA_LINK_CNA=ON \
  -DMYRA_CNA_CNA_GRAPHICS_BACKEND=SOFTWARE
cmake --build build-cna --parallel 3
ctest --test-dir build-cna --output-on-failure --parallel 3
```

Use no more than three compilation workers in every configuration.

When a parent project already supplies target `CNA`, add Myra-CNA after CNA;
the `MYRA_CNA` target links it automatically.

## Licensing and attribution

Myra-CNA's original code is MIT licensed. Every direct Myra translation will
retain Myra Team attribution and the applicable additional lineage notices.
See [NOTICE.md](NOTICE.md), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md),
and [UPSTREAM_MANIFEST.md](UPSTREAM_MANIFEST.md).
