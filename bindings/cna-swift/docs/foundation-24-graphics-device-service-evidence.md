# Foundation 24 — the type the nullability rule unblocked

Foundation 23 decided how a nullable CLR reference return projects into Swift.
This milestone spends that decision: it completes the one XNA type whose only
remaining undecided question was exactly that.

## Selection

Every one of the 134 still-missing types was re-checked mechanically against
the decided mapping table. A type is consumable when three things hold: no
undecided BCL type appears in its public signature, base or interfaces; no XNA
type it names is still missing; and its behaviour is reproducible without a
runtime, hardware, or a value the CLR leaves unspecified.

Fourteen types cleared the first two tests:

| Type | Third test |
|---|---|
| `Graphics.IGraphicsDeviceService` | **passes** — an interface has no behaviour to reproduce |
| `Audio.AudioCategory`, `Audio.Cue`, `Audio.SoundEffectInstance` | drive the XACT engine |
| `Audio.RendererDetail` | `GetHashCode` is `_name.GetHashCode() ^ _id.GetHashCode()`, unspecified by the CLR |
| `Media.VideoPlayer`, `Media.MediaSource` | real media devices |
| `FrameworkDispatcher` | pumps audio, media and networking |
| `TitleContainer` | opens real title storage |
| `Input.Mouse`, `Input.Touch.TouchPanel` | real hardware |
| `Graphics.EffectAnnotation` | every accessor reads `ID3DXBaseEffect` |
| `Graphics.GraphicsResource` | see below |
| `GameComponent` | see below |
| `GameWindow` | see below |

`IGraphicsDeviceService` is also the only still-missing *interface* with no
missing dependency: `IEffectLights` needs `DirectionalLight` and `IVertexType`
needs `VertexDeclaration`, both still absent.

### `GraphicsResource` — runtime, not managed

Its four reference returns all became decided this milestone (`Name`, `Tag`,
`GraphicsDevice` and even `ToString` are proven nullable, and every accessor is
infallible), but the IL is unambiguously runtime-owned:

```text
get_Name / ToString:
  ldfld  uint64 GraphicsResource::_internalHandle
  beq    <local path>
  ldfld  GraphicsDevice GraphicsResource::_parent
  call   GraphicsDevice::get_Resources()        -> DeviceResourceManager
  call   DeviceResourceManager::GetCachedName(uint64)
Dispose(bool):
  call   GraphicsResource::'~GraphicsResource'()   // C++/CLI destructor
  call   GraphicsResource::'!GraphicsResource'()   // C++/CLI finalizer
```

`Name` and `Tag` live in a device-side resource-name cache whenever the
resource has a native handle, and disposal runs a mixed-mode destructor. It is
also the base of `SpriteBatch` and (through `Texture`) of `Texture2D`, both
protected runtime partials, so declaring it would pull their bases and
lifecycles with it. Deferred, with the reason recorded rather than the type
quietly skipped.

### `GameComponent` — pure managed, blocked one call deep

Its own IL is entirely managed: the constructor stores `enabled = true` and the
game reference, `Initialize` and `Update` are empty virtuals, `Enabled` and
`UpdateOrder` are fields that raise their change events only on an actual
change, `Game` is a bare `ldfld`, and the three events are the standard
interlocked combine/remove pairs. But `Dispose(bool)` is not:

```text
Dispose(bool):
  call     GameComponent::get_Game()
  brfalse  <skip>
  callvirt Game::get_Components()          -> GameComponentCollection
  callvirt Collection`1<IGameComponent>::Remove(!0)
