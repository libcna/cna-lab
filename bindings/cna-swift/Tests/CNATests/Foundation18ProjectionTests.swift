// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift language projection qualification for the Foundation 18 batch. These
// are Swift/CLR mapping facts, not XNA runtime observations, so they are
// deliberately kept out of the pure XNA-derived behaviour corpus.
final class Foundation18ProjectionTests: XCTestCase {
    // Named ModeCollection so the alias cannot shadow Swift's own Collection.
    typealias ModeCollection = Microsoft.Xna.Framework.Graphics.DisplayModeCollection
    typealias Mode = Microsoft.Xna.Framework.Graphics.DisplayMode
    typealias Location = Microsoft.Xna.Framework.Input.Touch.TouchLocation
    typealias Sample = Microsoft.Xna.Framework.Input.Touch.GestureSample
    typealias Vector2 = Microsoft.Xna.Framework.Vector2

    func testDisplayModeCollectionMapsItsCollectionInterfacesWithoutFakeTypes() {
        let collection = ModeCollection(displayModes: [
            Mode(width: 640, height: 480, format: .Color),
        ])

        // CLR IEnumerator<T> maps to the CNAEnumerator<T> support type outside
        // the XNA namespace: a throwing live cursor, deliberately not a Swift
        // IteratorProtocol, because IteratorProtocol.next() cannot throw.
        let enumerator = collection.GetEnumerator()
        XCTAssertTrue(type(of: enumerator) == CNAEnumerator<Mode>.self)
        XCTAssertFalse(enumerator is any IteratorProtocol)

        // CLR IEnumerable<T> maps to a finite Swift array, so no synthetic
        // Microsoft collection type is introduced for the indexed property.
        let filtered = collection[.Color]
        XCTAssertTrue(type(of: filtered) == [Mode].self)

        // The declared IEnumerable<DisplayMode> interface adds no automatic
        // Swift Sequence or Collection conformance.
        XCTAssertFalse(collection is any Swift.Sequence)
        XCTAssertFalse(collection is any Swift.Collection)
    }

    func testDisplayModeCollectionExposesNoPublicConstructionRoute() {
        // The CLR constructor is `assembly` accessible, so the Swift mapping
        // exposes construction only inside this module and the compiler Symbol
        // Graph contains no public DisplayModeCollection init. The class is
        // deliberately neither `open` — no accessible constructor makes it
        // externally subclassable — nor `final`, because metadata
        // sealed=false must not be strengthened.
        let collection = ModeCollection(displayModes: [])
        XCTAssertTrue(type(of: collection) == ModeCollection.self)

        // A CLR class is a reference type: assignment aliases one instance.
        let alias = collection
        XCTAssertTrue(collection === alias)
        XCTAssertFalse(collection === ModeCollection(displayModes: []))

        // The filtered result is a fresh array each time, never a live view.
        let modes = [Mode(width: 1, height: 1, format: .Color)]
        let populated = ModeCollection(displayModes: modes)
        var first = populated[.Color]
        first.removeAll()
        XCTAssertEqual(populated[.Color].count, 1)
    }

    func testTouchValueStructsProjectAsSwiftValueTypesWithMappedMemberTypes() {
        // A CLR sequential value struct maps to a Swift struct with value
        // semantics, and a value-type constructor's external labels are all
        // `_`.
        var location = Location(1, .Moved, Vector2(2, 3))
        let copy = location
        location = Location(4, .Pressed, Vector2(5, 6))
        XCTAssertEqual(copy.Id, 1)
        XCTAssertEqual(location.Id, 4)

        // `out` maps to Swift `inout`: direction, type and mutability are
        // measured, and the callee writes through the caller's storage.
        var previous = Location(0, .Invalid, Vector2(0, 0))
        let withPrevious =
            Location(9, .Moved, Vector2(1, 1), .Pressed, Vector2(2, 2))
        XCTAssertTrue(withPrevious.TryGetPreviousLocation(&previous))
        XCTAssertEqual(previous.Id, 9)
        XCTAssertEqual(previous.State, .Pressed)

        // Mapped member types: System.Int32 -> Int32, Vector2 -> Vector2,
        // TouchLocationState -> TouchLocationState, System.TimeSpan ->
        // Duration.
        let id = location.Id
        let position = location.Position
        let state = location.State
        XCTAssertTrue(type(of: id) == Int32.self)
        XCTAssertTrue(type(of: position) == Vector2.self)
        XCTAssertTrue(
            type(of: state)
                == Microsoft.Xna.Framework.Input.Touch.TouchLocationState.self)

        let sample = Sample(
            .Tap, .seconds(2), Vector2(0, 0), Vector2(0, 0),
            Vector2(0, 0), Vector2(0, 0))
        let timestamp = sample.Timestamp
        XCTAssertTrue(type(of: timestamp) == Duration.self)
        XCTAssertEqual(timestamp, .seconds(2))
    }

    func testTouchLocationDeclaresNoSwiftEquatableOrHashableConformance() {
        // The pinned type declares IEquatable<TouchLocation>, which maps to
        // the exact typed Equals member plus the operators, and adds no Swift
        // Equatable or Hashable conformance.
        let location = Location(1, .Moved, Vector2(2, 3))
        XCTAssertFalse(location is any Equatable)
        XCTAssertFalse(location is any Hashable)

        // GestureSample declares no equality identity at all, so it has
        // neither the XNA members nor a Swift conformance.
        let sample = Sample(
            .Tap, .zero, Vector2(0, 0), Vector2(0, 0),
            Vector2(0, 0), Vector2(0, 0))
        XCTAssertFalse(sample is any Equatable)

        // The XNA members are the whole equality surface for TouchLocation.
        XCTAssertTrue(location == location)
        XCTAssertTrue(location.Equals(location))
        XCTAssertEqual(location.GetHashCode(), location.GetHashCode())
    }
}
