// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift language projection qualification for the Foundation 20 batch. These
// are Swift/CLR mapping facts, not XNA runtime observations, so they are
// deliberately kept out of the pure XNA-derived behaviour corpus.
final class Foundation20ProjectionTests: XCTestCase {
    typealias T = Microsoft.Xna.Framework.Input.Touch
    typealias F = Microsoft.Xna.Framework

    func testTouchCollectionMapsItsListInterfaceWithoutFakeTypes() throws {
        let collection = try T.TouchCollection([
            T.TouchLocation(1, .Pressed, F.Vector2(1, 2)),
        ])

        // The declared IList<TouchLocation> interface adds no automatic Swift
        // conformance: the concrete XNA members represent it.
        XCTAssertFalse(collection is any Swift.Sequence)
        XCTAssertFalse(collection is any Swift.Collection)
        XCTAssertFalse(collection is any Swift.RandomAccessCollection)

        // GetEnumerator returns the pinned nested struct, not the CNAEnumerator
        // support type, because the contract declares that concrete type.
        let enumerator = collection.GetEnumerator()
        XCTAssertTrue(type(of: enumerator) == T.TouchCollection.Enumerator.self)
        XCTAssertFalse(enumerator is any IteratorProtocol)
    }

    func testTouchCollectionIndexedPropertyUsesTheThrowingItemPair() throws {
        let collection = try T.TouchCollection([
            T.TouchLocation(1, .Pressed, F.Vector2(1, 2)),
        ])

        // The CLR indexer is read/write, and both accessors can throw, so it
        // expands to the throwing Item/SetItem pair rather than a Swift
        // subscript. Swift has no throwing setter accessor.
        XCTAssertEqual(try collection.Item(0).Id, 1)
        XCTAssertThrowsError(try collection.SetItem(
            0, T.TouchLocation(2, .Moved, F.Vector2(0, 0))))
    }

    func testTouchCollectionCopyToTakesACallerOwnedDestination() throws {
        let collection = try T.TouchCollection([
            T.TouchLocation(7, .Moved, F.Vector2(1, 2)),
        ])
        var destination = Array(
            repeating: T.TouchLocation(0, .Invalid, F.Vector2(0, 0)), count: 1)

        // The destination is `inout`: a value copy would silently discard the
        // write, which is the whole reason the array-mutation rule exists.
        try collection.CopyTo(&destination, arrayIndex: 0)

        XCTAssertEqual(destination[0].Id, 7)
    }

    func testTouchCollectionExposesNoPublicDefaultConstruction() throws {
        // The pinned type declares only .ctor(TouchLocation[]). The all-zero
        // CLR default value exists as a language artefact, and its Swift
        // counterpart is internal construction infrastructure.
        let defaulted = T.TouchCollection()
        XCTAssertFalse(defaulted.IsConnected)
        XCTAssertEqual(defaulted.Count, 0)

        // A constructed collection is always connected.
        XCTAssertTrue(try T.TouchCollection([]).IsConnected)
    }

    func testTouchCollectionIsAValueTypeWithIndependentCopies() throws {
        let collection = try T.TouchCollection([
            T.TouchLocation(1, .Pressed, F.Vector2(1, 2)),
        ])
        // A sequential CLR struct copies by value. Nothing can mutate a
        // TouchCollection after construction, so a copy is observationally
        // identical for its whole lifetime.
        let copy = collection
        XCTAssertEqual(copy.Count, collection.Count)
        XCTAssertEqual(try copy.Item(0).Id, try collection.Item(0).Id)
    }

    func testAudioListenerIsAReferenceTypeAndClaimsNoAudioCapability() {
        let listener = F.Audio.AudioListener()
        let alias = listener
        alias.Position = F.Vector3(5, 6, 7)

        // A CLR class projects to a Swift class, so the alias observes the
        // mutation. This is what distinguishes it from the value structs in the
        // same namespace.
        XCTAssertEqual(listener.Position.X, 5)
        XCTAssertTrue(alias === listener)

        // Naming and constructing the type requires no native library, no audio
        // device and no XACT engine: it is a data holder whose only XNA
        // consumer, Cue.Apply3D, is not implemented.
        XCTAssertEqual(String(describing: F.Audio.AudioListener.self), "AudioListener")
    }

    func testMediaVideoExposesNoPublicConstructionRoute() {
        typealias M = Microsoft.Xna.Framework.Media

        // The pinned class is sealed with an `assembly`-only constructor, so
        // the projection is final and exposes no public init. This closure
        // compiles only if all five identities are public with exactly these
        // names and types; it is deliberately never called with a value,
        // because no external construction route exists.
        let read: (M.Video) -> (Duration, Int32, Int32, Float, M.VideoSoundtrackType) = {
            ($0.Duration, $0.Width, $0.Height, $0.FramesPerSecond, $0.VideoSoundtrackType)
        }
        _ = read
        XCTAssertEqual(String(describing: M.Video.self), "Video")

        // A CLR class projects to a Swift class, so two descriptors built from
        // the same values are distinct objects.
        let first = M.Video(durationMilliseconds: 1, width: 2, height: 3,
                            framesPerSecond: 4, soundtrackType: .Music)
        let second = M.Video(durationMilliseconds: 1, width: 2, height: 3,
                             framesPerSecond: 4, soundtrackType: .Music)
        XCTAssertFalse(first === second)
        XCTAssertTrue(first === first)
    }

    func testFlipHandednessIsABitwiseInvolution() {
        // The support helper the accessors are built on. Applying it twice must
        // restore the exact bit pattern, which is what makes the XACT-space
        // storage invisible for round-trips.
        let samples: [Float] = [
            0, -0.0, 1, -1, .infinity, -.infinity,
            .leastNonzeroMagnitude, .leastNormalMagnitude, .greatestFiniteMagnitude,
        ]
        for z in samples {
            let once = F.Audio.flipHandedness(F.Vector3(1, 2, z))
            let twice = F.Audio.flipHandedness(once)
            XCTAssertEqual(twice.Z.bitPattern, z.bitPattern, "z = \(z)")
            XCTAssertEqual(twice.X, 1)
            XCTAssertEqual(twice.Y, 2)
        }
        let nan = F.Audio.flipHandedness(
            F.Audio.flipHandedness(F.Vector3(0, 0, .nan)))
        XCTAssertTrue(nan.Z.isNaN)
    }
}
