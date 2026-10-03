// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift-language qualification of the BCL collection base projection. None of
// this is XNA runtime behaviour and none of it is counted as such: it measures
// the shape the Swift compiler actually emitted, and several of the assertions
// below are compile-time facts that could not be written at all if the
// projection were wrong.
final class Foundation27ProjectionTests: XCTestCase {
    typealias Collection = Microsoft.Xna.Framework.GameComponentCollection
    typealias Component = any Microsoft.Xna.Framework.IGameComponent

    private final class Stub: Microsoft.Xna.Framework.IGameComponent {
        func Initialize() throws {}
    }

    // ------------------------------------------------------------------
    // The superclass identity and its specialization, proved by the compiler.
    // ------------------------------------------------------------------

    // A `GameComponentCollection` IS a `CNACollection<any IGameComponent>`.
    // The annotation is the proof: Swift accepts the assignment only if the
    // superclass is exactly this specialization, so an `Any` erasure or a
    // different element type would fail to compile rather than fail here.
    func testSuperclassIsTheExactCollectionSpecialization() {
        let collection = Collection()
        let asBase: CNACollection<Component> = collection
        XCTAssertTrue(asBase === collection)

        // ... and the runtime agrees with the static type. The cast goes
        // through `Any` deliberately: a direct `is` on the statically-typed
        // value is a compile-time tautology the compiler warns about, which
        // is itself the static half of this proof.
        XCTAssertNotNil(collection as Any as? CNACollection<Component>)
    }

    // The element type is preserved rather than erased: the *wrong*
    // specializations are not supertypes of this class.
    func testTheElementTypeIsNotErased() {
        let collection: Any = Collection()
        XCTAssertNil(collection as? CNACollection<Any>)
        XCTAssertNil(collection as? CNACollection<AnyObject>)
        XCTAssertNil(
            collection as? CNACollection<any Microsoft.Xna.Framework.IUpdateable>)
    }

    // A function that accepts the base accepts the subclass, which is what
    // "real Swift class inheritance" buys and what composition would not.
    func testTheSubclassIsUsableWhereTheBaseIsExpected() throws {
        func countThrough(_ collection: CNACollection<Component>) -> Int32 {
            collection.Count
        }
        let collection = Collection()
        try collection.Add(Stub())
        XCTAssertEqual(countThrough(collection), 1)
    }

    // The base is not a value type, and neither is the subclass: assigning
    // shares one object rather than copying.
    func testTheCollectionFamiliesAreReferenceTypes() throws {
        let collection = Collection()
        let alias: CNACollection<Component> = collection
        try alias.Add(Stub())
        XCTAssertEqual(collection.Count, 1, "a value type would have copied")

        let list = CNAList<Int>()
        let listAlias = list
        listAlias.Add(1)
        XCTAssertEqual(list.Count, 1)
        XCTAssertTrue(list === listAlias)
    }

    // CLR `sealed` is Swift `final`: `GameComponentCollection` cannot be
    // subclassed. Its own base is not final, which is what lets it exist.
    func testSealedProjectsToFinal() {
        // The support base is open, so this compiles; a `final` base would
        // make the whole projection impossible. `GameComponentCollection`
        // itself is `final`, so no such declaration can name it as a base --
        // that half is enforced by the compiler at every would-be use site.
        final class Derived: CNACollection<Int> {}
        XCTAssertEqual(Derived().Count, 0)
        XCTAssertNotNil(Derived() as Any as? CNACollection<Int>)
    }

    // ------------------------------------------------------------------
    // The hooks are overridable, and only the hooks.
    // ------------------------------------------------------------------

    // All four protected virtuals are `open`, so a subclass in this module and
    // in any other can override every one of them. This class compiling *is*
    // the assertion.
    func testAllFourHooksAreOverridable() throws {
        final class AllFour: CNACollection<Int> {
            var calls: [String] = []
            override func ClearItems() throws {
                calls.append("Clear"); try super.ClearItems()
            }
            override func InsertItem(_ index: Int32, item: Int) throws {
                calls.append("Insert"); try super.InsertItem(index, item: item)
            }
            override func RemoveItem(_ index: Int32) throws {
                calls.append("Remove"); try super.RemoveItem(index)
            }
            override func SetItem(_ index: Int32, item: Int) throws {
                calls.append("Set"); try super.SetItem(index, item: item)
            }
        }
        let collection = AllFour()
        try collection.Add(1)
        try collection.SetItem(0, 2)
        try collection.RemoveAt(0)
        try collection.Clear()
        XCTAssertEqual(collection.calls, ["Insert", "Set", "Remove", "Clear"])
    }

    // Dynamic dispatch reaches the override through a base-typed reference,
    // which is the whole point of modelling the relationship as inheritance.
    func testOverridesAreReachedThroughABaseTypedReference() throws {
        final class Counting: CNACollection<Int> {
            var inserts = 0
            override func InsertItem(_ index: Int32, item: Int) throws {
                inserts += 1; try super.InsertItem(index, item: item)
            }
        }
        let concrete = Counting()
        let base: CNACollection<Int> = concrete
        try base.Add(1)
        try base.Insert(0, item: 2)
        XCTAssertEqual(concrete.inserts, 2)
    }

