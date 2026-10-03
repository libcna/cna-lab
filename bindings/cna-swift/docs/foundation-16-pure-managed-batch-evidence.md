# Foundation Milestone 16 — Pure Managed Batch B

Foundation 16 completes **four entirely missing pure-managed XNA types**
carrying **21 mapped Swift XNA identities**, all drawn from the two already
pinned, hash-matched assemblies. No CNA source, C ABI, native binding,
renderer, device, adapter, mouse device, microphone, media player, or
filesystem work is included, and none of the five runtime-partial types was
touched.

```text
1 Input.MouseState          struct   14 identities
2 Media.MediaState          enum      3
3 Media.MediaSourceType     enum      2
4 Audio.MicrophoneState     enum      2
                            TOTAL    21
```

Completing these types claims **no runtime capability**. There is no mouse
device, no cursor, no microphone, no capture, no media player, no media
library, and no video. `MouseState` is a value snapshot with no producer:
`Microsoft.Xna.Framework.Input.Mouse` is not implemented and is not in the
currently pinned assembly set.

## Reference provenance

Public shape comes from the pinned contract SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`. All four
types were independently re-derived this milestone from the pinned,
hash-matched `Microsoft.Xna.Framework.dll`, SHA-256
`38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`, which was
re-hashed on the qualification host and matched the retained record exactly.
The IL was read with `ikdasm` and machine-compared against the pinned contract.
No Microsoft binary or extracted proprietary source is in the repository or the
archive.

## New namespace

`Microsoft.Xna.Framework.Media` gains its first implemented types, so it gains
its namespace marker. A marker is a formally measured language projection, not
an allowlist entry, and adds no XNA identity.

```text
NAMESPACE_MARKERS                8 -> 9
LANGUAGE_PROJECTION_EXCLUSIONS 114 -> 115
```

## The three enums

Each is pinned as `.class public ... sealed ... extends [mscorlib]System.Enum`
with an `int32 value__` and its literals, and **none carries a `[Flags]`
attribute** in the pinned binary. `[Flags]` presence is read out of the binary,
never assumed, so all three map to Swift `enum: Int32` rather than `OptionSet`.

```text
Media.MediaState        Stopped = 0   Playing = 1   Paused = 2
Media.MediaSourceType   LocalDevice = 0             WindowsMediaConnect = 4
Audio.MicrophoneState   Started = 0   Stopped = 1
```

Two details are transcribed rather than inferred:

- `MediaSourceType`'s two literals are **not contiguous** — `0x00000000` and
  `0x00000004`, with no literal for 1, 2 or 3. No `None` or `All` literal was
  invented to close the gap, and the Swift projection reports `nil` for every
  undeclared raw value, which an `OptionSet` projection could never do.
- `MicrophoneState.Started` is the **zero** literal and `Stopped` is 1, which
  is the opposite of the ordering a reader might assume.

`MediaState`'s literals also appear out of numeric order in the metadata
(`Paused`, `Playing`, `Stopped`); each raw value is taken from its own
`int32(...)` literal rather than from declaration position.

No enum gained a `description`, `ToString`, `String`, predicate, alias, or
native-conversion member.

## `MouseState`

```text
.class public sequential ansi sealed beforefieldinit
    Microsoft.Xna.Framework.Input.MouseState
    extends [mscorlib]System.ValueType
{
  .field assembly int32       x
  .field assembly int32       y
  .field assembly ButtonState leftButton
  .field assembly ButtonState rightButton
  .field assembly ButtonState middleButton
  .field assembly ButtonState xb1
  .field assembly ButtonState xb2
  .field assembly int32       wheel
}
```

A sealed sequential value struct with eight `assembly` fields that stay
internal. The public surface is exactly fourteen identities: one constructor,
eight get-only properties, `GetHashCode`, `ToString`, `Equals(object)`,
`op_Equality`, and `op_Inequality`. There is **no** typed
`Equals(MouseState)`, no `IEquatable<MouseState>` direct interface, no mutable
property, and no static factory. Its only XNA dependency, `ButtonState`, was
already strict-complete.

### The constructor parameter order

```text
.ctor(int32 x, int32 y, int32 scrollWheel,
      ButtonState leftButton,
      ButtonState middleButton,      <-- middle precedes right
      ButtonState rightButton,
      ButtonState xButton1,
      ButtonState xButton2)
