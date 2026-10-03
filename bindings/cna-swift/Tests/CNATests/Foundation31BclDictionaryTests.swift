// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure BCL-derived behaviour for the dictionary support family, transcribed
// from the CIL and the embedded resource table of the hash-registered
// Microsoft .NET Framework 4.0 `mscorlib` admitted in
// `tools/api_compat/bcl-authorities.json` (SHA-256 5634668d…acc63) and pinned
// in `tools/api_compat/reference/bcl40-selected-shape.json`.
//
// Nothing here is an XNA identity. These are the semantics
// `Microsoft.Xna.Framework.LaunchParameters` inherits rather than declares.
extension PureValueTests {

    /// A comparer that folds ASCII case, used to prove the dictionary asks the
    /// COMPARER for key identity and never Swift's own `Hashable`.
    private struct CaseFoldingComparer: CNAEqualityComparer {
        func Equals(_ x: String, _ y: String) -> Bool {
            x.lowercased() == y.lowercased()
        }
        func GetHashCode(_ obj: String) -> Int32 {
            Int32(truncatingIfNeeded: obj.lowercased().hashValue)
        }
    }

    /// A deliberately broken comparer: everything is equal, but every hash is
    /// distinct. The CLR compares the stored hash BEFORE asking `Equals`, so
    /// such a comparer cannot find its own entries — a fact about the
    /// algorithm, not a defect of this projection.
    private struct InconsistentComparer: CNAEqualityComparer {
        func Equals(_ x: String, _ y: String) -> Bool { true }
        func GetHashCode(_ obj: String) -> Int32 {
            Int32(truncatingIfNeeded: obj.hashValue)
        }
    }

    private func makeDictionary(
        _ pairs: [(String, String)]
    ) throws -> CNADictionary<String, String> {
        let dictionary = CNADictionary<String, String>()
        for (key, value) in pairs { try dictionary.Add(key, value: value) }
        return dictionary
    }

    private func keys(
        of dictionary: CNADictionary<String, String>
    ) throws -> [String] {
        var collected: [String] = []
        let enumerator = dictionary.GetEnumerator()
        while let pair = try enumerator.Next() { collected.append(pair.Key) }
        return collected
    }

    // ------------------------------------------------------------------
    // Construction.
    // ------------------------------------------------------------------

    // `.ctor()` allocates no table at all: `Initialize` is only reached from
    // the capacity constructor when capacity is positive, or from the first
    // `Insert`.
    func testDictionaryDefaultConstructorIsEmpty() throws {
        let dictionary = CNADictionary<String, String>()
        XCTAssertEqual(dictionary.Count, 0)
        XCTAssertFalse(dictionary.ContainsKey("a"))
        XCTAssertEqual(try keys(of: dictionary), [])
        // The first insertion is what allocates, and it must work.
        try dictionary.Add("a", value: "1")
        XCTAssertEqual(dictionary.Count, 1)
    }

    // `if (capacity < 0) ThrowArgumentOutOfRangeException(capacity)`, with the
    // assembly's own `ArgumentOutOfRange_NeedNonNegNum` message behind it. A
    // capacity of zero is legal and allocates nothing.
    func testDictionaryCapacityConstructorRejectsANegativeCapacity() throws {
        XCTAssertNoThrow(try CNADictionary<String, String>(capacity: 0))
        XCTAssertNoThrow(try CNADictionary<String, String>(capacity: 17))
        assertProjected(
            CNAArgumentOutOfRangeException.self,
            message: composedArgumentMessage(
                "Non-negative number required.", paramName: "capacity"),
            paramName: "capacity",
            hResult: Int32(bitPattern: 0x8013_1502)
        ) {
            _ = try CNADictionary<String, String>(capacity: -1)
        }
    }

