// SPDX-License-Identifier: MIT

// BCL collection support projection.
//
// A CLR class used as the direct base of an XNA class must not silently
// disappear. `Microsoft.Xna.Framework.GameComponentCollection` derives from
// `System.Collections.ObjectModel.Collection<IGameComponent>`, and the four
// `Model*Collection` types derive from `ReadOnlyCollection<T>`; the whole
// usable surface of every one of them is inherited, so dropping the base to
// `Object` would leave the subclass with nothing, and flattening it to a Swift
// `Array` would discard both the reference identity and the live view the CLR
// families are built on.
//
// The rule this file implements is therefore: a CLR BCL base class with a
// registered projection becomes a REAL Swift superclass, and the CLR generic
// argument is preserved in the specialization.
//
//     GameComponentCollection : CNACollection<any IGameComponent>
//
// The three types below live outside `Microsoft.Xna.Framework`. They are
// language- and BCL-support API, not XNA types and not XNA identities, and are
// counted in no XNA scoreboard. Their shape is nevertheless measured, against
// the pinned selected-BCL manifest in
// `tools/api_compat/reference/bcl40-selected-shape.json`, which is
// reconstructed from the hash-registered Microsoft .NET Framework 4.0
// `mscorlib` — see `docs/foundation-26-bcl-authority-evidence.md`.
//
// None of the three conforms to `Sendable`. Two of them are `open`, so a
// subclass anywhere may add mutable stored state and neither this file nor the
// compiler can make a cross-actor promise on its behalf; the third holds
// mutable storage of its own. `mscorlib` makes no thread-safety promise for
// any of them either, and `@unchecked Sendable` would assert exactly the
// guarantee that cannot be earned here.

/// The `EqualityComparer<T>.Default` projection.
///
/// `List<T>.IndexOf` — which every search on both collection families
/// ultimately reaches — compares with `EqualityComparer<T>.Default`. That
/// comparer is resolved by the CLR *at runtime from the element type*: it uses
/// `IEquatable<T>` when the type implements it, otherwise the virtual
/// `Object.Equals`, which is reference identity for a class that does not
/// override it.
///
/// Swift can resolve the same question the same way. A dynamic conformance to
/// `Equatable` is the analogue of `IEquatable<T>` and of an overridden
/// `Object.Equals`, and it is preferred exactly as the CLR prefers them;
/// otherwise two class instances are compared by identity, which is what
/// `Object.Equals` reduces to. The correspondence holds in all four cases that
/// occur:
///
/// | element | CLR | here |
/// |---|---|---|
/// | class, no equality of its own | `Object.Equals` → identity | identity |
/// | class with its own equality | `IEquatable<T>` / overridden `Equals` | `Equatable` |
/// | value type with equality | `EqualityComparer<T>.Default` | `Equatable` |
/// | value type with none | `ValueType.Equals`, field-wise | **not equal** |
///
/// The last row is the one divergence, and it is stated rather than hidden: a
/// Swift value type that does not conform to `Equatable` has no equality for
/// this code to use, and the CLR's reflective field-wise fallback is not
/// reconstructible from it. Every specialization the pinned XNA contract
/// actually declares — `IGameComponent`, `Effect`, `ModelBone`, `ModelMesh`,
/// `ModelMeshPart` — is a reference type, so no projected XNA surface reaches
/// that row.
@inline(__always)
internal func cnaDefaultEquals<Element>(_ first: Element, _ second: Element) -> Bool {
    if let equatable = first as? any Equatable {
        return cnaOpenedEquals(equatable, second)
    }
    // Swift boxes a value type into a *fresh* box on every `as AnyObject`
    // conversion, so this is identity for a class instance and always false
    // for a value type — the divergence documented above, never a silent
    // value comparison.
    return (first as AnyObject) === (second as AnyObject)
}

@inline(__always)
private func cnaOpenedEquals<T: Equatable>(_ first: T, _ second: Any) -> Bool {
    // A CLR `Equals` returns false for an argument of a different type, so a
    // failed downcast is `false` rather than an error.
    guard let other = second as? T else { return false }
    return first == other
}

