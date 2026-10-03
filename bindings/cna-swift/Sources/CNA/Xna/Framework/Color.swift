// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework {
    /// XNA's packed little-endian RGBA color value.
    public struct Color: Microsoft.Xna.Framework.Graphics.PackedVector.IPackedVectorOfT {
        public typealias TPacked = UInt32

        private var packedValue: UInt32

        public var R: UInt8 {
            get { UInt8(truncatingIfNeeded: packedValue) }
            set { packedValue = (packedValue & 0xFFFF_FF00) | UInt32(newValue) }
        }

        public var G: UInt8 {
            get { UInt8(truncatingIfNeeded: packedValue >> 8) }
            set { packedValue = (packedValue & 0xFFFF_00FF) | (UInt32(newValue) << 8) }
        }

        public var B: UInt8 {
            get { UInt8(truncatingIfNeeded: packedValue >> 16) }
            set { packedValue = (packedValue & 0xFF00_FFFF) | (UInt32(newValue) << 16) }
        }

        public var A: UInt8 {
            get { UInt8(truncatingIfNeeded: packedValue >> 24) }
            set { packedValue = (packedValue & 0x00FF_FFFF) | (UInt32(newValue) << 24) }
        }

        public var PackedValue: UInt32 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ r: Int32, _ g: Int32, _ b: Int32) {
            self.init(r, g, b, 255)
        }

        public init(_ r: Int32, _ g: Int32, _ b: Int32, _ a: Int32) {
            packedValue = Self.packIntegers(r, g, b, a)
        }

        public init(_ r: Float, _ g: Float, _ b: Float) {
            self.init(r, g, b, 1)
        }

        public init(_ r: Float, _ g: Float, _ b: Float, _ a: Float) {
            packedValue = Self.packFloats(r, g, b, a)
        }

        public init(_ vector: Vector3) {
            packedValue = Self.packFloats(vector.X, vector.Y, vector.Z, 1)
        }

        public init(_ vector: Vector4) {
            packedValue = Self.packFloats(vector.X, vector.Y, vector.Z, vector.W)
        }

        internal init(packedValue: UInt32) { self.packedValue = packedValue }

        public mutating func PackFromVector4(_ vector: Vector4) {
            packedValue = Self.packFloats(vector.X, vector.Y, vector.Z, vector.W)
        }

        public static func FromNonPremultiplied(_ vector: Vector4) -> Color {
            Color(
                packedValue: packFloats(
                    vector.X * vector.W,
                    vector.Y * vector.W,
                    vector.Z * vector.W,
                    vector.W
                )
            )
        }

        public static func FromNonPremultiplied(
            _ r: Int32,
            g: Int32,
            b: Int32,
            a: Int32
        ) -> Color {
            let alpha = Int64(a)
            let red = clampToByte(Int64(r) * alpha / 255)
            let green = clampToByte(Int64(g) * alpha / 255)
            let blue = clampToByte(Int64(b) * alpha / 255)
            let packedAlpha = clampToByte(alpha)
            return Color(
                packedValue: red | (green << 8) | (blue << 16) | (packedAlpha << 24)
            )
        }

        public func ToVector3() -> Vector3 {
            Vector3(
                Float(R) / Float(255),
                Float(G) / Float(255),
                Float(B) / Float(255)
            )
        }

        public func ToVector4() -> Vector4 {
            Vector4(
                Float(R) / Float(255),
                Float(G) / Float(255),
                Float(B) / Float(255),
                Float(A) / Float(255)
            )
        }

        public static func Lerp(_ value1: Color, value2: Color, amount: Float) -> Color {
            let factor = Int32(packUNorm(Float(65_536), amount))

            func interpolate(_ first: UInt8, _ second: UInt8) -> UInt32 {
                let start = Int32(first)
                let delta = Int32(second) - start
                return UInt32(start + ((delta * factor) >> 16))
            }

            let red = interpolate(value1.R, value2.R)
            let green = interpolate(value1.G, value2.G)
            let blue = interpolate(value1.B, value2.B)
            let alpha = interpolate(value1.A, value2.A)
            return Color(
                packedValue: red | (green << 8) | (blue << 16) | (alpha << 24)
            )
        }

        public static func Multiply(_ value: Color, scale: Float) -> Color {
            let scaled = scale * Float(65_536)
            let factor: UInt32
            if scaled.isNaN || scaled < 0 {
                factor = 0
            } else if scaled > 16_777_215 {
                factor = 16_777_215
            } else {
                factor = UInt32(scaled)
            }
            func channel(_ value: UInt8) -> UInt32 {
                Swift.min((UInt32(value) * factor) >> 16, 255)
            }
            return Color(packedValue: channel(value.R) | (channel(value.G) << 8) |
                (channel(value.B) << 16) | (channel(value.A) << 24))
        }

        public func Equals(_ other: Color) -> Bool { self == other }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Color else { return false }
            return self == other
        }

        public func GetHashCode() -> Int32 { Int32(bitPattern: packedValue) }
        public func ToString() -> String { "{R:\(R) G:\(G) B:\(B) A:\(A)}" }

        public static func * (lhs: Color, rhs: Float) -> Color { Multiply(lhs, scale: rhs) }
        public static func == (lhs: Color, rhs: Color) -> Bool { lhs.packedValue == rhs.packedValue }
        public static func != (lhs: Color, rhs: Color) -> Bool { !(lhs == rhs) }

        internal var native: CNASwift_Color { CNASwift_Color(r: R, g: G, b: B, a: A) }

        private static func packIntegers(_ r: Int32, _ g: Int32, _ b: Int32, _ a: Int32) -> UInt32 {
            let red = clampToByte(Int64(r))
            let green = clampToByte(Int64(g))
            let blue = clampToByte(Int64(b))
            let alpha = clampToByte(Int64(a))
            return red | (green << 8) | (blue << 16) | (alpha << 24)
        }

        private static func packFloats(_ r: Float, _ g: Float, _ b: Float, _ a: Float) -> UInt32 {
            packUNorm(Float(255), r) |
                (packUNorm(Float(255), g) << 8) |
                (packUNorm(Float(255), b) << 16) |
                (packUNorm(Float(255), a) << 24)
        }

        private static func packUNorm(_ bitmask: Float, _ value: Float) -> UInt32 {
            let scaled = value * bitmask
            if scaled.isNaN || scaled <= 0 { return 0 }
            if scaled >= bitmask { return UInt32(bitmask) }
            return UInt32(scaled.rounded(.toNearestOrEven))
        }

        private static func clampToByte(_ value: Int64) -> UInt32 {
            if value < 0 { return 0 }
            if value > 255 { return 255 }
            return UInt32(value)
        }
    }
}
