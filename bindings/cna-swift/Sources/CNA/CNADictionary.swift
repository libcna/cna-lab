// SPDX-License-Identifier: MIT

// BCL dictionary support projection.
//
// `Microsoft.Xna.Framework.LaunchParameters` derives from
// `System.Collections.Generic.Dictionary<string, string>` and declares nothing
// but a parameterless constructor, so — exactly as with
// `GameComponentCollection` and the eight exception types — its entire usable
// surface is inherited and the base cannot be dropped. The Foundation 27 rule
// applies unchanged:
//
//     System.Collections.Generic.Dictionary`2   ->  CNADictionary<Key, Value>
//     System.Collections.Generic.IEqualityComparer`1
//                                              ->  CNAEqualityComparer
//     System.Collections.Generic.KeyValuePair`2 ->  CNAKeyValuePair<Key, Value>
//     Dictionary`2+KeyCollection                ->  CNADictionary.KeyCollection
//     Dictionary`2+ValueCollection              ->  CNADictionary.ValueCollection
//
// This is a REFERENCE class, not a Swift `Dictionary`. `LaunchParameters` is a
// CLR object with identity that the `Game` hands out and continues to own; a
// Swift `Dictionary` value would give every caller a copy, and the whole
// point of the type is that a caller reads the launch data the game already
// holds. Beyond identity, a Swift `Dictionary` cannot express any of:
//
//   - `Add` refusing a duplicate key while the indexer setter overwrites it;
//   - enumeration in a defined order — the CLR's is entry order, which is
//     insertion order with LIFO reuse of removed slots;
//   - the version counter that invalidates a live enumerator, including on an
//     overwrite through the indexer;
//   - a caller-supplied `IEqualityComparer<TKey>` deciding key identity.
//
// So the CLR's own storage is reproduced rather than approximated: a bucket
// array of chain heads, an entry array carrying `hashCode`, `next`, `key` and
// `value`, a free list, and a version counter. Every one of the behaviours
// above then falls out of the algorithm instead of being simulated. See
// `docs/foundation-31-bcl-dictionary-projection-evidence.md`.
//
// These types live outside `Microsoft.Xna.Framework`, are counted in no XNA
// scoreboard, and no `::System` namespace is fabricated for them. None
// conforms to `Sendable`: `CNADictionary` is `open` over mutable storage and
// `mscorlib` makes no thread-safety promise for it either.

/// The `System.Collections.Generic.IEqualityComparer<T>` projection.
///
/// `Dictionary<TKey,TValue>` asks this — never Swift's `Hashable` — for both
/// halves of key identity, so a caller-supplied comparer decides which keys
/// collide and which are equal, exactly as in the CLR.
public protocol CNAEqualityComparer<Element> {
    associatedtype Element

    /// `bool Equals(T x, T y)`.
    func Equals(_ x: Element, _ y: Element) -> Bool

    /// `int GetHashCode(T obj)`.
    ///
    /// The value only has to be consistent with `Equals`; nothing in the
    /// projected dictionary surface exposes it further. See
    /// `CNADefaultEqualityComparer` for the one divergence this creates.
    func GetHashCode(_ obj: Element) -> Int32
}

/// The behaviour of `EqualityComparer<T>.Default`, which
/// `Dictionary<TKey,TValue>`'s constructors store when no comparer is
/// supplied.
///
/// It is `internal` on purpose. `Comparer`'s declared CLR type is
/// `IEqualityComparer<TKey>`, and the runtime type behind
/// `EqualityComparer<T>.Default` is an internal `mscorlib` class, so
/// projecting a public concrete class here would invent public API the CLR
/// does not offer. A consumer sees `any CNAEqualityComparer<Key>`, which is
/// exactly what a CLR consumer sees.
///
/// **`Equals` is exact; `GetHashCode` is consistent but not the CLR's number.**
/// The CLR resolves `EqualityComparer<T>.Default` from the element type at
/// runtime, preferring `IEquatable<T>` and otherwise using the virtual
/// `Object.Equals`; `cnaDefaultEquals` resolves it the same way and is
/// documented in `CNACollections.swift`. The hash is a different matter:
/// `EqualityComparer<T>.Default.GetHashCode(x)` is `x.GetHashCode()`, whose
/// value Microsoft documents as unspecified, version-dependent and unsafe to
/// persist, and which .NET later randomised per process. Reproducing a number
/// the platform itself refuses to guarantee would be false precision, so a
/// Swift-derived hash is used instead.
///
/// **Nothing observable in `CNADictionary` depends on that number**, which is
/// why the divergence is narrow enough to state rather than block on:
/// enumeration order is entry-array order and never hash order, `Count` is a
/// counter, and lookup needs only that equal keys hash equally — which holds.
/// A test walks a dictionary in insertion order to prove it.
internal final class CNADefaultEqualityComparer<Element>: CNAEqualityComparer {
    func Equals(_ x: Element, _ y: Element) -> Bool {
        cnaDefaultEquals(x, y)
    }

