// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Rectangle {
        public var X: Int32
        public var Y: Int32
        public var Width: Int32
        public var Height: Int32

        public init(_ x: Int32, _ y: Int32, _ width: Int32, _ height: Int32) {
            X = x
            Y = y
            Width = width
            Height = height
        }

        public var Left: Int32 { X }
        public var Right: Int32 { X &+ Width }
        public var Top: Int32 { Y }
        public var Bottom: Int32 { Y &+ Height }

        public var Location: Point {
            get { Point(X, Y) }
            set { X = newValue.X; Y = newValue.Y }
        }

        public var Center: Point {
            Point(X &+ Width / 2, Y &+ Height / 2)
        }

        public static var Empty: Rectangle { Rectangle(0, 0, 0, 0) }
        public var IsEmpty: Bool { X == 0 && Y == 0 && Width == 0 && Height == 0 }

        public mutating func Offset(_ amount: Point) {
            X = X &+ amount.X
            Y = Y &+ amount.Y
        }

        public mutating func Offset(_ offsetX: Int32, offsetY: Int32) {
            X = X &+ offsetX
            Y = Y &+ offsetY
        }

        public mutating func Inflate(_ horizontalAmount: Int32, verticalAmount: Int32) {
            X = X &- horizontalAmount
            Y = Y &- verticalAmount
            Width = Width &+ horizontalAmount &+ horizontalAmount
            Height = Height &+ verticalAmount &+ verticalAmount
        }

        public func Contains(_ x: Int32, y: Int32) -> Bool {
            X <= x && x < X &+ Width && Y <= y && y < Y &+ Height
        }

        public func Contains(_ value: Point) -> Bool { Contains(value.X, y: value.Y) }

        public func Contains(_ value: inout Point, result: inout Bool) {
            result = Contains(value)
        }

        public func Contains(_ value: Rectangle) -> Bool {
            X <= value.X && value.Right <= Right && Y <= value.Y && value.Bottom <= Bottom
        }

        public func Contains(_ value: inout Rectangle, result: inout Bool) {
            result = Contains(value)
        }

        public func Intersects(_ value: Rectangle) -> Bool {
            value.Left < Right && Left < value.Right && value.Top < Bottom && Top < value.Bottom
        }

        public func Intersects(_ value: inout Rectangle, result: inout Bool) {
            result = Intersects(value)
        }

        public static func Intersect(_ value1: Rectangle, value2: Rectangle) -> Rectangle {
            if !value1.Intersects(value2) { return Empty }
            let right = Swift.min(value1.Right, value2.Right)
            let bottom = Swift.min(value1.Bottom, value2.Bottom)
            let left = Swift.max(value1.Left, value2.Left)
            let top = Swift.max(value1.Top, value2.Top)
            return Rectangle(left, top, right &- left, bottom &- top)
        }

        public static func Intersect(
            _ value1: inout Rectangle,
            value2: inout Rectangle,
            result: inout Rectangle
        ) {
            result = Intersect(value1, value2: value2)
        }

        public static func Union(_ value1: Rectangle, value2: Rectangle) -> Rectangle {
            let left = Swift.min(value1.Left, value2.Left)
            let top = Swift.min(value1.Top, value2.Top)
            let right = Swift.max(value1.Right, value2.Right)
            let bottom = Swift.max(value1.Bottom, value2.Bottom)
            return Rectangle(left, top, right &- left, bottom &- top)
        }

        public static func Union(
            _ value1: inout Rectangle,
            value2: inout Rectangle,
            result: inout Rectangle
        ) {
            result = Union(value1, value2: value2)
        }

        public func Equals(_ other: Rectangle) -> Bool { self == other }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Rectangle else { return false }
            return self == other
        }

        public func ToString() -> String {
            "{X:\(X) Y:\(Y) Width:\(Width) Height:\(Height)}"
        }

        public func GetHashCode() -> Int32 { X ^ Y ^ Width ^ Height }

        public static func == (lhs: Rectangle, rhs: Rectangle) -> Bool {
            lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Width == rhs.Width && lhs.Height == rhs.Height
        }

        public static func != (lhs: Rectangle, rhs: Rectangle) -> Bool { !(lhs == rhs) }
    }
}
