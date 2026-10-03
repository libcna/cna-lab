// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the Foundation 18 batch, transcribed from the
// registered, hash-matched Microsoft.Xna.Framework.Graphics.dll and
// Microsoft.Xna.Framework.Input.Touch.dll IL.
extension PureValueTests {
    func testDisplayModeCollectionXnaContract() {
        typealias Collection = Microsoft.Xna.Framework.Graphics.DisplayModeCollection
        typealias Mode = Microsoft.Xna.Framework.Graphics.DisplayMode

        // The pinned constructor is `assembly` and simply stores the supplied
        // List<DisplayMode>; nothing is copied, sorted, filtered or validated.
        let modes = [
            Mode(width: 800, height: 600, format: .Color),
            Mode(width: 1280, height: 720, format: .Bgr565),
            Mode(width: 1920, height: 1080, format: .Color),
        ]
        let collection = Collection(displayModes: modes)

        // GetEnumerator walks the backing list in stored order.
        let enumerator = collection.GetEnumerator()
        var walked: [Mode] = []
        while let next = try? enumerator.Next() { walked.append(next) }
        XCTAssertEqual(walked.count, 3)
        XCTAssertEqual(walked[0].Width, 800)
        XCTAssertEqual(walked[1].Width, 1280)
        XCTAssertEqual(walked[2].Width, 1920)

        // Each call returns a fresh cursor.
        let second = collection.GetEnumerator()
        XCTAssertEqual(try second.Next()?.Width, 800)

        // get_Item builds a NEW list holding every mode whose Format equals
        // the argument, in the original order.
        let colorModes = collection[.Color]
        XCTAssertEqual(colorModes.count, 2)
        XCTAssertEqual(colorModes[0].Width, 800)
        XCTAssertEqual(colorModes[1].Width, 1920)

        let bgr565Modes = collection[.Bgr565]
        XCTAssertEqual(bgr565Modes.count, 1)
        XCTAssertEqual(bgr565Modes[0].Height, 720)

        // An unmatched format yields an empty result, not an error.
        XCTAssertTrue(collection[.Dxt5].isEmpty)
        XCTAssertTrue(collection[.HdrBlendable].isEmpty)

        // An empty collection enumerates to nothing and filters to nothing.
        let empty = Collection(displayModes: [])
        XCTAssertNil(try empty.GetEnumerator().Next())
        XCTAssertTrue(empty[.Color].isEmpty)
    }

    func testGestureSampleXnaContract() {
        typealias Sample = Microsoft.Xna.Framework.Input.Touch.GestureSample
        typealias Vector2 = Microsoft.Xna.Framework.Vector2

        // The single public constructor stores all six arguments verbatim and
        // validates nothing; the six properties are plain field loads.
        let sample = Sample(
            [.Pinch, .Flick],
            .milliseconds(1500),
            Vector2(1, 2), Vector2(3, 4), Vector2(5, 6), Vector2(7, 8))
        XCTAssertEqual(sample.GestureType, [.Pinch, .Flick])
        XCTAssertEqual(sample.Timestamp, .milliseconds(1500))
        XCTAssertEqual(sample.Position.X, 1)
        XCTAssertEqual(sample.Position2.X, 3)
        XCTAssertEqual(sample.Delta.X, 5)
        XCTAssertEqual(sample.Delta2.Y, 8)

        // A zero gesture, a zero timestamp and negative deltas all round-trip.
        let zero = Sample(
            .None, .zero,
            Vector2(0, 0), Vector2(0, 0), Vector2(-1, -2), Vector2(0, 0))
        XCTAssertEqual(zero.GestureType, .None)
        XCTAssertEqual(zero.Timestamp, .zero)
        XCTAssertEqual(zero.Delta.X, -1)
        XCTAssertEqual(zero.Delta.Y, -2)

        // The pinned struct declares no equality identity, no GetHashCode and
        // no ToString, so none is projected. It is a value type.
        var copy = sample
        copy = zero
        XCTAssertEqual(sample.Position.X, 1)
        XCTAssertEqual(copy.Position.X, 0)
    }

    func testTouchLocationXnaContractConstructionAndProperties() {
        typealias Location = Microsoft.Xna.Framework.Input.Touch.TouchLocation
        typealias Vector2 = Microsoft.Xna.Framework.Vector2

        // The three-argument constructor leaves the previous location at
        // Invalid/+0/+0. The position is split into two float fields and
        // Position rebuilds a Vector2 on every read.
        let simple = Location(7, .Moved, Vector2(1.5, -2.5))
        XCTAssertEqual(simple.Id, 7)
        XCTAssertEqual(simple.State, .Moved)
        XCTAssertEqual(simple.Position.X, 1.5)
        XCTAssertEqual(simple.Position.Y, -2.5)

        // The five-argument constructor stores the previous state and
        // position too. Nothing is validated: an Invalid current state and a
        // negative id are stored as given.
        let full = Location(
            -9, .Invalid, Vector2(3, 4), .Pressed, Vector2(5, 6))
        XCTAssertEqual(full.Id, -9)
        XCTAssertEqual(full.State, .Invalid)
        XCTAssertEqual(full.Position.X, 3)
        XCTAssertEqual(full.Position.Y, 4)

        // Position is recomputed, never cached, so repeated reads agree.
        XCTAssertEqual(simple.Position.X.bitPattern, Float(1.5).bitPattern)
        XCTAssertEqual(simple.Position.X.bitPattern, simple.Position.X.bitPattern)
    }

