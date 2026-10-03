// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Vector3 {
        public var X: Float
        public var Y: Float
        public var Z: Float

        public init(_ x: Float, _ y: Float, _ z: Float) { X = x; Y = y; Z = z }
        public init(_ value: Float) { X = value; Y = value; Z = value }
        public init(_ value: Vector2, _ z: Float) { X = value.X; Y = value.Y; Z = z }

        public static var Zero: Vector3 { Vector3(0, 0, 0) }
        public static var One: Vector3 { Vector3(1, 1, 1) }
        public static var UnitX: Vector3 { Vector3(1, 0, 0) }
        public static var UnitY: Vector3 { Vector3(0, 1, 0) }
        public static var UnitZ: Vector3 { Vector3(0, 0, 1) }
        public static var Up: Vector3 { Vector3(0, 1, 0) }
        public static var Down: Vector3 { Vector3(0, -1, 0) }
        public static var Right: Vector3 { Vector3(1, 0, 0) }
        public static var Left: Vector3 { Vector3(-1, 0, 0) }
        public static var Forward: Vector3 { Vector3(0, 0, -1) }
        public static var Backward: Vector3 { Vector3(0, 0, 1) }

        public func Length() -> Float { xnaSqrt(X * X + Y * Y + Z * Z) }
        public func LengthSquared() -> Float { X * X + Y * Y + Z * Z }
        public mutating func Normalize() {
            let factor: Float = 1 / xnaSqrt(X * X + Y * Y + Z * Z)
            X *= factor; Y *= factor; Z *= factor
        }
        public static func Normalize(_ value: Vector3) -> Vector3 { var value = value; value.Normalize(); return value }
        public static func Normalize(_ value: inout Vector3, result: inout Vector3) { result = Normalize(value) }

        public static func Distance(_ value1: Vector3, value2: Vector3) -> Float { xnaSqrt(DistanceSquared(value1, value2: value2)) }
        public static func Distance(_ value1: inout Vector3, value2: inout Vector3, result: inout Float) { result = Distance(value1, value2: value2) }
        public static func DistanceSquared(_ value1: Vector3, value2: Vector3) -> Float {
            let x = value1.X - value2.X; let y = value1.Y - value2.Y; let z = value1.Z - value2.Z
            return x * x + y * y + z * z
        }
        public static func DistanceSquared(_ value1: inout Vector3, value2: inout Vector3, result: inout Float) { result = DistanceSquared(value1, value2: value2) }
        public static func Dot(_ vector1: Vector3, vector2: Vector3) -> Float {
            vector1.X * vector2.X + vector1.Y * vector2.Y + vector1.Z * vector2.Z
        }
        public static func Dot(_ vector1: inout Vector3, vector2: inout Vector3, result: inout Float) { result = Dot(vector1, vector2: vector2) }
        public static func Cross(_ vector1: Vector3, vector2: Vector3) -> Vector3 {
            Vector3(vector1.Y * vector2.Z - vector1.Z * vector2.Y,
                    vector1.Z * vector2.X - vector1.X * vector2.Z,
                    vector1.X * vector2.Y - vector1.Y * vector2.X)
        }
        public static func Cross(_ vector1: inout Vector3, vector2: inout Vector3, result: inout Vector3) { result = Cross(vector1, vector2: vector2) }
        public static func Reflect(_ vector: Vector3, normal: Vector3) -> Vector3 {
            let dot = vector.X * normal.X + vector.Y * normal.Y + vector.Z * normal.Z
            return Vector3(vector.X - 2 * dot * normal.X,
                           vector.Y - 2 * dot * normal.Y,
                           vector.Z - 2 * dot * normal.Z)
        }
        public static func Reflect(_ vector: inout Vector3, normal: inout Vector3, result: inout Vector3) { result = Reflect(vector, normal: normal) }

        public static func Min(_ value1: Vector3, value2: Vector3) -> Vector3 {
            Vector3(value1.X < value2.X ? value1.X : value2.X,
                    value1.Y < value2.Y ? value1.Y : value2.Y,
                    value1.Z < value2.Z ? value1.Z : value2.Z)
        }
        public static func Min(_ value1: inout Vector3, value2: inout Vector3, result: inout Vector3) { result = Min(value1, value2: value2) }
        public static func Max(_ value1: Vector3, value2: Vector3) -> Vector3 {
            Vector3(value1.X > value2.X ? value1.X : value2.X,
                    value1.Y > value2.Y ? value1.Y : value2.Y,
                    value1.Z > value2.Z ? value1.Z : value2.Z)
        }
        public static func Max(_ value1: inout Vector3, value2: inout Vector3, result: inout Vector3) { result = Max(value1, value2: value2) }
        public static func Clamp(_ value1: Vector3, min: Vector3, max: Vector3) -> Vector3 {
            Vector3(MathHelper.Clamp(value1.X, min: min.X, max: max.X),
                    MathHelper.Clamp(value1.Y, min: min.Y, max: max.Y),
                    MathHelper.Clamp(value1.Z, min: min.Z, max: max.Z))
        }
        public static func Clamp(_ value1: inout Vector3, min: inout Vector3, max: inout Vector3, result: inout Vector3) { result = Clamp(value1, min: min, max: max) }
        public static func Lerp(_ value1: Vector3, value2: Vector3, amount: Float) -> Vector3 {
            Vector3(value1.X + (value2.X - value1.X) * amount,
                    value1.Y + (value2.Y - value1.Y) * amount,
                    value1.Z + (value2.Z - value1.Z) * amount)
        }
        public static func Lerp(_ value1: inout Vector3, value2: inout Vector3, amount: Float, result: inout Vector3) { result = Lerp(value1, value2: value2, amount: amount) }
        public static func Barycentric(_ value1: Vector3, value2: Vector3, value3: Vector3, amount1: Float, amount2: Float) -> Vector3 {
            Vector3(MathHelper.Barycentric(value1.X, value2: value2.X, value3: value3.X, amount1: amount1, amount2: amount2),
                    MathHelper.Barycentric(value1.Y, value2: value2.Y, value3: value3.Y, amount1: amount1, amount2: amount2),
                    MathHelper.Barycentric(value1.Z, value2: value2.Z, value3: value3.Z, amount1: amount1, amount2: amount2))
        }
        public static func Barycentric(_ value1: inout Vector3, value2: inout Vector3, value3: inout Vector3, amount1: Float, amount2: Float, result: inout Vector3) { result = Barycentric(value1, value2: value2, value3: value3, amount1: amount1, amount2: amount2) }
        public static func SmoothStep(_ value1: Vector3, value2: Vector3, amount: Float) -> Vector3 {
            Vector3(MathHelper.SmoothStep(value1.X, value2: value2.X, amount: amount),
                    MathHelper.SmoothStep(value1.Y, value2: value2.Y, amount: amount),
                    MathHelper.SmoothStep(value1.Z, value2: value2.Z, amount: amount))
        }
        public static func SmoothStep(_ value1: inout Vector3, value2: inout Vector3, amount: Float, result: inout Vector3) { result = SmoothStep(value1, value2: value2, amount: amount) }
        public static func CatmullRom(_ value1: Vector3, value2: Vector3, value3: Vector3, value4: Vector3, amount: Float) -> Vector3 {
            Vector3(MathHelper.CatmullRom(value1.X, value2: value2.X, value3: value3.X, value4: value4.X, amount: amount),
                    MathHelper.CatmullRom(value1.Y, value2: value2.Y, value3: value3.Y, value4: value4.Y, amount: amount),
                    MathHelper.CatmullRom(value1.Z, value2: value2.Z, value3: value3.Z, value4: value4.Z, amount: amount))
        }
        public static func CatmullRom(_ value1: inout Vector3, value2: inout Vector3, value3: inout Vector3, value4: inout Vector3, amount: Float, result: inout Vector3) { result = CatmullRom(value1, value2: value2, value3: value3, value4: value4, amount: amount) }
        public static func Hermite(_ value1: Vector3, tangent1: Vector3, value2: Vector3, tangent2: Vector3, amount: Float) -> Vector3 {
            Vector3(MathHelper.Hermite(value1.X, tangent1: tangent1.X, value2: value2.X, tangent2: tangent2.X, amount: amount),
                    MathHelper.Hermite(value1.Y, tangent1: tangent1.Y, value2: value2.Y, tangent2: tangent2.Y, amount: amount),
                    MathHelper.Hermite(value1.Z, tangent1: tangent1.Z, value2: value2.Z, tangent2: tangent2.Z, amount: amount))
        }
        public static func Hermite(_ value1: inout Vector3, tangent1: inout Vector3, value2: inout Vector3, tangent2: inout Vector3, amount: Float, result: inout Vector3) { result = Hermite(value1, tangent1: tangent1, value2: value2, tangent2: tangent2, amount: amount) }

        public static func Transform(_ position: Vector3, matrix: Matrix) -> Vector3 {
            Vector3(position.X * matrix.M11 + position.Y * matrix.M21 + position.Z * matrix.M31 + matrix.M41,
                    position.X * matrix.M12 + position.Y * matrix.M22 + position.Z * matrix.M32 + matrix.M42,
                    position.X * matrix.M13 + position.Y * matrix.M23 + position.Z * matrix.M33 + matrix.M43)
        }
        public static func Transform(_ position: inout Vector3, matrix: inout Matrix, result: inout Vector3) { result = Transform(position, matrix: matrix) }
        public static func TransformNormal(_ normal: Vector3, matrix: Matrix) -> Vector3 {
            Vector3(normal.X * matrix.M11 + normal.Y * matrix.M21 + normal.Z * matrix.M31,
                    normal.X * matrix.M12 + normal.Y * matrix.M22 + normal.Z * matrix.M32,
                    normal.X * matrix.M13 + normal.Y * matrix.M23 + normal.Z * matrix.M33)
        }
        public static func TransformNormal(_ normal: inout Vector3, matrix: inout Matrix, result: inout Vector3) { result = TransformNormal(normal, matrix: matrix) }
        public static func Transform(_ value: Vector3, rotation: Quaternion) -> Vector3 {
            let x2 = rotation.X + rotation.X; let y2 = rotation.Y + rotation.Y; let z2 = rotation.Z + rotation.Z
            let wx2 = rotation.W * x2; let wy2 = rotation.W * y2; let wz2 = rotation.W * z2
            let xx2 = rotation.X * x2; let xy2 = rotation.X * y2; let xz2 = rotation.X * z2
            let yy2 = rotation.Y * y2; let yz2 = rotation.Y * z2; let zz2 = rotation.Z * z2
            return Vector3(
                value.X * (1 - yy2 - zz2) + value.Y * (xy2 - wz2) + value.Z * (xz2 + wy2),
                value.X * (xy2 + wz2) + value.Y * (1 - xx2 - zz2) + value.Z * (yz2 - wx2),
                value.X * (xz2 - wy2) + value.Y * (yz2 + wx2) + value.Z * (1 - xx2 - yy2))
        }
        public static func Transform(_ value: inout Vector3, rotation: inout Quaternion, result: inout Vector3) { result = Transform(value, rotation: rotation) }

        public static func Transform(_ sourceArray: [Vector3], matrix: inout Matrix, destinationArray: inout [Vector3]) throws {
            try Transform(sourceArray, sourceIndex: 0, matrix: &matrix, destinationArray: &destinationArray, destinationIndex: 0, length: Int32(sourceArray.count))
        }
        public static func Transform(_ sourceArray: [Vector3], sourceIndex: Int32, matrix: inout Matrix, destinationArray: inout [Vector3], destinationIndex: Int32, length: Int32) throws {
            try xnaValidateTransformArrayRanges(sourceCount: sourceArray.count, sourceIndex: sourceIndex, destinationCount: destinationArray.count, destinationIndex: destinationIndex, length: length)
            var i: Int32 = 0
            while i < length { destinationArray[Int(destinationIndex + i)] = Transform(sourceArray[Int(sourceIndex + i)], matrix: matrix); i += 1 }
        }
        public static func TransformNormal(_ sourceArray: [Vector3], matrix: inout Matrix, destinationArray: inout [Vector3]) throws {
            try TransformNormal(sourceArray, sourceIndex: 0, matrix: &matrix, destinationArray: &destinationArray, destinationIndex: 0, length: Int32(sourceArray.count))
        }
        public static func TransformNormal(_ sourceArray: [Vector3], sourceIndex: Int32, matrix: inout Matrix, destinationArray: inout [Vector3], destinationIndex: Int32, length: Int32) throws {
            try xnaValidateTransformArrayRanges(sourceCount: sourceArray.count, sourceIndex: sourceIndex, destinationCount: destinationArray.count, destinationIndex: destinationIndex, length: length)
            var i: Int32 = 0
            while i < length { destinationArray[Int(destinationIndex + i)] = TransformNormal(sourceArray[Int(sourceIndex + i)], matrix: matrix); i += 1 }
        }
        public static func Transform(_ sourceArray: [Vector3], rotation: inout Quaternion, destinationArray: inout [Vector3]) throws {
            try Transform(sourceArray, sourceIndex: 0, rotation: &rotation, destinationArray: &destinationArray, destinationIndex: 0, length: Int32(sourceArray.count))
        }
        public static func Transform(_ sourceArray: [Vector3], sourceIndex: Int32, rotation: inout Quaternion, destinationArray: inout [Vector3], destinationIndex: Int32, length: Int32) throws {
            try xnaValidateTransformArrayRanges(sourceCount: sourceArray.count, sourceIndex: sourceIndex, destinationCount: destinationArray.count, destinationIndex: destinationIndex, length: length)
            var i: Int32 = 0
            while i < length { destinationArray[Int(destinationIndex + i)] = Transform(sourceArray[Int(sourceIndex + i)], rotation: rotation); i += 1 }
        }

        public static func Negate(_ value: Vector3) -> Vector3 { -value }
        public static func Negate(_ value: inout Vector3, result: inout Vector3) { result = -value }
        public static func Add(_ value1: Vector3, value2: Vector3) -> Vector3 { value1 + value2 }
        public static func Add(_ value1: inout Vector3, value2: inout Vector3, result: inout Vector3) { result = value1 + value2 }
        public static func Subtract(_ value1: Vector3, value2: Vector3) -> Vector3 { value1 - value2 }
        public static func Subtract(_ value1: inout Vector3, value2: inout Vector3, result: inout Vector3) { result = value1 - value2 }
        public static func Multiply(_ value1: Vector3, value2: Vector3) -> Vector3 { value1 * value2 }
        public static func Multiply(_ value1: inout Vector3, value2: inout Vector3, result: inout Vector3) { result = value1 * value2 }
        public static func Multiply(_ value1: Vector3, scaleFactor: Float) -> Vector3 { value1 * scaleFactor }
        public static func Multiply(_ value1: inout Vector3, scaleFactor: Float, result: inout Vector3) { result = value1 * scaleFactor }
        public static func Divide(_ value1: Vector3, value2: Vector3) -> Vector3 { value1 / value2 }
        public static func Divide(_ value1: inout Vector3, value2: inout Vector3, result: inout Vector3) { result = value1 / value2 }
        public static func Divide(_ value1: Vector3, value2: Float) -> Vector3 { value1 / value2 }
        public static func Divide(_ value1: inout Vector3, value2: Float, result: inout Vector3) { result = value1 / value2 }

        public func Equals(_ other: Vector3) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool { guard let other = obj as? Vector3 else { return false }; return self == other }
        public func GetHashCode() -> Int32 { xnaFloatHash(X) &+ xnaFloatHash(Y) &+ xnaFloatHash(Z) }
        public func ToString() -> String { "{X:\(xnaFloatString(X)) Y:\(xnaFloatString(Y)) Z:\(xnaFloatString(Z))}" }

        public static prefix func - (value: Vector3) -> Vector3 { Vector3(-value.X, -value.Y, -value.Z) }
        public static func + (lhs: Vector3, rhs: Vector3) -> Vector3 { Vector3(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z) }
        public static func - (lhs: Vector3, rhs: Vector3) -> Vector3 { Vector3(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z) }
        public static func * (lhs: Vector3, rhs: Vector3) -> Vector3 { Vector3(lhs.X * rhs.X, lhs.Y * rhs.Y, lhs.Z * rhs.Z) }
        public static func * (lhs: Vector3, rhs: Float) -> Vector3 { Vector3(lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs) }
        public static func * (lhs: Float, rhs: Vector3) -> Vector3 { rhs * lhs }
        public static func / (lhs: Vector3, rhs: Vector3) -> Vector3 { Vector3(lhs.X / rhs.X, lhs.Y / rhs.Y, lhs.Z / rhs.Z) }
        public static func / (lhs: Vector3, rhs: Float) -> Vector3 {
            let factor: Float = 1 / rhs
            return Vector3(lhs.X * factor, lhs.Y * factor, lhs.Z * factor)
        }
        public static func == (lhs: Vector3, rhs: Vector3) -> Bool { lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z }
        public static func != (lhs: Vector3, rhs: Vector3) -> Bool { !(lhs == rhs) }
    }
}
