// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the Foundation 16 batch, transcribed from the
// pinned, hash-matched Microsoft.Xna.Framework.dll IL.
extension PureValueTests {
    func testMouseStateXnaContractConstructionAndProperties() {
        typealias State = Microsoft.Xna.Framework.Input.MouseState
        typealias Button = Microsoft.Xna.Framework.Input.ButtonState

        // The pinned parameter order is
        // (x, y, scrollWheel, leftButton, middleButton, rightButton,
        //  xButton1, xButton2)
        // — middleButton precedes rightButton. The constructor body is eight
        // plain field stores and validates nothing.
        let state = State(10, 20, 30, .Pressed, .Released, .Pressed, .Released, .Pressed)
        XCTAssertEqual(state.X, 10)
        XCTAssertEqual(state.Y, 20)
        XCTAssertEqual(state.ScrollWheelValue, 30)
        XCTAssertEqual(state.LeftButton, .Pressed)
        XCTAssertEqual(state.MiddleButton, .Released)
        XCTAssertEqual(state.RightButton, .Pressed)
        XCTAssertEqual(state.XButton1, .Released)
        XCTAssertEqual(state.XButton2, .Pressed)

        // The middle/right pair really is transposed relative to the property
        // order: the fifth argument lands on MiddleButton and the sixth on
        // RightButton.
        let transposed = State(0, 0, 0, .Released, .Pressed, .Released, .Released, .Released)
        XCTAssertEqual(transposed.MiddleButton, .Pressed)
        XCTAssertEqual(transposed.RightButton, .Released)

        // Nothing is clamped or rejected: negative coordinates, a negative
        // wheel, and the full Int32 range all round-trip verbatim.
        let extreme = State(
            Int32.min, Int32.max, -12345,
            .Released, .Released, .Released, .Released, .Released)
        XCTAssertEqual(extreme.X, Int32.min)
        XCTAssertEqual(extreme.Y, Int32.max)
        XCTAssertEqual(extreme.ScrollWheelValue, -12345)

        // Every declared property is get-only in the pinned contract, and the
        // struct is a value type, so a copy is fully independent.
        var copy = state
        copy = State(1, 2, 3, .Released, .Released, .Released, .Released, .Released)
        XCTAssertEqual(state.X, 10)
        XCTAssertEqual(copy.X, 1)
        XCTAssertEqual(Button.Released.rawValue, 0)
        XCTAssertEqual(Button.Pressed.rawValue, 1)
    }

    func testMouseStateXnaContractEquality() {
        typealias State = Microsoft.Xna.Framework.Input.MouseState

        // op_Equality compares all eight fields and short-circuits on the
        // first difference; op_Inequality is its negation.
        let base = State(3, 4, 5, .Pressed, .Released, .Pressed, .Released, .Pressed)
        XCTAssertTrue(base == State(3, 4, 5, .Pressed, .Released, .Pressed, .Released, .Pressed))
        XCTAssertFalse(base != State(3, 4, 5, .Pressed, .Released, .Pressed, .Released, .Pressed))

        // One differing field at a time, covering every one of the eight.
        for other in [
            State(9, 4, 5, .Pressed, .Released, .Pressed, .Released, .Pressed),
            State(3, 9, 5, .Pressed, .Released, .Pressed, .Released, .Pressed),
            State(3, 4, 9, .Pressed, .Released, .Pressed, .Released, .Pressed),
            State(3, 4, 5, .Released, .Released, .Pressed, .Released, .Pressed),
            State(3, 4, 5, .Pressed, .Pressed, .Pressed, .Released, .Pressed),
            State(3, 4, 5, .Pressed, .Released, .Released, .Released, .Pressed),
            State(3, 4, 5, .Pressed, .Released, .Pressed, .Pressed, .Pressed),
            State(3, 4, 5, .Pressed, .Released, .Pressed, .Released, .Released),
        ] {
            XCTAssertFalse(base == other)
            XCTAssertTrue(base != other)
        }

        // Equals(object) is false for null and for a different runtime type,
        // and otherwise defers to op_Equality.
        XCTAssertFalse(base.Equals(nil))
        XCTAssertFalse(base.Equals(42))
        XCTAssertFalse(base.Equals("MouseState"))
        XCTAssertTrue(base.Equals(State(3, 4, 5, .Pressed, .Released, .Pressed, .Released, .Pressed)))
        XCTAssertFalse(base.Equals(State(3, 4, 6, .Pressed, .Released, .Pressed, .Released, .Pressed)))
    }