/// The `System.Collections.Generic.List<T>` projection.
///
/// This is the backing store `Collection<T>..ctor()` actually allocates, and
/// the only thing either collection family wraps. It is a class, because the
/// wrapping constructors of both families store their argument **by reference**
/// and observe every later mutation of it; a Swift `Array` would give the
/// caller a snapshot and break that live relationship.
///
/// Only the `IList<T>` / `ICollection<T>` surface either collection family
/// calls is projected, plus the parameterless constructor and `Add` a caller
/// needs to build a list to wrap. `List<T>`'s own conveniences —
/// `BinarySearch`, `Sort`, `Reverse`, `ConvertAll`, `ForEach`, `FindAll`,
/// `GetRange`, `InsertRange`, `RemoveAll`, `RemoveRange`, `ToArray`,
/// `TrimExcess`, `Capacity` and the rest — are deliberately absent: nothing in
/// the projected surface derives behaviour from them, and admitting BCL
/// behaviour is not the same as approving a public projection of it.
///
/// It is `final`. `mscorlib` leaves `List<T>` unsealed, but nothing in the
/// projected surface derives from it, and sealing it is what makes the backing
/// store's behaviour a fixed measured contract rather than one a subclass could
/// change underneath a `CNACollection` that has already wrapped it.
public final class CNAList<Element> {
    /// `ArgumentOutOfRange_Index`, ExceptionResource 22.
    ///
    /// `List<T>.get_Item`, `set_Item` and `RemoveAt` call the *no-argument*
    /// `ThrowHelper.ThrowArgumentOutOfRangeException()`, whose body pairs
    /// ExceptionArgument 13 (`index`) with this resource — so the paramName is
    /// `"index"` on all three, including the setter whose CLR parameter list
    /// also names a value.
    internal static var indexOutOfRangeMessage: String {
        "Index was out of range. Must be non-negative and less than the size "
        + "of the collection."
    }

    /// `ArgumentOutOfRange_ListInsert`, ExceptionResource 27.
    ///
    /// `Insert` passes a **different** resource from the one above, which is
    /// why an insert past the end and a read past the end do not report the
    /// same message even though both are `ArgumentOutOfRangeException("index")`.
    internal static var listInsertMessage: String {
        "Index must be within the bounds of the List."
    }

    /// `InvalidOperation_EnumFailedVersion`, ExceptionResource 32.
    internal static var enumFailedVersionMessage: String {
        "Collection was modified; enumeration operation may not execute."
    }

    /// `NotSupported_ReadOnlyCollection`, ExceptionResource 28.
    ///
    /// This is what every `Collection<T>` mutator raises for a read-only
    /// backing list. It is **not** `Arg_NotSupportedException` ("Specified
    /// method is not supported."), which is what a parameterless
    /// `NotSupportedException` carries.
    internal static var readOnlyCollectionMessage: String {
        "Collection is read-only."
    }

    private var storage: [Element] = []
    // `List<T>` bumps `_version` on every mutation and its enumerator throws
    // `InvalidOperationException` once the version it captured no longer
    // matches. `CNAEnumerator` projects that as the exact
    // `InvalidOperationException` the CLR raises.
    private var version: UInt64 = 0

    /// `List<T>..ctor()` — an empty list.
    public init() {}

    /// `ICollection<T>.Count`.
    public var Count: Int32 { Int32(storage.count) }

    /// `ICollection<T>.IsReadOnly`. `List<T>` returns the constant `false`.
    public var IsReadOnly: Bool { false }

    /// `IList<T>.this[int]` getter.
    ///
    /// `List<T>.get_Item` compares `(uint)index` against `_size`, so a negative
    /// index and an oversized one are the same `ArgumentOutOfRangeException`.
    public func Item(_ index: Int32) throws -> Element {
        storage[try checkedIndex(index)]
    }

    /// `IList<T>.this[int]` setter. Same bounds rule as the getter.
    public func SetItem(_ index: Int32, _ value: Element) throws {
        let resolved = try checkedIndex(index)
        storage[resolved] = value
        version &+= 1
    }

    /// `ICollection<T>.Add`. Appends; `List<T>.Add` cannot fail.
    public func Add(_ item: Element) {
        storage.append(item)
        version &+= 1
    }

    /// `ICollection<T>.Clear`.
    public func Clear() {
        storage.removeAll(keepingCapacity: true)
        version &+= 1
    }

    /// `ICollection<T>.Contains`.
    public func Contains(_ item: Element) -> Bool { IndexOf(item) >= 0 }