    func GetHashCode(_ obj: Element) -> Int32 {
        if let hashable = obj as? any Hashable {
            return Int32(truncatingIfNeeded: hashable.hashValue)
        }
        // The `cnaDefaultEquals` fallback compares class instances by
        // identity, so the hash must agree with identity or a lookup could
        // miss an entry it can prove equal.
        return Int32(
            truncatingIfNeeded: ObjectIdentifier(obj as AnyObject).hashValue)
    }
}

/// The `System.Collections.Generic.KeyValuePair<TKey,TValue>` projection.
///
/// A CLR `struct` with two get-only properties, so a Swift `struct` with the
/// same two. `ToString()` is deliberately absent: the CLR builds
/// `"[key, value]"` through the virtual `Object.ToString` of each component,
/// which Swift cannot dispatch for an unconstrained generic, and a
/// `String(describing:)` stand-in would be a different string presented as the
/// same member.
public struct CNAKeyValuePair<Key, Value> {
    /// `KeyValuePair<TKey,TValue>.Key`.
    public let Key: Key

    /// `KeyValuePair<TKey,TValue>.Value`.
    public let Value: Value

    /// `.ctor(TKey key, TValue value)`.
    public init(key: Key, value: Value) {
        self.Key = key
        self.Value = value
    }
}

/// The `System.Collections.Generic.Dictionary<TKey,TValue>` projection.
///
/// `mscorlib` declares an unsealed class of generic arity 2 on `System.Object`
/// whose entire public surface is either `virtual final` — a sealed interface
/// implementation — or plainly non-virtual. **Nothing on it is an override
/// point** except `GetObjectData` and `OnDeserialization`, neither of which is
/// projected. The Swift surface is therefore `final` throughout while the class
/// itself is `open`, which is what lets `LaunchParameters` derive from it and
/// change nothing, exactly as the CLR allows.
open class CNADictionary<Key, Value> {
    /// `Dictionary<TKey,TValue>.Entry`.
    ///
    /// Reproduced rather than replaced by a Swift dictionary, because the
    /// entry ARRAY is what defines enumeration order and the free list is what
    /// defines slot reuse. `hashCode` is `-1` on a freed slot, which is how
    /// the CLR marks one, and `next` doubles as the free-list link.
    private struct Entry {
        var hashCode: Int32
        var next: Int32
        var key: Key?
        var value: Value?
    }

    // `Dictionary`2::buckets` — chain heads, `-1` for empty. `nil` until the
    // first insertion, which is exactly how the CLR defers allocation for a
    // zero-capacity dictionary.
    private var buckets: [Int32]?
    private var entries: [Entry] = []
    private var entryCount: Int32 = 0
    private var freeList: Int32 = -1
    private var freeCount: Int32 = 0
    private var version: UInt64 = 0
    private let keyComparer: any CNAEqualityComparer<Key>

    /// The exact `Argument_AddingDuplicate` resource string, read from the
    /// admitted assembly's own embedded table.
    internal static var addingDuplicateMessage: String {
        "An item with the same key has already been added."
    }

    /// The exact `ArgumentOutOfRange_NeedNonNegNum` resource string.
    internal static var needNonNegativeMessage: String {
        "Non-negative number required."
    }

    /// The exact `Arg_ArrayPlusOffTooSmall` resource string.
    internal static var arrayPlusOffTooSmallMessage: String {
        "Destination array is not long enough to copy all the items in the "
        + "collection. Check array index and length."
    }

