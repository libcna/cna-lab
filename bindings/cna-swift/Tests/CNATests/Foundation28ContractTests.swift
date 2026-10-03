// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for `Microsoft.Xna.Framework.GameComponentCollection`,
// transcribed from the CIL of the registered, hash-matched
// Microsoft.Xna.Framework.Game.dll (SHA-256 b5dffdd8…a1f0). The two exception
// messages are the assembly's own resource strings, read out of its
// `Microsoft.Xna.Framework.Resources.resources` blob rather than invented.
//
// Nothing here is imported from any other binding: every ordering below was
// read off this assembly's IL.
extension PureValueTests {

    fileprivate final class TestComponent: Microsoft.Xna.Framework.IGameComponent {
        let name: String
        private(set) var initialized = 0
        init(_ name: String) { self.name = name }
        func Initialize() throws { initialized += 1 }
    }

    private func components(
        _ collection: Microsoft.Xna.Framework.GameComponentCollection
    ) throws -> [String] {
        var names: [String] = []
        var index: Int32 = 0
        while index < collection.Count {
            let item = try collection.Item(index)
            names.append((item as? TestComponent)?.name ?? "?")
            index += 1
        }
        return names
    }

    // ------------------------------------------------------------------
    // The inherited surface exists and is the Collection<T> behaviour.
    // ------------------------------------------------------------------

    // `.ctor()` is `ldarg.0; call Collection`1<IGameComponent>::.ctor(); ret`,
    // so the collection starts empty over its own fresh backing store and the
    // whole inherited surface is usable.
    func testGameComponentCollectionInheritsTheCollectionSurface() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        XCTAssertEqual(collection.Count, 0)

        let first = TestComponent("first")
        let second = TestComponent("second")
        try collection.Add(first)
        try collection.Insert(0, item: second)

        XCTAssertEqual(collection.Count, 2)
        XCTAssertEqual(try components(collection), ["second", "first"])
        XCTAssertEqual(collection.IndexOf(first), 1)
        XCTAssertTrue(collection.Contains(second))
        XCTAssertFalse(collection.Contains(TestComponent("absent")))

