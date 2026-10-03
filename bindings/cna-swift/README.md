# CNA-Swift

CNA-Swift is a real Swift projection of the retained Microsoft XNA Framework
4.0 public profile over the canonical CNA C ABI. All 257 retained types are
strict-complete; runtime capabilities CNA cannot provide remain explicit
refusals rather than silent compatibility claims.

The strict public identity is `Microsoft.Xna.Framework...`. The qualified
foundation has a compiler Symbol Graph scoreboard, ABI-0.35 admission, a
reviewed typed function table, owner-thread/generation/ownership enforcement,
callback error containment, and a native Game/2D/input canary. Its managed
surface includes exact binary32 linear algebra and intersection types, Color,
both packed-vector protocols, all 17 concrete PackedVector formats, and the
complete six-type Curve family. It also includes the exact eleven-type XNA
GamePad family through real canonical CNA state, capability, packet, dead-zone,
and vibration routes. It also includes the standalone managed
`DisplayOrientation` flags type without claiming platform rotation or window
orientation functionality, plus the standalone managed `BufferUsage` flags
type without claiming buffer or GPU resource support, plus the standalone
managed non-flags `DepthFormat` enum without claiming depth/stencil buffers or
renderer support, plus the standalone
managed non-flags `FillMode` enum without claiming rasterizer or wireframe
rendering support, plus the standalone managed non-flags `SurfaceFormat` enum
without claiming texture, render-target, display, DXT, HDR, or GPU format
support, plus the standalone managed `DisplayMode` descriptor class without
claiming monitor enumeration, display-mode discovery, or resolution switching,
plus the standalone managed non-flags `RenderTargetUsage` enum without claiming
render targets or any content discard/preserve behavior.

Foundation 14 adds a further 25 pure-managed types in one batch — 22 exact
enums, the `VertexElement` value struct, and the `IEffectFog` and
`IEffectMatrices` protocols — covering graphics profile, presentation, device
status, primitive, clear, render-state, vertex-declaration, cube-face, effect
parameter and audio metadata. None of them claims any renderer, device, buffer,
effect or audio runtime support.

Foundation 15 adds the managed `PresentationParameters` descriptor class
without claiming device creation, reset, adapter selection, back-buffer
creation, native window handling, presentation, or render targets. Its
`DeviceWindowHandle` is the first XNA public signature to use the general
`System.IntPtr -> Swift Int` language projection: an opaque pointer-width
signed value held as pure descriptor state, never dereferenced, resolved, or
handed to CNA. See `docs/xna-swift-mapping.md` for the general rule.

Foundation 16 adds four more pure-managed types — the `MouseState` value
struct and the `MediaState`, `MediaSourceType` and `MicrophoneState` enums —
without claiming any mouse device, cursor, microphone, capture, media player,
media library, or video support. `MouseState` is a value snapshot with no
producer: `Input.Mouse` is not implemented.

Foundation 17 registers five more Microsoft XNA assemblies as authoritative
reference inputs — `Game`, `Input.Touch`, `Xact`, `Video` and `Storage` — after
machine-comparing their public metadata against the retained contract, which
reproduces all 257 types and 2,964 members exactly from seven hash-identified
files. It then consumes the first seven types that unblocks: the `GestureType`
OptionSet, the `TouchLocationState`, `VideoSoundtrackType` and
`AudioStopOptions` enums, the `TouchPanelCapabilities` value struct, and the
`IGameComponent` and `IGraphicsDeviceManager` protocols. No touch panel, XACT
engine, video player, storage device, or graphics device capability is claimed;
nothing conforms to either protocol.

Foundation 18 adds the `TouchLocation` and `GestureSample` value structs and
the `DisplayModeCollection` descriptor, without claiming any touch panel,
display enumeration, or adapter capability. None of the three has a public
constructor path to real data: `TouchPanel` and `GraphicsAdapter` are not
implemented.

Foundation 19 decides and implements the general CLR event projection. Every
public XNA event maps to exactly one get-only `CNAEvent<TArgs>` property keeping
its XNA name; the CLR `add_`/`remove_`/`raise_` accessors are never XNA
identities. Removal uses an opaque `CNAEventSubscription` token, because CLR
matches handlers by delegate identity and Swift closures have none — recorded as
a measured language projection, not as equivalence. Raising lives on a separate
`CNAEventSource<TArgs>` that is *composed* with the consumer view rather than
derived from it, so a consumer holding a `CNAEvent` has no downcast to `Raise`,
and an external package can still conform to `IUpdateable` or `IDrawable` and
raise its own events. `CNAEventArgs` becomes an `open class` so the CLR
event-argument hierarchy is expressible, with `System.EventArgs` as a *measured*
base. The milestone completes `IUpdateable`, `IDrawable`,
`GameComponentCollectionEventArgs`, `ResourceCreatedEventArgs` and
`ResourceDestroyedEventArgs`, and adds no CNA ABI. No event on `Game`,
`GraphicsDeviceManager` or `GraphicsDevice` is implemented: those need real
lifecycle and native raising, and an event that never fires is not implemented.

