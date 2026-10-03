# Foundation 30 — a CLR exception class projects to a real Swift `Error` class

Eight XNA types are exception classes and every one of them declares **nothing
but constructors**. Their whole observable behaviour is inherited, so until
`System.Exception` had a decided projection none of them could be implemented
without dropping a base — the failure the Foundation 27 rule exists to prevent.
This milestone admits the exception families into the BCL authority, decides
the Swift projection, and completes six of the eight types.

## The rule

> A CLR exception class projects to a real Swift **class** conforming to Swift
> `Error`, and the CLR inheritance chain survives as a Swift inheritance chain
> at full length.

```text
System.Exception                                 ->  CNAException : Error
System.SystemException                           ->  CNASystemException
System.Runtime.InteropServices.ExternalException ->  CNAExternalException
```

`Error` is the point. A CLR `Exception` object is a thing you throw, so its
projection must be a thing Swift can `throw`, and `catch is DeviceLostException`
must work. A struct or an enum could not carry the chain; an alias to `CNAError`
would merge two channels the architecture keeps apart; an `NSObject` would be
convenient and wrong.

### The chain is three links, not two

`ExternalException`'s exact direct base in the admitted `mscorlib` is
`System.SystemException`, and that middle link is **not** a marker:

```text
SystemException::.ctor()        base..ctor(GetResourceString("Arg_SystemException"))
                                SetErrorCode(0x80131501)
SystemException::.ctor(string)  base..ctor(message); SetErrorCode(0x80131501)
ExternalException::.ctor()      base..ctor(GetResourceString("Arg_ExternalException"))
                                SetErrorCode(0x80004005)
```

Collapsing `CNAExternalException` straight onto `CNAException` would change both
the default `Message` and the `HResult` of `InstancePlayLimitException`,
`NoAudioHardwareException` and `StorageDeviceNotConnectedException`. The
verifier measures each link separately and a mutation for the collapse is one of
the twenty-one new BCL self-tests.

## `CNAError` stays a separate channel

`CNAError` is not in this hierarchy and must not be:

| | what it reports |
|---|---|
| `CNAException` and its subclasses | projected CLR/XNA exception objects |
| `CNAError` | CNA-Swift runtime and language-support failures — a native library that will not load, an owner-thread violation, a stale runtime generation, an unsupported platform, callback lifecycle misuse |

Neither is a subtype of the other and neither wraps the other. Two tests assert
it in both directions, including that a `catch is CNAException` does **not**
swallow a `CNAError`.

## The default messages are read out of the binary, not remembered

This is the part that could most easily have been faked. None of the three
default messages is an IL literal — each constructor or getter loads a resource
**key** and calls `Environment.GetResourceString`:

```text
Exception::get_Message      _message ?? GetRuntimeResourceString(
                              "Exception_WasThrown", GetClassName())
SystemException::.ctor()    GetResourceString("Arg_SystemException")
ExternalException::.ctor()  GetResourceString("Arg_ExternalException")
```

So the sentence `new DeviceLostException().Message` returns is a fact about the
assembly's embedded string table. `bcl_authority_audit.py` now walks the PE
optional header to the CLI data directory, finds the length-prefixed
`System.Resources.ResourceReader` blob, parses the v2 resource format and reads
the values out. No external tool is invoked and nothing is written to disk:

```text
Exception_WasThrown     "Exception of type '{0}' was thrown."
Arg_SystemException     "System error."
Arg_ExternalException   "External component has thrown an exception."
```

They are pinned in `reference/bcl40-selected-shape.json` under `resourceStrings`
and the strict verifier reads the literals back out of the compiled Swift source
and compares them (`BCL_RESOURCE_STRING_PROJECTIONS=3`). A message transcribed
from documentation instead of from the assembly is now a `BASE_MAPPING_MISMATCH`.

The extraction was cross-checked twice: `monodis --mresources` produced a
286,527-byte blob, `ikdasm` reports the same `Length 0x00045F3F` for
`mscorlib.resources`, and the in-tool reader independently produced a blob of
that exact size carrying 2,434 entries.

## The selected surface, and what is deliberately absent

The projected members are exactly those whose behaviour is reconstructible from
managed state alone:

