package Microsoft.Xna.Framework;

import java.util.Objects;

/** Mutable four-dimensional XNA value. */
public final class Vector4 {

    public float X;
    public float Y;
    public float Z;
    public float W;

    public Vector4() {
    }

    public Vector4(float value) { this(value, value, value, value); }
    public Vector4(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; }
    public Vector4(Vector2 value, float z, float w) {
        this(Objects.requireNonNull(value, "value").X, value.Y, z, w);
    }
    public Vector4(Vector3 value, float w) {
        this(Objects.requireNonNull(value, "value").X, value.Y, value.Z, w);
    }
    public Vector4(Vector4 value) {
        this(Objects.requireNonNull(value, "value").X, value.Y, value.Z, value.W);
    }

    public static Vector4 getZero() { return new Vector4(); }
    public static Vector4 getOne() { return new Vector4(1.0f); }
    public static Vector4 getUnitX() { return new Vector4(1.0f, 0.0f, 0.0f, 0.0f); }
    public static Vector4 getUnitY() { return new Vector4(0.0f, 1.0f, 0.0f, 0.0f); }
    public static Vector4 getUnitZ() { return new Vector4(0.0f, 0.0f, 1.0f, 0.0f); }
    public static Vector4 getUnitW() { return new Vector4(0.0f, 0.0f, 0.0f, 1.0f); }

    public float Length() {
        return (float)Math.sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
    }

    public float LengthSquared() {
        return (X * X) + (Y * Y) + (Z * Z) + (W * W);
    }

    public void Normalize() {
        float factor = 1.0f / (float)Math.sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
        X *= factor;
        Y *= factor;
        Z *= factor;
        W *= factor;
    }

    public static Vector4 Normalize(Vector4 vector) {
        float factor = 1.0f / (float)Math.sqrt(
                (vector.X * vector.X) + (vector.Y * vector.Y)
                        + (vector.Z * vector.Z) + (vector.W * vector.W));
        return new Vector4(vector.X * factor, vector.Y * factor, vector.Z * factor, vector.W * factor);
    }

    public static Vector4 Add(Vector4 value1, Vector4 value2) {
        return new Vector4(
                value1.X + value2.X, value1.Y + value2.Y,
                value1.Z + value2.Z, value1.W + value2.W);
    }

    public static Vector4 Subtract(Vector4 value1, Vector4 value2) {
        return new Vector4(
                value1.X - value2.X, value1.Y - value2.Y,
                value1.Z - value2.Z, value1.W - value2.W);
    }

    public static Vector4 Multiply(Vector4 value1, Vector4 value2) {
        return new Vector4(
                value1.X * value2.X, value1.Y * value2.Y,
                value1.Z * value2.Z, value1.W * value2.W);
    }

    public static Vector4 Multiply(Vector4 value1, float scaleFactor) {
        return new Vector4(
                value1.X * scaleFactor, value1.Y * scaleFactor,
                value1.Z * scaleFactor, value1.W * scaleFactor);
    }

    public static Vector4 Multiply(float scaleFactor, Vector4 value1) {
        return new Vector4(
                value1.X * scaleFactor, value1.Y * scaleFactor,
                value1.Z * scaleFactor, value1.W * scaleFactor);
    }

    public static Vector4 Divide(Vector4 value1, Vector4 value2) {
        return new Vector4(
                value1.X / value2.X, value1.Y / value2.Y,
                value1.Z / value2.Z, value1.W / value2.W);
    }

    public static Vector4 Divide(Vector4 value1, float divider) {
        float factor = 1.0f / divider;
        return new Vector4(
                value1.X * factor, value1.Y * factor,
                value1.Z * factor, value1.W * factor);
    }

    public static Vector4 Negate(Vector4 value) {
        return new Vector4(-value.X, -value.Y, -value.Z, -value.W);
    }

    public static float Dot(Vector4 vector1, Vector4 vector2) {
        return (vector1.X * vector2.X) + (vector1.Y * vector2.Y)
                + (vector1.Z * vector2.Z) + (vector1.W * vector2.W);
    }

