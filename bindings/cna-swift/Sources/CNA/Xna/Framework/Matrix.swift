// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct Matrix {
        public var M11: Float; public var M12: Float; public var M13: Float; public var M14: Float
        public var M21: Float; public var M22: Float; public var M23: Float; public var M24: Float
        public var M31: Float; public var M32: Float; public var M33: Float; public var M34: Float
        public var M41: Float; public var M42: Float; public var M43: Float; public var M44: Float

        public init(
            _ m11: Float, _ m12: Float, _ m13: Float, _ m14: Float,
            _ m21: Float, _ m22: Float, _ m23: Float, _ m24: Float,
            _ m31: Float, _ m32: Float, _ m33: Float, _ m34: Float,
            _ m41: Float, _ m42: Float, _ m43: Float, _ m44: Float
        ) {
            M11 = m11; M12 = m12; M13 = m13; M14 = m14
            M21 = m21; M22 = m22; M23 = m23; M24 = m24
            M31 = m31; M32 = m32; M33 = m33; M34 = m34
            M41 = m41; M42 = m42; M43 = m43; M44 = m44
        }

        public static var Identity: Matrix {
            Matrix(1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   0, 0, 0, 1)
        }

        public var Up: Vector3 {
            get { Vector3(M21, M22, M23) }
            set { M21 = newValue.X; M22 = newValue.Y; M23 = newValue.Z }
        }
        public var Down: Vector3 {
            get { Vector3(-M21, -M22, -M23) }
            set { M21 = -newValue.X; M22 = -newValue.Y; M23 = -newValue.Z }
        }
        public var Right: Vector3 {
            get { Vector3(M11, M12, M13) }
            set { M11 = newValue.X; M12 = newValue.Y; M13 = newValue.Z }
        }
        public var Left: Vector3 {
            get { Vector3(-M11, -M12, -M13) }
            set { M11 = -newValue.X; M12 = -newValue.Y; M13 = -newValue.Z }
        }
        public var Forward: Vector3 {
            get { Vector3(-M31, -M32, -M33) }
            set { M31 = -newValue.X; M32 = -newValue.Y; M33 = -newValue.Z }
        }
        public var Backward: Vector3 {
            get { Vector3(M31, M32, M33) }
            set { M31 = newValue.X; M32 = newValue.Y; M33 = newValue.Z }
        }
        public var Translation: Vector3 {
            get { Vector3(M41, M42, M43) }
            set { M41 = newValue.X; M42 = newValue.Y; M43 = newValue.Z }
        }

        public func Determinant() -> Float {
            let subFactor1 = M33 * M44 - M34 * M43
            let subFactor2 = M32 * M44 - M34 * M42
            let subFactor3 = M32 * M43 - M33 * M42
            let subFactor4 = M31 * M44 - M34 * M41
            let subFactor5 = M31 * M43 - M33 * M41
            let subFactor6 = M31 * M42 - M32 * M41
            return M11 * (M22 * subFactor1 - M23 * subFactor2 + M24 * subFactor3)
                - M12 * (M21 * subFactor1 - M23 * subFactor4 + M24 * subFactor5)
                + M13 * (M21 * subFactor2 - M22 * subFactor4 + M24 * subFactor6)
                - M14 * (M21 * subFactor3 - M22 * subFactor5 + M23 * subFactor6)
        }

        public static func Transpose(_ matrix: Matrix) -> Matrix {
            Matrix(matrix.M11, matrix.M21, matrix.M31, matrix.M41,
                   matrix.M12, matrix.M22, matrix.M32, matrix.M42,
                   matrix.M13, matrix.M23, matrix.M33, matrix.M43,
                   matrix.M14, matrix.M24, matrix.M34, matrix.M44)
        }
        public static func Transpose(_ matrix: inout Matrix, result: inout Matrix) { result = Transpose(matrix) }

        public static func Lerp(_ matrix1: Matrix, matrix2: Matrix, amount: Float) -> Matrix {
            Matrix(matrix1.M11 + (matrix2.M11 - matrix1.M11) * amount,
                   matrix1.M12 + (matrix2.M12 - matrix1.M12) * amount,
                   matrix1.M13 + (matrix2.M13 - matrix1.M13) * amount,
                   matrix1.M14 + (matrix2.M14 - matrix1.M14) * amount,
                   matrix1.M21 + (matrix2.M21 - matrix1.M21) * amount,
                   matrix1.M22 + (matrix2.M22 - matrix1.M22) * amount,
                   matrix1.M23 + (matrix2.M23 - matrix1.M23) * amount,
                   matrix1.M24 + (matrix2.M24 - matrix1.M24) * amount,
                   matrix1.M31 + (matrix2.M31 - matrix1.M31) * amount,
                   matrix1.M32 + (matrix2.M32 - matrix1.M32) * amount,
                   matrix1.M33 + (matrix2.M33 - matrix1.M33) * amount,
                   matrix1.M34 + (matrix2.M34 - matrix1.M34) * amount,
                   matrix1.M41 + (matrix2.M41 - matrix1.M41) * amount,
                   matrix1.M42 + (matrix2.M42 - matrix1.M42) * amount,
                   matrix1.M43 + (matrix2.M43 - matrix1.M43) * amount,
                   matrix1.M44 + (matrix2.M44 - matrix1.M44) * amount)
        }
        public static func Lerp(_ matrix1: inout Matrix, matrix2: inout Matrix, amount: Float, result: inout Matrix) { result = Lerp(matrix1, matrix2: matrix2, amount: amount) }

        public static func Negate(_ matrix: Matrix) -> Matrix { -matrix }
        public static func Negate(_ matrix: inout Matrix, result: inout Matrix) { result = -matrix }
        public static func Add(_ matrix1: Matrix, matrix2: Matrix) -> Matrix { matrix1 + matrix2 }
        public static func Add(_ matrix1: inout Matrix, matrix2: inout Matrix, result: inout Matrix) { result = matrix1 + matrix2 }
        public static func Subtract(_ matrix1: Matrix, matrix2: Matrix) -> Matrix { matrix1 - matrix2 }
        public static func Subtract(_ matrix1: inout Matrix, matrix2: inout Matrix, result: inout Matrix) { result = matrix1 - matrix2 }
        public static func Multiply(_ matrix1: Matrix, matrix2: Matrix) -> Matrix { matrix1 * matrix2 }
        public static func Multiply(_ matrix1: inout Matrix, matrix2: inout Matrix, result: inout Matrix) { result = matrix1 * matrix2 }
        public static func Multiply(_ matrix1: Matrix, scaleFactor: Float) -> Matrix { matrix1 * scaleFactor }
        public static func Multiply(_ matrix1: inout Matrix, scaleFactor: Float, result: inout Matrix) { result = matrix1 * scaleFactor }
        public static func Divide(_ matrix1: Matrix, matrix2: Matrix) -> Matrix { matrix1 / matrix2 }
        public static func Divide(_ matrix1: inout Matrix, matrix2: inout Matrix, result: inout Matrix) { result = matrix1 / matrix2 }
        public static func Divide(_ matrix1: Matrix, divider: Float) -> Matrix { matrix1 / divider }
        public static func Divide(_ matrix1: inout Matrix, divider: Float, result: inout Matrix) { result = matrix1 / divider }

        public func Equals(_ other: Matrix) -> Bool { self == other }
        public func Equals(_ obj: Any?) -> Bool { guard let other = obj as? Matrix else { return false }; return self == other }
        public func GetHashCode() -> Int32 {
            xnaFloatHash(M11) &+ xnaFloatHash(M12) &+ xnaFloatHash(M13) &+ xnaFloatHash(M14) &+
            xnaFloatHash(M21) &+ xnaFloatHash(M22) &+ xnaFloatHash(M23) &+ xnaFloatHash(M24) &+
            xnaFloatHash(M31) &+ xnaFloatHash(M32) &+ xnaFloatHash(M33) &+ xnaFloatHash(M34) &+
            xnaFloatHash(M41) &+ xnaFloatHash(M42) &+ xnaFloatHash(M43) &+ xnaFloatHash(M44)
        }
        public func ToString() -> String {
            "{ {M11:\(xnaFloatString(M11)) M12:\(xnaFloatString(M12)) M13:\(xnaFloatString(M13)) M14:\(xnaFloatString(M14))} " +
            "{M21:\(xnaFloatString(M21)) M22:\(xnaFloatString(M22)) M23:\(xnaFloatString(M23)) M24:\(xnaFloatString(M24))} " +
            "{M31:\(xnaFloatString(M31)) M32:\(xnaFloatString(M32)) M33:\(xnaFloatString(M33)) M34:\(xnaFloatString(M34))} " +
            "{M41:\(xnaFloatString(M41)) M42:\(xnaFloatString(M42)) M43:\(xnaFloatString(M43)) M44:\(xnaFloatString(M44))} }"
        }

        public static prefix func - (matrix: Matrix) -> Matrix {
            Matrix(-matrix.M11, -matrix.M12, -matrix.M13, -matrix.M14,
                   -matrix.M21, -matrix.M22, -matrix.M23, -matrix.M24,
                   -matrix.M31, -matrix.M32, -matrix.M33, -matrix.M34,
                   -matrix.M41, -matrix.M42, -matrix.M43, -matrix.M44)
        }
        public static func + (lhs: Matrix, rhs: Matrix) -> Matrix {
            Matrix(lhs.M11 + rhs.M11, lhs.M12 + rhs.M12, lhs.M13 + rhs.M13, lhs.M14 + rhs.M14,
                   lhs.M21 + rhs.M21, lhs.M22 + rhs.M22, lhs.M23 + rhs.M23, lhs.M24 + rhs.M24,
                   lhs.M31 + rhs.M31, lhs.M32 + rhs.M32, lhs.M33 + rhs.M33, lhs.M34 + rhs.M34,
                   lhs.M41 + rhs.M41, lhs.M42 + rhs.M42, lhs.M43 + rhs.M43, lhs.M44 + rhs.M44)
        }
        public static func - (lhs: Matrix, rhs: Matrix) -> Matrix {
            Matrix(lhs.M11 - rhs.M11, lhs.M12 - rhs.M12, lhs.M13 - rhs.M13, lhs.M14 - rhs.M14,
                   lhs.M21 - rhs.M21, lhs.M22 - rhs.M22, lhs.M23 - rhs.M23, lhs.M24 - rhs.M24,
                   lhs.M31 - rhs.M31, lhs.M32 - rhs.M32, lhs.M33 - rhs.M33, lhs.M34 - rhs.M34,
                   lhs.M41 - rhs.M41, lhs.M42 - rhs.M42, lhs.M43 - rhs.M43, lhs.M44 - rhs.M44)
        }
        public static func * (lhs: Matrix, rhs: Matrix) -> Matrix {
            Matrix(
                lhs.M11 * rhs.M11 + lhs.M12 * rhs.M21 + lhs.M13 * rhs.M31 + lhs.M14 * rhs.M41,
                lhs.M11 * rhs.M12 + lhs.M12 * rhs.M22 + lhs.M13 * rhs.M32 + lhs.M14 * rhs.M42,
                lhs.M11 * rhs.M13 + lhs.M12 * rhs.M23 + lhs.M13 * rhs.M33 + lhs.M14 * rhs.M43,
                lhs.M11 * rhs.M14 + lhs.M12 * rhs.M24 + lhs.M13 * rhs.M34 + lhs.M14 * rhs.M44,
                lhs.M21 * rhs.M11 + lhs.M22 * rhs.M21 + lhs.M23 * rhs.M31 + lhs.M24 * rhs.M41,
                lhs.M21 * rhs.M12 + lhs.M22 * rhs.M22 + lhs.M23 * rhs.M32 + lhs.M24 * rhs.M42,
                lhs.M21 * rhs.M13 + lhs.M22 * rhs.M23 + lhs.M23 * rhs.M33 + lhs.M24 * rhs.M43,
                lhs.M21 * rhs.M14 + lhs.M22 * rhs.M24 + lhs.M23 * rhs.M34 + lhs.M24 * rhs.M44,
                lhs.M31 * rhs.M11 + lhs.M32 * rhs.M21 + lhs.M33 * rhs.M31 + lhs.M34 * rhs.M41,
                lhs.M31 * rhs.M12 + lhs.M32 * rhs.M22 + lhs.M33 * rhs.M32 + lhs.M34 * rhs.M42,
                lhs.M31 * rhs.M13 + lhs.M32 * rhs.M23 + lhs.M33 * rhs.M33 + lhs.M34 * rhs.M43,
                lhs.M31 * rhs.M14 + lhs.M32 * rhs.M24 + lhs.M33 * rhs.M34 + lhs.M34 * rhs.M44,
                lhs.M41 * rhs.M11 + lhs.M42 * rhs.M21 + lhs.M43 * rhs.M31 + lhs.M44 * rhs.M41,
                lhs.M41 * rhs.M12 + lhs.M42 * rhs.M22 + lhs.M43 * rhs.M32 + lhs.M44 * rhs.M42,
                lhs.M41 * rhs.M13 + lhs.M42 * rhs.M23 + lhs.M43 * rhs.M33 + lhs.M44 * rhs.M43,
                lhs.M41 * rhs.M14 + lhs.M42 * rhs.M24 + lhs.M43 * rhs.M34 + lhs.M44 * rhs.M44)
        }
        public static func * (matrix: Matrix, scaleFactor: Float) -> Matrix {
            Matrix(matrix.M11 * scaleFactor, matrix.M12 * scaleFactor, matrix.M13 * scaleFactor, matrix.M14 * scaleFactor,
                   matrix.M21 * scaleFactor, matrix.M22 * scaleFactor, matrix.M23 * scaleFactor, matrix.M24 * scaleFactor,
                   matrix.M31 * scaleFactor, matrix.M32 * scaleFactor, matrix.M33 * scaleFactor, matrix.M34 * scaleFactor,
                   matrix.M41 * scaleFactor, matrix.M42 * scaleFactor, matrix.M43 * scaleFactor, matrix.M44 * scaleFactor)
        }
        public static func * (scaleFactor: Float, matrix: Matrix) -> Matrix { matrix * scaleFactor }
        public static func / (lhs: Matrix, rhs: Matrix) -> Matrix {
            Matrix(lhs.M11 / rhs.M11, lhs.M12 / rhs.M12, lhs.M13 / rhs.M13, lhs.M14 / rhs.M14,
                   lhs.M21 / rhs.M21, lhs.M22 / rhs.M22, lhs.M23 / rhs.M23, lhs.M24 / rhs.M24,
                   lhs.M31 / rhs.M31, lhs.M32 / rhs.M32, lhs.M33 / rhs.M33, lhs.M34 / rhs.M34,
                   lhs.M41 / rhs.M41, lhs.M42 / rhs.M42, lhs.M43 / rhs.M43, lhs.M44 / rhs.M44)
        }
        public static func / (matrix1: Matrix, divider: Float) -> Matrix { matrix1 * (1 / divider) }
        public static func == (lhs: Matrix, rhs: Matrix) -> Bool {
            lhs.M11 == rhs.M11 && lhs.M12 == rhs.M12 && lhs.M13 == rhs.M13 && lhs.M14 == rhs.M14 &&
            lhs.M21 == rhs.M21 && lhs.M22 == rhs.M22 && lhs.M23 == rhs.M23 && lhs.M24 == rhs.M24 &&
            lhs.M31 == rhs.M31 && lhs.M32 == rhs.M32 && lhs.M33 == rhs.M33 && lhs.M34 == rhs.M34 &&
            lhs.M41 == rhs.M41 && lhs.M42 == rhs.M42 && lhs.M43 == rhs.M43 && lhs.M44 == rhs.M44
        }
        public static func != (lhs: Matrix, rhs: Matrix) -> Bool { !(lhs == rhs) }
    }
}
