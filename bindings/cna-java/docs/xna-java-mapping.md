# Normative XNA 4.0 to Java mapping

This document defines the public metadata transformation used by CNA-Java. The authority on the
left is legally obtained Microsoft XNA Framework 4.0 CLR metadata. CNA headers and CNA-C# are
implementation references, not API authorities. Changes to these rules are compatibility changes
and must change `tools/api-compat/mapping-rules.json` in the same commit.

## Identity and casing

An XNA namespace becomes the identically cased Java package and a non-nested CLR type keeps its
name after removing CLR generic-arity suffixes. Nested CLR types become Java nested types. XNA
identifier spelling and casing is retained whenever Java syntax permits it. Consequently ordinary
methods remain `Run`, `Begin`, `Draw`, `Clear`, `GetState`, `CreateScale`, and so on. Java keywords
are escaped by appending `Value`; the mapping report emits `XNA_MAPPING_MISMATCH` unless the escape
is declared by the rules file.

CLR permits a generic and non-generic type with the same base name while Java does not. The
non-generic type keeps the XNA name and the colliding generic type deterministically receives an
`OfT` suffix (currently `ContentTypeReaderOfT<T>` and `IPackedVectorOfT<T>`). Every collision and
rename is explicit in `mapping-rules.json`; silently collapsing two CLR types is forbidden.

`ContentReader` generic read methods receive a leading `Class<T> targetType` token, by the same
rule used for `ContentManager.Load`. Java erases method type parameters, while XNA uses `typeof(T)`
for raw-reader selection, result checking, shared-resource fixups, and external references; the
token preserves those semantics and avoids untyped `Object` APIs. CLR's object dispatch bridge and
strongly typed `ContentTypeReader<T>.Read(ContentReader,T)` erase to the same Java descriptor, so
the bridge retains `Read` and the typed user extension point is deterministically named
`ReadTyped`. Both members remain in the strict contract; only the colliding name is adapted.

Public XNA fields remain public Java fields with the same names. Ordinary enum types become Java
enums and their members keep XNA spelling (`Keys.Escape`). CLR numeric values use Java ordinals
when they are exactly sequential; otherwise the enum receives `getValue()` and compiled-metadata
inspection verifies every named number.

CLR `[Flags]` enums cannot be Java enums because Java enum instances cannot represent unnamed bit
combinations. They therefore become final immutable value classes with same-cased named constants,
`getValue()`, `FromValue(value)`, `Or(other)`, and `Contains(value)`. Equality and hashing use the
underlying bit set. Thus `SpriteEffects.None` remains recognizable while a combined flags value is
representable without an invalid pseudo-enum constant. The CLR extractor records `FlagsAttribute`
directly; this transformation is not inferred from names.

An ordinary CLR enum remains a Java enum. Java cannot reproduce C# casts that manufacture an
unnamed numeric enum value. Numeric values of all declared constants are preserved and verified;
an API that can observe undeclared native values must specify its deterministic adaptation. For
`KeyboardState`, all 256 native bits remain part of snapshot equality and hashing, while
`GetPressedKeys()` can return only the 160 declared `Keys` constants. This is a recorded Java
language limitation, not permission to renumber or invent enum members.

## Properties and indexers

Instance and static properties use one rule:

```text
Foo { get; }       -> getFoo()
Foo { get; set; }  -> getFoo(), setFoo(value)
IsMouseVisible     -> getIsMouseVisible(), setIsMouseVisible(value)
```

Accessor visibility is mapped independently. Internal/private accessors do not become public.
Indexer properties become `get(index...)` and, when settable, `set(index..., value)`. A get-only
static value property may instead have a same-cased named field when its declaring value type is
explicitly listed in `mapping-rules.json`. All XNA named `Color` properties use this rule uniformly,
so they project as `Color.AliceBlue`, `Color.Red`, and `Color.White`, not a mixture of fields and
getters. The shared Java instances are frozen because mutating a public static object would corrupt
future reads; callers use `new Color(Color.Red)` when they need the mutable copy that CLR property
value semantics would have supplied. The verifier never infers this transformation ad hoc.

## Methods, operators, overloads, and defaults

Methods preserve XNA spelling, overloads, generic arity, static/abstract state, overridability,
parameter order, and mapped types. A CLR non-virtual instance method is a Java `final` method when
its declaring class remains extensible. In a Java `final` class, the class modifier already makes
every instance method non-overridable, so a redundant method-level `final` flag is not required and
the verifier compares effective overridability. Static methods and fields retain their literal
modifier state. Java has no optional-parameter metadata equivalent, so each callable XNA arity
becomes an overload. Default values are recorded in the neutral contract even when they do not
change the Java descriptor.

