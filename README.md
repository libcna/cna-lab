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
The high-level MML load result owns its parsed XML document and retained handles
for every context-created mapped object, so source-node mappings remain valid
after the loader itself is destroyed.
The dependency-safe `Project` core now persists `ExportOptions`, stylesheet and
designer-asset paths, and one implicit root widget. Default convenience methods
cover the built-in type table, while explicit registry/codec overloads support
custom widget catalogs. A loaded Project owns its DOM and mappings without a
self-retention cycle, rejects malformed/non-Project XML, and enforces unique
non-empty case-sensitive object IDs. Stylesheet application, asset loading,
stylesheet-aware object construction, and code generation remain unimplemented
dependencies. Dependency-safe single-object helpers already preserve legacy and
proportion-property tags, owning load results, scalar-only source replacement,
and parent attached-property context.
The widget tree also provides stable Z-ordered recursive enumeration, typed and
untyped descendant lookup, visibility-aware counts, reparenting, and ownership-cycle
rejection. `EnsureWidgetById` adds a throwing non-null lookup for generated UI
code, and derived widgets can temporarily suppress measure invalidation while
batching changes. Layout and traversal passes retain child snapshots across
reentrant callbacks so tree mutation cannot invalidate active C++ iterators.
Queued input processors are likewise retained through dispatch or propagation
filtering, preserving managed event lifetime without changing the non-owning
widget-to-Desktop link.
No-argument `Widget::Clone()` now uses a protected virtual construction factory,
so current concrete controls and non-default-constructible custom widgets keep
their exact dynamic type without reflection. Containers and content controls
deep-clone their owned widgets.
The dependency-free Widget property subset also includes drag settings,
recursive mouse-cursor inheritance, optional tooltips, modal/pressed/clipping
state, focus acceptance/state events, cancelable user-driven pressed changes,
named-style persistence, a non-owning drag handle, arbitrary tag data, and the
box bounds required by later rendering. Style application itself is not yet
implemented.
The CNA-linked configuration also has a checked, non-owning
`MyraEnvironment` Game/GraphicsDevice contract. Normal CNA disposal and RAII
destruction clear it automatically; custom lifetime arrangements must call
`MyraEnvironment::ClearGame()` before invalidating their game. The environment
also exposes the upstream event/debug/clipping settings and maps all twelve
Myra cursor types to CNA stock cursors.
The same configuration now includes the graphics-only `RenderContext`, its
rectangle/polygon/line/point/circle/arc helpers, retained texture regions
(colored, tinted, nine-patch, and XML atlases), a lifecycle-safe 1x1 default
white region, and the color/draw core of `SolidBrush`. Font/rich-text,
stylesheet selection, and asset-manager integration remain deliberately gated.
The completed [font and rich-text audit](docs/font-audit.md) pins the active
upstream API, licences, Unicode discrepancy, and implementation alternatives;
no font dependency or source will be added until the `needs_human` P3-004
decision is explicit.
The [production-source lineage audit](docs/source-lineage-audit.md) classifies
all 189 pinned Myra C# files and records the MonoGame, MonoGame.Extended, and
TextCopy exceptions and CNA replacements.
The [default-asset audit](docs/default-assets-audit.md) clears the exact Inter
font under OFL 1.1 but blocks copying the VisUI-derived atlas until
`needs_human` P0-015b chooses documented permission/compliance or replacement
artwork. No upstream font or skin binary is currently bundled.
The [test-asset audit](docs/test-assets-audit.md) classifies all 35 pinned
Myra.Tests assets. It permits Myra-authored text and a properly noticed libGDX
fixture, while requiring project-owned replacements for the MonoGame logo,
Arial/C64 bitmap fonts, unmatched DroidSans binary, and blocked default skin.
No upstream test binary is currently bundled.
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

An `SDL_RENDERER` build additionally registers
`myra_cna_sdlrenderer_smoke`, which opens a real CNA window, renders a Myra
solid brush, and validates backbuffer pixels. Point its explicit display
setting at an already-running X11/Xvfb server:

```bash
cmake -S . -B build-sdlrenderer \
  -DMYRA_CNA_LINK_CNA=ON \
  -DMYRA_CNA_CNA_GRAPHICS_BACKEND=SDL_RENDERER \
  -DMYRA_CNA_TEST_DISPLAY=:99
cmake --build build-sdlrenderer --parallel 3
ctest --test-dir build-sdlrenderer --output-on-failure \
  --parallel 3 -L RequiresDisplay
```

CTest supplies `SDL_VIDEODRIVER=x11` and the configured `DISPLAY`, serializes
this project's display resource, applies a 30-second timeout, and labels the
test `GraphicsSmoke`, `SDLRenderer`, and `RequiresDisplay`.

Use no more than three compilation workers in every configuration.

Strict warning, formatting, and optional include-what-you-use policies are
documented in [DEVELOPING.md](DEVELOPING.md). Project-owned targets treat
warnings as errors by default; formatter and IWYU tooling remain optional for
ordinary consumers.

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
