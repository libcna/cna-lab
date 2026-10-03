# Foundation Milestone 17 — Reference assembly registration, and Pure Managed Batch C

Foundation 17 does two things. It **registers five additional Microsoft XNA
assemblies as authoritative reference inputs** under the existing provenance
policy, and it consumes the first seven types that registration unblocks —
**26 mapped Swift XNA identities**.

No CNA source, C ABI, native binding, renderer, device, adapter, touch panel,
XACT engine, video player, or storage work is included, and none of the five
runtime-partial types was touched.

## Part 1 — Reference assembly registration

### The problem

Every earlier milestone recorded certain candidates as "outside the currently
pinned assembly set" and deferred them on that basis alone: `IGameComponent`,
`IUpdateable`, `IDrawable`, the Touch types, `AudioStopOptions`, and others.
Two assemblies were registered; the contract's 257 types are declared across
**seven**.

### What was done

Every one of the 257 retained contract types was machine-attributed to its
declaring assembly. All seven declaring assemblies are present on the
qualification host as one Microsoft XNA Framework 4.0.0.0 redistributable set,
built 2011-09-01. Two of them re-hashed to the records retained since
Foundation 1, byte for byte.

| Contract types | Members | Assembly | SHA-256 | Registered |
|---:|---:|---|---|---|
| 120 | 1753 | `Microsoft.Xna.Framework.dll` | `38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130` | Foundation 1 |
| 99 | 854 | `Microsoft.Xna.Framework.Graphics.dll` | `560080fc39021c611ca9d076dcebed312faf6d7d1413c2dc523683ea635e9f55` | Foundation 1 |
| 17 | 161 | `Microsoft.Xna.Framework.Game.dll` | `b5dffdd8125abef2a4507ba4e1d2f11062143f0a63d48fe4f298b95ad746a1f0` | **Foundation 17** |
| 8 | 65 | `Microsoft.Xna.Framework.Input.Touch.dll` | `b0585224c18022c3661057ae79544644c10f33f1dc529678364f3d6b25151c25` | **Foundation 17** |
| 7 | 72 | `Microsoft.Xna.Framework.Xact.dll` | `a14d5364dca7cf49fb90639e87ba04d52b59a700dc9198efa5707ce8eae28f0a` | **Foundation 17** |
| 3 | 24 | `Microsoft.Xna.Framework.Video.dll` | `17538b1ca9d48a993e2cd88c96b436df08e7abb4aec5d4758eb21feb580d6e06` | **Foundation 17** |
| 3 | 35 | `Microsoft.Xna.Framework.Storage.dll` | `798f678e9ae3d9afc3bed66c30123bc9634fb923b6d200188344b618e608cbb8` | **Foundation 17** |
| **257** | **2964** | | | |

### The registration gate

Registration is a deliberate provenance step, never a side effect of selecting
a type. `tools/api_compat/pinned_assembly_audit.py` is the gate. It
disassembles each assembly with `ikdasm`, reconstructs the public type and
member shape in the retained contract's own schema, and diffs it entry by
entry: kind, sealed bit, base type, declared interfaces, and every member's
name, kind, static identity, parameter types with `ref`/`out` direction, return
type, property get/set mutability, event type, and constant value.

```text
CONTRACT_TYPES_REPRODUCED=257
CONTRACT_MEMBERS_REPRODUCED=2964
UNATTRIBUTED_CONTRACT_TYPES=0
AUDIT_SELF_TESTS=60 AUDIT_SELF_TEST_STATUS=PASS
CALIBRATION_STATUS=PASS
```

The retained contract is **exactly reproducible** from these seven specific
files and no others, with zero mismatches.

### Why the result can be trusted