Foundation 20 adds the `AudioListener` data holder, the `TouchCollection` value
collection with its nested `Enumerator`, and the `Media.Video` descriptor,
claiming no audio, XACT, touch, or video capability: `Cue.Apply3D` and `TouchPanel` are not implemented, so nothing
produces a live listener consumer or a live touch collection, and `Video` has no
public constructor because its only XNA producer is `ContentManager`. `AudioListener`
reproduces the XACT handedness flip its pinned IL applies, including the
negative-zero Z its default `Position` and `Velocity` actually carry.

The old flat API and known fake behaviors are absent.

## Measured surface

```text
REFERENCE_TYPES=257
REFERENCE_MEMBERS=2964
EXPECTED_SWIFT_TYPES=257
EXPECTED_SWIFT_MEMBERS=2887
TARGET_TYPES=257
TARGET_MEMBERS=2890
TOTAL_DIAGNOSTICS=0
COMPLETE_TYPES=257
PARTIAL_TYPES=0
MISSING_TYPES=0
MISSING_MEMBER=0
REFERENCE_RETURN_PROJECTIONS=369
PROVEN_NULLABLE_RETURN_PROJECTIONS=113
PROVEN_NONNULL_RETURN_PROJECTIONS=134
UNKNOWN_RETURN_NULLABILITY_PROJECTIONS=122
BCL_BASE_PROJECTIONS=21
PROJECTED_BCL_BASE_TYPES=21
PENDING_BCL_BASE_TYPES=0
BCL_INHERITED_MEMBER_PROJECTIONS=144
BCL_SUPPORT_TYPE_MEASUREMENTS=45
```

Normal strict verification is green. No internal type, pointer, native handle,
or public FFI declaration leaks into the XNA surface. Every category that would
mean the projection *disagrees* with XNA —
kind, base, interface, field, property, signature, parameter, return, generic,
enum value, flags, event, operator, ref/out, inheritance, unexpected type or
member, unmeasured category, and all three leak categories — is 0. See `docs/generated/api-compat-report.json` and
`docs/generated/missing-type-inventory.md` for the exact inventory.

The strict-complete managed foundation includes MathHelper, Point, Rectangle,
GameTime, PlayerIndex, Vector2/3/4, Quaternion, Matrix, Viewport, Plane, Ray,
BoundingBox/Sphere/Frustum and their enums, keyboard values, SpriteSortMode,
SpriteEffects, Color, the packed protocols, all concrete PackedVector formats,
Curve, CurveKey, CurveKeyCollection, CurveContinuity, CurveLoopType,
CurveTangent, all eleven GamePad-family types, DisplayOrientation, BufferUsage,
DepthFormat, FillMode, SurfaceFormat, DisplayMode, RenderTargetUsage, the
25 Foundation-14 pure-managed types listed below, PresentationParameters,
MouseState, MediaState, MediaSourceType, MicrophoneState, the seven
Foundation-17 types, TouchLocation, GestureSample, DisplayModeCollection,
IUpdateable, IDrawable, GameComponentCollectionEventArgs,
ResourceCreatedEventArgs, ResourceDestroyedEventArgs, AudioListener,
TouchCollection, TouchCollection.Enumerator, Media.Video, AudioEmitter,
IGraphicsDeviceService, GameComponentCollection and VisualizationData.
Every retained type and member identity is projected and qualified.

Foundation 104 also completes the five-type Content reader family plus
`ContentManager`'s reader-facing protected surface. It provides a real
`CNABinaryReader` superclass, typed and erased content-reader dispatch,
uncompressed XNB loading with shared and external references, and
`CNAResourceManager`/`ResourceContentManager` integration. The end-to-end XNB
and resource fixtures are project-authored; no Microsoft runtime binary is
packaged and compressed XNB is reported as unsupported.

Foundation 105 completes all thirteen `Microsoft.Xna.Framework.Design` types.
The value converters provide culture-aware text round trips, stable property
descriptors and `CreateInstance`; their `InstanceDescriptor` conversion carries
and invokes a real constructor identity. The selected .NET Framework 4.0
ComponentModel/reflection support is implemented in Swift and adds no runtime
dependency on Microsoft assemblies.

## BCL base classes

A CLR class used as the direct base of an XNA class projects through **real
Swift class inheritance**, and the CLR generic argument is preserved:

```swift
public final class GameComponentCollection:
    CNACollection<any Microsoft.Xna.Framework.IGameComponent>
```

