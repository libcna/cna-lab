# Foundation 14 — Pure Managed Batch A

Foundation Milestone 14 is a **multi-type autonomous batch**, not a single-type
closure. It completes **25 entirely missing pure-managed XNA types** carrying
**144 mapped Swift XNA identities**, and stops on batch limit **A — 25 newly
complete types** (the 150-identity limit B was not reached; 144 were mapped).

No CNA source, C ABI, native binding, renderer, device, texture, sprite-batch,
callback, thread-affinity, filesystem, or audio-engine work is included, and
none of the five runtime-partial types was touched.

## Batch composition

Every completed type is one of:

- an ordinary CLR `Int32` enum → Swift `enum: Int32` with explicit raw values;
- a `[Flags]` CLR `Int32` enum → Swift `OptionSet` struct with `Int32` storage;
- a sealed sequential value struct with pure field storage;
- a pure abstract interface → Swift `protocol`.

### Ordered completed-type matrix

| # | XNA type | Swift kind | CLR / expected / target | Dependencies | Direct missing reverse | Direct partial reverse | Transitive missing reach |
|---:|---|---|---:|---|---:|---:|---:|
| 1 | `Graphics.GraphicsProfile` | enum | 3 / 2 / 2 | — | 2 | 2 | 53 |
| 2 | `Graphics.PresentInterval` | enum | 5 / 4 / 4 | — | 1 | 0 | 53 |
| 3 | `Graphics.VertexElementFormat` | enum | 13 / 12 / 12 | — | 0 | 0 | 50 |
| 4 | `Graphics.VertexElementUsage` | enum | 14 / 13 / 13 | — | 0 | 0 | 50 |
| 5 | `Graphics.CompareFunction` | enum | 9 / 8 / 8 | — | 2 | 0 | 50 |
| 6 | `Graphics.CubeMapFace` | enum | 7 / 6 / 6 | — | 2 | 1 | 50 |
| 7 | `Graphics.IndexElementSize` | enum | 3 / 2 / 2 | — | 2 | 0 | 50 |
| 8 | `Graphics.Blend` | enum | 14 / 13 / 13 | — | 1 | 0 | 50 |
| 9 | `Graphics.BlendFunction` | enum | 6 / 5 / 5 | — | 1 | 0 | 50 |
| 10 | `Graphics.ColorWriteChannels` | OptionSet struct | 7 / 6 / 6 | — | 1 | 0 | 50 |
| 11 | `Graphics.CullMode` | enum | 4 / 3 / 3 | — | 1 | 0 | 50 |
| 12 | `Graphics.StencilOperation` | enum | 9 / 8 / 8 | — | 1 | 0 | 50 |
| 13 | `Graphics.TextureAddressMode` | enum | 4 / 3 / 3 | — | 1 | 0 | 50 |
| 14 | `Graphics.TextureFilter` | enum | 10 / 9 / 9 | — | 1 | 0 | 50 |
| 15 | `Graphics.ClearOptions` | OptionSet struct | 4 / 3 / 3 | — | 0 | 1 | 50 |
| 16 | `Graphics.GraphicsDeviceStatus` | enum | 4 / 3 / 3 | — | 0 | 1 | 50 |
| 17 | `Graphics.PrimitiveType` | enum | 5 / 4 / 4 | — | 0 | 1 | 50 |
| 18 | `Graphics.EffectParameterClass` | enum | 6 / 5 / 5 | — | 2 | 0 | 26 |
| 19 | `Graphics.EffectParameterType` | enum | 11 / 10 / 10 | — | 2 | 0 | 26 |
| 20 | `Graphics.SetDataOptions` | OptionSet struct | 4 / 3 / 3 | — | 2 | 0 | 2 |
| 21 | `Graphics.VertexElement` | struct | 10 / 10 / 10 | VertexElementFormat, VertexElementUsage | 1 | 0 | 50 |
| 22 | `Graphics.IEffectFog` | protocol | 4 / 4 / 4 | Vector3 | 5 | 0 | 5 |
| 23 | `Graphics.IEffectMatrices` | protocol | 3 / 3 / 3 | Matrix | 5 | 0 | 5 |
| 24 | `Audio.SoundState` | enum | 4 / 3 / 3 | — | 1 | 0 | 3 |
| 25 | `Audio.AudioChannels` | enum | 3 / 2 / 2 | — | 2 | 0 | 2 |

