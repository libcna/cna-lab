// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Vector2 {
        public var X: Float
        public var Y: Float

        public init(_ x: Float, _ y: Float) { X = x; Y = y }
        public init(_ value: Float) { X = value; Y = value }

        public static var Zero: Vector2 { Vector2(0, 0) }
        public static var One: Vector2 { Vector2(1, 1) }
        public static var UnitX: Vector2 { Vector2(1, 0) }
        public static var UnitY: Vector2 { Vector2(0, 1) }

        public func Length() -> Float { xnaSqrt(X * X + Y * Y) }
        public func LengthSquared() -> Float { X * X + Y * Y }

        public mutating func Normalize() {
            let factor: Float = 1 / xnaSqrt(X * X + Y * Y)
            X *= factor; Y *= factor
        }

        public static func Normalize(_ value: Vector2) -> Vector2 {
            var value = value; value.Normalize(); return value
        }
        public static func Normalize(_ value: inout Vector2, result: inout Vector2) { result = Normalize(value) }

        public static func Distance(_ value1: Vector2, value2: Vector2) -> Float {
            xnaSqrt(DistanceSquared(value1, value2: value2))
        }
        public static func Distance(_ value1: inout Vector2, value2: inout Vector2, result: inout Float) {
            result = Distance(value1, value2: value2)
        }
        public static func DistanceSquared(_ value1: Vector2, value2: Vector2) -> Float {
            let x = value1.X - value2.X; let y = value1.Y - value2.Y
            return x * x + y * y
        }
        public static func DistanceSquared(_ value1: inout Vector2, value2: inout Vector2, result: inout Float) {
            result = DistanceSquared(value1, value2: value2)
        }

        public static func Dot(_ value1: Vector2, value2: Vector2) -> Float {
            value1.X * value2.X + value1.Y * value2.Y
        }
        public static func Dot(_ value1: inout Vector2, value2: inout Vector2, result: inout Float) {
            result = Dot(value1, value2: value2)
        }

        public static func Reflect(_ vector: Vector2, normal: Vector2) -> Vector2 {
            let factor = 2 * Dot(vector, value2: normal)
            return Vector2(vector.X - factor * normal.X, vector.Y - factor * normal.Y)
        }
        public static func Reflect(_ vector: inout Vector2, normal: inout Vector2, result: inout Vector2) {
            result = Reflect(vector, normal: normal)
        }

        public static func Min(_ value1: Vector2, value2: Vector2) -> Vector2 {
            Vector2(value1.X < value2.X ? value1.X : value2.X,
                    value1.Y < value2.Y ? value1.Y : value2.Y)
        }
        public static func Min(_ value1: inout Vector2, value2: inout Vector2, result: inout Vector2) {
            result = Min(value1, value2: value2)
        }
        public static func Max(_ value1: Vector2, value2: Vector2) -> Vector2 {
            Vector2(value1.X > value2.X ? value1.X : value2.X,
                    value1.Y > value2.Y ? value1.Y : value2.Y)
        }
        public static func Max(_ value1: inout Vector2, value2: inout Vector2, result: inout Vector2) {
            result = Max(value1, value2: value2)
        }
        public static func Clamp(_ value1: Vector2, min: Vector2, max: Vector2) -> Vector2 {
            Vector2(MathHelper.Clamp(value1.X, min: min.X, max: max.X),
                    MathHelper.Clamp(value1.Y, min: min.Y, max: max.Y))
        }
        public static func Clamp(_ value1: inout Vector2, min: inout Vector2, max: inout Vector2, result: inout Vector2) {
            result = Clamp(value1, min: min, max: max)
        }
        public static func Lerp(_ value1: Vector2, value2: Vector2, amount: Float) -> Vector2 {
            Vector2(value1.X + (value2.X - value1.X) * amount,
                    value1.Y + (value2.Y - value1.Y) * amount)
        }
        public static func Lerp(_ value1: inout Vector2, value2: inout Vector2, amount: Float, result: inout Vector2) {
            result = Lerp(value1, value2: value2, amount: amount)
        }
        public static func Barycentric(_ value1: Vector2, value2: Vector2, value3: Vector2, amount1: Float, amount2: Float) -> Vector2 {
            Vector2(MathHelper.Barycentric(value1.X, value2: value2.X, value3: value3.X, amount1: amount1, amount2: amount2),
                    MathHelper.Barycentric(value1.Y, value2: value2.Y, value3: value3.Y, amount1: amount1, amount2: amount2))
        }
        public static func Barycentric(_ value1: inout Vector2, value2: inout Vector2, value3: inout Vector2, amount1: Float, amount2: Float, result: inout Vector2) {
            result = Barycentric(value1, value2: value2, value3: value3, amount1: amount1, amount2: amount2)
        }
        public static func SmoothStep(_ value1: Vector2, value2: Vector2, amount: Float) -> Vector2 {
            Vector2(MathHelper.SmoothStep(value1.X, value2: value2.X, amount: amount),
                    MathHelper.SmoothStep(value1.Y, value2: value2.Y, amount: amount))
        }
        public static func SmoothStep(_ value1: inout Vector2, value2: inout Vector2, amount: Float, result: inout Vector2) {
            result = SmoothStep(value1, value2: value2, amount: amount)
        }
        public static func CatmullRom(_ value1: Vector2, value2: Vector2, value3: Vector2, value4: Vector2, amount: Float) -> Vector2 {
            Vector2(MathHelper.CatmullRom(value1.X, value2: value2.X, value3: value3.X, value4: value4.X, amount: amount),
                    MathHelper.CatmullRom(value1.Y, value2: value2.Y, value3: value3.Y, value4: value4.Y, amount: amount))
        }
        public static func CatmullRom(_ value1: inout Vector2, value2: inout Vector2, value3: inout Vector2, value4: inout Vector2, amount: Float, result: inout Vector2) {
            result = CatmullRom(value1, value2: value2, value3: value3, value4: value4, amount: amount)
        }
        public static func Hermite(_ value1: Vector2, tangent1: Vector2, value2: Vector2, tangent2: Vector2, amount: Float) -> Vector2 {
            Vector2(MathHelper.Hermite(value1.X, tangent1: tangent1.X, value2: value2.X, tangent2: tangent2.X, amount: amount),
                    MathHelper.Hermite(value1.Y, tangent1: tangent1.Y, value2: value2.Y, tangent2: tangent2.Y, amount: amount))
        }
        public static func Hermite(_ value1: inout Vector2, tangent1: inout Vector2, value2: inout Vector2, tangent2: inout Vector2, amount: Float, result: inout Vector2) {
            result = Hermite(value1, tangent1: tangent1, value2: value2, tangent2: tangent2, amount: amount)
        }

        public static func Transform(_ position: Vector2, matrix: Matrix) -> Vector2 {
            Vector2(position.X * matrix.M11 + position.Y * matrix.M21 + matrix.M41,
                    position.X * matrix.M12 + position.Y * matrix.M22 + matrix.M42)
        }
        public static func Transform(_ position: inout Vector2, matrix: inout Matrix, result: inout Vector2) {
            result = Transform(position, matrix: matrix)
        }
        public static func TransformNormal(_ normal: Vector2, matrix: Matrix) -> Vector2 {
            Vector2(normal.X * matrix.M11 + normal.Y * matrix.M21,
                    normal.X * matrix.M12 + normal.Y * matrix.M22)
        }
        public static func TransformNormal(_ normal: inout Vector2, matrix: inout Matrix, result: inout Vector2) {
            result = TransformNormal(normal, matrix: matrix)
        }
        public static func Transform(_ value: Vector2, rotation: Quaternion) -> Vector2 {
            let x2 = rotation.X + rotation.X; let y2 = rotation.Y + rotation.Y; let z2 = rotation.Z + rotation.Z
            let wz2 = rotation.W * z2; let xx2 = rotation.X * x2; let xy2 = rotation.X * y2
            let yy2 = rotation.Y * y2; let zz2 = rotation.Z * z2
            return Vector2(value.X * (1 - yy2 - zz2) + value.Y * (xy2 - wz2),
                           value.X * (xy2 + wz2) + value.Y * (1 - xx2 - zz2))
        }
        public static func Transform(_ value: inout Vector2, rotation: inout Quaternion, result: inout Vector2) {
            result = Transform(value, rotation: rotation)
        }

        public static func Transform(_ sourceArray: [Vector2], matrix: inout Matrix, destinationArray: inout [Vector2]) throws {
            try Transform(sourceArray, sourceIndex: 0, matrix: &matrix, destinationArray: &destinationArray, destinationIndex: 0, length: Int32(sourceArray.count))
        }
        public static func Transform(_ sourceArray: [Vector2], sourceIndex: Int32, matrix: inout Matrix, destinationArray: inout [Vector2], destinationIndex: Int32, length: Int32) throws {
            try xnaValidateTransformArrayRanges(sourceCount: sourceArray.count, sourceIndex: sourceIndex, destinationCount: destinationArray.count, destinationIndex: destinationIndex, length: length)
            var i: Int32 = 0
            while i < length {
                destinationArray[Int(destinationIndex + i)] = Transform(sourceArray[Int(sourceIndex + i)], matrix: matrix)
                i += 1
            }
        }
        public static func TransformNormal(_ sourceArray: [Vector2], matrix: inout Matrix, destinationArray: inout [Vector2]) throws {
            try TransformNormal(sourceArray, sourceIndex: 0, matrix: &matrix, destinationArray: &destinationArray, destinationIndex: 0, length: Int32(sourceArray.count))
        }
        public static func TransformNormal(_ sourceArray: [Vector2], sourceIndex: Int32, matrix: inout Matrix, destinationArray: inout [Vector2], destinationIndex: Int32, length: Int32) throws {
            try xnaValidateTransformArrayRanges(sourceCount: sourceArray.count, sourceIndex: sourceIndex, destinationCount: destinationArray.count, destinationIndex: destinationIndex, length: length)
            var i: Int32 = 0
            while i < length {
                destinationArray[Int(destinationIndex + i)] = TransformNormal(sourceArray[Int(sourceIndex + i)], matrix: matrix)
                i += 1
            }
        }
        public static func Transform(_ sourceArray: [Vector2], rotation: inout Quaternion, destinationArray: inout [Vector2]) throws {
            try Transform(sourceArray, sourceIndex: 0, rotation: &rotation, destinationArray: &destinationArray, destinationIndex: 0, length: Int32(sourceArray.count))
        }
        public static func Transform(_ sourceArray: [Vector2], sourceIndex: Int32, rotation: inout Quaternion, destinationArray: inout [Vector2], destinationIndex: Int32, length: Int32) throws {
            try xnaValidateTransformArrayRanges(sourceCount: sourceArray.count, sourceIndex: sourceIndex, destinationCount: destinationArray.count, destinationIndex: destinationIndex, length: length)
            var i: Int32 = 0
            while i < length {
                destinationArray[Int(destinationIndex + i)] = Transform(sourceArray[Int(sourceIndex + i)], rotation: rotation)
                i += 1
            }
        }

        public static func Negate(_ value: Vector2) -> Vector2 { -value }
        public static func Negate(_ value: inout Vector2, result: inout Vector2) { result = -value }
        public static func Add(_ value1: Vector2, value2: Vector2) -> Vector2 { value1 + value2 }
        public static func Add(_ value1: inout Vector2, value2: inout Vector2, result: inout Vector2) { result = value1 + value2 }
        public static func Subtract(_ value1: Vector2, value2: Vector2) -> Vector2 { value1 - value2 }
        public static func Subtract(_ value1: inout Vector2, value2: inout Vector2, result: inout Vector2) { result = value1 - value2 }
        public static func Multiply(_ value1: Vector2, value2: Vector2) -> Vector2 { value1 * value2 }
        public static func Multiply(_ value1: inout Vector2, value2: inout Vector2, result: inout Vector2) { result = value1 * value2 }
        public static func Multiply(_ value1: Vector2, scaleFactor: Float) -> Vector2 { value1 * scaleFactor }
        public static func Multiply(_ value1: inout Vector2, scaleFactor: Float, result: inout Vector2) { result = value1 * scaleFactor }
        public static func Divide(_ value1: Vector2, value2: Vector2) -> Vector2 { value1 / value2 }
        public static func Divide(_ value1: inout Vector2, value2: inout Vector2, result: inout Vector2) { result = value1 / value2 }
        public static func Divide(_ value1: Vector2, divider: Float) -> Vector2 { value1 / divider }
        public static func Divide(_ value1: inout Vector2, divider: Float, result: inout Vector2) { result = value1 / divider }

        public func Equals(_ other: Vector2) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool { guard let other = obj as? Vector2 else { return false }; return self == other }
        public func GetHashCode() -> Int32 { xnaFloatHash(X) &+ xnaFloatHash(Y) }
        public func ToString() -> String { "{X:\(xnaFloatString(X)) Y:\(xnaFloatString(Y))}" }

        public static prefix func - (value: Vector2) -> Vector2 { Vector2(-value.X, -value.Y) }
        public static func + (lhs: Vector2, rhs: Vector2) -> Vector2 { Vector2(lhs.X + rhs.X, lhs.Y + rhs.Y) }
        public static func - (lhs: Vector2, rhs: Vector2) -> Vector2 { Vector2(lhs.X - rhs.X, lhs.Y - rhs.Y) }
        public static func * (lhs: Vector2, rhs: Vector2) -> Vector2 { Vector2(lhs.X * rhs.X, lhs.Y * rhs.Y) }
        public static func * (lhs: Vector2, rhs: Float) -> Vector2 { Vector2(lhs.X * rhs, lhs.Y * rhs) }
        public static func * (lhs: Float, rhs: Vector2) -> Vector2 { Vector2(rhs.X * lhs, rhs.Y * lhs) }
        public static func / (lhs: Vector2, rhs: Vector2) -> Vector2 { Vector2(lhs.X / rhs.X, lhs.Y / rhs.Y) }
        public static func / (lhs: Vector2, rhs: Float) -> Vector2 {
            let factor: Float = 1 / rhs
            return Vector2(lhs.X * factor, lhs.Y * factor)
        }
        public static func == (lhs: Vector2, rhs: Vector2) -> Bool { lhs.X == rhs.X && lhs.Y == rhs.Y }
        public static func != (lhs: Vector2, rhs: Vector2) -> Bool { !(lhs == rhs) }
    }
}
