# Foundation Milestone 13 — RenderTargetUsage evidence

## Exact one-type closure and authority

Foundation Milestone 13 closes exactly one public XNA type:
`Microsoft.Xna.Framework.Graphics.RenderTargetUsage`. No `RenderTarget2D`,
`RenderTargetCube`, `RenderTargetBinding`, `PresentationParameters`,
`GraphicsDevice` member, `GraphicsAdapter`, renderer operation, framebuffer
allocation, content preservation policy, or native route is part of this
closure.

Public shape comes from the pinned Microsoft XNA Framework 4.0 Windows runtime
contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`. The
selected metadata entry was re-read independently for this milestone. It is a
sealed, non-flags CLR enum with `System.Int32` underlying storage,
`System.Enum` base, no direct interfaces, and exactly four CLR identities:

| CLR identity | Contract value |
|---|---:|
| `value__` | `System.Int32` storage |
| `DiscardContents` | 0 |
| `PreserveContents` | 1 |
| `PlatformContents` | 2 |

Thus `SOURCE_MEMBERS=4`. The existing enum-storage language rule excludes the
synthetic CLR `value__` identity, so `EXPECTED_SWIFT_MEMBERS=3`. There is no
declared constructor, method, property, event, or operator in the selected XNA
contract. The three inherited `System.IComparable`, `System.IConvertible`, and
`System.IFormattable` entries are the ordinary `System.Enum` inheritance set;
`directInterfaces` is empty, so no interface identity is projected.

The same shape was independently re-derived this milestone straight from the
pinned assembly rather than only from the retained contract JSON.
`Microsoft.Xna.Framework.Graphics.dll` SHA-256
`560080fc39021c611ca9d076dcebed312faf6d7d1413c2dc523683ea635e9f55` disassembles
to exactly:

```text
.class public auto ansi sealed Microsoft.Xna.Framework.Graphics.RenderTargetUsage
       extends [mscorlib]System.Enum
{
  .field public specialname rtspecialname int32 value__
  .field public static literal valuetype ...RenderTargetUsage DiscardContents = int32(0x00000000)
  .field public static literal valuetype ...RenderTargetUsage PreserveContents = int32(0x00000001)
  .field public static literal valuetype ...RenderTargetUsage PlatformContents = int32(0x00000002)
} // end of class Microsoft.Xna.Framework.Graphics.RenderTargetUsage
```

There is no `implements` clause, no method, property, event, or nested type,
and — decisively — **no `System.FlagsAttribute`**. The same assembly's
`BufferUsage` does carry
`.custom instance void [mscorlib]System.FlagsAttribute::.ctor()`, so
`FLAGS=false` here is a directly observed absence in the pinned image, not an
assumption inherited from the contract JSON. The `int32 value__` field is the
underlying-type evidence.

Because the entire contract of an enum is metadata, no behavior surrogate or
reference probe was needed or created. The environment limitation recorded in
Foundation 12 — the pinned Graphics assembly is a mixed-mode C++/CLI image that
cannot execute directly under Mono on this Linux qualification host — does not
apply to a type with no executable member.

## Swift ordinary-enum projection

The established CNA-Swift non-flags mapping produces a Swift `enum` in the
exact `Microsoft.Xna.Framework.Graphics` namespace. There is no Framework-root
alias, no `RenderTarget` subnamespace, and no native or interop namespace. Its
raw type is exactly `Int32`, and its only cases are `DiscardContents=0`,
`PreserveContents=1`, and `PlatformContents=2`, retaining XNA's exact
PascalCase spelling. Every raw value is written explicitly; nothing relies on
Swift source-order inference to establish API authority.

The compiler Symbol Graph and the source raw-value supplement establish enum
kind, `Int32` raw storage, non-flags status, and all three literal values. The
verifier reports exactly three target XNA identities and zero local
diagnostics. Compiler-provided `rawValue`, `init?(rawValue:)`, equality, and
value-copy behavior are Swift language surface and do not become XNA member
identities. No RenderTargetUsage-specific diagnostic suppression, member
omission, or raw-value exception exists, and no new general mapping rule was
required — registering the type in the existing `rawTypeChecks` list is the
same per-enum registration every previously completed enum uses.

The three literals are mutually exclusive alternatives, not bit flags. The type
does not conform to `OptionSet`, `CustomStringConvertible`,
`CustomDebugStringConvertible`, `CaseIterable`, or `Codable`. It has no
`description`, `debugDescription`, `String`, `ToString`, union, intersection,
`contains`, bitwise operation, `HasFlag`, alias such as `Default`, `None`,
`KeepContents`, `Discard`, or `Preserve`, no `isDiscard`, `preservesContents`,
`platformDefault`, or `requiresPreservation` predicate, no `nativeUsage`
conversion, and no convenience helper of any kind.

## Swift mapping qualification

The separately counted Swift projection test contains the authoritative table
`[(0, .DiscardContents), (1, .PreserveContents), (2, .PlatformContents)]` and
establishes:

- every raw initializer from 0 through 2 returns exactly its corresponding
  case;
- every case exposes the exact typed `Int32` raw value;
- `RenderTargetUsage.DiscardContents.rawValue == 0`;
- raw values 3, -1, `Int32.max`, and `Int32.min` return `nil`;
- assignment copies the enum value, and reassigning the copy leaves the
  original unchanged.

Unknown raw values returning `nil` are ordinary Swift enum semantics under the
established projection, not a claim that CLR enum storage rejects unknown
integers. Copy behavior is ordinary Swift value semantics: there is no
reference ownership, no native lifetime, no generation, and no initialization
side effect. The tests pass with `CNA_NATIVE_LIBRARY` unset and initialize no
`Game`, `GraphicsDevice`, `RenderTarget2D`, native function table, runtime
registry, or CNAShim route.

## Pure XNA-derived evidence

The compact `RENDER_TARGET_USAGE` group records only the pinned XNA facts: enum
kind, non-flags status, `System.Int32` underlying storage, and the complete
three-value literal table. Kind, flags, and storage are independently enforced
by the compiler and the verifier. The grouped table adds two source assertion
sites, moving the pure corpus from 1,269 to 1,271 observations and assertions
with zero failures. Swift raw initialization, unknown-value rejection, and
value-copy behavior remain separate mapping qualification and are not counted
as XNA runtime behavior.

## Structural result and strict-zero matrix

| Measurement | Result |
|---|---:|
| CLR identities | 4 |
| Expected Swift XNA identities | 3 |
| Target Swift XNA identities | 3 |
| Local diagnostics | 0 |
| CLR kind | enum |
| Swift kind | enum |
| Underlying type | `Int32` |
| Flags | false |
| Public constructors | 0 |

| Local diagnostic category | Count |
|---|---:|
| `MISSING_MEMBER` | 0 |
| `UNEXPECTED_MEMBER` | 0 |
| `TYPE_KIND_MISMATCH` | 0 |
| `BASE_MAPPING_MISMATCH` | 0 |
| `INTERFACE_MAPPING_MISMATCH` | 0 |
| `FIELD_MAPPING_MISMATCH` | 0 |
| `PROPERTY_MAPPING_MISMATCH` | 0 |
| `METHOD_SIGNATURE_MAPPING_MISMATCH` | 0 |
| `PARAMETER_MAPPING_MISMATCH` | 0 |
| `RETURN_MAPPING_MISMATCH` | 0 |
| `OVERLOAD_MAPPING_MISMATCH` | 0 |
| `GENERIC_MAPPING_MISMATCH` | 0 |
| `ENUM_VALUE_MISMATCH` | 0 |
| `FLAGS_MAPPING_MISMATCH` | 0 |
| `EVENT_MAPPING_MISMATCH` | 0 |
| `OPERATOR_MAPPING_MISMATCH` | 0 |
| `REF_OUT_MAPPING_MISMATCH` | 0 |
| `LANGUAGE_MAPPING_MISMATCH` | 0 |
| leak/safety categories | 0 |

The whole-profile formal projection counters are unchanged. The missing type
already contributed `value__` to the 49 enum-storage exclusions in the expected
profile, so implementation does not move that count. Manual/applied allowlists
and unmeasured structural categories remain zero, and `MISSING_MEMBER` stays at
131 because no partial type was touched.

The verifier suite grew from 201 to 235 mutation/self-tests. The 22 focused
RenderTargetUsage mutations cover a missing type, a Framework-root namespace, a
`RenderTarget` subnamespace, struct kind, `OptionSet` projection, `UInt32` and
`Int` raw types, incorrect flags metadata, each of the three wrong literals,
each of the three missing cases including the middle and final ones, two
renamed cases, an unexpected extra `Default` case, and public `description`,
`ToString`, predicate-helper, consumer-helper, and native-mapping leakage. The
12 additional assertions require the reference model to be diagnostic-free, the
Swift kind to be `enum`, flags to be false, the raw type to be a *verified*
`Int32`, the member count to be exactly 3, the raw table to be exactly the
pinned table, every identity to be a static literal, no public constructor
identity to exist, `value__` to be excluded, an incorrectly *required* `value__`
to fail, and an incorrectly *exposed* `value__` to fail. The flags and
`OptionSet` checks are general ordinary-enum invariants, not a
RenderTargetUsage suppression. No allowlist entry was added.

## Regenerated dependency boundary

The public-signature dependency graph was regenerated with the retained
`tools/api_compat/dependency_graph.py` after completion. RenderTargetUsage has
zero XNA dependencies, so it was trivially dependency-complete before this
milestone.

It directly appears in exactly three types that remain missing:

- `Microsoft.Xna.Framework.Graphics.PresentationParameters`
- `Microsoft.Xna.Framework.Graphics.RenderTarget2D`
- `Microsoft.Xna.Framework.Graphics.RenderTargetCube`

There is no direct partial reverse consumer. The transitive reverse closure
contains 50 still-missing types and all five unchanged partial types
(`Game`, `GraphicsDeviceManager`, `GraphicsDevice`, `Texture2D`,
`SpriteBatch`).

**Every one of these consumers remains deferred.** No dependent implementation
was started, and a dependency edge is not a scope argument: the graph explains
why this type was selected, not what else may be built.

## Native and runtime boundary

RenderTargetUsage is managed metadata only. No CNAShim declaration,
NativeManifest row, NativeFunctions entry, `CNA_RENDER_TARGET_USAGE_*`
constant, C structure, C layout, callback, canonical CNA source, or ABI symbol
changed. Exact ABI counts remain 29 bound functions, 91 prototype type
positions, 91 C/Swift measurements, 18 layouts, two callbacks, and 214
constants, with zero missing header symbols, zero missing library symbols, and
zero ABI mismatches. Foundation 13 has zero native surface.

Capability is limited to
`RenderTargetUsage managed enum contract: VERIFIED_MANAGED`.

The literal names describe XNA's render-target content-preservation policy, but
completing the enum does **not** prove that CNA-Swift implements any of it.
Discard semantics, preserve semantics, platform-defined semantics, and
render-target content restoration are neither implemented nor claimed.
`RenderTarget2D`, `RenderTargetCube`, `RenderTargetBinding`,
`PresentationParameters` and its `RenderTargetUsage` member, render-target
creation, depth attachment, MSAA behavior, `GraphicsDevice.SetRenderTarget`,
and native usage mapping all remain deferred. An enum case named
`PreserveContents` is metadata, not a working preservation path.

## Isolated external qualification

The archive consumer built from the exact source archive additionally names
`Microsoft.Xna.Framework.Graphics.RenderTargetUsage` in a fresh package outside
the development checkout, reads all three raw values, round-trips each through
the raw initializer, and confirms an undefined pattern yields `nil` — all with
no render target, no device, and no dependence on the native library for that
qualification. Compiling and naming the type externally is a public-surface
check only, and carries no runtime behavior claim.