    // ------------------------------------------------------------------
    // Construction.
    //
    // All six public CLR constructors funnel into `.ctor(int, IEqualityComparer)`:
    // a negative capacity raises `ArgumentOutOfRangeException(capacity)`, a
    // positive one pre-allocates, and a nil comparer becomes
    // `EqualityComparer<T>.Default`.
    // ------------------------------------------------------------------

    /// `Dictionary<TKey,TValue>..ctor()`.
    public init() {
        self.keyComparer = CNADefaultEqualityComparer<Key>()
    }

    /// `Dictionary<TKey,TValue>..ctor(int capacity)`.
    public init(capacity: Int32) throws {
        self.keyComparer = CNADefaultEqualityComparer<Key>()
        try reserve(capacity: capacity)
    }

    /// `Dictionary<TKey,TValue>..ctor(IEqualityComparer<TKey> comparer)`.
    ///
    /// The CLR accepts a null comparer here and substitutes the default; a
    /// non-Optional Swift parameter cannot be nil, so the substitution is
    /// unreachable through this signature and the parameterless constructor is
    /// the way to ask for the default.
    public init(comparer: any CNAEqualityComparer<Key>) {
        self.keyComparer = comparer
    }

    /// `Dictionary<TKey,TValue>..ctor(int capacity, IEqualityComparer<TKey> comparer)`.
    public init(capacity: Int32, comparer: any CNAEqualityComparer<Key>) throws {
        self.keyComparer = comparer
        try reserve(capacity: capacity)
    }

    /// `Dictionary<TKey,TValue>..ctor(IDictionary<TKey,TValue> dictionary)`.
    ///
    /// The CLR body is `this(dictionary.Count, null)` followed by a foreach
    /// that `Add`s every pair, so it is a COPY and not a live view — unlike
    /// `Collection<T>`'s wrapping constructor — and a duplicate key coming out
    /// of the source would surface as the same `Add` failure.
    public init(dictionary: CNADictionary<Key, Value>) throws {
        self.keyComparer = CNADefaultEqualityComparer<Key>()
        try copy(from: dictionary)
    }

    /// `Dictionary<TKey,TValue>..ctor(IDictionary<TKey,TValue>, IEqualityComparer<TKey>)`.
    public init(
        dictionary: CNADictionary<Key, Value>,
        comparer: any CNAEqualityComparer<Key>
    ) throws {
        self.keyComparer = comparer
        try copy(from: dictionary)
    }

    private func copy(from source: CNADictionary<Key, Value>) throws {
        try reserve(capacity: source.Count)
        let enumerator = source.GetEnumerator()
        while let pair = try enumerator.Next() {
            try Add(pair.Key, value: pair.Value)
        }
    }