`CNACollection`, `CNAReadOnlyCollection` and `CNAList` are the projections of
`System.Collections.ObjectModel.Collection<T>`, `ReadOnlyCollection<T>` and
`System.Collections.Generic.List<T>`, derived from a hash-registered Microsoft
.NET Framework 4.0 `mscorlib` admitted through a **separate** BCL authority
registry — `tools/api_compat/bcl-authorities.json`, audited by
`tools/api_compat/bcl_authority_audit.py` against a pinned selected-shape
manifest, sentinel facts, mutation self-tests, an independent metadata reader
and negative controls that must be refused.

They live outside `Microsoft.Xna.Framework`: they are language/BCL-support API,
not XNA types, and are counted in no XNA scoreboard. `REFERENCE_MEMBERS` stays
2,964 and `EXPECTED_SWIFT_MEMBERS` stays 2,887; the public surface an XNA type
inherits from a BCL base is counted separately as
`BCL_INHERITED_MEMBER_PROJECTIONS`.

Both collection families are `open` classes over a reference-typed backing
store, because both CLR wrapping constructors store the caller's list **live**
rather than copying it — a Swift `Array` would discard both the reference
identity and the live view. `CNACollection`'s public surface is `final` and only
its four protected hooks (`ClearItems`, `InsertItem`, `RemoveItem`, `SetItem`)
are `open`, reproducing `mscorlib`'s `virtual final` split exactly: a subclass
changes behaviour only through the hooks.

The base is measured in two halves, because the compiler's `inheritsFrom`
relationship names only the generic symbol: the superclass **identity** comes
from that relationship, and the generic **argument** is supplemented from the
compiled Swift source declaration, exactly as enum raw values already are. A
dropped base, a wrong support class, an `Array` substitution, composition in
place of inheritance, an `Any` erasure and a wrong element type are all
`BASE_MAPPING_MISMATCH`; unreadable source evidence is reported as unmeasured
rather than assumed. There is no allowlist.

`System.Exception`, `System.Attribute`, `Dictionary<K,V>`,
`ExpandableObjectConverter` and `BinaryReader` are admitted as real Swift base
classes where selected XNA types inherit them. Foundation 104 adds the exact
26-member `BinaryReader` subset reachable through `ContentReader` and refuses
the `Encoding` constructor and `ReadDecimal` under the demand rule.

## Throwing property accessors

Swift has no throwing property setter, and the compiler separately refuses any
`set` beside a getter that is `throws`. CNA-Swift projects CLR properties
**per accessor**: the getter keeps property or subscript syntax and gains
`get throws` exactly when the XNA getter is fallible, and a setter Swift cannot
express becomes the method `Set<PropertyName>` carrying the setter's own
`throws`.

```swift
// Microsoft.Xna.Framework.Audio.AudioEmitter
public var Position: Microsoft.Xna.Framework.Vector3 { get set }   // both infallible
public var DopplerScale: Float { get }                             // getter infallible
public func SetDopplerScale(_ value: Float) throws                 // setter validates
```

`SetDopplerScale` is the projection of the CLR **setter accessor**, not a second
XNA member; `DopplerScale` remains one member in every scoreboard, and there is
deliberately no writable property beside the writer, because an unchecked path
would accept values XNA rejects. The `Item`/`SetItem` rule for indexed
properties is the same rule, unchanged.

Which accessors are fallible is not a judgement call. It is derived from the
CIL of the seven registered assemblies by
`tools/api_compat/accessor_fallibility.py`, pinned as
`tools/api_compat/reference/xna40-accessor-fallibility.json`, and hash-checked
by the verifier exactly as the metadata contract is — 114 fallible getters and
114 fallible setters out of 840 properties, each carrying the call chain that
reaches the throw. See `docs/xna-swift-mapping.md` and
`docs/foundation-22-accessor-projection-evidence.md`.

## Nullable reference returns

A CLR reference return and a CLR failure are independent facts. `null` is a
normal, successful result; a thrown exception is not a value at all. CNA-Swift
projects the two orthogonally, so all four combinations occur and none stands
in for another.

```swift
// Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs
public var Resource: Any? { get }                 // nullable, infallible

// Microsoft.Xna.Framework.GraphicsDeviceManager -- the XNA-faithful shape
public var GraphicsDevice: Graphics.GraphicsDevice? { get }
```

`throws` never stands in for a normal null, and Optional never absorbs a real
failure: a member that can both return null and throw is `T?` **and** `throws`.
No placeholder object, sentinel, `fatalError`, force unwrap, `try?` or
`catch { return nil }` is used to avoid either.

Which returns are nullable is not a judgement call. It is derived from the CIL
of the seven registered assemblies by
`tools/api_compat/return_nullability.py`, pinned as
`tools/api_compat/reference/xna40-reference-return-nullability.json`, and
hash-checked by the verifier exactly as the metadata contract is — 113 proven
nullable, 134 proven non-null and 122 unproven, over all 369 public
reference-typed return positions. An unproven return keeps the non-Optional
projection and is named individually rather than guessed either way. See
`docs/xna-swift-mapping.md` and
`docs/foundation-23-reference-return-nullability-evidence.md`.

## Per-milestone records