```

A disposed component unregisters itself from its game's component collection,
and `GameComponentCollection` derives from `System.Collections.ObjectModel.
Collection<IGameComponent>` — the undecided BCL base that is still blocked on
the absence of an XNA-era Microsoft `mscorlib`. Implementing `Dispose(bool)`
without that removal would be a fabricated lifecycle, so `GameComponent` waits
for the collection decision rather than shipping a component that leaks itself
into a collection it cannot leave.

### `GameWindow` — abstract, and abstract is undecided

Nine of its public members are `abstract` with no body: `Handle`,
`ClientBounds`, `AllowUserResizing`, `ScreenDeviceName`, `CurrentOrientation`,
`BeginScreenDeviceChange`, the three-argument `EndScreenDeviceChange`,
`SetTitle` and `SetSupportedOrientations`. Swift has no abstract member, so
projecting a CLR abstract *class* member is a public API decision this project
has not made — and every one of these members is a real window-system
operation regardless. Two independent deferrals, both recorded.

## What was completed

```swift
// Microsoft.Xna.Framework.Graphics
public protocol IGraphicsDeviceService {
    var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? { get }
    var DeviceCreated: CNAEvent<CNAEventArgs> { get }
    var DeviceDisposing: CNAEvent<CNAEventArgs> { get }
    var DeviceReset: CNAEvent<CNAEventArgs> { get }
    var DeviceResetting: CNAEvent<CNAEventArgs> { get }
}
```

Five mapped XNA identities: one get-only property and four events.

### Why the property is `T?` and not `T`, and not `throws`

An abstract accessor has no body, so both its fallibility and its return
nullability are the ones a caller can actually be handed. This interface has
exactly one registered implementor:

```text
GraphicsDeviceManager::get_GraphicsDevice
  IL_0000:  ldarg.0
  IL_0001:  ldfld  GraphicsDevice GraphicsDeviceManager::device
  IL_0006:  ret
```

No branch, no call, no `throw`: **infallible**. And `device` is never assigned
by `GraphicsDeviceManager..ctor`, and is explicitly `ldnull`-stored by both
`Dispose` (IL_00b0) and `CreateDevice` (IL_0014): **nullable**. XNA's own
`IGraphicsDeviceManager.BeginDraw` and `.EndDraw` guard the field with
`brfalse` and return quietly rather than treating null as an error.

A service that has not created its device yet answers `nil`. That is a normal
result, so the requirement is Optional and carries no `throws` — which is
precisely the projection Foundation 23 made expressible and which the pinned
inventory records as `PROVEN_NULLABLE_SUCCESS` / `IL_ABSTRACT_DECLARATION`,
naming `GraphicsDeviceManager::get_GraphicsDevice/0` as the implementor it was
decided from.

### The four events

All four are `System.EventHandler<System.EventArgs>` in the pinned metadata,
declared with `add_`/`remove_` accessor pairs and no `raise_` accessor at all.
Each maps to one get-only `CNAEvent<CNAEventArgs>` property keeping its XNA
name, under the Foundation 19 event rule. A conformer owns a private
`CNAEventSource` and publishes only its consumer view, which is what the CLR
encoding means: an outside caller can add and remove handlers and cannot raise.

## What declaring it does not do

Nothing conforms to it. `GraphicsDeviceManager` is an untouched runtime partial
whose own reader is still `get throws` and non-Optional, and no
`IGraphicsDeviceService` requirement can be witnessed by a throwing getter, so
its `INTERFACE_MAPPING_MISMATCH` is unchanged — and would have been reported
whether or not the protocol existed, because the verifier expects every pinned
direct interface of a mapped type regardless of whether it is implemented. A
test asserts the non-conformance explicitly, so it is a recorded fact rather
than an oversight.

Declaring the protocol claims no device capability and adds no CNA ABI symbol.

## Scoreboard

```text
                       before   after
COMPLETE_TYPES            118     119
MISSING_TYPE              134     133
TARGET_TYPES              123     124
TARGET_MEMBERS           1690    1695
TOTAL_DIAGNOSTICS         288     287
MEASURED_RETURN_NULLABILITY_PROJECTIONS
                           59      60
PURE_XNA_DERIVED         1766    1768
DEBUG_TESTS               246     252
```

`EXPECTED_SWIFT_MEMBERS` is unchanged at 2887, `UNMEASURED_STRUCTURAL_CATEGORY`
and `ALLOWLIST_ENTRIES` stay 0, and no other diagnostic moved.

## Evidence that the shape is the compiler's, not the test's

- Two `PURE_XNA_DERIVED` observations: a service before device creation
  answers nil through a reader that needs no `try`, and the four events raise
  in order through a conformer that publishes only consumer views.
- Four projection tests: a key path can only be written when the requirement's
  type is exactly the Optional class and only formed at all when the reader
  does not throw; the existential read needs no `try`; the four event
  properties are get-only with stable identity and distinct from one another;
  and `GraphicsDeviceManager` does not conform.
- The isolated external consumer now conforms to the protocol from outside the
  package, reads `nil` with no `try`, raises all four events in order, and
  forms the same key path — compiled and run in both debug and release.
