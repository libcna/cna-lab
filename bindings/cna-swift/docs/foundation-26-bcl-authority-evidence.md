# Foundation 26 — a separate BCL behaviour authority

The XNA-era Microsoft `mscorlib` was found and its identity established in the
previous session, but nothing was registered, because the existing registration
gate could not honestly admit it. This milestone builds the mechanism that can,
and admits the assembly through it.

Nothing about the XNA contract changed. `REFERENCE_TYPES` is still 257 and
`REFERENCE_MEMBERS` is still 2,964, the seven registered XNA assemblies still
reproduce them exactly, and no XNA counter moved. The BCL is measured on its
own axis, with its own registry, its own calibration and its own counters.

## Why a separate mechanism, not another registered assembly

`pinned_assembly_audit.py` earns an XNA assembly its behaviour authority by
machine-comparing its public metadata against **every contract entry it
declares**. That is a strong gate precisely because the contract is large and
independent: `Microsoft.Xna.Framework.dll` has to reproduce 100-odd types and
their members exactly, or it is refused.

`mscorlib` declares **zero** XNA contract types. Run through that gate it would
compare nothing, find nothing wrong, and be reported as calibrated — the
textbook vacuous pass. Putting it in `registered-assemblies.json` would also
have corrupted the XNA scoreboard's meaning: `CONTRACT_TYPES_REPRODUCED` counts
contract types, and mscorlib owns none of them.

So the BCL gets `tools/api_compat/bcl-authorities.json`, audited by
`tools/api_compat/bcl_authority_audit.py`, calibrated against
`tools/api_compat/reference/bcl40-selected-shape.json`. The XNA registry is
untouched.

## What the assembly had to prove

Five independent things, all of which must pass. None of them is a restatement
of another.

### 1. Exact identity — 21 checks

```text
assembly name       mscorlib                      (read from .assembly)
assembly version    4.0.0.0                       (read from .ver 4:0:0:0)
public key          00000000000000000400000000000000
public key token    b77a5c561934e089              RECOMPUTED, not read
module              CommonLanguageRuntimeLibrary
file size           5196112
sha256              5634668d4775b0113f08ea31093b281fea69bfc4e99227f5ca761b4ed98acc63
```

The strong-name token is **computed** from the assembly's own public key — the
last eight bytes of its SHA-1, reversed — and compared to the registered
`b77a5c561934e089`. A binary whose key does not actually hash to that token
cannot be admitted by asserting the token, and the GAC directory name is
corroboration rather than evidence.

### 2. Microsoft provenance

```text
CompanyName        Microsoft Corporation
FileDescription    Microsoft Common Language Runtime Class Library
FileVersion        4.0.30319.1 (RTMRel.030319-0100)
ProductName        Microsoft® .NET Framework
ProductVersion     4.0.30319.1
OriginalFilename   mscorlib.dll
InternalName       mscorlib.dll
strong-name key    f:\dd\Tools\devdiv\ecmapublickey.snk
module imports     clr.dll, mscoree.dll        (both required, both present)
forbidden markers  Mono.Runtime, Mono.RuntimeStructs, System.Private.CoreLib,
                   MonoTODO                    (all four absent)
```

The version resource is read from the PE image anchored at its
`VS_VERSION_INFO` key, so an unrelated UTF-16 occurrence of a field name
elsewhere in a five-megabyte image cannot be mistaken for it. That anchoring is
load-bearing: an unanchored scan for `ProductName` in this exact binary returns
a fragment of an unrelated resource.

### 3. A pinned selected-shape manifest — 8 types, 94 members

Authority is demand-driven. mscorlib is **not** indexed as a pseudo-XNA
contract; only the families the binding actually derives behaviour from are
recorded, each with the reason it is there:

| Family | Why it is admitted |
|---|---|
| `Collection\`1` | the direct CLR base of `GameComponentCollection` |
| `ReadOnlyCollection\`1` | the direct CLR base of the four `Model*Collection` types |
| `List\`1` | the type `Collection<T>..ctor()` actually allocates as its backing store |
| `List\`1+Enumerator` | where the mutation-invalidation contract is declared |
| `IList\`1` | the declared type of the backing field, both wrapping constructors and `Items` |
| `ICollection\`1` | supplies the `Count`/`IsReadOnly` reads that guard every mutator |
| `IEnumerable\`1` | supplies the `GetEnumerator` both families delegate to |
| `IEnumerator\`1` | the declared return type of both `GetEnumerator`s |

For each, the manifest records the full CLR identity, kind, sealed/abstract,
base, generic arity and parameter names, declared interfaces, and every public
and protected member with its visibility, static/instance identity,
virtual/final/abstract flags, **overridability**, return type and parameter
types. It is generated mechanically from the admitted binary and regenerates
**byte-identically**, with and without an IL cache:

```text
bcl40-selected-shape.json
  SHA256=ef5f2428a546b677d797b5def4c07c44e04419844418b13de9a20f61aa323f0e
```