The sections from here to *Qualified runtime* were written when each type
landed and are kept as that milestone's record. Their closing "remains
deferred" lists were true on the day they were written and are **not**
maintained: `GraphicsDevice.SetRenderTarget`, `GraphicsDevice.RasterizerState`,
`RenderTarget2D` and the four state objects have all landed since, in
Foundations 38, 41, 45, 47 and 48. `plan.md` and the diagnostic block above are
the authority for what is true now.

## Foundation 14 pure managed batch

Foundation 14 is a multi-type batch rather than a single-type closure. It
completes 25 entirely missing pure-managed types carrying 144 mapped XNA
identities, and stops on the 25-type batch limit.

```text
Graphics.GraphicsProfile        Reach=0, HiDef=1
Graphics.PresentInterval        Default=0, One=1, Two=2, Immediate=3
Graphics.VertexElementFormat    Single=0 .. HalfVector4=11
Graphics.VertexElementUsage     Position=0 .. TessellateFactor=12
Graphics.CompareFunction        Always=0 .. NotEqual=7
Graphics.CubeMapFace            PositiveX=0 .. NegativeZ=5
Graphics.IndexElementSize       SixteenBits=0, ThirtyTwoBits=1
Graphics.Blend                  One=0 .. SourceAlphaSaturation=12
Graphics.BlendFunction          Add=0 .. Max=4
Graphics.ColorWriteChannels     [Flags] None=0, Red=1, Green=2, Blue=4,
                                Alpha=8, All=15
Graphics.CullMode               None=0, CullClockwiseFace=1,
                                CullCounterClockwiseFace=2
Graphics.StencilOperation       Keep=0 .. Invert=7
Graphics.TextureAddressMode     Wrap=0, Clamp=1, Mirror=2
Graphics.TextureFilter          Linear=0 .. MinPointMagLinearMipPoint=8
Graphics.ClearOptions           [Flags] Target=1, DepthBuffer=2, Stencil=4
Graphics.GraphicsDeviceStatus   Normal=0, Lost=1, NotReset=2
Graphics.PrimitiveType          TriangleList=0 .. LineStrip=3
Graphics.EffectParameterClass   Scalar=0 .. Struct=4
Graphics.EffectParameterType    Void=0 .. TextureCube=9
Graphics.SetDataOptions         [Flags] None=0, Discard=1, NoOverwrite=2
Graphics.VertexElement          value struct, 10 identities
Graphics.IEffectFog             protocol, 4 read/write properties
Graphics.IEffectMatrices        protocol, 3 read/write properties
Audio.SoundState                Playing=0, Paused=1, Stopped=2
Audio.AudioChannels             Mono=1, Stereo=2
```

`[Flags]` is an observed property of the pinned binaries: it is present on
exactly `ColorWriteChannels`, `ClearOptions`, and `SetDataOptions` and absent
from the other 19 enums, so those three are Swift `OptionSet` structs and the
rest are ordinary `enum: Int32`. No `None`, `Default`, or `All` literal was
invented, and no enum gained a `description`, `ToString`, predicate, alias, or
native-conversion helper.

`VertexElement` reproduces the pinned IL exactly: verbatim unvalidated
construction, plain field accessors, four-field `op_Equality` with `!=` as its
negation, `Equals(object)` null/type guards, the internal `SmartGetHashCode`
word XOR with `0x7FFFFFFF` for a zero result, and
`{Offset:O Format:F Usage:U UsageIndex:I}`. The two element enums gain no
public string surface; the name tables stay private to `VertexElement`.

`SoundState` and `AudioChannels` are the first `Microsoft.Xna.Framework.Audio`
types, so that namespace marker was added. They are metadata only: there is no
audio engine, and the audio backend remains NULL.

This batch is managed metadata and managed value logic only. No CNA source, C
ABI, native binding, renderer, device, texture, sprite-batch, callback,
thread-affinity, filesystem or audio-engine work is included, and none of the
five runtime-partial types was touched. See
`docs/foundation-14-pure-managed-batch-evidence.md`.

## Managed RenderTargetUsage

`Microsoft.Xna.Framework.Graphics.RenderTargetUsage` is the exact managed
non-flags `Int32` enum with `DiscardContents=0`, `PreserveContents=1`, and
`PlatformContents=2`. The three literals are mutually exclusive alternatives,
not bit flags, so the type is a Swift `enum` and never an `OptionSet`. The CLR
`value__` storage identity is the existing enum-storage language mapping. Swift
raw-value initialization accepts the complete 0...2 table, rejects
representative unknown positive and negative values with `nil`, and preserves
ordinary value-copy behavior without adding XNA members. There is no
`description`, `ToString`, predicate helper, alias, or native conversion.

