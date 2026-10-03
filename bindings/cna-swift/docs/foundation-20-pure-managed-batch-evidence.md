# Foundation 20 — pure managed batch E

Foundation 20 consumes the safe pure-managed types the regenerated frontier
surfaced after the event milestone, and records — with a machine survey rather
than an assertion — the general rule that blocks the rest of the same cluster.

## Types completed

Four types carrying 28 mapped Swift XNA identities:

| Type | Assembly | Identities |
|---|---|---|
| `Microsoft.Xna.Framework.Audio.AudioListener` | Microsoft.Xna.Framework.dll | 5 |
| `Microsoft.Xna.Framework.Input.Touch.TouchCollection` | Input.Touch.dll | 15 |
| `Microsoft.Xna.Framework.Input.Touch.TouchCollection.Enumerator` | Input.Touch.dll | 3 |
| `Microsoft.Xna.Framework.Media.Video` | Microsoft.Xna.Framework.Video.dll | 5 |

### `AudioListener` is data, not audio

The Foundation 18 handoff grouped `AudioListener` with the XACT types. The IL
does not support that: it is a plain class over four `Vector3` properties with a
public parameterless constructor, and it depends on nothing but `Vector3`. The
engine appears only in `Cue.Apply3D`, which *consumes* a listener and is not
implemented. Constructing one claims no audio capability.

What the IL *does* show is worth transcribing exactly. The instance holds an
`XACT_LISTENER_DATA` structure in X3DAudio's left-handed space, and every
accessor passes through `UnsafeNativeStructures.FlipHandedness`, which is
`Vector3(v.X, v.Y, -v.Z)`. That is a bitwise involution, so any written value
round-trips unchanged — negative zero and NaN payloads included — and the flip
is invisible for round-trips.

It is **not** invisible in the defaults. The constructor seeds `_Position` and
`_Velocity` with `Vector3.Zero` *without* flipping, while the getters flip on
the way out:

```text
AudioListener().Position.Z  ->  -0.0   (bit pattern 0x80000000)
AudioListener().Velocity.Z  ->  -0.0
AudioListener().Forward     ->  (0, 0, -1)   seeded through the flip
AudioListener().Up          ->  (0, 1, +0.0) seeded through the flip
```

A "just store the value" projection would have produced `+0.0` for the first
two and been quietly wrong. The Swift projection therefore mirrors the CLR
storage rather than simplifying it, and the tests assert bit patterns.

### `TouchCollection` is a value collection with no producer

Pinned as a sequential value struct implementing `IList<TouchLocation>`, with
eight *inline* `TouchLocation` slots, a `locationCount` and an `isConnected`
flag. Eight is a hard capacity: the constructor rejects a longer array, and the
private `AddTouchLocation` helper silently returns past slot seven.

The type also carries a private **static** `prevLocations` field and an
`assembly` `Update` entry point that drive the touch-panel frame state machine.
A machine scan of every public member confirms that **none** of them reads
either — which is what makes the type purely managed rather than
platform-coupled. Completing it claims no touch capability: `TouchPanel` is not
implemented, so nothing produces a live collection.

Behaviour transcribed from the IL, all of it measured by tests:

- the constructor rebuilds each entry through the seven-field `TouchLocation`
  constructor instead of storing the supplied value, carrying a previous
  location across when present and resetting it to `Invalid`/+0/+0 when absent;
- `get_Item` validates `index < 0 || index >= Count`;
- `FindById` writes `default(TouchLocation)` on a miss, so its `Id` is **0** —
  not the `-1` that `TouchLocation.TryGetPreviousLocation` writes for an absent
  previous location. Two out-parameter "absent" conventions in one namespace;
- `IndexOf` compares with `op_Equality`, the strict comparison that includes
  both states, not the typed `Equals` that ignores them;
- `CopyTo` validates the destination length in 64-bit so `arrayIndex + Count`
  cannot overflow, and reports a too-short destination on `"arrayIndex"`;