    /// `ICollection<T>.CopyTo`.
    ///
    /// `List<T>.CopyTo` performs no validation of its own; it calls
    /// `Array.Copy(_items, 0, array, arrayIndex, _size)`. The destination is
    /// caller-owned storage, so it is `inout`.
    ///
    /// **The two failure messages here are the only ones in this layer that
    /// are not the CLR's, and the reason is recorded rather than papered
    /// over.** `Array.Copy`'s five-argument public overload forwards to an
    /// `internalcall` six-argument one, so the validation that raises for a
    /// negative index or a short destination lives in CLR *native* code and is
    /// not in the admitted IL at all. The exception CLASSES are the documented
    /// ones and are projected; the messages are not reconstructible, so
    /// neither constructor is given one and `Message` reports each class's own
    /// substituted default. Inventing a sentence and presenting it as the
    /// CLR's would be the fabrication this projection exists to avoid.
    public func CopyTo(_ array: inout [Element], arrayIndex: Int32) throws {
        guard arrayIndex >= 0 else {
            throw CNAArgumentOutOfRangeException(paramName: "arrayIndex")
        }
        let start = Int(arrayIndex)
        guard start <= array.count, storage.count <= array.count - start else {
            throw CNAArgumentException()
        }
        for index in storage.indices { array[start + index] = storage[index] }
    }

    /// `IEnumerable<T>.GetEnumerator`.
    ///
    /// The enumerator captures the current version and refuses to continue once
    /// the list has been mutated, exactly as `List<T>.Enumerator.MoveNext`
    /// raises `InvalidOperationException`.
    public func GetEnumerator() -> CNAEnumerator<Element> {
        CNAEnumerator(expectedVersion: version) { [self] index, expected in
            guard version == expected else {
                throw CNAInvalidOperationException(
                    message: CNAList.enumFailedVersionMessage)
            }
            guard index < storage.count else { return nil }
            return storage[index]
        }
    }

    /// `IList<T>.IndexOf`. The first match under `EqualityComparer<T>.Default`,
    /// or `-1`.
    public func IndexOf(_ item: Element) -> Int32 {
        for (index, element) in storage.enumerated()
        where cnaDefaultEquals(element, item) {
            return Int32(index)
        }
        return -1
    }

    /// `IList<T>.Insert`.
    ///
    /// `List<T>.Insert` compares `(uint)index` against `_size` with `<=`, so
    /// `index == Count` is a legal append and only a greater index — or a
    /// negative one — raises `ArgumentOutOfRangeException`.
    public func Insert(_ index: Int32, item: Element) throws {
        guard index >= 0, Int(index) <= storage.count else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNAList.listInsertMessage)
        }
        storage.insert(item, at: Int(index))
        version &+= 1
    }

    /// `IList<T>.RemoveAt`. `index == Count` is out of range here, unlike
    /// `Insert`.
    public func RemoveAt(_ index: Int32) throws {
        let resolved = try checkedIndex(index)
        storage.remove(at: resolved)
        version &+= 1
    }

    private func checkedIndex(_ index: Int32) throws -> Int {
        guard index >= 0, Int(index) < storage.count else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNAList.indexOutOfRangeMessage)
        }
        return Int(index)
    }
}

/// The `System.Collections.ObjectModel.Collection<T>` projection.
///
/// `mscorlib` declares an unsealed class of generic arity 1 whose base is
/// `System.Object`, with two public constructors, a public surface that is
/// entirely `virtual final` — that is, *sealed* — and exactly four protected
/// virtual hooks. That shape is reproduced here: the public methods are
/// `final`, so the only way a subclass changes behaviour is through
/// `ClearItems`, `InsertItem`, `RemoveItem` and `SetItem`, which is the one
/// extension point the CLR actually offers.
///
/// `Items` is `protected` in the CLR and public here, because Swift has no
/// `protected`. That is a widening, and the only one: nothing else in the
/// class is more visible than `mscorlib` makes it.
///
/// The two `SetItem` members are one CLR distinction, not a collision. The
/// indexed property's writer is the projection of the accessor `set_Item`, and
/// takes the accessor's own `(_, _)` labels; the protected hook is the separate
/// CLR method `SetItem(int32 index, !T item)` and keeps its metadata parameter
/// name, so it takes `(_, item:)`. `mscorlib` declares both, and so does this.
open class CNACollection<Element> {
    // `Collection<T>.items`. Held by reference and never copied: the wrapping
    // constructor's whole purpose is that later mutations of the caller's list
    // are visible through the collection.
    private let items: CNAList<Element>

