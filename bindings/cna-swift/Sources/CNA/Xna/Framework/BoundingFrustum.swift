// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    open class BoundingFrustum {
        public static let CornerCount: Int32 = 8

        private var matrixValue: Matrix
        private var planes: [Plane]
        private var corners: [Vector3]
        private var gjk: XnaGjk?

        public init(value: Matrix) {
            matrixValue = value
            planes = Array(repeating: Plane(.Zero, 0), count: 6)
            corners = Array(repeating: .Zero, count: Int(Self.CornerCount))
            setMatrix(value)
        }

        public var Near: Plane { planes[0] }
        public var Far: Plane { planes[1] }
        public var Left: Plane { planes[2] }
        public var Right: Plane { planes[3] }
        public var Top: Plane { planes[4] }
        public var Bottom: Plane { planes[5] }

        public var Matrix: Microsoft.Xna.Framework.Matrix {
            get { matrixValue }
            set { setMatrix(newValue) }
        }

        public func GetCorners() -> [Vector3] { corners }

        public func GetCorners(_ corners: inout [Vector3]) throws {
            guard corners.count >= Int(Self.CornerCount) else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "corners",
                    message: Microsoft.Xna.Framework.BoundingBox.notEnoughCornersMessage)
            }
            for index in 0..<Int(Self.CornerCount) { corners[index] = self.corners[index] }
        }

        public func Contains(_ box: BoundingBox) -> ContainmentType {
            var intersects = false
            for plane in planes {
                switch box.Intersects(plane) {
                case .Front: return .Disjoint
                case .Intersecting: intersects = true
                case .Back: break
                }
            }
            return intersects ? .Intersects : .Contains
        }

        public func Contains(_ box: inout BoundingBox, result: inout ContainmentType) {
            result = Contains(box)
        }

        public func Contains(_ frustum: BoundingFrustum) -> ContainmentType {
            if !Intersects(frustum) { return .Disjoint }
            for corner in frustum.corners where Contains(corner) == .Disjoint { return .Intersects }
            return .Contains
        }

        public func Contains(_ point: Vector3) -> ContainmentType {
            for plane in planes {
                let distance = (plane.Normal.X * point.X) + (plane.Normal.Y * point.Y) +
                    (plane.Normal.Z * point.Z) + plane.D
                if distance > 1e-5 { return .Disjoint }
            }
            return .Contains
        }

        public func Contains(_ point: inout Vector3, result: inout ContainmentType) {
            result = Contains(point)
        }

        public func Contains(_ sphere: BoundingSphere) -> ContainmentType {
            var insidePlaneCount = 0
            for plane in planes {
                let dot = (plane.Normal.X * sphere.Center.X) +
                    (plane.Normal.Y * sphere.Center.Y) + (plane.Normal.Z * sphere.Center.Z)
                let distance = dot + plane.D
                if distance > sphere.Radius { return .Disjoint }
                if distance < -sphere.Radius { insidePlaneCount += 1 }
            }
            return insidePlaneCount == 6 ? .Contains : .Intersects
        }

        public func Contains(_ sphere: inout BoundingSphere, result: inout ContainmentType) {
            result = Contains(sphere)
        }

        public func Intersects(_ box: BoundingBox) -> Bool {
            var closest = corners[0] - box.Min
            if closest.LengthSquared() < 1e-5 { closest = corners[0] - box.Max }
            return intersectsGjk(initialClosestPoint: closest) { box.supportMapping($0) }
        }

        public func Intersects(_ box: inout BoundingBox, result: inout Bool) { result = Intersects(box) }

        public func Intersects(_ frustum: BoundingFrustum) -> Bool {
            var closest = corners[0] - frustum.corners[0]
            if closest.LengthSquared() < 1e-5 { closest = corners[0] - frustum.corners[1] }
            return intersectsGjk(initialClosestPoint: closest) { frustum.supportMapping($0) }
        }

        public func Intersects(_ plane: Plane) -> PlaneIntersectionType {
            var sideMask = 0
            for corner in corners {
                let dot = Vector3.Dot(corner, vector2: plane.Normal)
                sideMask = dot + plane.D > 0 ? sideMask | 1 : sideMask | 2
                if sideMask == 3 { return .Intersecting }
            }
            return sideMask == 1 ? .Front : .Back
        }

        public func Intersects(_ plane: inout Plane, result: inout PlaneIntersectionType) {
            result = Intersects(plane)
        }

        public func Intersects(_ ray: Ray) -> Float? {
            if Contains(ray.Position) == .Contains { return 0 }
            var entry = -Float.greatestFiniteMagnitude
            var exit = Float.greatestFiniteMagnitude
            for plane in planes {
                let directionDot = Vector3.Dot(ray.Direction, vector2: plane.Normal)
                let positionDot = Vector3.Dot(ray.Position, vector2: plane.Normal) + plane.D
                if abs(directionDot) < 1e-5 {
                    if positionDot > 0 { return nil }
                    continue
                }
                let distance = -positionDot / directionDot
                if directionDot < 0 {
                    if distance > exit { return nil }
                    if distance > entry { entry = distance }
                } else {
                    if distance < entry { return nil }
                    if distance < exit { exit = distance }
                }
            }
            let result = entry >= 0 ? entry : exit
            return result >= 0 ? result : nil
        }

        public func Intersects(_ ray: inout Ray, result: inout Float?) { result = Intersects(ray) }

        public func Intersects(_ sphere: BoundingSphere) -> Bool {
            var closest = corners[0] - sphere.Center
            if closest.LengthSquared() < 1e-5 { closest = .UnitX }
            return intersectsGjk(initialClosestPoint: closest) { sphere.supportMapping($0) }
        }

        public func Intersects(_ sphere: inout BoundingSphere, result: inout Bool) {
            result = Intersects(sphere)
        }

        public func Equals(_ other: BoundingFrustum) -> Bool { matrixValue == other.matrixValue }
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? BoundingFrustum else { return false }
            return matrixValue == other.matrixValue
        }
        public func GetHashCode() -> Int32 { matrixValue.GetHashCode() }
        public func ToString() -> String {
            "{Near:\(Near.ToString()) Far:\(Far.ToString()) Left:\(Left.ToString()) " +
                "Right:\(Right.ToString()) Top:\(Top.ToString()) Bottom:\(Bottom.ToString())}"
        }

        public static func == (lhs: BoundingFrustum, rhs: BoundingFrustum) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: BoundingFrustum, rhs: BoundingFrustum) -> Bool { !lhs.Equals(rhs) }

        private func intersectsGjk(
            initialClosestPoint: Vector3,
            otherSupport: (Vector3) -> Vector3
        ) -> Bool {
            if gjk == nil { gjk = XnaGjk() }
            guard let gjk else { return false }
            gjk.Reset()
            var closestPoint = initialClosestPoint
            var previousDistanceSquared = Float.greatestFiniteMagnitude
            var threshold: Float = 0
            repeat {
                let direction = -closestPoint
                let frustumPoint = supportMapping(direction)
                let otherPoint = otherSupport(closestPoint)
                let supportPoint = frustumPoint - otherPoint
                let dot = (closestPoint.X * supportPoint.X) +
                    (closestPoint.Y * supportPoint.Y) + (closestPoint.Z * supportPoint.Z)
                if dot > 0 { return false }
                gjk.AddSupportPoint(supportPoint)
                closestPoint = gjk.ClosestPoint
                let oldDistanceSquared = previousDistanceSquared
                previousDistanceSquared = closestPoint.LengthSquared()
                if oldDistanceSquared - previousDistanceSquared <= 1e-5 * oldDistanceSquared {
                    return false
                }
                threshold = 4e-5 * gjk.MaxLengthSquared
            } while !gjk.FullSimplex && previousDistanceSquared >= threshold
            return true
        }

        private func supportMapping(_ direction: Vector3) -> Vector3 {
            var selectedIndex = 0
            var selectedDot = Vector3.Dot(corners[0], vector2: direction)
            for index in 1..<corners.count {
                let dot = Vector3.Dot(corners[index], vector2: direction)
                if dot > selectedDot {
                    selectedIndex = index
                    selectedDot = dot
                }
            }
            return corners[selectedIndex]
        }

        private func setMatrix(_ value: Microsoft.Xna.Framework.Matrix) {
            matrixValue = value

            planes[2].Normal.X = -value.M14 - value.M11
            planes[2].Normal.Y = -value.M24 - value.M21
            planes[2].Normal.Z = -value.M34 - value.M31
            planes[2].D = -value.M44 - value.M41
            planes[3].Normal.X = -value.M14 + value.M11
            planes[3].Normal.Y = -value.M24 + value.M21
            planes[3].Normal.Z = -value.M34 + value.M31
            planes[3].D = -value.M44 + value.M41
            planes[4].Normal.X = -value.M14 + value.M12
            planes[4].Normal.Y = -value.M24 + value.M22
            planes[4].Normal.Z = -value.M34 + value.M32
            planes[4].D = -value.M44 + value.M42
            planes[5].Normal.X = -value.M14 - value.M12
            planes[5].Normal.Y = -value.M24 - value.M22
            planes[5].Normal.Z = -value.M34 - value.M32
            planes[5].D = -value.M44 - value.M42
            planes[0].Normal.X = -value.M13
            planes[0].Normal.Y = -value.M23
            planes[0].Normal.Z = -value.M33
            planes[0].D = -value.M43
            planes[1].Normal.X = -value.M14 + value.M13
            planes[1].Normal.Y = -value.M24 + value.M23
            planes[1].Normal.Z = -value.M34 + value.M33
            planes[1].D = -value.M44 + value.M43

            for index in planes.indices {
                let length = planes[index].Normal.Length()
                planes[index].Normal = planes[index].Normal / length
                planes[index].D /= length
            }

            var ray = Self.computeIntersectionLine(planes[0], planes[2])
            corners[0] = Self.computeIntersection(planes[4], ray)
            corners[3] = Self.computeIntersection(planes[5], ray)
            ray = Self.computeIntersectionLine(planes[3], planes[0])
            corners[1] = Self.computeIntersection(planes[4], ray)
            corners[2] = Self.computeIntersection(planes[5], ray)
            ray = Self.computeIntersectionLine(planes[2], planes[1])
            corners[4] = Self.computeIntersection(planes[4], ray)
            corners[7] = Self.computeIntersection(planes[5], ray)
            ray = Self.computeIntersectionLine(planes[1], planes[3])
            corners[5] = Self.computeIntersection(planes[4], ray)
            corners[6] = Self.computeIntersection(planes[5], ray)
        }

        private static func computeIntersectionLine(_ plane1: Plane, _ plane2: Plane) -> Ray {
            let direction = Vector3.Cross(plane1.Normal, vector2: plane2.Normal)
            let position = Vector3.Cross(
                (-plane1.D * plane2.Normal) + (plane2.D * plane1.Normal),
                vector2: direction) / direction.LengthSquared()
            return Ray(position, direction)
        }

        private static func computeIntersection(_ plane: Plane, _ ray: Ray) -> Vector3 {
            let distance = (-plane.D - Vector3.Dot(plane.Normal, vector2: ray.Position)) /
                Vector3.Dot(plane.Normal, vector2: ray.Direction)
            return ray.Position + ray.Direction * distance
        }
    }
}
