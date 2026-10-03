# Foundation 29 — the collection frontier, re-audited

With `Collection<T>` and `ReadOnlyCollection<T>` decided, every type that was
blocked on a BCL collection base was re-audited against its own IL. One is now
genuinely safe and is implemented. The rest are not, and each is recorded with
the exact reason rather than left as a gap.

## Implemented — `Media.VisualizationData`

Its whole IL is managed and reaches no native surface:

```text
.ctor()            frequencies = new float[256]
                   samples     = new float[256]
                   frequenciesCollection = new ReadOnlyCollection<float>(frequencies)
                   samplesCollection     = new ReadOnlyCollection<float>(samples)
get_Frequencies    ldarg.0; ldfld frequenciesCollection; ret
get_Samples        ldarg.0; ldfld samplesCollection; ret
```

Both getters are bare field reads — no branch, no call, no throw — so both are
infallible, and the pinned inventory proves both `PROVEN_NONNULL_SUCCESS`. They
are non-Optional and non-throwing:

```swift
public var Frequencies: CNAReadOnlyCollection<Float> { get }
public var Samples: CNAReadOnlyCollection<Float> { get }
```

The two collections wrap the arrays **live**, which is exactly why the
`ReadOnlyCollection<T>` decision unblocked this type and why a Swift `Array`
projection would not have: the published collection object never changes, and a
write into the storage is visible through it. The backing store is the
reference-typed `CNAList<Float>` for that reason, wrapped once in the
initializer as the IL does.

### Its producer is deliberately not fabricated

XNA fills the arrays from `MediaPlayer.GetVisualizationData(VisualizationData)`
— a media-runtime call on a still-missing type. That producer is **not**
invented here. A freshly constructed instance reads 256 zeros from each
collection, which is precisely what XNA's own constructor produces before any
producer has run. The write path exists as an `internal` method so the future
producer has somewhere to land; a negative compile fixture proves an external
consumer cannot reach it.

This is the same shape as `GameComponentCollectionEventArgs`, which was
implemented while *its* producer was still missing: the type's own contract is
complete and honest, and nothing about it pretends the runtime exists.

## Re-audited and still deferred

| Type | Collection member | Why it is still blocked |
|---|---|---|
| `Graphics.GraphicsAdapter` | `Adapters: ReadOnlyCollection<GraphicsAdapter>` | enumerating display adapters is a hardware query, and the type additionally names a still-missing type and a partial |
| `Audio.Microphone` | `All: ReadOnlyCollection<Microphone>` | enumerating capture devices is a hardware query |
| `Graphics.SpriteFont` | `Characters: ReadOnlyCollection<Char>` | the character set comes from loaded content, and the type reaches a partial |
| `Audio.RendererDetail` | reached via `AudioEngine.RendererDetails` | unchanged from Foundation 25: its `GetHashCode` is `_name.GetHashCode() ^ _id.GetHashCode()`, a value the CLR leaves unspecified |

In every one of these the BCL base is no longer the blocker — the runtime, the
hardware or an unspecified value is. Per the standing rule, a type that reduces
only to a native/hardware/runtime blocker stays deferred and no producer is
fabricated.

### The four `ReadOnlyCollection<T>` subclasses stay blocked on graphics

`ModelBoneCollection`, `ModelMeshCollection`, `ModelMeshPartCollection` and
`ModelEffectCollection` each derive from `ReadOnlyCollection<T>` and are
reported as `PENDING_BCL_BASE_TYPES=4`. Their bases are decided; their **element
types** are not, and the cluster is mutually recursive:

```text
ModelBoneCollection      -> ModelBone      -> ModelBoneCollection, Matrix
ModelMeshCollection      -> ModelMesh      -> ModelBone, ModelEffectCollection,
                                              ModelMeshPartCollection
ModelMeshPartCollection  -> ModelMeshPart  -> Effect, IndexBuffer, VertexBuffer
ModelEffectCollection    -> Effect         -> GraphicsResource, GraphicsDevice,
                                              EffectParameterCollection, …
```

Every path bottoms out at `Effect`/`GraphicsResource`/`GraphicsDevice`, which
are device-side graphics resources. Each of the four also declares a nested
public `Enumerator` struct that is a separate missing type. This is a graphics
frontier, not a BCL one.

