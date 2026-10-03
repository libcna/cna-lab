# Foundation 31 — `Dictionary<K,V>` is a reference class, and the CLR storage is reproduced

`Microsoft.Xna.Framework.LaunchParameters` derives from
`System.Collections.Generic.Dictionary<string, string>` and declares **one
parameterless constructor and nothing else**. Like `GameComponentCollection`
and the eight exception types before it, its entire usable surface is
inherited, so it was blocked on a base with no decided projection. This
milestone decides that projection, reproduces the CLR's own storage, and
completes `LaunchParameters`.

## The rule, and the shortcut that was refused

```text
System.Collections.Generic.Dictionary`2          ->  CNADictionary<Key, Value>
System.Collections.Generic.IEqualityComparer`1   ->  CNAEqualityComparer
System.Collections.Generic.KeyValuePair`2        ->  CNAKeyValuePair<Key, Value>
Dictionary`2+KeyCollection                       ->  CNADictionary.KeyCollection
Dictionary`2+ValueCollection                     ->  CNADictionary.ValueCollection
```

```swift
open class LaunchParameters: CNADictionary<String, String>
```

Not `[String: String]`, and not a class wrapping one. A Swift dictionary is a
**value**, so a caller would receive a copy of the launch data rather than the
object the `Game` owns, and three further behaviours would be inexpressible:

- `Add` refuses a duplicate key while the indexer setter overwrites it — one
  boolean argument to the same private `Insert`, and the whole difference;
- enumeration has a defined order, and the CLR's is insertion order;
- a version counter invalidates a live enumerator, **including on an
  overwrite**, where nothing was inserted at all.

## The storage is the CLR's, not an approximation

The obvious implementation — a Swift `Dictionary` plus a separate array to
remember order — would have got the easy cases right and the interesting ones
wrong. `Dictionary<TKey,TValue>` in the admitted `mscorlib` is:

```text
buckets   int[]      chain heads, -1 for empty
entries   Entry[]    { hashCode, next, key, value }, hashCode -1 when freed
count     int        the high-water mark of the entry array
freeList  int        head of the freed-slot chain, threaded through `next`
freeCount int        how many slots are on it
version   int        bumped by every mutation that changes anything
Count  =>  count - freeCount
```

All of it is reproduced, so the observable behaviour follows from the
algorithm rather than being simulated beside it:

```text
Initialize(capacity)  size = HashHelpers.GetPrime(capacity)
                      buckets = new int[size] filled with -1
                      entries = new Entry[size]; freeList = -1
FindEntry(key)        hash = comparer.GetHashCode(key) & 0x7FFFFFFF
                      walk buckets[hash % length]; compare hashCode FIRST,
                      then comparer.Equals
Insert(key, v, add)   found ? (add ? throw Argument_AddingDuplicate
                                   : entries[i].value = v, version++)
                      else  freeCount > 0 ? take freeList
                                          : (count == entries.Length ? Resize)
                                            index = count++
Remove(key)           unlink; hashCode = -1; next = freeList; freeList = i;
                      freeCount++; version++