Reverse-consumer columns are the **final-state** graph. Selection used the graph
as it stood before each type was consumed; for example `VertexElementFormat` and
`VertexElementUsage` each had one direct missing reverse consumer
(`VertexElement`) at selection time, and `VertexElement` only became
dependency-complete after both were finished. That mid-batch unlock is why the
graph is regenerated after every completion instead of preselecting the batch.

`CLR / expected / target` is the declared CLR identity count, the count after
formal Swift language-projection exclusions, and the count the compiler Symbol
Graph actually emitted. For every enum the CLR column is exactly one larger
than the other two: the synthetic `value__` storage identity is the existing
enum-storage language mapping and never appears in the public Swift surface.
`ENUM_STORAGE_FIELD_EXCLUSIONS` stays at 49 because it counts all pinned types
that declare `value__`, implemented or not.

### Pinned literal tables

| XNA type | `[Flags]` | Swift shape | Pinned literals |
|---|---|---|---|
| `Graphics.GraphicsProfile` | no | `enum: Int32` | `Reach=0`, `HiDef=1` |
| `Graphics.PresentInterval` | no | `enum: Int32` | `Default=0`, `One=1`, `Two=2`, `Immediate=3` |
| `Graphics.VertexElementFormat` | no | `enum: Int32` | `Single=0`, `Vector2=1`, `Vector3=2`, `Vector4=3`, `Color=4`, `Byte4=5`, `Short2=6`, `Short4=7`, `NormalizedShort2=8`, `NormalizedShort4=9`, `HalfVector2=10`, `HalfVector4=11` |
| `Graphics.VertexElementUsage` | no | `enum: Int32` | `Position=0`, `Color=1`, `TextureCoordinate=2`, `Normal=3`, `Binormal=4`, `Tangent=5`, `BlendIndices=6`, `BlendWeight=7`, `Depth=8`, `Fog=9`, `PointSize=10`, `Sample=11`, `TessellateFactor=12` |
| `Graphics.CompareFunction` | no | `enum: Int32` | `Always=0`, `Never=1`, `Less=2`, `LessEqual=3`, `Equal=4`, `GreaterEqual=5`, `Greater=6`, `NotEqual=7` |
| `Graphics.CubeMapFace` | no | `enum: Int32` | `PositiveX=0`, `NegativeX=1`, `PositiveY=2`, `NegativeY=3`, `PositiveZ=4`, `NegativeZ=5` |
| `Graphics.IndexElementSize` | no | `enum: Int32` | `SixteenBits=0`, `ThirtyTwoBits=1` |
| `Graphics.Blend` | no | `enum: Int32` | `One=0`, `Zero=1`, `SourceColor=2`, `InverseSourceColor=3`, `SourceAlpha=4`, `InverseSourceAlpha=5`, `DestinationColor=6`, `InverseDestinationColor=7`, `DestinationAlpha=8`, `InverseDestinationAlpha=9`, `BlendFactor=10`, `InverseBlendFactor=11`, `SourceAlphaSaturation=12` |
| `Graphics.BlendFunction` | no | `enum: Int32` | `Add=0`, `Subtract=1`, `ReverseSubtract=2`, `Min=3`, `Max=4` |
| `Graphics.ColorWriteChannels` | yes | `OptionSet` struct | `None=0`, `Red=1`, `Green=2`, `Blue=4`, `Alpha=8`, `All=15` |
| `Graphics.CullMode` | no | `enum: Int32` | `None=0`, `CullClockwiseFace=1`, `CullCounterClockwiseFace=2` |
| `Graphics.StencilOperation` | no | `enum: Int32` | `Keep=0`, `Zero=1`, `Replace=2`, `Increment=3`, `Decrement=4`, `IncrementSaturation=5`, `DecrementSaturation=6`, `Invert=7` |
| `Graphics.TextureAddressMode` | no | `enum: Int32` | `Wrap=0`, `Clamp=1`, `Mirror=2` |
| `Graphics.TextureFilter` | no | `enum: Int32` | `Linear=0`, `Point=1`, `Anisotropic=2`, `LinearMipPoint=3`, `PointMipLinear=4`, `MinLinearMagPointMipLinear=5`, `MinLinearMagPointMipPoint=6`, `MinPointMagLinearMipLinear=7`, `MinPointMagLinearMipPoint=8` |
| `Graphics.ClearOptions` | yes | `OptionSet` struct | `Target=1`, `DepthBuffer=2`, `Stencil=4` |
| `Graphics.GraphicsDeviceStatus` | no | `enum: Int32` | `Normal=0`, `Lost=1`, `NotReset=2` |
| `Graphics.PrimitiveType` | no | `enum: Int32` | `TriangleList=0`, `TriangleStrip=1`, `LineList=2`, `LineStrip=3` |
| `Graphics.EffectParameterClass` | no | `enum: Int32` | `Scalar=0`, `Vector=1`, `Matrix=2`, `Object=3`, `Struct=4` |
| `Graphics.EffectParameterType` | no | `enum: Int32` | `Void=0`, `Bool=1`, `Int32=2`, `Single=3`, `String=4`, `Texture=5`, `Texture1D=6`, `Texture2D=7`, `Texture3D=8`, `TextureCube=9` |
| `Graphics.SetDataOptions` | yes | `OptionSet` struct | `None=0`, `Discard=1`, `NoOverwrite=2` |
| `Audio.SoundState` | no | `enum: Int32` | `Playing=0`, `Paused=1`, `Stopped=2` |
| `Audio.AudioChannels` | no | `enum: Int32` | `Mono=1`, `Stereo=2` |

