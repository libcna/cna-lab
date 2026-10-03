// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the Foundation 17 batch, transcribed from the
// five Microsoft XNA assemblies registered as authoritative reference inputs
// this milestone.
extension PureValueTests {
    func testAudioStopOptionsXnaContract() {
        typealias Options = Microsoft.Xna.Framework.Audio.AudioStopOptions
        let contract: [(name: String, value: Options, rawValue: Int32)] = [
            ("AsAuthored", .AsAuthored, 0),
            ("Immediate", .Immediate, 1),
        ]

        XCTAssertEqual(contract.count, 2)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }

        // The pinned binary carries no [Flags] attribute. That absence is
        // deliberate rather than accidental: it instead carries a suppression
        // of the CA1027 "MarkEnumsWithFlags" analyzer suggestion, so the two
        // literals are ordinary alternatives and not combinable bits.
        XCTAssertNil(Options(rawValue: 2))
        XCTAssertNil(Options(rawValue: 3))
    }

    func testVideoSoundtrackTypeXnaContract() {
        typealias Soundtrack = Microsoft.Xna.Framework.Media.VideoSoundtrackType
        let contract: [(name: String, value: Soundtrack, rawValue: Int32)] = [
            ("Music", .Music, 0),
            ("Dialog", .Dialog, 1),
            ("MusicAndDialog", .MusicAndDialog, 2),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }

        // MusicAndDialog is its own literal value 2, not the bitwise union of
        // Music and Dialog; the pinned enum is not a flags enum.
        XCTAssertNil(Soundtrack(rawValue: 3))
    }

    func testTouchLocationStateXnaContract() {
        typealias State = Microsoft.Xna.Framework.Input.Touch.TouchLocationState
        let contract: [(name: String, value: State, rawValue: Int32)] = [
            ("Invalid", .Invalid, 0),
            ("Released", .Released, 1),
            ("Pressed", .Pressed, 2),
            ("Moved", .Moved, 3),
        ]

        XCTAssertEqual(contract.count, 4)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }

        // Invalid, not Released, is the zero literal.
        XCTAssertEqual(State.Invalid.rawValue, 0)
        XCTAssertNil(State(rawValue: 4))
        XCTAssertNil(State(rawValue: -1))
    }

    func testGestureTypeXnaContract() {
        typealias Gesture = Microsoft.Xna.Framework.Input.Touch.GestureType
        let contract: [(name: String, value: Gesture, rawValue: Int32)] = [
            ("None", .None, 0),
            ("Tap", .Tap, 1),
            ("DoubleTap", .DoubleTap, 2),
            ("Hold", .Hold, 4),
            ("HorizontalDrag", .HorizontalDrag, 8),
            ("VerticalDrag", .VerticalDrag, 16),
            ("FreeDrag", .FreeDrag, 32),
            ("Pinch", .Pinch, 64),
            ("Flick", .Flick, 128),
            ("DragComplete", .DragComplete, 256),
            ("PinchComplete", .PinchComplete, 512),
        ]

        XCTAssertEqual(contract.count, 11)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }

        // The pinned binary carries System.FlagsAttribute, so the literals
        // combine. Every non-zero literal is a distinct single bit.
        var seen: Int32 = 0
        for entry in contract where entry.rawValue != 0 {
            XCTAssertEqual(entry.rawValue & (entry.rawValue - 1), 0, entry.name)
            XCTAssertEqual(seen & entry.rawValue, 0, entry.name)
            seen |= entry.rawValue
        }
        XCTAssertEqual(seen, 0x3FF)

        // Combination and membership follow the raw flags word.
        let drag: Gesture = [.HorizontalDrag, .VerticalDrag, .FreeDrag]
        XCTAssertEqual(drag.rawValue, 8 | 16 | 32)
        XCTAssertTrue(drag.contains(.VerticalDrag))
        XCTAssertFalse(drag.contains(.Pinch))

        // None is the zero literal and is the empty set.
        XCTAssertEqual(Gesture.None.rawValue, 0)
        XCTAssertTrue(Gesture.None.isEmpty)
        XCTAssertTrue(drag.contains(.None))

        // An unnamed bit is preserved verbatim; a flags enum has no closed
        // literal table.
        XCTAssertEqual(Gesture(rawValue: 0x400).rawValue, 0x400)
    }

    func testTouchPanelCapabilitiesXnaContract() {
        typealias Capabilities =
            Microsoft.Xna.Framework.Input.Touch.TouchPanelCapabilities

        // Both members are compiler-generated auto properties with private
        // setters, so the public contract is exactly two get-only properties
        // and nothing else — no constructor, no equality, no ToString.
        let capabilities = Capabilities(isConnected: true, maximumTouchCount: 4)
        XCTAssertTrue(capabilities.IsConnected)
        XCTAssertEqual(capabilities.MaximumTouchCount, 4)

        // The struct stores whatever it is given; it queries nothing. The one
        // producer in the pinned assembly, the internal static GetCaps(), is
        // `initobj` — it returns the all-zero value — but TouchPanel is not
        // implemented, so no capability is claimed here either way.
        let zero = Capabilities(isConnected: false, maximumTouchCount: 0)
        XCTAssertFalse(zero.IsConnected)
        XCTAssertEqual(zero.MaximumTouchCount, 0)

        // A CLR sequential value struct has value semantics.
        var copy = capabilities
        copy = zero
        XCTAssertTrue(capabilities.IsConnected)
        XCTAssertFalse(copy.IsConnected)
    }
}