Clear()               guarded on count > 0 in its entirety
```

Three consequences are asserted by tests because they are exactly what a
plausible reimplementation gets wrong:

- **A removed slot is reused by the next insertion.** Remove `b` from
  `a, b, c` and add `d`, and the enumeration is `a, d, c` — not `a, c, d`.
- **The free list is last-freed-first.** Remove `b` then `c`, add `x` then `y`,
  and the order is `a, y, x, d`.
- **Clearing an already empty dictionary does not bump the version**, so it
  does not invalidate a live enumerator. The CLR guards the whole body on
  `count > 0`.

`Resize` copies the entries in order and re-chains them, so growing a
dictionary never reorders enumeration; a forty-entry test crosses several
growth points and still enumerates in insertion order.

## Enumeration order does not depend on the hash — and that settles the hash question

The single most useful fact found while deriving this: **an entry's index comes
from `count++` or from the free list, never from its hash.** The bucket array
decides only which chain a lookup walks. So enumeration order — the one thing a
consumer can see — is reproducible without reproducing a single CLR hash code.

That is what makes the comparer's `GetHashCode` a narrow, statable divergence
rather than a blocker:

| | status |
|---|---|
| `Equals` on the default comparer | **exact** — `cnaDefaultEquals` resolves it the way the CLR resolves `EqualityComparer<T>.Default`, preferring `IEquatable<T>` and otherwise reducing to reference identity |
| `GetHashCode` on the default comparer | **consistent with `Equals`, but not the CLR's number** |

`EqualityComparer<T>.Default.GetHashCode(x)` is `x.GetHashCode()`, a value
Microsoft documents as unspecified, version-dependent and unsafe to persist,
and which later .NET randomised per process. Reproducing a number the platform
itself refuses to guarantee would be false precision. Nothing observable in
`CNADictionary` depends on it: enumeration order is entry order, `Count` is a
counter, and a lookup needs only that equal keys hash equally.

A **caller-supplied** comparer is a different matter, and it is exact. The
dictionary asks the comparer — never Swift's `Hashable` — for both halves of
key identity, and a test proves the sharp end of that: a comparer whose
`Equals` says everything is equal but whose hashes differ **cannot find its own
entries**, because `FindEntry` compares the stored hash before it asks
`Equals`. A projection built on Swift hashing could not have exhibited that.

## The sizing table is read out of the assembly

`Initialize` and `Resize` size their arrays through `HashHelpers.GetPrime`,
which reads a 72-entry static table. Nothing observable depends on its values —
a lookup is correct for any bucket count — but transcribing it by hand would
have been a sizing decision of this projection's own, which is exactly what the
pinning exists to exclude. So the audit follows the chain the C# compiler
actually emits:

```text
HashHelpers::.cctor  ldc.i4.s 72; newarr System.Int32
                     ldtoken field ...'$$method0x600612d-1'
                     call RuntimeHelpers::InitializeArray
field '$$method0x600612d-1' at I_0042E144
.data cil I_0042E144 = bytearray (03 00 00 00 07 00 00 00 ...)   288 bytes
```

and reads 72 little-endian `int32`s out of it. The `newarr` count bounds the
read, so a longer blob cannot silently add entries, and the `//` rendering
`ikdasm` appends to each row is stripped before any byte is taken. The table is
pinned in the manifest under `staticTables`, and the verifier reads the Swift
literal back out of the compiled source and compares it element for element
(`BCL_STATIC_TABLE_PROJECTIONS=1`).

The table this session first wrote from memory turned out to match the binary
exactly. That is not the point: it is now checked rather than trusted, and a
future edit to it fails the gate.

## Four more resource strings, read the same way

`Dictionary`'s failures are resource lookups, not IL literals, so the four
messages it can produce were read out of the embedded `mscorlib.resources`
table by the same PE walk Foundation 30 built:

```text
Argument_AddingDuplicate          "An item with the same key has already been added."
Arg_KeyNotFound                   "The given key was not present in the dictionary."
ArgumentOutOfRange_NeedNonNegNum  "Non-negative number required."
Arg_ArrayPlusOffTooSmall          "Destination array is not long enough to copy all
                                   the items in the collection. Check array index
                                   and length."
```

`BCL_RESOURCE_STRING_PROJECTIONS` is now 7, and the source reader was
generalised twice to keep the check honest: it now scans every `CNA*.swift`
support source rather than one file, and it collapses `"first " + "second"`
seams before matching, so a message too long for one line is compared against
what the program produces rather than against however the source wrapped it.

## The selected surface

Projected: the six public constructors, `Add`, `Clear`, `ContainsKey`,
`ContainsValue`, `Remove`, `TryGetValue`, `GetEnumerator`, `Count`, `Comparer`,
the read/write indexer, `Keys` and `Values`.

Forbidden, for the same reason as on the exception families — they need a CLR
runtime service this projection does not have:

| member | needs |
|---|---|
| `GetObjectData` | the serialization runtime |
| `OnDeserialization` | the deserialization callback runtime |
| the protected `(SerializationInfo, StreamingContext)` constructor | both |

**Every public member of `Dictionary<TKey,TValue>` is `virtual final` or
non-virtual**, so nothing on it is an override point; the only two overridable
members are the two that are not projected. The Swift surface is therefore
`final` throughout while the class itself is `open` — which is exactly what
lets `LaunchParameters` derive from it and change nothing. A sentinel asserts
the split and a mutation proves the assertion fires.

