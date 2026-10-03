// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Input {
    public enum ButtonState: Int32 {
        case Released = 0
        case Pressed = 1
    }

    public struct Buttons: OptionSet {
        public let rawValue: Int32

        public init(rawValue: Int32) { self.rawValue = rawValue }

        public static let DPadUp = Buttons(rawValue: 1)
        public static let DPadDown = Buttons(rawValue: 2)
        public static let DPadLeft = Buttons(rawValue: 4)
        public static let DPadRight = Buttons(rawValue: 8)
        public static let Start = Buttons(rawValue: 16)
        public static let Back = Buttons(rawValue: 32)
        public static let LeftStick = Buttons(rawValue: 64)
        public static let RightStick = Buttons(rawValue: 128)
        public static let LeftShoulder = Buttons(rawValue: 256)
        public static let RightShoulder = Buttons(rawValue: 512)
        public static let BigButton = Buttons(rawValue: 2_048)
        public static let A = Buttons(rawValue: 4_096)
        public static let B = Buttons(rawValue: 8_192)
        public static let X = Buttons(rawValue: 16_384)
        public static let Y = Buttons(rawValue: 32_768)
        public static let LeftThumbstickLeft = Buttons(rawValue: 2_097_152)
        public static let RightTrigger = Buttons(rawValue: 4_194_304)
        public static let LeftTrigger = Buttons(rawValue: 8_388_608)
        public static let RightThumbstickUp = Buttons(rawValue: 16_777_216)
        public static let RightThumbstickDown = Buttons(rawValue: 33_554_432)
        public static let RightThumbstickRight = Buttons(rawValue: 67_108_864)
        public static let RightThumbstickLeft = Buttons(rawValue: 134_217_728)
        public static let LeftThumbstickUp = Buttons(rawValue: 268_435_456)
        public static let LeftThumbstickDown = Buttons(rawValue: 536_870_912)
        public static let LeftThumbstickRight = Buttons(rawValue: 1_073_741_824)

        internal static let xnaDefinedMask = Buttons(rawValue: 0x7FE0_FBFF)
        internal static let xnaPhysicalMask = Buttons(rawValue: 0x0000_FBFF)
    }

    public enum GamePadDeadZone: Int32 {
        case None = 0
        case IndependentAxes = 1
        case Circular = 2
    }

    public enum GamePadType: Int32 {
        case Unknown = 0
        case GamePad = 1
        case Wheel = 2
        case ArcadeStick = 3
        case FlightStick = 4
        case DancePad = 5
        case Guitar = 6
        case AlternateGuitar = 7
        case DrumKit = 8
        case BigButtonPad = 768
    }

    public struct GamePadButtons {
        private let a: ButtonState
        private let b: ButtonState
        private let back: ButtonState
        private let x: ButtonState
        private let y: ButtonState
        private let start: ButtonState
        private let leftShoulder: ButtonState
        private let leftStick: ButtonState
        private let rightShoulder: ButtonState
        private let rightStick: ButtonState
        private let bigButton: ButtonState

        public init(_ buttons: Buttons) {
            a = Self.state(buttons, .A)
            b = Self.state(buttons, .B)
            back = Self.state(buttons, .Back)
            x = Self.state(buttons, .X)
            y = Self.state(buttons, .Y)
            start = Self.state(buttons, .Start)
            leftShoulder = Self.state(buttons, .LeftShoulder)
            leftStick = Self.state(buttons, .LeftStick)
            rightShoulder = Self.state(buttons, .RightShoulder)
            rightStick = Self.state(buttons, .RightStick)
            bigButton = Self.state(buttons, .BigButton)
        }

        public var A: ButtonState { a }
        public var B: ButtonState { b }
        public var Back: ButtonState { back }
        public var X: ButtonState { x }
        public var Y: ButtonState { y }
        public var Start: ButtonState { start }
        public var LeftShoulder: ButtonState { leftShoulder }
        public var LeftStick: ButtonState { leftStick }
        public var RightShoulder: ButtonState { rightShoulder }
        public var RightStick: ButtonState { rightStick }
        public var BigButton: ButtonState { bigButton }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? GamePadButtons else { return false }
            return self == other
        }

        public func GetHashCode() -> Int32 {
            let hash = UInt32(bitPattern: a.rawValue) ^ UInt32(bitPattern: b.rawValue) ^
                UInt32(bitPattern: back.rawValue) ^ UInt32(bitPattern: x.rawValue) ^
                UInt32(bitPattern: y.rawValue) ^ UInt32(bitPattern: start.rawValue) ^
                UInt32(bitPattern: leftShoulder.rawValue) ^ UInt32(bitPattern: leftStick.rawValue) ^
                UInt32(bitPattern: rightShoulder.rawValue) ^ UInt32(bitPattern: rightStick.rawValue) ^
                UInt32(bitPattern: bigButton.rawValue)
            return hash == 0 ? Int32.max : Int32(bitPattern: hash)
        }

        public func ToString() -> String {
            var names: [String] = []
            if a == .Pressed { names.append("A") }
            if b == .Pressed { names.append("B") }
            if x == .Pressed { names.append("X") }
            if y == .Pressed { names.append("Y") }
            if leftShoulder == .Pressed { names.append("LeftShoulder") }
            if rightShoulder == .Pressed { names.append("RightShoulder") }
            if leftStick == .Pressed { names.append("LeftStick") }
            if rightStick == .Pressed { names.append("RightStick") }
            if start == .Pressed { names.append("Start") }
            if back == .Pressed { names.append("Back") }
            if bigButton == .Pressed { names.append("BigButton") }
            return "{Buttons:\(names.isEmpty ? "None" : names.joined(separator: " "))}"
        }

        public static func == (lhs: GamePadButtons, rhs: GamePadButtons) -> Bool {
            lhs.a == rhs.a && lhs.b == rhs.b && lhs.back == rhs.back &&
                lhs.x == rhs.x && lhs.y == rhs.y && lhs.start == rhs.start &&
                lhs.leftShoulder == rhs.leftShoulder && lhs.leftStick == rhs.leftStick &&
                lhs.rightShoulder == rhs.rightShoulder && lhs.rightStick == rhs.rightStick &&
                lhs.bigButton == rhs.bigButton
        }

        public static func != (lhs: GamePadButtons, rhs: GamePadButtons) -> Bool { !(lhs == rhs) }

        internal var physicalMask: Buttons {
            var result = Buttons(rawValue: 0)
            if a == .Pressed { result.insert(.A) }
            if b == .Pressed { result.insert(.B) }
            if back == .Pressed { result.insert(.Back) }
            if x == .Pressed { result.insert(.X) }
            if y == .Pressed { result.insert(.Y) }
            if start == .Pressed { result.insert(.Start) }
            if leftShoulder == .Pressed { result.insert(.LeftShoulder) }
            if leftStick == .Pressed { result.insert(.LeftStick) }
            if rightShoulder == .Pressed { result.insert(.RightShoulder) }
            if rightStick == .Pressed { result.insert(.RightStick) }
            if bigButton == .Pressed { result.insert(.BigButton) }
            return result
        }

        private static func state(_ buttons: Buttons, _ button: Buttons) -> ButtonState {
            (buttons.rawValue & button.rawValue) == button.rawValue ? .Pressed : .Released
        }
    }

    public struct GamePadDPad {
        private let up: ButtonState
        private let right: ButtonState
        private let down: ButtonState
        private let left: ButtonState

        public init(
            _ upValue: ButtonState,
            _ downValue: ButtonState,
            _ leftValue: ButtonState,
            _ rightValue: ButtonState
        ) {
            up = upValue
            right = rightValue
            down = downValue
            left = leftValue
        }

        public var Up: ButtonState { up }
        public var Down: ButtonState { down }
        public var Right: ButtonState { right }
        public var Left: ButtonState { left }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? GamePadDPad else { return false }
            return self == other
        }

        public func GetHashCode() -> Int32 {
            let hash = UInt32(bitPattern: up.rawValue) ^ UInt32(bitPattern: right.rawValue) ^
                UInt32(bitPattern: down.rawValue) ^ UInt32(bitPattern: left.rawValue)
            return hash == 0 ? Int32.max : Int32(bitPattern: hash)
        }

        public func ToString() -> String {
            var names: [String] = []
            if up == .Pressed { names.append("Up") }
            if down == .Pressed { names.append("Down") }
            if left == .Pressed { names.append("Left") }
            if right == .Pressed { names.append("Right") }
            return "{DPad:\(names.isEmpty ? "None" : names.joined(separator: " "))}"
        }

        public static func == (lhs: GamePadDPad, rhs: GamePadDPad) -> Bool {
            lhs.up == rhs.up && lhs.right == rhs.right && lhs.down == rhs.down && lhs.left == rhs.left
        }

        public static func != (lhs: GamePadDPad, rhs: GamePadDPad) -> Bool { !(lhs == rhs) }

        internal var physicalMask: Buttons {
            var result = Buttons(rawValue: 0)
            if up == .Pressed { result.insert(.DPadUp) }
            if down == .Pressed { result.insert(.DPadDown) }
            if left == .Pressed { result.insert(.DPadLeft) }
            if right == .Pressed { result.insert(.DPadRight) }
            return result
        }

        internal init(buttons: Buttons) {
            self.init(
                buttons.contains(.DPadUp) ? .Pressed : .Released,
                buttons.contains(.DPadDown) ? .Pressed : .Released,
                buttons.contains(.DPadLeft) ? .Pressed : .Released,
                buttons.contains(.DPadRight) ? .Pressed : .Released
            )
        }
    }

    public struct GamePadTriggers {
        private let left: Float
        private let right: Float

        public init(_ leftTrigger: Float, _ rightTrigger: Float) {
            left = Self.clamp(leftTrigger)
            right = Self.clamp(rightTrigger)
        }

        public var Left: Float { left }
        public var Right: Float { right }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? GamePadTriggers else { return false }
            return self == other
        }

        public func GetHashCode() -> Int32 {
            let hash = left.bitPattern ^ right.bitPattern
            return hash == 0 ? Int32.max : Int32(bitPattern: hash)
        }

        public func ToString() -> String {
            "{Left:\(xnaFloatString(left)) Right:\(xnaFloatString(right))}"
        }

        public static func == (lhs: GamePadTriggers, rhs: GamePadTriggers) -> Bool {
            lhs.left == rhs.left && lhs.right == rhs.right
        }

        public static func != (lhs: GamePadTriggers, rhs: GamePadTriggers) -> Bool { !(lhs == rhs) }

        private static func clamp(_ value: Float) -> Float {
            if value.isNaN { return value }
            let upper = value < 1 ? value : Float(1)
            if upper.isNaN { return upper }
            return upper > 0 ? upper : Float(0)
        }
    }

    public struct GamePadThumbSticks {
        private let left: Microsoft.Xna.Framework.Vector2
        private let right: Microsoft.Xna.Framework.Vector2

        public init(
            _ leftThumbstick: Microsoft.Xna.Framework.Vector2,
            _ rightThumbstick: Microsoft.Xna.Framework.Vector2
        ) {
            let one = Microsoft.Xna.Framework.Vector2.One
            left = Microsoft.Xna.Framework.Vector2.Max(
                Microsoft.Xna.Framework.Vector2.Min(leftThumbstick, value2: one),
                value2: -one
            )
            right = Microsoft.Xna.Framework.Vector2.Max(
                Microsoft.Xna.Framework.Vector2.Min(rightThumbstick, value2: one),
                value2: -one
            )
        }

        public var Left: Microsoft.Xna.Framework.Vector2 { left }
        public var Right: Microsoft.Xna.Framework.Vector2 { right }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? GamePadThumbSticks else { return false }
            return self == other
        }

        public func GetHashCode() -> Int32 {
            let hash = left.X.bitPattern ^ left.Y.bitPattern ^
                right.X.bitPattern ^ right.Y.bitPattern
            return hash == 0 ? Int32.max : Int32(bitPattern: hash)
        }

        public func ToString() -> String {
            "{Left:\(left.ToString()) Right:\(right.ToString())}"
        }

        public static func == (lhs: GamePadThumbSticks, rhs: GamePadThumbSticks) -> Bool {
            lhs.left == rhs.left && lhs.right == rhs.right
        }

        public static func != (lhs: GamePadThumbSticks, rhs: GamePadThumbSticks) -> Bool {
            !(lhs == rhs)
        }
    }

    public struct GamePadCapabilities {
        private let gamePadType: GamePadType
        private let isConnected: Bool
        private let hasAButton: Bool
        private let hasBackButton: Bool
        private let hasBButton: Bool
        private let hasDPadDownButton: Bool
        private let hasDPadLeftButton: Bool
        private let hasDPadRightButton: Bool
        private let hasDPadUpButton: Bool
        private let hasLeftShoulderButton: Bool
        private let hasLeftStickButton: Bool
        private let hasRightShoulderButton: Bool
        private let hasRightStickButton: Bool
        private let hasStartButton: Bool
        private let hasXButton: Bool
        private let hasYButton: Bool
        private let hasBigButton: Bool
        private let hasLeftXThumbStick: Bool
        private let hasLeftYThumbStick: Bool
        private let hasRightXThumbStick: Bool
        private let hasRightYThumbStick: Bool
        private let hasLeftTrigger: Bool
        private let hasRightTrigger: Bool
        private let hasLeftVibrationMotor: Bool
        private let hasRightVibrationMotor: Bool
        private let hasVoiceSupport: Bool

        public var GamePadType: GamePadType { gamePadType }
        public var IsConnected: Bool { isConnected }
        public var HasAButton: Bool { hasAButton }
        public var HasBackButton: Bool { hasBackButton }
        public var HasBButton: Bool { hasBButton }
        public var HasDPadDownButton: Bool { hasDPadDownButton }
        public var HasDPadLeftButton: Bool { hasDPadLeftButton }
        public var HasDPadRightButton: Bool { hasDPadRightButton }
        public var HasDPadUpButton: Bool { hasDPadUpButton }
        public var HasLeftShoulderButton: Bool { hasLeftShoulderButton }
        public var HasLeftStickButton: Bool { hasLeftStickButton }
        public var HasRightShoulderButton: Bool { hasRightShoulderButton }
        public var HasRightStickButton: Bool { hasRightStickButton }
        public var HasStartButton: Bool { hasStartButton }
        public var HasXButton: Bool { hasXButton }
        public var HasYButton: Bool { hasYButton }
        public var HasBigButton: Bool { hasBigButton }
        public var HasLeftXThumbStick: Bool { hasLeftXThumbStick }
        public var HasLeftYThumbStick: Bool { hasLeftYThumbStick }
        public var HasRightXThumbStick: Bool { hasRightXThumbStick }
        public var HasRightYThumbStick: Bool { hasRightYThumbStick }
        public var HasLeftTrigger: Bool { hasLeftTrigger }
        public var HasRightTrigger: Bool { hasRightTrigger }
        public var HasLeftVibrationMotor: Bool { hasLeftVibrationMotor }
        public var HasRightVibrationMotor: Bool { hasRightVibrationMotor }
        public var HasVoiceSupport: Bool { hasVoiceSupport }

        internal init(
            gamePadType: GamePadType,
            isConnected: Bool,
            hasAButton: Bool,
            hasBackButton: Bool,
            hasBButton: Bool,
            hasDPadDownButton: Bool,
            hasDPadLeftButton: Bool,
            hasDPadRightButton: Bool,
            hasDPadUpButton: Bool,
            hasLeftShoulderButton: Bool,
            hasLeftStickButton: Bool,
            hasRightShoulderButton: Bool,
            hasRightStickButton: Bool,
            hasStartButton: Bool,
            hasXButton: Bool,
            hasYButton: Bool,
            hasBigButton: Bool,
            hasLeftXThumbStick: Bool,
            hasLeftYThumbStick: Bool,
            hasRightXThumbStick: Bool,
            hasRightYThumbStick: Bool,
            hasLeftTrigger: Bool,
            hasRightTrigger: Bool,
            hasLeftVibrationMotor: Bool,
            hasRightVibrationMotor: Bool,
            hasVoiceSupport: Bool
        ) {
            self.gamePadType = gamePadType
            self.isConnected = isConnected
            self.hasAButton = hasAButton
            self.hasBackButton = hasBackButton
            self.hasBButton = hasBButton
            self.hasDPadDownButton = hasDPadDownButton
            self.hasDPadLeftButton = hasDPadLeftButton
            self.hasDPadRightButton = hasDPadRightButton
            self.hasDPadUpButton = hasDPadUpButton
            self.hasLeftShoulderButton = hasLeftShoulderButton
            self.hasLeftStickButton = hasLeftStickButton
            self.hasRightShoulderButton = hasRightShoulderButton
            self.hasRightStickButton = hasRightStickButton
            self.hasStartButton = hasStartButton
            self.hasXButton = hasXButton
            self.hasYButton = hasYButton
            self.hasBigButton = hasBigButton
            self.hasLeftXThumbStick = hasLeftXThumbStick
            self.hasLeftYThumbStick = hasLeftYThumbStick
            self.hasRightXThumbStick = hasRightXThumbStick
            self.hasRightYThumbStick = hasRightYThumbStick
            self.hasLeftTrigger = hasLeftTrigger
            self.hasRightTrigger = hasRightTrigger
            self.hasLeftVibrationMotor = hasLeftVibrationMotor
            self.hasRightVibrationMotor = hasRightVibrationMotor
            self.hasVoiceSupport = hasVoiceSupport
        }
    }

    public struct GamePadState {
        private let connected: Bool
        private let packet: Int32
        private let thumbSticks: GamePadThumbSticks
        private let triggers: GamePadTriggers
        private let buttons: GamePadButtons
        private let dPad: GamePadDPad
        private let pressedButtons: Buttons

        public init(
            _ thumbSticks: GamePadThumbSticks,
            _ triggers: GamePadTriggers,
            _ buttons: GamePadButtons,
            _ dPad: GamePadDPad
        ) {
            self.init(
                thumbSticks: thumbSticks,
                triggers: triggers,
                buttons: buttons,
                dPad: dPad,
                connected: true,
                packet: 0,
                pressedButtons: Self.derivePressedButtons(
                    thumbSticks: thumbSticks,
                    triggers: triggers,
                    buttons: buttons,
                    dPad: dPad
                )
            )
        }

        public init(
            _ leftThumbStick: Microsoft.Xna.Framework.Vector2,
            _ rightThumbStick: Microsoft.Xna.Framework.Vector2,
            _ leftTrigger: Float,
            _ rightTrigger: Float,
            _ buttons: [Buttons]
        ) {
            let thumbSticks = GamePadThumbSticks(leftThumbStick, rightThumbStick)
            let triggers = GamePadTriggers(leftTrigger, rightTrigger)
            let combined = buttons.reduce(
                into: Microsoft.Xna.Framework.Input.Buttons(rawValue: 0)
            ) { $0.formUnion($1) }
            let gamePadButtons = GamePadButtons(combined)
            let dPad = GamePadDPad(buttons: combined)
            self.init(
                thumbSticks: thumbSticks,
                triggers: triggers,
                buttons: gamePadButtons,
                dPad: dPad,
                connected: true,
                packet: 0,
                pressedButtons: Self.derivePressedButtons(
                    thumbSticks: thumbSticks,
                    triggers: triggers,
                    buttons: gamePadButtons,
                    dPad: dPad
                )
            )
        }

        public var Buttons: GamePadButtons { buttons }
        public var DPad: GamePadDPad { dPad }
        public var IsConnected: Bool { connected }
        public var PacketNumber: Int32 { packet }
        public var ThumbSticks: GamePadThumbSticks { thumbSticks }
        public var Triggers: GamePadTriggers { triggers }

        public func IsButtonDown(_ button: Buttons) -> Bool {
            (pressedButtons.rawValue & button.rawValue) == button.rawValue
        }

        public func IsButtonUp(_ button: Buttons) -> Bool { !IsButtonDown(button) }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? GamePadState else { return false }
            return self == other
        }

        public func GetHashCode() -> Int32 {
            thumbSticks.GetHashCode() ^ triggers.GetHashCode() ^
                (buttons.GetHashCode() ^ (connected ? 1 : 0)) ^
                (dPad.GetHashCode() ^ packet)
        }

        public func ToString() -> String { "{IsConnected:\(connected ? "True" : "False")}" }

        public static func == (lhs: GamePadState, rhs: GamePadState) -> Bool {
            lhs.connected == rhs.connected && lhs.packet == rhs.packet &&
                lhs.thumbSticks == rhs.thumbSticks && lhs.triggers == rhs.triggers &&
                lhs.buttons == rhs.buttons && lhs.dPad == rhs.dPad
        }

        public static func != (lhs: GamePadState, rhs: GamePadState) -> Bool { !(lhs == rhs) }

        internal init(
            thumbSticks: GamePadThumbSticks,
            triggers: GamePadTriggers,
            buttons: GamePadButtons,
            dPad: GamePadDPad,
            connected: Bool,
            packet: Int32,
            pressedButtons: Buttons
        ) {
            self.thumbSticks = thumbSticks
            self.triggers = triggers
            self.buttons = buttons
            self.dPad = dPad
            self.connected = connected
            self.packet = packet
            self.pressedButtons = pressedButtons.intersection(.xnaDefinedMask)
        }

        private static func derivePressedButtons(
            thumbSticks: GamePadThumbSticks,
            triggers: GamePadTriggers,
            buttons: GamePadButtons,
            dPad: GamePadDPad
        ) -> Buttons {
            var result = buttons.physicalMask.union(dPad.physicalMask)
            let leftX = quantizedAxis(thumbSticks.Left.X)
            let leftY = quantizedAxis(thumbSticks.Left.Y)
            let rightX = quantizedAxis(thumbSticks.Right.X)
            let rightY = quantizedAxis(thumbSticks.Right.Y)
            if leftX < -7_849 { result.insert(.LeftThumbstickLeft) }
            if leftX > 7_849 { result.insert(.LeftThumbstickRight) }
            if leftY < -7_849 { result.insert(.LeftThumbstickDown) }
            if leftY > 7_849 { result.insert(.LeftThumbstickUp) }
            if rightX < -8_689 { result.insert(.RightThumbstickLeft) }
            if rightX > 8_689 { result.insert(.RightThumbstickRight) }
            if rightY < -8_689 { result.insert(.RightThumbstickDown) }
            if rightY > 8_689 { result.insert(.RightThumbstickUp) }
            if quantizedTrigger(triggers.Left) > 30 { result.insert(.LeftTrigger) }
            if quantizedTrigger(triggers.Right) > 30 { result.insert(.RightTrigger) }
            return result
        }

        private static func quantizedAxis(_ value: Float) -> Int32 {
            Int32(value * Float(32_767))
        }

        private static func quantizedTrigger(_ value: Float) -> Int32 {
            guard value.isFinite else { return 0 }
            return Int32(value * Float(255))
        }
    }
}