An operator is mapped to the identically purposed named XNA method (`op_Addition` to `Add`,
`op_Multiply` to `Multiply`, and so on) and is deduplicated when that method already exists.
Equality operators map to `equals(Object)`/`hashCode()`. Conversion operators map to explicitly
declared `from<Type>` static factories or `to<Type>` instance methods. The first such rule maps
`RenderTargetBinding.op_Implicit(RenderTarget2D)` to `fromRenderTarget2D(RenderTarget2D)`. An
operator with no full-signature rule is a hard `XNA_MAPPING_MISMATCH`, not a guessed API.

The redundant XNA performance overload pattern whose only difference is `ref` inputs plus one
`out` result maps to the ordinary return-value overload and is deduplicated. Other `ref`/`out`
parameters map to `org.openeggbert.cna.extensions.Ref<T>` and `Out<T>` only when an explicit rule
marks the transformation. The strict packages never expose these extension holders without that
rule.

`Matrix.Decompose(out scale, out rotation, out translation)` is the first explicit multi-output
rule: Java receives `Matrix.Decomposition Decompose()`. Its immutable carrier exposes
`getSucceeded()`, `getScale()`, `getRotation()`, and `getTranslation()`; mutable value results are
returned as snapshots. XNA `ContentManager.Load<T>(name)` receives the class-token parameter
required by erasure: `Load(Class<T> assetType, String assetName)`.

The touch value projection has two reviewed single-output adaptations where the CLR Boolean return
and `out` value must remain observable together. `TouchLocation.TryGetPreviousLocation(out value)`
returns an immutable `PreviousLocationResult`, and `TouchCollection.FindById(id, out value)` returns
an immutable `FindResult`. Each carrier exposes `getSucceeded()` and a snapshot getter for its touch
location. These signatures are declared in `refOutMemberMappings`; other unclassified `ref`/`out`
members remain verifier failures.

The Model name collections have a narrower reviewed single-output rule. XNA's
`TryGetValue(name, out value)` maps to `TryGetValue(name)` returning the matching ModelBone or
ModelMesh, and returns `null` when the Boolean/out pair would report failure. This mapping is listed
by full CLR signature; it does not establish a general rule for discarding Boolean results.

`GraphicsAdapter.QueryBackBufferFormat` and `QueryRenderTargetFormat` have three `out` values in
addition to their Boolean exact-match result. Java returns an immutable `FormatSelectionResult`
carrying `getExactMatch()`, the selected surface and depth formats, and the selected multisample
count. `DisplayModeCollection` retains `GetEnumerator()` and also supplies the required Java
`Iterable<DisplayMode>.iterator()` bridge.

The four Effect reflection collections follow the same explicit bridge rule. They retain XNA's
concrete `GetEnumerator()` member and also declare `iterator()` so their mapped Java `Iterable`
contracts are actual interfaces rather than metadata-only claims.

The four Model collection enumerator value types likewise retain `MoveNext()`, `getCurrent()`, and
their no-op mapped `close()` member while implementing Java Iterator through explicit `hasNext()`
and `next()` bridges. The bridge does not make the read-only owning collection mutable.

A public parameterless CLR `Dispose()` keeps its XNA name as `Dispose()`, with the CLR member's
own accessibility and overridability. Java attaches no reserved meaning to that identifier, so the
project's identity rule -- an XNA member keeps its spelling whenever Java syntax permits -- applies
to it like any other method. Every disposable type additionally declares `public final void
close()`, which delegates to `Dispose()`; the two names are one operation, and `close()` exists so
the type is a real `java.lang.AutoCloseable` and works in try-with-resources.

The `close()` bridge is synthesized for a type that reaches `IDisposable` directly and for one that
reaches it through `IEnumerator<T>`. It is concrete and non-overridable for a class, because XNA's
own `Dispose()` is `virtual final`, and abstract only on an interface. When CLR implements
`IDisposable.Dispose` explicitly rather than publicly -- `GraphicsDeviceManager` is the one such
type -- there is no public `Dispose()` in the CLR contract either, so Java projects `close()` alone.

A distinct protected `Dispose(boolean)` lifetime hook keeps its XNA name as `Dispose(boolean)` and
remains the overridable subclass extension point. CLR finalization is not projected: Java explicit
cleanup is normative and deprecated Java finalization must not be added.