`KeyValuePair.ToString()` is absent for the same class of reason: the CLR
builds `"[key, value]"` through the virtual `Object.ToString` of each
component, which Swift cannot dispatch for an unconstrained generic, and a
`String(describing:)` stand-in would be a different string presented as the
same member.

## Two shapes that needed a decision

**`TryGetValue`'s out parameter is `Value?`.** The CLR writes
`default(TValue)` on failure — null for every reference type. Swift cannot
synthesize a default for an unconstrained generic, so an `inout Value` would
have had to leave the caller's previous value in place, which the CLR never
does. The Optional projection reproduces the CLR result exactly for a reference
type and is an honest "no value" otherwise. A test passes a non-nil value in
and asserts it comes back nil.

**`GetEnumerator` returns `CNAEnumerator<CNAKeyValuePair<Key, Value>>`.** The
CLR returns its sealed nested `Enumerator` struct, whose entire surface is
`Current`, `MoveNext` and `Dispose` — that is, the enumeration contract and
nothing else. A sentinel asserts exactly that member set, so the claim "the
struct adds nothing the established `CNAEnumerator` projection would drop" is
checked rather than assumed.

## `LaunchParameters` — the constructor parses the command line

**Corrected after this document was first written.** The first version of this
milestone implemented the constructor as a bare `base..ctor()` and recorded
"nothing here fabricates launch data" as a virtue. That was wrong: the IL is

```text
LaunchParameters..ctor()
    base..ctor()
    ParseCommandLineArguments(Environment.GetCommandLineArgs())
```

so a `LaunchParameters` populates itself from the process it is running in, and
an implementation that left it empty was the fabrication — it would have
reported "no launch parameters" to a game that had been given some. The error
was caught while deriving `Game`'s own constructor, which is where the type is
allocated.

The parse, exactly:

```text
ParseCommandLineArguments(args)
    separators = { '/', '-' }
    if (args.Length <= 1) return              // element 0 is the executable
    for (i = 1; i < args.Length; i++) {
        argument = args[i].TrimStart(separators)
        ParseKeyValuePair(argument, out key, out value)
        if (!ContainsKey(key) && key != "") Add(key, value)
    }

ParseKeyValuePair(argument, out key, out value)
    key = argument; value = ""
    colon = argument.IndexOf(':')
    if (colon == -1) return
    key = argument.Substring(0, colon); value = argument.Substring(colon + 1)
```

`CommandLine.arguments` is the exact analogue of `GetCommandLineArgs()` —
element zero is the executable and the rest are the arguments — which is why
the parse starts at index one. Five details have tests, because each is a way a
plausible reimplementation differs:

- the executable is **skipped**, so a vector of length one produces nothing;
- `TrimStart` removes **every** leading `/` or `-`, in any mix, so `--x` and
  `//y` are the keys `x` and `y`;
- an argument with no colon is a key with an **empty value**, not a missing one;
- only the **first** colon splits, so `/path:c:/tmp` is `path` → `c:/tmp`;
- the **first** occurrence of a repeated key wins, because the guard is
  `!ContainsKey(key)` — a later one is discarded rather than overwriting.

An argument that trims away to nothing produces an empty key and is dropped.

`Game.LaunchParameters` is still missing, so what a consumer cannot yet get is
the instance the `Game` allocated — but constructing one now gives the same
contents XNA would, because both read the same process.

### `SetItem` does not throw

Corrected with it. `set_Item` is `Insert(key, value, add: false)`, and
`Insert`'s only failures are `ArgumentNullException` for a null key —
unreachable through a non-Optional Swift parameter — and the duplicate-key
`ArgumentException`, which it raises **only when `add` is true**. So the
indexer setter cannot fail for any value a caller can supply, and it is no
longer `throws`; `Add`, which passes `add: true`, is the one that can. `Insert`
now reports the duplicate as a `Bool` so the two members can differ, which is
also what lets `LaunchParameters` insert under the CLR's own `ContainsKey`
guard without a `try` that could never fire.

## Exception identity is the next milestone, and it is stated rather than hidden

`CNADictionary` reports its failures as `CNAError` values carrying the CLR's
**exact messages** — `CNAError.argument("An item with the same key has already
been added.")`, `CNAError.keyNotFound(…)`, `CNAError.argumentOutOfRange("capacity")`.
The message is right; the exception **class** is not. The CLR raises
`ArgumentException`, `KeyNotFoundException` and `ArgumentOutOfRangeException`,
and Foundation 30 has just made those projectable.

