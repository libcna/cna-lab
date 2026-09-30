# Canonical CNA C ABI boundary

## The admitted window

CNA-Swift admits **CNA C ABI major 0 with minor 35 or later**, and is qualified
against exactly `0.35.0` (`0x00002300`). The rule is CNA's own, not this
binding's invention: `docs/c-api/ABI_VERSIONING.md` says a consumer *must reject
a different major and may require a minimum minor*, and the installed CNA
package enforces the same thing with `COMPATIBILITY SameMajorVersion`. Under
`0.x` an incompatible change is what moves the minor, so the minimum minor is
the generation this binding was measured against.

A later minor is admitted by that rule. The protection against a later minor
that removed a route is not a version number: every one of the 778 bound
symbols must resolve by name before the runtime starts, and a missing one throws
`CNAError.missingNativeSymbol`.

A rejection names the admitted window, the reported version, and the file that
was selected:

```text
CNA native library /opt/cna/libcna_c_api.so reports C ABI 0.7.0 (0x00000700);
CNA-Swift admits major 0 with minor 35 or later (qualified against 0.35.0)
```

The loader accepts `CNA_NATIVE_LIBRARY` only when it is an absolute file path;
without it Linux tries only the installed soname `libcna_c_api.so`. There is no
sibling checkout, build directory, developer path, or C++ fallback.

## What is recorded and what is verified

`NativeManifest.swift` records, for each bound route: the canonical C symbol,
the single `NativeFunctions` property that holds it, that route's own
`@convention(c)` type, the canonical return and parameter declarations,
ownership, result lifetime, error lifetime, and callback ABI. `NativeFunctions`
resolves each symbol once into an immutable typed property. No two routes share
a route type, and no strict XNA type exposes a handle or function pointer.

`tools/native_abi/verify.py` independently:

1. compares each manifest declaration **textually** with the canonical
   declaration parsed from the CNA headers, parameter names included, so a
   merely compatible spelling is a mismatch;
2. compiles `__builtin_types_compatible_p` assertions for every manifest
   prototype against `&symbol`;
3. pairs every Swift property with exactly one symbol and one route type, each
   route type being re-derived from its symbol rather than trusted;
4. compares every mirrored `CNASwift_*` structure with its canonical
   counterpart field for field — names, order, offsets and widths;
5. proves every mirrored callback is the canonical callback type;
6. compiles the canonical constants and every selected `Keys` literal;
7. audits ELF exports and calls `cna_get_abi_version` on the explicit library,
   requiring it to be inside the admitted window *and* to agree with the header.

Qualified result on CNA 0.35.0 (both evidence libraries below):

```text
BOUND_FUNCTIONS=778  ROUTE_PAIRINGS=778  PROTOTYPE_TYPE_POSITIONS=2657
CANONICAL_DECLARATION_CHECKS=2657  C_SWIFT_MEASUREMENTS=2657
LAYOUTS=68  LAYOUT_FIELDS=522  CALLBACKS=9  CONSTANTS=228  SCALAR_FACTS=3
MISSING_HEADER_SYMBOLS=0  MISSING_LIBRARY_SYMBOLS=0  ABI_MISMATCHES=0
```

None of the 855 routes CNA removed between 0.21 and 0.35 (the engine layer, the
avatar real-rendering and Guide setter extensions, the SpriteBatch mesh route)
was bound here, and every bound prototype and mirrored structure is unchanged.

Every count is derived from the source the verifier just compiled. None is a
hand-maintained literal.

`tools/native_abi/mutations.py` is the falsifiability gate: fourteen planted
defects — wrong parameter width, a compatible-but-wrong canonical spelling, a
stale symbol, a swapped route symbol, a swapped route type, a shared route type,
a wrong Swift position width, an unsatisfiable ABI window, an omitted structure
field, transposed fields, a narrowed field, a wrong callback signature, a wrong
constant and a wrong `Keys` literal — each of which the verifier must reject.
All fourteen are caught, and the tree is proven byte-identical afterwards.

## The evidence libraries

Both are CNA `next` built from source during the 2026-09-30 retirement pass and
staged with a `PROVENANCE.txt` (configuration, source revision, hashes):