```

**`middleButton` precedes `rightButton`**, which is neither the property order
nor the order `ToString` emits. This is the single most error-prone fact about
the type, and it is invisible to the ordinary checks: both parameters have the
same mapped Swift type and the same `_` external label, so a transposition
changes neither the arity, the labels, nor the signature.

It is caught because `Microsoft.Xna.Framework.Input.MouseState.ctor` is
registered in `internalParameterOrderChecks`, which compares the internal
parameter names positionally against the CLR metadata names. The guard was
mutation-proved twice:

- **Against the real source.** Transposing `middleButton` and `rightButton` in
  `MouseState.swift`, rebuilding, and re-emitting the Symbol Graph produces
  `PARAMETER_MAPPING_MISMATCH=1` with the detail
  `expected internal parameter order ('x', 'y', 'scrollWheel', 'leftButton', 'middleButton', 'rightButton', 'xButton1', 'xButton2')`.
- **Against the verifier.** Removing the entry from
  `internalParameterOrderChecks` makes nine self-test fixtures fail at once.

The self-test asserts **every** adjacent transposition of the eight parameters,
not only the middle/right pair, plus a renamed parameter, so the rule is
positional rather than one hand-picked case.

The constructor body is eight plain field stores. It validates nothing:
`Int32.min`, `Int32.max`, and negative wheel deltas all round-trip verbatim.

### `GetHashCode`

```text
x.GetHashCode() ^ y.GetHashCode()
  ^ leftButton ^ rightButton ^ middleButton ^ xb1 ^ xb2   (each boxed)
  ^ wheel.GetHashCode()
```

`Int32.GetHashCode()` returns the value itself, and a boxed `Int32`-backed CLR
enum hashes to its underlying value, so the whole result is an ordinary `Int32`
XOR over the eight fields.

Unlike the GamePad value types, **MouseState calls no `SmartGetHashCode`
helper**, so a zero XOR is returned as `0` and is *not* substituted with
`Int32.max`. Both a fully default state and a non-trivial state that happens to
XOR to zero are asserted, so the absence of the substitution is proved rather
than assumed.

Asserted literals: `(10,20,30,P,R,P,R,P) -> 1`,
`(1024,768,120,R,R,P,R,R) -> 1913`, `(-1,-2,-3,P,P,R,R,P) -> -3`,
`(Int32.max,Int32.min,0,R,R,R,R,R) -> -1`,
`(0,0,0,R,R,R,R,R) -> 0`, `(-5,7,-3,P,P,P,P,P) -> 0`.

### `ToString`

The button list is accumulated in the pinned order **Left, Right, Middle,
XButton1, XButton2** — neither the constructor order nor the property order —
by `String.Concat(accumulator, IsNullOrEmpty(accumulator) ? "" : " ", name)`
for each button equal to `1`. An empty accumulator becomes the literal
`"None"`. The composite format is
`"{{X:{0} Y:{1} Buttons:{2} Wheel:{3}}}"`, whose doubled braces are escapes, so
exactly one brace pair is emitted.

```text
(10,20,30,P,R,P,R,P)  -> {X:10 Y:20 Buttons:Left Right XButton2 Wheel:30}
(0,0,0,R,R,R,R,R)     -> {X:0 Y:0 Buttons:None Wheel:0}
(1,2,3,P,P,P,P,P)     -> {X:1 Y:2 Buttons:Left Right Middle XButton1 XButton2 Wheel:3}
(0,0,0,R,P,P,R,R)     -> {X:0 Y:0 Buttons:Right Middle Wheel:0}
```

The last line is the constructor transposition made visible: the fifth argument
is `middleButton` and the sixth is `rightButton`, yet `Right` is emitted first.

### Equality

`op_Equality` compares all eight fields and short-circuits on the first
difference; `op_Inequality` is its negation; `Equals(object)` is
`obj is MouseState ? op_Equality(this, (MouseState)obj) : false`. Every one of
the eight fields is exercised with a one-field-differs case.

The Swift mapping adds **no** `Equatable` or `Hashable` conformance and no
typed `Equals` overload; the pinned XNA members are the entire equality
surface, and the projection tests assert the conformances are absent.

## Verifier coverage

All four types were added to the existing generic structural mutation matrices,
which are built from their own pinned reference models. The three enums also
required registration in `rawTypeChecks`; the self-test refuses a batch enum
whose raw type is unverified, which it demonstrated by failing until they were
registered.

```text
API_COMPAT_SELF_TESTS  1396 -> 1606   (+210)
```

## Structural scoreboard, start -> end

```text
                              start    end
