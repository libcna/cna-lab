// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testGamePadEnumsAndButtonsOptionSet() {
        typealias I = Microsoft.Xna.Framework.Input
        XCTAssertEqual(I.ButtonState.Released.rawValue, 0)
        XCTAssertEqual(I.ButtonState.Pressed.rawValue, 1)
        XCTAssertEqual(I.GamePadDeadZone.None.rawValue, 0)
        XCTAssertEqual(I.GamePadDeadZone.IndependentAxes.rawValue, 1)
        XCTAssertEqual(I.GamePadDeadZone.Circular.rawValue, 2)
        XCTAssertEqual(I.GamePadType.Unknown.rawValue, 0)
        XCTAssertEqual(I.GamePadType.GamePad.rawValue, 1)
        XCTAssertEqual(I.GamePadType.Wheel.rawValue, 2)
        XCTAssertEqual(I.GamePadType.ArcadeStick.rawValue, 3)
        XCTAssertEqual(I.GamePadType.FlightStick.rawValue, 4)
        XCTAssertEqual(I.GamePadType.DancePad.rawValue, 5)
        XCTAssertEqual(I.GamePadType.Guitar.rawValue, 6)
        XCTAssertEqual(I.GamePadType.AlternateGuitar.rawValue, 7)
        XCTAssertEqual(I.GamePadType.DrumKit.rawValue, 8)
        XCTAssertEqual(I.GamePadType.BigButtonPad.rawValue, 768)

        XCTAssertEqual(I.Buttons.DPadUp.rawValue, 1)
        XCTAssertEqual(I.Buttons.DPadDown.rawValue, 2)
        XCTAssertEqual(I.Buttons.DPadLeft.rawValue, 4)
        XCTAssertEqual(I.Buttons.DPadRight.rawValue, 8)
        XCTAssertEqual(I.Buttons.Start.rawValue, 16)
        XCTAssertEqual(I.Buttons.Back.rawValue, 32)
        XCTAssertEqual(I.Buttons.LeftStick.rawValue, 64)
        XCTAssertEqual(I.Buttons.RightStick.rawValue, 128)
        XCTAssertEqual(I.Buttons.LeftShoulder.rawValue, 256)
        XCTAssertEqual(I.Buttons.RightShoulder.rawValue, 512)
        XCTAssertEqual(I.Buttons.BigButton.rawValue, 2_048)
        XCTAssertEqual(I.Buttons.LeftThumbstickLeft.rawValue, 2_097_152)
        XCTAssertEqual(I.Buttons.RightTrigger.rawValue, 4_194_304)
        XCTAssertEqual(I.Buttons.LeftTrigger.rawValue, 8_388_608)
        XCTAssertEqual(I.Buttons.RightThumbstickUp.rawValue, 16_777_216)
        XCTAssertEqual(I.Buttons.RightThumbstickDown.rawValue, 33_554_432)
        XCTAssertEqual(I.Buttons.RightThumbstickRight.rawValue, 67_108_864)
        XCTAssertEqual(I.Buttons.RightThumbstickLeft.rawValue, 134_217_728)
        XCTAssertEqual(I.Buttons.LeftThumbstickUp.rawValue, 268_435_456)
        XCTAssertEqual(I.Buttons.LeftThumbstickDown.rawValue, 536_870_912)
        XCTAssertEqual(I.Buttons.LeftThumbstickRight.rawValue, 1_073_741_824)
        XCTAssertEqual(I.Buttons.A.rawValue, 4_096)
        XCTAssertEqual(I.Buttons.B.rawValue, 8_192)
        XCTAssertEqual(I.Buttons.X.rawValue, 16_384)
        XCTAssertEqual(I.Buttons.Y.rawValue, 32_768)

        let combined: I.Buttons = [.A, .B, .DPadLeft, .LeftTrigger]
        XCTAssertTrue(combined.contains(.A))
        XCTAssertTrue(combined.contains(.B))
        XCTAssertTrue(combined.contains(.DPadLeft))
        XCTAssertTrue(combined.contains(.LeftTrigger))
        XCTAssertFalse(combined.contains(.X))
        XCTAssertEqual(combined.rawValue, 8_400_900)
        let undefined = I.Buttons(rawValue: 1_024 | 65_536)
        XCTAssertEqual(undefined.rawValue, 66_560)
        XCTAssertEqual(undefined.union(.A).rawValue, 70_656)
    }

    func testGamePadButtonsPhysicalPropertiesEqualityHashStringAndCopy() {
        typealias I = Microsoft.Xna.Framework.Input
        let mask: I.Buttons = [
            .A, .Back, .Y, .Start, .LeftShoulder, .LeftStick,
            .RightShoulder, .BigButton, .LeftTrigger, .RightThumbstickUp,
        ]
        let value = I.GamePadButtons(mask)
        XCTAssertEqual(value.A, .Pressed)
        XCTAssertEqual(value.B, .Released)
        XCTAssertEqual(value.Back, .Pressed)
        XCTAssertEqual(value.X, .Released)
        XCTAssertEqual(value.Y, .Pressed)
        XCTAssertEqual(value.Start, .Pressed)
        XCTAssertEqual(value.LeftShoulder, .Pressed)
        XCTAssertEqual(value.LeftStick, .Pressed)
        XCTAssertEqual(value.RightShoulder, .Pressed)
        XCTAssertEqual(value.RightStick, .Released)
        XCTAssertEqual(value.BigButton, .Pressed)
        XCTAssertEqual(
            value.ToString(),
            "{Buttons:A Y LeftShoulder RightShoulder LeftStick Start Back BigButton}"
        )
        XCTAssertTrue(value.Equals(value as Any))
        XCTAssertFalse(value.Equals(nil))
        XCTAssertFalse(value.Equals("buttons"))
        XCTAssertTrue(value == I.GamePadButtons(mask))
        XCTAssertFalse(value != I.GamePadButtons(mask))
        XCTAssertEqual(I.GamePadButtons(I.Buttons(rawValue: 0)).GetHashCode(), Int32.max)
        XCTAssertEqual(I.GamePadButtons([.A, .B, .X]).GetHashCode(), 1)
        let copied = value
        XCTAssertEqual(copied.A, .Pressed)
        XCTAssertEqual(copied.B, .Released)
        XCTAssertEqual(I.GamePadButtons([.LeftTrigger, .RightTrigger]).ToString(), "{Buttons:None}")
    }

    func testGamePadDPadAsymmetricOrderingEqualityHashStringAndCopy() {
        typealias I = Microsoft.Xna.Framework.Input
        let value = I.GamePadDPad(.Pressed, .Released, .Pressed, .Released)
        XCTAssertEqual(value.Up, .Pressed)
        XCTAssertEqual(value.Down, .Released)
        XCTAssertEqual(value.Right, .Released)
        XCTAssertEqual(value.Left, .Pressed)
        XCTAssertEqual(value.ToString(), "{DPad:Up Left}")
        XCTAssertEqual(value.GetHashCode(), Int32.max)
        XCTAssertTrue(value.Equals(I.GamePadDPad(.Pressed, .Released, .Pressed, .Released) as Any))
        XCTAssertFalse(value.Equals(I.GamePadDPad(.Pressed, .Released, .Released, .Pressed) as Any))
        XCTAssertFalse(value.Equals(nil))
        XCTAssertTrue(value == value)
        XCTAssertFalse(value != value)
        XCTAssertEqual(I.GamePadDPad(.Released, .Released, .Released, .Released).ToString(), "{DPad:None}")
        let copied = value
        XCTAssertEqual(copied.Left, .Pressed)
        XCTAssertEqual(copied.Right, .Released)
    }

    func testGamePadTriggersClampSpecialFloatsEqualityHashStringAndCopy() {
        typealias I = Microsoft.Xna.Framework.Input
        let ordinary = I.GamePadTriggers(-0.25, 1.25)
        XCTAssertEqual(ordinary.Left.bitPattern, Float(0).bitPattern)
        XCTAssertEqual(ordinary.Right.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(ordinary.ToString(), "{Left:0 Right:1}")
        XCTAssertEqual(ordinary.GetHashCode(), Int32(bitPattern: 0x3F80_0000))
        let midpoint = I.GamePadTriggers(0.5, -0.0)
        XCTAssertEqual(midpoint.Left.bitPattern, Float(0.5).bitPattern)
        XCTAssertEqual(midpoint.Right.bitPattern, Float(0).bitPattern)
        XCTAssertEqual(midpoint.GetHashCode(), Int32(bitPattern: 0x3F00_0000))
        let infinities = I.GamePadTriggers(.infinity, -.infinity)
        XCTAssertEqual(infinities.Left, 1)
        XCTAssertEqual(infinities.Right, 0)
        let payload = Float(bitPattern: 0x7FC1_2345)
        let nan = I.GamePadTriggers(payload, 0.5)
        XCTAssertEqual(nan.Left.bitPattern, payload.bitPattern)
        XCTAssertTrue(nan.Left.isNaN)
        XCTAssertFalse(nan.Equals(nan as Any))
        XCTAssertFalse(nan == nan)
        XCTAssertTrue(nan != nan)
        XCTAssertEqual(nan.GetHashCode(), Int32(bitPattern: 0x40C1_2345))
        XCTAssertEqual(nan.ToString(), "{Left:NaN Right:0.5}")
        XCTAssertFalse(ordinary.Equals(nil))
        XCTAssertFalse(ordinary.Equals("triggers"))
        XCTAssertTrue(ordinary.Equals(I.GamePadTriggers(0, 1) as Any))
        let copied = nan
        XCTAssertEqual(copied.Left.bitPattern, payload.bitPattern)
        XCTAssertEqual(copied.Right.bitPattern, Float(0.5).bitPattern)
    }

    func testGamePadThumbSticksSquareClampSpecialFloatsHashStringAndCopy() {
        typealias F = Microsoft.Xna.Framework
        typealias I = Microsoft.Xna.Framework.Input
        let ordinary = I.GamePadThumbSticks(F.Vector2(-2, 0.5), F.Vector2(2, -0.5))
        XCTAssertEqual(ordinary.Left.X, -1)
        XCTAssertEqual(ordinary.Left.Y, 0.5)
        XCTAssertEqual(ordinary.Right.X, 1)
        XCTAssertEqual(ordinary.Right.Y, -0.5)
        XCTAssertEqual(
            ordinary.ToString(),
            "{Left:{X:-1 Y:0.5} Right:{X:1 Y:-0.5}}"
        )
        XCTAssertEqual(ordinary.GetHashCode(), Int32.max)
        let diagonal = I.GamePadThumbSticks(F.Vector2(1, 1), F.Vector2(-1, -1))
        XCTAssertTrue(diagonal.Left == F.Vector2(1, 1))
        XCTAssertTrue(diagonal.Right == F.Vector2(-1, -1))
        let special = I.GamePadThumbSticks(
            F.Vector2(Float(bitPattern: 0x7FC1_2345), .infinity),
            F.Vector2(-.infinity, -0.0)
        )
        XCTAssertEqual(special.Left.X.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(special.Left.Y.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(special.Right.X.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(special.Right.Y.bitPattern, Float(-0.0).bitPattern)
        XCTAssertEqual(special.GetHashCode(), Int32(bitPattern: 0x3F80_0000))
        let signedZero = I.GamePadThumbSticks(F.Vector2(-0.0, 0), F.Vector2(0, 0))
        XCTAssertEqual(signedZero.GetHashCode(), Int32.min)
        XCTAssertTrue(ordinary.Equals(ordinary as Any))
        XCTAssertFalse(ordinary.Equals(nil))
        XCTAssertFalse(ordinary.Equals("sticks"))
        XCTAssertTrue(ordinary == ordinary)
        XCTAssertFalse(ordinary != ordinary)
        let copied = special
        XCTAssertEqual(copied.Left.X, 1)
        XCTAssertEqual(copied.Right.Y.bitPattern, Float(-0.0).bitPattern)
    }

    func testGamePadStateConstructorsPropertiesArrayUnionAndCopies() {
        typealias F = Microsoft.Xna.Framework
        typealias I = Microsoft.Xna.Framework.Input
        let thumbSticks = I.GamePadThumbSticks(F.Vector2(0.1, 0.2), F.Vector2(0.3, 0.4))
        let triggers = I.GamePadTriggers(0.5, 0.75)
        let buttons = I.GamePadButtons([.A, .LeftShoulder])
        let dPad = I.GamePadDPad(.Pressed, .Released, .Pressed, .Released)
        let components = I.GamePadState(thumbSticks, triggers, buttons, dPad)
        XCTAssertTrue(components.IsConnected)
        XCTAssertEqual(components.PacketNumber, 0)
        XCTAssertTrue(components.ThumbSticks == thumbSticks)
        XCTAssertTrue(components.Triggers == triggers)
        XCTAssertTrue(components.Buttons == buttons)
        XCTAssertTrue(components.DPad == dPad)
        XCTAssertEqual(components.ToString(), "{IsConnected:True}")

        let values = I.GamePadState(
            F.Vector2(-2, 0.5), F.Vector2(2, -0.5), -1, 2,
            [.A, [.B, .DPadRight], .A, I.Buttons(rawValue: 1_024)]
        )
        XCTAssertTrue(values.IsConnected)
        XCTAssertEqual(values.PacketNumber, 0)
        XCTAssertTrue(values.ThumbSticks.Left == F.Vector2(-1, 0.5))
        XCTAssertTrue(values.ThumbSticks.Right == F.Vector2(1, -0.5))
        XCTAssertEqual(values.Triggers.Left, 0)
        XCTAssertEqual(values.Triggers.Right, 1)
        XCTAssertEqual(values.Buttons.A, .Pressed)
        XCTAssertEqual(values.Buttons.B, .Pressed)
        XCTAssertEqual(values.DPad.Right, .Pressed)
        XCTAssertFalse(values.IsButtonDown(I.Buttons(rawValue: 1_024)))
        let empty = I.GamePadState(.Zero, .Zero, 0, 0, [])
        XCTAssertTrue(empty.IsConnected)
        XCTAssertEqual(empty.PacketNumber, 0)
        XCTAssertEqual(empty.GetHashCode(), 1)
        let copied = values
        XCTAssertEqual(copied.Buttons.A, .Pressed)
        XCTAssertEqual(copied.DPad.Right, .Pressed)
        XCTAssertEqual(copied.ThumbSticks.Left.X, -1)
        XCTAssertEqual(copied.Triggers.Right, 1)
    }

    func testGamePadStatePhysicalButtonQueriesAreExact() {
        typealias F = Microsoft.Xna.Framework
        typealias I = Microsoft.Xna.Framework.Input
        let state = I.GamePadState(
            F.Vector2.Zero, F.Vector2.Zero, 0, 0,
            [.A, .B, .X, .Y, .DPadUp, .DPadDown, .DPadLeft, .DPadRight,
             .Start, .Back, .LeftStick, .RightStick, .LeftShoulder,
             .RightShoulder, .BigButton]
        )
        XCTAssertTrue(state.IsButtonDown(.A))
        XCTAssertTrue(state.IsButtonDown(.B))
        XCTAssertTrue(state.IsButtonDown(.X))
        XCTAssertTrue(state.IsButtonDown(.Y))
        XCTAssertTrue(state.IsButtonDown(.DPadUp))
        XCTAssertTrue(state.IsButtonDown(.DPadDown))
        XCTAssertTrue(state.IsButtonDown(.DPadLeft))
        XCTAssertTrue(state.IsButtonDown(.DPadRight))
        XCTAssertTrue(state.IsButtonDown(.Start))
        XCTAssertTrue(state.IsButtonDown(.Back))
        XCTAssertTrue(state.IsButtonDown(.LeftStick))
        XCTAssertTrue(state.IsButtonDown(.RightStick))
        XCTAssertTrue(state.IsButtonDown(.LeftShoulder))
        XCTAssertTrue(state.IsButtonDown(.RightShoulder))
        XCTAssertTrue(state.IsButtonDown(.BigButton))
        XCTAssertTrue(state.IsButtonDown([.A, .B, .DPadLeft, .BigButton]))
        XCTAssertFalse(state.IsButtonDown([.A, .LeftTrigger]))
        XCTAssertTrue(state.IsButtonUp([.A, .LeftTrigger]))
        XCTAssertFalse(state.IsButtonUp([.A, .B]))
        XCTAssertTrue(state.IsButtonDown(I.Buttons(rawValue: 0)))
        XCTAssertFalse(state.IsButtonUp(I.Buttons(rawValue: 0)))
        XCTAssertFalse(state.IsButtonDown(I.Buttons(rawValue: 1_024)))
        XCTAssertTrue(state.IsButtonUp(I.Buttons(rawValue: 1_024)))
    }

    func testGamePadStateVirtualButtonQueriesAndBinary32Boundaries() {
        typealias F = Microsoft.Xna.Framework
        typealias I = Microsoft.Xna.Framework.Input
        let negativePositive = I.GamePadState(
            F.Vector2(-1, 1), F.Vector2(-1, 1), 1, 1, []
        )
        XCTAssertTrue(negativePositive.IsButtonDown(.LeftThumbstickLeft))
        XCTAssertTrue(negativePositive.IsButtonDown(.LeftThumbstickUp))
        XCTAssertTrue(negativePositive.IsButtonDown(.RightThumbstickLeft))
        XCTAssertTrue(negativePositive.IsButtonDown(.RightThumbstickUp))
        XCTAssertTrue(negativePositive.IsButtonDown(.LeftTrigger))
        XCTAssertTrue(negativePositive.IsButtonDown(.RightTrigger))
        XCTAssertTrue(negativePositive.IsButtonDown([
            .LeftThumbstickLeft, .RightThumbstickUp, .LeftTrigger,
        ]))

        let positiveNegative = I.GamePadState(
            F.Vector2(1, -1), F.Vector2(1, -1), 0, 0, []
        )
        XCTAssertTrue(positiveNegative.IsButtonDown(.LeftThumbstickRight))
        XCTAssertTrue(positiveNegative.IsButtonDown(.LeftThumbstickDown))
        XCTAssertTrue(positiveNegative.IsButtonDown(.RightThumbstickRight))
        XCTAssertTrue(positiveNegative.IsButtonDown(.RightThumbstickDown))
        XCTAssertFalse(positiveNegative.IsButtonDown(.LeftTrigger))
        XCTAssertFalse(positiveNegative.IsButtonDown(.RightTrigger))
        XCTAssertFalse(positiveNegative.IsButtonDown([
            .LeftThumbstickRight, .RightTrigger,
        ]))

        let leftAt = Float(7_849) / Float(32_767)
        let leftAbove = Float(7_850) / Float(32_767)
        let rightAt = Float(8_689) / Float(32_767)
        let rightAbove = Float(8_690) / Float(32_767)
        let triggerAt = Float(30) / Float(255)
        let triggerAbove = Float(31) / Float(255)
        let at = I.GamePadState(
            F.Vector2(leftAt, -leftAt), F.Vector2(rightAt, -rightAt),
            triggerAt, triggerAt, []
        )
        XCTAssertFalse(at.IsButtonDown(.LeftThumbstickRight))
        XCTAssertFalse(at.IsButtonDown(.LeftThumbstickDown))
        XCTAssertFalse(at.IsButtonDown(.RightThumbstickRight))
        XCTAssertFalse(at.IsButtonDown(.RightThumbstickDown))
        XCTAssertFalse(at.IsButtonDown(.LeftTrigger))
        XCTAssertFalse(at.IsButtonDown(.RightTrigger))
        let above = I.GamePadState(
            F.Vector2(leftAbove, -leftAbove), F.Vector2(rightAbove, -rightAbove),
            triggerAbove, triggerAbove, []
        )
        XCTAssertTrue(above.IsButtonDown(.LeftThumbstickRight))
        XCTAssertTrue(above.IsButtonDown(.LeftThumbstickDown))
        XCTAssertTrue(above.IsButtonDown(.RightThumbstickRight))
        XCTAssertTrue(above.IsButtonDown(.RightThumbstickDown))
        XCTAssertTrue(above.IsButtonDown(.LeftTrigger))
        XCTAssertTrue(above.IsButtonDown(.RightTrigger))
    }

    func testGamePadStateEqualityHashStringAndNativeSnapshotFieldSet() {
        typealias F = Microsoft.Xna.Framework
        typealias I = Microsoft.Xna.Framework.Input
        let thumbs = I.GamePadThumbSticks(F.Vector2(0.25, -0.5), F.Vector2(0.75, -1))
        let triggers = I.GamePadTriggers(0.125, 1)
        let buttons = I.GamePadButtons([.A, .B])
        let dPad = I.GamePadDPad(.Pressed, .Released, .Released, .Pressed)
        let base = I.GamePadState(
            thumbSticks: thumbs, triggers: triggers, buttons: buttons, dPad: dPad,
            connected: true, packet: 17, pressedButtons: [.A, .B, .DPadUp, .DPadRight]
        )
        XCTAssertTrue(base.IsConnected)
        XCTAssertEqual(base.PacketNumber, 17)
        XCTAssertEqual(base.ToString(), "{IsConnected:True}")
        XCTAssertTrue(base.Equals(base as Any))
        XCTAssertFalse(base.Equals(nil))
        XCTAssertFalse(base.Equals("state"))
        XCTAssertTrue(base == base)
        XCTAssertFalse(base != base)
        XCTAssertEqual(
            base.GetHashCode(),
            thumbs.GetHashCode() ^ triggers.GetHashCode() ^
                (buttons.GetHashCode() ^ 1) ^ (dPad.GetHashCode() ^ 17)
        )
        let disconnected = I.GamePadState(
            thumbSticks: thumbs, triggers: triggers, buttons: buttons, dPad: dPad,
            connected: false, packet: 17, pressedButtons: [.A, .B, .DPadUp, .DPadRight]
        )
        XCTAssertFalse(base == disconnected)
        XCTAssertEqual(disconnected.ToString(), "{IsConnected:False}")
        let packet = I.GamePadState(
            thumbSticks: thumbs, triggers: triggers, buttons: buttons, dPad: dPad,
            connected: true, packet: 18, pressedButtons: [.A, .B, .DPadUp, .DPadRight]
        )
        XCTAssertFalse(base == packet)
        XCTAssertFalse(base == I.GamePadState(
            thumbSticks: I.GamePadThumbSticks(.Zero, .Zero), triggers: triggers,
            buttons: buttons, dPad: dPad, connected: true, packet: 17,
            pressedButtons: [.A, .B, .DPadUp, .DPadRight]
        ))
        XCTAssertFalse(base == I.GamePadState(
            thumbSticks: thumbs, triggers: I.GamePadTriggers(0, 0),
            buttons: buttons, dPad: dPad, connected: true, packet: 17,
            pressedButtons: [.A, .B, .DPadUp, .DPadRight]
        ))
        XCTAssertFalse(base == I.GamePadState(
            thumbSticks: thumbs, triggers: triggers, buttons: I.GamePadButtons(.A),
            dPad: dPad, connected: true, packet: 17,
            pressedButtons: [.A, .DPadUp, .DPadRight]
        ))
        XCTAssertFalse(base == I.GamePadState(
            thumbSticks: thumbs, triggers: triggers, buttons: buttons,
            dPad: I.GamePadDPad(.Released, .Released, .Released, .Pressed),
            connected: true, packet: 17, pressedButtons: [.A, .B, .DPadRight]
        ))
        let samePublicDifferentPrivateMask = I.GamePadState(
            thumbSticks: thumbs, triggers: triggers, buttons: buttons, dPad: dPad,
            connected: true, packet: 17, pressedButtons: [.A]
        )
        XCTAssertTrue(base == samePublicDifferentPrivateMask)
    }

    func testGamePadCapabilitiesAllPropertiesAndValueCopy() {
        typealias I = Microsoft.Xna.Framework.Input
        let value = I.GamePadCapabilities(
            gamePadType: .BigButtonPad,
            isConnected: true,
            hasAButton: true,
            hasBackButton: false,
            hasBButton: true,
            hasDPadDownButton: false,
            hasDPadLeftButton: true,
            hasDPadRightButton: false,
            hasDPadUpButton: true,
            hasLeftShoulderButton: false,
            hasLeftStickButton: true,
            hasRightShoulderButton: false,
            hasRightStickButton: true,
            hasStartButton: false,
            hasXButton: true,
            hasYButton: false,
            hasBigButton: true,
            hasLeftXThumbStick: false,
            hasLeftYThumbStick: true,
            hasRightXThumbStick: false,
            hasRightYThumbStick: true,
            hasLeftTrigger: false,
            hasRightTrigger: true,
            hasLeftVibrationMotor: false,
            hasRightVibrationMotor: true,
            hasVoiceSupport: false
        )
        XCTAssertEqual(value.GamePadType, .BigButtonPad)
        XCTAssertTrue(value.IsConnected)
        XCTAssertTrue(value.HasAButton)
        XCTAssertFalse(value.HasBackButton)
        XCTAssertTrue(value.HasBButton)
        XCTAssertFalse(value.HasDPadDownButton)
        XCTAssertTrue(value.HasDPadLeftButton)
        XCTAssertFalse(value.HasDPadRightButton)
        XCTAssertTrue(value.HasDPadUpButton)
        XCTAssertFalse(value.HasLeftShoulderButton)
        XCTAssertTrue(value.HasLeftStickButton)
        XCTAssertFalse(value.HasRightShoulderButton)
        XCTAssertTrue(value.HasRightStickButton)
        XCTAssertFalse(value.HasStartButton)
        XCTAssertTrue(value.HasXButton)
        XCTAssertFalse(value.HasYButton)
        XCTAssertTrue(value.HasBigButton)
        XCTAssertFalse(value.HasLeftXThumbStick)
        XCTAssertTrue(value.HasLeftYThumbStick)
        XCTAssertFalse(value.HasRightXThumbStick)
        XCTAssertTrue(value.HasRightYThumbStick)
        XCTAssertFalse(value.HasLeftTrigger)
        XCTAssertTrue(value.HasRightTrigger)
        XCTAssertFalse(value.HasLeftVibrationMotor)
        XCTAssertTrue(value.HasRightVibrationMotor)
        XCTAssertFalse(value.HasVoiceSupport)
        let copied = value
        XCTAssertEqual(copied.GamePadType, .BigButtonPad)
        XCTAssertTrue(copied.HasBigButton)
        XCTAssertFalse(copied.HasVoiceSupport)
    }

    func testGamePadVibrationInputUsesPinnedUncheckedInt16Words() {
        typealias I = Microsoft.Xna.Framework.Input
        XCTAssertEqual(I.GamePad.xnaMotorStrength(-0.25).bitPattern,
                       (Float(49_153) / Float(65_535)).bitPattern)
        XCTAssertEqual(I.GamePad.xnaMotorStrength(0).bitPattern, Float(0).bitPattern)
        XCTAssertEqual(I.GamePad.xnaMotorStrength(0.5).bitPattern,
                       (Float(32_767) / Float(65_535)).bitPattern)
        XCTAssertEqual(I.GamePad.xnaMotorStrength(1).bitPattern, Float(1).bitPattern)
        XCTAssertEqual(I.GamePad.xnaMotorStrength(1.25).bitPattern,
                       (Float(16_382) / Float(65_535)).bitPattern)
        XCTAssertEqual(I.GamePad.xnaMotorStrength(.nan).bitPattern, Float(0).bitPattern)
        XCTAssertEqual(I.GamePad.xnaMotorStrength(.infinity).bitPattern, Float(0).bitPattern)
        XCTAssertEqual(I.GamePad.xnaMotorStrength(-.infinity).bitPattern, Float(0).bitPattern)
    }
}
