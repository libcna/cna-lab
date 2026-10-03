// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

private func XCTAssertVector3ArrayEqual(
    _ lhs: [Microsoft.Xna.Framework.Vector3],
    _ rhs: [Microsoft.Xna.Framework.Vector3],
    file: StaticString = #filePath,
    line: UInt = #line
) {
    XCTAssertEqual(lhs.count, rhs.count, file: file, line: line)
    for (left, right) in zip(lhs, rhs) {
        XCTAssertTrue(left == right, file: file, line: line)
    }
}

extension PureValueTests {
    func testGeometryEnumsXnaRawValues() {
        XCTAssertEqual(Framework.PlaneIntersectionType.Front.rawValue, 0)
        XCTAssertEqual(Framework.PlaneIntersectionType.Back.rawValue, 1)
        XCTAssertEqual(Framework.PlaneIntersectionType.Intersecting.rawValue, 2)
        XCTAssertEqual(Framework.ContainmentType.Disjoint.rawValue, 0)
        XCTAssertEqual(Framework.ContainmentType.Contains.rawValue, 1)
        XCTAssertEqual(Framework.ContainmentType.Intersects.rawValue, 2)
    }

    func testPlaneXnaDerivedTransformsDotsAndClassification() throws {
        let asymmetric = Framework.Plane(Framework.Vector3(2, -3, 4), -5)
        let value4 = Framework.Vector4(7, 11, -13, 17)
        XCTAssertEqual(asymmetric.Dot(value4).bitPattern, Float(-156).bitPattern)
        XCTAssertEqual(asymmetric.DotCoordinate(Framework.Vector3(7, 11, -13)).bitPattern, Float(-76).bitPattern)
        XCTAssertEqual(asymmetric.DotNormal(Framework.Vector3(7, 11, -13)).bitPattern, Float(-71).bitPattern)
        var inoutValue4 = value4
        var dot: Float = 0
        asymmetric.Dot(&inoutValue4, result: &dot)
        XCTAssertEqual(dot.bitPattern, Float(-156).bitPattern)
        var inoutValue3 = Framework.Vector3(7, 11, -13)
        asymmetric.DotCoordinate(&inoutValue3, result: &dot)
        XCTAssertEqual(dot.bitPattern, Float(-76).bitPattern)
        asymmetric.DotNormal(&inoutValue3, result: &dot)
        XCTAssertEqual(dot.bitPattern, Float(-71).bitPattern)

        let translated = Framework.Plane.Transform(
            Framework.Plane(.Up, -2),
            matrix: Framework.Matrix.CreateTranslation(0, yPosition: 5, zPosition: 0))
        XCTAssertEqual(translated.Normal.X.bitPattern, 0x0000_0000)
        XCTAssertEqual(translated.Normal.Y.bitPattern, 0x3F80_0000)
        XCTAssertEqual(translated.Normal.Z.bitPattern, 0x0000_0000)
        XCTAssertEqual(translated.D.bitPattern, 0xC0E0_0000)

        let nonuniform = Framework.Matrix.CreateScale(2, yScale: 3, zScale: 4) *
            Framework.Matrix.CreateTranslation(5, yPosition: -7, zPosition: 11)
        var plane = Framework.Plane(1, 2, -3, 4)
        var matrix = nonuniform
        var transformed = Framework.Plane(.Zero, 0)
        Framework.Plane.Transform(&plane, matrix: &matrix, result: &transformed)
        XCTAssertEqual(transformed.Normal.X.bitPattern, Float(0.5).bitPattern)
        XCTAssertEqual(transformed.Normal.Y.bitPattern, Float(2.0 / 3.0).bitPattern)
        XCTAssertEqual(transformed.Normal.Z.bitPattern, Float(-0.75).bitPattern)
        XCTAssertEqual(transformed.D.bitPattern, 0x4166_AAAB)

        var rotation = Framework.Quaternion.CreateFromAxisAngle(.Forward, angle: Framework.MathHelper.PiOver2)
        var rotated = Framework.Plane(.Zero, 0)
        Framework.Plane.Transform(&plane, rotation: &rotation, result: &rotated)
        XCTAssertEqual(rotated.D.bitPattern, plane.D.bitPattern)
        XCTAssertEqual(rotated.Normal.LengthSquared().bitPattern, plane.Normal.LengthSquared().bitPattern)

        let degenerate = Framework.Plane(.Zero, .Zero, .Zero)
        XCTAssertTrue(degenerate.Normal.X.isNaN)
        XCTAssertTrue(degenerate.Normal.Y.isNaN)
        XCTAssertTrue(degenerate.Normal.Z.isNaN)
        XCTAssertTrue(degenerate.D.isNaN)
        let nearUnit = Framework.Plane.Normalize(Framework.Plane(Framework.Vector3(0.6, 0.79999995, 0), 2))
        XCTAssertEqual(nearUnit.Normal.X.bitPattern, 0x3F19_999A)
        XCTAssertEqual(nearUnit.Normal.Y.bitPattern, 0x3F4C_CCCC)
        XCTAssertEqual(nearUnit.D.bitPattern, 0x4000_0000)
        XCTAssertTrue(Framework.Plane.Normalize(Framework.Plane(.Zero, 1)).Normal.X.isNaN)
        XCTAssertTrue(Framework.Plane.Transform(Framework.Plane(.Up, 0), matrix: Framework.Matrix(
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)).D.isNaN)

        let box = Framework.BoundingBox(Framework.Vector3(-1), Framework.Vector3(1))
        XCTAssertEqual(Framework.Plane(.Zero, 0).Intersects(box), .Intersecting)
        XCTAssertEqual(Framework.Plane(.UnitX, -2).Intersects(box), .Back)
        XCTAssertEqual(Framework.Plane(.UnitX, 2).Intersects(box), .Front)
        let sphere = try Framework.BoundingSphere(.Zero, 1)
        XCTAssertEqual(Framework.Plane(.UnitX, -2).Intersects(sphere), .Back)
        XCTAssertEqual(Framework.Plane(.UnitX, 0).Intersects(sphere), .Intersecting)
        XCTAssertTrue(asymmetric.Equals(Framework.Plane(Framework.Vector3(2, -3, 4), -5)))
        XCTAssertEqual(asymmetric.GetHashCode(), asymmetric.Normal.GetHashCode() &+ xnaFloatHash(-5))
        XCTAssertEqual(asymmetric.ToString(), "{Normal:{X:2 Y:-3 Z:4} D:-5}")
    }