`ColorWriteChannels.All=15` is a pinned composite literal, not an invented
convenience: the contract test asserts `All == Red | Green | Blue | Alpha`
from the pinned raw values alone. `ClearOptions` has no zero literal and
`AudioChannels` starts at `Mono=1`; neither gained an invented `None`.

### The three non-enum closures

`Graphics.VertexElement` is a sealed sequential value struct with four
`assembly` fields (`_offset`, `_format`, `_usage`, `_usageIndex`) behind four
public read/write properties. Every accessor in the pinned IL is a plain field
load or store, and the constructor assigns all four arguments verbatim with no
validation, so no clamp, range check, or normalization was invented.

- `op_Equality` compares `_offset`, then `_usageIndex`, then `_usage`, then
  `_format`, short-circuiting to false on the first difference.
  `op_Inequality` is exactly its negation.
- `Equals(object)` returns false for null, false when the runtime types differ,
  and otherwise defers to `op_Equality`.
- `GetHashCode` boxes the value and calls the internal
  `Microsoft.Xna.Framework.Helpers.SmartGetHashCode`, which pins the box, XORs
  `Marshal.SizeOf(obj) / 4` consecutive `int32` words, and substitutes
  `0x7FFFFFFF` when the accumulated XOR is zero. This layout is exactly four
  `int32` words, and XOR is order independent. This is the same reference hash
  the GamePad value family already reproduces.
- `ToString` is `string.Format(CultureInfo.CurrentCulture,
  "{{Offset:{0} Format:{1} Usage:{2} UsageIndex:{3}}}", ...)`. The doubled
  braces are composite-format escapes, so one brace pair is emitted, and the two
  boxed enum arguments render through `Enum.ToString()` as their declared
  literal names. `VertexElementFormat` and `VertexElementUsage` deliberately
  gain no public string surface, so both name tables stay `private` to
  `VertexElement` — the same containment `DisplayMode` already uses for
  `SurfaceFormat`.

`Graphics.IEffectFog` and `Graphics.IEffectMatrices` are pure abstract
interfaces with no base interface, no method, and only abstract read/write
properties. They carry no behavior at all, so there is nothing to reproduce
beyond the exact requirement set. No XNA conformer exists yet and none was
invented; the projection tests declare private test-only witnesses purely to
prove the Swift requirement sets are satisfiable and exactly the pinned sets.

## Reference provenance

Public shape comes from the pinned contract SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.

Every one of the 25 types was **independently re-derived this milestone** by
disassembling the hash-matched pinned assemblies on the Linux qualification
host and machine-comparing the result against that contract:

```text
Microsoft.Xna.Framework.Graphics.dll
  SHA-256 560080fc39021c611ca9d076dcebed312faf6d7d1413c2dc523683ea635e9f55
  provides 23 of the 25 completed types
Microsoft.Xna.Framework.dll
  SHA-256 38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130
  provides Audio.SoundState and Audio.AudioChannels
```