    // The dictionary-taking constructor is `this(dictionary.Count, comparer)`
    // followed by an `Add` of every pair — a COPY, unlike `Collection<T>`'s
    // wrapping constructor, which stores its argument live.
    func testDictionaryCopyConstructorCopiesAndIsNotALiveView() throws {
        let source = try makeDictionary([("a", "1"), ("b", "2")])
        let copy = try CNADictionary<String, String>(dictionary: source)
        XCTAssertEqual(copy.Count, 2)
        XCTAssertEqual(try copy.Item("b"), "2")

        try source.Add("c", value: "3")
        XCTAssertEqual(source.Count, 3)
        XCTAssertEqual(copy.Count, 2, "the copy is a live view of the source")
        XCTAssertEqual(try keys(of: copy), ["a", "b"])
    }

    // ------------------------------------------------------------------
    // Add, the indexer, and the difference between them.
    // ------------------------------------------------------------------

    // `Add` is `Insert(key, value, add: true)` and `set_Item` is
    // `Insert(key, value, add: false)`. That single boolean is the whole
    // difference, and it is the behaviour a Swift `Dictionary` cannot express.
    func testAddRefusesADuplicateKeyWhileTheIndexerOverwritesIt() throws {
        let dictionary = try makeDictionary([("a", "1")])

        // ThrowArgumentException(Argument_AddingDuplicate) selects the
        // message-only overload, so there is no parameter name to compose in.
        assertProjected(
            CNAArgumentException.self,
            message: "An item with the same key has already been added.",
            hResult: Int32(bitPattern: 0x8007_0057)
        ) {
            try dictionary.Add("a", value: "2")
        }
        XCTAssertEqual(try dictionary.Item("a"), "1",
                       "a refused Add must leave the entry untouched")
        XCTAssertEqual(dictionary.Count, 1)

        dictionary.SetItem("a", "2")
        XCTAssertEqual(try dictionary.Item("a"), "2")
        XCTAssertEqual(dictionary.Count, 1, "an overwrite is not an insertion")
    }

    // `get_Item` calls `ThrowKeyNotFoundException()`, whose message is the
    // assembly's own `Arg_KeyNotFound`.
    func testIndexerGetterRaisesKeyNotFoundForAnAbsentKey() throws {
        let dictionary = try makeDictionary([("a", "1")])
        // KeyNotFoundException derives from SystemException directly, so it
        // is NOT an ArgumentException and carries no parameter name.
        assertProjected(
            CNAKeyNotFoundException.self,
            message: "The given key was not present in the dictionary.",
            hResult: Int32(bitPattern: 0x8013_1577)
        ) {
            _ = try dictionary.Item("missing")
        }
        XCTAssertFalse(
            (try? dictionary.Item("missing")) != nil,
            "the getter must not have inserted anything")
        // The indexer SETTER inserts where the getter would have failed.
        dictionary.SetItem("missing", "2")
        XCTAssertEqual(try dictionary.Item("missing"), "2")
        XCTAssertEqual(dictionary.Count, 2)
    }

    // The indexer setter passes `add: false` to `Insert`, whose only other
    // failure is a null key -- unreachable through a non-Optional Swift
    // parameter. So `set_Item` cannot fail for any value a caller can supply,
    // and the projection is deliberately NOT `throws`. `Add`, which passes
    // `add: true`, is the one that can.
    func testTheIndexerSetterCannotFailWhileAddCan() throws {
        let dictionary = try makeDictionary([("a", "1")])
        dictionary.SetItem("a", "2")
        dictionary.SetItem("b", "3")
        XCTAssertEqual(dictionary.Count, 2)
        XCTAssertThrowsError(try dictionary.Add("a", value: "4"))
    }