TARGET_TYPES                     99    103
TARGET_MEMBERS                 1573   1594   (+21)
TOTAL_DIAGNOSTICS               309    305
COMPLETE_TYPES                   94     98
PARTIAL_TYPES                     5      5
MISSING_TYPE                    158    154
MISSING_MEMBER                  131    131   (unchanged)
NAMESPACE_MARKERS                 8      9
LANGUAGE_PROJECTION_EXCLUSIONS  114    115
```

Every mapping and safety counter is unchanged: `UNEXPECTED_TYPE`,
`UNEXPECTED_MEMBER`, `TYPE_KIND_MISMATCH`, `FIELD_MAPPING_MISMATCH`,
`METHOD_SIGNATURE_MAPPING_MISMATCH`, `PARAMETER_MAPPING_MISMATCH`,
`RETURN_MAPPING_MISMATCH`, `GENERIC_MAPPING_MISMATCH`, `ENUM_VALUE_MISMATCH`,
`FLAGS_MAPPING_MISMATCH`, `EVENT_MAPPING_MISMATCH`, `OPERATOR_MAPPING_MISMATCH`,
`REF_OUT_MAPPING_MISMATCH`, `LANGUAGE_MAPPING_MISMATCH`, `INTERNAL_TYPE_LEAK`,
`RAW_HANDLE_LEAK`, `PUBLIC_NATIVE_FFI_LEAK`, and
`UNMEASURED_STRUCTURAL_CATEGORY` all remain **0**; `BASE_MAPPING_MISMATCH=2`,
`INTERFACE_MAPPING_MISMATCH=1`, `PROPERTY_MAPPING_MISMATCH=1` and
`OVERLOAD_MAPPING_MISMATCH=16` are the unchanged deferred profile.
`ALLOWLIST_ENTRIES=0` and `NONPUBLIC_CONSTRUCTION_PROJECTIONS=4`.

All 151 non-`MISSING_TYPE` diagnostics remain owned exclusively by the five
partials. The four batch types carry zero diagnostics between them.

## Behaviour corpus

```text
PURE_XNA_DERIVED  1443 -> 1493 observations / 1493 assertions / 0 failures
```

Seven new pure XNA-derived groups: `MOUSE_STATE_CONSTRUCTION`,
`MOUSE_STATE_EQUALITY`, `MOUSE_STATE_HASH`, `MOUSE_STATE_TO_STRING`,
`MEDIA_STATE`, `MEDIA_SOURCE_TYPE`, `MICROPHONE_STATE`. The report now carries
`foundation16PureManagedBatchContracts` and `mouseStateContract` alongside the
Foundation-14 tables; each is read back out of the pinned contract rather than
transcribed, so it cannot drift from the reference. Swift projection
qualification stays in `Foundation16ProjectionTests` and is not counted as XNA
behaviour.

## Gates

```text
SWIFT_VERSION=6.0.3
DEBUG_BUILD=PASS
RELEASE_BUILD=PASS
DEBUG_TESTS=168 PASS
RELEASE_TESTS=168 PASS
WARNINGS_AS_ERRORS=PASS_DEBUG_AND_RELEASE_INCLUDING_TESTS
SYMBOL_GRAPH=PASS
API_SELF_TESTS=1606 PASS
NORMAL_STRICT=EXPECTED_RED_DEFERRED_PROFILE_ONLY
LEAK_ONLY=PASS
PURE_XNA_DERIVED=1493/1493/0
NATIVE_ABI=29/91/91/18/2/214 MISSING=0/0 MISMATCHES=0
```

Foundation 16 has zero native surface, so the native ABI report is unchanged
and was re-derived against the retained ABI-0.7.0 artifact SHA-256
`c62949d23d3745964f5e557a06665875621ed4cb6e2930e3f282afd5911f2dcb`.