The reconstruction's correctness is not asserted, it is **calibrated**. The two
assemblies whose provenance was established long before this tool existed must
reproduce their contract entries exactly; `--require-exact` makes that a hard
gate that exits nonzero. Only once those pass is the same reconstruction
trusted for the other five. That gate did real work: it failed on the first six
iterations of the tool, catching hexadecimal-versus-decimal enum literals,
unmapped `uint8` arrays, `marshal(...)` directives that captured the parameter
list, named-versus-positional generic parameters, unstripped by-ref markers,
`raise_` event accessors, and Single-constant precision — every one a way the
audit could have silently mis-read metadata.

A passing calibration is not enough on its own, because a comparison that
always reports "exact" would also pass. Sixty mutation self-tests run against
five already-matching types and require each perturbation to be detected: a
dropped method, property, field or constructor; a retyped method result,
property or field; a changed constant value; a flipped `sealed` bit; a changed
base type; a dropped declared interface; an invented extra member; a renamed
method; and a flipped static identity. Stubbing `compare_type` to return no
issues flips `AUDIT_SELF_TEST_STATUS` to `FAIL` and the process exit to
nonzero.

### One documented asymmetry

The retained contract records a **reduced** direct-interface set: an interface
already implied as a base of another listed entry is omitted —
`System.Collections.IEnumerable` behind `IEnumerable<T>`,
`IGraphicsResource` behind `IDynamicGraphicsResource`. Reproducing that
reduction would mean modelling BCL interface hierarchies, so the audit requires
the sound relation instead: **every interface the contract records must
actually be declared by the assembly**. The 43 interfaces the assemblies
declare beyond the contract are reported as
`contractInterfaceReductions` rather than being hidden or treated as
discrepancies.

No Microsoft binary or extracted proprietary source is in the repository or the
release archive. Only the SHA-256 of each registered assembly is retained.

## Part 2 — The seven consumed types

```text
1 Input.Touch.GestureType             OptionSet  11 identities
2 Input.Touch.TouchLocationState      enum        4
3 Media.VideoSoundtrackType           enum        3
4 IGraphicsDeviceManager              protocol    3
5 Audio.AudioStopOptions              enum        2
6 Input.Touch.TouchPanelCapabilities  struct      2
7 IGameComponent                      protocol    1
                                      TOTAL      26
```

`Microsoft.Xna.Framework.Input.Touch` gains its first implemented types and
therefore its namespace marker, taking `NAMESPACE_MARKERS` from 9 to 10 and
`LANGUAGE_PROJECTION_EXCLUSIONS` from 115 to 116.

### The four enums

`[Flags]` presence is read out of each binary, never assumed.

- **`GestureType`** is the only one carrying
  `System.FlagsAttribute`, so it is the only one that maps to a Swift
  `OptionSet`. Its zero literal `None` is spelled as the empty set; the other
  ten are distinct single bits from `0x001` to `0x200`, asserted to be
  pairwise disjoint powers of two summing to `0x3FF`.
- **`AudioStopOptions`** carries no `[Flags]` attribute, and that absence is
  demonstrably deliberate rather than an oversight: the pinned binary carries
  `SuppressMessageAttribute("Microsoft.Design", "CA1027:MarkEnumsWithFlags")`,
  which is precisely the analyzer suggestion to mark it as flags being
  suppressed by its author.
- **`TouchLocationState`** has four contiguous literals with `Invalid`, not
  `Released`, at zero.
- **`VideoSoundtrackType`** has three contiguous literals. `MusicAndDialog` is
  its own literal `2`, not the bitwise union of `Music` and `Dialog`, because
  the enum is not a flags enum.

### `TouchPanelCapabilities`

A sealed sequential value struct whose two members are compiler-generated auto
properties with **private** setters, so the public contract is exactly two
get-only properties: no constructor, no equality, no `ToString`. The Swift
projection keeps its initializer internal and both stored properties private,
so Swift's implicit memberwise initializer is not public either.

