// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Input.Touch {
    // Pinned as `.class public sequential ansi sealed beforefieldinit ...
    // extends [mscorlib]System.ValueType implements IEquatable<TouchLocation>`.
    //
    // The position is stored as two separate `float32` fields, not as a
    // `Vector2`, and both the current and the previous location live in one
    // value: `id`, `state`, `x`, `y`, `prevState`, `prevX`, `prevY`. The
    // pinned `assembly` seven-argument constructor that sets all seven
    // directly is not public contract and is not projected.
    //
    // Completing this type claims no touch capability: `TouchPanel` is not
    // implemented, so nothing produces a location.
    public struct TouchLocation {
        private let id: Int32
        private let state: Microsoft.Xna.Framework.Input.Touch.TouchLocationState
        private let x: Float
        private let y: Float
        private let prevState: Microsoft.Xna.Framework.Input.Touch.TouchLocationState
        private let prevX: Float
        private let prevY: Float

        // Stores id and state, splits the position into x/y, and leaves the
        // previous location at `Invalid`/+0/+0. There is no validation.
        public init(
            _ id: Int32,
            _ state: Microsoft.Xna.Framework.Input.Touch.TouchLocationState,
            _ position: Microsoft.Xna.Framework.Vector2
        ) {
            self.id = id
            self.state = state
            x = position.X
            y = position.Y
            prevState = .Invalid
            prevX = 0
            prevY = 0
        }

        public init(
            _ id: Int32,
            _ state: Microsoft.Xna.Framework.Input.Touch.TouchLocationState,
            _ position: Microsoft.Xna.Framework.Vector2,
            _ previousState: Microsoft.Xna.Framework.Input.Touch.TouchLocationState,
            _ previousPosition: Microsoft.Xna.Framework.Vector2
        ) {
            self.id = id
            self.state = state
            x = position.X
            y = position.Y
            prevState = previousState
            prevX = previousPosition.X
            prevY = previousPosition.Y
        }

        // Internal seven-field construction, mirroring the pinned `assembly`
        // constructor. It is implementation infrastructure and never public.
        internal init(
            id: Int32,
            state: Microsoft.Xna.Framework.Input.Touch.TouchLocationState,
            x: Float,
            y: Float,
            prevState: Microsoft.Xna.Framework.Input.Touch.TouchLocationState,
            prevX: Float,
            prevY: Float
        ) {
            self.id = id
            self.state = state
            self.x = x
            self.y = y
            self.prevState = prevState
            self.prevX = prevX
            self.prevY = prevY
        }

        public var State: Microsoft.Xna.Framework.Input.Touch.TouchLocationState {
            state
        }

        public var Id: Int32 { id }

        // `get_Position` rebuilds a Vector2 from the two stored floats on
        // every read; there is no stored Vector2.
        public var Position: Microsoft.Xna.Framework.Vector2 {
            Microsoft.Xna.Framework.Vector2(x, y)
        }

        // The previous location is present only when `prevState` is not the
        // zero literal `Invalid`. When it is absent XNA still writes the out
        // parameter, with id -1 and every other field zeroed, and returns
        // false. When it is present the result carries this location's `id`,
        // the previous state and position promoted into the current slots, and
        // its own previous slots cleared — so a returned previous location
        // never itself has a previous location.
        public func TryGetPreviousLocation(
            _ previousLocation: inout TouchLocation
        ) -> Bool {
            guard prevState != .Invalid else {
                previousLocation = TouchLocation(
                    id: -1, state: .Invalid, x: 0, y: 0,
                    prevState: .Invalid, prevX: 0, prevY: 0)
                return false
            }
            previousLocation = TouchLocation(
                id: id, state: prevState, x: prevX, y: prevY,
                prevState: .Invalid, prevX: 0, prevY: 0)
            return true
        }

        // string.Format(CultureInfo.CurrentCulture, "{{Position:{0}}}",
        //   Position) — the doubled braces are composite-format escapes, and
        // the single argument is the boxed Vector2's own ToString().
        public func ToString() -> String {
            "{Position:\(Position.ToString())}"
        }

        // The typed Equals compares id, x, y, prevX and prevY and
        // deliberately does **not** compare either state field. This is not
        // symmetric with op_Equality below, which compares all seven; the
        // asymmetry is in the pinned IL and is preserved rather than
        // normalised.
        public func Equals(_ other: TouchLocation) -> Bool {
            id == other.id && x == other.x && y == other.y &&
                prevX == other.prevX && prevY == other.prevY
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? TouchLocation else { return false }
            return Equals(other)
        }

        // id.GetHashCode() + x.GetHashCode() + y.GetHashCode(), with CLR
        // unchecked int32 addition. `Single.GetHashCode()` is the raw bit
        // pattern except that both +0 and -0 hash to 0. Neither state field
        // nor the previous position participates.
        public func GetHashCode() -> Int32 {
            id &+ xnaFloatHash(x) &+ xnaFloatHash(y)
        }

        // op_Equality compares all seven fields, including both states, and
        // short-circuits on the first difference.
        public static func == (lhs: TouchLocation, rhs: TouchLocation) -> Bool {
            lhs.id == rhs.id && lhs.state == rhs.state &&
                lhs.x == rhs.x && lhs.y == rhs.y &&
                lhs.prevState == rhs.prevState &&
                lhs.prevX == rhs.prevX && lhs.prevY == rhs.prevY
        }

        public static func != (lhs: TouchLocation, rhs: TouchLocation) -> Bool {
            !(lhs == rhs)
        }
    }
}