This is managed metadata only. RenderTarget2D, RenderTargetCube,
RenderTargetBinding, PresentationParameters, render-target creation, depth
attachments, MSAA, `GraphicsDevice.SetRenderTarget`, and native usage mapping
remain deferred. The names describe XNA's content-preservation policy;
CNA-Swift does not implement or claim discard, preserve, or platform-defined
content semantics. See `docs/render-target-usage-evidence.md`.

## Managed DisplayMode

`Microsoft.Xna.Framework.Graphics.DisplayMode` is the exact managed descriptor
class with the pinned six identities: `ToString`, `Format`, `Height`, `Width`,
`AspectRatio`, and `TitleSafeArea`. The pinned public contract declares **no
constructor**, so the Swift class exposes no public initializer; the CLR's
`assembly`-accessible `.ctor(width, height, format)` maps to an `internal` Swift
initializer that is implementation infrastructure and never public XNA surface.
The class is deliberately neither `open` — no accessible CLR constructor makes
it externally subclassable — nor `final`, because metadata says `sealed=false`.

`Width`, `Height` and `Format` are verbatim get-only stored values with no
validation. `AspectRatio` is the guarded binary32 quotient: positive zero
whenever either dimension is zero, otherwise `Float(Width) / Float(Height)`
computed without Double widening, without clamping and without absolute value.
`TitleSafeArea` is exactly `Rectangle(0, 0, Width, Height)` — the unmodified
Windows result, with no overscan inset, no Xbox policy and no display query.
`ToString` reproduces `{Width:W Height:H Format:F AspectRatio:A}`.

There is no `Equals`, `GetHashCode`, `==`, `!=`, `Equatable`, `Hashable`,
setter, static factory, `description`, or convenience helper, and SurfaceFormat
gains no public string surface.

This is a managed descriptor only. DisplayModeCollection, GraphicsAdapter,
`GraphicsDevice.DisplayMode`, PresentationParameters, monitor enumeration,
display-mode discovery, resolution switching, fullscreen mode management, and
native display support remain deferred. See `docs/display-mode-evidence.md`.

## Managed DepthFormat

`Microsoft.Xna.Framework.Graphics.DepthFormat` is the exact managed non-flags
`Int32` enum with `None=0`, `Depth16=1`, `Depth24=2`, and
`Depth24Stencil8=3`. The CLR `value__` storage identity is the existing
enum-storage language mapping. Swift raw-value initialization accepts the
complete 0...3 table, rejects representative unknown positive and negative
values with `nil`, and preserves ordinary value-copy behavior without adding
XNA members.

This is managed metadata only. GraphicsAdapter, PresentationParameters,
RenderTarget2D, RenderTargetCube, GraphicsDeviceManager depth-format members,
DepthStencilState, depth/stencil allocation or testing, renderer capability,
and native DepthFormat mapping remain deferred. See
`docs/depth-format-evidence.md`.

## Managed SurfaceFormat

`Microsoft.Xna.Framework.Graphics.SurfaceFormat` is the exact managed
non-flags `Int32` enum with all twenty pinned literals from `Color=0` through
`HdrBlendable=19`. The CLR `value__` storage identity is the existing
enum-storage language mapping. Swift raw-value initialization accepts the
complete 0...19 table, rejects representative unknown positive and negative
values with `nil`, and preserves ordinary value-copy behavior without adding
XNA members.

This is managed metadata only. DisplayModeCollection,
GraphicsAdapter, PresentationParameters, texture and render-target APIs,
GraphicsDeviceManager format properties, GraphicsDevice format operations,
pixel/DXT conversion, HDR capability, GPU format negotiation, and native
SurfaceFormat mapping remain deferred. See
`docs/surface-format-evidence.md`.

## Managed FillMode

`Microsoft.Xna.Framework.Graphics.FillMode` is the exact managed non-flags
`Int32` enum with `Solid=0` and `WireFrame=1`. The CLR `value__` storage
identity is the existing enum-storage language mapping. Swift raw-value
initialization accepts 0 and 1, rejects representative unknown values with
`nil`, and preserves ordinary value-copy behavior without adding XNA members.

This is only a managed enum. `RasterizerState`,
`GraphicsDevice.RasterizerState`, drawing, wireframe rendering, polygon-mode
switching, and native renderer support remain deferred. See
`docs/fill-mode-evidence.md`.

## Managed BufferUsage

`Microsoft.Xna.Framework.Graphics.BufferUsage` is the exact managed `Int32`
OptionSet with `None=0` and `WriteOnly=1`. The CLR `value__` storage identity
is the existing enum-storage language mapping; Swift raw-value and OptionSet
surface adds no XNA identity. Unknown positive and negative raw patterns,
ordinary union/intersection, and value-copy semantics are qualified separately
from the two pinned literal observations.

This is only a managed flags value. VertexBuffer, IndexBuffer, dynamic buffers,
VertexDeclaration, IVertexType, SetData/GetData, and all GraphicsDevice buffer
and draw operations remain deferred. See `docs/buffer-usage-evidence.md`.

## Managed DisplayOrientation