- `Add`, `Clear`, `Insert`, `RemoveAt`, `Remove` and `set_Item` are each exactly
  `throw new NotSupportedException()` — no message, no validation, so an
  out-of-range index does not change which failure is reported;
- the nested `Enumerator` holds the collection **by value** and starts at −1;
  `Current` forwards straight to the indexer, so reading it before the first
  `MoveNext` or after the last throws rather than returning a default;
  `MoveNext` clamps the cursor to `Count`; `Dispose` is a single `ret`.

### `Media.Video` is a descriptor with no producer

Pinned as a public **sealed** class whose only constructor is `assembly`, so it
maps to a `final class` with no public initializer -- the
`DisplayModeCollection` precedent again. The five public identities each return
a stored field verbatim.

One derived detail is worth keeping: the pinned constructor takes the duration
as an `int32` and builds the stored `TimeSpan` with
`TimeSpan(0, 0, 0, 0, duration)`, the days/hours/minutes/seconds/**milliseconds**
overload. The argument is whole milliseconds, and the internal initializer
preserves that conversion rather than asking a caller for a `Duration`.

The IL also declares `assembly` `GraphicsDevice` and `Filename` accessors.
Neither is public contract, and their only consumer is `VideoPlayer`, which is
not implemented, so neither is carried -- nothing could read them. Completing
the type claims no video capability: there is no public constructor, and XNA's
only producer is `ContentManager`. Nothing opens a file, decodes a frame, or
invents a duration.

## Verifier

`IList<T>` is now a measured direct interface rather than a silently
language-mapped one, with its own required-member set: the seven inherited
`ICollection<T>` members plus `IndexOf`, `Insert`, `RemoveAt` and the indexed
`Item`.

The `CopyTo` caller-owned-destination rule previously keyed on `ICollection<T>`
alone. `TouchCollection`'s pinned direct interface is `IList<T>`, which inherits
`ICollection<T>`, so the rule would have silently projected its destination
array as a value copy and discarded every written element. The rule now accepts
either interface. `ARRAY_MUTATION_MAPPINGS` rises 19 → 20.

`CNAError` gains `notSupported`, the projection of
`System.NotSupportedException`, alongside the existing argument, range, index,
null-reference and collection-modified cases.

Self-tests rose 2,119 → 2,126, and the new rule was confirmed to bite: reverting
`COLLECTION_COPY_INTERFACES` to `ICollection<T>` alone fails three of them.

## The frontier this batch stops on

`AudioEmitter` sits beside `AudioListener` — same file in the IL, same
`FlipHandedness` accessors, one extra property — and is **not** implemented.
Its `DopplerScale` setter validates:

```text
IL_0000:  ldarg.1
IL_0001:  ldc.r4     0.0
IL_0006:  bge.un.s   IL_0018      // >= 0 *or unordered*: NaN is accepted
IL_0008:  ldstr      "value"
...       newobj     ArgumentOutOfRangeException; throw
```

Swift has no throwing property setter. The project has already met this exact
problem and has **not** resolved it: `GraphicsDevice.Viewport` is projected
get-only, and the resulting `PROPERTY_MAPPING_MISMATCH` is one of the four
deferred partial-owned diagnostics carried since Foundation 14. Repository
policy decides the *indexed* case only — a read/write CLR indexer becomes a
throwing `Item` plus a throwing `SetItem` — and says nothing about a plain
property.

This is not a one-off. A machine scan of every public instance setter in the
registered `Microsoft.Xna.Framework.dll`, `Graphics.dll` and `Game.dll` finds
**35 of 278 can throw**. Three are the indexers the existing rule already
covers; the other **32 are plain properties**:

```text
 6  GraphicsDevice        BlendState, DepthStencilState, RasterizerState,
                          Viewport, Indices, ScissorRectangle
 5  SoundEffect           Name, MasterVolume, SpeedOfSound, DopplerScale,
                          DistanceScale
 4  SoundEffectInstance   Volume, Pitch, Pan, IsLooped
 3  Game                  InactiveSleepTime, TargetElapsedTime, Content
 2  GraphicsDeviceManager PreferredBackBufferWidth, PreferredBackBufferHeight
 1  each                  AudioEmitter.DopplerScale, Microphone.BufferDuration,
                          ContentManager.RootDirectory, SpriteFont.DefaultCharacter,
                          GameWindow.Title, GraphicsDeviceInformation.Adapter,
                          Effect.CurrentTechnique, SkinnedEffect.WeightsPerVertex,
                          DynamicSoundEffectInstance.IsLooped,
                          ContentSerializerAttribute.CollectionItemName,
                          DecompressStream.Position, ImageStream.Position
```

Eleven of those live on the protected runtime partials, whose eventual native
implementations must follow whatever is chosen. Deciding it inside a
pure-managed batch, for one property on one type, would be the wrong place.

## Scoreboard

```text
                        before   after
COMPLETE_TYPES             113     117
PARTIAL_TYPES                5       5
MISSING_TYPE               139     135
TARGET_TYPES               118     122
TARGET_MEMBERS            1656    1684
TOTAL_DIAGNOSTICS          290     286
MISSING_MEMBER             131     131
ARRAY_MUTATION_MAPPINGS     19      20
NONPUBLIC_CONSTRUCTION       7       8
API_COMPAT_SELF_TESTS     2119    2126
DEBUG_TESTS                218     231
BEHAVIOR_ASSERTIONS       1628    1717
```

Every mapping/leak category is unchanged: `BASE_MAPPING_MISMATCH=2`,
`INTERFACE_MAPPING_MISMATCH=1`, `PROPERTY_MAPPING_MISMATCH=1`,
`OVERLOAD_MAPPING_MISMATCH=16`, everything else zero, `ALLOWLIST_ENTRIES=0`,
`UNMEASURED_STRUCTURAL_CATEGORY=0`.

`MISSING_MEMBER` holds at 131 and the ABI is unchanged at 29/91/91/18/2/214.

## A masked warning, and the gate that caught it

An incremental `swift build -Xswiftc -warnings-as-errors --build-tests` reported
success while a real diagnostic sat in an unchanged-looking test file: SwiftPM
reused artifacts built without the flag. Re-running after touching every source
surfaced it (`'1e-45' underflows and loses precision during conversion to
'Float'`), and it is fixed. The warnings-as-errors gate is only meaningful after
forcing a full rebuild, and both configurations are now green that way. Release
additionally needs `-Xswiftc -enable-testing` to build the test target at all.

## The frontier is now exhausted for safe managed work

After this batch the dependency graph reports 32 dependency-complete missing
types, and every one of them is blocked on something outside a managed batch:

| Blocker | Types |
|---|---|
| Undecided BCL base | 13 — 8 exception types, 5 `ContentSerializer*Attribute`, plus `MathTypeConverter`, `GameComponentCollection`, `LaunchParameters` |
| Undecided BCL member type | `VisualizationData` and `SpriteFont` (`ReadOnlyCollection<T>`), `GameServiceContainer` (`System.Type`) |
| Undecided throwing property writer | `AudioEmitter`, `GameWindow`, `ContentManager`, `SpriteFont` |
| Real hardware or device enumeration | `GraphicsAdapter`, `Input.Mouse`, `TouchPanel`, `Microphone` |
| Real runtime subsystem | `EffectAnnotation`, `ContentManager`, `TitleContainer`, `FrameworkDispatcher`, `AudioCategory`, `MediaSource` |
| Unspecified BCL hash | `RendererDetail` (`System.String.GetHashCode`) |
| Protected runtime partial | `SpriteFont` (`Texture2D`) |

`GameComponent` and `DrawableGameComponent` remain outside that list entirely:
they depend on the `Game` and `GraphicsDevice` partials, so they are not
dependency-complete and were not forced.