    // ------------------------------------------------------------------
    // Effects and Optionality of the projected signatures.
    // ------------------------------------------------------------------

    // `Count` and `Items` are infallible non-Optional reads: a key path can
    // only be formed for a non-throwing property, so the compiler states it.
    func testCountAndItemsAreInfallibleReads() {
        let countPath: KeyPath<Collection, Int32> = \Collection.Count
        let itemsPath: KeyPath<Collection, CNAList<Component>> = \Collection.Items
        let collection = Collection()
        XCTAssertEqual(collection[keyPath: countPath], 0)
        XCTAssertEqual(collection[keyPath: itemsPath].Count, 0)
    }

    // Neither is settable: `Count` and `Items` are get-only in `mscorlib`.
    func testCountAndItemsAreNotWritable() {
        XCTAssertNil(
            (\Collection.Count as KeyPath<Collection, Int32>)
                as? ReferenceWritableKeyPath<Collection, Int32>)
        XCTAssertNil(
            (\Collection.Items as KeyPath<Collection, CNAList<Component>>)
                as? ReferenceWritableKeyPath<Collection, CNAList<Component>>)
    }

    // `Contains` and `IndexOf` delegate to the backing store and cannot fail,
    // so they need no `try` -- the absence of one here is the assertion.
    func testSearchMembersAreInfallible() {
        let collection = Collection()
        XCTAssertFalse(collection.Contains(Stub()))
        XCTAssertEqual(collection.IndexOf(Stub()), -1)
    }

    // `CopyTo` takes the destination `inout`, because a CLR `CopyTo`
    // destination is caller-owned storage that the call writes into.
    func testCopyToTakesTheDestinationInout() throws {
        let collection = Collection()
        try collection.Add(Stub())
        var destination: [Component] = [Stub(), Stub()]
        let before = destination[0]
        try collection.CopyTo(&destination, index: 0)
        XCTAssertFalse(destination[0] as AnyObject === before as AnyObject)
    }

    // The enumerator is the throwing support projection, deliberately not
    // `IteratorProtocol`: `next()` cannot throw and CLR enumeration can.
    func testEnumeratorIsTheThrowingSupportProjection() throws {
        let collection = Collection()
        try collection.Add(Stub())
        let enumerator: CNAEnumerator<Component> = collection.GetEnumerator()
        XCTAssertNotNil(try enumerator.Next())
        XCTAssertNil(try enumerator.Next())
    }

    // The support classes are deliberately NOT Swift `Sequence`s: adopting it
    // would supply a non-throwing iterator that cannot express the CLR
    // invalidation contract.
    func testSupportClassesDoNotConformToSequence() {
        XCTAssertNil(Collection() as Any as? any Sequence)
        XCTAssertNil(CNAList<Int>() as Any as? any Sequence)
        XCTAssertNil(CNAReadOnlyCollection<Int>(list: CNAList<Int>())
            as Any as? any Sequence)
    }

    // ------------------------------------------------------------------
    // The read-only family exposes no mutator.
    // ------------------------------------------------------------------

    // `CNAReadOnlyCollection` is a class, is open, and carries only readers.
    // A subclass compiling here proves `open`; the absence of any mutating
    // call proves the surface.
    func testReadOnlyCollectionIsAnOpenReaderOnlyClass() throws {
        final class Derived: CNAReadOnlyCollection<Int> {}
        let backing = CNAList<Int>()
        backing.Add(1)
        let derived = Derived(list: backing)
        XCTAssertEqual(derived.Count, 1)
        XCTAssertEqual(try derived.Item(0), 1)
        XCTAssertTrue(derived.Items === backing)
    }

    // Neither collection family is the other's supertype, so a read-only view
    // cannot be widened into a mutable collection at runtime either.
    func testTheFamiliesAreNotRelatedByInheritance() {
        let backing = CNAList<Int>()
        let readOnly: Any = CNAReadOnlyCollection<Int>(list: backing)
        let mutable: Any = CNACollection<Int>(list: backing)
        XCTAssertNil(readOnly as? CNACollection<Int>)
        XCTAssertNil(mutable as? CNAReadOnlyCollection<Int>)
    }

    // ------------------------------------------------------------------
    // Namespace placement.
    // ------------------------------------------------------------------

    // The support classes live outside `Microsoft.Xna.Framework`, so they are
    // not XNA identities and no `System` namespace is fabricated for them.
    // Their reflected names carry the module and the bare type name only.
    func testSupportClassesLiveOutsideTheXnaNamespace() {
        for name in [
            String(reflecting: CNACollection<Int>.self),
            String(reflecting: CNAReadOnlyCollection<Int>.self),
            String(reflecting: CNAList<Int>.self),
        ] {
            XCTAssertFalse(name.contains("Microsoft.Xna.Framework"), name)
            XCTAssertFalse(name.contains("System."), name)
            XCTAssertTrue(name.hasPrefix("CNA."), name)
        }
        // The XNA subclass, by contrast, keeps its full XNA identity.
        let xna = String(reflecting: Collection.self)
        XCTAssertTrue(xna.contains("Microsoft.Xna.Framework"), xna)
    }
}
