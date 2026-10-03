// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Plane {
        public var Normal: Vector3
        public var D: Float

        public init(_ a: Float, _ b: Float, _ c: Float, _ d: Float) {
            Normal = Vector3(a, b, c); D = d
        }
        public init(_ normal: Vector3, _ d: Float) { Normal = normal; D = d }
        public init(_ value: Vector4) { Normal = Vector3(value.X, value.Y, value.Z); D = value.W }
        public init(_ point1: Vector3, _ point2: Vector3, _ point3: Vector3) {
            let difference1 = point2 - point1
            let difference2 = point3 - point1
            Normal = Vector3.Cross(difference1, vector2: difference2)
            Normal.Normalize()
            D = -Vector3.Dot(Normal, vector2: point1)
        }

        public mutating func Normalize() {
            let lengthSquared = Normal.LengthSquared()
            if abs(lengthSquared - 1) < 1.1920929e-7 { return }
            let factor: Float = 1 / xnaSqrt(lengthSquared)
            Normal = Normal * factor; D *= factor
        }
        public static func Normalize(_ value: Plane) -> Plane { var value = value; value.Normalize(); return value }
        public static func Normalize(_ value: inout Plane, result: inout Plane) { result = Normalize(value) }

        public static func Transform(_ plane: Plane, matrix: Matrix) -> Plane {
            let inverse = Matrix.Invert(matrix)
            let x = plane.Normal.X, y = plane.Normal.Y, z = plane.Normal.Z, d = plane.D
            return Plane(
                (x * inverse.M11) + (y * inverse.M12) + (z * inverse.M13) + (d * inverse.M14),
                (x * inverse.M21) + (y * inverse.M22) + (z * inverse.M23) + (d * inverse.M24),
                (x * inverse.M31) + (y * inverse.M32) + (z * inverse.M33) + (d * inverse.M34),
                (x * inverse.M41) + (y * inverse.M42) + (z * inverse.M43) + (d * inverse.M44))
        }

        public static func Transform(_ plane: inout Plane, matrix: inout Matrix, result: inout Plane) {
            result = Transform(plane, matrix: matrix)
        }

        public static func Transform(_ plane: Plane, rotation: Quaternion) -> Plane {
            Plane(Vector3.Transform(plane.Normal, rotation: rotation), plane.D)
        }

        public static func Transform(_ plane: inout Plane, rotation: inout Quaternion, result: inout Plane) {
            result = Transform(plane, rotation: rotation)
        }

        public func Dot(_ value: Vector4) -> Float {
            (Normal.X * value.X) + (Normal.Y * value.Y) + (Normal.Z * value.Z) + (D * value.W)
        }

        public func Dot(_ value: inout Vector4, result: inout Float) { result = Dot(value) }

        public func DotCoordinate(_ value: Vector3) -> Float {
            (Normal.X * value.X) + (Normal.Y * value.Y) + (Normal.Z * value.Z) + D
        }

        public func DotCoordinate(_ value: inout Vector3, result: inout Float) { result = DotCoordinate(value) }

        public func DotNormal(_ value: Vector3) -> Float {
            (Normal.X * value.X) + (Normal.Y * value.Y) + (Normal.Z * value.Z)
        }

        public func DotNormal(_ value: inout Vector3, result: inout Float) { result = DotNormal(value) }

        public func Intersects(_ box: BoundingBox) -> PlaneIntersectionType {
            let negative = Vector3(
                Normal.X >= 0 ? box.Min.X : box.Max.X,
                Normal.Y >= 0 ? box.Min.Y : box.Max.Y,
                Normal.Z >= 0 ? box.Min.Z : box.Max.Z)
            let positive = Vector3(
                Normal.X >= 0 ? box.Max.X : box.Min.X,
                Normal.Y >= 0 ? box.Max.Y : box.Min.Y,
                Normal.Z >= 0 ? box.Max.Z : box.Min.Z)
            var distance = (Normal.X * negative.X) + (Normal.Y * negative.Y) + (Normal.Z * negative.Z) + D
            if distance > 0 { return .Front }
            distance = (Normal.X * positive.X) + (Normal.Y * positive.Y) + (Normal.Z * positive.Z) + D
            return distance < 0 ? .Back : .Intersecting
        }

        public func Intersects(_ box: inout BoundingBox, result: inout PlaneIntersectionType) {
            result = Intersects(box)
        }

        public func Intersects(_ frustum: BoundingFrustum) -> PlaneIntersectionType {
            frustum.Intersects(self)
        }

        public func Intersects(_ sphere: BoundingSphere) -> PlaneIntersectionType {
            let distance = (sphere.Center.X * Normal.X) + (sphere.Center.Y * Normal.Y) +
                (sphere.Center.Z * Normal.Z) + D
            if distance > sphere.Radius { return .Front }
            if distance < -sphere.Radius { return .Back }
            return .Intersecting
        }

        public func Intersects(_ sphere: inout BoundingSphere, result: inout PlaneIntersectionType) {
            result = Intersects(sphere)
        }

        public func Equals(_ other: Plane) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Plane else { return false }
            return self == other
        }
        public func GetHashCode() -> Int32 { Normal.GetHashCode() &+ xnaFloatHash(D) }
        public func ToString() -> String { "{Normal:\(Normal.ToString()) D:\(xnaFloatString(D))}" }

        public static func == (lhs: Plane, rhs: Plane) -> Bool {
            lhs.Normal == rhs.Normal && lhs.D == rhs.D
        }
        public static func != (lhs: Plane, rhs: Plane) -> Bool { !(lhs == rhs) }
    }
}