| member | why it is projectable |
|---|---|
| `Message` | `_message`, or the resource default formatted with the class name |
| `InnerException` | a stored field; `virtual final`, so `final` here |
| `HResult` | an `Int32` field the constructors write; `protected`, widened |
| `HelpLink` | plain `_helpURL` storage, `virtual` both ways, so `open` |
| `GetBaseException()` | a walk down the `InnerException` chain |
| `ExternalException.ErrorCode` | `call get_HResult`, nothing else |

Everything else `mscorlib` declares needs a CLR **runtime service** this
projection does not have, and is therefore **forbidden** by the verifier rather
than merely missing:

| member | the runtime service it needs |
|---|---|
| `StackTrace`, `Source` | the runtime's captured managed stack |
| `TargetSite` | `System.Reflection.MethodBase` |
| `GetType` | `System.Type` |
| `Data` | `System.Collections.IDictionary` and `ListDictionaryInternal` |
| `GetObjectData`, `SerializeObjectState` | the serialization runtime |
| `ToString` | `GetClassName` plus a stack trace it appends |

A `StackTrace` answering with an empty string would be a fabrication; an absence
the gate enforces is not. Four of these have a mutation proving the gate fires.

## `GetClassName`, and its one recorded divergence

`get_Message` formats the resource string with `GetClassName()`, the
namespace-qualified CLR name of the object's **dynamic** type. The Swift
namespace enums mirror the CLR namespaces exactly, so the reflected Swift name
is that CLR name with this module's qualifier in front, and removing that
qualifier recovers it. All eleven projected types are asserted by name.

Two details worth stating, both found by the tests rather than by inspection:

- **Only this module's qualifier is removed, and only when it is there.**
  Stripping whichever component sorts first would also strip a *consumer's*
  module from their own subclass of one of the two unsealed XNA exceptions,
  leaving `MyException` where the CLR reports `MyGame.MyException`. The
  qualifier is read back from a type known to live in this module rather than
  written down, so renaming the module cannot desynchronize it.
- **A `private`/`fileprivate` or function-local subclass gets a mangled name.**
  Swift has no nominal context to name there and substitutes
  `(unknown context at $ADDRESS)`, which is not stable between runs. It is
  reachable only by a consumer subclassing `ContentLoadException` or
  `StorageDeviceNotConnectedException` in such a scope, affects nothing but the
  synthesized default message of that consumer's own type, and a test asserts
  it so the divergence cannot quietly change.

It is a single implementation rather than a per-class override, because an
override on `CNAExternalException` would be inherited by its three XNA
subclasses and would report the base's name for all of them.

## Nullability

`message` and the inner-exception parameter are Optional on every projected
constructor. The CLR stores both without a null test, and `get_Message`
explicitly **branches** on `_message == null` to select a different documented
result — which is the repository's definition of an observable selected
operation rather than an immediate invalid argument. An empty string is a
different state and stays one; a test asserts that `Exception("")` reports `""`
while `Exception(nil)` reports the synthesized default.

`Message` itself is non-Optional: the CLR getter can never return null.
`InnerException` and `HelpLink` are Optional; `GetBaseException()` is not,
because it returns `this` at minimum.

## The eight XNA types

Re-derived from `Microsoft.Xna.Framework.dll` (`38e7093f…a130`),
`Microsoft.Xna.Framework.Graphics.dll` (`560080fc…9f55`) and
`Microsoft.Xna.Framework.Storage.dll` (`798f678e…cbb8`).

| type | CLR base | sealed | ctors |
|---|---|---|---|
| `Audio.InstancePlayLimitException` | `ExternalException` | yes | 3 |
| `Audio.NoAudioHardwareException` | `ExternalException` | yes | 3 |
| `Audio.NoMicrophoneConnectedException` | `Exception` | yes | 3 |
| `Content.ContentLoadException` | `Exception` | **no** | 3 + 1 protected |
| `Graphics.DeviceLostException` | `Exception` | yes | 3 |
| `Graphics.DeviceNotResetException` | `Exception` | yes | 3 |
| `Graphics.NoSuitableGraphicsDeviceException` | `Exception` | yes | 3 |
| `Storage.StorageDeviceNotConnectedException` | `ExternalException` | **no** | 3 + 1 protected |