```text
CNA_SOURCE_REVISION=4228ff913987452c6fe03d125d186072b1405187 (libcna/cna, branch next)
CNA_ABI_VERSION=0.35.0   EXPORTS=3202   PLATFORM=Linux x86-64   AUDIO_BACKEND=SDL3
HEADLESS   ~/deps/cna-c-abi-0.35.0                 Debug, CNA_DEVICES=OFF, CNA_CNAEXT=OFF, video OFF
           NATIVE_LIBRARY_SHA256=070ed77bec5af09f64c7215e25c90de0970f0316b6d50982eece85fd1963c449
OPENGLES3  ~/deps/cna-c-abi-0.35.0-opengles3-fx    Release, EasyGL compiled effects, CNAEXT, DEVICES, video ON
           NATIVE_LIBRARY_SHA256=4e38a2dafd68e22c1a77c00a34b2ef6a7b1cb7b6832d904ba4a21b05f374f4d6
```

Neither is shipped. The previous boundary, CNA C ABI 0.21.0 (cnanext
`0a6158e4f`, HEADLESS, SHA-256 `c32bfbd3…`, 4,054 exports), is recorded in
`docs/native-abi-migration-evidence.md`; `docs/cna-0-35-requalification.md`
records what changed between the two.

## Historical record: the retired CNA 0.7.0 boundary

Foundation Milestone 1 admitted exactly CNA C ABI `0.7.0` (`0x00000700`) and
rejected every other version. That measurement happened and is retained here;
it is no longer the boundary.

```text
CNA_SOURCE_REVISION=a09196a6477f69a7a57c8364f990658d31531a5b
CNA_ABI_VERSION=0.7.0
NATIVE_LIBRARY_SHA256=42e099146bf3b470f82fd963a516f8bdd7ff0406da8c37dd53747699117db086
PLATFORM=Linux x86-64   RENDERER=HEADLESS   AUDIO_BACKEND=NULL
NATIVE_ABI=29 functions / 91 prototype positions / 91 C-Swift measurements /
           18 layouts / 2 callbacks / 214 constants / 0 missing / 0 mismatches
```

That binary no longer exists on this machine; the reproduced, ABI- and
behaviour-equivalent build documented in
`~/deps/cna-c-abi-0.7.0-pinned-foundation11/PROVENANCE.md`
(`c62949d23d3745964f5e557a06665875621ed4cb6e2930e3f282afd5911f2dcb`) reproduced
every one of those numbers exactly. `214` was a hand-maintained literal; the
derived count for the same assertions is 212 — see the migration document.

Foundation Milestone 6 added only the four required existing GamePad functions,
three exact copied-POD layouts, and 46 player/dead-zone/threshold/button/type
constants. Every field offset and function position was compiler-measured; no
adjacent CNA controller extension route was bound. All of it still holds on
0.35.0, unchanged.

## The render-target routes

Foundation 38 added the first native surface since the migration: seven routes
(`cna_texture_get_info`, `cna_render_target2d_create`,
`cna_render_target_get_info`, `cna_render_target_destroy`,
`cna_graphics_device_set_render_target2d` and the ContentLost
subscribe/unsubscribe pair), three mirrored structures and one mirrored
callback. Their first verification run failed on two positions the verifier
could not yet spell — `CNA_RenderTargetEventRegistrationHandle` and `void*` —
which is the canonical-declaration check working on the first surface added
since it existed.

## The game-host and device-service routes

Foundations 39 and 40 took the count from 43 to 55. Foundation 39 bound the
`Game` host members — timing writers, `Tick`, `SuppressDraw`,
`ResetElapsedTime`, `ShowMissingRequirementMessage`, the activation state and
the host-event subscribe/unsubscribe pair — with a fourth mirrored callback,
`CNA_GameEventCallback`, whose shape is the parameterless
`void (*)(void* context)`. Foundation 40 bound the graphics device manager's
own creation, `BeginDraw`/`EndDraw`, its device-event subscription pair, and the
callback-scoped device accessor.

The device accessor is the one route whose *refusal* is part of the measured
contract: outside a CNA callback `cna_graphics_device_manager_get_device`
answers `CNA_RESULT_INVALID_STATE` with a zero handle. The Swift guard that
returns `nil` there is documented defence in depth over a native refusal, not
the only thing preventing a dangling handle — which is why the mutation that
removed it was withdrawn as unfalsifiable rather than left in the harness
claiming coverage it did not have.