For all 20 enums the comparison checked type kind, `sealed`, base type, absence
of a declared interface, `System.Int32` underlying storage, the presence of
`value__`, the complete literal name/value table, and declaration order; all 20
matched exactly. `System.FlagsAttribute` is present on exactly
`ColorWriteChannels`, `ClearOptions`, and `SetDataOptions` and absent from the
other 17, so `flags=false` is an **observed absence** in the pinned binaries,
not an assumption. `Blend` carries only a `SuppressMessageAttribute`, which is
not `[Flags]`.

`VertexElement`, `IEffectFog`, and `IEffectMatrices` were read as full IL,
including method bodies, and the behavior above is transcribed from those
bodies. No Microsoft binary or extracted proprietary source is in this
repository or in the source archive.

## Behavior provenance

`PURE_XNA_DERIVED` and `GO_LANGUAGE_PROJECTION`-equivalent Swift projection
evidence are kept in separate test files and separately reported.

- **XNA-derived** (`Tests/CNATests/Foundation14GraphicsEnumContractTests.swift`
  and `Tests/CNATests/Foundation14ManagedTypeContractTests.swift`, both
  extensions of `PureValueTests` and both counted in the behavior corpus): the
  pinned kind, `[Flags]` state, underlying storage, complete literal tables,
  the `ColorWriteChannels.All` composite identity, and every `VertexElement`
  construction, equality, hash, and string observation.
- **Swift language projection** (`...ProjectionTests.swift`, separate
  `XCTestCase` classes, **not** counted as XNA behavior): raw-value
  round-tripping, `nil` for undefined raw patterns, arbitrary `Int32` raws
  staying representable in `OptionSet` storage, `union`/`intersection`/`insert`,
  Swift value-copy semantics, and protocol-witness satisfiability.

Swift value-copy behavior, permissive `OptionSet` raw storage, and `init?(rawValue:)`
returning `nil` are Swift language semantics and are never labelled XNA runtime
behavior.

## Structural scoreboard

```text
                                  BEFORE      AFTER     DELTA
REFERENCE_TYPES                      257        257         0
REFERENCE_MEMBERS                   2964       2964         0
EXPECTED_SWIFT_TYPES                 257        257         0
EXPECTED_SWIFT_MEMBERS              2887       2887         0
TARGET_TYPES                          73         98       +25
TARGET_MEMBERS                      1416       1560      +144
TOTAL_DIAGNOSTICS                    335        310       -25
MISSING_TYPE                         184        159       -25
MISSING_MEMBER                       131        131         0
COMPLETE_TYPES                        68         93       +25
PARTIAL_TYPES                          5          5         0
MISSING_TYPES                        184        159       -25
UNEXPECTED_TYPE                        0          0         0
UNEXPECTED_MEMBER                      0          0         0
TYPE_KIND_MISMATCH                     0          0         0
BASE_MAPPING_MISMATCH                  2          2         0
INTERFACE_MAPPING_MISMATCH             1          1         0
FIELD_MAPPING_MISMATCH                 0          0         0
PROPERTY_MAPPING_MISMATCH              1          1         0
METHOD_SIGNATURE_MAPPING_MISMATCH      0          0         0
PARAMETER_MAPPING_MISMATCH             0          0         0
RETURN_MAPPING_MISMATCH                0          0         0
OVERLOAD_MAPPING_MISMATCH             16         16         0
GENERIC_MAPPING_MISMATCH               0          0         0
ENUM_VALUE_MISMATCH                    0          0         0
FLAGS_MAPPING_MISMATCH                 0          0         0
EVENT_MAPPING_MISMATCH                 0          0         0
OPERATOR_MAPPING_MISMATCH              0          0         0
REF_OUT_MAPPING_MISMATCH               0          0         0
LANGUAGE_MAPPING_MISMATCH              0          0         0
INTERNAL_TYPE_LEAK                     0          0         0
RAW_HANDLE_LEAK                        0          0         0
PUBLIC_NATIVE_FFI_LEAK                 0          0         0
UNMEASURED_STRUCTURAL_CATEGORY         0          0         0
ALLOWLIST_ENTRIES                      0          0         0
APPLIED_ALLOWLIST_ENTRIES              0          0         0
LANGUAGE_PROJECTION_EXCLUSIONS       113        114        +1
ENUM_STORAGE_FIELD_EXCLUSIONS         49         49         0
FINALIZER_LANGUAGE_MAPPINGS           28         28         0
NAMESPACE_MARKERS                      7          8        +1
INHERITED_MEMBER_PROJECTIONS           3          3         0
PROTOCOL_WITNESS_MEMBER_PROJECTIONS   26         26         0
ARRAY_MUTATION_MAPPINGS               19         19         0
COMPARABLE_INTERFACE_PROJECTIONS       1          1         0
COLLECTION_INTERFACE_PROJECTIONS       1          1         0
ENUMERATOR_SUPPORT_PROJECTIONS         9          9         0
INDEXED_PROPERTY_ACCESSOR_PROJECTIONS  4          4         0
GLOBAL_OPTIONAL_OPERATOR_PROJECTIONS   2          2         0
NONPUBLIC_CONSTRUCTION_PROJECTIONS     4          4         0
```