    func testRayXnaDerivedNullableIntersections() throws {
        let sphere = try Framework.BoundingSphere(.Zero, 1)
        let sphereHit = Framework.Ray(Framework.Vector3(-5, 0.25, 0), .UnitX).Intersects(sphere)
        XCTAssertEqual(sphereHit?.bitPattern, UInt32(0x4081_0421))
        XCTAssertEqual(Framework.Ray(.Zero, .UnitX).Intersects(sphere)?.bitPattern, Float(0).bitPattern)
        XCTAssertNil(Framework.Ray(Framework.Vector3(-5, 0, 0), .Left).Intersects(sphere))
        XCTAssertEqual(Framework.Ray(Framework.Vector3(-5, 1, 0), .UnitX).Intersects(sphere)?.bitPattern,
                       Float(5).bitPattern)

        let box = Framework.BoundingBox(Framework.Vector3(-1), Framework.Vector3(1))
        XCTAssertEqual(Framework.Ray(Framework.Vector3(-5, 0, 0), .UnitX).Intersects(box), 4)
        XCTAssertEqual(Framework.Ray(.Zero, .UnitX).Intersects(box), 0)
        XCTAssertNil(Framework.Ray(Framework.Vector3(2, 0, 0), Framework.Vector3(-5e-7, 0, 0)).Intersects(box))
        XCTAssertNil(Framework.Ray(Framework.Vector3(2, 0, 0), .Zero).Intersects(box))
        XCTAssertEqual(Framework.Ray(.Zero, .Zero).Intersects(box), 0)

        let nearParallel = Framework.Ray(.Zero, Framework.Vector3(5e-6, 1, 0))
        XCTAssertNil(nearParallel.Intersects(Framework.Plane(.UnitX, -1)))
        let justBehind = Framework.Ray(Framework.Vector3(5e-6, 0, 0), .UnitX)
        var originPlane = Framework.Plane(.UnitX, 0)
        var outDistance: Float? = nil
        justBehind.Intersects(&originPlane, result: &outDistance)
        XCTAssertEqual(justBehind.Intersects(originPlane)?.bitPattern, Float(0).bitPattern)
        XCTAssertEqual(outDistance?.bitPattern, Float(0).bitPattern)
        XCTAssertNil(Framework.Ray(Framework.Vector3(1, 0, 0), .UnitX).Intersects(originPlane))

        var inoutBox = box
        outDistance = nil
        Framework.Ray(Framework.Vector3(-5, 0, 0), .UnitX).Intersects(&inoutBox, result: &outDistance)
        XCTAssertEqual(outDistance, 4)
        XCTAssertEqual(Framework.Ray(.Zero, .UnitX).ToString(),
                       "{Position:{X:0 Y:0 Z:0} Direction:{X:1 Y:0 Z:0}}")
        let signedZeroRay = Framework.Ray(Framework.Vector3(-0.0, 0, 0), .UnitX)
        XCTAssertTrue(signedZeroRay == Framework.Ray(.Zero, .UnitX))
        XCTAssertEqual(signedZeroRay.GetHashCode(), Framework.Ray(.Zero, .UnitX).GetHashCode())
        XCTAssertFalse(Framework.Ray(Framework.Vector3(.nan, 0, 0), .UnitX).Equals(
            Framework.Ray(Framework.Vector3(.nan, 0, 0), .UnitX)))
    }