They were not converted here on purpose. `CNAList` and `CNACollection` report
the same family of failures the same way, and converting one support type while
leaving the others would make the BCL layer internally inconsistent — worse
than either uniform choice. The conversion is a milestone of its own, covering
`ArgumentException`, `ArgumentNullException`, `ArgumentOutOfRangeException`,
`InvalidOperationException`, `NotSupportedException` and `KeyNotFoundException`
across every BCL support type at once, and it is named here so it cannot be
quietly forgotten.

`CNAError` gained one case, `keyNotFound`, so that a missing key is not
mislabelled as an argument failure in the meantime.

## What the audit gained

```text
BCL_AUTHORITY_TYPES     11  ->  18
BCL_AUTHORITY_MEMBERS  122  -> 167
BCL_SENTINEL_CHECKS    194  -> 294
BCL_MUTATION_SELF_TESTS 168 -> 320
BCL_CROSS_CHECKS        56  ->  91   (monodis)
BCL_RESOURCE_CHECKS      3  ->   7
BCL_STATIC_TABLE_CHECKS  0  ->   1   new
BCL_NEGATIVE_CONTROLS    4       4   all still refused
BCL_AUTHORITY_STATUS=PASS

bcl40-selected-shape.json
  SHA256=89b4e292eb4709f87349c645e692072fb95104ddfa19806887722898f2543601
```

Nineteen new mutations aim at the dictionary family specifically: sealing the
class, changing its arity, rebasing it, making a public method an override
point, dropping a constructor, a property, a method or a declared interface,
retyping a property, unsealing a collection view, giving a view a mutator,
turning the nested `Enumerator` into a class or giving it a member beyond the
contract, turning `KeyValuePair` into a class or giving `Key` a setter, and
turning `IEqualityComparer<T>` into a class or dropping its `GetHashCode`.

## Structural scoreboard

```text
REFERENCE_TYPES=257                  unchanged
REFERENCE_MEMBERS=2964               unchanged
EXPECTED_SWIFT_MEMBERS=2887          unchanged
TARGET_TYPES=135                     (134 -> 135)
TARGET_MEMBERS=1731                  (1730 -> 1731)
TOTAL_DIAGNOSTICS=279                (280 -> 279)
COMPLETE_TYPES=128                   (127 -> 128)
PARTIAL_TYPES=7                      unchanged
MISSING_TYPE=122                     (123 -> 122)
MISSING_MEMBER=132                   unchanged
BASE_MAPPING_MISMATCH=2              unchanged
INTERFACE_MAPPING_MISMATCH=1         unchanged
PROPERTY_MAPPING_MISMATCH=4          unchanged
OVERLOAD_MAPPING_MISMATCH=18         unchanged
INHERITANCE_MAPPING_MISMATCH=0       unchanged
every other mismatch/leak category=0
UNMEASURED_STRUCTURAL_CATEGORY=0
ALLOWLIST_ENTRIES=0

BCL_SUPPORT_TYPE_MEASUREMENTS=9      (6 -> 9)
BCL_BASE_PROJECTIONS=14              (13 -> 14)
PROJECTED_BCL_BASE_TYPES=10          (9 -> 10)
PENDING_BCL_BASE_TYPES=4             unchanged
BCL_INHERITED_MEMBER_PROJECTIONS=72  (59 -> 72)
BCL_RESOURCE_STRING_PROJECTIONS=7    (3 -> 7)
BCL_STATIC_TABLE_PROJECTIONS=1       new
```

`EXPECTED_SWIFT_MEMBERS` again did not move: the one new `TARGET_MEMBER` is
`LaunchParameters`'s declared constructor, already in the pinned contract and
already counted. The thirteen members it *inherits* are counted once, on their
own axis, in `BCL_INHERITED_MEMBER_PROJECTIONS`.

## Behaviour

```text
PURE_XNA_DERIVED  1923/1923/0    (1900 -> 1923; see the LaunchParameters correction above, which raised it further)
PURE_BCL_DERIVED   208/208/0     (126 -> 208)
DEBUG_TESTS=366 PASS             (340 -> 366)
API_COMPAT_SELF_TESTS=2378       (2344 -> 2378)
SYMBOL_GRAPH_SELF_TESTS=17 PASS
```
