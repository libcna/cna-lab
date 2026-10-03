# The XNA-era Microsoft BCL is available — provenance record

Every remaining CNA-Swift frontier that is not a runtime, a piece of hardware,
or a value the CLR leaves unspecified is blocked on a BCL mapping decision, and
three of those decisions were additionally blocked on a **missing input**: no
XNA-era Microsoft `mscorlib` was known to be present, and Mono's
reimplementation and the .NET 8 facades are not Microsoft's XNA-era behaviour.

That input is no longer missing. This document records what was found and
establishes its identity, so that registering it is a mechanical step rather
than a new provenance judgement. **Nothing here has been registered and nothing
here has been used**: no mapping was decided, `registered-assemblies.json` is
unchanged, and no verdict in any pinned reference derives from these binaries.

## What was found

A complete .NET Framework 4.0 installation inside a Wine prefix on this
machine, whose global assembly cache also contains the XNA Game Studio 4.0
assemblies — an XNA-era environment, not a modern one.

```text
prefix   ~/.wine-cna-xna40/drive_c/windows/Microsoft.NET
```

### `mscorlib`

```text
path       assembly/GAC_32/mscorlib/v4.0_4.0.0.0__b77a5c561934e089/mscorlib.dll
size       5196112
modified   2010-03-18
sha256     5634668d4775b0113f08ea31093b281fea69bfc4e99227f5ca761b4ed98acc63
```

Identity, read from the assembly's own metadata:

```text
.assembly mscorlib
  .publickey = (00 00 00 00 00 00 00 00 04 00 00 00 00 00 00 00)
  .hash algorithm 0x00008004                      // SHA-1
  .ver 4:0:0:0
.module CommonLanguageRuntimeLibrary
// MVID: {98BD4ED7-7337-4018-9551-EE0825ADA7BA}
```

The `.publickey` is the ECMA standard key whose token is `b77a5c561934e089`,
which is exactly the token in the GAC directory name — the two agree, so the
binary is the one the CLR would bind for that identity.

PE version resource:

```text
CompanyName       Microsoft Corporation
FileDescription   Microsoft Common Language Runtime Class Library
FileVersion       4.0.30319.1 (RTMRel.030319-0100)
ProductName       Microsoft® .NET Framework
ProductVersion    4.0.30319.1
OriginalFilename  mscorlib.dll
LegalCopyright    © Microsoft Corporation.  All rights reserved.
```

Two further facts rule out a reimplementation:

- the strong-name key attribute records Microsoft's internal build path,
  `f:\dd\Tools\devdiv\ecmapublickey.snk` (`dd` is Microsoft's Dev Division
  share);
- the module imports `clr.dll` and `mscoree.dll` — the Microsoft CLR — and the
  disassembly contains **zero** `Mono.` type references and no `mono_` entry
  point.

`4.0.30319.1` is the .NET Framework 4.0 **RTM** build, released April 2010.
XNA Game Studio 4.0 targets .NET Framework 4.0, and the seven registered XNA
assemblies in this repository are from the same era, so this is the matching
BCL rather than a later in-place servicing update.

### `System.dll`

The `MathTypeConverter` frontier needs `System.ComponentModel`, which lives in
`System.dll` rather than `mscorlib`. The same prefix carries it:

```text
path       assembly/GAC_MSIL/System/v4.0_4.0.0.0__b77a5c561934e089/System.dll
size       3481928
modified   2010-03-18
sha256     c3182e40f09a8d3a0167a833dc1ce7c3cb2bfddbd32031d8d3f41481d0467462
.ver 4:0:0:0   .publickeytoken = (B7 7A 5C 56 19 34 E0 89)
FileVersion    4.0.30319.1
```

It declares `System.ComponentModel.TypeConverter` and
`ExpandableObjectConverter`.

## What it would make available

Reconstructing the public shape with the repository's own
`pinned_assembly_audit.Parser` yields 1,406 public types from `mscorlib`,
including every BCL type the still-deferred frontiers name except the
`System.ComponentModel` ones, which `System.dll` supplies:

