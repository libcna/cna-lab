// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure BCL-derived behaviour for the two collection support families,
// transcribed from the CIL of the hash-registered Microsoft .NET Framework
// 4.0 `mscorlib` admitted in `tools/api_compat/bcl-authorities.json`
// (SHA-256 5634668d…acc63) and pinned in
// `tools/api_compat/reference/bcl40-selected-shape.json`.
//
// Nothing here is an XNA identity and nothing here is counted as one. These
// are the BCL semantics that `GameComponentCollection` and the four
// `Model*Collection` types inherit rather than declare.
extension PureValueTests {

    private final class Component: Microsoft.Xna.Framework.IGameComponent {
        let tag: Int
        init(_ tag: Int) { self.tag = tag }
        func Initialize() throws {}
    }

    // ------------------------------------------------------------------
    // Construction and the live backing store.
    // ------------------------------------------------------------------

    // `Collection`1::.ctor()` is
    // `ldarg.0; call Object::.ctor; ldarg.0; newobj List`1::.ctor(); stfld items`
    // -- a *fresh* backing store the caller cannot reach.
    func testCollectionDefaultConstructorAllocatesItsOwnBackingStore() throws {
        let first = CNACollection<Int>()
        let second = CNACollection<Int>()
        try first.Add(1)
        XCTAssertEqual(first.Count, 1)
        XCTAssertEqual(second.Count, 0, "the two collections share a backing store")
        XCTAssertFalse(first.Items === second.Items)
    }

    // `Collection`1::.ctor(IList`1 list)` stores `ldarg.1` into `items` with no
    // copy, so the collection is a LIVE view of the caller's list. This is the
    // fact that makes flattening the family to a Swift Array wrong.
    func testCollectionWrappingConstructorObservesLaterMutationsOfTheSuppliedList() throws {
        let backing = CNAList<Int>()
        backing.Add(10)
        let collection = CNACollection<Int>(list: backing)
        XCTAssertEqual(collection.Count, 1)

        // Mutating the list the caller still holds is visible through the
        // collection, because `get_Count` reads `items` on every call.
        backing.Add(20)
        XCTAssertEqual(collection.Count, 2)
        XCTAssertEqual(try collection.Item(1), 20)

        // ... and in the other direction, through the hooks.
        try collection.Add(30)
        XCTAssertEqual(backing.Count, 3)
        XCTAssertEqual(try backing.Item(2), 30)
        XCTAssertTrue(collection.Items === backing)
    }

    // `get_Items` is `ldarg.0; ldfld items; ret` -- the field itself, so the
    // protected view is the same object every time and never a copy.
    func testCollectionItemsViewIsTheBackingStoreItself() {
        let backing = CNAList<Int>()
        let collection = CNACollection<Int>(list: backing)
        XCTAssertTrue(collection.Items === backing)
        XCTAssertTrue(collection.Items === collection.Items)
    }

    // `ReadOnlyCollection`1::.ctor(IList`1 list)` stores the argument the same
    // way. "Read-only" is about the surface this class exposes, not about the
    // contents being frozen.
    func testReadOnlyCollectionIsALiveViewNotASnapshot() throws {
        let backing = CNAList<Int>()
        backing.Add(1)
        let view = CNAReadOnlyCollection<Int>(list: backing)
        XCTAssertEqual(view.Count, 1)

        backing.Add(2)
        backing.Add(3)
        XCTAssertEqual(view.Count, 3, "a read-only view is not a snapshot")
        XCTAssertEqual(try view.Item(2), 3)

        try backing.RemoveAt(0)
        XCTAssertEqual(view.Count, 2)
        XCTAssertEqual(try view.Item(0), 2)
    }

    // ------------------------------------------------------------------
    // Every public mutator routes through a hook.
    // ------------------------------------------------------------------