Two counters moved by design. `SoundState` and `AudioChannels` are the first
types in `Microsoft.Xna.Framework.Audio`, so that namespace gained its marker:
`NAMESPACE_MARKERS` 7 → 8 and therefore `LANGUAGE_PROJECTION_EXCLUSIONS`
113 → 114. A namespace marker is a formally measured language projection, not
an allowlist entry, and it adds no XNA identity.

Every mismatch, unexpected-surface, leak, allowlist, and unmeasured counter is
unchanged. All 151 non-`MISSING_TYPE` diagnostics still belong exclusively to
the five runtime-partial types, and their per-type split is byte-identical
before and after the batch:

```text
partial type                 diagnostics   of which MISSING_MEMBER
Game                                  22                        21
GraphicsDeviceManager                 29                        27
Graphics.GraphicsDevice               57                        54
Graphics.SpriteBatch                  27                        16
Graphics.Texture2D                    16                        13
TOTAL                                151                       131
```

That is `MISSING_MEMBER=131` plus the 16 overload, two base, one property, and
one interface mismatches those five types have always owned. The 25 batch types
carry **zero** diagnostics between them.

## Verifier coverage added

`API_COMPAT_SELF_TESTS` rose 235 → **1259**. The batch adds two data-driven
fixture generators rather than transcribed per-type blocks, so a type cannot be
registered without negative coverage for its own exact literals or members.

For each of the 22 batch enums: missing type, wrong namespace, wrong Swift kind
in both directions, wrong raw type, `Int` raw type, inverted `[Flags]` metadata,
an invented extra literal, and invented `description` / `ToString` / predicate /
native-mapping helpers; plus, **for every pinned literal individually**, a wrong
raw value, a removed literal, and a renamed literal; plus required-`value__` and
publicly-exposed-`value__` fixtures and nine reference-model assertions.

For `VertexElement`, `IEffectFog`, and `IEffectMatrices`: missing type, wrong
namespace, wrong Swift kind, three invented-member fixtures; plus, **for every
pinned member individually**, removal, rename, wrong Swift kind, flipped static
identity, wrong result type, flipped mutability, and — where the member takes
parameters — a wrong parameter type, a wrong external label, and a dropped
parameter.

Four negative controls confirmed the fixtures fail on real defects rather than
passing vacuously:

```text
removed GraphicsProfile from rawTypeChecks   -> 3 self-test failures
PrimitiveType.LineList = 9 in Swift source   -> ENUM_VALUE_MISMATCH 0 -> 1
VertexElement.Offset made read-only          -> PROPERTY_MAPPING_MISMATCH 1 -> 2
IEffectFog.FogEnd setter dropped             -> PROPERTY_MAPPING_MISMATCH 1 -> 2
```

All four were reverted and the sources verified byte-identical afterwards.

## Dependency-graph correction

`tools/api_compat/dependency_graph.py` built its nodes from raw CLR type names
while `complete`/`partial`/`missing` came from the strict report, which uses
**mapped** names. A CLR nested name (`A+B`) and a generic collision name
(`ContentTypeReader` + arity suffix) therefore never matched a graph key, so
`forward.get(name)` returned an empty set and the type was ranked as trivially
dependency-complete.

This was a live ranking defect, not a cosmetic one: it advertised six candidates
as dependency-complete when every one of them has an unmapped dependency.

```text
ContentTypeReaderOfT                   needs ContentReader, ContentTypeReader
ModelBoneCollection.Enumerator         needs ModelBone
ModelEffectCollection.Enumerator       needs Effect
ModelMeshCollection.Enumerator         needs ModelMesh
ModelMeshPartCollection.Enumerator     needs ModelMeshPart
TouchCollection.Enumerator             needs TouchLocation
```