    /// `Collection<T>..ctor()`.
    ///
    /// The CLR body is `items = new List<T>()`, so the collection owns a fresh
    /// backing store that nothing else can reach.
    public init() {
        self.items = CNAList<Element>()
    }

    /// `Collection<T>..ctor(IList<T> list)`.
    ///
    /// The CLR body stores the argument — it does **not** copy it — so this is
    /// a live wrapper and every later mutation of `list` is observed here. The
    /// CLR also raises `ArgumentNullException` for a null argument; Swift
    /// cannot present nil for a non-Optional class parameter, so that path is
    /// unreachable and the initializer does not throw.
    public init(list: CNAList<Element>) {
        self.items = list
    }

    /// `Collection<T>.Count`. Read through the backing store on every call,
    /// which is what makes a wrapped list's changes visible.
    public final var Count: Int32 { items.Count }

    /// `Collection<T>.Items` — the protected backing-store view. The CLR
    /// getter returns the field itself, so this is the same live object.
    public final var Items: CNAList<Element> { items }

    /// `Collection<T>.this[int]` getter.
    ///
    /// The CLR getter delegates straight to `items[index]` and adds no bounds
    /// check of its own; the backing list raises
    /// `ArgumentOutOfRangeException`.
    public final func Item(_ index: Int32) throws -> Element {
        try items.Item(index)
    }

    /// `Collection<T>.this[int]` setter — the projection of `set_Item`.
    ///
    /// The CLR order is exact and observable: read-only check, then bounds
    /// check, then the `SetItem` hook. An out-of-range index on a collection
    /// whose hook always refuses therefore reports the range, not the refusal.
    public final func SetItem(_ index: Int32, _ value: Element) throws {
        try requireMutable()
        guard index >= 0, index < Count else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNAList<Element>.indexOutOfRangeMessage)
        }
        try SetItem(index, item: value)
    }

    /// `Collection<T>.Add`.
    ///
    /// The CLR reads `items.Count` **before** calling the hook, so an override
    /// receives the index the item had when `Add` was entered.
    public final func Add(_ item: Element) throws {
        try requireMutable()
        let index = items.Count
        try InsertItem(index, item: item)
    }

    /// `Collection<T>.Clear`.
    public final func Clear() throws {
        try requireMutable()
        try ClearItems()
    }

    /// `Collection<T>.Contains`. Delegates to the backing store and cannot
    /// fail.
    public final func Contains(_ item: Element) -> Bool { items.Contains(item) }

    /// `Collection<T>.CopyTo`. Delegates without validating; the backing store
    /// decides.
    public final func CopyTo(_ array: inout [Element], index: Int32) throws {
        try items.CopyTo(&array, arrayIndex: index)
    }

    /// `Collection<T>.GetEnumerator`.
    ///
    /// The CLR returns the *backing store's* enumerator, so enumeration is
    /// invalidated by any mutation of that list — including one made directly
    /// through a wrapped list rather than through this collection.
    public final func GetEnumerator() -> CNAEnumerator<Element> {
        items.GetEnumerator()
    }

    /// `Collection<T>.IndexOf`.
    public final func IndexOf(_ item: Element) -> Int32 { items.IndexOf(item) }

    /// `Collection<T>.Insert`.
    ///
    /// `index == Count` is legal — the CLR compares with `<=` here and with
    /// `<` in the setter and in `RemoveAt`.
    public final func Insert(_ index: Int32, item: Element) throws {
        try requireMutable()
        guard index >= 0, index <= Count else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNAList<Element>.listInsertMessage)
        }
        try InsertItem(index, item: item)
    }

    /// `Collection<T>.Remove`.
    ///
    /// The CLR asks the *backing store* for the index, returns `false` without
    /// calling any hook when the item is absent, and otherwise removes by
    /// index and returns `true`.
    @discardableResult
    public final func Remove(_ item: Element) throws -> Bool {
        try requireMutable()
        let index = items.IndexOf(item)
        guard index >= 0 else { return false }
        try RemoveItem(index)
        return true
    }

    /// `Collection<T>.RemoveAt`. `index == Count` is out of range.
    public final func RemoveAt(_ index: Int32) throws {
        try requireMutable()
        guard index >= 0, index < Count else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNAList<Element>.indexOutOfRangeMessage)
        }
        try RemoveItem(index)
    }

    // ------------------------------------------------------------------
    // The four protected virtual hooks.
    //
    // Every mutating public member above routes through one of these and none
    // of them touches the backing store directly, so an override is always
    // reached. The base implementations are exactly the CLR's: a single
    // delegation to `items`.
    // ------------------------------------------------------------------

    /// `protected virtual void ClearItems()`.
    open func ClearItems() throws {
        items.Clear()
    }

    /// `protected virtual void InsertItem(int index, T item)`.
    open func InsertItem(_ index: Int32, item: Element) throws {
        try items.Insert(index, item: item)
    }

    /// `protected virtual void RemoveItem(int index)`.
    open func RemoveItem(_ index: Int32) throws {
        try items.RemoveAt(index)
    }

    /// `protected virtual void SetItem(int index, T item)`.
    open func SetItem(_ index: Int32, item: Element) throws {
        try items.SetItem(index, item)
    }

    /// The read-only guard every CLR mutator performs first.
    ///
    /// `Collection<T>` wraps an arbitrary `IList<T>`, which may report itself
    /// read-only, and each mutator raises `NotSupportedException` before doing
    /// anything else. The guard is transcribed because it is part of the
    /// member's observable order; with `CNAList` — whose `IsReadOnly` is the
    /// constant `false` that `mscorlib`'s `List<T>` declares — as the only
    /// backing store, it has no reachable path today. The message is
    /// `NotSupported_ReadOnlyCollection` and not the parameterless
    /// constructor's `Arg_NotSupportedException`; the two differ and the
    /// ThrowHelper resource identity settles which.
    private func requireMutable() throws {
        guard !items.IsReadOnly else {
            throw CNANotSupportedException(
                message: CNAList<Element>.readOnlyCollectionMessage)
        }
    }
}