## The device-state routes

Foundation 41 added the four graphics state objects and bound **no** route for
them, because the milestone projected the managed types and did not implement
`GraphicsDevice.BlendState`. Binding a route for count, without a member that
uses it, is the thing this boundary exists to prevent.

Foundation 45 added the members and, with them, fourteen routes: get and set for
blend, depth-stencil and rasterizer state, get and set for one sampler slot of
one shader stage, and get and set for the three values those state setters copy
out — `blend_factor`, `multi_sample_mask` and `reference_stencil`, each of which
XNA exposes as a device property in its own right. Four more mirrored structures
came with them, taking the layout wall from 21 structures and 157 fields to 25
and 209.

## The sprite routes, and the two that were removed

Foundation 53 bound `cna_sprite_batch_submit_many` with its mirrored
`CNA_SpriteCommand`, because XNA's destination-rectangle `Draw` overloads carry
a rectangle where the scaled ones carry position and scale, and CNA splits its
commands along the identical line.

Foundation 54 bound `cna_sprite_batch_begin_with_states` and **unbound**
`cna_sprite_batch_begin`, whose consumer disappeared when `Begin()` began
forwarding through the state overload. `CNA_SpriteBatchBeginInfo` went with it:
a mirrored structure with no route is the same unearned count as a route with
no member. `cna_sprite_batch_begin_with_effect` stays unbound because `Effect`
is not projected.

## The clear routes, and the one that was removed

Foundation 48 bound `cna_graphics_device_clear_options` and
`cna_graphics_device_get_presentation_parameters`, with
`CNA_PresentationParameters` as the twenty-sixth mirrored structure, and
**unbound** `cna_graphics_device_clear_rgba`. Once `Clear(Color)` forwards
through `Clear(ClearOptions, Color, Single, Int32)` as the pinned IL does, no
projected member consumes the colour-only route, and an unconsumed route is the
thing this boundary exists to prevent. The net is 74, not 75.

Three canonical constants joined the probe with them —
`CNA_CLEAR_OPTION_TARGET`, `_DEPTH_BUFFER` and `_STENCIL`, taking `CONSTANTS`
from 212 to 215. They are compiled rather than tested because no runtime
observation on a HEADLESS device can see which buffers a clear touched;
`docs/foundation-48-clear-evidence.md` records that and the one realistic
defect that consequently has no mutation control.

## The manager preference routes

Foundation 51 bound the nine `cna_graphics_device_manager_set_*` preference
routes and **none** of the nine matching getters. XNA keeps every preference in
a managed field — which is what makes each of its getters
`IL_NO_FAILURE_PATH` — and applies them at `ChangeDevice`, so the projection
has a member that consumes each setter and no member that would consume a
getter. `cna_graphics_device_manager_toggle_full_screen` is unbound for the
same reason: `ToggleFullScreen` is projected as XNA writes it, through the
property and a device change, and the probe shows the two reach the same state.

`cna_blend_state_init` and its three neighbours are still **not** bound. They
are CNA's own preset descriptors, and the Swift presets come from the pinned
`.cctor`; binding them would add a route no member consumes and would put a
native value where XNA authority belongs.

The first verification run of the eight failed on two positions:

```text
cna_graphics_device_get_sampler_state: parameters
  ['uint64_t', 'uint32_t', 'uint32_t', 'CNA_SamplerState*'] !=
  ['uint64_t', 'CNA_ShaderStage', 'uint32_t', 'CNA_SamplerState*']
```

`CNA_ShaderStage` — `typedef uint32_t` at `graphics_state.h:214` — was missing
from the alias table the type-compatibility comparison uses, and Foundation 47's
four routes then found `CNA_GraphicsDeviceStatus` missing the same way. The textual
canonical-declaration check had already passed on all 198 positions, because the
manifest keeps CNA's own spelling, and an alias never weakens that check. Same
class of gap as the two `RenderTarget2D` positions in Foundation 38, and the
same conclusion: the check working on the first surface added since it existed.
