// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the Foundation 20 batch, transcribed from the
// registered, hash-matched Microsoft.Xna.Framework.dll and
// Microsoft.Xna.Framework.Input.Touch.dll IL.
extension PureValueTests {
    // AudioListener stores an XACT_LISTENER_DATA structure in X3DAudio's
    // left-handed space, and every accessor passes through
    // UnsafeNativeStructures.FlipHandedness == Vector3(v.X, v.Y, -v.Z).
    func testAudioListenerXnaContract() {
        typealias F = Microsoft.Xna.Framework
        let listener = F.Audio.AudioListener()

        // The constructor seeds _Position and _Velocity with Vector3.Zero
        // WITHOUT flipping, while the getters flip on the way out. The default
        // Z is therefore negative zero, which compares equal to zero but is a
        // different bit pattern.
        XCTAssertEqual(listener.Position.X, 0)
        XCTAssertEqual(listener.Position.Y, 0)
        XCTAssertEqual(listener.Position.Z, 0)
        XCTAssertEqual(listener.Position.Z.sign, .minus)
        XCTAssertEqual(listener.Position.Z.bitPattern, Float(-0.0).bitPattern)
        XCTAssertEqual(listener.Velocity.Z.bitPattern, Float(-0.0).bitPattern)

        // Forward and Up are seeded *through* the flip, so they read back as
        // exactly the Vector3 constants.
        XCTAssertEqual(listener.Forward.Z.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(listener.Forward.X, 0)
        XCTAssertEqual(listener.Forward.Y, 0)
        XCTAssertEqual(listener.Up.X, 0)
        XCTAssertEqual(listener.Up.Y, 1)
        XCTAssertEqual(listener.Up.Z.bitPattern, Float(0).bitPattern)

        // The flip is a bitwise involution, so any written value round-trips
        // unchanged -- negative zero and NaN payloads included.
        listener.Position = F.Vector3(1, -2, 3)
        XCTAssertEqual(listener.Position.X, 1)
        XCTAssertEqual(listener.Position.Y, -2)
        XCTAssertEqual(listener.Position.Z, 3)

        listener.Velocity = F.Vector3(0, 0, -0.0)
        XCTAssertEqual(listener.Velocity.Z.bitPattern, Float(-0.0).bitPattern)
        listener.Velocity = F.Vector3(0, 0, 0)
        XCTAssertEqual(listener.Velocity.Z.bitPattern, Float(0).bitPattern)

        listener.Forward = F.Vector3(4, 5, 6)
        listener.Up = F.Vector3(7, 8, 9)
        XCTAssertEqual(listener.Forward.Z, 6)
        XCTAssertEqual(listener.Up.Z, 9)

        // No setter validates: XNA stores whatever it is given, including
        // non-normalized and non-finite vectors.
        listener.Forward = F.Vector3(.nan, .infinity, -.infinity)
        XCTAssertTrue(listener.Forward.X.isNaN)
        XCTAssertEqual(listener.Forward.Y, .infinity)
        XCTAssertEqual(listener.Forward.Z, -.infinity)

        // Reference semantics: two listeners are independent objects.
        let other = F.Audio.AudioListener()
        XCTAssertEqual(other.Forward.Z, -1)
    }

    // TouchCollection is a sequential value struct with eight inline
    // TouchLocation slots, a locationCount and an isConnected flag.
    func testTouchCollectionXnaContract() throws {
        typealias T = Microsoft.Xna.Framework.Input.Touch
        typealias F = Microsoft.Xna.Framework

        let first = T.TouchLocation(1, .Pressed, F.Vector2(10, 20))
        let second = T.TouchLocation(2, .Moved, F.Vector2(30, 40),
                                     .Pressed, F.Vector2(25, 35))
        let collection = try T.TouchCollection([first, second])

        // The constructor sets isConnected true and counts what it stored.
        XCTAssertTrue(collection.IsConnected)
        XCTAssertEqual(collection.Count, 2)
        // get_IsReadOnly is a constant ldc.i4.1.
        XCTAssertTrue(collection.IsReadOnly)

        // Entries are rebuilt through the seven-field constructor rather than
        // stored verbatim, but the observable fields survive.
        XCTAssertEqual(try collection.Item(0).Id, 1)
        XCTAssertEqual(try collection.Item(0).State, .Pressed)
        XCTAssertEqual(try collection.Item(0).Position.X, 10)
        XCTAssertEqual(try collection.Item(1).Id, 2)
        XCTAssertEqual(try collection.Item(1).Position.Y, 40)

        // A touch that carries a previous location keeps it; one that does not
        // is rebuilt with prevState Invalid and a zeroed previous position.
        var previous = T.TouchLocation(0, .Invalid, F.Vector2(0, 0))
        XCTAssertTrue(try collection.Item(1).TryGetPreviousLocation(&previous))
        XCTAssertEqual(previous.State, .Pressed)
        XCTAssertEqual(previous.Position.X, 25)
        XCTAssertFalse(try collection.Item(0).TryGetPreviousLocation(&previous))

        // get_Item validates index < 0 || index >= Count.
        XCTAssertThrowsError(try collection.Item(-1))
        XCTAssertThrowsError(try collection.Item(2))

        // FindById scans by Id and writes the match out.
        var found = T.TouchLocation(0, .Invalid, F.Vector2(0, 0))
        XCTAssertTrue(collection.FindById(2, touchLocation: &found))
        XCTAssertEqual(found.Id, 2)
        XCTAssertEqual(found.Position.X, 30)

        // When no location matches, XNA still writes the out parameter, with
        // default(TouchLocation): every field zeroed, so Id is 0 -- not the -1
        // that TouchLocation.TryGetPreviousLocation writes for an absent
        // previous location.
        XCTAssertFalse(collection.FindById(99, touchLocation: &found))
        XCTAssertEqual(found.Id, 0)
        XCTAssertEqual(found.State, .Invalid)
        XCTAssertEqual(found.Position.X, 0)
        XCTAssertEqual(found.Position.Y, 0)

        // IndexOf compares with op_Equality, the strict comparison that
        // includes both states, and returns -1 when absent. Contains is
        // IndexOf(item) >= 0.
        let rebuiltFirst = try collection.Item(0)
        XCTAssertEqual(collection.IndexOf(rebuiltFirst), 0)
        XCTAssertTrue(collection.Contains(rebuiltFirst))
        let strangerSameId = T.TouchLocation(1, .Released, F.Vector2(10, 20))
        XCTAssertEqual(collection.IndexOf(strangerSameId), -1)
        XCTAssertFalse(collection.Contains(strangerSameId))

        // CopyTo writes into the caller's array at arrayIndex.
        var destination = Array(repeating: T.TouchLocation(0, .Invalid, F.Vector2(0, 0)),
                                count: 4)
        try collection.CopyTo(&destination, arrayIndex: 1)
        XCTAssertEqual(destination[0].Id, 0)
        XCTAssertEqual(destination[1].Id, 1)
        XCTAssertEqual(destination[2].Id, 2)
        XCTAssertEqual(destination[3].Id, 0)

        // A negative index, and a destination too short for arrayIndex + Count,
        // both fail on "arrayIndex".
        XCTAssertThrowsError(try collection.CopyTo(&destination, arrayIndex: -1))
        XCTAssertThrowsError(try collection.CopyTo(&destination, arrayIndex: 3))
        // arrayIndex + Count == array.Length is exactly permitted.
        XCTAssertNoThrow(try collection.CopyTo(&destination, arrayIndex: 2))

        // Eight is the hard inline capacity; nine is rejected.
        let eight = (0..<8).map { T.TouchLocation(Int32($0), .Moved, F.Vector2(0, 0)) }
        XCTAssertEqual(try T.TouchCollection(eight).Count, 8)
        XCTAssertThrowsError(try T.TouchCollection(
            eight + [T.TouchLocation(8, .Moved, F.Vector2(0, 0))]))

        // An empty collection is still connected, and its indexer still throws.
        let empty = try T.TouchCollection([])
        XCTAssertEqual(empty.Count, 0)
        XCTAssertTrue(empty.IsConnected)
        XCTAssertThrowsError(try empty.Item(0))
    }

    // Every mutating member of the pinned IList implementation is exactly
    // `throw new NotSupportedException()` -- no message, no validation, so an
    // out-of-range index does not change which failure is reported.
    func testTouchCollectionMutationIsNotSupportedXnaContract() throws {
        typealias T = Microsoft.Xna.Framework.Input.Touch
        typealias F = Microsoft.Xna.Framework
        let item = T.TouchLocation(1, .Pressed, F.Vector2(1, 2))
        let collection = try T.TouchCollection([item])

        func expectNotSupported(_ body: () throws -> Void, _ what: String) {
            XCTAssertThrowsError(try body(), what) { error in
                guard let failure = error as? CNANotSupportedException else {
                    return XCTFail("\(what) did not report NotSupported: \(error)")
                }
                // The parameterless constructor, so the message is the
                // substituted Arg_NotSupportedException -- NOT the
                // NotSupported_ReadOnlyCollection that Collection<T>'s own
                // read-only guard raises.
                XCTAssertEqual(
                    failure.Message, "Specified method is not supported.", what)
                XCTAssertEqual(
                    failure.HResult, Int32(bitPattern: 0x8013_1515), what)
            }
        }

        expectNotSupported({ try collection.Add(item) }, "Add")
        expectNotSupported({ try collection.Clear() }, "Clear")
        expectNotSupported({ try collection.Insert(0, item: item) }, "Insert")
        expectNotSupported({ try collection.Insert(-5, item: item) }, "Insert out of range")
        expectNotSupported({ try collection.RemoveAt(0) }, "RemoveAt")
        expectNotSupported({ _ = try collection.Remove(item) }, "Remove")
        expectNotSupported({ try collection.SetItem(0, item) }, "SetItem")
        expectNotSupported({ try collection.SetItem(99, item) }, "SetItem out of range")

        // Nothing changed.
        XCTAssertEqual(collection.Count, 1)
        XCTAssertEqual(try collection.Item(0).Id, 1)
    }

    // Media.Video is a sealed descriptor whose only constructor is `assembly`.
    // Its five public identities each return a stored field verbatim.
    func testMediaVideoXnaContract() {
        typealias M = Microsoft.Xna.Framework.Media

        // The pinned constructor takes the duration as an int32 and builds the
        // stored TimeSpan with TimeSpan(0, 0, 0, 0, duration) -- the
        // days/hours/minutes/seconds/MILLISECONDS overload -- so the argument
        // is whole milliseconds.
        let video = M.Video(
            durationMilliseconds: 90_500, width: 1280, height: 720,
            framesPerSecond: 29.97, soundtrackType: .MusicAndDialog)

        XCTAssertEqual(video.Duration, .milliseconds(90_500))
        XCTAssertEqual(video.Duration, .seconds(90) + .milliseconds(500))
        XCTAssertEqual(video.Width, 1280)
        XCTAssertEqual(video.Height, 720)
        XCTAssertEqual(video.FramesPerSecond, 29.97)
        XCTAssertEqual(video.VideoSoundtrackType, .MusicAndDialog)

        // Nothing is validated or defaulted: the fields are stored verbatim.
        let degenerate = M.Video(
            durationMilliseconds: 0, width: 0, height: 0,
            framesPerSecond: 0, soundtrackType: .Music)
        XCTAssertEqual(degenerate.Duration, .zero)
        XCTAssertEqual(degenerate.Width, 0)
        XCTAssertEqual(degenerate.FramesPerSecond, 0)

        // A negative duration is representable and is stored as given.
        XCTAssertEqual(
            M.Video(durationMilliseconds: -1, width: 1, height: 1,
                    framesPerSecond: 1, soundtrackType: .Dialog).Duration,
            .milliseconds(-1))
    }

    // The nested Enumerator is a value struct holding a copy of the collection
    // and a cursor starting at -1.
    func testTouchCollectionEnumeratorXnaContract() throws {
        typealias T = Microsoft.Xna.Framework.Input.Touch
        typealias F = Microsoft.Xna.Framework
        let collection = try T.TouchCollection([
            T.TouchLocation(1, .Pressed, F.Vector2(1, 1)),
            T.TouchLocation(2, .Moved, F.Vector2(2, 2)),
        ])

        var enumerator = collection.GetEnumerator()
        // get_Current forwards straight to the indexer, so reading it before
        // the first MoveNext throws rather than returning a default.
        XCTAssertThrowsError(try enumerator.Current)

        XCTAssertTrue(enumerator.MoveNext())
        XCTAssertEqual(try enumerator.Current.Id, 1)
        XCTAssertTrue(enumerator.MoveNext())
        XCTAssertEqual(try enumerator.Current.Id, 2)
        XCTAssertFalse(enumerator.MoveNext())
        // Past the end the cursor is clamped to Count, so Current throws and
        // repeated calls keep returning false without running away.
        XCTAssertThrowsError(try enumerator.Current)
        XCTAssertFalse(enumerator.MoveNext())
        XCTAssertFalse(enumerator.MoveNext())
        XCTAssertThrowsError(try enumerator.Current)

        // Dispose is a single `ret`, and does not disturb the cursor.
        enumerator.Dispose()
        XCTAssertFalse(enumerator.MoveNext())

        // Each call returns a fresh cursor over a snapshot of the collection.
        var second = collection.GetEnumerator()
        XCTAssertTrue(second.MoveNext())
        XCTAssertEqual(try second.Current.Id, 1)

        // An empty collection yields nothing.
        var none = try T.TouchCollection([]).GetEnumerator()
        XCTAssertFalse(none.MoveNext())
        XCTAssertThrowsError(try none.Current)
    }
}