    public static float Distance(Vector4 value1, Vector4 value2) {
        float x = value1.X - value2.X;
        float y = value1.Y - value2.Y;
        float z = value1.Z - value2.Z;
        float w = value1.W - value2.W;
        return (float)Math.sqrt((x * x) + (y * y) + (z * z) + (w * w));
    }

    public static float DistanceSquared(Vector4 value1, Vector4 value2) {
        float x = value1.X - value2.X;
        float y = value1.Y - value2.Y;
        float z = value1.Z - value2.Z;
        float w = value1.W - value2.W;
        return (x * x) + (y * y) + (z * z) + (w * w);
    }

    public static Vector4 Min(Vector4 value1, Vector4 value2) {
        return new Vector4(
                value1.X < value2.X ? value1.X : value2.X,
                value1.Y < value2.Y ? value1.Y : value2.Y,
                value1.Z < value2.Z ? value1.Z : value2.Z,
                value1.W < value2.W ? value1.W : value2.W);
    }

    public static Vector4 Max(Vector4 value1, Vector4 value2) {
        return new Vector4(
                value1.X > value2.X ? value1.X : value2.X,
                value1.Y > value2.Y ? value1.Y : value2.Y,
                value1.Z > value2.Z ? value1.Z : value2.Z,
                value1.W > value2.W ? value1.W : value2.W);
    }

    public static Vector4 Clamp(Vector4 value1, Vector4 min, Vector4 max) {
        float x = value1.X;
        x = x > max.X ? max.X : x;
        x = x < min.X ? min.X : x;
        float y = value1.Y;
        y = y > max.Y ? max.Y : y;
        y = y < min.Y ? min.Y : y;
        float z = value1.Z;
        z = z > max.Z ? max.Z : z;
        z = z < min.Z ? min.Z : z;
        float w = value1.W;
        w = w > max.W ? max.W : w;
        w = w < min.W ? min.W : w;
        return new Vector4(x, y, z, w);
    }

    public static Vector4 Lerp(Vector4 value1, Vector4 value2, float amount) {
        return new Vector4(
                value1.X + ((value2.X - value1.X) * amount),
                value1.Y + ((value2.Y - value1.Y) * amount),
                value1.Z + ((value2.Z - value1.Z) * amount),
                value1.W + ((value2.W - value1.W) * amount));
    }

    public static Vector4 Barycentric(
            Vector4 value1, Vector4 value2, Vector4 value3, float amount1, float amount2) {
        return new Vector4(
                value1.X + (amount1 * (value2.X - value1.X)) + (amount2 * (value3.X - value1.X)),
                value1.Y + (amount1 * (value2.Y - value1.Y)) + (amount2 * (value3.Y - value1.Y)),
                value1.Z + (amount1 * (value2.Z - value1.Z)) + (amount2 * (value3.Z - value1.Z)),
                value1.W + (amount1 * (value2.W - value1.W)) + (amount2 * (value3.W - value1.W)));
    }

    public static Vector4 SmoothStep(Vector4 value1, Vector4 value2, float amount) {
        amount = amount > 1.0f ? 1.0f : amount < 0.0f ? 0.0f : amount;
        amount = amount * amount * (3.0f - (2.0f * amount));
        return new Vector4(
                value1.X + ((value2.X - value1.X) * amount),
                value1.Y + ((value2.Y - value1.Y) * amount),
                value1.Z + ((value2.Z - value1.Z) * amount),
                value1.W + ((value2.W - value1.W) * amount));
    }

    public static Vector4 CatmullRom(
            Vector4 value1, Vector4 value2, Vector4 value3, Vector4 value4, float amount) {
        float squared = amount * amount;
        float cubed = amount * squared;
        return new Vector4(
                catmull(value1.X, value2.X, value3.X, value4.X, amount, squared, cubed),
                catmull(value1.Y, value2.Y, value3.Y, value4.Y, amount, squared, cubed),
                catmull(value1.Z, value2.Z, value3.Z, value4.Z, amount, squared, cubed),
                catmull(value1.W, value2.W, value3.W, value4.W, amount, squared, cubed));
    }

