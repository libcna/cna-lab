// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct BoundingBox {
        public static let CornerCount: Int32 = 8

        public var Min: Vector3
        public var Max: Vector3

        public init(_ min: Vector3, _ max: Vector3) {
            Min = min
            Max = max
        }

        public func GetCorners() -> [Vector3] {
            [
                Vector3(Min.X, Max.Y, Max.Z),
                Vector3(Max.X, Max.Y, Max.Z),
                Vector3(Max.X, Min.Y, Max.Z),
                Vector3(Min.X, Min.Y, Max.Z),
                Vector3(Min.X, Max.Y, Min.Z),
                Vector3(Max.X, Max.Y, Min.Z),
                Vector3(Max.X, Min.Y, Min.Z),
                Vector3(Min.X, Min.Y, Min.Z),
            ]
        }

    /// The exact `NotEnoughCorners` message, read out of
        /// `Microsoft.Xna.Framework.dll`'s own resource table.
        internal static let notEnoughCornersMessage =
            "You have to have at least 8 elements to copy corners."

        public func GetCorners(_ corners: inout [Vector3]) throws {
            guard corners.count >= Int(Self.CornerCount) else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "corners",
                    message: BoundingBox.notEnoughCornersMessage)
            }
            let values = GetCorners()
            for index in 0..<Int(Self.CornerCount) { corners[index] = values[index] }
        }

        public static func CreateMerged(_ original: BoundingBox, additional: BoundingBox) -> BoundingBox {
            BoundingBox(Vector3.Min(original.Min, value2: additional.Min),
                        Vector3.Max(original.Max, value2: additional.Max))
        }

        public static func CreateMerged(
            _ original: inout BoundingBox,
            additional: inout BoundingBox,
            result: inout BoundingBox
        ) {
            result = CreateMerged(original, additional: additional)
        }

        public static func CreateFromSphere(_ sphere: BoundingSphere) -> BoundingBox {
            let radius = Vector3(sphere.Radius)
            return BoundingBox(sphere.Center - radius, sphere.Center + radius)
        }

        public static func CreateFromSphere(_ sphere: inout BoundingSphere, result: inout BoundingBox) {
            result = CreateFromSphere(sphere)
        }

        public static func CreateFromPoints(_ points: [Vector3]) throws -> BoundingBox {
            guard !points.isEmpty else {
                // `BoundingBoxZeroPoints`, which — unlike the BoundingSphere
                // message it otherwise resembles — has no trailing period.
                throw CNAArgumentException(
                    message: "You should have at least one point in points")
            }
            var minimum = Vector3(Float.greatestFiniteMagnitude)
            var maximum = Vector3(-Float.greatestFiniteMagnitude)
            for point in points {
                minimum = Vector3.Min(minimum, value2: point)
                maximum = Vector3.Max(maximum, value2: point)
            }
            return BoundingBox(minimum, maximum)
        }

        public func Intersects(_ box: BoundingBox) -> Bool {
            !(Max.X < box.Min.X || Min.X > box.Max.X ||
              Max.Y < box.Min.Y || Min.Y > box.Max.Y ||
              Max.Z < box.Min.Z || Min.Z > box.Max.Z)
        }

        public func Intersects(_ box: inout BoundingBox, result: inout Bool) { result = Intersects(box) }

        public func Intersects(_ frustum: BoundingFrustum) -> Bool { frustum.Intersects(self) }

        public func Intersects(_ plane: Plane) -> PlaneIntersectionType { plane.Intersects(self) }
        public func Intersects(_ plane: inout Plane, result: inout PlaneIntersectionType) {
            result = Intersects(plane)
        }

        public func Intersects(_ ray: Ray) -> Float? {
            var distance: Float = 0
            var maximumDistance = Float.greatestFiniteMagnitude
            if !Self.intersectSlab(ray.Position.X, ray.Direction.X, Min.X, Max.X, &distance, &maximumDistance) ||
                !Self.intersectSlab(ray.Position.Y, ray.Direction.Y, Min.Y, Max.Y, &distance, &maximumDistance) ||
                !Self.intersectSlab(ray.Position.Z, ray.Direction.Z, Min.Z, Max.Z, &distance, &maximumDistance) {
                return nil
            }
            return distance
        }

        public func Intersects(_ ray: inout Ray, result: inout Float?) { result = Intersects(ray) }

        public func Intersects(_ sphere: BoundingSphere) -> Bool {
            let closest = Vector3.Clamp(sphere.Center, min: Min, max: Max)
            return !(Vector3.DistanceSquared(sphere.Center, value2: closest) > sphere.Radius * sphere.Radius)
        }

        public func Intersects(_ sphere: inout BoundingSphere, result: inout Bool) {
            result = Intersects(sphere)
        }

        public func Contains(_ box: BoundingBox) -> ContainmentType {
            if Max.X < box.Min.X || Min.X > box.Max.X ||
                Max.Y < box.Min.Y || Min.Y > box.Max.Y ||
                Max.Z < box.Min.Z || Min.Z > box.Max.Z {
                return .Disjoint
            }
            return Min.X <= box.Min.X && box.Max.X <= Max.X &&
                Min.Y <= box.Min.Y && box.Max.Y <= Max.Y &&
                Min.Z <= box.Min.Z && box.Max.Z <= Max.Z ? .Contains : .Intersects
        }

        public func Contains(_ box: inout BoundingBox, result: inout ContainmentType) {
            result = Contains(box)
        }

        public func Contains(_ frustum: BoundingFrustum) -> ContainmentType {
            var allInside = true
            for corner in frustum.GetCorners() where Contains(corner) == .Disjoint {
                allInside = false
                break
            }
            if allInside { return .Contains }
            return Intersects(frustum) ? .Intersects : .Disjoint
        }

        public func Contains(_ point: Vector3) -> ContainmentType {
            Min.X <= point.X && point.X <= Max.X &&
                Min.Y <= point.Y && point.Y <= Max.Y &&
                Min.Z <= point.Z && point.Z <= Max.Z ? .Contains : .Disjoint
        }

        public func Contains(_ point: inout Vector3, result: inout ContainmentType) {
            result = Contains(point)
        }

        public func Contains(_ sphere: BoundingSphere) -> ContainmentType {
            let closest = Vector3.Clamp(sphere.Center, min: Min, max: Max)
            let distanceSquared = Vector3.DistanceSquared(sphere.Center, value2: closest)
            let radius = sphere.Radius
            if distanceSquared > radius * radius { return .Disjoint }
            // XNA 4.0 observably repeats the X-width check for the Z dimension.
            return Min.X + radius <= sphere.Center.X && sphere.Center.X <= Max.X - radius &&
                Max.X - Min.X > radius &&
                Min.Y + radius <= sphere.Center.Y && sphere.Center.Y <= Max.Y - radius &&
                Max.Y - Min.Y > radius &&
                Min.Z + radius <= sphere.Center.Z && sphere.Center.Z <= Max.Z - radius &&
                Max.X - Min.X > radius ? .Contains : .Intersects
        }

        public func Contains(_ sphere: inout BoundingSphere, result: inout ContainmentType) {
            result = Contains(sphere)
        }

        public func Equals(_ other: BoundingBox) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? BoundingBox else { return false }
            return self == other
        }
        public func GetHashCode() -> Int32 { Min.GetHashCode() &+ Max.GetHashCode() }
        public func ToString() -> String { "{Min:\(Min.ToString()) Max:\(Max.ToString())}" }

        public static func == (lhs: BoundingBox, rhs: BoundingBox) -> Bool {
            lhs.Min == rhs.Min && lhs.Max == rhs.Max
        }
        public static func != (lhs: BoundingBox, rhs: BoundingBox) -> Bool { !(lhs == rhs) }

        internal func supportMapping(_ direction: Vector3) -> Vector3 {
            Vector3(direction.X >= 0 ? Max.X : Min.X,
                    direction.Y >= 0 ? Max.Y : Min.Y,
                    direction.Z >= 0 ? Max.Z : Min.Z)
        }

        private static func intersectSlab(
            _ position: Float,
            _ direction: Float,
            _ minimum: Float,
            _ maximum: Float,
            _ distance: inout Float,
            _ maximumDistance: inout Float
        ) -> Bool {
            if abs(direction) < 1e-6 { return position >= minimum && position <= maximum }
            let inverseDirection: Float = 1 / direction
            var near = (minimum - position) * inverseDirection
            var far = (maximum - position) * inverseDirection
            if near > far { swap(&near, &far) }
            distance = MathHelper.Max(near, value2: distance)
            maximumDistance = MathHelper.Min(far, value2: maximumDistance)
            return distance <= maximumDistance
        }
    }
}