    // `TryGetValue` writes `default(TValue)` on failure, which for a reference
    // type is null. The out parameter is Optional so that result can be
    // reproduced rather than the caller's previous value being left in place.
    func testTryGetValueClearsTheOutParameterOnFailure() throws {
        let dictionary = try makeDictionary([("a", "1")])
        var value: String? = "left over"
        XCTAssertTrue(dictionary.TryGetValue("a", value: &value))
        XCTAssertEqual(value, "1")

        value = "left over"
        XCTAssertFalse(dictionary.TryGetValue("missing", value: &value))
        XCTAssertNil(value, "the CLR writes default(TValue) on failure")
    }

    // `ContainsValue` walks the live entries under the VALUE comparer, which
    // is never the caller-supplied key comparer.
    func testContainsKeyAndContainsValue() throws {
        let dictionary = try makeDictionary([("a", "1"), ("b", "2")])
        XCTAssertTrue(dictionary.ContainsKey("a"))
        XCTAssertFalse(dictionary.ContainsKey("A"))
        XCTAssertTrue(dictionary.ContainsValue("2"))
        XCTAssertFalse(dictionary.ContainsValue("3"))
    }

    // ------------------------------------------------------------------
    // Enumeration order — the reason the CLR storage is reproduced.
    // ------------------------------------------------------------------

    // The enumerator walks the ENTRY ARRAY in index order, and an entry's
    // index comes from `count++` or from the free list, never from its hash.
    // Insertion order therefore survives, and it survives a `Resize` too,
    // because `Resize` copies the entries in order.
    func testEnumerationIsInsertionOrderAndSurvivesResize() throws {
        let inserted = (0..<40).map { ("k\($0)", "v\($0)") }
        let dictionary = try makeDictionary(inserted)
        XCTAssertEqual(try keys(of: dictionary), inserted.map { $0.0 })
        XCTAssertEqual(dictionary.Count, 40)
    }

    // `Remove` pushes the freed slot onto the free list and the next `Add`
    // takes it back, so a re-added key lands where the removed one was — not
    // at the end. This is the sharpest observable consequence of reproducing
    // the CLR storage instead of approximating it.
    func testARemovedSlotIsReusedByTheNextInsertion() throws {
        let dictionary = try makeDictionary([
            ("a", "1"), ("b", "2"), ("c", "3"),
        ])
        XCTAssertTrue(dictionary.Remove("b"))
        XCTAssertEqual(try keys(of: dictionary), ["a", "c"])

        try dictionary.Add("d", value: "4")
        XCTAssertEqual(
            try keys(of: dictionary), ["a", "d", "c"],
            "the new entry must take the freed slot, not the end")
    }

    // Two removals, then two insertions: the free list is LIFO, so the
    // last-freed slot is the first reused.
    func testTheFreeListIsLastFreedFirstReused() throws {
        let dictionary = try makeDictionary([
            ("a", "1"), ("b", "2"), ("c", "3"), ("d", "4"),
        ])
        XCTAssertTrue(dictionary.Remove("b"))
        XCTAssertTrue(dictionary.Remove("c"))
        try dictionary.Add("x", value: "9")
        try dictionary.Add("y", value: "8")
        XCTAssertEqual(try keys(of: dictionary), ["a", "y", "x", "d"])
    }

    func testRemoveReportsWhetherItRemovedAnything() throws {
        let dictionary = try makeDictionary([("a", "1")])
        XCTAssertFalse(dictionary.Remove("missing"))
        XCTAssertEqual(dictionary.Count, 1)
        XCTAssertTrue(dictionary.Remove("a"))
        XCTAssertEqual(dictionary.Count, 0)
        XCTAssertFalse(dictionary.Remove("a"))
    }

    // ------------------------------------------------------------------
    // The version counter.
    // ------------------------------------------------------------------