`Microsoft.Xna.Framework.DisplayOrientation` is the exact root-framework
`Int32` OptionSet with `Default=0`, `LandscapeLeft=1`, `LandscapeRight=2`, and
`Portrait=4`. The CLR `value__` storage field and the Swift raw-value/OptionSet
surface remain formal language projections, so the type has exactly four XNA
identities and zero local diagnostics. Arbitrary raw bits are preserved.

This is only a managed flags value. GraphicsDeviceManager orientation members,
GameWindow, display detection, and platform rotation remain deferred. See
`docs/display-orientation-evidence.md`.

## Managed Curve family

Curve, CurveKey, and CurveKeyCollection are open reference classes, matching
the non-sealed CLR types. CurveKey has exact field equality/hash and the XNA
direct-branch CompareTo behavior, including `NaN/finite=+1`,
`finite/NaN=+1`, and `NaN/NaN=+1`.

CurveKeyCollection preserves XNA sorted insertion and reference identity. Its
CLR collection interfaces map without fake Microsoft collection types or
automatic Swift Collection conformance. `GetEnumerator` returns the root
support type `CNAEnumerator<CurveKey>` so mutation invalidation throws; the
read/write CLR indexer maps to throwing Item/SetItem accessors so invalid
indices cannot become Swift Array traps. Tangents, Hermite evaluation, Double
position widening, Step continuity, and Constant/Cycle/CycleOffset/Oscillate/
Linear loops reproduce the pinned XNA IL, including negative cycles.

See `docs/curve-evidence.md` for exact formulas, clone depth, collection
versioning, null policy, projection rules, and authority provenance. Earlier
managed families are documented in the other evidence files under `docs/`.

## Qualified GamePad family

`Buttons` is an exact `Int32` OptionSet with all 25 XNA flags. The six public
GamePad value types remain Swift structs; capabilities expose 26 read-only
properties and no public initializer. State constructors, physical and virtual
button queries, all-bit combination behavior, packet/connection fields,
binary32 clamps, equality, hashing, and strings follow the pinned XNA IL.

The four static GamePad methods bind only existing canonical CNA C ABI routes.
Native snapshots carry real packet numbers and per-control capabilities;
vibration returns CNA's device-accepted Boolean. The qualified HEADLESS host
has no controller, so disconnected routes are verified while positive state,
capability diversity, and physical rumble remain `HARDWARE_PENDING`. See
`docs/gamepad-evidence.md`, `docs/gamepad-native-inventory.md`, and the separate
generated native qualification report.

## Qualified runtime

The qualified host is Linux x86-64 with Swift 6.0.3
(`x86_64-unknown-linux-gnu`) and an external CNA C ABI 0.35.0 library built from
CNA `next`, measured with two renderers: HEADLESS, and OPENGLES3 (EasyGL) on a
private Weston + Xwayland display. Audio runs through SDL3 with SDL's dummy
driver. Supply the library explicitly:

```bash
export CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so
swift test
```

`CNA_NATIVE_LIBRARY` must be absolute. Without it Linux tries only an installed
`libcna_c_api.so`; there is no developer-tree fallback. The admitted ABI window
is CNA's own published consumer rule — major `0` exactly, minor `35` or later —
so every earlier generation, `0.7.0` and `0.21.0` included, is rejected by a
diagnostic that names the window, the reported version and the selected file. No native binary
ships in the source package. Managed Curve and GamePad value tests require no
native library. See `docs/native-abi.md` and
`docs/native-abi-migration-evidence.md`.

HEADLESS executes real viewport, clear, PNG decode, SpriteBatch, keyboard, and
GamePad disconnected routes but has no visible window or attached controller.
OPENGLES3 adds a real window, cube-face storage and back-buffer readback, which
the suite asserts where the renderer provides them. Positive controller
behaviour is not claimed (no controller is attached). macOS, iOS, tvOS,
visionOS, Windows, and Web/Wasm are unqualified.

One host behaviour is **measured and not corrected by this binding**: under a
fixed time step CNA issues a leading `Update` with a zero `ElapsedGameTime` --
which is XNA's own pre-loop `Update` in `Game.RunGame` -- followed by a `Draw`
that XNA's `Game.Tick` would not issue before its first step. Every later frame
reproduces XNA's sequence exactly. The whole sequence is pinned by
`NativeLifecycleTests.testHostGameTimeSequenceIsMeasuredNotAssumed`.

## Verification

Nothing here may open a window on the physical desktop (`:0`, `wayland-0`).
The HEADLESS runs use SDL's dummy drivers and no display at all:

```bash
unset DISPLAY WAYLAND_DISPLAY    # nothing to fall back to
export SDL_VIDEODRIVER=dummy SDL_AUDIO_DRIVER=dummy
export CNA_NATIVE_LIBRARY=/abs/cna-c-abi-0.35.0/lib/libcna_c_api.so
```