    func testTouchLocationXnaContractTryGetPreviousLocation() {
        typealias Location = Microsoft.Xna.Framework.Input.Touch.TouchLocation
        typealias Vector2 = Microsoft.Xna.Framework.Vector2

        // A location whose previous state is the zero literal Invalid has no
        // previous location. XNA still writes the out parameter — id -1 and
        // every other field zeroed — and returns false.
        var out = Location(99, .Pressed, Vector2(9, 9))
        let absent = Location(7, .Moved, Vector2(1, 2))
        XCTAssertFalse(absent.TryGetPreviousLocation(&out))
        XCTAssertEqual(out.Id, -1)
        XCTAssertEqual(out.State, .Invalid)
        XCTAssertEqual(out.Position.X, 0)
        XCTAssertEqual(out.Position.Y, 0)

        // When present, the result carries this location's own id, the
        // previous state and position promoted into the current slots, and its
        // own previous slots cleared.
        let present = Location(
            42, .Moved, Vector2(10, 20), .Pressed, Vector2(30, 40))
        XCTAssertTrue(present.TryGetPreviousLocation(&out))
        XCTAssertEqual(out.Id, 42)
        XCTAssertEqual(out.State, .Pressed)
        XCTAssertEqual(out.Position.X, 30)
        XCTAssertEqual(out.Position.Y, 40)

        // A returned previous location never itself has a previous location.
        var chained = Location(0, .Invalid, Vector2(0, 0))
        XCTAssertFalse(out.TryGetPreviousLocation(&chained))
        XCTAssertEqual(chained.Id, -1)

        // The source is unchanged by the call.
        XCTAssertEqual(present.Id, 42)
        XCTAssertEqual(present.State, .Moved)
        XCTAssertEqual(present.Position.X, 10)
    }

    func testTouchLocationXnaContractEqualityAsymmetry() {
        typealias Location = Microsoft.Xna.Framework.Input.Touch.TouchLocation
        typealias Vector2 = Microsoft.Xna.Framework.Vector2

        // op_Equality compares all seven fields; the typed Equals compares
        // only id, x, y, prevX and prevY and ignores BOTH state fields. The
        // asymmetry is in the pinned IL and is preserved, not normalised.
        let a = Location(1, .Moved, Vector2(2, 3), .Pressed, Vector2(4, 5))
        let differentStates =
            Location(1, .Released, Vector2(2, 3), .Released, Vector2(4, 5))

        XCTAssertTrue(a.Equals(differentStates))
        XCTAssertFalse(a == differentStates)
        XCTAssertTrue(a != differentStates)

        // Identical values agree under both.
        let same = Location(1, .Moved, Vector2(2, 3), .Pressed, Vector2(4, 5))
        XCTAssertTrue(a.Equals(same))
        XCTAssertTrue(a == same)
        XCTAssertFalse(a != same)

        // Every positional field difference is caught by both.
        for other in [
            Location(9, .Moved, Vector2(2, 3), .Pressed, Vector2(4, 5)),
            Location(1, .Moved, Vector2(9, 3), .Pressed, Vector2(4, 5)),
            Location(1, .Moved, Vector2(2, 9), .Pressed, Vector2(4, 5)),
            Location(1, .Moved, Vector2(2, 3), .Pressed, Vector2(9, 5)),
            Location(1, .Moved, Vector2(2, 3), .Pressed, Vector2(4, 9)),
        ] {
            XCTAssertFalse(a.Equals(other))
            XCTAssertFalse(a == other)
            XCTAssertTrue(a != other)
        }

        // Equals(object) is false for null and for a different runtime type.
        XCTAssertFalse(a.Equals(nil))
        XCTAssertFalse(a.Equals(42))
        XCTAssertTrue(a.Equals(same as Any?))
    }

    func testTouchLocationXnaContractGetHashCodeAndToString() {
        typealias Location = Microsoft.Xna.Framework.Input.Touch.TouchLocation
        typealias Vector2 = Microsoft.Xna.Framework.Vector2

        // id.GetHashCode() + x.GetHashCode() + y.GetHashCode() with CLR
        // unchecked int32 addition. Single.GetHashCode() is the raw bit
        // pattern except that both +0 and -0 hash to 0.
        XCTAssertEqual(Location(7, .Moved, Vector2(1, 2)).GetHashCode(), 2139095047)
        XCTAssertEqual(Location(1, .Moved, Vector2(-1.5, 0.25)).GetHashCode(), -29360127)
        XCTAssertEqual(Location(0, .Invalid, Vector2(0, 0)).GetHashCode(), 0)

        // Both signed zeroes hash to 0, so -0 and +0 agree.
        XCTAssertEqual(Location(-3, .Moved, Vector2(0, -0.0)).GetHashCode(), -3)
        XCTAssertEqual(
            Location(-3, .Moved, Vector2(0, -0.0)).GetHashCode(),
            Location(-3, .Moved, Vector2(-0.0, 0)).GetHashCode())

        // The addition wraps rather than trapping.
        XCTAssertEqual(Location(0, .Moved, Vector2(2, 2)).GetHashCode(), Int32.min)

        // Neither state field nor the previous position participates.
        XCTAssertEqual(
            Location(7, .Moved, Vector2(1, 2)).GetHashCode(),
            Location(7, .Released, Vector2(1, 2), .Pressed, Vector2(8, 9)).GetHashCode())

        // string.Format(CurrentCulture, "{{Position:{0}}}", Position) — the
        // doubled braces are escapes and the argument is the boxed Vector2's
        // own ToString().
        XCTAssertEqual(
            Location(1, .Moved, Vector2(1, 2)).ToString(), "{Position:{X:1 Y:2}}")
        XCTAssertEqual(
            Location(0, .Invalid, Vector2(-1.5, 0.25)).ToString(),
            "{Position:{X:-1.5 Y:0.25}}")
    }
}