    func testEveryMutationInvalidatesALiveEnumerator() throws {
        for mutate in [
            { (dictionary: CNADictionary<String, String>) in
                try dictionary.Add("z", value: "9") },
            { dictionary in dictionary.SetItem("a", "9") },
            { dictionary in _ = dictionary.Remove("a") },
            { dictionary in dictionary.Clear() },
        ] {
            let dictionary = try makeDictionary([("a", "1"), ("b", "2")])
            let enumerator = dictionary.GetEnumerator()
            XCTAssertEqual(try enumerator.Next()?.Key, "a")
            try mutate(dictionary)
            assertProjected(
                CNAInvalidOperationException.self,
                message: "Collection was modified; enumeration operation may "
                    + "not execute.",
                hResult: Int32(bitPattern: 0x8013_1509)
            ) {
                _ = try enumerator.Next()
            }
        }
    }

    // An overwrite through the indexer bumps the version even though nothing
    // was inserted — the CLR does `entries[i].value = value; version++` on the
    // found path, and a projection that skipped it would leave a stale
    // enumerator running.
    func testAnOverwriteInvalidatesALiveEnumerator() throws {
        let dictionary = try makeDictionary([("a", "1")])
        let enumerator = dictionary.GetEnumerator()
        dictionary.SetItem("a", "2")
        XCTAssertThrowsError(try enumerator.Next())
    }

    // `Clear`'s whole body is guarded on `count > 0`, so clearing an already
    // empty dictionary changes nothing — including the version.
    func testClearingAnEmptyDictionaryDoesNotInvalidateAnEnumerator() throws {
        let dictionary = CNADictionary<String, String>()
        let enumerator = dictionary.GetEnumerator()
        dictionary.Clear()
        XCTAssertNil(try enumerator.Next())
    }

    func testClearResetsCountAndReleasesEverySlot() throws {
        let dictionary = try makeDictionary([("a", "1"), ("b", "2")])
        dictionary.Clear()
        XCTAssertEqual(dictionary.Count, 0)
        XCTAssertFalse(dictionary.ContainsKey("a"))
        XCTAssertEqual(try keys(of: dictionary), [])
        // Insertion after a clear starts again at slot 0.
        try dictionary.Add("c", value: "3")
        XCTAssertEqual(try keys(of: dictionary), ["c"])
    }

    // ------------------------------------------------------------------
    // Keys and Values.
    // ------------------------------------------------------------------

    func testKeysAndValuesAreLiveViewsInEntryOrder() throws {
        let dictionary = try makeDictionary([("a", "1"), ("b", "2")])
        let keyView = dictionary.Keys
        let valueView = dictionary.Values
        XCTAssertEqual(keyView.Count, 2)
        XCTAssertEqual(valueView.Count, 2)

        try dictionary.Add("c", value: "3")
        XCTAssertEqual(keyView.Count, 3, "the view is not live")

        var collected: [String] = []
        let enumerator = keyView.GetEnumerator()
        while let key = try enumerator.Next() { collected.append(key) }
        XCTAssertEqual(collected, ["a", "b", "c"])

        var values: [String] = []
        let valueEnumerator = valueView.GetEnumerator()
        while let value = try valueEnumerator.Next() { values.append(value) }
        XCTAssertEqual(values, ["1", "2", "3"])
    }

    func testKeyCollectionCopyToChecksTheDestination() throws {
        let dictionary = try makeDictionary([("a", "1"), ("b", "2")])
        var destination = ["", "", ""]
        try dictionary.Keys.CopyTo(&destination, index: 1)
        XCTAssertEqual(destination, ["", "a", "b"])

        var negative = ["", ""]
        // The key/value collections pass ArgumentOutOfRange_NeedNonNegNum,
        // NOT List<T>'s ArgumentOutOfRange_Index, and name "index" even on the
        // overload whose CLR parameter is called `index`.
        assertProjected(
            CNAArgumentOutOfRangeException.self,
            message: composedArgumentMessage(
                "Non-negative number required.", paramName: "index"),
            paramName: "index",
            hResult: Int32(bitPattern: 0x8013_1502)
        ) {
            try dictionary.Keys.CopyTo(&negative, index: -1)
        }
        var tooShort = [""]
        assertProjected(
            CNAArgumentException.self,
            message: "Destination array is not long enough to copy all the "
                + "items in the collection. Check array index and length.",
            hResult: Int32(bitPattern: 0x8007_0057)
        ) {
            try dictionary.Values.CopyTo(&tooShort, index: 0)
        }
    }