    private static float catmull(
            float value1, float value2, float value3, float value4,
            float amount, float squared, float cubed) {
        return 0.5f * ((2.0f * value2) + ((0.0f - value1 + value3) * amount)
                + (((2.0f * value1) - (5.0f * value2) + (4.0f * value3) - value4) * squared)
                + (((0.0f - value1) + (3.0f * value2) - (3.0f * value3) + value4) * cubed));
    }

    public static Vector4 Hermite(
            Vector4 value1, Vector4 tangent1, Vector4 value2, Vector4 tangent2, float amount) {
        float squared = amount * amount;
        float cubed = amount * squared;
        float first = (2.0f * cubed) - (3.0f * squared) + 1.0f;
        float second = (-2.0f * cubed) + (3.0f * squared);
        float third = cubed - (2.0f * squared) + amount;
        float fourth = cubed - squared;
        return new Vector4(
                hermite(value1.X, tangent1.X, value2.X, tangent2.X, first, second, third, fourth),
                hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, first, second, third, fourth),
                hermite(value1.Z, tangent1.Z, value2.Z, tangent2.Z, first, second, third, fourth),
                hermite(value1.W, tangent1.W, value2.W, tangent2.W, first, second, third, fourth));
    }

    private static float hermite(
            float value1, float tangent1, float value2, float tangent2,
            float first, float second, float third, float fourth) {
        return (value1 * first) + (value2 * second) + (tangent1 * third) + (tangent2 * fourth);
    }

    public static Vector4 Transform(Vector2 position, Matrix matrix) {
        return new Vector4(
                (position.X * matrix.M11) + (position.Y * matrix.M21) + matrix.M41,
                (position.X * matrix.M12) + (position.Y * matrix.M22) + matrix.M42,
                (position.X * matrix.M13) + (position.Y * matrix.M23) + matrix.M43,
                (position.X * matrix.M14) + (position.Y * matrix.M24) + matrix.M44);
    }

    public static Vector4 Transform(Vector3 position, Matrix matrix) {
        return new Vector4(
                (position.X * matrix.M11) + (position.Y * matrix.M21) + (position.Z * matrix.M31) + matrix.M41,
                (position.X * matrix.M12) + (position.Y * matrix.M22) + (position.Z * matrix.M32) + matrix.M42,
                (position.X * matrix.M13) + (position.Y * matrix.M23) + (position.Z * matrix.M33) + matrix.M43,
                (position.X * matrix.M14) + (position.Y * matrix.M24) + (position.Z * matrix.M34) + matrix.M44);
    }

    public static Vector4 Transform(Vector4 vector, Matrix matrix) {
        return new Vector4(
                (vector.X * matrix.M11) + (vector.Y * matrix.M21) + (vector.Z * matrix.M31) + (vector.W * matrix.M41),
                (vector.X * matrix.M12) + (vector.Y * matrix.M22) + (vector.Z * matrix.M32) + (vector.W * matrix.M42),
                (vector.X * matrix.M13) + (vector.Y * matrix.M23) + (vector.Z * matrix.M33) + (vector.W * matrix.M43),
                (vector.X * matrix.M14) + (vector.Y * matrix.M24) + (vector.Z * matrix.M34) + (vector.W * matrix.M44));
    }

    public static Vector4 Transform(Vector2 value, Quaternion rotation) {
        float[] terms = quaternionTerms(rotation);
        return new Vector4(
                (value.X * (1.0f - terms[9] - terms[11])) + (value.Y * (terms[7] - terms[5])),
                (value.X * (terms[7] + terms[5])) + (value.Y * (1.0f - terms[6] - terms[11])),
                (value.X * (terms[8] - terms[4])) + (value.Y * (terms[10] + terms[3])),
                1.0f);
    }

    public static Vector4 Transform(Vector3 value, Quaternion rotation) {
        float[] terms = quaternionTerms(rotation);
        return new Vector4(
                (value.X * (1.0f - terms[9] - terms[11])) + (value.Y * (terms[7] - terms[5])) + (value.Z * (terms[8] + terms[4])),
                (value.X * (terms[7] + terms[5])) + (value.Y * (1.0f - terms[6] - terms[11])) + (value.Z * (terms[10] - terms[3])),
                (value.X * (terms[8] - terms[4])) + (value.Y * (terms[10] + terms[3])) + (value.Z * (1.0f - terms[6] - terms[9])),
                1.0f);
    }

    public static Vector4 Transform(Vector4 value, Quaternion rotation) {
        float[] terms = quaternionTerms(rotation);
        return new Vector4(
                (value.X * (1.0f - terms[9] - terms[11])) + (value.Y * (terms[7] - terms[5])) + (value.Z * (terms[8] + terms[4])),
                (value.X * (terms[7] + terms[5])) + (value.Y * (1.0f - terms[6] - terms[11])) + (value.Z * (terms[10] - terms[3])),
                (value.X * (terms[8] - terms[4])) + (value.Y * (terms[10] + terms[3])) + (value.Z * (1.0f - terms[6] - terms[9])),
                value.W);
    }

    private static float[] quaternionTerms(Quaternion rotation) {
        float x2 = rotation.X + rotation.X;
        float y2 = rotation.Y + rotation.Y;
        float z2 = rotation.Z + rotation.Z;
        return new float[] {
                x2, y2, z2,
                rotation.W * x2, rotation.W * y2, rotation.W * z2,
                rotation.X * x2, rotation.X * y2, rotation.X * z2,
                rotation.Y * y2, rotation.Y * z2, rotation.Z * z2
        };
    }

    public static void Transform(Vector4[] sourceArray, Matrix matrix, Vector4[] destinationArray) {
        Objects.requireNonNull(sourceArray, "sourceArray");
        Objects.requireNonNull(destinationArray, "destinationArray");
        if (destinationArray.length < sourceArray.length) {
            throw new IllegalArgumentException("The destination array is too small.");
        }
        for (int i = 0; i < sourceArray.length; i++) {
            destinationArray[i] = Transform(sourceArray[i], matrix);
        }
    }

    public static void Transform(
            Vector4[] sourceArray, int sourceIndex, Matrix matrix,
            Vector4[] destinationArray, int destinationIndex, int length) {
        checkTransformRange(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
        while (length > 0) {
            destinationArray[destinationIndex] = Transform(sourceArray[sourceIndex], matrix);
            sourceIndex++;
            destinationIndex++;
            length--;
        }
    }

    public static void Transform(Vector4[] sourceArray, Quaternion rotation, Vector4[] destinationArray) {
        Objects.requireNonNull(sourceArray, "sourceArray");
        Objects.requireNonNull(destinationArray, "destinationArray");
        if (destinationArray.length < sourceArray.length) {
            throw new IllegalArgumentException("The destination array is too small.");
        }
        for (int i = 0; i < sourceArray.length; i++) {
            destinationArray[i] = Transform(sourceArray[i], rotation);
        }
    }

    public static void Transform(
            Vector4[] sourceArray, int sourceIndex, Quaternion rotation,
            Vector4[] destinationArray, int destinationIndex, int length) {
        checkTransformRange(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
        while (length > 0) {
            destinationArray[destinationIndex] = Transform(sourceArray[sourceIndex], rotation);
            sourceIndex++;
            destinationIndex++;
            length--;
        }
    }

    private static void checkTransformRange(
            Vector4[] sourceArray, int sourceIndex,
            Vector4[] destinationArray, int destinationIndex, int length) {
        Objects.requireNonNull(sourceArray, "sourceArray");
        Objects.requireNonNull(destinationArray, "destinationArray");
        if (sourceArray.length < ((long)sourceIndex + length)) {
            throw new IllegalArgumentException("The source array is too small.");
        }
        if (destinationArray.length < ((long)destinationIndex + length)) {
            throw new IllegalArgumentException("The destination array is too small.");
        }
    }

    public boolean equals(Vector4 other) {
        return other != null && X == other.X && Y == other.Y && Z == other.Z && W == other.W;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Vector4 value && equals(value);
    }

    @Override
    public int hashCode() {
        return FloatSemantics.hash(X) + FloatSemantics.hash(Y)
                + FloatSemantics.hash(Z) + FloatSemantics.hash(W);
    }

    @Override
    public String toString() {
        return "{X:" + X + " Y:" + Y + " Z:" + Z + " W:" + W + '}';
    }
}
