# Myra-CNA

Myra-CNA is an independent C++23 port of [Myra](https://github.com/MyraUI/Myra)
for [CNA](https://github.com/openeggbert/cna). It targets the behavior of
upstream Myra's FNA/XNA-oriented build directly through CNA; it does not depend
on FNA or .NET at runtime. It is not affiliated with, maintained by, or endorsed
by the Myra or CNA upstream projects.

The project is an incomplete but validated retained-mode UI port. Its current
surface includes the Widget tree/layout/rendering kernel, Grid and stack layout,
MML metadata and XML project loading, CNA SpriteBatch rendering primitives,
images/atlases/brushes, separators, progress bars, and style-independent
Button/ToggleButton/CheckButtonBase interaction cores. The Desktop now owns and
lays out retained roots, propagates placement and transforms, maintains stable
Z-order and focus, and exposes traversal/menu/modal queries. Its input core
polls injectable CNA mouse/key snapshots, tracks pointer/touch/wheel state,
dispatches retained global events, repeats keys, moves Tab focus, and routes
keys to menus or the focused widget. Reverse-Z widget hit testing now tracks
local mouse/touch transitions, input fall-through, hover visuals, touch focus,
the deepest wheel target, and upstream-compatible local double-click timing.
Generic Widget dragging now honors in-tree handles, direction flags, parent/
Desktop capture, release reset, and bounds clamping. Cursor routing, injectable
tooltip overlays, Desktop rendering, context menus, Grid selection, and the
Button/Slider/ScrollViewer capture slices are implemented. Label-backed default
tooltips, remaining control-specific input, styles, fonts/rich text, advanced
controls, and the full upstream widget catalog are not implemented yet.
Primitive, optional, explicitly mapped enum, and audited geometry codecs drive registry-backed XML
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
User-defined C++ types are not discovered automatically: applications register
their factories, properties, adapters, metadata, and finite codecs before MML
or future DataGrid/PropertyGrid use. Missing registration is an explicit
unsupported-type error, not a silent reflection fallback. The exact extension
contract is documented under [DEV-002](docs/cpp-deviations.md#finite-application-registry-boundary).
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
also exposes the upstream event/debug/clipping settings, injectable input
providers with CNA-backed defaults, and maps all twelve Myra cursor types to
CNA stock cursors.
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

## Prerequisites

- CMake 3.20 or newer and a C++23 compiler.
- A CNA checkout and its sibling sharp-runtime checkout. Current modular CNA
  must be the top-level CMake project for linked rendering and input builds.
- The platform and renderer dependencies required by the selected CNA backend.
- Python 3 and the GoogleTest sources vendored by CNA when building this
  repository's tests.
- An X11/Xvfb display only for the optional `SDL_RENDERER` smoke test.

No font library, skin, or copied upstream binary asset is currently bundled.
The dependency-free configuration is a compile/test aid; applications that
render or use CNA-backed platform behavior need the linked CNA configuration.

## Build

The basic bootstrap configuration only needs the sibling CNA checkout for its
GoogleTest vendoring path:

```bash
cmake -S . -B build
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure --parallel 3
```

Here “headers-only compile-check mode” describes dependency resolution, not the
shape of Myra-CNA itself: `Myra::CNA` is still a compiled static library. This
mode consumes CNA and sharp-runtime headers but deliberately defers their
libraries to a future consumer. It validates dependency-independent code,
public-header compilation, provenance, and explicit missing-backend diagnostics;
it cannot link an application that constructs CNA-backed widgets or renders a
Desktop. The `myra_cna_minimal_consumer` CTest verifies this boundary and the
same test constructs a widget tree in the linked configuration.

Current modular CNA must remain the top-level CMake project. From the Myra-CNA
repository root, use the project-specific include driver to add and link this
project without enabling CNA's unrelated tests or examples:

```bash
cmake -S ../cna -B build-cna-parent \
  -DCMAKE_PROJECT_CNA_INCLUDE="$PWD/cmake/AddMyraCnaToCnaBuild.cmake" \
  -DCNA_BUILD_TESTS=OFF \
  -DCNA_BUILD_EXAMPLES=OFF \
  -DCNA_GRAPHICS_RENDERER=SOFTWARE \
  -DMYRA_CNA_BUILD_TESTS=ON \
  -DMYRA_CNA_BUILD_EXAMPLES=ON
cmake --build build-cna-parent --parallel 3
ctest --test-dir build-cna-parent/_myra_cna \
  --output-on-failure --parallel 3
```

`MYRA_CNA_LINK_CNA=ON` remains a compatibility path for older CNA layouts that
support child-project embedding. A current modular checkout produces a focused
diagnostic directing callers to the command above instead of failing inside
CNA's top-level source-partition checks.

An `SDL_RENDERER` build additionally registers
`myra_cna_sdlrenderer_smoke`, which opens a real CNA window, renders a Myra
solid brush, and validates backbuffer pixels. Point its explicit display
setting at an already-running X11/Xvfb server:

```bash
cmake -S ../cna -B build-sdlrenderer-parent \
  -DCMAKE_PROJECT_CNA_INCLUDE="$PWD/cmake/AddMyraCnaToCnaBuild.cmake" \
  -DCNA_BUILD_TESTS=OFF \
  -DCNA_BUILD_EXAMPLES=OFF \
  -DCNA_GRAPHICS_RENDERER=SDL_RENDERER \
  -DMYRA_CNA_BUILD_TESTS=ON \
  -DMYRA_CNA_BUILD_EXAMPLES=ON \
  -DMYRA_CNA_TEST_DISPLAY=:99
cmake --build build-sdlrenderer-parent --parallel 3
ctest --test-dir build-sdlrenderer-parent/_myra_cna --output-on-failure \
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
cmake -S ../cna -B build-sanitize-parent \
  -DCMAKE_PROJECT_CNA_INCLUDE="$PWD/cmake/AddMyraCnaToCnaBuild.cmake" \
  -DCNA_BUILD_TESTS=OFF \
  -DCNA_BUILD_EXAMPLES=OFF \
  -DCNA_GRAPHICS_RENDERER=SOFTWARE \
  -DMYRA_CNA_BUILD_TESTS=ON \
  -DMYRA_CNA_BUILD_EXAMPLES=ON \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
ASAN_OPTIONS=detect_leaks=0 \
  cmake --build build-sanitize-parent --parallel 3
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 \
  ctest --test-dir build-sanitize-parent/_myra_cna \
  --output-on-failure --parallel 3
```

Only LeakSanitizer is disabled in these commands because this environment runs
under `ptrace`; AddressSanitizer's memory checks and UndefinedBehaviorSanitizer
remain enabled.

When another parent project already supplies target `CNA`, add Myra-CNA after
CNA; the `MYRA_CNA` target links it automatically. The driver above handles the
current CNA layout, whose umbrella target is created later in its top-level
configure pass, and also declares Myra's direct `SharpRuntime::Xml` dependency.
The default CTest suite also configures `tests/consumer-parent` as an isolated
parent project: it defines the CNA/Xml target contract first, adds Myra-CNA as a
subdirectory, compiles the complete linked source selection, and links and runs
an umbrella-header consumer through `Myra::CNA`. The regular linked SOFTWARE
matrix supplies the real CNA and sharp-runtime libraries for runtime coverage.

## Minimal usage

Once a parent has supplied `CNA`, add the repository and link the stable alias:

```cmake
add_subdirectory(path/to/myra-cna)
target_link_libraries(my_app PRIVATE Myra::CNA)
```

The umbrella header exposes the currently ported widget surface. This
asset-independent example builds a logical tree without requiring a font or skin:

```cpp
#include "Myra/Myra.hpp"

#include <memory>

int main()
{
    auto root = std::make_shared<Myra::Graphics2D::UI::VerticalStackPanel>();
    auto button = std::make_shared<Myra::Graphics2D::UI::Button>();
    button->setWidthProperty(120);
    button->setHeightProperty(32);
    root->AddWidget(button);

    return root->getWidgetsProperty().size() == 1 ? 0 : 1;
}
```

In a linked CNA build, `examples/Minimal.cpp` performs the same construction and
prints the library version. In headers-only compile-check mode it deliberately
limits itself to the dependency-free version API. A rendered application
additionally configures a CNA `Game` through `MyraEnvironment`, owns a `Desktop`,
and drives its update/render lifecycle; those APIs require a linked CNA build.
Until the font/style phases are complete, compose only controls listed as
available above and in [NEXT.md](NEXT.md).

## Licensing and attribution

Myra-CNA's original code is MIT licensed. Every direct Myra translation will
retain Myra Team attribution and the applicable additional lineage notices.
See [NOTICE.md](NOTICE.md), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md),
and [UPSTREAM_MANIFEST.md](UPSTREAM_MANIFEST.md).