        XCTAssertTrue(try collection.Remove(second))
        XCTAssertEqual(try components(collection), ["first"])
        try collection.RemoveAt(0)
        XCTAssertEqual(collection.Count, 0)
    }

    // ------------------------------------------------------------------
    // InsertItem: duplicate check BEFORE the insert.
    // ------------------------------------------------------------------

    // The IL is `IndexOf(item); ldc.i4.m1; beq.s` -- so a component already in
    // the collection is refused with `ArgumentException` and the assembly's
    // `CannotAddSameComponentMultipleTimes` string, before anything is stored.
    func testGameComponentCollectionRefusesTheSameComponentTwice() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        let component = TestComponent("only")
        try collection.Add(component)

        // `newobj ArgumentException::.ctor(string)` -- message only, so no
        // parameter name is composed into the message.
        assertProjected(
            CNAArgumentException.self,
            message: "Cannot add the same game component to a game component "
                + "collection multiple times.",
            hResult: Int32(bitPattern: 0x8007_0057)
        ) {
            try collection.Add(component)
        }
        XCTAssertEqual(collection.Count, 1, "the refused insert must not store")
    }

    // The check is by identity, not by value: two distinct components are two
    // distinct entries even when they are otherwise indistinguishable.
    func testGameComponentCollectionAcceptsDistinctEqualLookingComponents() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        try collection.Add(TestComponent("same"))
        try collection.Add(TestComponent("same"))
        XCTAssertEqual(collection.Count, 2)
    }

    // A refused duplicate raises no event at all: the throw precedes both the
    // insert and the raise.
    func testRefusedDuplicateRaisesNoEvent() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        let component = TestComponent("only")
        try collection.Add(component)

        var added = 0
        collection.ComponentAdded.Add { _, _ in added += 1 }
        XCTAssertThrowsError(try collection.Add(component))
        XCTAssertEqual(added, 0)
    }

    // ------------------------------------------------------------------
    // Event ordering, sender and argument identity.
    // ------------------------------------------------------------------

    // `InsertItem` calls `base.InsertItem` and only then `OnComponentAdded`,
    // so a handler observes the collection with the item ALREADY present.
    func testComponentAddedFiresAfterTheItemIsInTheCollection() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        let component = TestComponent("added")

        var observedCount: Int32 = -1
        var observedContains = false
        collection.ComponentAdded.Add { _, _ in
            observedCount = collection.Count
            observedContains = collection.Contains(component)
        }
        try collection.Add(component)

        XCTAssertEqual(observedCount, 1)
        XCTAssertTrue(observedContains)
    }

    // `OnComponentAdded` invokes the delegate with `ldarg.0` as the sender --
    // the collection itself -- and a fresh `GameComponentCollectionEventArgs`
    // built from the item.
    func testComponentAddedCarriesTheCollectionAsSenderAndTheItemAsArgument()
        throws
    {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        let component = TestComponent("added")

        var senderIsCollection = false
        var argumentIsComponent = false
        collection.ComponentAdded.Add { sender, args in
            senderIsCollection = (sender as AnyObject) === collection
            argumentIsComponent = (args.GameComponent as AnyObject) === component
        }
        try collection.Add(component)

        XCTAssertTrue(senderIsCollection)
        XCTAssertTrue(argumentIsComponent)
    }

    // Every raise executes `newobj GameComponentCollectionEventArgs::.ctor`,
    // so two notifications never share an argument object.
    func testEachNotificationGetsItsOwnEventArgumentObject() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        var seen: [Microsoft.Xna.Framework.GameComponentCollectionEventArgs] = []
        collection.ComponentAdded.Add { _, args in seen.append(args) }

        try collection.Add(TestComponent("a"))
        try collection.Add(TestComponent("b"))

        XCTAssertEqual(seen.count, 2)
        XCTAssertFalse(seen[0] === seen[1])
    }

    // `RemoveItem` reads the item first, removes it, and raises last, so a
    // handler observes the collection with the item ALREADY gone.
    func testComponentRemovedFiresAfterTheItemIsGone() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        let component = TestComponent("removed")
        try collection.Add(component)

        var observedCount: Int32 = -1
        var observedContains = true
        var argumentIsComponent = false
        var senderIsCollection = false
        collection.ComponentRemoved.Add { sender, args in
            observedCount = collection.Count
            observedContains = collection.Contains(component)
            argumentIsComponent = (args.GameComponent as AnyObject) === component
            senderIsCollection = (sender as AnyObject) === collection
        }
        try collection.RemoveAt(0)

        XCTAssertEqual(observedCount, 0)
        XCTAssertFalse(observedContains)
        XCTAssertTrue(argumentIsComponent)
        XCTAssertTrue(senderIsCollection)
    }

    // `Remove` finds the index and delegates to `RemoveItem`, so it announces
    // the component that was actually removed.
    func testRemoveAnnouncesTheComponentItRemoved() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        let first = TestComponent("first")
        let second = TestComponent("second")
        try collection.Add(first)
        try collection.Add(second)

        var announced: [String] = []
        collection.ComponentRemoved.Add { _, args in
            announced.append((args.GameComponent as? TestComponent)?.name ?? "?")
        }
        XCTAssertTrue(try collection.Remove(first))
        XCTAssertEqual(announced, ["first"])

        // An absent component reaches no hook, so nothing is announced.
        XCTAssertFalse(try collection.Remove(TestComponent("absent")))
        XCTAssertEqual(announced, ["first"])
    }

    // ------------------------------------------------------------------
    // ClearItems: the forward walk that announces BEFORE emptying.
    // ------------------------------------------------------------------

    // The IL loop runs `for (i = 0; i < Count; i++) OnComponentRemoved(...)`
    // and calls `base.ClearItems()` only afterwards. Every handler therefore
    // runs while the collection is still FULL. This is the ordering that the
    // obvious implementation gets wrong.
    func testClearAnnouncesEveryComponentBeforeEmptyingTheCollection() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        for name in ["a", "b", "c"] { try collection.Add(TestComponent(name)) }

        var announced: [String] = []
        var countsDuringClear: [Int32] = []
        var firstItemDuringClear: [String] = []
        collection.ComponentRemoved.Add { _, args in
            announced.append((args.GameComponent as? TestComponent)?.name ?? "?")
            countsDuringClear.append(collection.Count)
            let first = try? collection.Item(0)
            firstItemDuringClear.append((first as? TestComponent)?.name ?? "?")
        }
        try collection.Clear()

        // Forward order, not reverse.
        XCTAssertEqual(announced, ["a", "b", "c"])
        // The collection was still full for every single notification.
        XCTAssertEqual(countsDuringClear, [3, 3, 3])
        XCTAssertEqual(firstItemDuringClear, ["a", "a", "a"])
        // And empty once the loop finished.
        XCTAssertEqual(collection.Count, 0)
    }

    // Clearing an empty collection runs the loop zero times and announces
    // nothing.
    func testClearingAnEmptyCollectionAnnouncesNothing() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        var announced = 0
        collection.ComponentRemoved.Add { _, _ in announced += 1 }
        try collection.Clear()
        XCTAssertEqual(announced, 0)
        XCTAssertEqual(collection.Count, 0)
    }

    // ------------------------------------------------------------------
    // SetItem: refused outright.
    // ------------------------------------------------------------------

    // The hook's IL is a single `throw`: no bounds check, no store, no event.
    // The message is the assembly's own
    // `CannotSetItemsIntoGameComponentCollection` resource string.
    func testIndexedAssignmentIsRefused() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        let original = TestComponent("original")
        try collection.Add(original)

        var events = 0
        collection.ComponentAdded.Add { _, _ in events += 1 }
        collection.ComponentRemoved.Add { _, _ in events += 1 }

        // The exact `CannotSetItemsIntoGameComponentCollection` string, read
        // out of the registered assembly's own resource table and pinned in
        // reference/xna40-selected-resource-strings.json -- including the
        // double space, which is XNA's.
        assertProjected(
            CNANotSupportedException.self,
            message: "Cannot set a value using operator[] on "
                + "GameComponentCollection.  Use Add/Remove instead.",
            hResult: Int32(bitPattern: 0x8013_1515)
        ) {
            try collection.SetItem(0, TestComponent("replacement"))
        }
        XCTAssertEqual(collection.Count, 1)
        XCTAssertTrue(try collection.Item(0) as AnyObject === original)
        XCTAssertEqual(events, 0)
    }

    // `Collection<T>.set_Item` performs its bounds check BEFORE reaching the
    // hook, so an out-of-range index reports the range rather than the
    // refusal. The inherited order is preserved, not short-circuited.
    func testOutOfRangeAssignmentReportsTheRangeNotTheRefusal() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        try collection.Add(TestComponent("only"))

        // `Collection<T>.set_Item` uses the NO-ARGUMENT ThrowHelper overload,
        // which pairs ExceptionArgument `index` with ArgumentOutOfRange_Index
        // -- a different resource from the one `Insert` uses.
        assertProjected(
            CNAArgumentOutOfRangeException.self,
            message: composedArgumentMessage(
                "Index was out of range. Must be non-negative and less than "
                + "the size of the collection.", paramName: "index"),
            paramName: "index",
            hResult: Int32(bitPattern: 0x8013_1502)
        ) {
            try collection.SetItem(5, TestComponent("x"))
        }
    }

    // ------------------------------------------------------------------
    // Inherited bounds and identity.
    // ------------------------------------------------------------------

    // The inherited `Insert`/`RemoveAt` asymmetry survives the override: the
    // overrides add validation of their own but change neither bound.
    func testInheritedBoundsSurviveTheOverrides() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        try collection.Add(TestComponent("a"))

        try collection.Insert(1, item: TestComponent("b"))   // index == Count
        XCTAssertEqual(collection.Count, 2)
        XCTAssertThrowsError(try collection.Insert(3, item: TestComponent("c")))
        XCTAssertThrowsError(try collection.RemoveAt(2))
        XCTAssertThrowsError(try collection.Item(2))
        XCTAssertEqual(collection.Count, 2)
    }

    // The collection is a reference type with a stable backing store: two
    // references are one collection, and `Items` is the same object every time.
    func testGameComponentCollectionHasReferenceIdentity() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        let alias = collection
        try alias.Add(TestComponent("through the alias"))
        XCTAssertEqual(collection.Count, 1)
        XCTAssertTrue(collection === alias)
        XCTAssertTrue(collection.Items === collection.Items)
    }

    // Enumeration is the backing store's, so it is invalidated by any mutation
    // -- including one made by an event handler.
    func testEnumerationIsInvalidatedByAMutationDuringNotification() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        try collection.Add(TestComponent("a"))
        try collection.Add(TestComponent("b"))

        let enumerator = collection.GetEnumerator()
        XCTAssertNotNil(try enumerator.Next())
        try collection.Add(TestComponent("c"))
        assertProjected(
            CNAInvalidOperationException.self,
            message: "Collection was modified; enumeration operation may not "
                + "execute.",
            hResult: Int32(bitPattern: 0x8013_1509)
        ) {
            _ = try enumerator.Next()
        }
    }

    // A throwing handler propagates to the mutator that raised the event, and
    // the mutation itself has already happened -- the raise is the last thing
    // `InsertItem` does.
    func testAThrowingHandlerPropagatesAfterTheMutation() throws {
        struct HandlerFailure: Error {}
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        collection.ComponentAdded.Add { _, _ in throw HandlerFailure() }

        XCTAssertThrowsError(try collection.Add(TestComponent("a"))) { error in
            XCTAssertTrue(error is HandlerFailure)
        }
        XCTAssertEqual(collection.Count, 1, "the insert precedes the raise")
    }

    // Subscriptions are independent and removable, and removal stops later
    // notifications without disturbing the others.
    func testSubscriptionsAreIndependentAndRemovable() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        var first = 0
        var second = 0
        let token = collection.ComponentAdded.Add { _, _ in first += 1 }
        collection.ComponentAdded.Add { _, _ in second += 1 }

        try collection.Add(TestComponent("a"))
        XCTAssertEqual(first, 1)
        XCTAssertEqual(second, 1)

        collection.ComponentAdded.Remove(token)
        try collection.Add(TestComponent("b"))
        XCTAssertEqual(first, 1)
        XCTAssertEqual(second, 2)
    }

    // The two events are distinct: subscribing to one never sees the other.
    func testTheTwoEventsAreDistinct() throws {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        var addedSeen = 0
        var removedSeen = 0
        collection.ComponentAdded.Add { _, _ in addedSeen += 1 }
        collection.ComponentRemoved.Add { _, _ in removedSeen += 1 }

        let component = TestComponent("a")
        try collection.Add(component)
        XCTAssertEqual(addedSeen, 1)
        XCTAssertEqual(removedSeen, 0)

        XCTAssertTrue(try collection.Remove(component))
        XCTAssertEqual(addedSeen, 1)
        XCTAssertEqual(removedSeen, 1)
    }

    // The published event view has stable identity, as a CLR event field does.
    func testEventViewsHaveStableIdentity() {
        let collection = Microsoft.Xna.Framework.GameComponentCollection()
        XCTAssertTrue(collection.ComponentAdded === collection.ComponentAdded)
        XCTAssertTrue(collection.ComponentRemoved === collection.ComponentRemoved)
        XCTAssertFalse(
            collection.ComponentAdded === collection.ComponentRemoved as AnyObject)
    }
}