    // Each of `Add`, `Clear`, `Insert`, `Remove`, `RemoveAt` and the `Item`
    // setter ends in a `callvirt` to one of the four protected virtuals and
    // touches `items` nowhere else, so a subclass override is always reached.
    func testEveryPublicMutatorRoutesThroughTheProtectedHooks() throws {
        final class Recording: CNACollection<Int> {
            var log: [String] = []
            override func ClearItems() throws {
                log.append("ClearItems"); try super.ClearItems()
            }
            override func InsertItem(_ index: Int32, item: Int) throws {
                log.append("InsertItem(\(index))")
                try super.InsertItem(index, item: item)
            }
            override func RemoveItem(_ index: Int32) throws {
                log.append("RemoveItem(\(index))"); try super.RemoveItem(index)
            }
            override func SetItem(_ index: Int32, item: Int) throws {
                log.append("SetItem(\(index))")
                try super.SetItem(index, item: item)
            }
        }

        let collection = Recording()
        try collection.Add(1)                    // InsertItem(0)
        try collection.Insert(0, item: 2)        // InsertItem(0)
        try collection.SetItem(1, 3)             // SetItem(1)
        XCTAssertTrue(try collection.Remove(2))  // RemoveItem(0)
        try collection.RemoveAt(0)               // RemoveItem(0)
        try collection.Add(4)                    // InsertItem(0)
        try collection.Clear()                   // ClearItems

        XCTAssertEqual(collection.log, [
            "InsertItem(0)", "InsertItem(0)", "SetItem(1)",
            "RemoveItem(0)", "RemoveItem(0)", "InsertItem(0)", "ClearItems",
        ])
    }

    // A hook that does not call `super` mutates nothing: the base class never
    // writes to `items` on any public path.
    func testAnOverrideThatRefusesLeavesTheCollectionUnchanged() throws {
        // A subclass's own failure, deliberately neither a projected CLR
        // exception nor a CNAError: an override may throw anything, and the
        // base class must propagate it unchanged and mutate nothing.
        struct RefusedByOverride: Error {}
        final class Refusing: CNACollection<Int> {
            override func InsertItem(_ index: Int32, item: Int) throws {
                throw RefusedByOverride()
            }
        }
        let collection = Refusing()
        XCTAssertThrowsError(try collection.Add(1)) { error in
            XCTAssertTrue(error is RefusedByOverride, "\(error)")
        }
        XCTAssertEqual(collection.Count, 0)
        XCTAssertEqual(collection.Items.Count, 0)
    }

    // `Add` is `ldfld items; callvirt get_Count; stloc.0; ... ldloc.0;
    // callvirt InsertItem` -- the count is read BEFORE the hook runs, so the
    // override sees the index the item had on entry.
    func testAddPassesThePreCallCountAsTheInsertionIndex() throws {
        final class Observing: CNACollection<Int> {
            var indices: [Int32] = []
            override func InsertItem(_ index: Int32, item: Int) throws {
                indices.append(index)
                try super.InsertItem(index, item: item)
            }
        }
        let collection = Observing()
        try collection.Add(10)
        try collection.Add(20)
        try collection.Add(30)
        XCTAssertEqual(collection.indices, [0, 1, 2])
    }

    // ------------------------------------------------------------------
    // Bounds. `Insert` uses `ble`; the `Item` setter and `RemoveAt` use `blt`.
    // ------------------------------------------------------------------

    // `Insert` compares `index` against `Count` with `ble.s`, so inserting AT
    // the count is a legal append.
    func testInsertAcceptsTheCountAsAnIndexButNothingBeyondIt() throws {
        let collection = CNACollection<Int>()
        try collection.Add(1)
        try collection.Insert(1, item: 2)          // index == Count: legal
        XCTAssertEqual(collection.Count, 2)
        XCTAssertEqual(try collection.Item(1), 2)

        XCTAssertThrowsError(try collection.Insert(3, item: 3))
        XCTAssertThrowsError(try collection.Insert(-1, item: 4))
        XCTAssertEqual(collection.Count, 2)
    }

