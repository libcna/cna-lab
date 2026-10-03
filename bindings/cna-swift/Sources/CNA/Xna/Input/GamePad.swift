// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Input {
    public final class GamePad {
        private init() {}

        public static func GetState(
            _ playerIndex: Microsoft.Xna.Framework.PlayerIndex
        ) throws -> GamePadState {
            let runtime = try RuntimeRegistry.current()
            try runtime.owner.validate("GamePad.GetState")
            var state = initializedNativeState()
            try runtime.functions.check(
                runtime.functions.gamePadGetState(
                    runtime.gameHandle,
                    nativePlayerIndex(playerIndex),
                    &state
                ),
                operation: "cna_gamepad_get_state"
            )
            return managedState(state)
        }

        public static func GetState(
            _ playerIndex: Microsoft.Xna.Framework.PlayerIndex,
            deadZoneMode: GamePadDeadZone
        ) throws -> GamePadState {
            let runtime = try RuntimeRegistry.current()
            try runtime.owner.validate("GamePad.GetState")
            var state = initializedNativeState()
            try runtime.functions.check(
                runtime.functions.gamePadGetStateWithDeadZone(
                    runtime.gameHandle,
                    nativePlayerIndex(playerIndex),
                    nativeDeadZone(deadZoneMode),
                    &state
                ),
                operation: "cna_gamepad_get_state_with_dead_zone"
            )
            return managedState(state)
        }

        public static func GetCapabilities(
            _ playerIndex: Microsoft.Xna.Framework.PlayerIndex
        ) throws -> GamePadCapabilities {
            let runtime = try RuntimeRegistry.current()
            try runtime.owner.validate("GamePad.GetCapabilities")
            var capabilities = CNASwift_GamePadCapabilities()
            capabilities.struct_size = UInt32(MemoryLayout<CNASwift_GamePadCapabilities>.size)
            capabilities.struct_version = 1
            try runtime.functions.check(
                runtime.functions.gamePadGetCapabilities(
                    runtime.gameHandle,
                    nativePlayerIndex(playerIndex),
                    &capabilities
                ),
                operation: "cna_gamepad_get_capabilities"
            )
            return managedCapabilities(capabilities)
        }

        public static func SetVibration(
            _ playerIndex: Microsoft.Xna.Framework.PlayerIndex,
            leftMotor: Float,
            rightMotor: Float
        ) throws -> Bool {
            let runtime = try RuntimeRegistry.current()
            try runtime.owner.validate("GamePad.SetVibration")
            var applied: UInt8 = 0
            try runtime.functions.check(
                runtime.functions.gamePadSetVibration(
                    runtime.gameHandle,
                    nativePlayerIndex(playerIndex),
                    xnaMotorStrength(leftMotor),
                    xnaMotorStrength(rightMotor),
                    &applied
                ),
                operation: "cna_gamepad_set_vibration"
            )
            return applied != 0
        }

        private static func initializedNativeState() -> CNASwift_GamePadState {
            var state = CNASwift_GamePadState()
            state.struct_size = UInt32(MemoryLayout<CNASwift_GamePadState>.size)
            state.struct_version = 1
            return state
        }

        private static func managedState(_ native: CNASwift_GamePadState) -> GamePadState {
            let pressed = managedButtons(native.pressed_buttons)
            return GamePadState(
                thumbSticks: GamePadThumbSticks(
                    Microsoft.Xna.Framework.Vector2(
                        native.analog.left_thumb_stick.x,
                        native.analog.left_thumb_stick.y
                    ),
                    Microsoft.Xna.Framework.Vector2(
                        native.analog.right_thumb_stick.x,
                        native.analog.right_thumb_stick.y
                    )
                ),
                triggers: GamePadTriggers(
                    native.analog.left_trigger,
                    native.analog.right_trigger
                ),
                buttons: GamePadButtons(pressed),
                dPad: GamePadDPad(buttons: pressed),
                connected: native.is_connected != 0,
                packet: native.packet_number,
                pressedButtons: pressed
            )
        }

        private static func managedButtons(_ native: UInt32) -> Buttons {
            var mapped = Buttons(rawValue: 0)
            let mappings: [(UInt32, Buttons)] = [
                (1, .DPadUp),
                (2, .DPadDown),
                (4, .DPadLeft),
                (8, .DPadRight),
                (16, .Start),
                (32, .Back),
                (64, .LeftStick),
                (128, .RightStick),
                (256, .LeftShoulder),
                (512, .RightShoulder),
                (2_048, .BigButton),
                (4_096, .A),
                (8_192, .B),
                (16_384, .X),
                (32_768, .Y),
                (2_097_152, .LeftThumbstickLeft),
                (4_194_304, .RightTrigger),
                (8_388_608, .LeftTrigger),
                (16_777_216, .RightThumbstickUp),
                (33_554_432, .RightThumbstickDown),
                (67_108_864, .RightThumbstickRight),
                (134_217_728, .RightThumbstickLeft),
                (268_435_456, .LeftThumbstickUp),
                (536_870_912, .LeftThumbstickDown),
                (1_073_741_824, .LeftThumbstickRight),
            ]
            for (cnaBit, xnaButton) in mappings where native & cnaBit == cnaBit {
                mapped.insert(xnaButton)
            }
            return mapped
        }

        private static func managedCapabilities(
            _ native: CNASwift_GamePadCapabilities
        ) -> GamePadCapabilities {
            GamePadCapabilities(
                gamePadType: managedGamePadType(native.gamepad_type),
                isConnected: native.is_connected != 0,
                hasAButton: native.has_a_button != 0,
                hasBackButton: native.has_back_button != 0,
                hasBButton: native.has_b_button != 0,
                hasDPadDownButton: native.has_dpad_down_button != 0,
                hasDPadLeftButton: native.has_dpad_left_button != 0,
                hasDPadRightButton: native.has_dpad_right_button != 0,
                hasDPadUpButton: native.has_dpad_up_button != 0,
                hasLeftShoulderButton: native.has_left_shoulder_button != 0,
                hasLeftStickButton: native.has_left_stick_button != 0,
                hasRightShoulderButton: native.has_right_shoulder_button != 0,
                hasRightStickButton: native.has_right_stick_button != 0,
                hasStartButton: native.has_start_button != 0,
                hasXButton: native.has_x_button != 0,
                hasYButton: native.has_y_button != 0,
                hasBigButton: native.has_big_button != 0,
                hasLeftXThumbStick: native.has_left_x_thumb_stick != 0,
                hasLeftYThumbStick: native.has_left_y_thumb_stick != 0,
                hasRightXThumbStick: native.has_right_x_thumb_stick != 0,
                hasRightYThumbStick: native.has_right_y_thumb_stick != 0,
                hasLeftTrigger: native.has_left_trigger != 0,
                hasRightTrigger: native.has_right_trigger != 0,
                hasLeftVibrationMotor: native.has_left_vibration_motor != 0,
                hasRightVibrationMotor: native.has_right_vibration_motor != 0,
                hasVoiceSupport: native.has_voice_support != 0
            )
        }

        private static func nativePlayerIndex(
            _ value: Microsoft.Xna.Framework.PlayerIndex
        ) -> UInt32 {
            switch value {
            case .One: return 0
            case .Two: return 1
            case .Three: return 2
            case .Four: return 3
            }
        }

        private static func nativeDeadZone(_ value: GamePadDeadZone) -> UInt32 {
            switch value {
            case .None: return 0
            case .IndependentAxes: return 1
            case .Circular: return 2
            }
        }

        private static func managedGamePadType(_ value: UInt32) -> GamePadType {
            switch value {
            case 1: return .GamePad
            case 2: return .Wheel
            case 3: return .ArcadeStick
            case 4: return .FlightStick
            case 5: return .DancePad
            case 6: return .Guitar
            case 7: return .AlternateGuitar
            case 8: return .DrumKit
            case 9: return .BigButtonPad
            default: return .Unknown
            }
        }

        // The pinned XNA method multiplies by 65535 and executes unchecked
        // conv.i2 before calling XInput. CNA's public route accepts normalized
        // motor strengths, so preserve the resulting 16-bit motor word and
        // map that word back into CNA's documented [0, 1] domain.
        internal static func xnaMotorStrength(_ value: Float) -> Float {
            let scaled = value * Float(65_535)
            guard scaled.isFinite,
                  scaled >= Float(-2_147_483_648),
                  scaled < Float(2_147_483_648) else {
                return 0
            }
            let converted = Int64(scaled)
            let word = UInt16(truncatingIfNeeded: converted)
            return Float(word) / Float(65_535)
        }
    }
}