Graph nodes are now mapped through `mapping-rules.json` exactly as the strict
verifier maps them. Measured against the unchanged Foundation-13 report this
moves `DEPENDENCY_COMPLETE_MISSING_TYPES` from 71 to 65 and removes all six
false positives; it does **not** change the Foundation-13 selection, which
still ranks `GraphicsProfile` first.

This is a distinct issue from the historical 71/72 `GameComponent` nuance, which
concerns whether declared direct interfaces count as dependencies. That question
is untouched: `IGameComponent` and `IUpdateable` remain missing, so
`GameComponent` remains correctly dependency-incomplete.

One behavior-corpus grouping defect was fixed alongside it. The `Color` group
pattern was `Color`, which silently absorbed `ColorWriteChannels`.
`Microsoft.Xna.Framework.Color` and
`Microsoft.Xna.Framework.Graphics.ColorWriteChannels` are distinct XNA types, so
the pattern is now `Color(?!WriteChannels)` and the `Color` group stays at 13.

## Skipped candidates

Each of these outranked at least one type that was consumed, or ranked at the
head of the graph, and each was materially inspected before being skipped. The
loop was never stopped by one of them; it skipped and continued.

| Candidate | Rank at consideration | Reason skipped | Category |
|---|---|---|---|
| `Graphics.DisplayModeCollection` | reach 53, top of graph | Its only members are `GetEnumerator` and an indexer over `DisplayMode`. `DisplayMode` has no public constructor and instances exist only through `GraphicsAdapter` mode enumeration, which does not exist. Any implementation is either permanently empty or fabricates adapter data. | fake display/adapter capability; no-op/stub behavior |
| `Graphics.PresentationParameters` | reach 52 | The `GraphicsDevice` creation/reset descriptor, including a `DeviceWindowHandle` platform window handle and a `Clone` whose defaults come from the device-creation path. | device creation/reset/loss handling; platform service |
| `Graphics.ResourceCreatedEventArgs` | reach 50 | Its base `System.EventArgs` maps to `CNAEventArgs`, which is a `struct` in this codebase. A Swift class cannot inherit from a struct, so completing it requires converting `CNAEventArgs` to a class, which changes the signature of `Game.OnExiting(_:args:)` — a member of a protected partial type. | would modify a protected partial type |
| `Graphics.ResourceDestroyedEventArgs` | reach 50 | Same `System.EventArgs` base blocker. | would modify a protected partial type |
| `Graphics.EffectAnnotation` | reach 25 | Its `GetValueSingle`/`GetValueMatrix`/… members read values out of a compiled effect. No deterministic semantics exist without an effect runtime, and it would also have exceeded the identity budget. | uncertain semantics lacking authoritative evidence |
| `Design.MathTypeConverter` | reach 12 | Derives from `System.ComponentModel.TypeConverter`, an unmapped BCL type requiring a new projection subsystem. | substantial new BCL projection framework |
| `Content.ContentManager` | reach 8 | Content pipeline plus filesystem, service provider, and disposal lifetime. | filesystem/platform service; ownership/lifetime architecture |
| `Audio.AudioEmitter` | reach 5 | Not a pure managed data holder. Its single field is the native `UnsafeNativeStructures.XACT_EMITTER_DATA` interop struct; every vector accessor calls `UnsafeNativeStructures.FlipHandedness`, and the constructor also initialises native-only `ChannelCount`, `ChannelRadius`, and `CurveDistanceScaler` fields that have no public surface. Faithful implementation is native XACT interop. | audio engine / native audio |
| `Audio.AudioListener` | reach 5 | Same native XACT interop storage and handedness conversion. | audio engine / native audio |
| `IGameComponent` | reach 5 | Declared in `Microsoft.Xna.Framework.Game.dll`, which is outside the pinned assembly set, so the per-milestone independent IL re-derivation used for every other type in this batch is not available from pinned material. It is also one of the two interfaces named in the retained Foundation-13 `GameComponent` dependency nuance, which is explicitly not being reopened. | deferred boundary; provenance outside the pinned set |
| `Audio.AudioStopOptions` | reach 5 | Declared in `Microsoft.Xna.Framework.Xact.dll`, outside the pinned assembly set. Surface is known from the pinned contract, but it cannot be independently re-derived the way the rest of this batch was. | provenance outside the pinned set |
| `Input.Touch.TouchLocationState`, `Input.Touch.GestureType`, `Input.Touch.TouchPanelCapabilities`, `Media.VideoSoundtrackType`, `Audio.RendererDetail`, `IUpdateable`, `IDrawable` | reach 1–4 | Same provenance boundary: declared outside the two pinned assemblies. `IUpdateable` and `IDrawable` additionally declare CLR events. | provenance outside the pinned set |
| `Input.MouseState` | reach 1 | Pinned-derivable and pure managed, but 14 identities would have exceeded the remaining budget after the 23rd type. It is a genuine future candidate, not an unsafe one. | batch budget |
| `Graphics.SpriteFont`, `Graphics.DeviceLostException`, `Graphics.DeviceNotResetException`, `Graphics.NoSuitableGraphicsDeviceException`, `IGraphicsDeviceManager`, `FrameworkDispatcher`, `TitleContainer`, the `Content.ContentSerializer*` attributes | reach 0 | Glyph/texture data, `System.Exception` base projection, device creation, audio pumping, filesystem, and `System.Attribute` base projection respectively. | new BCL base projection / native / platform service |

