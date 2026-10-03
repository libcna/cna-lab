// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct BoundingSphere {
        public var Center: Vector3
        public var Radius: Float

        public init(_ center: Vector3, _ radius: Float) throws {
            // `ArgumentException(FrameworkResources.NegativeRadius)` --
            // the message-only overload, so ParamName is nil.
            if radius < 0 { throw CNAArgumentException(message: "Radius must be greater than 0.") }
            Center = center
            Radius = radius
        }

        internal init(uncheckedCenter center: Vector3, radius: Float) {
            Center = center
            Radius = radius
        }

        public static func CreateMerged(_ original: BoundingSphere, additional: BoundingSphere) -> BoundingSphere {
            let difference = additional.Center - original.Center
            let distance = difference.Length()
            let radius = original.Radius
            let otherRadius = additional.Radius
            if radius + otherRadius >= distance {
                if radius - otherRadius >= distance { return original }
                if otherRadius - radius >= distance { return additional }
            }
            let direction = difference * (1 / distance)
            let minimum = MathHelper.Min(-radius, value2: distance - otherRadius)
            let maximum = MathHelper.Max(radius, value2: distance + otherRadius)
            let mergedRadius = (maximum - minimum) * 0.5
            return BoundingSphere(
                uncheckedCenter: original.Center + direction * (mergedRadius + minimum),
                radius: mergedRadius)
        }

        public static func CreateMerged(
            _ original: inout BoundingSphere,
            additional: inout BoundingSphere,
            result: inout BoundingSphere
        ) {
            result = CreateMerged(original, additional: additional)
        }

        public static func CreateFromBoundingBox(_ box: BoundingBox) -> BoundingSphere {
            let center = Vector3.Lerp(box.Min, value2: box.Max, amount: 0.5)
            let radius = Vector3.Distance(box.Min, value2: box.Max) * 0.5
            return BoundingSphere(uncheckedCenter: center, radius: radius)
        }

        public static func CreateFromBoundingBox(_ box: inout BoundingBox, result: inout BoundingSphere) {
            result = CreateFromBoundingBox(box)
        }

        public static func CreateFromPoints(_ points: [Vector3]) throws -> BoundingSphere {
            guard let first = points.first else {
                // `BoundingSphereZeroPoints`. It ends with a period and
                // `BoundingBoxZeroPoints` does not; the two are separate
                // resources and are not interchangeable.
                throw CNAArgumentException(
                    message: "You should have at least one point in points.")
            }
            var minimumX = first, maximumX = first
            var minimumY = first, maximumY = first
            var minimumZ = first, maximumZ = first
            for point in points {
                if point.X < minimumX.X { minimumX = point }
                if point.X > maximumX.X { maximumX = point }
                if point.Y < minimumY.Y { minimumY = point }
                if point.Y > maximumY.Y { maximumY = point }
                if point.Z < minimumZ.Z { minimumZ = point }
                if point.Z > maximumZ.Z { maximumZ = point }
            }

            let distanceX = Vector3.Distance(maximumX, value2: minimumX)
            let distanceY = Vector3.Distance(maximumY, value2: minimumY)
            let distanceZ = Vector3.Distance(maximumZ, value2: minimumZ)
            var center: Vector3
            var radius: Float
            if distanceX > distanceY {
                if distanceX > distanceZ {
                    center = Vector3.Lerp(maximumX, value2: minimumX, amount: 0.5)
                    radius = distanceX * 0.5
                } else {
                    center = Vector3.Lerp(maximumZ, value2: minimumZ, amount: 0.5)
                    radius = distanceZ * 0.5
                }
            } else if distanceY > distanceZ {
                center = Vector3.Lerp(maximumY, value2: minimumY, amount: 0.5)
                radius = distanceY * 0.5
            } else {
                center = Vector3.Lerp(maximumZ, value2: minimumZ, amount: 0.5)
                radius = distanceZ * 0.5
            }

            for point in points {
                let offset = point - center
                let distance = offset.Length()
                if distance > radius {
                    radius = (radius + distance) * 0.5
                    center = center + (1 - (radius / distance)) * offset
                }
            }
            return BoundingSphere(uncheckedCenter: center, radius: radius)
        }

        public static func CreateFromFrustum(_ frustum: BoundingFrustum) -> BoundingSphere {
            // A frustum always has exactly eight corners.
            try! CreateFromPoints(frustum.GetCorners())
        }

        public func Intersects(_ box: BoundingBox) -> Bool {
            let closest = Vector3.Clamp(Center, min: box.Min, max: box.Max)
            return !(Vector3.DistanceSquared(Center, value2: closest) > Radius * Radius)
        }

        public func Intersects(_ box: inout BoundingBox, result: inout Bool) { result = Intersects(box) }
        public func Intersects(_ frustum: BoundingFrustum) -> Bool { frustum.Intersects(self) }
        public func Intersects(_ plane: Plane) -> PlaneIntersectionType { plane.Intersects(self) }
        public func Intersects(_ plane: inout Plane, result: inout PlaneIntersectionType) {
            result = Intersects(plane)
        }
        public func Intersects(_ ray: Ray) -> Float? { ray.Intersects(self) }
        public func Intersects(_ ray: inout Ray, result: inout Float?) { result = Intersects(ray) }

        public func Intersects(_ sphere: BoundingSphere) -> Bool {
            let distanceSquared = Vector3.DistanceSquared(Center, value2: sphere.Center)
            return (Radius * Radius) + (2 * Radius * sphere.Radius) + (sphere.Radius * sphere.Radius) >
                distanceSquared
        }

        public func Intersects(_ sphere: inout BoundingSphere, result: inout Bool) {
            result = Intersects(sphere)
        }

        public func Contains(_ box: BoundingBox) -> ContainmentType {
            if !box.Intersects(self) { return .Disjoint }
            let radiusSquared = Radius * Radius
            for corner in box.GetCorners() {
                if (Center - corner).LengthSquared() > radiusSquared { return .Intersects }
            }
            return .Contains
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
            Vector3.DistanceSquared(Center, value2: point) < Radius * Radius ? .Contains : .Disjoint
        }

        public func Contains(_ point: inout Vector3, result: inout ContainmentType) {
            result = Contains(point)
        }

        public func Contains(_ sphere: BoundingSphere) -> ContainmentType {
            let distance = Vector3.Distance(Center, value2: sphere.Center)
            if !(Radius + sphere.Radius >= distance) { return .Disjoint }
            return Radius - sphere.Radius >= distance ? .Contains : .Intersects
        }

        public func Contains(_ sphere: inout BoundingSphere, result: inout ContainmentType) {
            result = Contains(sphere)
        }

        public func Transform(_ matrix: Matrix) -> BoundingSphere {
            let row1 = (matrix.M11 * matrix.M11) + (matrix.M12 * matrix.M12) + (matrix.M13 * matrix.M13)
            let row2 = (matrix.M21 * matrix.M21) + (matrix.M22 * matrix.M22) + (matrix.M23 * matrix.M23)
            let row3 = (matrix.M31 * matrix.M31) + (matrix.M32 * matrix.M32) + (matrix.M33 * matrix.M33)
            let maximum = MathHelper.Max(row1, value2: MathHelper.Max(row2, value2: row3))
            return BoundingSphere(
                uncheckedCenter: Vector3.Transform(Center, matrix: matrix),
                radius: Radius * xnaSqrt(maximum))
        }

        public func Transform(_ matrix: inout Matrix, result: inout BoundingSphere) {
            result = Transform(matrix)
        }

        public func Equals(_ other: BoundingSphere) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? BoundingSphere else { return false }
            return self == other
        }
        public func GetHashCode() -> Int32 { Center.GetHashCode() &+ xnaFloatHash(Radius) }
        public func ToString() -> String { "{Center:\(Center.ToString()) Radius:\(xnaFloatString(Radius))}" }

        public static func == (lhs: BoundingSphere, rhs: BoundingSphere) -> Bool {
            lhs.Center == rhs.Center && lhs.Radius == rhs.Radius
        }
        public static func != (lhs: BoundingSphere, rhs: BoundingSphere) -> Bool { !(lhs == rhs) }

        internal func supportMapping(_ direction: Vector3) -> Vector3 {
            let scale = Radius / direction.Length()
            return Vector3(Center.X + direction.X * scale,
                           Center.Y + direction.Y * scale,
                           Center.Z + direction.Z * scale)
        }
    }
}