Its only producer in the pinned assembly is the `assembly` static `GetCaps()`,
which is `ldloca; initobj; ldloc; ret` — it returns the all-zero value. That is
recorded here as an observation about the pinned Windows build, not as
implemented behaviour: `TouchPanel` is **not** implemented, and this milestone
claims no touch capability, queries no device, and invents no capability value.

### The two protocols

A CLR interface maps to a Swift protocol with one requirement per declared
member and no invented conformance.

- **`IGameComponent`** — one requirement, `Initialize()`.
- **`IGraphicsDeviceManager`** — three requirements, `CreateDevice()`,
  `BeginDraw() -> Bool`, `EndDraw()`.

All four requirements take the established `throws` language projection,
matching the existing `Game.Initialize`, `Game.BeginDraw` and `Game.EndDraw`
signatures and `GraphicsDeviceManager.ApplyChanges`. Every one is an XNA
runtime failure path — device creation, frame begin, frame end, component
initialisation — and `throws` is a measured `LANGUAGE_MAPPING` that adds no XNA
member. Choosing it now also keeps a future real conformer possible, which a
non-throwing requirement would have foreclosed.

Declaring these protocols claims **no** device capability. Nothing in the
binding conforms to either; `GraphicsDeviceManager` remains an untouched
runtime partial and is deliberately *not* an `IGraphicsDeviceManager`, which
the projection tests assert directly.

## Deferred, with reasons

- **`Audio.RendererDetail`** was dependency-complete, pure managed, and within
  a newly registered assembly — and is still deferred. Its `GetHashCode` is
  `(IsNullOrEmpty(_name) ? 0 : _name.GetHashCode()) ^
   (IsNullOrEmpty(_id) ? 0 : _id.GetHashCode())`, and
  `System.String.GetHashCode()` is explicitly an unspecified,
  implementation-defined function that .NET documents as unstable across
  versions and architectures. It is not derivable from the pinned IL, no
  existing type in this binding hashes a string, and Swift's own `String`
  hashing is per-process seeded, so it could not even be stable within the
  binding. Semantic fidelity outranks scoreboard progress, so the type is
  recorded as blocked on a general `System.String.GetHashCode` decision rather
  than implemented with a substitute hash.
  Its `ToString()` is, by contrast, fully derivable: the IL boxes and calls
  `System.ValueType::ToString()`, which returns the fully-qualified CLR type
  name.
- **`IUpdateable`** and **`IDrawable`** are blocked on a general CLR
  event/delegate projection. Both declare
  `System.EventHandler<System.EventArgs>` events, and no event has ever been
  mapped in this binding — `EVENT_MAPPING_MISMATCH` is a measured category that
  has been zero because all 49 contract events live in missing or partial
  types.
- **`GameServiceContainer`** needs `System.Type` and `System.IServiceProvider`.
- **`LaunchParameters`** derives from
  `System.Collections.Generic.Dictionary<System.String,System.String>`.
- **`StorageDeviceNotConnectedException`** needs the general
  `System.Exception` projection.
- **`GameWindow`**, **`TouchPanel`**, **`VideoPlayer`**, **`StorageDevice`**,
  **`AudioEngine`** and the rest of the newly registered assemblies' types need
  real platform, hardware or runtime capability.

## Verifier coverage

The four enums joined the batch enum matrix and required registration in
`rawTypeChecks`; the two protocols and the struct joined the managed matrix.

One real verifier defect was found and fixed while adding them. The managed
matrix's "wrong namespace" mutation relocated a type to
`Microsoft.Xna.Framework.{simple}`, which is a **no-op** for any type that
already lives at the namespace root — the mutation silently proved nothing.
`IGameComponent` and `IGraphicsDeviceManager` are the first root-namespace
types to enter that matrix, and they exposed it by failing. The mutation now
relocates to `Microsoft.Xna.Framework.Relocated.{simple}`, which is a genuine
move for every type.

```text
API_COMPAT_SELF_TESTS  1606 -> 1822   (+216)
AUDIT_SELF_TESTS       new  -> 60
```

