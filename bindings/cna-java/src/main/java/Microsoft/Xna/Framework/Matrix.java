package Microsoft.Xna.Framework;

import java.util.Objects;

/** Mutable row-major XNA 4x4 matrix value. */
public final class Matrix {

    public float M11; public float M12; public float M13; public float M14;
    public float M21; public float M22; public float M23; public float M24;
    public float M31; public float M32; public float M33; public float M34;
    public float M41; public float M42; public float M43; public float M44;

    public Matrix() {
    }

    public Matrix(
            float m11, float m12, float m13, float m14,
            float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34,
            float m41, float m42, float m43, float m44) {
        M11 = m11; M12 = m12; M13 = m13; M14 = m14;
        M21 = m21; M22 = m22; M23 = m23; M24 = m24;
        M31 = m31; M32 = m32; M33 = m33; M34 = m34;
        M41 = m41; M42 = m42; M43 = m43; M44 = m44;
    }

    public Matrix(Matrix value) {
        this(Objects.requireNonNull(value, "value").M11, value.M12, value.M13, value.M14,
                value.M21, value.M22, value.M23, value.M24,
                value.M31, value.M32, value.M33, value.M34,
                value.M41, value.M42, value.M43, value.M44);
    }

    public static Matrix getIdentity() {
        return new Matrix(1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    public Vector3 getUp() { return new Vector3(M21, M22, M23); }
    public void setUp(Vector3 value) { M21 = value.X; M22 = value.Y; M23 = value.Z; }
    public Vector3 getDown() { return new Vector3(-M21, -M22, -M23); }
    public void setDown(Vector3 value) { M21 = -value.X; M22 = -value.Y; M23 = -value.Z; }
    public Vector3 getRight() { return new Vector3(M11, M12, M13); }
    public void setRight(Vector3 value) { M11 = value.X; M12 = value.Y; M13 = value.Z; }
    public Vector3 getLeft() { return new Vector3(-M11, -M12, -M13); }
    public void setLeft(Vector3 value) { M11 = -value.X; M12 = -value.Y; M13 = -value.Z; }
    public Vector3 getForward() { return new Vector3(-M31, -M32, -M33); }
    public void setForward(Vector3 value) { M31 = -value.X; M32 = -value.Y; M33 = -value.Z; }
    public Vector3 getBackward() { return new Vector3(M31, M32, M33); }
    public void setBackward(Vector3 value) { M31 = value.X; M32 = value.Y; M33 = value.Z; }
    public Vector3 getTranslation() { return new Vector3(M41, M42, M43); }
    public void setTranslation(Vector3 value) { M41 = value.X; M42 = value.Y; M43 = value.Z; }

    public float Determinant() {
        float sub1 = (M33 * M44) - (M34 * M43);
        float sub2 = (M32 * M44) - (M34 * M42);
        float sub3 = (M32 * M43) - (M33 * M42);
        float sub4 = (M31 * M44) - (M34 * M41);
        float sub5 = (M31 * M43) - (M33 * M41);
        float sub6 = (M31 * M42) - (M32 * M41);
        return (M11 * ((M22 * sub1) - (M23 * sub2) + (M24 * sub3)))
                - (M12 * ((M21 * sub1) - (M23 * sub4) + (M24 * sub5)))
                + (M13 * ((M21 * sub2) - (M22 * sub4) + (M24 * sub6)))
                - (M14 * ((M21 * sub3) - (M22 * sub5) + (M23 * sub6)));
    }

    public static Matrix Add(Matrix matrix1, Matrix matrix2) {
        return new Matrix(
                matrix1.M11 + matrix2.M11, matrix1.M12 + matrix2.M12,
                matrix1.M13 + matrix2.M13, matrix1.M14 + matrix2.M14,
                matrix1.M21 + matrix2.M21, matrix1.M22 + matrix2.M22,
                matrix1.M23 + matrix2.M23, matrix1.M24 + matrix2.M24,
                matrix1.M31 + matrix2.M31, matrix1.M32 + matrix2.M32,
                matrix1.M33 + matrix2.M33, matrix1.M34 + matrix2.M34,
                matrix1.M41 + matrix2.M41, matrix1.M42 + matrix2.M42,
                matrix1.M43 + matrix2.M43, matrix1.M44 + matrix2.M44);
    }

    public static Matrix Subtract(Matrix matrix1, Matrix matrix2) {
        return new Matrix(
                matrix1.M11 - matrix2.M11, matrix1.M12 - matrix2.M12,
                matrix1.M13 - matrix2.M13, matrix1.M14 - matrix2.M14,
                matrix1.M21 - matrix2.M21, matrix1.M22 - matrix2.M22,
                matrix1.M23 - matrix2.M23, matrix1.M24 - matrix2.M24,
                matrix1.M31 - matrix2.M31, matrix1.M32 - matrix2.M32,
                matrix1.M33 - matrix2.M33, matrix1.M34 - matrix2.M34,
                matrix1.M41 - matrix2.M41, matrix1.M42 - matrix2.M42,
                matrix1.M43 - matrix2.M43, matrix1.M44 - matrix2.M44);
    }

    public static Matrix Multiply(Matrix matrix1, Matrix matrix2) {
        return new Matrix(
                (matrix1.M11 * matrix2.M11) + (matrix1.M12 * matrix2.M21) + (matrix1.M13 * matrix2.M31) + (matrix1.M14 * matrix2.M41),
                (matrix1.M11 * matrix2.M12) + (matrix1.M12 * matrix2.M22) + (matrix1.M13 * matrix2.M32) + (matrix1.M14 * matrix2.M42),
                (matrix1.M11 * matrix2.M13) + (matrix1.M12 * matrix2.M23) + (matrix1.M13 * matrix2.M33) + (matrix1.M14 * matrix2.M43),
                (matrix1.M11 * matrix2.M14) + (matrix1.M12 * matrix2.M24) + (matrix1.M13 * matrix2.M34) + (matrix1.M14 * matrix2.M44),
                (matrix1.M21 * matrix2.M11) + (matrix1.M22 * matrix2.M21) + (matrix1.M23 * matrix2.M31) + (matrix1.M24 * matrix2.M41),
                (matrix1.M21 * matrix2.M12) + (matrix1.M22 * matrix2.M22) + (matrix1.M23 * matrix2.M32) + (matrix1.M24 * matrix2.M42),
                (matrix1.M21 * matrix2.M13) + (matrix1.M22 * matrix2.M23) + (matrix1.M23 * matrix2.M33) + (matrix1.M24 * matrix2.M43),
                (matrix1.M21 * matrix2.M14) + (matrix1.M22 * matrix2.M24) + (matrix1.M23 * matrix2.M34) + (matrix1.M24 * matrix2.M44),
                (matrix1.M31 * matrix2.M11) + (matrix1.M32 * matrix2.M21) + (matrix1.M33 * matrix2.M31) + (matrix1.M34 * matrix2.M41),
                (matrix1.M31 * matrix2.M12) + (matrix1.M32 * matrix2.M22) + (matrix1.M33 * matrix2.M32) + (matrix1.M34 * matrix2.M42),
                (matrix1.M31 * matrix2.M13) + (matrix1.M32 * matrix2.M23) + (matrix1.M33 * matrix2.M33) + (matrix1.M34 * matrix2.M43),
                (matrix1.M31 * matrix2.M14) + (matrix1.M32 * matrix2.M24) + (matrix1.M33 * matrix2.M34) + (matrix1.M34 * matrix2.M44),
                (matrix1.M41 * matrix2.M11) + (matrix1.M42 * matrix2.M21) + (matrix1.M43 * matrix2.M31) + (matrix1.M44 * matrix2.M41),
                (matrix1.M41 * matrix2.M12) + (matrix1.M42 * matrix2.M22) + (matrix1.M43 * matrix2.M32) + (matrix1.M44 * matrix2.M42),
                (matrix1.M41 * matrix2.M13) + (matrix1.M42 * matrix2.M23) + (matrix1.M43 * matrix2.M33) + (matrix1.M44 * matrix2.M43),
                (matrix1.M41 * matrix2.M14) + (matrix1.M42 * matrix2.M24) + (matrix1.M43 * matrix2.M34) + (matrix1.M44 * matrix2.M44));
    }

    public static Matrix Multiply(Matrix matrix, float scaleFactor) {
        return scale(matrix, scaleFactor);
    }

    public static Matrix Multiply(float scaleFactor, Matrix matrix) {
        return scale(matrix, scaleFactor);
    }

    private static Matrix scale(Matrix matrix, float factor) {
        return new Matrix(
                matrix.M11 * factor, matrix.M12 * factor, matrix.M13 * factor, matrix.M14 * factor,
                matrix.M21 * factor, matrix.M22 * factor, matrix.M23 * factor, matrix.M24 * factor,
                matrix.M31 * factor, matrix.M32 * factor, matrix.M33 * factor, matrix.M34 * factor,
                matrix.M41 * factor, matrix.M42 * factor, matrix.M43 * factor, matrix.M44 * factor);
    }

    public static Matrix Divide(Matrix matrix1, Matrix matrix2) {
        return new Matrix(
                matrix1.M11 / matrix2.M11, matrix1.M12 / matrix2.M12,
                matrix1.M13 / matrix2.M13, matrix1.M14 / matrix2.M14,
                matrix1.M21 / matrix2.M21, matrix1.M22 / matrix2.M22,
                matrix1.M23 / matrix2.M23, matrix1.M24 / matrix2.M24,
                matrix1.M31 / matrix2.M31, matrix1.M32 / matrix2.M32,
                matrix1.M33 / matrix2.M33, matrix1.M34 / matrix2.M34,
                matrix1.M41 / matrix2.M41, matrix1.M42 / matrix2.M42,
                matrix1.M43 / matrix2.M43, matrix1.M44 / matrix2.M44);
    }

    public static Matrix Divide(Matrix matrix1, float divider) {
        return scale(matrix1, 1.0f / divider);
    }

    public static Matrix Negate(Matrix matrix1) {
        return new Matrix(
                -matrix1.M11, -matrix1.M12, -matrix1.M13, -matrix1.M14,
                -matrix1.M21, -matrix1.M22, -matrix1.M23, -matrix1.M24,
                -matrix1.M31, -matrix1.M32, -matrix1.M33, -matrix1.M34,
                -matrix1.M41, -matrix1.M42, -matrix1.M43, -matrix1.M44);
    }

    public static Matrix Lerp(Matrix matrix1, Matrix matrix2, float amount) {
        return new Matrix(
                matrix1.M11 + ((matrix2.M11 - matrix1.M11) * amount),
                matrix1.M12 + ((matrix2.M12 - matrix1.M12) * amount),
                matrix1.M13 + ((matrix2.M13 - matrix1.M13) * amount),
                matrix1.M14 + ((matrix2.M14 - matrix1.M14) * amount),
                matrix1.M21 + ((matrix2.M21 - matrix1.M21) * amount),
                matrix1.M22 + ((matrix2.M22 - matrix1.M22) * amount),
                matrix1.M23 + ((matrix2.M23 - matrix1.M23) * amount),
                matrix1.M24 + ((matrix2.M24 - matrix1.M24) * amount),
                matrix1.M31 + ((matrix2.M31 - matrix1.M31) * amount),
                matrix1.M32 + ((matrix2.M32 - matrix1.M32) * amount),
                matrix1.M33 + ((matrix2.M33 - matrix1.M33) * amount),
                matrix1.M34 + ((matrix2.M34 - matrix1.M34) * amount),
                matrix1.M41 + ((matrix2.M41 - matrix1.M41) * amount),
                matrix1.M42 + ((matrix2.M42 - matrix1.M42) * amount),
                matrix1.M43 + ((matrix2.M43 - matrix1.M43) * amount),
                matrix1.M44 + ((matrix2.M44 - matrix1.M44) * amount));
    }

    public static Matrix Transpose(Matrix matrix) {
        return new Matrix(
                matrix.M11, matrix.M21, matrix.M31, matrix.M41,
                matrix.M12, matrix.M22, matrix.M32, matrix.M42,
                matrix.M13, matrix.M23, matrix.M33, matrix.M43,
                matrix.M14, matrix.M24, matrix.M34, matrix.M44);
    }

    public static Matrix Invert(Matrix matrix) {
        float m = matrix.M11; float m2 = matrix.M12; float m3 = matrix.M13; float m4 = matrix.M14;
        float m5 = matrix.M21; float m6 = matrix.M22; float m7 = matrix.M23; float m8 = matrix.M24;
        float m9 = matrix.M31; float m10 = matrix.M32; float m11 = matrix.M33; float m12 = matrix.M34;
        float m13 = matrix.M41; float m14 = matrix.M42; float m15 = matrix.M43; float m16 = matrix.M44;
        float n = (m11 * m16) - (m12 * m15);
        float n2 = (m10 * m16) - (m12 * m14);
        float n3 = (m10 * m15) - (m11 * m14);
        float n4 = (m9 * m16) - (m12 * m13);
        float n5 = (m9 * m15) - (m11 * m13);
        float n6 = (m9 * m14) - (m10 * m13);
        float n7 = (m6 * n) - (m7 * n2) + (m8 * n3);
        float n8 = -((m5 * n) - (m7 * n4) + (m8 * n5));
        float n9 = (m5 * n2) - (m6 * n4) + (m8 * n6);
        float n10 = -((m5 * n3) - (m6 * n5) + (m7 * n6));
        float inverse = 1.0f / ((m * n7) + (m2 * n8) + (m3 * n9) + (m4 * n10));
        float n12 = (m7 * m16) - (m8 * m15);
        float n13 = (m6 * m16) - (m8 * m14);
        float n14 = (m6 * m15) - (m7 * m14);
        float n15 = (m5 * m16) - (m8 * m13);
        float n16 = (m5 * m15) - (m7 * m13);
        float n17 = (m5 * m14) - (m6 * m13);
        float n18 = (m7 * m12) - (m8 * m11);
        float n19 = (m6 * m12) - (m8 * m10);
        float n20 = (m6 * m11) - (m7 * m10);
        float n21 = (m5 * m12) - (m8 * m9);
        float n22 = (m5 * m11) - (m7 * m9);
        float n23 = (m5 * m10) - (m6 * m9);
        return new Matrix(
                n7 * inverse,
                -((m2 * n) - (m3 * n2) + (m4 * n3)) * inverse,
                ((m2 * n12) - (m3 * n13) + (m4 * n14)) * inverse,
                -((m2 * n18) - (m3 * n19) + (m4 * n20)) * inverse,
                n8 * inverse,
                ((m * n) - (m3 * n4) + (m4 * n5)) * inverse,
                -((m * n12) - (m3 * n15) + (m4 * n16)) * inverse,
                ((m * n18) - (m3 * n21) + (m4 * n22)) * inverse,
                n9 * inverse,
                -((m * n2) - (m2 * n4) + (m4 * n6)) * inverse,
                ((m * n13) - (m2 * n15) + (m4 * n17)) * inverse,
                -((m * n19) - (m2 * n21) + (m4 * n23)) * inverse,
                n10 * inverse,
                ((m * n3) - (m2 * n5) + (m3 * n6)) * inverse,
                -((m * n14) - (m2 * n16) + (m3 * n17)) * inverse,
                ((m * n20) - (m2 * n22) + (m3 * n23)) * inverse);
    }

    public static Matrix CreateScale(float scale) { return CreateScale(scale, scale, scale); }
    public static Matrix CreateScale(Vector3 scales) { return CreateScale(scales.X, scales.Y, scales.Z); }
    public static Matrix CreateScale(float xScale, float yScale, float zScale) {
        return new Matrix(xScale, 0.0f, 0.0f, 0.0f,
                0.0f, yScale, 0.0f, 0.0f,
                0.0f, 0.0f, zScale, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    public static Matrix CreateTranslation(float xPosition, float yPosition, float zPosition) {
        return new Matrix(1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                xPosition, yPosition, zPosition, 1.0f);
    }

    public static Matrix CreateTranslation(Vector3 position) {
        return CreateTranslation(position.X, position.Y, position.Z);
    }

    public static Matrix CreateRotationX(float radians) {
        float cosine = (float)Math.cos(radians);
        float sine = (float)Math.sin(radians);
        return new Matrix(1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, cosine, sine, 0.0f,
                0.0f, -sine, cosine, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    public static Matrix CreateRotationY(float radians) {
        float cosine = (float)Math.cos(radians);
        float sine = (float)Math.sin(radians);
        return new Matrix(cosine, 0.0f, -sine, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                sine, 0.0f, cosine, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    public static Matrix CreateRotationZ(float radians) {
        float cosine = (float)Math.cos(radians);
        float sine = (float)Math.sin(radians);
        return new Matrix(cosine, sine, 0.0f, 0.0f,
                -sine, cosine, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    public static Matrix CreateFromAxisAngle(Vector3 axis, float angle) {
        float x = axis.X; float y = axis.Y; float z = axis.Z;
        float sine = (float)Math.sin(angle);
        float cosine = (float)Math.cos(angle);
        float xx = x * x; float yy = y * y; float zz = z * z;
        float xy = x * y; float xz = x * z; float yz = y * z;
        return new Matrix(
                xx + (cosine * (1.0f - xx)), xy - (cosine * xy) + (sine * z), xz - (cosine * xz) - (sine * y), 0.0f,
                xy - (cosine * xy) - (sine * z), yy + (cosine * (1.0f - yy)), yz - (cosine * yz) + (sine * x), 0.0f,
                xz - (cosine * xz) + (sine * y), yz - (cosine * yz) - (sine * x), zz + (cosine * (1.0f - zz)), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    public static Matrix CreateFromQuaternion(Quaternion quaternion) {
        float xx = quaternion.X * quaternion.X;
        float yy = quaternion.Y * quaternion.Y;
        float zz = quaternion.Z * quaternion.Z;
        float xy = quaternion.X * quaternion.Y;
        float zw = quaternion.Z * quaternion.W;
        float zx = quaternion.Z * quaternion.X;
        float yw = quaternion.Y * quaternion.W;
        float yz = quaternion.Y * quaternion.Z;
        float xw = quaternion.X * quaternion.W;
        return new Matrix(
                1.0f - (2.0f * (yy + zz)), 2.0f * (xy + zw), 2.0f * (zx - yw), 0.0f,
                2.0f * (xy - zw), 1.0f - (2.0f * (zz + xx)), 2.0f * (yz + xw), 0.0f,
                2.0f * (zx + yw), 2.0f * (yz - xw), 1.0f - (2.0f * (yy + xx)), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    public static Matrix CreateFromYawPitchRoll(float yaw, float pitch, float roll) {
        return CreateFromQuaternion(Quaternion.CreateFromYawPitchRoll(yaw, pitch, roll));
    }

    public static Matrix CreatePerspectiveFieldOfView(
            float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance) {
        if (fieldOfView <= 0.0f || fieldOfView >= (float)Math.PI) {
            throw new IllegalArgumentException("fieldOfView must be greater than zero and less than Pi.");
        }
        validatePerspectivePlanes(nearPlaneDistance, farPlaneDistance);
        float height = 1.0f / (float)Math.tan(fieldOfView * 0.5f);
        float width = height / aspectRatio;
        float range = nearPlaneDistance - farPlaneDistance;
        return new Matrix(width, 0.0f, 0.0f, 0.0f,
                0.0f, height, 0.0f, 0.0f,
                0.0f, 0.0f, farPlaneDistance / range, -1.0f,
                0.0f, 0.0f, (nearPlaneDistance * farPlaneDistance) / range, 0.0f);
    }

    public static Matrix CreatePerspective(
            float width, float height, float nearPlaneDistance, float farPlaneDistance) {
        validatePerspectivePlanes(nearPlaneDistance, farPlaneDistance);
        float range = nearPlaneDistance - farPlaneDistance;
        return new Matrix((2.0f * nearPlaneDistance) / width, 0.0f, 0.0f, 0.0f,
                0.0f, (2.0f * nearPlaneDistance) / height, 0.0f, 0.0f,
                0.0f, 0.0f, farPlaneDistance / range, -1.0f,
                0.0f, 0.0f, (nearPlaneDistance * farPlaneDistance) / range, 0.0f);
    }

    public static Matrix CreatePerspectiveOffCenter(
            float left, float right, float bottom, float top,
            float nearPlaneDistance, float farPlaneDistance) {
        validatePerspectivePlanes(nearPlaneDistance, farPlaneDistance);
        float depthRange = nearPlaneDistance - farPlaneDistance;
        return new Matrix(
                (2.0f * nearPlaneDistance) / (right - left), 0.0f, 0.0f, 0.0f,
                0.0f, (2.0f * nearPlaneDistance) / (top - bottom), 0.0f, 0.0f,
                (left + right) / (right - left), (top + bottom) / (top - bottom),
                farPlaneDistance / depthRange, -1.0f,
                0.0f, 0.0f, (nearPlaneDistance * farPlaneDistance) / depthRange, 0.0f);
    }

    private static void validatePerspectivePlanes(float nearPlaneDistance, float farPlaneDistance) {
        if (nearPlaneDistance <= 0.0f) {
            throw new IllegalArgumentException("nearPlaneDistance must be greater than zero.");
        }
        if (farPlaneDistance <= 0.0f) {
            throw new IllegalArgumentException("farPlaneDistance must be greater than zero.");
        }
        if (nearPlaneDistance >= farPlaneDistance) {
            throw new IllegalArgumentException("nearPlaneDistance must be less than farPlaneDistance.");
        }
    }

    public static Matrix CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) {
        float range = zNearPlane - zFarPlane;
        return new Matrix(2.0f / width, 0.0f, 0.0f, 0.0f,
                0.0f, 2.0f / height, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f / range, 0.0f,
                0.0f, 0.0f, zNearPlane / range, 1.0f);
    }

    public static Matrix CreateOrthographicOffCenter(
            float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) {
        return new Matrix(2.0f / (right - left), 0.0f, 0.0f, 0.0f,
                0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f / (zNearPlane - zFarPlane), 0.0f,
                (left + right) / (left - right), (top + bottom) / (bottom - top),
                zNearPlane / (zNearPlane - zFarPlane), 1.0f);
    }

    public static Matrix CreateLookAt(Vector3 cameraPosition, Vector3 cameraTarget, Vector3 cameraUpVector) {
        Vector3 backward = Vector3.Normalize(Vector3.Subtract(cameraPosition, cameraTarget));
        Vector3 right = Vector3.Normalize(Vector3.Cross(cameraUpVector, backward));
        Vector3 up = Vector3.Cross(backward, right);
        return new Matrix(
                right.X, up.X, backward.X, 0.0f,
                right.Y, up.Y, backward.Y, 0.0f,
                right.Z, up.Z, backward.Z, 0.0f,
                -Vector3.Dot(right, cameraPosition),
                -Vector3.Dot(up, cameraPosition),
                -Vector3.Dot(backward, cameraPosition), 1.0f);
    }

    public static Matrix CreateWorld(Vector3 position, Vector3 forward, Vector3 up) {
        Vector3 backward = Vector3.Normalize(new Vector3(
                -forward.X, -forward.Y, -forward.Z));
        Vector3 right = Vector3.Normalize(Vector3.Cross(up, backward));
        Vector3 actualUp = Vector3.Cross(backward, right);
        return new Matrix(
                right.X, right.Y, right.Z, 0.0f,
                actualUp.X, actualUp.Y, actualUp.Z, 0.0f,
                backward.X, backward.Y, backward.Z, 0.0f,
                position.X, position.Y, position.Z, 1.0f);
    }

    public static Matrix CreateBillboard(
            Vector3 objectPosition, Vector3 cameraPosition,
            Vector3 cameraUpVector, Vector3 cameraForwardVector) {
        Vector3 backward = Vector3.Subtract(objectPosition, cameraPosition);
        float lengthSquared = backward.LengthSquared();
        if (lengthSquared < 0.0001f) {
            backward = cameraForwardVector == null
                    ? Vector3.getForward()
                    : new Vector3(-cameraForwardVector.X,
                            -cameraForwardVector.Y, -cameraForwardVector.Z);
        } else {
            backward = Vector3.Multiply(backward, 1.0f / (float)Math.sqrt(lengthSquared));
        }
        Vector3 right = Vector3.Cross(cameraUpVector, backward);
        right.Normalize();
        Vector3 up = Vector3.Cross(backward, right);
        return new Matrix(
                right.X, right.Y, right.Z, 0.0f,
                up.X, up.Y, up.Z, 0.0f,
                backward.X, backward.Y, backward.Z, 0.0f,
                objectPosition.X, objectPosition.Y, objectPosition.Z, 1.0f);
    }

    public static Matrix CreateConstrainedBillboard(
            Vector3 objectPosition, Vector3 cameraPosition, Vector3 rotateAxis,
            Vector3 cameraForwardVector, Vector3 objectForwardVector) {
        Vector3 backward = Vector3.Subtract(objectPosition, cameraPosition);
        float lengthSquared = backward.LengthSquared();
        if (lengthSquared < 0.0001f) {
            backward = cameraForwardVector == null
                    ? Vector3.getForward()
                    : new Vector3(-cameraForwardVector.X,
                            -cameraForwardVector.Y, -cameraForwardVector.Z);
        } else {
            backward = Vector3.Multiply(backward, 1.0f / (float)Math.sqrt(lengthSquared));
        }
        Vector3 axis = new Vector3(rotateAxis);
        float dot = Vector3.Dot(rotateAxis, backward);
        Vector3 right;
        Vector3 actualBackward;
        if (Math.abs(dot) > 0.99825466f) {
            if (objectForwardVector != null) {
                actualBackward = new Vector3(objectForwardVector);
                dot = Vector3.Dot(rotateAxis, actualBackward);
                if (Math.abs(dot) > 0.99825466f) {
                    dot = (rotateAxis.X * Vector3.getForward().X)
                            + (rotateAxis.Y * Vector3.getForward().Y)
                            + (rotateAxis.Z * Vector3.getForward().Z);
                    actualBackward = Math.abs(dot) > 0.99825466f
                            ? Vector3.getRight() : Vector3.getForward();
                }
            } else {
                dot = (rotateAxis.X * Vector3.getForward().X)
                        + (rotateAxis.Y * Vector3.getForward().Y)
                        + (rotateAxis.Z * Vector3.getForward().Z);
                actualBackward = Math.abs(dot) > 0.99825466f
                        ? Vector3.getRight() : Vector3.getForward();
            }
            right = Vector3.Cross(rotateAxis, actualBackward);
            right.Normalize();
            actualBackward = Vector3.Cross(right, rotateAxis);
            actualBackward.Normalize();
        } else {
            right = Vector3.Cross(rotateAxis, backward);
            right.Normalize();
            actualBackward = Vector3.Cross(right, axis);
            actualBackward.Normalize();
        }
        return new Matrix(
                right.X, right.Y, right.Z, 0.0f,
                axis.X, axis.Y, axis.Z, 0.0f,
                actualBackward.X, actualBackward.Y, actualBackward.Z, 0.0f,
                objectPosition.X, objectPosition.Y, objectPosition.Z, 1.0f);
    }

    public static Matrix CreateShadow(Vector3 lightDirection, Plane plane) {
        Plane normalized = Plane.Normalize(plane);
        float dot = (normalized.Normal.X * lightDirection.X)
                + (normalized.Normal.Y * lightDirection.Y)
                + (normalized.Normal.Z * lightDirection.Z);
        float x = -normalized.Normal.X;
        float y = -normalized.Normal.Y;
        float z = -normalized.Normal.Z;
        float d = -normalized.D;
        return new Matrix(
                (x * lightDirection.X) + dot, x * lightDirection.Y, x * lightDirection.Z, 0.0f,
                y * lightDirection.X, (y * lightDirection.Y) + dot, y * lightDirection.Z, 0.0f,
                z * lightDirection.X, z * lightDirection.Y, (z * lightDirection.Z) + dot, 0.0f,
                d * lightDirection.X, d * lightDirection.Y, d * lightDirection.Z, dot);
    }

    public static Matrix CreateReflection(Plane value) {
        Plane normalized = Plane.Normalize(value);
        float x = normalized.Normal.X;
        float y = normalized.Normal.Y;
        float z = normalized.Normal.Z;
        float x2 = -2.0f * x;
        float y2 = -2.0f * y;
        float z2 = -2.0f * z;
        return new Matrix(
                (x2 * x) + 1.0f, y2 * x, z2 * x, 0.0f,
                x2 * y, (y2 * y) + 1.0f, z2 * y, 0.0f,
                x2 * z, y2 * z, (z2 * z) + 1.0f, 0.0f,
                x2 * normalized.D, y2 * normalized.D, z2 * normalized.D, 1.0f);
    }

    public Decomposition Decompose() {
        Vector3 translation = new Vector3(M41, M42, M43);
        Vector3[] basis = {
            new Vector3(M11, M12, M13),
            new Vector3(M21, M22, M23),
            new Vector3(M31, M32, M33)
        };
        float[] scale = {basis[0].Length(), basis[1].Length(), basis[2].Length()};
        int largest;
        int middle;
        int smallest;
        if (scale[0] < scale[1]) {
            if (scale[1] < scale[2]) {
                largest = 2; middle = 1; smallest = 0;
            } else {
                largest = 1;
                if (scale[0] < scale[2]) { middle = 2; smallest = 0; }
                else { middle = 0; smallest = 2; }
            }
        } else if (scale[0] < scale[2]) {
            largest = 2; middle = 0; smallest = 1;
        } else {
            largest = 0;
            if (scale[1] < scale[2]) { middle = 2; smallest = 1; }
            else { middle = 1; smallest = 2; }
        }
        Vector3[] canonical = {Vector3.getUnitX(), Vector3.getUnitY(), Vector3.getUnitZ()};
        if (scale[largest] < 0.0001f) {
            basis[largest] = new Vector3(canonical[largest]);
        }
        basis[largest].Normalize();
        if (scale[middle] < 0.0001f) {
            float x = Math.abs(basis[largest].X);
            float y = Math.abs(basis[largest].Y);
            float z = Math.abs(basis[largest].Z);
            int leastAligned = x < y
                    ? (y < z ? 0 : x < z ? 0 : 2)
                    : (x < z ? 1 : y < z ? 1 : 2);
            canonical[leastAligned] = Vector3.Cross(basis[middle], basis[largest]);
        }
        basis[middle].Normalize();
        if (scale[smallest] < 0.0001f) {
            basis[middle] = Vector3.Cross(basis[smallest], basis[largest]);
        }
        basis[smallest].Normalize();
        Matrix rotationMatrix = matrixFromBasis(basis);
        float determinant = rotationMatrix.Determinant();
        if (determinant < 0.0f) {
            scale[largest] = -scale[largest];
            basis[largest] = new Vector3(
                    -basis[largest].X,
                    -basis[largest].Y,
                    -basis[largest].Z);
            determinant = -determinant;
            rotationMatrix = matrixFromBasis(basis);
        }
        determinant -= 1.0f;
        determinant *= determinant;
        boolean succeeded = !(0.0001f < determinant);
        Quaternion rotation = succeeded
                ? Quaternion.CreateFromRotationMatrix(rotationMatrix)
                : Quaternion.getIdentity();
        return new Decomposition(
                succeeded, new Vector3(scale[0], scale[1], scale[2]), rotation, translation);
    }

    private static Matrix matrixFromBasis(Vector3[] basis) {
        return new Matrix(
                basis[0].X, basis[0].Y, basis[0].Z, 0.0f,
                basis[1].X, basis[1].Y, basis[1].Z, 0.0f,
                basis[2].X, basis[2].Y, basis[2].Z, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    public static Matrix Transform(Matrix value, Quaternion rotation) {
        float x2 = rotation.X + rotation.X;
        float y2 = rotation.Y + rotation.Y;
        float z2 = rotation.Z + rotation.Z;
        float wx2 = rotation.W * x2;
        float wy2 = rotation.W * y2;
        float wz2 = rotation.W * z2;
        float xx2 = rotation.X * x2;
        float xy2 = rotation.X * y2;
        float xz2 = rotation.X * z2;
        float yy2 = rotation.Y * y2;
        float yz2 = rotation.Y * z2;
        float zz2 = rotation.Z * z2;
        float r11 = 1.0f - yy2 - zz2;
        float r12 = xy2 - wz2;
        float r13 = xz2 + wy2;
        float r21 = xy2 + wz2;
        float r22 = 1.0f - xx2 - zz2;
        float r23 = yz2 - wx2;
        float r31 = xz2 - wy2;
        float r32 = yz2 + wx2;
        float r33 = 1.0f - xx2 - yy2;
        return new Matrix(
                (value.M11 * r11) + (value.M12 * r12) + (value.M13 * r13),
                (value.M11 * r21) + (value.M12 * r22) + (value.M13 * r23),
                (value.M11 * r31) + (value.M12 * r32) + (value.M13 * r33), value.M14,
                (value.M21 * r11) + (value.M22 * r12) + (value.M23 * r13),
                (value.M21 * r21) + (value.M22 * r22) + (value.M23 * r23),
                (value.M21 * r31) + (value.M22 * r32) + (value.M23 * r33), value.M24,
                (value.M31 * r11) + (value.M32 * r12) + (value.M33 * r13),
                (value.M31 * r21) + (value.M32 * r22) + (value.M33 * r23),
                (value.M31 * r31) + (value.M32 * r32) + (value.M33 * r33), value.M34,
                (value.M41 * r11) + (value.M42 * r12) + (value.M43 * r13),
                (value.M41 * r21) + (value.M42 * r22) + (value.M43 * r23),
                (value.M41 * r31) + (value.M42 * r32) + (value.M43 * r33), value.M44);
    }

    public boolean equals(Matrix other) {
        return other != null
                && M11 == other.M11 && M22 == other.M22 && M33 == other.M33 && M44 == other.M44
                && M12 == other.M12 && M13 == other.M13 && M14 == other.M14
                && M21 == other.M21 && M23 == other.M23 && M24 == other.M24
                && M31 == other.M31 && M32 == other.M32 && M34 == other.M34
                && M41 == other.M41 && M42 == other.M42 && M43 == other.M43;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Matrix value && equals(value);
    }

    @Override
    public int hashCode() {
        return FloatSemantics.hash(M11) + FloatSemantics.hash(M12)
                + FloatSemantics.hash(M13) + FloatSemantics.hash(M14)
                + FloatSemantics.hash(M21) + FloatSemantics.hash(M22)
                + FloatSemantics.hash(M23) + FloatSemantics.hash(M24)
                + FloatSemantics.hash(M31) + FloatSemantics.hash(M32)
                + FloatSemantics.hash(M33) + FloatSemantics.hash(M34)
                + FloatSemantics.hash(M41) + FloatSemantics.hash(M42)
                + FloatSemantics.hash(M43) + FloatSemantics.hash(M44);
    }

    @Override
    public String toString() {
        return "{ {M11:" + M11 + " M12:" + M12 + " M13:" + M13 + " M14:" + M14 + "} "
                + "{M21:" + M21 + " M22:" + M22 + " M23:" + M23 + " M24:" + M24 + "} "
                + "{M31:" + M31 + " M32:" + M32 + " M33:" + M33 + " M34:" + M34 + "} "
                + "{M41:" + M41 + " M42:" + M42 + " M43:" + M43 + " M44:" + M44 + "} }";
    }

    /** Immutable Java carrier for XNA's multi-output {@code Decompose} operation. */
    public static final class Decomposition {
        private final boolean succeeded;
        private final Vector3 scale;
        private final Quaternion rotation;
        private final Vector3 translation;

        private Decomposition(
                boolean succeeded, Vector3 scale, Quaternion rotation, Vector3 translation) {
            this.succeeded = succeeded;
            this.scale = new Vector3(scale);
            this.rotation = new Quaternion(rotation);
            this.translation = new Vector3(translation);
        }

        public boolean getSucceeded() { return succeeded; }
        public Vector3 getScale() { return new Vector3(scale); }
        public Quaternion getRotation() { return new Quaternion(rotation); }
        public Vector3 getTranslation() { return new Vector3(translation); }
    }
}
