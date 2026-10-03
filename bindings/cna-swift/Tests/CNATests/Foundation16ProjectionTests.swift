// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift language projection qualification for the Foundation 16 batch. These
// are Swift/CLR mapping facts, not XNA runtime observations, so they are
// deliberately kept out of the pure XNA-derived behaviour corpus.
final class Foundation16ProjectionTests: XCTestCase {
    typealias State = Microsoft.Xna.Framework.Input.MouseState
    typealias Button = Microsoft.Xna.Framework.Input.ButtonState

    func testMouseStateProjectsAsAValueTypeWithUnlabelledConstructorArguments() {
        // A CLR sequential value struct maps to a Swift struct, and a
        // value-type constructor's external labels are all `_`. The call below
        // compiles only because every label is omitted.
        let state = State(5, 6, 7, .Pressed, .Released, .Pressed, .Released, .Pressed)
        let stateType = type(of: state)
        XCTAssertTrue(stateType == State.self)

        // Value semantics: assignment copies, and the copy is independent.
        var copy = state
        copy = State(0, 0, 0, .Released, .Released, .Released, .Released, .Released)
        XCTAssertEqual(state.X, 5)
        XCTAssertEqual(copy.X, 0)

        // Every mapped property's static Swift type is the projection of its
        // pinned CLR type: System.Int32 -> Int32 and ButtonState -> ButtonState.
        let x = state.X
        let wheel = state.ScrollWheelValue
        let left = state.LeftButton
        XCTAssertTrue(type(of: x) == Int32.self)
        XCTAssertTrue(type(of: wheel) == Int32.self)
        XCTAssertTrue(type(of: left) == Button.self)

        // Int32 is deliberately not widened to Swift's native Int.
        XCTAssertFalse(type(of: x) == Int.self)
    }

    func testMouseStateDeclaresNoSwiftEquatableOrHashableConformance() {
        // The pinned contract declares op_Equality, op_Inequality,
        // Equals(object), and GetHashCode, but no IEquatable<MouseState>.
        // The mapping therefore supplies those exact members and adds no
        // Swift Equatable or Hashable conformance and no typed Equals
        // overload.
        let state = State(1, 2, 3, .Pressed, .Released, .Pressed, .Released, .Pressed)
        XCTAssertFalse(state is any Equatable)
        XCTAssertFalse(state is any Hashable)

        // The XNA members themselves are present and are the only equality
        // surface.
        XCTAssertTrue(state == state)
        XCTAssertFalse(state != state)
        XCTAssertTrue(state.Equals(state))
        XCTAssertEqual(state.GetHashCode(), state.GetHashCode())
    }

    func testFoundation16EnumsProjectAsInt32RawEnumsAndNotOptionSets() {
        typealias Media = Microsoft.Xna.Framework.Media.MediaState
        typealias Source = Microsoft.Xna.Framework.Media.MediaSourceType
        typealias Mic = Microsoft.Xna.Framework.Audio.MicrophoneState

        // None of the three carries a [Flags] attribute in the pinned binary,
        // so each maps to a Swift enum with an Int32 raw type rather than to
        // an OptionSet struct. A raw value with no declared literal is nil,
        // which an OptionSet projection could never report.
        let mediaExpected: [(rawValue: Int32, value: Media)] = [
            (0, .Stopped), (1, .Playing), (2, .Paused),
        ]
        XCTAssertEqual(mediaExpected.count, 3)
        for entry in mediaExpected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(Media(rawValue: entry.rawValue), entry.value)
        }
        XCTAssertNil(Media(rawValue: 3))
        XCTAssertNil(Media(rawValue: -1))
        XCTAssertNil(Media(rawValue: Int32.max))

        let sourceExpected: [(rawValue: Int32, value: Source)] = [
            (0, .LocalDevice), (4, .WindowsMediaConnect),
        ]
        XCTAssertEqual(sourceExpected.count, 2)
        for entry in sourceExpected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(Source(rawValue: entry.rawValue), entry.value)
        }
        // The gap between the two pinned literals stays a gap. An OptionSet
        // projection would have silently accepted 1, 2, 3 and 5.
        XCTAssertNil(Source(rawValue: 1))
        XCTAssertNil(Source(rawValue: 5))

        let micExpected: [(rawValue: Int32, value: Mic)] = [
            (0, .Started), (1, .Stopped),
        ]
        XCTAssertEqual(micExpected.count, 2)
        for entry in micExpected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(Mic(rawValue: entry.rawValue), entry.value)
        }
        XCTAssertNil(Mic(rawValue: 2))

        // Enum value semantics.
        let original = Media.Playing
        var copy = original
        copy = .Paused
        XCTAssertEqual(original, .Playing)
        XCTAssertEqual(copy, .Paused)
    }
}