No candidate was skipped because it was merely inconvenient, and no new mapping
policy was created to make a candidate fit.

## Checkpoints

```text
after 20 completed types   focused unit tests, focused verifier fixtures,
                           focused behavior, debug build
                           full swift test, verifier self-test, strict report,
                           leak-only, behavior corpus, regenerated graph

after 25 completed types   full final qualification, below
```

## Final qualification

```text
SWIFT_VERSION=6.0.3
SWIFT_TARGET=x86_64-pc-linux-gnu
SWIFT_TOOLS_VERSION=5.9
DEBUG_BUILD=PASS
RELEASE_BUILD=PASS
WARNINGS_AS_ERRORS=PASS_DEBUG_AND_RELEASE
DEBUG_TESTS=147 PASS
RELEASE_TESTS=147 PASS
MANAGED_TESTS=137 PASS_WITHOUT_CNA_NATIVE_LIBRARY (10 native tests skipped)
SYMBOL_GRAPH=PASS
API_SELF_TESTS=1259 PASS
NORMAL_STRICT=EXPECTED_RED_DEFERRED_PROFILE_ONLY (exit 1)
LEAK_ONLY=PASS (exit 0)
PURE_XNA_DERIVED=1348/1348/0
SWIFT_ASAN=PASS_PURE_CORPUS_DETECT_LEAKS_DISABLED (102 tests)
SWIFT_TSAN=PASS_MANAGED_CORPUS (147 tests, 10 skipped, no data race reported)
NATIVE_CNA_SANITIZER=NOT_INSTRUMENTED
GAMEPAD_NATIVE_FAILURES=0
```

`swift test -c release` and the ASan/TSan runs use dedicated scratch paths so
the warm incremental `.build` tree is never invalidated.

## Native and ABI boundary

Foundation 14 has **zero** native surface. No `CNAShim` declaration,
`NativeManifest` row, `NativeFunctions` entry, C structure, C layout, callback,
constant, canonical CNA source file, or ABI symbol changed. The regenerated
native ABI report is byte-identical:

```text
CNA_ABI_VERSION=0.7.0
BOUND_FUNCTIONS=29
PROTOTYPE_TYPE_POSITIONS=91
C_SWIFT_MEASUREMENTS=91
LAYOUTS=18
CALLBACKS=2
CONSTANTS=214
MISSING_HEADER_SYMBOLS=0
MISSING_LIBRARY_SYMBOLS=0
ABI_MISMATCHES=0
```

Native stress is unchanged: 20 Game, 20 Game-recreation, 20 Texture2D, 20
SpriteBatch and 20 callback-error cycles, 50 GamePad `GetState` cycles per mode
and 20 capability cycles, with `NATIVE_CRASHES=0`, `OBSERVED_UAF=0`, and
`OBSERVED_DOUBLE_FREE=0`.

## Capability boundary — completing a type is not a runtime claim

All 25 types are managed metadata or pure managed value logic. Their names
describe XNA concepts that CNA-Swift does **not** implement, and none of the
following is claimed:

- `GraphicsProfile` complete does **not** imply Reach/HiDef profile selection,
  `GraphicsAdapter`, or any capability negotiation.
