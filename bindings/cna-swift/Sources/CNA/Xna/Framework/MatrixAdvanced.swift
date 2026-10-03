// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Matrix {
    public static func Invert(_ matrix: Microsoft.Xna.Framework.Matrix) -> Microsoft.Xna.Framework.Matrix {
        var matrix = matrix
        var result = Identity
        Invert(&matrix, result: &result)
        return result
    }

    public static func Invert(
        _ matrix: inout Microsoft.Xna.Framework.Matrix,
        result: inout Microsoft.Xna.Framework.Matrix
    ) {
        let num1 = matrix.M11; let num2 = matrix.M12; let num3 = matrix.M13; let num4 = matrix.M14
        let num5 = matrix.M21; let num6 = matrix.M22; let num7 = matrix.M23; let num8 = matrix.M24
        let num9 = matrix.M31; let num10 = matrix.M32; let num11 = matrix.M33; let num12 = matrix.M34
        let num13 = matrix.M41; let num14 = matrix.M42; let num15 = matrix.M43; let num16 = matrix.M44
        let num17 = num11 * num16 - num12 * num15
        let num18 = num10 * num16 - num12 * num14
        let num19 = num10 * num15 - num11 * num14
        let num20 = num9 * num16 - num12 * num13
        let num21 = num9 * num15 - num11 * num13
        let num22 = num9 * num14 - num10 * num13
        let num23 = num6 * num17 - num7 * num18 + num8 * num19
        let num24 = -(num5 * num17 - num7 * num20 + num8 * num21)
        let num25 = num5 * num18 - num6 * num20 + num8 * num22
        let num26 = -(num5 * num19 - num6 * num21 + num7 * num22)
        let num27: Float = 1 / (num1 * num23 + num2 * num24 + num3 * num25 + num4 * num26)

        result.M11 = num23 * num27; result.M21 = num24 * num27
        result.M31 = num25 * num27; result.M41 = num26 * num27
        result.M12 = -(num2 * num17 - num3 * num18 + num4 * num19) * num27
        result.M22 = (num1 * num17 - num3 * num20 + num4 * num21) * num27
        result.M32 = -(num1 * num18 - num2 * num20 + num4 * num22) * num27
        result.M42 = (num1 * num19 - num2 * num21 + num3 * num22) * num27

        let num28 = num7 * num16 - num8 * num15
        let num29 = num6 * num16 - num8 * num14
        let num30 = num6 * num15 - num7 * num14
        let num31 = num5 * num16 - num8 * num13
        let num32 = num5 * num15 - num7 * num13
        let num33 = num5 * num14 - num6 * num13
        result.M13 = (num2 * num28 - num3 * num29 + num4 * num30) * num27
        result.M23 = -(num1 * num28 - num3 * num31 + num4 * num32) * num27
        result.M33 = (num1 * num29 - num2 * num31 + num4 * num33) * num27
        result.M43 = -(num1 * num30 - num2 * num32 + num3 * num33) * num27

        let num34 = num7 * num12 - num8 * num11
        let num35 = num6 * num12 - num8 * num10
        let num36 = num6 * num11 - num7 * num10
        let num37 = num5 * num12 - num8 * num9
        let num38 = num5 * num11 - num7 * num9
        let num39 = num5 * num10 - num6 * num9
        result.M14 = -(num2 * num34 - num3 * num35 + num4 * num36) * num27
        result.M24 = (num1 * num34 - num3 * num37 + num4 * num38) * num27
        result.M34 = -(num1 * num35 - num2 * num37 + num4 * num39) * num27
        result.M44 = (num1 * num36 - num2 * num38 + num3 * num39) * num27
    }

    public func Decompose(
        _ scale: inout Microsoft.Xna.Framework.Vector3,
        rotation: inout Microsoft.Xna.Framework.Quaternion,
        translation: inout Microsoft.Xna.Framework.Vector3
    ) -> Bool {
        translation = Microsoft.Xna.Framework.Vector3(M41, M42, M43)
        var basis = [
            Microsoft.Xna.Framework.Vector3(M11, M12, M13),
            Microsoft.Xna.Framework.Vector3(M21, M22, M23),
            Microsoft.Xna.Framework.Vector3(M31, M32, M33),
        ]
        var scales = [basis[0].Length(), basis[1].Length(), basis[2].Length()]
        let canonical: [Microsoft.Xna.Framework.Vector3] = [.UnitX, .UnitY, .UnitZ]

        let largest: Int
        let middle: Int
        let smallest: Int
        if scales[0] < scales[1] {
            if scales[1] < scales[2] { largest = 2; middle = 1; smallest = 0 }
            else if scales[0] < scales[2] { largest = 1; middle = 2; smallest = 0 }
            else { largest = 1; middle = 0; smallest = 2 }
        } else if scales[0] < scales[2] {
            largest = 2; middle = 0; smallest = 1
        } else if scales[1] < scales[2] {
            largest = 0; middle = 2; smallest = 1
        } else {
            largest = 0; middle = 1; smallest = 2
        }

        if scales[largest] < 0.0001 { basis[largest] = canonical[largest] }
        basis[largest].Normalize()
        if scales[middle] < 0.0001 {
            let absolute = Microsoft.Xna.Framework.Vector3(abs(basis[largest].X), abs(basis[largest].Y), abs(basis[largest].Z))
            let leastAligned: Int
            if absolute.X < absolute.Y {
                leastAligned = absolute.X < absolute.Z ? 0 : 2
            } else {
                leastAligned = absolute.Y < absolute.Z ? 1 : 2
            }
            basis[middle] = Microsoft.Xna.Framework.Vector3.Cross(basis[largest], vector2: canonical[leastAligned])
        }
        basis[middle].Normalize()
        if scales[smallest] < 0.0001 {
            basis[smallest] = Microsoft.Xna.Framework.Vector3.Cross(basis[largest], vector2: basis[middle])
        }
        basis[smallest].Normalize()

        var rotationMatrix = Microsoft.Xna.Framework.Matrix(
            basis[0].X, basis[0].Y, basis[0].Z, 0,
            basis[1].X, basis[1].Y, basis[1].Z, 0,
            basis[2].X, basis[2].Y, basis[2].Z, 0,
            0, 0, 0, 1)
        var determinant = rotationMatrix.Determinant()
        if determinant < 0 {
            scales[largest] = -scales[largest]
            basis[largest] = -basis[largest]
            determinant = -determinant
            rotationMatrix = Microsoft.Xna.Framework.Matrix(
                basis[0].X, basis[0].Y, basis[0].Z, 0,
                basis[1].X, basis[1].Y, basis[1].Z, 0,
                basis[2].X, basis[2].Y, basis[2].Z, 0,
                0, 0, 0, 1)
        }
        scale = Microsoft.Xna.Framework.Vector3(scales[0], scales[1], scales[2])
        let error = determinant - 1
        if error * error > 0.0001 {
            rotation = .Identity
            return false
        }
        rotation = Microsoft.Xna.Framework.Quaternion.CreateFromRotationMatrix(rotationMatrix)
        return true
    }

    public static func Transform(
        _ value: Microsoft.Xna.Framework.Matrix,
        rotation: Microsoft.Xna.Framework.Quaternion
    ) -> Microsoft.Xna.Framework.Matrix {
        var value = value; var rotation = rotation; var result = Identity
        Transform(&value, rotation: &rotation, result: &result)
        return result
    }

    public static func Transform(
        _ value: inout Microsoft.Xna.Framework.Matrix,
        rotation: inout Microsoft.Xna.Framework.Quaternion,
        result: inout Microsoft.Xna.Framework.Matrix
    ) {
        let x2 = rotation.X + rotation.X; let y2 = rotation.Y + rotation.Y; let z2 = rotation.Z + rotation.Z
        let wx2 = rotation.W * x2; let wy2 = rotation.W * y2; let wz2 = rotation.W * z2
        let xx2 = rotation.X * x2; let xy2 = rotation.X * y2; let xz2 = rotation.X * z2
        let yy2 = rotation.Y * y2; let yz2 = rotation.Y * z2; let zz2 = rotation.Z * z2
        let m11 = 1 - yy2 - zz2; let m12 = xy2 - wz2; let m13 = xz2 + wy2
        let m21 = xy2 + wz2; let m22 = 1 - xx2 - zz2; let m23 = yz2 - wx2
        let m31 = xz2 - wy2; let m32 = yz2 + wx2; let m33 = 1 - xx2 - yy2
        result = Microsoft.Xna.Framework.Matrix(
            value.M11 * m11 + value.M12 * m12 + value.M13 * m13,
            value.M11 * m21 + value.M12 * m22 + value.M13 * m23,
            value.M11 * m31 + value.M12 * m32 + value.M13 * m33, value.M14,
            value.M21 * m11 + value.M22 * m12 + value.M23 * m13,
            value.M21 * m21 + value.M22 * m22 + value.M23 * m23,
            value.M21 * m31 + value.M22 * m32 + value.M23 * m33, value.M24,
            value.M31 * m11 + value.M32 * m12 + value.M33 * m13,
            value.M31 * m21 + value.M32 * m22 + value.M33 * m23,
            value.M31 * m31 + value.M32 * m32 + value.M33 * m33, value.M34,
            value.M41 * m11 + value.M42 * m12 + value.M43 * m13,
            value.M41 * m21 + value.M42 * m22 + value.M43 * m23,
            value.M41 * m31 + value.M42 * m32 + value.M43 * m33, value.M44)
    }
}
