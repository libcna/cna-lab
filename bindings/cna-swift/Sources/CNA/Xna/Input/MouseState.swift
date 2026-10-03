// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Input {
    // Pinned as `.class public sequential ansi sealed beforefieldinit
    // ... extends [mscorlib]System.ValueType`, so a Swift struct. The eight
    // backing fields are `assembly` and stay internal; the public surface is
    // the constructor, eight get-only properties, `GetHashCode`, `ToString`,
    // `Equals(object)`, and the two equality operators. Nothing else is
    // declared: no typed `Equals(MouseState)`, no `IEquatable`, no mutable
    // property, and no static factory.
    public struct MouseState {
        private let x: Int32
        private let y: Int32
        private let leftButton: ButtonState
        private let rightButton: ButtonState
        private let middleButton: ButtonState
        private let xb1: ButtonState
        private let xb2: ButtonState
        private let wheel: Int32

        // The pinned parameter order is `(x, y, scrollWheel, leftButton,
        // middleButton, rightButton, xButton1, xButton2)` — **middleButton
        // precedes rightButton**, which is not the order the property list
        // suggests. The internal Swift names reproduce the CLR metadata names
        // exactly and the verifier measures that order, so the pair cannot be
        // silently transposed. The body is eight plain field stores with no
        // validation of any kind.
        public init(
            _ x: Int32,
            _ y: Int32,
            _ scrollWheel: Int32,
            _ leftButton: ButtonState,
            _ middleButton: ButtonState,
            _ rightButton: ButtonState,
            _ xButton1: ButtonState,
            _ xButton2: ButtonState
        ) {
            self.x = x
            self.y = y
            wheel = scrollWheel
            self.leftButton = leftButton
            self.rightButton = rightButton
            self.middleButton = middleButton
            xb1 = xButton1
            xb2 = xButton2
        }

        public var X: Int32 { x }

        public var Y: Int32 { y }

        public var LeftButton: ButtonState { leftButton }

        public var RightButton: ButtonState { rightButton }

        public var MiddleButton: ButtonState { middleButton }

        public var XButton1: ButtonState { xb1 }

        public var XButton2: ButtonState { xb2 }

        public var ScrollWheelValue: Int32 { wheel }

        // `obj is MouseState ? op_Equality(this, (MouseState)obj) : false`.
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? MouseState else { return false }
            return self == other
        }

        // A plain XOR chain over the eight fields, in pinned IL order:
        // `Int32.GetHashCode()` is the value itself, and a boxed
        // `Int32`-backed CLR enum hashes to its underlying value. Unlike the
        // GamePad value types, MouseState calls no `SmartGetHashCode` helper,
        // so a zero result is returned as zero and is not substituted.
        public func GetHashCode() -> Int32 {
            x ^ y ^ leftButton.rawValue ^ rightButton.rawValue
                ^ middleButton.rawValue ^ xb1.rawValue ^ xb2.rawValue ^ wheel
        }

        // The button list is accumulated in the pinned order Left, Right,
        // Middle, XButton1, XButton2 — which is neither the constructor order
        // nor the property order — with a single space inserted only when the
        // accumulator is already non-empty, and "None" substituted when it
        // stays empty. The composite format string is
        // "{{X:{0} Y:{1} Buttons:{2} Wheel:{3}}}", whose doubled braces are
        // escapes, so exactly one brace pair is emitted.
        public func ToString() -> String {
            var names: [String] = []
            if leftButton == .Pressed { names.append("Left") }
            if rightButton == .Pressed { names.append("Right") }
            if middleButton == .Pressed { names.append("Middle") }
            if xb1 == .Pressed { names.append("XButton1") }
            if xb2 == .Pressed { names.append("XButton2") }
            let buttons = names.isEmpty ? "None" : names.joined(separator: " ")
            return "{X:\(x) Y:\(y) Buttons:\(buttons) Wheel:\(wheel)}"
        }

        // All eight fields participate; the first difference short-circuits.
        public static func == (lhs: MouseState, rhs: MouseState) -> Bool {
            lhs.x == rhs.x && lhs.y == rhs.y &&
                lhs.leftButton == rhs.leftButton &&
                lhs.rightButton == rhs.rightButton &&
                lhs.middleButton == rhs.middleButton &&
                lhs.xb1 == rhs.xb1 && lhs.xb2 == rhs.xb2 &&
                lhs.wheel == rhs.wheel
        }

        public static func != (lhs: MouseState, rhs: MouseState) -> Bool {
            !(lhs == rhs)
        }
    }
}