    func testBoundingBoxXnaCornersFactoriesContainmentAndInout() throws {
        let box = Framework.BoundingBox(Framework.Vector3(-2, -3, -5), Framework.Vector3(7, 11, 13))
        let expected = [
            Framework.Vector3(-2, 11, 13), Framework.Vector3(7, 11, 13),
            Framework.Vector3(7, -3, 13), Framework.Vector3(-2, -3, 13),
            Framework.Vector3(-2, 11, -5), Framework.Vector3(7, 11, -5),
            Framework.Vector3(7, -3, -5), Framework.Vector3(-2, -3, -5),
        ]
        XCTAssertEqual(Framework.BoundingBox.CornerCount, 8)
        XCTAssertVector3ArrayEqual(box.GetCorners(), expected)
        var destination = Array(repeating: Framework.Vector3(99), count: 10)
        try box.GetCorners(&destination)
        XCTAssertVector3ArrayEqual(Array(destination.prefix(8)), expected)
        XCTAssertTrue(destination[8] == Framework.Vector3(99))
        var short = Array(repeating: Framework.Vector3.Zero, count: 7)
        XCTAssertThrowsError(try box.GetCorners(&short))

        let pointsBox = try Framework.BoundingBox.CreateFromPoints([
            Framework.Vector3(-2, 1, 4), Framework.Vector3(3, -5, 2), Framework.Vector3(-2, 1, 4),
        ])
        XCTAssertTrue(pointsBox.Min == Framework.Vector3(-2, -5, 2))
        XCTAssertTrue(pointsBox.Max == Framework.Vector3(3, 1, 4))
        XCTAssertThrowsError(try Framework.BoundingBox.CreateFromPoints([]))
        let merged = Framework.BoundingBox.CreateMerged(
            Framework.BoundingBox(Framework.Vector3(-4), Framework.Vector3(-2)),
            additional: Framework.BoundingBox(Framework.Vector3(1), Framework.Vector3(6)))
        XCTAssertTrue(merged.Min == Framework.Vector3(-4))
        XCTAssertTrue(merged.Max == Framework.Vector3(6))

        let unit = Framework.BoundingBox(Framework.Vector3(-1), Framework.Vector3(1))
        XCTAssertEqual(unit.Contains(Framework.Vector3(1, 0, 0)), .Contains)
        XCTAssertEqual(unit.Contains(Framework.Vector3(2, 0, 0)), .Disjoint)
        XCTAssertEqual(unit.Contains(Framework.BoundingBox(Framework.Vector3(-0.5), Framework.Vector3(0.5))), .Contains)
        XCTAssertEqual(unit.Contains(Framework.BoundingBox(Framework.Vector3(0.5), Framework.Vector3(2))), .Intersects)
        XCTAssertEqual(unit.Contains(Framework.BoundingBox(Framework.Vector3(2), Framework.Vector3(3))), .Disjoint)
        let edgeSphere = try Framework.BoundingSphere(Framework.Vector3(1.5, 0, 0), 0.75)
        XCTAssertTrue(unit.Intersects(edgeSphere))
        XCTAssertEqual(unit.Contains(edgeSphere), .Intersects)
        let nanBox = Framework.BoundingBox(Framework.Vector3(.nan, -1, -1), Framework.Vector3(.nan, 1, 1))
        XCTAssertEqual(unit.Contains(Framework.Vector3(.nan, 0, 0)), .Disjoint)
        XCTAssertTrue(unit.Intersects(nanBox))

        var ray = Framework.Ray(Framework.Vector3(-5, 0, 0), .UnitX)
        var distance: Float? = nil
        unit.Intersects(&ray, result: &distance)
        XCTAssertEqual(distance, 4)
        var sphere = try Framework.BoundingSphere(.Zero, 0.5)
        var containment = Framework.ContainmentType.Disjoint
        unit.Contains(&sphere, result: &containment)
        XCTAssertEqual(containment, .Contains)
        XCTAssertEqual(box.ToString(), "{Min:{X:-2 Y:-3 Z:-5} Max:{X:7 Y:11 Z:13}}")
        XCTAssertTrue(Framework.BoundingBox(Framework.Vector3(-0.0), .Zero) ==
            Framework.BoundingBox(.Zero, .Zero))
        XCTAssertEqual(Framework.BoundingBox(Framework.Vector3(-0.0), .Zero).GetHashCode(), 0)
        XCTAssertFalse(Framework.BoundingBox(Framework.Vector3(.nan), .Zero).Equals(
            Framework.BoundingBox(Framework.Vector3(.nan), .Zero)))
    }