    // `set_Item` and `RemoveAt` compare with `blt.s`, so `index == Count` is
    // out of range for both -- the asymmetry with `Insert` is real.
    func testSetterAndRemoveAtRejectTheCountAsAnIndex() throws {
        let collection = CNACollection<Int>()
        try collection.Add(1)
        XCTAssertThrowsError(try collection.SetItem(1, 9))
        XCTAssertThrowsError(try collection.RemoveAt(1))
        XCTAssertThrowsError(try collection.SetItem(-1, 9))
        XCTAssertThrowsError(try collection.RemoveAt(-1))
        XCTAssertEqual(collection.Count, 1)
        XCTAssertEqual(try collection.Item(0), 1)
    }

    // `Remove` asks the BACKING STORE for the index and returns `false`
    // without calling any hook when the item is absent.
    func testRemoveReturnsFalseAndCallsNoHookWhenTheItemIsAbsent() throws {
        final class Counting: CNACollection<Int> {
            var removals = 0
            override func RemoveItem(_ index: Int32) throws {
                removals += 1; try super.RemoveItem(index)
            }
        }
        let collection = Counting()
        try collection.Add(1)
        XCTAssertFalse(try collection.Remove(99))
        XCTAssertEqual(collection.removals, 0)
        XCTAssertEqual(collection.Count, 1)

        XCTAssertTrue(try collection.Remove(1))
        XCTAssertEqual(collection.removals, 1)
        XCTAssertEqual(collection.Count, 0)
    }

    // ------------------------------------------------------------------
    // Search, copy and enumeration all delegate to the backing store.
    // ------------------------------------------------------------------

    // `IndexOf` returns the first match, `-1` when absent, and `Contains` is
    // `IndexOf(item) >= 0`.
    func testIndexOfAndContainsFindTheFirstMatch() throws {
        let collection = CNACollection<Int>()
        for value in [5, 7, 5] { try collection.Add(value) }
        XCTAssertEqual(collection.IndexOf(5), 0)
        XCTAssertEqual(collection.IndexOf(7), 1)
        XCTAssertEqual(collection.IndexOf(99), -1)
        XCTAssertTrue(collection.Contains(7))
        XCTAssertFalse(collection.Contains(99))
    }

    // `EqualityComparer<T>.Default` reduces to reference identity for a class
    // that declares no equality of its own, which is every element type the
    // pinned XNA contract specializes these families with.
    func testReferenceElementsCompareByIdentity() throws {
        let first = Component(1)
        let second = Component(1)
        let collection = CNACollection<any Microsoft.Xna.Framework.IGameComponent>()
        try collection.Add(first)

        XCTAssertEqual(collection.IndexOf(first), 0)
        XCTAssertEqual(collection.IndexOf(second), -1, "equal tags are not identity")
        XCTAssertTrue(collection.Contains(first))
        XCTAssertFalse(collection.Contains(second))
    }

    // `CopyTo` delegates to `Array.Copy`, which raises for a negative index
    // and for a destination that is too short. The destination is caller-owned.
    func testCopyToWritesIntoTheCallerArrayAndValidatesIt() throws {
        let collection = CNACollection<Int>()
        for value in [1, 2, 3] { try collection.Add(value) }

        var destination = [0, 0, 0, 0, 0]
        try collection.CopyTo(&destination, index: 1)
        XCTAssertEqual(destination, [0, 1, 2, 3, 0])

        var tooShort = [0, 0]
        XCTAssertThrowsError(try collection.CopyTo(&tooShort, index: 0))
        var negative = [0, 0, 0, 0]
        XCTAssertThrowsError(try collection.CopyTo(&negative, index: -1))
    }

    // `GetEnumerator` returns the BACKING STORE's enumerator, and
    // `List`1/Enumerator::MoveNext` throws once `_version` no longer matches
    // the version it captured.
    func testEnumerationIsInvalidatedByMutation() throws {
        let collection = CNACollection<Int>()
        for value in [1, 2, 3] { try collection.Add(value) }

        let enumerator = collection.GetEnumerator()
        XCTAssertEqual(try enumerator.Next(), 1)
        try collection.Add(4)
        assertProjected(
            CNAInvalidOperationException.self,
            message: "Collection was modified; enumeration operation may not "
                + "execute.",
            hResult: Int32(bitPattern: 0x8013_1509)
        ) {
            _ = try enumerator.Next()
        }
    }

