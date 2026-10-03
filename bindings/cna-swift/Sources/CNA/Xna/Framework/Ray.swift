// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Ray {
        public var Position: Vector3
        public var Direction: Vector3

        public init(_ position: Vector3, _ direction: Vector3) {
            Position = position
            Direction = direction
        }

        public func Intersects(_ box: BoundingBox) -> Float? { box.Intersects(self) }
        public func Intersects(_ box: inout BoundingBox, result: inout Float?) { result = Intersects(box) }

        public func Intersects(_ frustum: BoundingFrustum) -> Float? { frustum.Intersects(self) }

        public func Intersects(_ plane: Plane) -> Float? {
            let denominator = (plane.Normal.X * Direction.X) +
                (plane.Normal.Y * Direction.Y) + (plane.Normal.Z * Direction.Z)
            if abs(denominator) < 1e-5 { return nil }
            let positionDot = (plane.Normal.X * Position.X) +
                (plane.Normal.Y * Position.Y) + (plane.Normal.Z * Position.Z)
            let distance = (-plane.D - positionDot) / denominator
            if distance < 0 { return distance < -1e-5 ? nil : 0 }
            return distance
        }

        public func Intersects(_ plane: inout Plane, result: inout Float?) { result = Intersects(plane) }

        public func Intersects(_ sphere: BoundingSphere) -> Float? {
            let x = sphere.Center.X - Position.X
            let y = sphere.Center.Y - Position.Y
            let z = sphere.Center.Z - Position.Z
            let distanceSquared = (x * x) + (y * y) + (z * z)
            let radiusSquared = sphere.Radius * sphere.Radius
            if distanceSquared <= radiusSquared { return 0 }
            let projection = (x * Direction.X) + (y * Direction.Y) + (z * Direction.Z)
            if projection < 0 { return nil }
            let closestDistanceSquared = distanceSquared - (projection * projection)
            if closestDistanceSquared > radiusSquared { return nil }
            return projection - xnaSqrt(radiusSquared - closestDistanceSquared)
        }

        public func Intersects(_ sphere: inout BoundingSphere, result: inout Float?) {
            result = Intersects(sphere)
        }

        public func Equals(_ other: Ray) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Ray else { return false }
            return self == other
        }
        public func GetHashCode() -> Int32 { Position.GetHashCode() &+ Direction.GetHashCode() }
        public func ToString() -> String { "{Position:\(Position.ToString()) Direction:\(Direction.ToString())}" }

        public static func == (lhs: Ray, rhs: Ray) -> Bool {
            lhs.Position == rhs.Position && lhs.Direction == rhs.Direction
        }
        public static func != (lhs: Ray, rhs: Ray) -> Bool { !(lhs == rhs) }
    }
}
