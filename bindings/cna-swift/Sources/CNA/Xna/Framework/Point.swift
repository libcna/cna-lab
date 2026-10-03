// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Point {
        public var X: Int32
        public var Y: Int32

        public init(_ x: Int32, _ y: Int32) {
            X = x
            Y = y
        }

        public static var Zero: Point { Point(0, 0) }

        public func Equals(_ other: Point) -> Bool { self == other }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Point else { return false }
            return self == other
        }

        public func GetHashCode() -> Int32 { X ^ Y }

        public func ToString() -> String { "{X:\(X) Y:\(Y)}" }

        public static func == (lhs: Point, rhs: Point) -> Bool {
            lhs.X == rhs.X && lhs.Y == rhs.Y
        }

        public static func != (lhs: Point, rhs: Point) -> Bool { !(lhs == rhs) }
    }
}
