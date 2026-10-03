// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Matrix {
    public static func CreateTranslation(_ position: Microsoft.Xna.Framework.Vector3) -> Self {
        CreateTranslation(position.X, yPosition: position.Y, zPosition: position.Z)
    }
    public static func CreateTranslation(_ position: inout Microsoft.Xna.Framework.Vector3, result: inout Self) {
        result = CreateTranslation(position)
    }
    public static func CreateTranslation(_ xPosition: Float, yPosition: Float, zPosition: Float) -> Self {
        Self(1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             xPosition, yPosition, zPosition, 1)
    }
    public static func CreateTranslation(_ xPosition: Float, yPosition: Float, zPosition: Float, result: inout Self) {
        result = CreateTranslation(xPosition, yPosition: yPosition, zPosition: zPosition)
    }

    public static func CreateScale(_ xScale: Float, yScale: Float, zScale: Float) -> Self {
        Self(xScale, 0, 0, 0,
             0, yScale, 0, 0,
             0, 0, zScale, 0,
             0, 0, 0, 1)
    }
    public static func CreateScale(_ xScale: Float, yScale: Float, zScale: Float, result: inout Self) {
        result = CreateScale(xScale, yScale: yScale, zScale: zScale)
    }
    public static func CreateScale(_ scales: Microsoft.Xna.Framework.Vector3) -> Self {
        CreateScale(scales.X, yScale: scales.Y, zScale: scales.Z)
    }
    public static func CreateScale(_ scales: inout Microsoft.Xna.Framework.Vector3, result: inout Self) { result = CreateScale(scales) }
    public static func CreateScale(_ scale: Float) -> Self { CreateScale(scale, yScale: scale, zScale: scale) }
    public static func CreateScale(_ scale: Float, result: inout Self) { result = CreateScale(scale) }

    public static func CreateRotationX(_ radians: Float) -> Self {
        let cos = xnaCos(radians); let sin = xnaSin(radians)
        return Self(1, 0, 0, 0,
                    0, cos, sin, 0,
                    0, -sin, cos, 0,
                    0, 0, 0, 1)
    }
    public static func CreateRotationX(_ radians: Float, result: inout Self) { result = CreateRotationX(radians) }
    public static func CreateRotationY(_ radians: Float) -> Self {
        let cos = xnaCos(radians); let sin = xnaSin(radians)
        return Self(cos, 0, -sin, 0,
                    0, 1, 0, 0,
                    sin, 0, cos, 0,
                    0, 0, 0, 1)
    }
    public static func CreateRotationY(_ radians: Float, result: inout Self) { result = CreateRotationY(radians) }
    public static func CreateRotationZ(_ radians: Float) -> Self {
        let cos = xnaCos(radians); let sin = xnaSin(radians)
        return Self(cos, sin, 0, 0,
                    -sin, cos, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1)
    }
    public static func CreateRotationZ(_ radians: Float, result: inout Self) { result = CreateRotationZ(radians) }

    public static func CreateFromAxisAngle(_ axis: Microsoft.Xna.Framework.Vector3, angle: Float) -> Self {
        let x = axis.X; let y = axis.Y; let z = axis.Z
        let sin = xnaSin(angle); let cos = xnaCos(angle)
        let xx = x * x; let yy = y * y; let zz = z * z
        let xy = x * y; let xz = x * z; let yz = y * z
        return Self(
            xx + cos * (1 - xx), xy - cos * xy + sin * z, xz - cos * xz - sin * y, 0,
            xy - cos * xy - sin * z, yy + cos * (1 - yy), yz - cos * yz + sin * x, 0,
            xz - cos * xz + sin * y, yz - cos * yz - sin * x, zz + cos * (1 - zz), 0,
            0, 0, 0, 1)
    }
    public static func CreateFromAxisAngle(_ axis: inout Microsoft.Xna.Framework.Vector3, angle: Float, result: inout Self) {
        result = CreateFromAxisAngle(axis, angle: angle)
    }
    public static func CreateFromQuaternion(_ quaternion: Microsoft.Xna.Framework.Quaternion) -> Self {
        let xx = quaternion.X * quaternion.X; let yy = quaternion.Y * quaternion.Y; let zz = quaternion.Z * quaternion.Z
        let xy = quaternion.X * quaternion.Y; let zw = quaternion.Z * quaternion.W
        let zx = quaternion.Z * quaternion.X; let yw = quaternion.Y * quaternion.W
        let yz = quaternion.Y * quaternion.Z; let xw = quaternion.X * quaternion.W
        return Self(1 - 2 * (yy + zz), 2 * (xy + zw), 2 * (zx - yw), 0,
                    2 * (xy - zw), 1 - 2 * (zz + xx), 2 * (yz + xw), 0,
                    2 * (zx + yw), 2 * (yz - xw), 1 - 2 * (yy + xx), 0,
                    0, 0, 0, 1)
    }
    public static func CreateFromQuaternion(_ quaternion: inout Microsoft.Xna.Framework.Quaternion, result: inout Self) {
        result = CreateFromQuaternion(quaternion)
    }
    public static func CreateFromYawPitchRoll(_ yaw: Float, pitch: Float, roll: Float) -> Self {
        CreateFromQuaternion(Microsoft.Xna.Framework.Quaternion.CreateFromYawPitchRoll(yaw, pitch: pitch, roll: roll))
    }
    public static func CreateFromYawPitchRoll(_ yaw: Float, pitch: Float, roll: Float, result: inout Self) {
        result = CreateFromYawPitchRoll(yaw, pitch: pitch, roll: roll)
    }

    public static func CreateLookAt(
        _ cameraPosition: Microsoft.Xna.Framework.Vector3,
        cameraTarget: Microsoft.Xna.Framework.Vector3,
        cameraUpVector: Microsoft.Xna.Framework.Vector3
    ) -> Self {
        let backward = Microsoft.Xna.Framework.Vector3.Normalize(cameraPosition - cameraTarget)
        let right = Microsoft.Xna.Framework.Vector3.Normalize(Microsoft.Xna.Framework.Vector3.Cross(cameraUpVector, vector2: backward))
        let up = Microsoft.Xna.Framework.Vector3.Cross(backward, vector2: right)
        return Self(right.X, up.X, backward.X, 0,
                    right.Y, up.Y, backward.Y, 0,
                    right.Z, up.Z, backward.Z, 0,
                    -Microsoft.Xna.Framework.Vector3.Dot(right, vector2: cameraPosition),
                    -Microsoft.Xna.Framework.Vector3.Dot(up, vector2: cameraPosition),
                    -Microsoft.Xna.Framework.Vector3.Dot(backward, vector2: cameraPosition), 1)
    }
    public static func CreateLookAt(
        _ cameraPosition: inout Microsoft.Xna.Framework.Vector3,
        cameraTarget: inout Microsoft.Xna.Framework.Vector3,
        cameraUpVector: inout Microsoft.Xna.Framework.Vector3,
        result: inout Self
    ) { result = CreateLookAt(cameraPosition, cameraTarget: cameraTarget, cameraUpVector: cameraUpVector) }

    public static func CreateWorld(
        _ position: Microsoft.Xna.Framework.Vector3,
        forward: Microsoft.Xna.Framework.Vector3,
        up: Microsoft.Xna.Framework.Vector3
    ) -> Self {
        let backward = Microsoft.Xna.Framework.Vector3.Normalize(-forward)
        let right = Microsoft.Xna.Framework.Vector3.Normalize(Microsoft.Xna.Framework.Vector3.Cross(up, vector2: backward))
        let correctedUp = Microsoft.Xna.Framework.Vector3.Cross(backward, vector2: right)
        return Self(right.X, right.Y, right.Z, 0,
                    correctedUp.X, correctedUp.Y, correctedUp.Z, 0,
                    backward.X, backward.Y, backward.Z, 0,
                    position.X, position.Y, position.Z, 1)
    }
    public static func CreateWorld(
        _ position: inout Microsoft.Xna.Framework.Vector3,
        forward: inout Microsoft.Xna.Framework.Vector3,
        up: inout Microsoft.Xna.Framework.Vector3,
        result: inout Self
    ) { result = CreateWorld(position, forward: forward, up: up) }

    public static func CreateOrthographic(_ width: Float, height: Float, zNearPlane: Float, zFarPlane: Float) -> Self {
        Self(2 / width, 0, 0, 0,
             0, 2 / height, 0, 0,
             0, 0, 1 / (zNearPlane - zFarPlane), 0,
             0, 0, zNearPlane / (zNearPlane - zFarPlane), 1)
    }
    public static func CreateOrthographic(_ width: Float, height: Float, zNearPlane: Float, zFarPlane: Float, result: inout Self) {
        result = CreateOrthographic(width, height: height, zNearPlane: zNearPlane, zFarPlane: zFarPlane)
    }
    public static func CreateOrthographicOffCenter(
        _ left: Float, right: Float, bottom: Float, top: Float, zNearPlane: Float, zFarPlane: Float
    ) -> Self {
        Self(2 / (right - left), 0, 0, 0,
             0, 2 / (top - bottom), 0, 0,
             0, 0, 1 / (zNearPlane - zFarPlane), 0,
             (left + right) / (left - right),
             (top + bottom) / (bottom - top),
             zNearPlane / (zNearPlane - zFarPlane), 1)
    }
    public static func CreateOrthographicOffCenter(
        _ left: Float, right: Float, bottom: Float, top: Float, zNearPlane: Float, zFarPlane: Float, result: inout Self
    ) { result = CreateOrthographicOffCenter(left, right: right, bottom: bottom, top: top, zNearPlane: zNearPlane, zFarPlane: zFarPlane) }

    public static func CreatePerspectiveFieldOfView(
        _ fieldOfView: Float, aspectRatio: Float, nearPlaneDistance: Float, farPlaneDistance: Float
    ) throws -> Self {
        if fieldOfView <= 0 || fieldOfView >= Microsoft.Xna.Framework.MathHelper.Pi {
            // ArgumentOutOfRangeException("fieldOfView",
            //   String.Format(CurrentCulture, OutRangeFieldOfView, "fieldOfView"))
            throw CNAArgumentOutOfRangeException(
                paramName: "fieldOfView",
                message: Microsoft.Xna.Framework.Matrix.outRangeFieldOfViewFormat
                    .replacingOccurrences(of: "{0}", with: "fieldOfView"))
        }
        try validatePerspectivePlanes(nearPlaneDistance, farPlaneDistance: farPlaneDistance)
        let yScale: Float = 1 / xnaTan(fieldOfView * 0.5)
        let xScale = yScale / aspectRatio
        let depth = farPlaneDistance / (nearPlaneDistance - farPlaneDistance)
        return Self(xScale, 0, 0, 0,
                    0, yScale, 0, 0,
                    0, 0, depth, -1,
                    0, 0, nearPlaneDistance * farPlaneDistance / (nearPlaneDistance - farPlaneDistance), 0)
    }
    public static func CreatePerspectiveFieldOfView(
        _ fieldOfView: Float, aspectRatio: Float, nearPlaneDistance: Float, farPlaneDistance: Float, result: inout Self
    ) throws { result = try CreatePerspectiveFieldOfView(fieldOfView, aspectRatio: aspectRatio, nearPlaneDistance: nearPlaneDistance, farPlaneDistance: farPlaneDistance) }
    public static func CreatePerspective(
        _ width: Float, height: Float, nearPlaneDistance: Float, farPlaneDistance: Float
    ) throws -> Self {
        try validatePerspectivePlanes(nearPlaneDistance, farPlaneDistance: farPlaneDistance)
        return Self(2 * nearPlaneDistance / width, 0, 0, 0,
                    0, 2 * nearPlaneDistance / height, 0, 0,
                    0, 0, farPlaneDistance / (nearPlaneDistance - farPlaneDistance), -1,
                    0, 0, nearPlaneDistance * farPlaneDistance / (nearPlaneDistance - farPlaneDistance), 0)
    }
    public static func CreatePerspective(
        _ width: Float, height: Float, nearPlaneDistance: Float, farPlaneDistance: Float, result: inout Self
    ) throws { result = try CreatePerspective(width, height: height, nearPlaneDistance: nearPlaneDistance, farPlaneDistance: farPlaneDistance) }
    public static func CreatePerspectiveOffCenter(
        _ left: Float, right: Float, bottom: Float, top: Float,
        nearPlaneDistance: Float, farPlaneDistance: Float
    ) throws -> Self {
        try validatePerspectivePlanes(nearPlaneDistance, farPlaneDistance: farPlaneDistance)
        return Self(2 * nearPlaneDistance / (right - left), 0, 0, 0,
                    0, 2 * nearPlaneDistance / (top - bottom), 0, 0,
                    (left + right) / (right - left),
                    (top + bottom) / (top - bottom),
                    farPlaneDistance / (nearPlaneDistance - farPlaneDistance), -1,
                    0, 0, nearPlaneDistance * farPlaneDistance / (nearPlaneDistance - farPlaneDistance), 0)
    }
    public static func CreatePerspectiveOffCenter(
        _ left: Float, right: Float, bottom: Float, top: Float,
        nearPlaneDistance: Float, farPlaneDistance: Float, result: inout Self
    ) throws { result = try CreatePerspectiveOffCenter(left, right: right, bottom: bottom, top: top, nearPlaneDistance: nearPlaneDistance, farPlaneDistance: farPlaneDistance) }

    public static func CreateBillboard(
        _ objectPosition: Microsoft.Xna.Framework.Vector3,
        cameraPosition: Microsoft.Xna.Framework.Vector3,
        cameraUpVector: Microsoft.Xna.Framework.Vector3,
        cameraForwardVector: Microsoft.Xna.Framework.Vector3?
    ) -> Self {
        var facing = objectPosition - cameraPosition
        let lengthSquared = facing.LengthSquared()
        if lengthSquared < 0.0001 {
            facing = cameraForwardVector.map { -$0 } ?? .Forward
        } else {
            facing = facing * (1 / xnaSqrt(lengthSquared))
        }
        var right = Microsoft.Xna.Framework.Vector3.Cross(cameraUpVector, vector2: facing)
        right.Normalize()
        let up = Microsoft.Xna.Framework.Vector3.Cross(facing, vector2: right)
        return Self(right.X, right.Y, right.Z, 0,
                    up.X, up.Y, up.Z, 0,
                    facing.X, facing.Y, facing.Z, 0,
                    objectPosition.X, objectPosition.Y, objectPosition.Z, 1)
    }
    public static func CreateBillboard(
        _ objectPosition: inout Microsoft.Xna.Framework.Vector3,
        cameraPosition: inout Microsoft.Xna.Framework.Vector3,
        cameraUpVector: inout Microsoft.Xna.Framework.Vector3,
        cameraForwardVector: Microsoft.Xna.Framework.Vector3?, result: inout Self
    ) { result = CreateBillboard(objectPosition, cameraPosition: cameraPosition, cameraUpVector: cameraUpVector, cameraForwardVector: cameraForwardVector) }

    public static func CreateConstrainedBillboard(
        _ objectPosition: Microsoft.Xna.Framework.Vector3,
        cameraPosition: Microsoft.Xna.Framework.Vector3,
        rotateAxis: Microsoft.Xna.Framework.Vector3,
        cameraForwardVector: Microsoft.Xna.Framework.Vector3?,
        objectForwardVector: Microsoft.Xna.Framework.Vector3?
    ) -> Self {
        var facing = objectPosition - cameraPosition
        let lengthSquared = facing.LengthSquared()
        if lengthSquared < 0.0001 { facing = cameraForwardVector.map { -$0 } ?? .Forward }
        else { facing = facing * (1 / xnaSqrt(lengthSquared)) }
        let up = rotateAxis
        var alignment = Microsoft.Xna.Framework.Vector3.Dot(rotateAxis, vector2: facing)
        var forward: Microsoft.Xna.Framework.Vector3
        var right: Microsoft.Xna.Framework.Vector3
        if abs(alignment) > 0.99825466 {
            if let objectForwardVector {
                forward = objectForwardVector
                alignment = Microsoft.Xna.Framework.Vector3.Dot(rotateAxis, vector2: forward)
                if abs(alignment) > 0.99825466 {
                    alignment = Microsoft.Xna.Framework.Vector3.Dot(rotateAxis, vector2: .Forward)
                    forward = abs(alignment) > 0.99825466 ? .Right : .Forward
                }
            } else {
                alignment = Microsoft.Xna.Framework.Vector3.Dot(rotateAxis, vector2: .Forward)
                forward = abs(alignment) > 0.99825466 ? .Right : .Forward
            }
            right = Microsoft.Xna.Framework.Vector3.Cross(rotateAxis, vector2: forward)
            right.Normalize()
            forward = Microsoft.Xna.Framework.Vector3.Cross(right, vector2: rotateAxis)
            forward.Normalize()
        } else {
            right = Microsoft.Xna.Framework.Vector3.Cross(rotateAxis, vector2: facing)
            right.Normalize()
            forward = Microsoft.Xna.Framework.Vector3.Cross(right, vector2: up)
            forward.Normalize()
        }
        return Self(right.X, right.Y, right.Z, 0,
                    up.X, up.Y, up.Z, 0,
                    forward.X, forward.Y, forward.Z, 0,
                    objectPosition.X, objectPosition.Y, objectPosition.Z, 1)
    }
    public static func CreateConstrainedBillboard(
        _ objectPosition: inout Microsoft.Xna.Framework.Vector3,
        cameraPosition: inout Microsoft.Xna.Framework.Vector3,
        rotateAxis: inout Microsoft.Xna.Framework.Vector3,
        cameraForwardVector: Microsoft.Xna.Framework.Vector3?,
        objectForwardVector: Microsoft.Xna.Framework.Vector3?, result: inout Self
    ) { result = CreateConstrainedBillboard(objectPosition, cameraPosition: cameraPosition, rotateAxis: rotateAxis, cameraForwardVector: cameraForwardVector, objectForwardVector: objectForwardVector) }

    public static func CreateShadow(
        _ lightDirection: Microsoft.Xna.Framework.Vector3,
        plane: Microsoft.Xna.Framework.Plane
    ) -> Self {
        let normalized = Microsoft.Xna.Framework.Plane.Normalize(plane)
        let dot = normalized.Normal.X * lightDirection.X + normalized.Normal.Y * lightDirection.Y + normalized.Normal.Z * lightDirection.Z
        let x = -normalized.Normal.X; let y = -normalized.Normal.Y; let z = -normalized.Normal.Z; let d = -normalized.D
        return Self(x * lightDirection.X + dot, x * lightDirection.Y, x * lightDirection.Z, 0,
                    y * lightDirection.X, y * lightDirection.Y + dot, y * lightDirection.Z, 0,
                    z * lightDirection.X, z * lightDirection.Y, z * lightDirection.Z + dot, 0,
                    d * lightDirection.X, d * lightDirection.Y, d * lightDirection.Z, dot)
    }
    public static func CreateShadow(
        _ lightDirection: inout Microsoft.Xna.Framework.Vector3,
        plane: inout Microsoft.Xna.Framework.Plane, result: inout Self
    ) { result = CreateShadow(lightDirection, plane: plane) }

    public static func CreateReflection(_ value: Microsoft.Xna.Framework.Plane) -> Self {
        reflectionFromNormalized(Microsoft.Xna.Framework.Plane.Normalize(value))
    }
    public static func CreateReflection(_ value: inout Microsoft.Xna.Framework.Plane, result: inout Self) {
        let normalized = Microsoft.Xna.Framework.Plane.Normalize(value)
        value.Normalize()
        result = reflectionFromNormalized(normalized)
    }

    private static func reflectionFromNormalized(_ value: Microsoft.Xna.Framework.Plane) -> Self {
        let x = value.Normal.X; let y = value.Normal.Y; let z = value.Normal.Z
        let doubledX = -2 * x; let doubledY = -2 * y; let doubledZ = -2 * z
        return Self(doubledX * x + 1, doubledY * x, doubledZ * x, 0,
                    doubledX * y, doubledY * y + 1, doubledZ * y, 0,
                    doubledX * z, doubledY * z, doubledZ * z + 1, 0,
                    doubledX * value.D, doubledY * value.D, doubledZ * value.D, 1)
    }

    /// The three plane checks every perspective factory performs, in order.
    ///
    /// The first two format `NegativePlaneDistance` with the offending
    /// parameter's own name; the third reports `nearPlaneDistance` with the
    /// fixed `OppositePlanes` sentence, so a caller whose far plane is the
    /// problem still sees the near plane named — which is XNA's own choice and
    /// is reproduced rather than corrected.
    private static func validatePerspectivePlanes(_ nearPlaneDistance: Float, farPlaneDistance: Float) throws {
        if nearPlaneDistance <= 0 {
            throw CNAArgumentOutOfRangeException(
                paramName: "nearPlaneDistance",
                message: Microsoft.Xna.Framework.Matrix.negativePlaneDistanceFormat
                    .replacingOccurrences(of: "{0}", with: "nearPlaneDistance"))
        }
        if farPlaneDistance <= 0 {
            throw CNAArgumentOutOfRangeException(
                paramName: "farPlaneDistance",
                message: Microsoft.Xna.Framework.Matrix.negativePlaneDistanceFormat
                    .replacingOccurrences(of: "{0}", with: "farPlaneDistance"))
        }
        if nearPlaneDistance >= farPlaneDistance {
            throw CNAArgumentOutOfRangeException(
                paramName: "nearPlaneDistance",
                message: Microsoft.Xna.Framework.Matrix.oppositePlanesMessage)
        }
    }

    /// The exact `OutRangeFieldOfView` template, read out of
    /// `Microsoft.Xna.Framework.dll`'s own resource table.
    internal static let outRangeFieldOfViewFormat =
        "{0} takes a value between 0 and Pi (180 degrees) in radians."

    /// The exact `NegativePlaneDistance` template.
    internal static let negativePlaneDistanceFormat =
        "You should specify positive value for {0}."

    /// The exact `OppositePlanes` message.
    internal static let oppositePlanesMessage =
        "Near plane distance is larger than Far plane distance. Near plane "
        + "distance must be smaller than Far plane distance."
}
