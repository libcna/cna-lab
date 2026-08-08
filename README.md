# Myra-CNA

Myra-CNA is an independent C++23 port of [Myra](https://github.com/MyraUI/Myra)
for [CNA](https://github.com/openeggbert/cna). It targets the behavior of
upstream Myra's FNA/XNA-oriented build directly through CNA; it does not depend
on FNA or .NET at runtime.

The project is at the first shared-runtime stage. It provides build,
attribution, provenance, and test infrastructure plus faithful translations of
event arguments/propagation, `EventHandlingStrategy`, and `Thickness`; no
widget has been ported yet.

## Build

The basic bootstrap configuration only needs the sibling CNA checkout for its
GoogleTest vendoring path:

```bash
cmake -S . -B build
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure
```

To make this project add and link sibling CNA itself, select one backend:

```bash
cmake -S . -B build-cna \
  -DMYRA_CNA_LINK_CNA=ON \
  -DMYRA_CNA_CNA_GRAPHICS_BACKEND=SOFTWARE
cmake --build build-cna --parallel 3
```

When a parent project already supplies target `CNA`, add Myra-CNA after CNA;
the `MYRA_CNA` target links it automatically.

## Licensing and attribution

Myra-CNA's original code is MIT licensed. Every direct Myra translation will
retain Myra Team attribution and the applicable additional lineage notices.
See [NOTICE.md](NOTICE.md), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md),
and [UPSTREAM_MANIFEST.md](UPSTREAM_MANIFEST.md).