A windowed renderer runs only inside CNA's private display runner, which starts
a headless Weston and a rootful Xwayland on the real GPU:

```bash
cna/tools/platform/run_gpu_tests_private.sh --exec env -u WAYLAND_DISPLAY \
  SDL_VIDEODRIVER=x11 SDL_AUDIO_DRIVER=dummy \
  CNA_NATIVE_LIBRARY=/abs/cna-c-abi-0.35.0-opengles3-fx/lib/libcna_c_api.so \
  .build/x86_64-unknown-linux-gnu/debug/CNAPackageTests.xctest
```

```bash
swift build
swift build -c release
swift test
swift test -c release
# ASan is the memory-error gate: use-after-free and out-of-bounds, which it
# reports as `ERROR:`. LeakSanitizer is disabled because what it reports here
# is the toolchain's own process-lifetime allocation -- the suite that never
# starts the CNA runtime leaks twelve times what the one that starts a whole
# game does, and no frame is in Sources/. Measured in
# docs/foundation-49-viewport-scissor-evidence.md.
ASAN_OPTIONS=detect_leaks=0 swift test --sanitize=address --scratch-path build-asan
swift test --sanitize=thread --scratch-path build-tsan
swift package dump-symbol-graph
python3 tools/api_compat/verify.py --self-test
python3 tools/api_compat/verify.py --graph-self-test \
  --symbol-graph .build/x86_64-unknown-linux-gnu/symbolgraph/CNA.symbols.json
python3 tools/api_compat/verify.py \
  --symbol-graph .build/x86_64-unknown-linux-gnu/symbolgraph/CNA.symbols.json \
  --output docs/generated/api-compat-report.json \
  --inventory-output docs/generated/missing-type-inventory.md
python3 tools/api_compat/dependency_graph.py \
  --report docs/generated/api-compat-report.json \
  --output docs/generated/dependency-graph.json
python3 tools/native_abi/verify.py \
  --cna-include /path/to/cna/modules/c-api/include \
  --library "$CNA_NATIVE_LIBRARY"
python3 tools/native_abi/mutations.py \
  --cna-include /path/to/cna/modules/c-api/include \
  --library "$CNA_NATIVE_LIBRARY"
# The two mutation harnesses take an exclusive lock on .mutation-gate.lock and
# refuse to run at the same time: they both edit files under Sources/, and a
# swift test here that compiles the other one's planted defect reports a CAUGHT
# it did not earn. Run them one after the other, never in parallel.
CNA_NATIVE_LIBRARY=/path/to/libcna_c_api.so \
  python3 tools/projection_mutations/run.py
python3 tools/api_compat/bcl_authority_audit.py \
  --assembly mscorlib.dll=/path/to/net4/mscorlib.dll \
  --cross-check \
  --negative-control mscorlib.dll=/usr/lib/mono/4.5/mscorlib.dll \
  --negative-control mscorlib.dll=/usr/lib/mono/4.0/mscorlib.dll \
  --negative-control mscorlib.dll=/usr/lib/mono/2.0-api/mscorlib.dll \
  --negative-control mscorlib.dll=/path/to/another/net4/mscorlib.dll \
  --output docs/generated/bcl-authority-audit.json
python3 tools/runtime_capabilities/render.py --check
python3 tools/api_compat/profile_capabilities.py --check \
  --assembly-dir /path/to/xna/redistributable --il-cache ~/deps/xna-il-cache
python3 tools/consumer_canary/verify.py --self-test
python3 tools/consumer_canary/verify.py \
  --template ../cna-swift-template \
  --library "$CNA_NATIVE_LIBRARY" --frames 60 \
  --output docs/generated/consumer-canary-report.json
python3 tools/status_gate/verify.py --self-test
python3 tools/status_gate/verify.py \
  --symbol-graph .build/x86_64-unknown-linux-gnu/symbolgraph/CNA.symbols.json \
  --cna-include /path/to/cna/modules/c-api/include \
  --library "$CNA_NATIVE_LIBRARY" \
  --assembly-dir /path/to/xna/redistributable \
  --il-cache ~/deps/xna-il-cache \
  --bcl-dir ~/deps/bcl-4.0-pinned \
  --bcl-negative-control mscorlib.dll=/usr/lib/mono/4.5/mscorlib.dll \
  --bcl-negative-control mscorlib.dll=/usr/lib/mono/4.0-api/mscorlib.dll \
  --bcl-negative-control mscorlib.dll=/usr/lib/mono/2.0-api/mscorlib.dll \
  --bcl-negative-control mscorlib.dll=/path/to/another/net4/mscorlib.dll
python3 tools/gamepad_native/run.py \
  --library "$CNA_NATIVE_LIBRARY" \
  --output docs/generated/gamepad-native-report.json
python3 tools/api_compat/pinned_assembly_audit.py \
  --assembly-dir /path/to/xna/redistributable \
  --require-exact Microsoft.Xna.Framework.dll \
  --require-exact Microsoft.Xna.Framework.Graphics.dll \
  --output docs/generated/pinned-assembly-audit.json
python3 tools/api_compat/message_coverage.py --self-test \
  --assembly-dir /path/to/xna/redistributable --il-cache ~/deps/xna-il-cache
python3 tools/api_compat/message_coverage.py --mutations \
  --assembly-dir /path/to/xna/redistributable --il-cache ~/deps/xna-il-cache
python3 tools/api_compat/message_coverage.py \
  --assembly-dir /path/to/xna/redistributable \
  --il-cache ~/deps/xna-il-cache \
  --output docs/generated/message-coverage.json
python3 tools/api_compat/accessor_fallibility.py \
  --assembly-dir /path/to/xna/redistributable \
  --output tools/api_compat/reference/xna40-accessor-fallibility.json \
  --markdown docs/generated/accessor-fallibility-inventory.md
python3 tools/api_compat/return_nullability.py \
  --assembly-dir /path/to/xna/redistributable \
  --output tools/api_compat/reference/xna40-reference-return-nullability.json \
  --markdown docs/generated/return-nullability-inventory.md
```