    // ------------------------------------------------------------------
    // The comparer decides key identity.
    // ------------------------------------------------------------------

    func testASuppliedComparerDecidesKeyIdentityAndIsHandedBack() throws {
        let comparer = CaseFoldingComparer()
        let dictionary = CNADictionary<String, String>(comparer: comparer)
        try dictionary.Add("Key", value: "1")

        XCTAssertTrue(dictionary.ContainsKey("KEY"))
        XCTAssertEqual(try dictionary.Item("key"), "1")
        XCTAssertThrowsError(try dictionary.Add("kEy", value: "2"),
                             "a case-folding comparer makes this a duplicate")
        XCTAssertTrue(dictionary.Comparer is CaseFoldingComparer)

        // The default comparer does NOT fold case, so the same keys are
        // distinct there — which is what proves the comparer was consulted.
        let byDefault = try makeDictionary([("Key", "1")])
        XCTAssertFalse(byDefault.ContainsKey("KEY"))
    }

    // `FindEntry` compares the STORED hash first and only then asks `Equals`,
    // so a comparer whose hash disagrees with its equality cannot find its own
    // entries. That is the CLR algorithm, reproduced rather than smoothed
    // over: a projection built on Swift's `Hashable` could not exhibit it.
    func testAComparerWhoseHashDisagreesWithEqualsCannotFindItsEntries() throws {
        let dictionary = CNADictionary<String, String>(
            comparer: InconsistentComparer())
        try dictionary.Add("a", value: "1")
        XCTAssertTrue(dictionary.ContainsKey("a"))
        XCTAssertFalse(
            dictionary.ContainsKey("b"),
            "the hashes differ, so the chain is never even walked")
        // ... and because the lookup misses, the "duplicate" is accepted.
        XCTAssertNoThrow(try dictionary.Add("b", value: "2"))
        XCTAssertEqual(dictionary.Count, 2)
    }

    // ------------------------------------------------------------------
    // The sizing table.
    // ------------------------------------------------------------------

    // `HashHelpers.GetPrime` returns the first table entry at least `min`, and
    // computes the next odd prime beyond the table. The table itself is
    // compared against the assembly by the verifier; this pins the lookup.
    func testGetPrimeMatchesTheAdmittedTable() {
        XCTAssertEqual(CNAHashHelpers.prime(atLeast: 0), 3)
        XCTAssertEqual(CNAHashHelpers.prime(atLeast: 3), 3)
        XCTAssertEqual(CNAHashHelpers.prime(atLeast: 4), 7)
        XCTAssertEqual(CNAHashHelpers.prime(atLeast: 12), 17)
        XCTAssertEqual(CNAHashHelpers.prime(atLeast: 7199369), 7199369)
        // Past the end of the table the CLR searches odd candidates upward.
        XCTAssertEqual(CNAHashHelpers.prime(atLeast: 7199370), 7199371)
        XCTAssertEqual(CNAHashHelpers.hashPrimes.count, 72)
    }

    // ------------------------------------------------------------------
    // KeyValuePair.
    // ------------------------------------------------------------------

    func testKeyValuePairCarriesBothHalves() throws {
        let dictionary = try makeDictionary([("a", "1")])
        let enumerator = dictionary.GetEnumerator()
        let pair = try enumerator.Next()
        XCTAssertEqual(pair?.Key, "a")
        XCTAssertEqual(pair?.Value, "1")
        XCTAssertNil(try enumerator.Next())

        let constructed = CNAKeyValuePair(key: 1, value: "one")
        XCTAssertEqual(constructed.Key, 1)
        XCTAssertEqual(constructed.Value, "one")
    }
}