`Equals`, `GetHashCode` and `ToString` are the reviewed exception to the identity rule and are
lowered to `equals`, `hashCode` and `toString`. `java.lang.Object` already declares all three, so
an XNA-cased twin would not override them: collections, hashing and printing would silently use
the inherited implementation instead. Java syntax does not permit both spellings to mean one
member, so the Java contract wins here and only here.

## CLR and framework types

The core deterministic mappings are:

```text
System.Boolean/SByte/Int16/Int32/Int64/Single/Double/Char -> Java primitives
System.Byte                                              -> int (validated 0 through 255)
System.String                                            -> java.lang.String
System.Object                                            -> java.lang.Object
System.Uri                                               -> java.net.URI
System.DateTime                                          -> java.time.Instant
System.Exception                                         -> java.lang.RuntimeException
System.Runtime.InteropServices.ExternalException         -> java.lang.RuntimeException
System.Text.StringBuilder                                -> java.lang.StringBuilder
System.Type                                              -> java.lang.Class<?>
System.IntPtr                                            -> Microsoft.Xna.Framework.WindowHandle
System.TimeSpan                                          -> java.time.Duration (100 ns precision)
System.IDisposable                                       -> java.lang.AutoCloseable
                                                            (Dispose() retained, close() bridges)
System.Collections.Generic.IEnumerable<T>                -> java.lang.Iterable<T>
System.Collections.Generic.IEnumerator<T>                -> java.util.Iterator<T>
System.Collections.Generic.ICollection<T>                -> java.util.Collection<T>
System.Collections.Generic.IList<T>                      -> java.util.List<T>
System.Nullable<T>                                       -> boxed T or Optional<T>, by explicit rule
System.EventArgs                                         -> Microsoft.Xna.Framework.EventArgs
System.EventHandler<T>                                   -> Microsoft.Xna.Framework.EventHandler<T>
System.IServiceProvider                                  -> Microsoft.Xna.Framework.ServiceProvider
System.IAsyncResult                                      -> System.IAsyncResult
System.AsyncCallback                                     -> System.AsyncCallback
System.IO.Stream (Storage read/write/seek results)       -> System.IO.Stream
System.IO.FileMode/FileAccess/FileShare/SeekOrigin       -> same-named System.IO compatibility values
System.Collections.ObjectModel.Collection<T>             -> java.util.AbstractList<T>
System.Collections.ObjectModel.ReadOnlyCollection<T>     -> java.util.List<T>
System.Collections.Generic.Dictionary<K,V>               -> java.util.LinkedHashMap<K,V>
```

When an XNA concrete type directly implements `ICollection<T>`, it retains its mapped XNA-named
members and also implements the required lower-cased `java.util.Collection<T>` bridge. Those Java
bridge members are explicit expected-contract additions, not verifier-ignored extras. The first
such type is `CurveKeyCollection`; both mutation paths maintain the same ordering and cache state.
`TouchCollection` similarly implements the full `List<TouchLocation>` bridge while retaining its
XNA-named members. Its indexer setter maps to Java's value-returning `List.set` signature and still
throws because XNA marks the collection read-only. The nested XNA enumerator adds `hasNext()` and
`next()` as explicit bridge obligations alongside `MoveNext()` and `getCurrent()`.