    /// The constructors' own capacity handling, which is NOT part of
    /// `Initialize`.
    ///
    /// The IL is `if (capacity < 0) ThrowArgumentOutOfRangeException(capacity);
    /// if (capacity > 0) Initialize(capacity);` — so a zero capacity allocates
    /// nothing at all and the first insertion is what triggers allocation.
    /// Keeping the guard here rather than inside `initialize` matters, because
    /// `Insert` calls `Initialize(0)` and must get a real table back.
    private func reserve(capacity: Int32) throws {
        guard capacity >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "capacity",
                message: CNADictionary.needNonNegativeMessage)
        }
        guard capacity > 0 else { return }
        initialize(capacity: capacity)
    }

    /// `Dictionary`2::Initialize(int32 capacity)`.
    ///
    /// `size = HashHelpers.GetPrime(capacity)`, a bucket array of that size
    /// filled with `-1`, an entry array of the same size, and `freeList = -1`.
    /// `GetPrime(0)` is 3, the first entry of the table, so `Initialize(0)`
    /// allocates rather than doing nothing.
    private func initialize(capacity: Int32) {
        let size = CNAHashHelpers.prime(atLeast: capacity)
        buckets = Array(repeating: -1, count: Int(size))
        entries = Array(
            repeating: Entry(hashCode: -1, next: -1, key: nil, value: nil),
            count: Int(size))
        freeList = -1
    }

    // ------------------------------------------------------------------
    // Reads.
    // ------------------------------------------------------------------

    /// `Dictionary<TKey,TValue>.Count`.
    ///
    /// `count - freeCount`, so a removed slot stops counting immediately even
    /// though its entry is still in the array waiting to be reused.
    public final var Count: Int32 { entryCount - freeCount }

    /// `Dictionary<TKey,TValue>.Comparer`.
    ///
    /// The field itself, so the object a caller supplied is the object it gets
    /// back.
    public final var Comparer: any CNAEqualityComparer<Key> { keyComparer }

    /// `Dictionary<TKey,TValue>.this[TKey]` getter.
    ///
    /// `FindEntry`, then `ThrowKeyNotFoundException()` when the key is absent —
    /// which is the CLR's own `Arg_KeyNotFound` message.
    public final func Item(_ key: Key) throws -> Value {
        let index = findEntry(key)
        guard index >= 0 else {
            // ThrowHelper.ThrowKeyNotFoundException's whole body is
            // `new KeyNotFoundException()`, so the message is the
            // parameterless constructor's substituted Arg_KeyNotFound.
            throw CNAKeyNotFoundException()
        }
        return entries[Int(index)].value!
    }

    /// `Dictionary<TKey,TValue>.this[TKey]` setter — the projection of
    /// `set_Item`, which is `Insert(key, value, add: false)`.
    ///
    /// An existing key is overwritten rather than refused, and **the version
    /// still advances**, so a live enumerator is invalidated by an overwrite
    /// just as it is by an insertion.
    ///
    /// It does **not** throw. `Insert`'s only failures are
    /// `ArgumentNullException` for a null key -- unreachable through a
    /// non-Optional Swift parameter -- and the duplicate-key
    /// `ArgumentException`, which `Insert` raises only when `add` is true. The
    /// setter passes false, so no value a caller can supply reaches a failure.
    public final func SetItem(_ key: Key, _ value: Value) {
        insert(key, value, add: false)
    }

    /// `Dictionary<TKey,TValue>.ContainsKey`.
    public final func ContainsKey(_ key: Key) -> Bool { findEntry(key) >= 0 }

    /// `Dictionary<TKey,TValue>.ContainsValue`.
    ///
    /// A linear walk of the live entries under
    /// `EqualityComparer<TValue>.Default` — the VALUE comparer, which is never
    /// the caller-supplied key comparer.
    public final func ContainsValue(_ value: Value) -> Bool {
        for index in 0..<Int(entryCount)
        where entries[index].hashCode >= 0 &&
            cnaDefaultEquals(entries[index].value!, value) {
            return true
        }
        return false
    }

    /// `Dictionary<TKey,TValue>.TryGetValue`.
    ///
    /// On failure the CLR writes `default(TValue)` to the out parameter, which
    /// for every reference type is null. Swift cannot synthesize a default for
    /// an unconstrained generic, so the out parameter is projected `Optional`
    /// and set to nil — which is exactly the CLR result for a reference type
    /// and an honest "no value" for any other. An `inout Value` would have had
    /// to leave the caller's previous value in place, which the CLR never does.
    @discardableResult
    public final func TryGetValue(_ key: Key, value: inout Value?) -> Bool {
        let index = findEntry(key)
        guard index >= 0 else {
            value = nil
            return false
        }
        value = entries[Int(index)].value
        return true
    }

    // ------------------------------------------------------------------
    // Mutation.
    // ------------------------------------------------------------------

    /// `Dictionary<TKey,TValue>.Add` — `Insert(key, value, add: true)`.
    ///
    /// A duplicate key raises `ArgumentException` with the assembly's own
    /// `Argument_AddingDuplicate` string, and nothing is stored.
    public final func Add(_ key: Key, value: Value) throws {
        guard insert(key, value, add: true) else {
            // ThrowHelper.ThrowArgumentException(Argument_AddingDuplicate)
            // selects the message-only constructor, so ParamName is nil.
            throw CNAArgumentException(
                message: CNADictionary.addingDuplicateMessage)
        }
    }

    /// `Dictionary<TKey,TValue>.Remove`.
    ///
    /// Unlinks the entry from its chain, marks the slot `hashCode = -1`, and
    /// pushes it onto the free list — so the NEXT insertion reuses this exact
    /// slot and lands at this position in enumeration order. Returns `false`
    /// without touching the version when the key is absent.
    @discardableResult
    public final func Remove(_ key: Key) -> Bool {
        guard let bucketArray = buckets else { return false }
        let hashCode = hash(of: key)
        let bucket = Int(hashCode % Int32(bucketArray.count))
        var last: Int32 = -1
        var index = bucketArray[bucket]
        while index >= 0 {
            let entry = entries[Int(index)]
            if entry.hashCode == hashCode, keyComparer.Equals(entry.key!, key) {
                if last < 0 {
                    buckets![bucket] = entry.next
                } else {
                    entries[Int(last)].next = entry.next
                }
                entries[Int(index)].hashCode = -1
                entries[Int(index)].next = freeList
                entries[Int(index)].key = nil
                entries[Int(index)].value = nil
                freeList = index
                freeCount += 1
                version &+= 1
                return true
            }
            last = index
            index = entry.next
        }
        return false
    }

    /// `Dictionary<TKey,TValue>.Clear`.
    ///
    /// The CLR guards the whole body on `count > 0`, so clearing an already
    /// empty dictionary does **not** advance the version and does not
    /// invalidate an enumerator. That is transcribed rather than simplified.
    public final func Clear() {
        guard entryCount > 0 else { return }
        if buckets != nil {
            for index in buckets!.indices { buckets![index] = -1 }
        }
        for index in 0..<Int(entryCount) {
            entries[index] = Entry(hashCode: -1, next: -1, key: nil, value: nil)
        }
        freeList = -1
        entryCount = 0
        freeCount = 0
        version &+= 1
    }

    // ------------------------------------------------------------------
    // Enumeration.
    // ------------------------------------------------------------------

    /// `Dictionary<TKey,TValue>.GetEnumerator()`.
    ///
    /// The CLR returns the sealed nested `Enumerator` struct, whose whole
    /// surface is `Current`, `MoveNext` and `Dispose` — that is,
    /// `IEnumerator<KeyValuePair<TKey,TValue>>` and nothing more. It is
    /// projected as `CNAEnumerator`, the established projection of CLR
    /// enumeration in this binding, which already carries the
    /// version-invalidation contract that `MoveNext` raises
    /// `InvalidOperationException` for.
    ///
    /// The walk is over the ENTRY ARRAY in index order, skipping freed slots,
    /// so the order is insertion order with removed slots reused in
    /// last-freed-first order. It is never hash order.
    public final func GetEnumerator() -> CNAEnumerator<CNAKeyValuePair<Key, Value>> {
        enumerator { entry in
            CNAKeyValuePair(key: entry.key!, value: entry.value!)
        }
    }

    private func enumerator<Projected>(
        _ project: @escaping (Entry) -> Projected
    ) -> CNAEnumerator<Projected> {
        var cursor = 0
        return CNAEnumerator(expectedVersion: version) { [self] _, expected in
            guard version == expected else {
                throw CNAInvalidOperationException(
                    message: CNAList<Key>.enumFailedVersionMessage)
            }
            while cursor < Int(entryCount) {
                let entry = entries[cursor]
                cursor += 1
                if entry.hashCode >= 0 { return project(entry) }
            }
            return nil
        }
    }

    /// `Dictionary<TKey,TValue>.Keys`.
    ///
    /// A live view over this dictionary, allocated fresh on each read exactly
    /// as `get_Keys` does when its cached field is null. Nothing is copied.
    public final var Keys: KeyCollection { KeyCollection(dictionary: self) }

    /// `Dictionary<TKey,TValue>.Values`.
    public final var Values: ValueCollection { ValueCollection(dictionary: self) }

    /// `Dictionary<TKey,TValue>.KeyCollection`.
    ///
    /// CLR `sealed`, so Swift `final`. It holds the dictionary by reference and
    /// reads through it, so every later change is visible.
    public final class KeyCollection {
        private let dictionary: CNADictionary<Key, Value>

        /// `.ctor(Dictionary<TKey,TValue> dictionary)`.
        public init(dictionary: CNADictionary<Key, Value>) {
            self.dictionary = dictionary
        }

        /// `KeyCollection.Count`, read live from the dictionary.
        public var Count: Int32 { dictionary.Count }

        /// `KeyCollection.CopyTo(TKey[] array, int index)`.
        public func CopyTo(_ array: inout [Key], index: Int32) throws {
            try dictionary.copyTo(&array, index: index) { $0.key! }
        }

        /// `KeyCollection.GetEnumerator()`, in the dictionary's entry order.
        public func GetEnumerator() -> CNAEnumerator<Key> {
            dictionary.enumerator { $0.key! }
        }
    }

    /// `Dictionary<TKey,TValue>.ValueCollection`.
    public final class ValueCollection {
        private let dictionary: CNADictionary<Key, Value>

        /// `.ctor(Dictionary<TKey,TValue> dictionary)`.
        public init(dictionary: CNADictionary<Key, Value>) {
            self.dictionary = dictionary
        }

        /// `ValueCollection.Count`, read live from the dictionary.
        public var Count: Int32 { dictionary.Count }

        /// `ValueCollection.CopyTo(TValue[] array, int index)`.
        public func CopyTo(_ array: inout [Value], index: Int32) throws {
            try dictionary.copyTo(&array, index: index) { $0.value! }
        }

        /// `ValueCollection.GetEnumerator()`, in the dictionary's entry order.
        public func GetEnumerator() -> CNAEnumerator<Value> {
            dictionary.enumerator { $0.value! }
        }
    }

    /// The shared body of both `CopyTo` overloads.
    ///
    /// The CLR order is exact: a negative index is
    /// `ArgumentOutOfRangeException`, an index past the end or a destination
    /// too short is `ArgumentException` with `Arg_ArrayPlusOffTooSmall`, and
    /// only then are the live entries written in entry order.
    private func copyTo<Projected>(
        _ array: inout [Projected], index: Int32,
        _ project: (Entry) -> Projected
    ) throws {
        // ThrowArgumentOutOfRangeException(index, ArgumentOutOfRange_NeedNonNegNum)
        // -- note the resource: the dictionary's key and value collections use
        // a different one from List<T>'s indexer, and the paramName is `index`
        // even on the arrayIndex-named overload.
        guard index >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index",
                message: CNADictionary.needNonNegativeMessage)
        }
        let start = Int(index)
        guard start <= array.count,
              array.count - start >= Int(Count) else {
            throw CNAArgumentException(
                message: CNADictionary.arrayPlusOffTooSmallMessage)
        }
        var offset = start
        for position in 0..<Int(entryCount) where entries[position].hashCode >= 0 {
            array[offset] = project(entries[position])
            offset += 1
        }
    }

    // ------------------------------------------------------------------
    // The CLR algorithm.
    // ------------------------------------------------------------------

    /// `comparer.GetHashCode(key) & 0x7FFFFFFF`.
    ///
    /// The mask is what makes `-1` usable as the freed-slot marker: a real
    /// hash code is always non-negative here, whatever the comparer returns.
    private func hash(of key: Key) -> Int32 {
        keyComparer.GetHashCode(key) & 0x7FFF_FFFF
    }

    /// `Dictionary`2::FindEntry(!TKey key)`.
    ///
    /// Walks one bucket chain, comparing the stored `hashCode` FIRST and only
    /// then asking the comparer. A comparer whose `GetHashCode` disagrees with
    /// its `Equals` therefore fails to find entries it would call equal —
    /// which is the CLR's behaviour, not a defect of this projection.
    private func findEntry(_ key: Key) -> Int32 {
        guard let bucketArray = buckets else { return -1 }
        let hashCode = hash(of: key)
        var index = bucketArray[Int(hashCode % Int32(bucketArray.count))]
        while index >= 0 {
            let entry = entries[Int(index)]
            if entry.hashCode == hashCode, keyComparer.Equals(entry.key!, key) {
                return index
            }
            index = entry.next
        }
        return -1
    }

    /// `Dictionary`2::Insert(!TKey key, !TValue value, bool add)`.
    ///
    /// Returns `false` for the one failure a Swift caller can reach: an `Add`
    /// whose key is already present. Reporting it rather than throwing from
    /// here is what lets `SetItem` -- which passes `add: false` and therefore
    /// cannot fail -- keep the non-throwing signature the CLR gives it.
    @discardableResult
    private func insert(_ key: Key, _ value: Value, add: Bool) -> Bool {
        if buckets == nil { initialize(capacity: 0) }
        let hashCode = hash(of: key)
        var targetBucket = Int(hashCode % Int32(buckets!.count))

        var index = buckets![targetBucket]
        while index >= 0 {
            let entry = entries[Int(index)]
            if entry.hashCode == hashCode, keyComparer.Equals(entry.key!, key) {
                if add { return false }
                entries[Int(index)].value = value
                version &+= 1
                return true
            }
            index = entry.next
        }

        let slot: Int32
        if freeCount > 0 {
            slot = freeList
            freeList = entries[Int(slot)].next
            freeCount -= 1
        } else {
            if entryCount == Int32(entries.count) {
                resize()
                targetBucket = Int(hashCode % Int32(buckets!.count))
            }
            slot = entryCount
            entryCount += 1
        }

        entries[Int(slot)] = Entry(
            hashCode: hashCode, next: buckets![targetBucket],
            key: key, value: value)
        buckets![targetBucket] = slot
        version &+= 1
        return true
    }

    /// `Dictionary`2::Resize()`.
    ///
    /// A new bucket array of `GetPrime(count * 2)`, the entries COPIED IN
    /// ORDER, and every live entry rechained. Entry indices are preserved, so
    /// growing the dictionary never reorders enumeration.
    private func resize() {
        let newSize = CNAHashHelpers.prime(atLeast: entryCount * 2)
        var newBuckets = [Int32](repeating: -1, count: Int(newSize))
        var newEntries = entries
        newEntries.append(contentsOf: Array(
            repeating: Entry(hashCode: -1, next: -1, key: nil, value: nil),
            count: Int(newSize) - entries.count))
        for position in 0..<Int(entryCount) where newEntries[position].hashCode >= 0 {
            let bucket = Int(newEntries[position].hashCode % newSize)
            newEntries[position].next = newBuckets[bucket]
            newBuckets[bucket] = Int32(position)
        }
        buckets = newBuckets
        entries = newEntries
    }
}