Every constructor body is a bare forward — `ldarg.0`, the arguments,
`call base..ctor`, `ret`. **No XNA type synthesizes a message of its own**, so
whatever the base produces is what the subclass reports, and the split between
the two halves is directly observable: `NoAudioHardwareException()` reports
`"External component has thrown an exception."` and `0x80004005`, while
`NoMicrophoneConnectedException()` reports
`"Exception of type 'Microsoft.Xna.Framework.Audio.NoMicrophoneConnectedException' was thrown."`
and `0x80131500`.

CLR `sealed` maps to Swift `final`; the two unsealed types are `open` and a test
derives from both.

### Six complete, two partial — the serialization constructors

`ContentLoadException` and `StorageDeviceNotConnectedException` each declare a
fourth, `protected`, `(SerializationInfo, StreamingContext)` constructor. The
other six declare the same constructor **private**, so it is not in the public
contract and is not owed.

Those two are deliberately **not** implemented. `Exception`'s own deserialization
constructor reads thirteen named values back out of the `SerializationInfo` —
`ClassName`, `Message`, `Data`, `InnerException`, `HelpURL`, `StackTraceString`,
`RemoteStackTraceString`, `RemoteStackIndex`, `ExceptionMethod`, `HResult`,
`Source`, `WatsonBuckets`, `SafeSerializationManager` — through
`GetString`, `GetInt32` and `GetValue(name, System.Type)`, then throws
`SerializationException` if the state is insufficient. Reproducing that needs
`System.Type`, `System.Collections.IDictionary` and a deserialization runtime,
and Swift has no `protected`, so a projected version would be **publicly
callable** and would silently hand back an exception carrying none of the
serialized state.

They are therefore two named `MISSING_MEMBER`s with a stated blocker, and the
two types stay `PARTIAL`. Nothing in XNA calls either constructor.

## What the audit gained

```text
BCL_AUTHORITY_TYPES     8  ->  11
BCL_AUTHORITY_MEMBERS  94  -> 122
BCL_SENTINEL_CHECKS   125  -> 194
BCL_MUTATION_SELF_TESTS 97 -> 168
BCL_CROSS_CHECKS       41  ->  56   (monodis)
BCL_RESOURCE_CHECKS     0  ->   3   new
BCL_NEGATIVE_CONTROLS   4      4    all still refused
BCL_AUTHORITY_STATUS=PASS

bcl40-selected-shape.json
  SHA256=463f1f334156113714a0e410ccb2d25f10dafe560e2bef42f82fa65db1808f23
  regenerates BYTE-IDENTICALLY with and without an IL cache
```

## The defect this milestone's own gates caught

**The IL parser silently merged two types.** `Parser.skip_block` stopped only on
the commented closing line `ikdasm` normally emits — but a
`pinvokeimpl(...) ... preservesig` method whose body is nothing but `.custom`
attributes closes with a **bare `}`**. `System.Exception` contains exactly such
a method (`GetMessageFromNativeResources`), so the skip ran past the method,
past the class, and into `System.ValueType`, attributing all of that type's
members — a second `ToString()`, a protected parameterless constructor,
`Equals`, `GetHashCode` — to `System.Exception`.

Nothing had hit it before because no XNA type in the pinned contract contains
that construct; the first BCL family to need it exposed it. The boundary is now
brace depth, which for a well-formed method is the same line the comment marks,
so the XNA contract still reproduces **257 types / 2,964 members exactly**. A
minimised fixture is now one of the audit's self-tests
(`AUDIT_SELF_TESTS` 60 → 64) and it fails against the old implementation.

## Two new general rules, both currently green

**`INHERITANCE_MAPPING_MISMATCH`: a CLR `sealed` class must be a Swift `final`
class.** Measured over every implemented reference class, never per named type:
32 classes measured, 13 sealed, all `final`, so the category is 0.

The converse is **recorded, not diagnosed**. `NONDERIVABLE_UNSEALED_CLASSES=5`
names five classes XNA leaves derivable that this projection has sealed:

```text
Microsoft.Xna.Framework.GameTime
Microsoft.Xna.Framework.Graphics.GraphicsDevice
Microsoft.Xna.Framework.Graphics.SpriteBatch
Microsoft.Xna.Framework.Graphics.Texture2D
Microsoft.Xna.Framework.GraphicsDeviceManager
```

`Texture2D` is the one that matters: XNA's `RenderTarget2D` derives from it, so
while it is `final` that type is inexpressible. Unsealing a public class is a
public-API decision of its own and not a side effect of this rule, so the five
are named in the report and left for their own milestone rather than changed
here or suppressed.