    func testMouseStateXnaContractGetHashCode() {
        typealias State = Microsoft.Xna.Framework.Input.MouseState

        // A plain XOR chain over the eight fields. Int32.GetHashCode() is the
        // value itself and a boxed Int32-backed CLR enum hashes to its
        // underlying value, so the whole result is an ordinary Int32 XOR.
        XCTAssertEqual(
            State(10, 20, 30, .Pressed, .Released, .Pressed, .Released, .Pressed)
                .GetHashCode(), 1)
        XCTAssertEqual(
            State(1024, 768, 120, .Released, .Released, .Pressed, .Released, .Released)
                .GetHashCode(), 1913)
        XCTAssertEqual(
            State(-1, -2, -3, .Pressed, .Pressed, .Released, .Released, .Pressed)
                .GetHashCode(), -3)
        XCTAssertEqual(
            State(Int32.max, Int32.min, 0, .Released, .Released, .Released, .Released, .Released)
                .GetHashCode(), -1)

        // MouseState calls no SmartGetHashCode helper, so unlike the GamePad
        // value types a zero XOR is returned as 0 and is NOT substituted with
        // Int32.max. Both a fully default state and a non-trivial state that
        // happens to XOR to zero prove it.
        XCTAssertEqual(
            State(0, 0, 0, .Released, .Released, .Released, .Released, .Released)
                .GetHashCode(), 0)
        XCTAssertEqual(
            State(-5, 7, -3, .Pressed, .Pressed, .Pressed, .Pressed, .Pressed)
                .GetHashCode(), 0)

        // Equal states hash equally.
        let a = State(7, 8, 9, .Pressed, .Released, .Pressed, .Pressed, .Released)
        let b = State(7, 8, 9, .Pressed, .Released, .Pressed, .Pressed, .Released)
        XCTAssertEqual(a.GetHashCode(), b.GetHashCode())
    }

    func testMouseStateXnaContractToString() {
        typealias State = Microsoft.Xna.Framework.Input.MouseState

        // The composite format is "{{X:{0} Y:{1} Buttons:{2} Wheel:{3}}}";
        // the doubled braces are escapes, so one brace pair is emitted.
        // The button list is accumulated in the pinned order
        // Left, Right, Middle, XButton1, XButton2 — neither the constructor
        // order nor the property order — with a single separating space.
        XCTAssertEqual(
            State(10, 20, 30, .Pressed, .Released, .Pressed, .Released, .Pressed).ToString(),
            "{X:10 Y:20 Buttons:Left Right XButton2 Wheel:30}")

        // No pressed button substitutes the literal "None".
        XCTAssertEqual(
            State(0, 0, 0, .Released, .Released, .Released, .Released, .Released).ToString(),
            "{X:0 Y:0 Buttons:None Wheel:0}")

        // Every button pressed emits the full list in pinned order. Note the
        // constructor's middle/right transposition is invisible here because
        // all five are pressed.
        XCTAssertEqual(
            State(1, 2, 3, .Pressed, .Pressed, .Pressed, .Pressed, .Pressed).ToString(),
            "{X:1 Y:2 Buttons:Left Right Middle XButton1 XButton2 Wheel:3}")

        // A single pressed button emits no leading or trailing space.
        XCTAssertEqual(
            State(0, 0, 0, .Released, .Pressed, .Released, .Released, .Released).ToString(),
            "{X:0 Y:0 Buttons:Middle Wheel:0}")
        XCTAssertEqual(
            State(0, 0, 0, .Released, .Released, .Released, .Pressed, .Released).ToString(),
            "{X:0 Y:0 Buttons:XButton1 Wheel:0}")

        // Middle sorts after Right in the emitted list even though it comes
        // first in the constructor.
        XCTAssertEqual(
            State(0, 0, 0, .Released, .Pressed, .Pressed, .Released, .Released).ToString(),
            "{X:0 Y:0 Buttons:Right Middle Wheel:0}")

        // Negative coordinates and wheel render with a leading minus.
        XCTAssertEqual(
            State(-4, -9, -1, .Released, .Released, .Released, .Released, .Released).ToString(),
            "{X:-4 Y:-9 Buttons:None Wheel:-1}")
    }

    func testMediaStateXnaContract() {
        typealias State = Microsoft.Xna.Framework.Media.MediaState
        let contract: [(name: String, value: State, rawValue: Int32)] = [
            ("Stopped", .Stopped, 0),
            ("Playing", .Playing, 1),
            ("Paused", .Paused, 2),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testMediaSourceTypeXnaContract() {
        typealias Source = Microsoft.Xna.Framework.Media.MediaSourceType
        let contract: [(name: String, value: Source, rawValue: Int32)] = [
            ("LocalDevice", .LocalDevice, 0),
            ("WindowsMediaConnect", .WindowsMediaConnect, 4),
        ]

        XCTAssertEqual(contract.count, 2)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }

        // The two pinned literals are not contiguous, and no literal exists
        // for the intervening values.
        XCTAssertNil(Source(rawValue: 1))
        XCTAssertNil(Source(rawValue: 2))
        XCTAssertNil(Source(rawValue: 3))
    }

    func testMicrophoneStateXnaContract() {
        typealias State = Microsoft.Xna.Framework.Audio.MicrophoneState
        let contract: [(name: String, value: State, rawValue: Int32)] = [
            ("Started", .Started, 0),
            ("Stopped", .Stopped, 1),
        ]

        XCTAssertEqual(contract.count, 2)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }

        // Started, not Stopped, is the zero literal.
        XCTAssertEqual(State.Started.rawValue, 0)
    }
}