/// The `System.Collections.HashHelpers` projection.
///
/// `internal`, exactly as `mscorlib` declares it: it is the sizing helper
/// `Dictionary<TKey,TValue>` calls, not public API. It lives outside the
/// generic class because Swift has no static stored property in a generic
/// type, which is also where the CLR puts it.
internal enum CNAHashHelpers {
    /// `System.Collections.HashHelpers.GetPrime(int min)`.
    ///
    /// The first entry of `mscorlib`'s own prime table that is at least `min`,
    /// and otherwise the next odd number that is prime and satisfies the
    /// Hashtable load-factor rule. The table is transcribed from the admitted
    /// assembly rather than invented, so no sizing decision of this
    /// projection's own enters the algorithm — though none would have been
    /// observable, because enumeration order is entry order and a lookup is
    /// correct for any bucket count.
    static func prime(atLeast min: Int32) -> Int32 {
        for candidate in hashPrimes where candidate >= min { return candidate }
        var candidate = min | 1
        while candidate < Int32.max {
            if isPrime(candidate) { return candidate }
            candidate += 2
        }
        return min
    }

    static func isPrime(_ candidate: Int32) -> Bool {
        guard candidate & 1 != 0 else { return candidate == 2 }
        let limit = Int32(Double(candidate).squareRoot())
        var divisor: Int32 = 3
        while divisor <= limit {
            if candidate % divisor == 0 { return false }
            divisor += 2
        }
        return true
    }

    static let hashPrimes: [Int32] = [
        3, 7, 11, 17, 23, 29, 37, 47, 59, 71, 89, 107, 131, 163, 197, 239, 293,
        353, 431, 521, 631, 761, 919, 1103, 1327, 1597, 1931, 2333, 2801, 3371,
        4049, 4861, 5839, 7013, 8419, 10103, 12143, 14591, 17519, 21023, 25229,
        30293, 36353, 43627, 52361, 62851, 75431, 90523, 108631, 130363,
        156437, 187751, 225307, 270371, 324449, 389357, 467237, 560689, 672827,
        807403, 968897, 1162687, 1395263, 1674319, 2009191, 2411033, 2893249,
        3471899, 4166287, 4999559, 5999471, 7199369,
    ]
}