## Structural scoreboard

```text
REFERENCE_TYPES=257                  unchanged
REFERENCE_MEMBERS=2964               unchanged
EXPECTED_SWIFT_MEMBERS=2887          unchanged
TARGET_TYPES=134                     (126 -> 134)
TARGET_MEMBERS=1730                  (1706 -> 1730)
TOTAL_DIAGNOSTICS=280                (284 -> 280)
COMPLETE_TYPES=127                   (121 -> 127)
PARTIAL_TYPES=7                      (5 -> 7)
MISSING_TYPE=123                     (131 -> 123)
MISSING_MEMBER=132                   (130 -> 132)
BASE_MAPPING_MISMATCH=2              unchanged
INTERFACE_MAPPING_MISMATCH=1         unchanged
PROPERTY_MAPPING_MISMATCH=4          unchanged
OVERLOAD_MAPPING_MISMATCH=18         (16 -> 18)
INHERITANCE_MAPPING_MISMATCH=0       new
every other mismatch/leak category=0
UNMEASURED_STRUCTURAL_CATEGORY=0
ALLOWLIST_ENTRIES=0

BCL_SUPPORT_TYPE_MEASUREMENTS=6      (3 -> 6)
BCL_BASE_PROJECTIONS=13              (5 -> 13)
PROJECTED_BCL_BASE_TYPES=9           (1 -> 9)
PENDING_BCL_BASE_TYPES=4             unchanged
BCL_INHERITED_MEMBER_PROJECTIONS=59  (16 -> 59)
BCL_RESOURCE_STRING_PROJECTIONS=3    new
XNA_SEALED_CLASS_PROJECTIONS=13      new
NONDERIVABLE_UNSEALED_CLASSES=5      new, recorded not diagnosed
MEASURED_SUPPORT_BASE_PROJECTIONS=17 (9 -> 17)
NAMESPACE_MARKERS=11                 (10 -> 11, Storage)
```

**`EXPECTED_SWIFT_MEMBERS` did not move**, and it should not have: the 24 new
`TARGET_MEMBERS` are the declared XNA identities of the eight new types, every
one of which was already in the pinned contract and already counted. The surface
those types *inherit* is real and usable and is **not** an XNA identity, so it
is counted once, on its own axis, as `BCL_INHERITED_MEMBER_PROJECTIONS` —
16 for `GameComponentCollection`, 5 for each of the five `CNAException`
subclasses, and 6 for each of the three on `CNAExternalException`.

## Behaviour

```text
PURE_XNA_DERIVED  1900/1900/0    (1861 -> 1900)
PURE_BCL_DERIVED   126/126/0     (77 -> 126)
DEBUG_TESTS=340 PASS             (318 -> 340)
API_COMPAT_SELF_TESTS=2344       (2288 -> 2344)
SYMBOL_GRAPH_SELF_TESTS=17
AUDIT_SELF_TESTS=64              (60 -> 64)
```

The behaviour corpus keeps the two authorities apart: the BCL half's authority
is the admitted `mscorlib` and is counted in no XNA total.

## A note on `Sendable`

The three support classes carry a `Sendable` conformance and it is worth being
exact about why: the Swift standard library declares `Error` as refining
`Sendable`, so conforming to `Error` conforms to `Sendable` and this projection
can neither add nor decline it. It is the language's conformance, not a claim.
`HResult` and `HelpLink` are mutable exactly as `mscorlib` declares them, and
all three classes are `open`, so nothing about the cross-actor safety of a
`CNAException` is being asserted.

## Runtime throw sites — nothing was rewired, and why

The exact-payload rule was applied and produced **no** change to any existing
`throw`. Every implemented member whose failure path could in principle be one
of these eight raises it through the canonical CNA C ABI, which reports device
failures as a single undifferentiated result code. Nothing implemented can yet
tell a lost device from a not-reset one, so `DeviceLostException` and
`DeviceNotResetException` would be a guess; the throw sites keep their truthful
`CNAError.nativeFailure` and the distinction is recorded as a producer blocker.
The other six have no reachable producer at all — `SoundEffect`, `Microphone`,
`ContentManager`, `StorageDevice` and adapter enumeration are all still absent.

The exception **types** are nevertheless complete: a consumer can construct,
throw and catch every one of them today, and a future producer changes the
throw site rather than the type.
