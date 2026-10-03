// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testVector2CompleteBinary32Behavior() throws {
        let original = Framework.Vector2(3, 4)
        var copy = original; copy.X = 99
        XCTAssertEqual(original.X.bitPattern, Float(3).bitPattern)
        XCTAssertEqual(copy.X.bitPattern, Float(99).bitPattern)
        let reflected = Framework.Vector2.Reflect(Framework.Vector2(1, -1), normal: Framework.Vector2.UnitY)
        XCTAssertEqual(reflected.X.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(reflected.Y.bitPattern, Float(1).bitPattern)
        XCTAssertTrue(Framework.Vector2.Normalize(.Zero).X.isNaN)
        XCTAssertTrue(Framework.Vector2.Normalize(.Zero).Y.isNaN)
        XCTAssertEqual((Framework.Vector2(3) / 7).X.bitPattern, 0x3EDB_6DB8)
        XCTAssertEqual((-Framework.Vector2.Zero).X.bitPattern, 0x8000_0000)
        XCTAssertEqual(Framework.Vector2(1, 2).ToString(), "{X:1 Y:2}")
        XCTAssertEqual(Framework.Vector2(-0.0, 0).GetHashCode(), 0)

        var matrix = Framework.Matrix.CreateTranslation(5, yPosition: 7, zPosition: 0)
        XCTAssertTrue(Framework.Vector2.Transform(Framework.Vector2(2, 3), matrix: matrix) == Framework.Vector2(7, 10))
        XCTAssertTrue(Framework.Vector2.TransformNormal(Framework.Vector2(2, 3), matrix: matrix) == Framework.Vector2(2, 3))
        var result = Framework.Vector2.Zero
        var input = Framework.Vector2(2, 3)
        Framework.Vector2.Transform(&input, matrix: &matrix, result: &result)
        XCTAssertTrue(result == Framework.Vector2(7, 10))

        var destination = [Framework.Vector2.Zero, .Zero, .Zero]
        try Framework.Vector2.Transform([Framework.Vector2(1, 2), Framework.Vector2(3, 4)], matrix: &matrix, destinationArray: &destination)
        XCTAssertTrue(destination[0] == Framework.Vector2(6, 9))
        XCTAssertTrue(destination[1] == Framework.Vector2(8, 11))
        XCTAssertTrue(destination[2] == .Zero)
        var overlapping = [Framework.Vector2(1, 0), Framework.Vector2(2, 0), Framework.Vector2(3, 0)]
        try Framework.Vector2.Transform(overlapping, sourceIndex: 0, matrix: &matrix, destinationArray: &overlapping, destinationIndex: 1, length: 2)
        XCTAssertTrue(overlapping[1] == Framework.Vector2(6, 7))
        XCTAssertTrue(overlapping[2] == Framework.Vector2(7, 7))
        XCTAssertNoThrow(try Framework.Vector2.Transform([.Zero], sourceIndex: 0, matrix: &matrix, destinationArray: &destination, destinationIndex: 0, length: -1))
        XCTAssertThrowsError(try Framework.Vector2.Transform([.Zero], sourceIndex: -1, matrix: &matrix, destinationArray: &destination, destinationIndex: 0, length: 1))
        XCTAssertThrowsError(try Framework.Vector2.Transform([.Zero], sourceIndex: 0, matrix: &matrix, destinationArray: &destination, destinationIndex: 3, length: 1))
        XCTAssertThrowsError(try Framework.Vector2.Transform([.Zero], sourceIndex: 0, matrix: &matrix, destinationArray: &destination, destinationIndex: -1, length: 1))
        var empty: [Framework.Vector2] = []
        XCTAssertNoThrow(try Framework.Vector2.Transform([], matrix: &matrix, destinationArray: &empty))
    }

    func testVector3HandednessTransformsAndArrays() throws {
        var fresh = Framework.Vector3.Up; fresh.Y = 9
        XCTAssertTrue(Framework.Vector3.Up == Framework.Vector3(0, 1, 0))
        XCTAssertTrue(Framework.Vector3.Forward == Framework.Vector3(0, 0, -1))
        XCTAssertTrue(Framework.Vector3.Backward == Framework.Vector3(0, 0, 1))
        XCTAssertTrue(Framework.Vector3.Cross(.UnitX, vector2: .UnitY) == .UnitZ)
        XCTAssertTrue(Framework.Vector3.Cross(.UnitY, vector2: .UnitX) == -Framework.Vector3.UnitZ)
        XCTAssertTrue(Framework.Vector3.Normalize(.Zero).X.isNaN)
        XCTAssertEqual((Framework.Vector3(7) / 3).X.bitPattern, 0x4015_5556)
        let nan = Float.nan
        let minimum = Framework.Vector3.Min(Framework.Vector3(nan, 1, nan), value2: Framework.Vector3(7, nan, nan))
        XCTAssertEqual(minimum.X.bitPattern, 0x40E0_0000)
        XCTAssertTrue(minimum.Y.isNaN)
        XCTAssertTrue(minimum.Z.isNaN)
        XCTAssertTrue(Framework.Vector3.Clamp(.Zero, min: Framework.Vector3(2), max: Framework.Vector3(1)) == Framework.Vector3(2))
        XCTAssertEqual(Framework.Vector3(1, 2, 3).GetHashCode(), -1_077_936_128)

        let yaw = Framework.Quaternion.CreateFromAxisAngle(.Up, angle: 0.7)
        let pitch = Framework.Quaternion.CreateFromAxisAngle(.Right, angle: -0.4)
        let transformed = Framework.Vector3.Transform(Framework.Vector3(1.25, -2.5, 3.75), rotation: yaw * pitch)
        XCTAssertEqual(transformed.X.bitPattern, 0x4073_BBE2)
        XCTAssertEqual(transformed.Y.bitPattern, 0xBF57_A32E)
        XCTAssertEqual(transformed.Z.bitPattern, 0x4025_3083)

        var rotation = yaw
        var destination = [Framework.Vector3.Zero, .Zero]
        try Framework.Vector3.Transform([.UnitX, .UnitY], rotation: &rotation, destinationArray: &destination)
        XCTAssertFalse(destination[0] == .UnitX)
        XCTAssertTrue(destination[1].LengthSquared() > 0.99)
        var identity = Framework.Matrix.Identity
        try Framework.Vector3.TransformNormal(destination, matrix: &identity, destinationArray: &destination)
        XCTAssertTrue(destination[0].LengthSquared() > 0.99)
        XCTAssertTrue(Framework.Vector3.Normalize(Framework.Vector3(.infinity, 1, 1)).X.isNaN)
    }

    func testVector4ConstructorsTransformsAndInout() throws {
        let original = Framework.Vector4(1, 2, 3, 4)
        var copy = original; copy.W = 8
        XCTAssertEqual(original.W.bitPattern, Float(4).bitPattern)
        XCTAssertEqual(copy.W.bitPattern, Float(8).bitPattern)
        XCTAssertTrue(Framework.Vector4(Framework.Vector2(1, 2), 3, 4) == Framework.Vector4(1, 2, 3, 4))
        XCTAssertTrue(Framework.Vector4(Framework.Vector3(1, 2, 3), 4) == Framework.Vector4(1, 2, 3, 4))
        XCTAssertTrue(Framework.Vector4.Normalize(.Zero).W.isNaN)
        XCTAssertEqual((Framework.Vector4(12_345.67) / 3).X.bitPattern, 0x4580_99CA)
        XCTAssertEqual((-Framework.Vector4.Zero).X.bitPattern, 0x8000_0000)
        var value1 = Framework.Vector4(1, 2, 3, 4)
        var value2 = Framework.Vector4(5, 6, 7, 8)
        var result = Framework.Vector4.Zero
        Framework.Vector4.Add(&value1, value2: &value2, result: &result)
        XCTAssertTrue(result == Framework.Vector4(6, 8, 10, 12))
        var matrix = Framework.Matrix.CreateTranslation(5, yPosition: 6, zPosition: 7)
        let transformed = Framework.Vector4.Transform(Framework.Vector3(1, 2, 3), matrix: matrix)
        XCTAssertTrue(transformed == Framework.Vector4(6, 8, 10, 1))
        var destination = [Framework.Vector4.Zero]
        try Framework.Vector4.Transform([Framework.Vector4(1, 2, 3, 1)], matrix: &matrix, destinationArray: &destination)
        XCTAssertTrue(destination[0] == Framework.Vector4(6, 8, 10, 1))
    }

    func testQuaternionXnaDerivedBranchesAndOrdering() {
        let yaw = Framework.Quaternion.CreateFromAxisAngle(.Up, angle: 0.7)
        let pitch = Framework.Quaternion.CreateFromAxisAngle(.Right, angle: -0.4)
        let multiplied = yaw * pitch
        XCTAssertEqual(multiplied.X.bitPattern, 0xBE3F_1A81)
        XCTAssertEqual(multiplied.Y.bitPattern, 0x3EAC_1068)
        XCTAssertEqual(multiplied.Z.bitPattern, 0x3D8B_8437)
        XCTAssertEqual(multiplied.W.bitPattern, 0x3F6B_AF93)
        let concatenated = Framework.Quaternion.Concatenate(yaw, value2: pitch)
        XCTAssertEqual(concatenated.X.bitPattern, 0xBE3F_1A81)
        XCTAssertEqual(concatenated.Y.bitPattern, 0x3EAC_1068)
        XCTAssertEqual(concatenated.Z.bitPattern, 0xBD8B_8437)
        XCTAssertEqual(concatenated.W.bitPattern, 0x3F6B_AF93)
        let grouped = Framework.Quaternion(45_889.05859375, -42_412.4453125, 96_034.96875, -76_386.84375) *
            Framework.Quaternion(-16_375.435546875, 51_428.1875, -69_603.09375, -2_207.3798828125)
        XCTAssertEqual(grouped.X.bitPattern, 0xCE47_A05E)
        XCTAssertEqual(grouped.Y.bitPattern, 0xCF03_EDF7)
        XCTAssertEqual(grouped.Z.bitPattern, 0x4FC9_C4DD)
        XCTAssertEqual(grouped.W.bitPattern, 0x5011_D115)
        let slerp = Framework.Quaternion.Slerp(yaw, quaternion2: pitch, amount: 0.37)
        XCTAssertEqual(slerp.X.bitPattern, 0xBD9A_16EC)
        XCTAssertEqual(slerp.Y.bitPattern, 0x3E60_D7E7)
        XCTAssertEqual(slerp.Z.bitPattern, 0x0000_0000)
        XCTAssertEqual(slerp.W.bitPattern, 0x3F79_023D)
        let largeAxis = Framework.Quaternion.CreateFromAxisAngle(.Up, angle: 123_456.789)
        XCTAssertEqual(largeAxis.X.bitPattern, 0x0000_0000)
        XCTAssertEqual(largeAxis.Y.bitPattern, 0x3F30_464F)
        XCTAssertEqual(largeAxis.Z.bitPattern, 0x0000_0000)
        XCTAssertEqual(largeAxis.W.bitPattern, 0xBF39_A48F)
        let fromMatrix = Framework.Quaternion.CreateFromRotationMatrix(Framework.Matrix.CreateRotationY(0.7))
        XCTAssertEqual(fromMatrix.Y.bitPattern, 0x3EAF_904C)
        XCTAssertEqual(fromMatrix.W.bitPattern, 0x3F70_7ABB)
        XCTAssertTrue(Framework.Quaternion.Normalize(Framework.Quaternion(0, 0, 0, 0)).X.isNaN)
        XCTAssertTrue(Framework.Quaternion.Inverse(Framework.Quaternion(0, 0, 0, 0)).W.isNaN)
        XCTAssertEqual((-Framework.Quaternion(0, 0, 0, 0)).X.bitPattern, 0x8000_0000)
        var identityCopy = Framework.Quaternion.Identity; identityCopy.W = 2
        XCTAssertEqual(Framework.Quaternion.Identity.W.bitPattern, Float(1).bitPattern)
        let roundTripSource = Framework.Quaternion.CreateFromYawPitchRoll(0.3, pitch: -0.2, roll: 0.7)
        let roundTrip = Framework.Quaternion.CreateFromRotationMatrix(Framework.Matrix.CreateFromQuaternion(roundTripSource))
        XCTAssertGreaterThan(abs(Framework.Quaternion.Dot(roundTripSource, quaternion2: roundTrip)), 0.99999)
    }

    func testMatrixXnaDerivedArithmeticAndConventions() throws {
        let matrix = Framework.Matrix.CreateScale(2, yScale: 3, zScale: 4) *
            Framework.Matrix.CreateRotationY(0.25) *
            Framework.Matrix.CreateTranslation(5, yPosition: 6, zPosition: 7)
        let v2 = Framework.Vector2.Transform(Framework.Vector2(1.5, -2), matrix: matrix)
        XCTAssertEqual(v2.X.bitPattern, 0x40FD_03FE)
        XCTAssertEqual(v2.Y.bitPattern, 0x0000_0000)
        let v3 = Framework.Vector3.Transform(Framework.Vector3(1.5, -2, 0.25), matrix: matrix)
        XCTAssertEqual(v3.X.bitPattern, 0x4102_775D)
        XCTAssertEqual(v3.Y.bitPattern, 0x0000_0000)
        XCTAssertEqual(v3.Z.bitPattern, 0x40E7_4122)
        let product = matrix * Framework.Matrix.Invert(matrix)
        XCTAssertEqual(product.M11.bitPattern, 0x3F80_0000)
        XCTAssertEqual(product.M13.bitPattern, 0xB200_0000)
        XCTAssertEqual(product.M22.bitPattern, 0x3F80_0000)
        XCTAssertEqual(product.M31.bitPattern, 0x3300_0000)
        XCTAssertEqual(product.M33.bitPattern, 0x3F80_0000)
        XCTAssertEqual(product.M41.bitPattern, 0x3400_0000)
        XCTAssertEqual(product.M44.bitPattern, 0x3F80_0000)
        let singular = Framework.Matrix.Invert(Framework.Matrix(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0))
        XCTAssertTrue(singular.M11.isNaN)
        XCTAssertTrue(singular.M22.isNaN)
        XCTAssertTrue(singular.M33.isNaN)
        XCTAssertTrue(singular.M44.isNaN)
        XCTAssertEqual((Framework.Matrix.Identity / 3).M11.bitPattern, 0x3EAA_AAAB)
        XCTAssertEqual(Framework.Matrix.Identity.GetHashCode(), -33_554_432)
        XCTAssertEqual(Framework.Matrix.Identity.ToString(), "{ {M11:1 M12:0 M13:0 M14:0} {M21:0 M22:1 M23:0 M24:0} {M31:0 M32:0 M33:1 M34:0} {M41:0 M42:0 M43:0 M44:1} }")
        XCTAssertEqual((-Framework.Matrix(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)).M11.bitPattern, 0x8000_0000)

        var directions = Framework.Matrix.Identity
        directions.Translation = Framework.Vector3(4, 5, 6)
        directions.Forward = Framework.Vector3(0, 0, -2)
        XCTAssertTrue(directions.Translation == Framework.Vector3(4, 5, 6))
        XCTAssertTrue(directions.Forward == Framework.Vector3(0, 0, -2))
        XCTAssertEqual(directions.M33.bitPattern, Float(2).bitPattern)
        let asymmetric = Framework.Matrix.CreateScale(2, yScale: 3, zScale: 4) * Framework.Matrix.CreateTranslation(5, yPosition: 6, zPosition: 7)
        XCTAssertTrue(asymmetric.Translation == Framework.Vector3(5, 6, 7))
        XCTAssertFalse((Framework.Matrix.CreateTranslation(5, yPosition: 6, zPosition: 7) * Framework.Matrix.CreateScale(2, yScale: 3, zScale: 4)).Translation == Framework.Vector3(5, 6, 7))

        let largeRotation = Framework.Matrix.CreateRotationY(123_456.789)
        XCTAssertEqual(largeRotation.M11.bitPattern, 0x3D53_E807)
        XCTAssertEqual(largeRotation.M31.bitPattern, 0xBF7F_A83D)
        let infinite = try Framework.Matrix.CreatePerspective(4, height: 3, nearPlaneDistance: 0.1, farPlaneDistance: .infinity)
        XCTAssertTrue(infinite.M33.isNaN)
        XCTAssertTrue(infinite.M43.isNaN)
        XCTAssertThrowsError(try Framework.Matrix.CreatePerspectiveFieldOfView(0, aspectRatio: 1, nearPlaneDistance: 0.1, farPlaneDistance: 100))
        XCTAssertThrowsError(try Framework.Matrix.CreatePerspective(4, height: 3, nearPlaneDistance: 0, farPlaneDistance: 100))
        XCTAssertThrowsError(try Framework.Matrix.CreatePerspectiveOffCenter(-2, right: 2, bottom: -1.5, top: 1.5, nearPlaneDistance: 10, farPlaneDistance: 5))
        let zeroAspect = try Framework.Matrix.CreatePerspectiveFieldOfView(0.9, aspectRatio: 0, nearPlaneDistance: 0.1, farPlaneDistance: 100)
        XCTAssertEqual(zeroAspect.M11, .infinity)
        let nanAspect = try Framework.Matrix.CreatePerspectiveFieldOfView(0.9, aspectRatio: .nan, nearPlaneDistance: 0.1, farPlaneDistance: 100)
        XCTAssertTrue(nanAspect.M11.isNaN)

        let mirrored = Framework.Matrix.CreateScale(-2, yScale: 3, zScale: 4) * Framework.Matrix.CreateRotationY(0.25) * Framework.Matrix.CreateTranslation(5, yPosition: 6, zPosition: 7)
        var scale = Framework.Vector3.Zero; var rotation = Framework.Quaternion.Identity; var translation = Framework.Vector3.Zero
        XCTAssertTrue(mirrored.Decompose(&scale, rotation: &rotation, translation: &translation))
        XCTAssertEqual(scale.X.bitPattern, 0x4000_0000)
        XCTAssertEqual(scale.Y.bitPattern, 0x4040_0000)
        XCTAssertEqual(scale.Z.bitPattern, 0xC080_0000)
        XCTAssertEqual(rotation.Y.bitPattern, 0x3F7E_00AA)
        XCTAssertEqual(rotation.W.bitPattern, 0xBDFF_5579)
        XCTAssertTrue(translation == Framework.Vector3(5, 6, 7))

        let billboard = Framework.Matrix.CreateConstrainedBillboard(Framework.Vector3(0, 10, 0), cameraPosition: .Zero, rotateAxis: Framework.Vector3(0, 2, 0), cameraForwardVector: nil, objectForwardVector: nil)
        XCTAssertEqual(billboard.M11.bitPattern, 0xBF80_0000)
        XCTAssertEqual(billboard.M22.bitPattern, 0x4000_0000)
        XCTAssertEqual(billboard.M33.bitPattern, 0xBF80_0000)
        let shadow = Framework.Matrix.CreateShadow(.Forward, plane: Framework.Plane(.Zero, 0))
        XCTAssertTrue(shadow.M11.isNaN)
        XCTAssertTrue(shadow.M44.isNaN)
        var plane = Framework.Plane(Framework.Vector3(2, 0, 0), 4)
        var reflection = Framework.Matrix.Identity
        Framework.Matrix.CreateReflection(&plane, result: &reflection)
        XCTAssertEqual(plane.Normal.X.bitPattern, 0x3F80_0000)
        XCTAssertEqual(plane.D.bitPattern, 0x4000_0000)
        XCTAssertEqual(reflection.M11.bitPattern, 0xBF80_0000)
        XCTAssertEqual(reflection.M41.bitPattern, 0xC080_0000)

        let degenerate = Framework.Matrix.CreateLookAt(.Zero, cameraTarget: .Zero, cameraUpVector: .Up)
        XCTAssertTrue(degenerate.M11.isNaN)
        XCTAssertTrue(degenerate.M22.isNaN)
        XCTAssertTrue(degenerate.M33.isNaN)
        XCTAssertTrue(degenerate.M41.isNaN)
        var infinityMatrix = Framework.Matrix.Identity; infinityMatrix.M14 = .infinity
        let infinityTransform = Framework.Matrix.Transform(infinityMatrix, rotation: .Identity)
        XCTAssertEqual(infinityTransform.M11.bitPattern, 0x3F80_0000)
        XCTAssertEqual(infinityTransform.M14, .infinity)
        var identityCopy = Framework.Matrix.Identity; identityCopy.M11 = 9
        XCTAssertEqual(Framework.Matrix.Identity.M11.bitPattern, Float(1).bitPattern)
    }

    func testViewportXnaDerivedProjectUnproject() throws {
        var viewport = Framework.Graphics.Viewport(11, 13, 640, 360)
        viewport.MinDepth = 0.2; viewport.MaxDepth = 0.9
        let world = Framework.Matrix.CreateScale(1.5, yScale: 0.75, zScale: 2) *
            Framework.Matrix.CreateRotationY(0.31) * Framework.Matrix.CreateTranslation(2, yPosition: -1, zPosition: 0.5)
        let view = Framework.Matrix.CreateLookAt(Framework.Vector3(4, 3, 8), cameraTarget: .Zero, cameraUpVector: .Up)
        let projection = try Framework.Matrix.CreatePerspectiveFieldOfView(0.9, aspectRatio: 16 / 9, nearPlaneDistance: 0.1, farPlaneDistance: 100)
        let projected = viewport.Project(Framework.Vector3(0.25, -0.5, 1.25), projection: projection, view: view, world: world)
        XCTAssertEqual(projected.X.bitPattern, 0x43D4_2808)
        XCTAssertEqual(projected.Y.bitPattern, 0x43AC_9F3C)
        XCTAssertEqual(projected.Z.bitPattern, 0x3F63_AFF4)
        let unprojected = viewport.Unproject(projected, projection: projection, view: view, world: world)
        XCTAssertEqual(unprojected.X.bitPattern, 0x3E7F_FE10)
        XCTAssertEqual(unprojected.Y.bitPattern, 0xBEFF_F906)
        XCTAssertEqual(unprojected.Z.bitPattern, 0x3FA0_0111)
        let singular = viewport.Unproject(Framework.Vector3(100, 50, 0.5), projection: .Identity, view: .Identity, world: Framework.Matrix(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0))
        XCTAssertTrue(singular.X.isNaN)
        XCTAssertTrue(singular.Y.isNaN)
        XCTAssertTrue(singular.Z.isNaN)
        XCTAssertEqual(viewport.ToString(), "{X:11 Y:13 Width:640 Height:360 MinDepth:0.2 MaxDepth:0.9}")
        XCTAssertEqual(Framework.Graphics.Viewport(0, 0, 0, 10).AspectRatio.bitPattern, Float(0).bitPattern)
        let nearDepth = viewport.Unproject(Framework.Vector3(projected.X, projected.Y, viewport.MinDepth), projection: projection, view: view, world: world)
        let farDepth = viewport.Unproject(Framework.Vector3(projected.X, projected.Y, viewport.MaxDepth), projection: projection, view: view, world: world)
        XCTAssertFalse(nearDepth == farDepth)
    }
}