    // Because the enumerator belongs to the backing store, a mutation made
    // through a wrapped list -- never touching the collection -- invalidates it
    // too.
    func testEnumerationIsInvalidatedThroughTheWrappedListAsWell() throws {
        let backing = CNAList<Int>()
        backing.Add(1)
        let collection = CNACollection<Int>(list: backing)

        let enumerator = collection.GetEnumerator()
        XCTAssertEqual(try enumerator.Next(), 1)
        backing.Add(2)
        XCTAssertThrowsError(try enumerator.Next())
    }

    // A complete walk of an unmutated collection yields every element in order
    // and then nil.
    func testEnumerationYieldsEveryElementInOrder() throws {
        let collection = CNACollection<Int>()
        for value in [7, 8, 9] { try collection.Add(value) }
        let enumerator = collection.GetEnumerator()
        var seen: [Int] = []
        while let value = try enumerator.Next() { seen.append(value) }
        XCTAssertEqual(seen, [7, 8, 9])
        XCTAssertNil(try enumerator.Next())
    }

    // ------------------------------------------------------------------
    // ReadOnlyCollection exposes readers only.
    // ------------------------------------------------------------------

    func testReadOnlyCollectionReadersMatchTheBackingStore() throws {
        let backing = CNAList<Int>()
        for value in [4, 5, 6] { backing.Add(value) }
        let view = CNAReadOnlyCollection<Int>(list: backing)

        XCTAssertEqual(view.Count, 3)
        XCTAssertEqual(try view.Item(0), 4)
        XCTAssertEqual(view.IndexOf(6), 2)
        XCTAssertEqual(view.IndexOf(99), -1)
        XCTAssertTrue(view.Contains(5))
        XCTAssertTrue(view.Items === backing)

        var destination = [0, 0, 0]
        try view.CopyTo(&destination, index: 0)
        XCTAssertEqual(destination, [4, 5, 6])

        XCTAssertThrowsError(try view.Item(3))
        XCTAssertThrowsError(try view.Item(-1))
    }

    // The CLR declares both families directly on `System.Object`. Neither is a
    // base of the other, so a read-only view is not a mutable collection and
    // cannot be made into one by casting.
    func testTheTwoCollectionFamiliesAreSiblingsNotAChain() {
        let backing = CNAList<Int>()
        let view = CNAReadOnlyCollection<Int>(list: backing)
        let collection = CNACollection<Int>(list: backing)
        XCTAssertNil(view as Any as? CNACollection<Int>)
        XCTAssertNil(collection as Any as? CNAReadOnlyCollection<Int>)
    }

    // ------------------------------------------------------------------
    // The backing store's own rules.
    // ------------------------------------------------------------------

    // `List`1` compares `(uint)index` against `_size`, so a negative index and
    // an oversized one take the same branch.
    func testBackingListBoundsUseTheSameRuleForNegativeAndOversizedIndices() throws {
        let list = CNAList<Int>()
        list.Add(1)
        XCTAssertThrowsError(try list.Item(-1))
        XCTAssertThrowsError(try list.Item(1))
        XCTAssertThrowsError(try list.SetItem(-1, 9))
        XCTAssertThrowsError(try list.SetItem(1, 9))
        XCTAssertThrowsError(try list.RemoveAt(-1))
        XCTAssertThrowsError(try list.RemoveAt(1))
        // `Insert` alone accepts `index == Count`.
        try list.Insert(1, item: 2)
        XCTAssertEqual(list.Count, 2)
    }

    // `List`1::get_IsReadOnly` returns the constant `false`.
    func testBackingListIsNeverReadOnly() {
        XCTAssertFalse(CNAList<Int>().IsReadOnly)
    }
}