| Frontier | BCL type | Assembly |
|---|---|---|
| `GameComponentCollection` | `System.Collections.ObjectModel.Collection\`1` | mscorlib |
| `GraphicsAdapter`, `Microphone`, `SpriteFont`, `VisualizationData` | `System.Collections.ObjectModel.ReadOnlyCollection\`1` | mscorlib |
| the eight exception types | `System.Exception`, `System.Runtime.InteropServices.ExternalException` | mscorlib |
| the five `ContentSerializer*Attribute` types | `System.Attribute` | mscorlib |
| `GameServiceContainer`, `ContentManager` | `System.Type`, `System.IServiceProvider`, `System.Action\`1` | mscorlib |
| `LaunchParameters` | `System.Collections.Generic.Dictionary\`2` | mscorlib |
| `SpriteFont` | `System.Text.StringBuilder` | mscorlib |
| the two serialization constructors | `System.Runtime.Serialization.SerializationInfo`, `StreamingContext` | mscorlib |
| `MathTypeConverter` | `System.ComponentModel.TypeConverter`, `ExpandableObjectConverter`, `ITypeDescriptorContext`, `PropertyDescriptorCollection` | System.dll |

### The shape the collection decision needs

`GameComponentCollection` declares only a constructor, four protected overrides
and two events; everything usable is inherited. That inherited surface, read
from the authoritative binary, is:

```text
System.Collections.ObjectModel.Collection`1<T> : System.Object
  implements IList<T>, ICollection<T>, IEnumerable<T>, IList, ICollection, IEnumerable
  public    .ctor()                          // items = new List<T>()
  public    .ctor(IList<T> list)             // wraps the caller's list live
  public    Count      : Int32   { get }
  public    Item       : T       { get set }
  protected Items      : IList<T> { get }
  public    Add(T), Clear(), Contains(T) -> Bool, CopyTo(T[], Int32),
            GetEnumerator() -> IEnumerator<T>, IndexOf(T) -> Int32,
            Insert(Int32, T), Remove(T) -> Bool, RemoveAt(Int32)
  protected virtual ClearItems(), InsertItem(Int32, T),
                    RemoveItem(Int32), SetItem(Int32, T)

System.Collections.ObjectModel.ReadOnlyCollection`1<T> : System.Object
  implements the same six interfaces
  public    .ctor(IList<T> list)             // wraps the caller's list live
  public    Count      : Int32   { get }
  public    Item       : T       { get }
  protected Items      : IList<T> { get }
  public    Contains(T) -> Bool, CopyTo(T[], Int32),
            GetEnumerator() -> IEnumerator<T>, IndexOf(T) -> Int32
```

The four protected virtuals are precisely the hooks `GameComponentCollection`
overrides, and `Collection<T>..ctor()` allocating a `List<T>` while
`.ctor(IList<T>)` wraps the caller's list live is the behaviour that makes
flattening either type to a Swift `Array` wrong: an `Array` would discard both
the reference identity and the live view.

## What is still undecided

Everything that matters. This document records an **input**, not a decision:

- how a CLR *class* used as a base — `Collection<T>`, `ReadOnlyCollection<T>`,
  `Dictionary<K,V>`, `Exception`, `Attribute`, `TypeConverter` — projects into
  Swift, and whether the inherited members become declared Swift members of the
  derived XNA type;
- whether an assembly that declares **no** contract type can be registered as a
  behaviour authority at all. The current registration gate in
  `pinned_assembly_audit.py` earns authority by machine-comparing an assembly's
  public metadata against every contract entry it declares; `mscorlib` declares
  none, so that gate would pass vacuously and calibrate nothing. A BCL
  authority needs its own calibration rule before it can be registered
  honestly.

Both are public API/architecture decisions, and both are outside what this
session was authorised to decide. The blocker has changed from *"the exact
XNA-era Microsoft mscorlib is not available"* to *"the mapping and the
registration rule are undecided"*, which is a materially better place to stand.
