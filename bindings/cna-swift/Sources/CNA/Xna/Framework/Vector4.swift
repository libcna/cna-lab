// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Vector4 {
        public var X: Float
        public var Y: Float
        public var Z: Float
        public var W: Float

        public init(_ x: Float, _ y: Float, _ z: Float, _ w: Float) { X = x; Y = y; Z = z; W = w }
        public init(_ value: Vector2, _ z: Float, _ w: Float) { X = value.X; Y = value.Y; Z = z; W = w }
        public init(_ value: Vector3, _ w: Float) { X = value.X; Y = value.Y; Z = value.Z; W = w }
        public init(_ value: Float) { X = value; Y = value; Z = value; W = value }

        public static var Zero: Vector4 { Vector4(0, 0, 0, 0) }
        public static var One: Vector4 { Vector4(1, 1, 1, 1) }
        public static var UnitX: Vector4 { Vector4(1, 0, 0, 0) }
        public static var UnitY: Vector4 { Vector4(0, 1, 0, 0) }
        public static var UnitZ: Vector4 { Vector4(0, 0, 1, 0) }
        public static var UnitW: Vector4 { Vector4(0, 0, 0, 1) }

        public func Length() -> Float { xnaSqrt(X * X + Y * Y + Z * Z + W * W) }
        public func LengthSquared() -> Float { X * X + Y * Y + Z * Z + W * W }
        public mutating func Normalize() {
            let factor: Float = 1 / xnaSqrt(X * X + Y * Y + Z * Z + W * W)
            X *= factor; Y *= factor; Z *= factor; W *= factor
        }
        public static func Normalize(_ vector: Vector4) -> Vector4 { var vector = vector; vector.Normalize(); return vector }
        public static func Normalize(_ vector: inout Vector4, result: inout Vector4) { result = Normalize(vector) }

        public static func Distance(_ value1: Vector4, value2: Vector4) -> Float { xnaSqrt(DistanceSquared(value1, value2: value2)) }
        public static func Distance(_ value1: inout Vector4, value2: inout Vector4, result: inout Float) { result = Distance(value1, value2: value2) }
        public static func DistanceSquared(_ value1: Vector4, value2: Vector4) -> Float {
            let x = value1.X - value2.X; let y = value1.Y - value2.Y
            let z = value1.Z - value2.Z; let w = value1.W - value2.W
            return x * x + y * y + z * z + w * w
        }
        public static func DistanceSquared(_ value1: inout Vector4, value2: inout Vector4, result: inout Float) { result = DistanceSquared(value1, value2: value2) }
        public static func Dot(_ vector1: Vector4, vector2: Vector4) -> Float {
            vector1.X * vector2.X + vector1.Y * vector2.Y + vector1.Z * vector2.Z + vector1.W * vector2.W
        }
        public static func Dot(_ vector1: inout Vector4, vector2: inout Vector4, result: inout Float) { result = Dot(vector1, vector2: vector2) }

        public static func Min(_ value1: Vector4, value2: Vector4) -> Vector4 {
            Vector4(value1.X < value2.X ? value1.X : value2.X,
                    value1.Y < value2.Y ? value1.Y : value2.Y,
                    value1.Z < value2.Z ? value1.Z : value2.Z,
                    value1.W < value2.W ? value1.W : value2.W)
        }
        public static func Min(_ value1: inout Vector4, value2: inout Vector4, result: inout Vector4) { result = Min(value1, value2: value2) }
        public static func Max(_ value1: Vector4, value2: Vector4) -> Vector4 {
            Vector4(value1.X > value2.X ? value1.X : value2.X,
                    value1.Y > value2.Y ? value1.Y : value2.Y,
                    value1.Z > value2.Z ? value1.Z : value2.Z,
                    value1.W > value2.W ? value1.W : value2.W)
        }
        public static func Max(_ value1: inout Vector4, value2: inout Vector4, result: inout Vector4) { result = Max(value1, value2: value2) }
        public static func Clamp(_ value1: Vector4, min: Vector4, max: Vector4) -> Vector4 {
            Vector4(MathHelper.Clamp(value1.X, min: min.X, max: max.X),
                    MathHelper.Clamp(value1.Y, min: min.Y, max: max.Y),
                    MathHelper.Clamp(value1.Z, min: min.Z, max: max.Z),
                    MathHelper.Clamp(value1.W, min: min.W, max: max.W))
        }
        public static func Clamp(_ value1: inout Vector4, min: inout Vector4, max: inout Vector4, result: inout Vector4) { result = Clamp(value1, min: min, max: max) }
        public static func Lerp(_ value1: Vector4, value2: Vector4, amount: Float) -> Vector4 {
            Vector4(value1.X + (value2.X - value1.X) * amount,
                    value1.Y + (value2.Y - value1.Y) * amount,
                    value1.Z + (value2.Z - value1.Z) * amount,
                    value1.W + (value2.W - value1.W) * amount)
        }
        public static func Lerp(_ value1: inout Vector4, value2: inout Vector4, amount: Float, result: inout Vector4) { result = Lerp(value1, value2: value2, amount: amount) }
        public static func Barycentric(_ value1: Vector4, value2: Vector4, value3: Vector4, amount1: Float, amount2: Float) -> Vector4 {
            Vector4(MathHelper.Barycentric(value1.X, value2: value2.X, value3: value3.X, amount1: amount1, amount2: amount2),
                    MathHelper.Barycentric(value1.Y, value2: value2.Y, value3: value3.Y, amount1: amount1, amount2: amount2),
                    MathHelper.Barycentric(value1.Z, value2: value2.Z, value3: value3.Z, amount1: amount1, amount2: amount2),
                    MathHelper.Barycentric(value1.W, value2: value2.W, value3: value3.W, amount1: amount1, amount2: amount2))
        }
        public static func Barycentric(_ value1: inout Vector4, value2: inout Vector4, value3: inout Vector4, amount1: Float, amount2: Float, result: inout Vector4) { result = Barycentric(value1, value2: value2, value3: value3, amount1: amount1, amount2: amount2) }
        public static func SmoothStep(_ value1: Vector4, value2: Vector4, amount: Float) -> Vector4 {
            Vector4(MathHelper.SmoothStep(value1.X, value2: value2.X, amount: amount),
                    MathHelper.SmoothStep(value1.Y, value2: value2.Y, amount: amount),
                    MathHelper.SmoothStep(value1.Z, value2: value2.Z, amount: amount),
                    MathHelper.SmoothStep(value1.W, value2: value2.W, amount: amount))
        }
        public static func SmoothStep(_ value1: inout Vector4, value2: inout Vector4, amount: Float, result: inout Vector4) { result = SmoothStep(value1, value2: value2, amount: amount) }
        public static func CatmullRom(_ value1: Vector4, value2: Vector4, value3: Vector4, value4: Vector4, amount: Float) -> Vector4 {
            Vector4(MathHelper.CatmullRom(value1.X, value2: value2.X, value3: value3.X, value4: value4.X, amount: amount),
                    MathHelper.CatmullRom(value1.Y, value2: value2.Y, value3: value3.Y, value4: value4.Y, amount: amount),
                    MathHelper.CatmullRom(value1.Z, value2: value2.Z, value3: value3.Z, value4: value4.Z, amount: amount),
                    MathHelper.CatmullRom(value1.W, value2: value2.W, value3: value3.W, value4: value4.W, amount: amount))
        }
        public static func CatmullRom(_ value1: inout Vector4, value2: inout Vector4, value3: inout Vector4, value4: inout Vector4, amount: Float, result: inout Vector4) { result = CatmullRom(value1, value2: value2, value3: value3, value4: value4, amount: amount) }
        public static func Hermite(_ value1: Vector4, tangent1: Vector4, value2: Vector4, tangent2: Vector4, amount: Float) -> Vector4 {
            Vector4(MathHelper.Hermite(value1.X, tangent1: tangent1.X, value2: value2.X, tangent2: tangent2.X, amount: amount),
                    MathHelper.Hermite(value1.Y, tangent1: tangent1.Y, value2: value2.Y, tangent2: tangent2.Y, amount: amount),
                    MathHelper.Hermite(value1.Z, tangent1: tangent1.Z, value2: value2.Z, tangent2: tangent2.Z, amount: amount),
                    MathHelper.Hermite(value1.W, tangent1: tangent1.W, value2: value2.W, tangent2: tangent2.W, amount: amount))
        }
        public static func Hermite(_ value1: inout Vector4, tangent1: inout Vector4, value2: inout Vector4, tangent2: inout Vector4, amount: Float, result: inout Vector4) { result = Hermite(value1, tangent1: tangent1, value2: value2, tangent2: tangent2, amount: amount) }

        public static func Transform(_ position: Vector2, matrix: Matrix) -> Vector4 {
            Vector4(position.X * matrix.M11 + position.Y * matrix.M21 + matrix.M41,
                    position.X * matrix.M12 + position.Y * matrix.M22 + matrix.M42,
                    position.X * matrix.M13 + position.Y * matrix.M23 + matrix.M43,
                    position.X * matrix.M14 + position.Y * matrix.M24 + matrix.M44)
        }
        public static func Transform(_ position: inout Vector2, matrix: inout Matrix, result: inout Vector4) { result = Transform(position, matrix: matrix) }
        public static func Transform(_ position: Vector3, matrix: Matrix) -> Vector4 {
            Vector4(position.X * matrix.M11 + position.Y * matrix.M21 + position.Z * matrix.M31 + matrix.M41,
                    position.X * matrix.M12 + position.Y * matrix.M22 + position.Z * matrix.M32 + matrix.M42,
                    position.X * matrix.M13 + position.Y * matrix.M23 + position.Z * matrix.M33 + matrix.M43,
                    position.X * matrix.M14 + position.Y * matrix.M24 + position.Z * matrix.M34 + matrix.M44)
        }
        public static func Transform(_ position: inout Vector3, matrix: inout Matrix, result: inout Vector4) { result = Transform(position, matrix: matrix) }
        public static func Transform(_ vector: Vector4, matrix: Matrix) -> Vector4 {
            Vector4(vector.X * matrix.M11 + vector.Y * matrix.M21 + vector.Z * matrix.M31 + vector.W * matrix.M41,
                    vector.X * matrix.M12 + vector.Y * matrix.M22 + vector.Z * matrix.M32 + vector.W * matrix.M42,
                    vector.X * matrix.M13 + vector.Y * matrix.M23 + vector.Z * matrix.M33 + vector.W * matrix.M43,
                    vector.X * matrix.M14 + vector.Y * matrix.M24 + vector.Z * matrix.M34 + vector.W * matrix.M44)
        }
        public static func Transform(_ vector: inout Vector4, matrix: inout Matrix, result: inout Vector4) { result = Transform(vector, matrix: matrix) }
        public static func Transform(_ value: Vector2, rotation: Quaternion) -> Vector4 {
            let rotated = Vector3.Transform(Vector3(value, 0), rotation: rotation)
            return Vector4(rotated, 1)
        }
        public static func Transform(_ value: inout Vector2, rotation: inout Quaternion, result: inout Vector4) { result = Transform(value, rotation: rotation) }
        public static func Transform(_ value: Vector3, rotation: Quaternion) -> Vector4 { Vector4(Vector3.Transform(value, rotation: rotation), 1) }
        public static func Transform(_ value: inout Vector3, rotation: inout Quaternion, result: inout Vector4) { result = Transform(value, rotation: rotation) }
        public static func Transform(_ value: Vector4, rotation: Quaternion) -> Vector4 {
            Vector4(Vector3.Transform(Vector3(value.X, value.Y, value.Z), rotation: rotation), value.W)
        }
        public static func Transform(_ value: inout Vector4, rotation: inout Quaternion, result: inout Vector4) { result = Transform(value, rotation: rotation) }

        public static func Transform(_ sourceArray: [Vector4], matrix: inout Matrix, destinationArray: inout [Vector4]) throws {
            try Transform(sourceArray, sourceIndex: 0, matrix: &matrix, destinationArray: &destinationArray, destinationIndex: 0, length: Int32(sourceArray.count))
        }
        public static func Transform(_ sourceArray: [Vector4], sourceIndex: Int32, matrix: inout Matrix, destinationArray: inout [Vector4], destinationIndex: Int32, length: Int32) throws {
            try xnaValidateTransformArrayRanges(sourceCount: sourceArray.count, sourceIndex: sourceIndex, destinationCount: destinationArray.count, destinationIndex: destinationIndex, length: length)
            var i: Int32 = 0
            while i < length { destinationArray[Int(destinationIndex + i)] = Transform(sourceArray[Int(sourceIndex + i)], matrix: matrix); i += 1 }
        }
        public static func Transform(_ sourceArray: [Vector4], rotation: inout Quaternion, destinationArray: inout [Vector4]) throws {
            try Transform(sourceArray, sourceIndex: 0, rotation: &rotation, destinationArray: &destinationArray, destinationIndex: 0, length: Int32(sourceArray.count))
        }
        public static func Transform(_ sourceArray: [Vector4], sourceIndex: Int32, rotation: inout Quaternion, destinationArray: inout [Vector4], destinationIndex: Int32, length: Int32) throws {
            try xnaValidateTransformArrayRanges(sourceCount: sourceArray.count, sourceIndex: sourceIndex, destinationCount: destinationArray.count, destinationIndex: destinationIndex, length: length)
            var i: Int32 = 0
            while i < length { destinationArray[Int(destinationIndex + i)] = Transform(sourceArray[Int(sourceIndex + i)], rotation: rotation); i += 1 }
        }

        public static func Negate(_ value: Vector4) -> Vector4 { -value }
        public static func Negate(_ value: inout Vector4, result: inout Vector4) { result = -value }
        public static func Add(_ value1: Vector4, value2: Vector4) -> Vector4 { value1 + value2 }
        public static func Add(_ value1: inout Vector4, value2: inout Vector4, result: inout Vector4) { result = value1 + value2 }
        public static func Subtract(_ value1: Vector4, value2: Vector4) -> Vector4 { value1 - value2 }
        public static func Subtract(_ value1: inout Vector4, value2: inout Vector4, result: inout Vector4) { result = value1 - value2 }
        public static func Multiply(_ value1: Vector4, value2: Vector4) -> Vector4 { value1 * value2 }
        public static func Multiply(_ value1: inout Vector4, value2: inout Vector4, result: inout Vector4) { result = value1 * value2 }
        public static func Multiply(_ value1: Vector4, scaleFactor: Float) -> Vector4 { value1 * scaleFactor }
        public static func Multiply(_ value1: inout Vector4, scaleFactor: Float, result: inout Vector4) { result = value1 * scaleFactor }
        public static func Divide(_ value1: Vector4, value2: Vector4) -> Vector4 { value1 / value2 }
        public static func Divide(_ value1: inout Vector4, value2: inout Vector4, result: inout Vector4) { result = value1 / value2 }
        public static func Divide(_ value1: Vector4, divider: Float) -> Vector4 { value1 / divider }
        public static func Divide(_ value1: inout Vector4, divider: Float, result: inout Vector4) { result = value1 / divider }

        public func Equals(_ other: Vector4) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool { guard let other = obj as? Vector4 else { return false }; return self == other }
        public func GetHashCode() -> Int32 { xnaFloatHash(X) &+ xnaFloatHash(Y) &+ xnaFloatHash(Z) &+ xnaFloatHash(W) }
        public func ToString() -> String { "{X:\(xnaFloatString(X)) Y:\(xnaFloatString(Y)) Z:\(xnaFloatString(Z)) W:\(xnaFloatString(W))}" }

        public static prefix func - (value: Vector4) -> Vector4 { Vector4(-value.X, -value.Y, -value.Z, -value.W) }
        public static func + (lhs: Vector4, rhs: Vector4) -> Vector4 { Vector4(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z, lhs.W + rhs.W) }
        public static func - (lhs: Vector4, rhs: Vector4) -> Vector4 { Vector4(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z, lhs.W - rhs.W) }
        public static func * (lhs: Vector4, rhs: Vector4) -> Vector4 { Vector4(lhs.X * rhs.X, lhs.Y * rhs.Y, lhs.Z * rhs.Z, lhs.W * rhs.W) }
        public static func * (lhs: Vector4, rhs: Float) -> Vector4 { Vector4(lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs, lhs.W * rhs) }
        public static func * (lhs: Float, rhs: Vector4) -> Vector4 { rhs * lhs }
        public static func / (lhs: Vector4, rhs: Vector4) -> Vector4 { Vector4(lhs.X / rhs.X, lhs.Y / rhs.Y, lhs.Z / rhs.Z, lhs.W / rhs.W) }
        public static func / (lhs: Vector4, rhs: Float) -> Vector4 {
            let factor: Float = 1 / rhs
            return Vector4(lhs.X * factor, lhs.Y * factor, lhs.Z * factor, lhs.W * factor)
        }
        public static func == (lhs: Vector4, rhs: Vector4) -> Bool { lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z && lhs.W == rhs.W }
        public static func != (lhs: Vector4, rhs: Vector4) -> Bool { !(lhs == rhs) }
    }
}