`accessor_fallibility.py` regenerates the pinned per-accessor fallibility
verdicts from the same registered assemblies. It carries 34 self-tests,
including five mutations that must flip a verdict and a bound proving its one
approximation immaterial, and the verifier refuses to run if the pinned file's
digest does not match `accessorFallibilitySha256` in `mapping-rules.json`.

`return_nullability.py` regenerates the pinned per-return-position nullability
verdicts from the same registered assemblies by abstractly interpreting every
method body and resolving each field's whole construction-and-store lifecycle.
It carries 115 self-tests, including mutations that must flip a verdict and a
two-sided per-overload bound, and the verifier refuses to run if the pinned
file's digest does not match `returnNullabilitySha256` in `mapping-rules.json`.

`verify.py --graph-self-test` runs seventeen negative fixtures cut from the
Symbol Graph the compiler actually emitted: each mutates a real declaration the
way a wrong Swift signature would and must introduce a diagnostic the unmutated
graph does not already carry.

`tools/native_abi/mutations.py` is the native ABI gate's falsifiability
control. It plants fourteen realistic defects one at a time — a wrong parameter
width, a canonical type recorded as a merely compatible alias, a stale symbol, a
swapped route symbol, a swapped route type, two routes sharing one type, a wrong
Swift position width, an unsatisfiable ABI window, an omitted structure field,
transposed fields, a narrowed field, a wrong callback signature, a wrong
constant and a wrong `Keys` literal — runs the unmodified verifier, requires it
to fail every time, and proves the tree is byte-identical afterwards.

`tools/projection_mutations/run.py` is the companion gate over the projected
behavior. It plants sixty realistic defects one at a time — a
neighbouring exception class at a raise site, a message that reports the Swift
class name to a user, a mirror that moves on a write the host refused, a
disposal that leaks a native subscription, an IL-derived state default changed,
a preset's blend pair transposed, a bound-state message that reads the dynamic
type instead of the declaring one — and requires the whole test suite to fail
each time. It refuses to run without a selected `CNA_NATIVE_LIBRARY`: sixteen
of its mutations are caught only by suites that start a CNA runtime, and those
suites *skip* rather than fail when no library is selected, which would report
a coverage loss as sixteen projection defects. It also counts every mutation
site before starting, so a site that has drifted is reported in seconds rather
than arriving disguised as a survivor, and it turns SIGTERM and SIGHUP into the
same unwind Ctrl-C gets, so a killed run still restores the tree.

`pinned_assembly_audit.py` decides whether an XNA assembly may be used as a
behavior authority. It reconstructs each assembly's public metadata from
`ikdasm` output — including each field's `initonly` attribute, which decides
whether the Swift projection is a `let` or a `var` — and diffs it against the
retained contract; the seven registered assemblies reproduce all 257 types and
2,964 members exactly. It is calibrated on six assemblies-worth of subjects and
carries 80 mutation self-tests, so it cannot pass vacuously. Only assembly
hashes are retained; no Microsoft binary is in the repository or the release
archive.

`docs/frontier-research-graphics-device-state-and-vertex-declaration.md`
records what has been measured about the next two frontiers but not yet
implemented: CNA's graphics-device handle is a per-callback capability token
rather than a stable identity, `BlendFunction.Min`/`.Max` are numbered the
opposite way round in CNA and in XNA, and `VertexDeclaration`'s stride rule,
its five validation failures and their order are read out of the pinned IL. It
also records a defect it found in this repository's own accessor-fallibility
analyser.

The normal API verifier exits nonzero until the full selected profile is
complete; use `--leak-only` for the green encapsulation gate. Architecture,
mapping, ABI provenance, capabilities, evidence, and the exact continuation
handoff are retained under `docs/`, `plan.md`, and `NEXT.md`.
