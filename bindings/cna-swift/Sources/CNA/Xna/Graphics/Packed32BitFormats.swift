// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics.PackedVector {
    public struct Byte4: IPackedVectorOfT {
        public typealias TPacked = UInt32
        private var packedValue: UInt32

        public var PackedValue: UInt32 {
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
                Float(packedValue & 0xFF),
                Float((packedValue >> 8) & 0xFF),
                Float((packedValue >> 16) & 0xFF),
                Float((packedValue >> 24) & 0xFF)
            )
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 8) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Byte4).map { Equals($0) } ?? false }
        public func Equals(_ other: Byte4) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Byte4, rhs: Byte4) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Byte4, rhs: Byte4) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt32 {
            xnaPackUnsigned(255, x) |
                (xnaPackUnsigned(255, y) << 8) |
                (xnaPackUnsigned(255, z) << 16) |
                (xnaPackUnsigned(255, w) << 24)
        }
    }

    public struct HalfVector2: IPackedVectorOfT {
        public typealias TPacked = UInt32
        private var packedValue: UInt32

        public var PackedValue: UInt32 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float) { packedValue = Self.pack(x, y) }
        public init(_ vector: Microsoft.Xna.Framework.Vector2) { packedValue = Self.pack(vector.X, vector.Y) }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y)
        }

        public func ToVector2() -> Microsoft.Xna.Framework.Vector2 {
            Microsoft.Xna.Framework.Vector2(
                xnaUnpackHalf(UInt16(truncatingIfNeeded: packedValue)),
                xnaUnpackHalf(UInt16(truncatingIfNeeded: packedValue >> 16))
            )
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            let value = ToVector2()
            return Microsoft.Xna.Framework.Vector4(value.X, value.Y, 0, 1)
        }

        public func ToString() -> String { ToVector2().ToString() }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? HalfVector2).map { Equals($0) } ?? false }
        public func Equals(_ other: HalfVector2) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: HalfVector2, rhs: HalfVector2) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: HalfVector2, rhs: HalfVector2) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float) -> UInt32 {
            UInt32(xnaPackHalf(x)) | (UInt32(xnaPackHalf(y)) << 16)
        }
    }

    public struct NormalizedByte4: IPackedVectorOfT {
        public typealias TPacked = UInt32
        private var packedValue: UInt32

        public var PackedValue: UInt32 {
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
                xnaUnpackSNorm(255, packedValue),
                xnaUnpackSNorm(255, packedValue >> 8),
                xnaUnpackSNorm(255, packedValue >> 16),
                xnaUnpackSNorm(255, packedValue >> 24)
            )
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 8) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? NormalizedByte4).map { Equals($0) } ?? false }
        public func Equals(_ other: NormalizedByte4) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: NormalizedByte4, rhs: NormalizedByte4) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: NormalizedByte4, rhs: NormalizedByte4) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt32 {
            xnaPackSNorm(255, x) |
                (xnaPackSNorm(255, y) << 8) |
                (xnaPackSNorm(255, z) << 16) |
                (xnaPackSNorm(255, w) << 24)
        }
    }

    public struct NormalizedShort2: IPackedVectorOfT {
        public typealias TPacked = UInt32
        private var packedValue: UInt32

        public var PackedValue: UInt32 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float) { packedValue = Self.pack(x, y) }
        public init(_ vector: Microsoft.Xna.Framework.Vector2) { packedValue = Self.pack(vector.X, vector.Y) }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y)
        }

        public func ToVector2() -> Microsoft.Xna.Framework.Vector2 {
            Microsoft.Xna.Framework.Vector2(
                xnaUnpackSNorm(65535, packedValue),
                xnaUnpackSNorm(65535, packedValue >> 16)
            )
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            let value = ToVector2()
            return Microsoft.Xna.Framework.Vector4(value.X, value.Y, 0, 1)
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 8) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? NormalizedShort2).map { Equals($0) } ?? false }
        public func Equals(_ other: NormalizedShort2) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: NormalizedShort2, rhs: NormalizedShort2) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: NormalizedShort2, rhs: NormalizedShort2) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float) -> UInt32 {
            xnaPackSNorm(65535, x) | (xnaPackSNorm(65535, y) << 16)
        }
    }

    public struct Rg32: IPackedVectorOfT {
        public typealias TPacked = UInt32
        private var packedValue: UInt32

        public var PackedValue: UInt32 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float) { packedValue = Self.pack(x, y) }
        public init(_ vector: Microsoft.Xna.Framework.Vector2) { packedValue = Self.pack(vector.X, vector.Y) }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y)
        }

        public func ToVector2() -> Microsoft.Xna.Framework.Vector2 {
            Microsoft.Xna.Framework.Vector2(
                xnaUnpackUNorm(65535, packedValue),
                xnaUnpackUNorm(65535, packedValue >> 16)
            )
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            let value = ToVector2()
            return Microsoft.Xna.Framework.Vector4(value.X, value.Y, 0, 1)
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 8) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Rg32).map { Equals($0) } ?? false }
        public func Equals(_ other: Rg32) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Rg32, rhs: Rg32) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Rg32, rhs: Rg32) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float) -> UInt32 {
            xnaPackUNorm(65535, x) | (xnaPackUNorm(65535, y) << 16)
        }
    }

    public struct Rgba1010102: IPackedVectorOfT {
        public typealias TPacked = UInt32
        private var packedValue: UInt32

        public var PackedValue: UInt32 {
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
                xnaUnpackUNorm(1023, packedValue),
                xnaUnpackUNorm(1023, packedValue >> 10),
                xnaUnpackUNorm(1023, packedValue >> 20),
                xnaUnpackUNorm(3, packedValue >> 30)
            )
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 8) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Rgba1010102).map { Equals($0) } ?? false }
        public func Equals(_ other: Rgba1010102) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Rgba1010102, rhs: Rgba1010102) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Rgba1010102, rhs: Rgba1010102) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float, _ z: Float, _ w: Float) -> UInt32 {
            xnaPackUNorm(1023, x) |
                (xnaPackUNorm(1023, y) << 10) |
                (xnaPackUNorm(1023, z) << 20) |
                (xnaPackUNorm(3, w) << 30)
        }
    }

    public struct Short2: IPackedVectorOfT {
        public typealias TPacked = UInt32
        private var packedValue: UInt32

        public var PackedValue: UInt32 {
            get { packedValue }
            set { packedValue = newValue }
        }

        public init(_ x: Float, _ y: Float) { packedValue = Self.pack(x, y) }
        public init(_ vector: Microsoft.Xna.Framework.Vector2) { packedValue = Self.pack(vector.X, vector.Y) }

        public mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4) {
            packedValue = Self.pack(vector.X, vector.Y)
        }

        public func ToVector2() -> Microsoft.Xna.Framework.Vector2 {
            Microsoft.Xna.Framework.Vector2(
                Float(Int16(bitPattern: UInt16(truncatingIfNeeded: packedValue))),
                Float(Int16(bitPattern: UInt16(truncatingIfNeeded: packedValue >> 16)))
            )
        }

        public func ToVector4() -> Microsoft.Xna.Framework.Vector4 {
            let value = ToVector2()
            return Microsoft.Xna.Framework.Vector4(value.X, value.Y, 0, 1)
        }

        public func ToString() -> String { xnaPackedHex(packedValue, width: 8) }
        public func GetHashCode() -> Int32 { xnaPackedHash(packedValue) }
        public func Equals(_ obj: Any?) -> Bool { (obj as? Short2).map { Equals($0) } ?? false }
        public func Equals(_ other: Short2) -> Bool { packedValue == other.packedValue }
        public static func == (lhs: Short2, rhs: Short2) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Short2, rhs: Short2) -> Bool { !lhs.Equals(rhs) }

        private static func pack(_ x: Float, _ y: Float) -> UInt32 {
            xnaPackSigned(65535, x) | (xnaPackSigned(65535, y) << 16)
        }
    }
}
