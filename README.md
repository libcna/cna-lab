# Myra-CNA

Myra-CNA is an independent C++23 port of [Myra](https://github.com/MyraUI/Myra)
for [CNA](https://github.com/openeggbert/cna). It targets the behavior of
upstream Myra's FNA/XNA-oriented build directly through CNA; it does not depend
on FNA or .NET at runtime.

The project is in an early, validated retained-mode layout stage. In addition
to build, attribution, provenance, events, and utility foundations (including
type-safe table cloning/sorting), it has
partial ports of `Widget`, abstract `ContentControl`, `Container`, `Panel`, `Proportion`, and
`SingleItemLayout<T>`, `GridLayout`, and layout-only `Grid`, along with MML
type and attached-property registries, and layout-only stack panels. They support layout
measurement/arrangement, transforms, explicit child ownership, and registered
metadata plus basic MML geometry serialization; rendering, Desktop integration,
styles, rich text, Grid selection/input, interactive controls, and registration
of the full upstream widget catalog are not implemented yet. Primitive, optional,
explicitly mapped enum, and audited geometry codecs drive registry-backed XML
loading/saving with defaults, skips, legacy/XML names, explicit external-asset
callbacks, attached properties, BaseObject user data, and recursive
single/sequence/dictionary/content adapters. A central metadata table registers
all currently ported types and is exercised by real Grid/stack-panel round trips;
it rejects ambiguous inherited C++/XML property identities and null factory or
callback-owner results at the registry boundary.
The widget tree also provides stable Z-ordered recursive enumeration, typed and
untyped descendant lookup, visibility-aware counts, reparenting, and ownership-cycle
rejection. `EnsureWidgetById` adds a throwing non-null lookup for generated UI
code, and derived widgets can temporarily suppress measure invalidation while
batching changes. Layout and traversal passes retain child snapshots across
reentrant callbacks so tree mutation cannot invalidate active C++ iterators.
The dependency-free Widget property subset also includes drag settings,
recursive mouse-cursor inheritance, optional tooltips, modal/pressed/clipping
state, focus acceptance/state events, cancelable user-driven pressed changes,
named-style persistence, a non-owning drag handle, arbitrary tag data, and the
box bounds required by later rendering. Style application itself is not yet
implemented.
See
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

For a GCC/Clang linked-CNA AddressSanitizer and UndefinedBehaviorSanitizer run:

```bash
cmake -S . -B build-sanitize \
  -DMYRA_CNA_LINK_CNA=ON \
  -DMYRA_CNA_CNA_GRAPHICS_BACKEND=SOFTWARE \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
ASAN_OPTIONS=detect_leaks=0 \
  cmake --build build-sanitize --parallel 3
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 \
  ctest --test-dir build-sanitize --output-on-failure --parallel 3
```

Only LeakSanitizer is disabled in these commands because this environment runs
under `ptrace`; AddressSanitizer's memory checks and UndefinedBehaviorSanitizer
remain enabled.

When a parent project already supplies target `CNA`, add Myra-CNA after CNA;
the `MYRA_CNA` target links it automatically.

## Licensing and attribution

Myra-CNA's original code is MIT licensed. Every direct Myra translation will
retain Myra Team attribution and the applicable additional lineage notices.
See [NOTICE.md](NOTICE.md), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md),
and [UPSTREAM_MANIFEST.md](UPSTREAM_MANIFEST.md).
