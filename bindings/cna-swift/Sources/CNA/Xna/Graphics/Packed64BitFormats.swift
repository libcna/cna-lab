// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics.PackedVector {
    public struct HalfVector4: IPackedVectorOfT {
        public typealias TPacked = UInt64
        private var packedValue: UInt64

        public var PackedValue: UInt64 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float, _ z: Float, _ w: Float) {
            packedValue = Self.pack(x, y, z, w)
        }

        public init(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z, vector.W)
        }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z, vector.W)
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            Microsoft.Xna.Framework.Vector4(
                xnaUnpackHalf(UInt16(truncatingIfNeeded: packedValue)),
                xnaUnpackHalf(UInt16(truncatingIfNeeded: packedValue >> 16)),
                xnaUnpackHalf(UInt16(truncatingIfNeeded: packedValue >> 32)),
                xnaUnpackHalf(UInt16(truncatingIfNeeded: packedValue >> 48))
            )
        }

        public func ToString() -> String { ToVector4().ToString() }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? HalfVector4).map { Equals($0) } ?? false }
        public func Equals(_ other: HalfVector4) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: HalfVector4, rhs: HalfVector4) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: HalfVector4, rhs: HalfVector4) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt64 {
            UInt64(xnaPackHalf(x)) |
                (UInt64(xnaPackHalf(y)) << 16) |
                (UInt64(xnaPackHalf(z)) << 32) |
                (UInt64(xnaPackHalf(w)) << 48)
        }
    }

    public struct NormalizedShort4: IPackedVectorOfT {
        public typealias TPacked = UInt64
        private var packedValue: UInt64

        public var PackedValue: UInt64 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float, _ z: Float, _ w: Float) {
            packedValue = Self.pack(x, y, z, w)
        }

        public init(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z, vector.W)
        }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z, vector.W)
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            Microsoft.Xna.Framework.Vector4(
                xnaUnpackSNorm(65535, UInt32(truncatingIfNeeded: packedValue)),
                xnaUnpackSNorm(65535, UInt32(truncatingIfNeeded: packedValue >> 16)),
                xnaUnpackSNorm(65535, UInt32(truncatingIfNeeded: packedValue >> 32)),
                xnaUnpackSNorm(65535, UInt32(truncatingIfNeeded: packedValue >> 48))
            )
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 16) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? NormalizedShort4).map { Equals($0) } ?? false }
        public func Equals(_ other: NormalizedShort4) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: NormalizedShort4, rhs: NormalizedShort4) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: NormalizedShort4, rhs: NormalizedShort4) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt64 {
            UInt64(xnaPackSNorm(65535, x)) |
                (UInt64(xnaPackSNorm(65535, y)) << 16) |
                (UInt64(xnaPackSNorm(65535, z)) << 32) |
                (UInt64(xnaPackSNorm(65535, w)) << 48)
        }
    }

    public struct Rgba64: IPackedVectorOfT {
        public typealias TPacked = UInt64
        private var packedValue: UInt64

        public var PackedValue: UInt64 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float, _ z: Float, _ w: Float) {
            packedValue = Self.pack(x, y, z, w)
        }

        public init(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z, vector.W)
        }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z, vector.W)
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            Microsoft.Xna.Framework.Vector4(
                xnaUnpackUNorm(65535, UInt32(truncatingIfNeeded: packedValue)),
                xnaUnpackUNorm(65535, UInt32(truncatingIfNeeded: packedValue >> 16)),
                xnaUnpackUNorm(65535, UInt32(truncatingIfNeeded: packedValue >> 32)),
                xnaUnpackUNorm(65535, UInt32(truncatingIfNeeded: packedValue >> 48))
            )
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 16) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Rgba64).map { Equals($0) } ?? false }
        public func Equals(_ other: Rgba64) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Rgba64, rhs: Rgba64) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Rgba64, rhs: Rgba64) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt64 {
            UInt64(xnaPackUNorm(65535, x)) |
                (UInt64(xnaPackUNorm(65535, y)) << 16) |
                (UInt64(xnaPackUNorm(65535, z)) << 32) |
                (UInt64(xnaPackUNorm(65535, w)) << 48)
        }
    }

    public struct Short4: IPackedVectorOfT {
        public typealias TPacked = UInt64
        private var packedValue: UInt64

        public var PackedValue: UInt64 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float, _ z: Float, _ w: Float) {
            packedValue = Self.pack(x, y, z, w)
        }

        public init(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z, vector.W)
        }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z, vector.W)
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            Microsoft.Xna.Framework.Vector4(
                Self.unpack(packedValue),
                Self.unpack(packedValue >> 16),
                Self.unpack(packedValue >> 32),
                Self.unpack(packedValue >> 48)
            )
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 16) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Short4).map { Equals($0) } ?? false }
        public func Equals(_ other: Short4) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Short4, rhs: Short4) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Short4, rhs: Short4) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt64 {
            UInt64(xnaPackSigned(65535, x)) |
                (UInt64(xnaPackSigned(65535, y)) << 16) |
                (UInt64(xnaPackSigned(65535, z)) << 32) |
                (UInt64(xnaPackSigned(65535, w)) << 48)
        }

        private static func unpack(_ value: UInt64) -> Float {
            Float(Int16(bitPattern: UInt16(truncatingIfNeeded: value)))
        }
    }
}