At a `TimeSpan` API boundary, `Duration` is normalized downward to the nearest
100-nanosecond CLR tick; a value outside the signed `TimeSpan` tick range is rejected.
Individual XNA properties retain their own range rules (for example, positive target elapsed time
and non-negative inactive sleep time). `RuntimeException` is used for CLR `Exception` because a
checked Java base exception would introduce call-site obligations absent from the CLR contract.
`ExternalException` uses the same unchecked Java base: its platform/native-error distinction is
preserved by the concrete mapped XNA exception type and constructors, while adding a checked Java
supertype would change every caller. This is an explicit base-type mapping, not an allowlist.
Unsigned CLR `Byte` deliberately does not become signed Java `byte`: values 128 through 255 must
remain numerically observable, so it projects to `int` and setters/constructors validate the XNA
0-through-255 domain. This is the same width-preserving policy used for other unsigned CLR values
whose full range a same-width Java primitive cannot represent.
The protected `ContentLoadException(SerializationInfo, StreamingContext)` and
`StorageDeviceNotConnectedException(SerializationInfo, StreamingContext)` constructors have no
Java source or serialization-protocol equivalent and are explicitly excluded by their full CLR
signatures.
Java exception serialization instead uses `RuntimeException`'s serial form and `serialVersionUID`;
the three ordinary public constructors remain part of the strict mapped contract. This is a
mapping rule, not an allowlist entry.
CLR parameter metadata without a name deterministically maps to `argN`, where `N` is its zero-based
position. An inaccessible CLR interface implemented by a public XNA type is omitted unless the
interface itself belongs to the selected reference profile. CLR `Stream` is direction-sensitive
rather than forced onto one misleading Java type: `Texture2D.FromStream` maps its input to
`InputStream`, `ContentManager.OpenStream` and `TitleContainer.OpenStream` map their returned
readable streams to `InputStream`, `Album.GetAlbumArt`, `Album.GetThumbnail`, `Picture.GetImage`,
and `Picture.GetThumbnail` likewise return `InputStream`, and `MediaLibrary.SavePicture` accepts an
`InputStream`. `SaveAsPng` and `SaveAsJpeg` map their output to `OutputStream`. These
full-signature transformations are recorded in `mapping-rules.json`.
Storage is the bidirectional exception to the direction-only stream mappings: its public methods
return the same `System.IO.Stream` compatibility type because callers must observe read/write/seek
capabilities, position, length, and the selected `FileMode`, `FileAccess`, `FileShare`, and
`SeekOrigin` identities. XNA's storage `Begin` methods retain their fake-async shape through
`System.IAsyncResult` and `System.AsyncCallback`; their result is already completed and invokes a
non-null callback synchronously, while creation occurs at the corresponding `End` method.
`WindowHandle` is an opaque value that supports equality and a zero test but intentionally has no
numeric/address accessor. It preserves the XNA window-token round trip without exposing a raw
native address to game code; CNA-specific native-window interop belongs in the extensions layer.

A CLR explicit implementation of an interface in the selected profile is private on its declaring
type, but Java has no explicit-interface-member syntax. The extractor records those interface-map
entries and the Java projection promotes each one to a public method, retaining its effective
non-overridability. This is an added Java obligation, not an omitted CLR member. When a primitive
`PackedValue` property also implements generic `IPackedVector<T>`, its Java getter and setter use
the corresponding boxed type so the declaring value type can actually implement
`IPackedVectorOfT<T>` after erasure. The boxed generic-property rule is explicit in
`mapping-rules.json`; ordinary non-generic primitive properties remain primitive.

XNA collections that are fixed-size or read-only use dedicated facade types or unmodifiable Java
views; mapping to `List<T>` does not grant mutation that XNA refused. Generic bounds are preserved
where the Java type system can express them. CLR attributes with behavioral or contract meaning
map to annotations listed in the rules file; other attributes remain recorded as unmapped
diagnostics until reviewed.

When a public XNA concrete type derives from `ReadOnlyCollection<T>`, Java cannot extend the
`List<T>` interface as a class. Such concrete facades use the explicitly recorded
`AbstractList<T>` base mapping while members that merely return `ReadOnlyCollection<T>` continue to
return `List<T>`. The four Model collection facades are the first reviewed instances of this rule.

The seven XNA Media collection facades directly implement `IEnumerable<T>`. They retain the
concrete XNA `GetEnumerator()` member and add the lower-cased `iterator()` bridge required by
Java's `Iterable<T>` contract. This adds seven expected Java members; it does not make the
collections mutable. Their XNA read-only, index, ordering, and disposal behavior remains in the
named facade types.

## Design-time converter projection

Java has no counterpart to the CLR `System.ComponentModel.TypeConverter` service graph. The XNA
Design family therefore uses one compact language projection rather than a synthetic
`System.ComponentModel` namespace:

```text
ExpandableObjectConverter / TypeConverter base -> java.lang.Object; the XNA-declared
                                                MathTypeConverter surface is the Java contract
System.Type                                    -> java.lang.Class<?>
System.Globalization.CultureInfo               -> java.util.Locale
ITypeDescriptorContext                         -> omitted parameter
PropertyDescriptorCollection (protected state) -> LinkedHashMap<String, Class<?>>
GetProperties(..., value, ...)                 -> LinkedHashMap<String, Object>
InstanceDescriptor                             -> java.beans.Expression
System.Collections.IDictionary                 -> Map<String, Object>
System.Attribute[] property filter             -> omitted parameter
```