    func testBoundingSphereXnaFactoriesMergeTransformAndContainment() throws {
        XCTAssertThrowsError(try Framework.BoundingSphere(.Zero, -1))
        let sphere = try Framework.BoundingSphere(.Zero, 1)
        XCTAssertEqual(sphere.Contains(.UnitX), .Disjoint)
        XCTAssertFalse(sphere.Intersects(try Framework.BoundingSphere(Framework.Vector3(2, 0, 0), 1)))

        let points = [
            Framework.Vector3(-4, 1, 0), Framework.Vector3(6, -2, 3),
            Framework.Vector3(0, 8, -5), Framework.Vector3(2, 0, 9),
        ]
        let pointsSphere = try Framework.BoundingSphere.CreateFromPoints(points)
        XCTAssertEqual(pointsSphere.Center.X.bitPattern, 0x3F80_0000)
        XCTAssertEqual(pointsSphere.Center.Y.bitPattern, 0x4080_0000)
        XCTAssertEqual(pointsSphere.Center.Z.bitPattern, 0x4000_0000)
        XCTAssertEqual(pointsSphere.Radius.bitPattern, 0x4101_FC10)
        for point in points {
            XCTAssertLessThanOrEqual(Framework.Vector3.Distance(pointsSphere.Center, value2: point),
                                     pointsSphere.Radius + 1e-5)
        }
        XCTAssertThrowsError(try Framework.BoundingSphere.CreateFromPoints([]))
        let single = try Framework.BoundingSphere.CreateFromPoints([Framework.Vector3(3, 4, 5)])
        XCTAssertTrue(single.Center == Framework.Vector3(3, 4, 5))
        XCTAssertEqual(single.Radius.bitPattern, Float(0).bitPattern)

        let original = try Framework.BoundingSphere(.Zero, 2)
        let inside = try Framework.BoundingSphere(Framework.Vector3(0.5, 0, 0), 0.5)
        XCTAssertTrue(Framework.BoundingSphere.CreateMerged(original, additional: inside) == original)
        let separate = try Framework.BoundingSphere(Framework.Vector3(6, 0, 0), 1)
        let merged = Framework.BoundingSphere.CreateMerged(original, additional: separate)
        XCTAssertEqual(merged.Center.X.bitPattern, Float(2.5).bitPattern)
        XCTAssertEqual(merged.Radius.bitPattern, Float(4.5).bitPattern)

        let transformed = (try Framework.BoundingSphere(Framework.Vector3(1, 2, 3), 2)).Transform(
            Framework.Matrix.CreateScale(-1, yScale: 3, zScale: 2) *
                Framework.Matrix.CreateTranslation(10, yPosition: 0, zPosition: -4))
        XCTAssertTrue(transformed.Center == Framework.Vector3(9, 6, 2))
        XCTAssertEqual(transformed.Radius.bitPattern, Float(6).bitPattern)
        var matrix = Framework.Matrix.CreateScale(1, yScale: 3, zScale: 2)
        var transformedOut = try Framework.BoundingSphere(.Zero, 0)
        sphere.Transform(&matrix, result: &transformedOut)
        XCTAssertEqual(transformedOut.Radius.bitPattern, Float(3).bitPattern)

        let box = Framework.BoundingBox(Framework.Vector3(-1), Framework.Vector3(1))
        let fromBox = Framework.BoundingSphere.CreateFromBoundingBox(box)
        XCTAssertTrue(fromBox.Center == .Zero)
        XCTAssertEqual(fromBox.Radius.bitPattern, (xnaSqrt(12) * 0.5).bitPattern)
        XCTAssertEqual(fromBox.Contains(box), .Contains)
        XCTAssertEqual(sphere.ToString(), "{Center:{X:0 Y:0 Z:0} Radius:1}")
        XCTAssertTrue(try Framework.BoundingSphere(Framework.Vector3(-0.0, 0, 0), 1) == sphere)
        XCTAssertEqual(try Framework.BoundingSphere(Framework.Vector3(-0.0, 0, 0), 1).GetHashCode(),
                       sphere.GetHashCode())
        XCTAssertFalse(try Framework.BoundingSphere(Framework.Vector3(.nan, 0, 0), 1).Equals(
            Framework.BoundingSphere(Framework.Vector3(.nan, 0, 0), 1)))
    }