- `PresentInterval`, `ClearOptions`, `GraphicsDeviceStatus`, `PrimitiveType`
  complete do **not** imply present intervals, vsync, a working `Clear` option
  path, device-lost handling, or `DrawPrimitives`.
- `Blend`, `BlendFunction`, `ColorWriteChannels`, `CompareFunction`,
  `StencilOperation`, `CullMode`, `TextureFilter`, `TextureAddressMode`,
  `SetDataOptions` complete do **not** imply `BlendState`,
  `DepthStencilState`, `RasterizerState`, `SamplerState`, buffer updates, or
  any render-state pipeline.
- `VertexElementFormat`, `VertexElementUsage`, `VertexElement`,
  `IndexElementSize` complete do **not** imply `VertexDeclaration`,
  `IVertexType`, vertex or index buffers, or any vertex submission path.
- `CubeMapFace` complete does **not** imply `TextureCube` or cube render
  targets.
- `EffectParameterClass`, `EffectParameterType`, `IEffectFog`,
  `IEffectMatrices` complete do **not** imply `Effect`, `EffectParameter`,
  shader compilation, fog, or any effect matrix being applied to anything.
- `SoundState` and `AudioChannels` complete do **not** imply `SoundEffect`,
  `SoundEffectInstance`, playback, mixing, or any audio device. There is no
  audio engine.

Capability entries added for this batch are exactly the managed contracts and
nothing more.

## Exact source archive and isolated external qualification

The batch archive is a deterministic exact-worktree source archive with a single
`cna-swift` package root. Its identity is a final handoff artifact and is
deliberately not embedded in the source it contains. The audited gates are:

```text
TWO_PASS_DETERMINISM=PASS
SOURCE_ARCHIVE_ENTRIES=169
PACKAGE_ROOTS=1 (cna-swift)
FORBIDDEN_ENTRY_MATCHES=0
GIT_METADATA_ENTRIES=0
NATIVE_LIBRARY_ENTRIES=0
MICROSOFT_REFERENCE_BINARIES=0
UNSAFE_PATH_ENTRIES=0
DEVELOPER_PATH_LEAKS=0
WORKTREE_CONTENT_MISMATCHES=0
```

`WORKTREE_CONTENT_MISMATCHES=0` is bidirectional: every archive entry byte-matches
the worktree file it names, and every non-excluded worktree file is present.

A fresh consumer package is then built outside the development checkout against
the extracted, read-only archive. The canary was extended this milestone to name
**all 25** Foundation-14 types in their exact namespaces, read every pinned raw
value, confirm `nil` for undefined raw patterns (including `AudioChannels`
having no zero literal), exercise `VertexElement` construction, mutation,
equality, the zero-XOR hash substitution and the exact string, and conform two
external witnesses to `IEffectFog` and `IEffectMatrices` — with no native
library needed for any of that part.

```text
DEBUG_BUILD=PASS
RELEASE_BUILD=PASS
RUN_60=PASS
RUN_600=PASS
DEVELOPMENT_CHECKOUT_REFERENCES=0
```

One genuine toolchain defect surfaced here and is recorded rather than papered
over. Swift 6.0.3 asserts and crashes the frontend while mangling debug info for
an existential named through a nested typealias — `let x: G.IEffectFog` where
`G` is a typealias for `Microsoft.Xna.Framework.Graphics`:

```text
While emitting IR SIL function "@$s13ArchiveCanary33qualifyFoundation14ManagedSurfaceyyKF".
While mangling type for debugger type 'any G.IEffectFog'
error: compile command failed due to signal 6
```

Spelling the protocol in full compiles and runs correctly. This is a compiler
bug, not a property of the projected protocol: the same protocol used through
its full name works in the package tests, the release build, and the isolated
consumer. The canary now spells it in full with a comment recording why.

## Template

The maintained template is unchanged and still passes its gates against this
batch:

```text
TEMPLATE_COMMIT=86687f62c3a13ee2b59798f338fc083f7399f447
SOURCE_CHANGED=NO
WORKTREE_CLEAN=YES
DEBUG_BUILD=PASS
RELEASE_BUILD=PASS
RUN_60=PASS  updates=60 draws=60 viewport=800x480 texture=128x128
RUN_600=PASS updates=600 draws=600 viewport=800x480 texture=128x128
```

The template declares no test target, so there is no template test gate to run.
Swift has no `-trimpath` equivalent and this repository has never maintained one;
the established build gates are debug and release with warnings-as-errors.