The Microsoft binary is not stored in the repository; the derived manifest is.

### 4. Sentinel calibration — 125 checks

This is what stops the calibration being circular. The manifest is generated by
the extractor, so a broken extractor would agree with a manifest it had itself
written. The sentinels are therefore facts about the selected families stated
in the audit *independently of the extraction*, and every one must hold:

- `Collection<T>` and `ReadOnlyCollection<T>` are unsealed, non-abstract
  classes of generic arity 1 whose base is `System.Object` — and specifically
  **neither derives from the other**. Getting that backwards would produce a
  Swift hierarchy in which a mutable collection inherits a read-only one;
- both declare all six of `IList<T>`, `ICollection<T>`, `IEnumerable<T>`,
  `IList`, `ICollection`, `IEnumerable`;
- `Collection<T>` declares **exactly two** public constructors, one
  parameterless and one taking `IList<T>`; `ReadOnlyCollection<T>` declares
  **exactly one**, taking `IList<T>`, and deliberately has no parameterless
  constructor — a read-only view must always be a view *of* something;
- the four hooks `ClearItems`/`InsertItem`/`RemoveItem`/`SetItem` are each
  declared exactly once, protected, non-abstract, void, of arity 0/2/1/2, and
  **overridable**;
- `ReadOnlyCollection<T>` declares none of those four, and exposes no mutator;
- every public method of `Collection<T>` is `virtual final` — a *sealed*
  interface implementation. A subclass changes behaviour only through the four
  hooks, so a projection that made `Add` or `Insert` overridable would invent
  an extension point the CLR does not have;
- `Count` is a public get-only `Int32` on both; `Items` is a **protected**
  get-only `IList<T>` on both; `Collection<T>.Item` is a public read/write
  `Int32` indexer and `ReadOnlyCollection<T>.Item` has no setter at all.

The sentinels found a real defect in the extractor on first run: it filtered
members by the raw IL access tokens (`family`) rather than the mapped
spellings (`protected`), silently dropping every protected member — which is to
say, all four hooks and both `Items` properties. That is precisely the failure
mode a self-agreeing manifest would have hidden.

### 5. Mutation self-tests — 97 checks

Twenty mutations are applied to each already-matching record and each must be
detected: dropped method / constructor / property, invented member, changed
generic arity, renamed generic parameter, changed base, dropped or invented
interface, changed kind, flipped sealed, widened visibility, removed a virtual
flag, unsealed a sealed public member, changed parameter type, changed return
type, changed property type, flipped static, renamed member, invented a
setter. Twelve further mutations must each break at least one **sentinel**, and
removing any selected family entirely must break them too. A final check
asserts the sentinels fail against an empty extraction.

### 6. An independent metadata reader — 41 checks

`--cross-check` re-reads the same binary with `monodis`, a separate
implementation from `ikdasm`. The comparison is index-sliced rather than
name-grepped: `monodis --typedef` gives each type's first method-table row, so
one type's rows are exactly the half-open range up to the next type's. For
every admitted family the type's interface/sealed/abstract flags are decoded
from `TypeAttributes` and compared, and every member the extraction claims —
with property and event accessors expanded back to `get_`/`set_`/`add_`/
`remove_` — must appear in monodis's method table.

The exact Microsoft binary remains the authority. `monodis` validates this
tool's *parsing*, never the behaviour. It immediately earned its place by
catching a defect: generic methods are rendered `ConvertAll<TOutput>`, and the
cross-check's own name parser was taking the whole token.

## Negative controls — the gate actually rejects

A gate that admits everything passes every positive test above. Four binaries
are therefore offered to the audit as `mscorlib.dll` and must be refused, each
run through exactly the checks a real admission runs through:

| Offered binary | Rejected by |
|---|---|
| Mono 4.5 `mscorlib.dll` | 16 of 21 checks |
| Mono 4.0-api `mscorlib.dll` | 12 of 21 checks |
| Microsoft `System.dll` 4.0.0.0 | 10 of 21 checks |
| `Microsoft.Xna.Framework.Game.dll` | 14 of 21 checks |

The Mono 4.0-api facade is the strongest of the four, and the reason the
provenance checks exist rather than the hash alone: it is *named* `mscorlib`,
declares assembly version `4.0.0.0`, carries the ECMA public key and token
`b77a5c561934e089`, and even reports `FileVersion 4.0.30319.1`. It is rejected
because `CompanyName` is `Mono development team`, `ProductName` is
`Mono Common Language Infrastructure`, the DevDiv strong-name key file is
absent, `clr.dll` and `mscoree.dll` are not imported, and `MonoTODO` appears
352 times. "Do not use Mono as the behaviour authority" is now a mechanical
refusal, not a convention.

### A cache-collision defect the controls exposed

The negative controls initially reported the Mono binaries as rejected without
any provenance marker firing. The cause was real and would have mattered: the
shared IL cache was keyed by file *stem*, and every one of these files is named
`mscorlib.dll`, so the control was handed the **admitted** assembly's
disassembly. Every IL-derived check was therefore reading the right binary
while judging the wrong one.