/// The `System.Collections.ObjectModel.ReadOnlyCollection<T>` projection.
///
/// A separate family, not a base or a subclass of `CNACollection`: `mscorlib`
/// declares both classes directly on `System.Object` and neither derives from
/// the other. Modelling `Collection<T>` as a subclass of `ReadOnlyCollection<T>`
/// would give a mutable collection a read-only supertype the CLR never gave it.
///
/// "Read-only" means this class exposes no mutator — **not** that the contents
/// are frozen. The single constructor stores the caller's list by reference,
/// every member reads through it, and XNA hands out these views over
/// collections it continues to own and change. An immutable snapshot would be
/// wrong in precisely the way that matters.
open class CNAReadOnlyCollection<Element> {
    private let list: CNAList<Element>

    /// `ReadOnlyCollection<T>..ctor(IList<T> list)` — the only constructor.
    ///
    /// A read-only collection is always a view *of* something, so `mscorlib`
    /// declares no parameterless constructor and neither does this. The CLR
    /// raises `ArgumentNullException` for a null argument; a non-Optional Swift
    /// class parameter cannot be nil, so that path is unreachable and the
    /// initializer does not throw.
    public init(list: CNAList<Element>) {
        self.list = list
    }

    /// `ReadOnlyCollection<T>.Count`, read live.
    public final var Count: Int32 { list.Count }

    /// `ReadOnlyCollection<T>.Items` — the protected backing-store view, public
    /// here only because Swift has no `protected`.
    public final var Items: CNAList<Element> { list }

    /// `ReadOnlyCollection<T>.this[int]` — get-only. There is no setter to
    /// project.
    public final func Item(_ index: Int32) throws -> Element {
        try list.Item(index)
    }

    /// `ReadOnlyCollection<T>.Contains`.
    public final func Contains(_ value: Element) -> Bool { list.Contains(value) }

    /// `ReadOnlyCollection<T>.CopyTo`.
    public final func CopyTo(_ array: inout [Element], index: Int32) throws {
        try list.CopyTo(&array, arrayIndex: index)
    }

    /// `ReadOnlyCollection<T>.GetEnumerator`, over the live backing store.
    public final func GetEnumerator() -> CNAEnumerator<Element> {
        list.GetEnumerator()
    }

    /// `ReadOnlyCollection<T>.IndexOf`.
    public final func IndexOf(_ value: Element) -> Int32 { list.IndexOf(value) }
}