`MathTypeConverter` remains public, concrete, and directly constructible exactly as XNA metadata
declares. It is the Java converter base contract; inherited CLR ComponentModel methods that XNA
does not declare are not fictionalized. The context parameter is absent because every XNA Design
converter only forwards it to CLR primitive conversion services and Java has no ambient component
site/container transaction with equivalent meaning. The attribute-filter parameter is likewise
absent because the XNA implementation ignores it. These are deterministic parameter omissions in
`mapping-rules.json`, not verifier exceptions.

Java property decomposition is value-oriented. The protected ordered metadata map records each
stable XNA property name and declared Java component class. `GetProperties(value)` returns a new
ordered mutable map of those names to snapshotted component values. Callers may edit that map and
pass it to `CreateInstance`; this replaces CLR descriptor objects, their collection, and their
boxing-based get/set machinery without adding a fake descriptor class. Required keys and exact
boxed Java value types follow the XNA constructor contract, while unrelated extra keys are ignored
as XNA's dictionary lookups do. Matrix retains XNA's observable 17-property order (`Translation`
then `M11` through `M44`), although reconstruction reads only the 16 scalar constructor keys.

`java.beans.Expression` is the standard-library reconstruction description corresponding to an
XNA `InstanceDescriptor`: the target is the mapped value class, the method name is `new`, and the
arguments are constructor snapshots in XNA order. Calling `getValue()` reconstructs the value.
There is no Java equivalent for ComponentModel designer hosts, services, change notifications,
reset/serialization policy, or attribute filtering. Those unsupported design-host concepts are
represented by the two explicit parameter omissions and by value decomposition; they do not add
types, weaken comparison, or claim a CLR designer environment exists.

Locale-sensitive component strings use the locale decimal separator and a semicolon list
separator when the decimal separator is a comma, otherwise a comma. Formatting appends one ASCII
space after each list separator, matching XNA. Floating components use XNA/CLR `Single` general
format semantics (seven significant digits, XNA NaN/infinity tokens, and signed zero formatted as
zero), not Java `Float.toString()` by accident. A null locale selects `Locale.getDefault()` as the
Java counterpart to CLR current culture. XNA's non-string converters still allow conversion to
`String` through the base fallback (`value.toString()`), but do not accept strings as input.

## Delegates and events

An XNA delegate becomes a same-package `@FunctionalInterface`; its `Invoke` signature becomes
`invoke`. The standard CLR `EventHandler<TEventArgs>` delegate maps once to the synthetic,
machine-declared compatibility interface `Microsoft.Xna.Framework.EventHandler<TEventArgs>`.
`System.EventArgs` similarly maps to the small `Microsoft.Xna.Framework.EventArgs` compatibility
value. These synthetic Java necessities are exact contracts in `mapping-rules.json`, not
allowlisted unexpected types.

An event `Foo` maps to `addFooListener(EventHandler<TEventArgs>)` and
`removeFooListener(EventHandler<TEventArgs>)`. Listener invocation order is registration order,
duplicate registrations remain duplicate, removal removes one matching registration, and listener
mutation during dispatch observes a stable snapshot. Native callback pointers and contexts are
always hidden below `org.openeggbert.cna.internal`.

## Value types and copying

CLR structs become value-oriented Java classes. Mutable public fields stay mutable where required,
but Java assignment aliases an object instead of copying a struct. This unavoidable difference is
never described as source-semantic identity. Constructors, collection insertion, retained
properties, and every managed/native boundary snapshot mutable values so later caller mutation
cannot alter the already-passed value. Value classes implement XNA-aware `equals` and `hashCode`;
floating-point equality treats NaNs and signed zero as CLR `Single.Equals` does.

Get-only static struct properties return fresh objects when their Java representation is mutable.
Immutable named values may be public same-cased constants. Native handles, memory addresses, JNI
types, and implementation adapters are not part of a value type's public contract.

## Lifetime and unsupported behavior

`IDisposable` maps to `AutoCloseable`; `Dispose()` and its `close()` bridge are idempotent for
CNA-owned resources. Owned,
borrowed, parent-owned, and adopted handles are distinct internal states. Explicit cleanup is the
contract; no Java finalizer is used. A Cleaner may only be a safety net.

An API may be shape-present while returning a documented deterministic unsupported exception.
That is implementation coverage, not API-contract completeness, and is tracked separately from
metadata. CNA-specific renderer diagnostics and capabilities live under
`org.openeggbert.cna.extensions`, never under `Microsoft.Xna.Framework.*`.