The cache is now keyed by the binary's own SHA-256, the controls do not write
to the cache at all, and the markers fire as they should. This is the concrete
form of the standing rule that the shared IL cache is a convenience and never
an authority.

## Counters — a separate axis, no double counting

```text
BCL_AUTHORITY_ASSEMBLIES=1
BCL_AUTHORITY_TYPES=8
BCL_AUTHORITY_MEMBERS=94
BCL_IDENTITY_CHECKS=21
BCL_SENTINEL_CHECKS=125
BCL_MANIFEST_CHECKS=8
BCL_MUTATION_SELF_TESTS=97
BCL_CROSS_CHECKS=41 (monodis)
BCL_NEGATIVE_CONTROLS=4
BCL_AUTHORITY_STATUS=PASS
```

These are new names. No XNA counter is reused, incremented or reinterpreted,
and none of the 8 BCL types or 94 BCL members is an XNA type or an XNA
identity. `PINNED_ASSEMBLY_AUDIT` is still `257_TYPES/2964_MEMBERS`.

## The behaviour this admits — derived from IL, not from documentation

Every statement below was read from the admitted binary's CIL.

### `Collection<T>` construction and the live backing store

```text
.ctor()             items = new List<T>()
.ctor(IList<T> list)  list == null -> ArgumentNullException
                      items = list          // stored, NOT copied
```

The wrapping constructor stores the caller's list by reference. `Count`,
`Item`, `Contains`, `IndexOf`, `CopyTo` and `GetEnumerator` all read through
`items` on every call, so a later mutation of the supplied list **is** observed
through the collection. Flattening either constructor to a value-semantics
snapshot would discard both the reference identity and the live view.

### Every public mutator routes through a hook

```text
Add(item)            IsReadOnly -> NotSupportedException
                     InsertItem(items.Count, item)
Clear()              IsReadOnly -> NotSupportedException
                     ClearItems()
Insert(index, item)  IsReadOnly -> NotSupportedException
                     index < 0 || index >  Count -> ArgumentOutOfRangeException
                     InsertItem(index, item)
Remove(item)         IsReadOnly -> NotSupportedException
                     index = items.IndexOf(item); index < 0 -> return false
                     RemoveItem(index); return true
RemoveAt(index)      IsReadOnly -> NotSupportedException
                     index < 0 || index >= Count -> ArgumentOutOfRangeException
                     RemoveItem(index)
Item setter          IsReadOnly -> NotSupportedException
                     index < 0 || index >= Count -> ArgumentOutOfRangeException
                     SetItem(index, value)
```

Two details that a plausible-looking reimplementation would get wrong:

- `Insert` accepts `index == Count` (`ble`) while the `Item` setter and
  `RemoveAt` reject it (`blt`). Appending by index is legal; assigning or
  removing one past the end is not;
- `Add` computes `items.Count` **before** calling `InsertItem`, so an override
  that changes the count sees the pre-call index.

The four hooks themselves each delegate straight to `items`:
`ClearItems -> items.Clear()`, `InsertItem -> items.Insert(index, item)`,
`RemoveItem -> items.RemoveAt(index)`, `SetItem -> items[index] = item`.

`CopyTo`, `Contains`, `IndexOf`, `GetEnumerator` and `Count` perform no
validation of their own; they delegate and let the backing list decide.

### `ReadOnlyCollection<T>` is a live view, not a snapshot

```text
.ctor(IList<T> list)  list == null -> ArgumentNullException
                      list = list          // stored, NOT copied
```

Every member reads through the stored reference. `IsReadOnly` is a constant
`true`, and there is no mutation surface at all. Modelling this as an immutable
snapshot would be wrong in the one way that matters: XNA hands out
`ReadOnlyCollection` views over collections it continues to own and change.

### The backing list's own rules

`List<T>` uses unsigned comparisons, so a negative index is caught by the same
branch as an oversized one. `get_Item`, `set_Item` and `RemoveAt` require
`(uint)index < _size`; `Insert` requires `(uint)index <= _size`. Every
mutation bumps `_version`, and `List<T>.Enumerator.MoveNext` throws
`InvalidOperationException` when the version it captured no longer matches —
the exact contract `CNAEnumerator` already projects as
`CNAError.collectionModified`.

## What this milestone deliberately does not decide

Admitted behaviour is not an approved public projection. This milestone
registers an authority and pins a shape; it decides no Swift API. In
particular `System.Exception`, `System.Attribute`, `Dictionary<K,V>` and the
`System.ComponentModel` families are **not** admitted, because nothing
implemented consumes them and each carries its own unresolved public-projection
question. `System.dll` is recorded in the registry as available and
identity-established but deliberately unadmitted, for the same reason.

The registry is shaped to take them later — an authority is a hash-pinned
identity plus a demand-driven list of selected families, and adding a family
adds manifest entries and sentinels without redesigning anything.