## Structural scoreboard, start -> end

```text
                              start    end
TARGET_TYPES                    103    110
TARGET_MEMBERS                 1594   1620   (+26)
TOTAL_DIAGNOSTICS               305    298
COMPLETE_TYPES                   98    105
PARTIAL_TYPES                     5      5
MISSING_TYPE                    154    147
MISSING_MEMBER                  131    131   (unchanged)
NAMESPACE_MARKERS                 9     10
LANGUAGE_PROJECTION_EXCLUSIONS  115    116
```

Every mapping and safety counter is unchanged. `UNEXPECTED_TYPE`,
`UNEXPECTED_MEMBER`, `TYPE_KIND_MISMATCH`, `FIELD_MAPPING_MISMATCH`,
`METHOD_SIGNATURE_MAPPING_MISMATCH`, `PARAMETER_MAPPING_MISMATCH`,
`RETURN_MAPPING_MISMATCH`, `GENERIC_MAPPING_MISMATCH`, `ENUM_VALUE_MISMATCH`,
`FLAGS_MAPPING_MISMATCH`, `EVENT_MAPPING_MISMATCH`, `OPERATOR_MAPPING_MISMATCH`,
`REF_OUT_MAPPING_MISMATCH`, `LANGUAGE_MAPPING_MISMATCH`, `INTERNAL_TYPE_LEAK`,
`RAW_HANDLE_LEAK`, `PUBLIC_NATIVE_FFI_LEAK` and
`UNMEASURED_STRUCTURAL_CATEGORY` remain **0**; `BASE_MAPPING_MISMATCH=2`,
`INTERFACE_MAPPING_MISMATCH=1`, `PROPERTY_MAPPING_MISMATCH=1` and
`OVERLOAD_MAPPING_MISMATCH=16` are the unchanged deferred profile.
`ALLOWLIST_ENTRIES=0` and `NONPUBLIC_CONSTRUCTION_PROJECTIONS=4`.

All 151 non-`MISSING_TYPE` diagnostics remain owned exclusively by the five
partials. The seven new types carry zero diagnostics between them.

## Behaviour corpus

```text
PURE_XNA_DERIVED  1493 -> 1523 observations / 1523 assertions / 0 failures
```

Five new pure XNA-derived groups: `AUDIO_STOP_OPTIONS`,
`VIDEO_SOUNDTRACK_TYPE`, `TOUCH_LOCATION_STATE`, `GESTURE_TYPE`,
`TOUCH_PANEL_CAPABILITIES`. The report carries
`foundation17PureManagedBatchContracts`, read back out of the pinned contract,
and `foundation17ReferenceInputs`, naming the declaring assembly of every
consumed type. Swift projection qualification stays in
`Foundation17ProjectionTests` and is not counted as XNA behaviour.

## Gates

```text
SWIFT_VERSION=6.0.3
DEBUG_BUILD=PASS
RELEASE_BUILD=PASS
DEBUG_TESTS=176 PASS
RELEASE_TESTS=176 PASS
WARNINGS_AS_ERRORS=PASS_DEBUG_AND_RELEASE_INCLUDING_TESTS
SYMBOL_GRAPH=PASS
API_SELF_TESTS=1822 PASS
PINNED_ASSEMBLY_AUDIT=257/2964 CALIBRATION=PASS SELF_TESTS=60 PASS
NORMAL_STRICT=EXPECTED_RED_DEFERRED_PROFILE_ONLY
LEAK_ONLY=PASS
PURE_XNA_DERIVED=1523/1523/0
NATIVE_ABI=29/91/91/18/2/214 MISSING=0/0 MISMATCHES=0
```

Foundation 17 has zero native surface, so the native ABI report is unchanged
and was re-derived against the retained ABI-0.7.0 artifact SHA-256
`c62949d23d3745964f5e557a06665875621ed4cb6e2930e3f282afd5911f2dcb`.
