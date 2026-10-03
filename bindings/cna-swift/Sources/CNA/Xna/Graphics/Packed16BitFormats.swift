// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics.PackedVector {
    public struct Alpha8: IPackedVectorOfT {
        public typealias TPacked = UInt8
        private var packedValue: UInt8

        public var PackedValue: UInt8 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ alpha: Float) {
            packedValue = UInt8(xnaPackUNorm(255, alpha))
        }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = UInt8(xnaPackUNorm(255, vector.W))
        }

        public func ToAlpha() -> Float { xnaUnpackUNorm(255, UInt32(packedValue)) }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            Microsoft.Xna.Framework.Vector4(0, 0, 0, ToAlpha())
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 2) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Alpha8).map { Equals($0) } ?? false }
        public func Equals(_ other: Alpha8) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Alpha8, rhs: Alpha8) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Alpha8, rhs: Alpha8) -> Bool { !lhs.Equals(rhs) }
    }

    public struct Bgr565: IPackedVectorOfT {
        public typealias TPacked = UInt16
        private var packedValue: UInt16

        public var PackedValue: UInt16 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float, _ z: Float) {
            packedValue = Self.pack(x, y, z)
        }

        public init(_ vector: Microsoft.Xna.Framework.Vector3) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z)
        }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y, vector.Z)
        }

        public func ToVector3() -> Microsoft.Xna.Framework.Vector3 {
            let value = UInt32(packedValue)
            return Microsoft.Xna.Framework.Vector3(
                xnaUnpackUNorm(31, value >> 11),
                xnaUnpackUNorm(63, value >> 5),
                xnaUnpackUNorm(31, value)
            )
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            let value = ToVector3()
            return Microsoft.Xna.Framework.Vector4(value.X, value.Y, value.Z, 1)
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 4) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Bgr565).map { Equals($0) } ?? false }
        public func Equals(_ other: Bgr565) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Bgr565, rhs: Bgr565) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Bgr565, rhs: Bgr565) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float) -> UInt16 {
            UInt16((xnaPackUNorm(31, x) << 11) |
                (xnaPackUNorm(63, y) << 5) |
                xnaPackUNorm(31, z))
        }
    }

    public struct Bgra4444: IPackedVectorOfT {
        public typealias TPacked = UInt16
        private var packedValue: UInt16

        public var PackedValue: UInt16 {
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
            let value = UInt32(packedValue)
            return Microsoft.Xna.Framework.Vector4(
                xnaUnpackUNorm(15, value >> 8),
                xnaUnpackUNorm(15, value >> 4),
                xnaUnpackUNorm(15, value),
                xnaUnpackUNorm(15, value >> 12)
            )
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 4) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Bgra4444).map { Equals($0) } ?? false }
        public func Equals(_ other: Bgra4444) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Bgra4444, rhs: Bgra4444) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Bgra4444, rhs: Bgra4444) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt16 {
            UInt16((xnaPackUNorm(15, x) << 8) |
                (xnaPackUNorm(15, y) << 4) |
                xnaPackUNorm(15, z) |
                (xnaPackUNorm(15, w) << 12))
        }
    }

    public struct Bgra5551: IPackedVectorOfT {
        public typealias TPacked = UInt16
        private var packedValue: UInt16

        public var PackedValue: UInt16 {
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
            let value = UInt32(packedValue)
            return Microsoft.Xna.Framework.Vector4(
                xnaUnpackUNorm(31, value >> 10),
                xnaUnpackUNorm(31, value >> 5),
                xnaUnpackUNorm(31, value),
                xnaUnpackUNorm(1, value >> 15)
            )
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 4) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Bgra5551).map { Equals($0) } ?? false }
        public func Equals(_ other: Bgra5551) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Bgra5551, rhs: Bgra5551) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Bgra5551, rhs: Bgra5551) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt16 {
            UInt16((xnaPackUNorm(31, x) << 10) |
                (xnaPackUNorm(31, y) << 5) |
                xnaPackUNorm(31, z) |
                (xnaPackUNorm(1, w) << 15))
        }
    }

    public struct HalfSingle: IPackedVectorOfT {
        public typealias TPacked = UInt16
        private var packedValue: UInt16

        public var PackedValue: UInt16 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ value: Float) { packedValue = xnaPackHalf(value) }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = xnaPackHalf(vector.X)
        }

        public func ToSingle() -> Float { xnaUnpackHalf(packedValue) }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            Microsoft.Xna.Framework.Vector4(ToSingle(), 0, 0, 1)
        }

        public func ToString() -> String { xnaFloatString(ToSingle()) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? HalfSingle).map { Equals($0) } ?? false }
        public func Equals(_ other: HalfSingle) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: HalfSingle, rhs: HalfSingle) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: HalfSingle, rhs: HalfSingle) -> Bool { !lhs.Equals(rhs) }
    }

    public struct NormalizedByte2: IPackedVectorOfT {
        public typealias TPacked = UInt16
        private var packedValue: UInt16

        public var PackedValue: UInt16 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float) { packedValue = Self.pack(x, y) }
        public init(_ vector: Microsoft.Xna.Framework.Vector2) { packedValue = Self.pack(vector.X, vector.Y) }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y)
        }

        public func ToVector2() -> Microsoft.Xna.Framework.Vector2 {
            let value = UInt32(packedValue)
            return Microsoft.Xna.Framework.Vector2(
                xnaUnpackSNorm(255, value),
                xnaUnpackSNorm(255, value >> 8)
            )
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            let value = ToVector2()
            return Microsoft.Xna.Framework.Vector4(value.X, value.Y, 0, 1)
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 4) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? NormalizedByte2).map { Equals($0) } ?? false }
        public func Equals(_ other: NormalizedByte2) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: NormalizedByte2, rhs: NormalizedByte2) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: NormalizedByte2, rhs: NormalizedByte2) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float) -> UInt16 {
            UInt16(xnaPackSNorm(255, x) | (xnaPackSNorm(255, y) << 8))
        }
    }
}