    func testBoundingFrustumXnaPlanesCornersMutationAndRelations() throws {
        let projection = try Framework.Matrix.CreatePerspectiveFieldOfView(
            Framework.MathHelper.PiOver4,
            aspectRatio: 4.0 / 3.0,
            nearPlaneDistance: 1,
            farPlaneDistance: 10)
        let view = Framework.Matrix.CreateLookAt(Framework.Vector3(0, 0, 5), cameraTarget: .Zero, cameraUpVector: .Up)
        let frustum = Framework.BoundingFrustum(value: view * projection)
        XCTAssertEqual(Framework.BoundingFrustum.CornerCount, 8)
        XCTAssertEqual(frustum.Near.Normal.X.bitPattern, 0x8000_0000)
        XCTAssertEqual(frustum.Near.Normal.Y.bitPattern, 0x8000_0000)
        XCTAssertEqual(frustum.Near.Normal.Z.bitPattern, 0x3F80_0000)
        XCTAssertEqual(frustum.Near.D.bitPattern, 0xC080_0000)
        XCTAssertEqual(frustum.Top.Normal.X.bitPattern, 0x0000_0000)
        XCTAssertEqual(frustum.Top.Normal.Y.bitPattern, 0x3F6C_835F)
        XCTAssertEqual(frustum.Top.Normal.Z.bitPattern, 0x3EC3_EF16)
        XCTAssertEqual(frustum.Top.D.bitPattern, 0xBFF4_EADB)

        let corners = frustum.GetCorners()
        XCTAssertEqual(corners.count, 8)
        XCTAssertEqual(corners[0].X.bitPattern, 0xBF0D_6289)
        XCTAssertEqual(corners[0].Y.bitPattern, 0x3ED4_13CB)
        XCTAssertEqual(corners[0].Z.bitPattern, 0x4080_0000)
        XCTAssertLessThan(corners[0].X, 0); XCTAssertGreaterThan(corners[0].Y, 0)
        XCTAssertGreaterThan(corners[1].X, 0); XCTAssertGreaterThan(corners[1].Y, 0)
        XCTAssertGreaterThan(corners[2].X, 0); XCTAssertLessThan(corners[2].Y, 0)
        XCTAssertLessThan(corners[3].X, 0); XCTAssertLessThan(corners[3].Y, 0)
        XCTAssertLessThan(corners[4].X, 0); XCTAssertGreaterThan(corners[4].Y, 0)
        XCTAssertGreaterThan(corners[5].X, 0); XCTAssertGreaterThan(corners[5].Y, 0)
        XCTAssertEqual(corners[6].X.bitPattern, 0x40B0_BB28)
        XCTAssertEqual(corners[6].Y.bitPattern, 0xC084_8C5D)
        XCTAssertEqual(corners[6].Z.bitPattern, 0xC09F_FFF8)
        XCTAssertLessThan(corners[7].X, 0); XCTAssertLessThan(corners[7].Y, 0)
        for index in 0..<4 { XCTAssertEqual(corners[index].Z.bitPattern, Float(4).bitPattern) }
        for index in 4..<8 { XCTAssertLessThan(abs(corners[index].Z + 5), 1e-5) }

        var destination = Array(repeating: Framework.Vector3(99), count: 10)
        try frustum.GetCorners(&destination)
        XCTAssertVector3ArrayEqual(Array(destination.prefix(8)), corners)
        XCTAssertTrue(destination[8] == Framework.Vector3(99))
        var short = Array(repeating: Framework.Vector3.Zero, count: 7)
        XCTAssertThrowsError(try frustum.GetCorners(&short))

        XCTAssertEqual(frustum.Contains(.Zero), .Contains)
        XCTAssertEqual(frustum.Contains(Framework.Vector3(0, 0, 6)), .Disjoint)
        let centerBox = Framework.BoundingBox(Framework.Vector3(-0.5), Framework.Vector3(0.5))
        XCTAssertEqual(frustum.Contains(centerBox), .Contains)
        let centerSphere = try Framework.BoundingSphere(.Zero, 0.5)
        XCTAssertEqual(frustum.Contains(centerSphere), .Contains)
        XCTAssertTrue(frustum.Intersects(centerBox))
        XCTAssertFalse(frustum.Intersects(Framework.BoundingBox(Framework.Vector3(100), Framework.Vector3(101))))
        XCTAssertTrue(frustum.Intersects(centerSphere))
        XCTAssertFalse(frustum.Intersects(try Framework.BoundingSphere(Framework.Vector3(100), 0.5)))
        XCTAssertEqual(frustum.Intersects(Framework.Ray(Framework.Vector3(0, 0, 20), .Forward))?.bitPattern,
                       UInt32(0x4180_0000))

        let distantView = Framework.Matrix.CreateLookAt(
            Framework.Vector3(100, 0, 5), cameraTarget: Framework.Vector3(100, 0, 0), cameraUpVector: .Up)
        let distant = Framework.BoundingFrustum(value: distantView * projection)
        XCTAssertFalse(frustum.Intersects(distant))
        XCTAssertEqual(frustum.Contains(distant), .Disjoint)
        XCTAssertEqual(frustum.Intersects(frustum.Near), .Back)
        XCTAssertTrue(frustum.Equals(Framework.BoundingFrustum(value: frustum.Matrix)))
        XCTAssertEqual(frustum.GetHashCode(), frustum.Matrix.GetHashCode())
        XCTAssertTrue(frustum.ToString().hasPrefix("{Near:{Normal:"))
        XCTAssertTrue(frustum.ToString().contains(" Far:"))
        XCTAssertTrue(frustum.ToString().contains(" Bottom:"))

        let oldCorners = frustum.GetCorners()
        let alias = frustum
        alias.Matrix = distantView * projection
        XCTAssertTrue(zip(alias.GetCorners(), oldCorners).contains { $0 != $1 })
        XCTAssertVector3ArrayEqual(frustum.GetCorners(), alias.GetCorners())
        XCTAssertEqual(frustum.Contains(Framework.Vector3(100, 0, 0)), .Contains)
        var detachedMatrix = frustum.Matrix
        detachedMatrix.M11 = 999
        XCTAssertNotEqual(frustum.Matrix.M11, 999)
    }
}