## `Game.Components` — audited, and the branch is stopped

The limited authorization to implement this one member was taken up and the IL
was audited. **The member itself is trivially safe; its reachable contract is
not**, and implementing it alone would produce a working-looking API that
silently does nothing.

`Game::get_Components` is a bare field read:

```text
IL_0000: ldarg.0
IL_0001: ldfld  class GameComponentCollection Game::gameComponents
IL_0006: ret
```

and the field is assigned exactly once, in `Game..ctor` at `IL_009b`:

```text
IL_009b: newobj  GameComponentCollection::.ctor()
IL_00a0: stfld   Game::gameComponents
IL_00a5..IL_00d3:
         gameComponents.add_ComponentAdded(new EventHandler(this.GameComponentAdded))
         gameComponents.add_ComponentRemoved(new EventHandler(this.GameComponentRemoved))
```

Those two subscriptions are the whole point of the property. `GameComponentAdded`
does real work on every insert:

1. if `inRun`, call `component.Initialize()` immediately; otherwise append to
   `notYetInitialized`;
2. if the component is `IUpdateable`, `BinarySearch` `updateableComponents` with
   `UpdateOrderComparer.Default`, insert at the sorted position after walking
   past equal `UpdateOrder` values, and subscribe `UpdateableUpdateOrderChanged`;
3. if it is `IDrawable`, the same for `drawableComponents` and
   `DrawableDrawOrderChanged`.

`GameComponentRemoved` reverses all three. And `updateableComponents` /
`drawableComponents` are exactly what the **base bodies** of `Game.Update` and
`Game.Draw` iterate:

```text
Game::Update(gameTime)
    currentlyUpdatingComponents.AddRange(updateableComponents)   // snapshot
    foreach c in currentlyUpdatingComponents
        if (c.Enabled) c.Update(gameTime)

Game::Initialize()
    HookDeviceEvents()
    while (notYetInitialized.Count != 0)
        notYetInitialized[0].Initialize(); notYetInitialized.RemoveAt(0)
    if (graphicsDeviceService?.GraphicsDevice != null) LoadContent()
```

In CNA-Swift, `Game.Initialize`, `Game.Update` and `Game.Draw` are already
implemented, as `open func … throws {}` — **empty** bodies, dispatched from the
native host's lifecycle callbacks. So:

- adding `Components` alone would let a consumer write
  `game.Components.Add(component)` and get **no** `Initialize`, **no** per-frame
  `Update`, **no** `Draw`, and no error. That is a silent behavioural lie, and
  exactly the class of fabrication the Foundation 25 audit rejected 28 members
  for;
- making it honest requires changing the base bodies of three
  **already-implemented public members**, which is outside the granted
  authorization ("do not open unrelated Game runtime members") and is itself a
  new public design decision: `super.Update(gameTime)` would change meaning from
  "no-op" to "dispatch components" for every existing CNA subclass;
- `Game.Initialize`'s base body also ends by calling `LoadContent()`, which
  CNA's native host already drives through its own `load_content` callback.
  Transcribing it would double-call `LoadContent` unless the native contract is
  renegotiated.

`inRun` itself is *not* the blocker — CNA does wire `begin_run` and `end_run`
callbacks, so the flag is observable. The blocker is the three empty base
bodies and the native host's ownership of the loop.

The branch is therefore stopped and reported, as the authorization required.
`GameComponent` follows it: its own IL is pure managed, but `Dispose(bool)`
unregisters from `Game.Components`, so it inherits this blocker unchanged. No
part of either was implemented.

## Scoreboard

```text
                              before   after
TARGET_TYPES                     125     126
TARGET_MEMBERS                  1703    1706
COMPLETE_TYPES                   120     121
MISSING_TYPE                     132     131
TOTAL_DIAGNOSTICS                285     284
UNMEASURED_STRUCTURAL_CATEGORY     0       0
DEPENDENCY_COMPLETE_MISSING_TYPES 32      31

PURE_XNA_DERIVED  1835 -> 1861 observations, 0 failures
XCTEST             309 -> 318  tests, 0 failures
```

`REFERENCE_TYPES=257`, `REFERENCE_MEMBERS=2964` and `EXPECTED_SWIFT_MEMBERS=2887`
are unchanged, and no CNA ABI symbol was added.
