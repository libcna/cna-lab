package Microsoft.Xna.Framework;

import java.util.Objects;

/** Mutable three-dimensional XNA value. */
public final class Vector3 {

    public float X;
    public float Y;
    public float Z;

    public Vector3() {
    }

    public Vector3(float value) { this(value, value, value); }
    public Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }
    public Vector3(Vector2 value, float z) { this(Objects.requireNonNull(value, "value").X, value.Y, z); }
    public Vector3(Vector3 value) { this(Objects.requireNonNull(value, "value").X, value.Y, value.Z); }

    public static Vector3 getZero() { return new Vector3(); }
    public static Vector3 getOne() { return new Vector3(1.0f); }
    public static Vector3 getUnitX() { return new Vector3(1.0f, 0.0f, 0.0f); }
    public static Vector3 getUnitY() { return new Vector3(0.0f, 1.0f, 0.0f); }
    public static Vector3 getUnitZ() { return new Vector3(0.0f, 0.0f, 1.0f); }
    public static Vector3 getUp() { return new Vector3(0.0f, 1.0f, 0.0f); }
    public static Vector3 getDown() { return new Vector3(0.0f, -1.0f, 0.0f); }
    public static Vector3 getRight() { return new Vector3(1.0f, 0.0f, 0.0f); }
    public static Vector3 getLeft() { return new Vector3(-1.0f, 0.0f, 0.0f); }
    public static Vector3 getForward() { return new Vector3(0.0f, 0.0f, -1.0f); }
    public static Vector3 getBackward() { return new Vector3(0.0f, 0.0f, 1.0f); }

    public float Length() { return (float)Math.sqrt((X * X) + (Y * Y) + (Z * Z)); }
    public float LengthSquared() { return (X * X) + (Y * Y) + (Z * Z); }

    public void Normalize() {
        float factor = 1.0f / (float)Math.sqrt((X * X) + (Y * Y) + (Z * Z));
        X *= factor;
        Y *= factor;
        Z *= factor;
    }

    public static Vector3 Normalize(Vector3 value) {
        float factor = 1.0f / (float)Math.sqrt(
                (value.X * value.X) + (value.Y * value.Y) + (value.Z * value.Z));
        return new Vector3(value.X * factor, value.Y * factor, value.Z * factor);
    }

    public static Vector3 Add(Vector3 value1, Vector3 value2) {
        return new Vector3(value1.X + value2.X, value1.Y + value2.Y, value1.Z + value2.Z);
    }

    public static Vector3 Subtract(Vector3 value1, Vector3 value2) {
        return new Vector3(value1.X - value2.X, value1.Y - value2.Y, value1.Z - value2.Z);
    }

    public static Vector3 Multiply(Vector3 value1, Vector3 value2) {
        return new Vector3(value1.X * value2.X, value1.Y * value2.Y, value1.Z * value2.Z);
    }

    public static Vector3 Multiply(Vector3 value, float scaleFactor) {
        return new Vector3(value.X * scaleFactor, value.Y * scaleFactor, value.Z * scaleFactor);
    }

    public static Vector3 Multiply(float scaleFactor, Vector3 value) {
        return new Vector3(value.X * scaleFactor, value.Y * scaleFactor, value.Z * scaleFactor);
    }

    public static Vector3 Divide(Vector3 value1, Vector3 value2) {
        return new Vector3(value1.X / value2.X, value1.Y / value2.Y, value1.Z / value2.Z);
    }

    public static Vector3 Divide(Vector3 value, float divider) {
        float factor = 1.0f / divider;
        return new Vector3(value.X * factor, value.Y * factor, value.Z * factor);
    }

    public static Vector3 Negate(Vector3 value) {
        return new Vector3(-value.X, -value.Y, -value.Z);
    }

    public static float Dot(Vector3 vector1, Vector3 vector2) {
        return (vector1.X * vector2.X) + (vector1.Y * vector2.Y) + (vector1.Z * vector2.Z);
    }

    public static Vector3 Cross(Vector3 vector1, Vector3 vector2) {
        return new Vector3(
                (vector1.Y * vector2.Z) - (vector1.Z * vector2.Y),
                (vector1.Z * vector2.X) - (vector1.X * vector2.Z),
                (vector1.X * vector2.Y) - (vector1.Y * vector2.X));
    }

    public static Vector3 Reflect(Vector3 vector, Vector3 normal) {
        float dot = (vector.X * normal.X) + (vector.Y * normal.Y) + (vector.Z * normal.Z);
        return new Vector3(
                vector.X - (2.0f * dot * normal.X),
                vector.Y - (2.0f * dot * normal.Y),
                vector.Z - (2.0f * dot * normal.Z));
    }

    public static float Distance(Vector3 value1, Vector3 value2) {
        float x = value1.X - value2.X;
        float y = value1.Y - value2.Y;
        float z = value1.Z - value2.Z;
        return (float)Math.sqrt((x * x) + (y * y) + (z * z));
    }

    public static float DistanceSquared(Vector3 value1, Vector3 value2) {
        float x = value1.X - value2.X;
        float y = value1.Y - value2.Y;
        float z = value1.Z - value2.Z;
        return (x * x) + (y * y) + (z * z);
    }

    public static Vector3 Min(Vector3 value1, Vector3 value2) {
        return new Vector3(
                value1.X < value2.X ? value1.X : value2.X,
                value1.Y < value2.Y ? value1.Y : value2.Y,
                value1.Z < value2.Z ? value1.Z : value2.Z);
    }

    public static Vector3 Max(Vector3 value1, Vector3 value2) {
        return new Vector3(
                value1.X > value2.X ? value1.X : value2.X,
                value1.Y > value2.Y ? value1.Y : value2.Y,
                value1.Z > value2.Z ? value1.Z : value2.Z);
    }

    public static Vector3 Clamp(Vector3 value1, Vector3 min, Vector3 max) {
        float x = value1.X;
        x = x > max.X ? max.X : x;
        x = x < min.X ? min.X : x;
        float y = value1.Y;
        y = y > max.Y ? max.Y : y;
        y = y < min.Y ? min.Y : y;
        float z = value1.Z;
        z = z > max.Z ? max.Z : z;
        z = z < min.Z ? min.Z : z;
        return new Vector3(x, y, z);
    }

    public static Vector3 Lerp(Vector3 value1, Vector3 value2, float amount) {
        return new Vector3(
                value1.X + ((value2.X - value1.X) * amount),
                value1.Y + ((value2.Y - value1.Y) * amount),
                value1.Z + ((value2.Z - value1.Z) * amount));
    }

    public static Vector3 Barycentric(
            Vector3 value1, Vector3 value2, Vector3 value3, float amount1, float amount2) {
        return new Vector3(
                value1.X + (amount1 * (value2.X - value1.X)) + (amount2 * (value3.X - value1.X)),
                value1.Y + (amount1 * (value2.Y - value1.Y)) + (amount2 * (value3.Y - value1.Y)),
                value1.Z + (amount1 * (value2.Z - value1.Z)) + (amount2 * (value3.Z - value1.Z)));
    }

    public static Vector3 SmoothStep(Vector3 value1, Vector3 value2, float amount) {
        amount = amount > 1.0f ? 1.0f : amount < 0.0f ? 0.0f : amount;
        amount = amount * amount * (3.0f - (2.0f * amount));
        return new Vector3(
                value1.X + ((value2.X - value1.X) * amount),
                value1.Y + ((value2.Y - value1.Y) * amount),
                value1.Z + ((value2.Z - value1.Z) * amount));
    }

    public static Vector3 CatmullRom(
            Vector3 value1, Vector3 value2, Vector3 value3, Vector3 value4, float amount) {
        float squared = amount * amount;
        float cubed = amount * squared;
        return new Vector3(
                catmull(value1.X, value2.X, value3.X, value4.X, amount, squared, cubed),
                catmull(value1.Y, value2.Y, value3.Y, value4.Y, amount, squared, cubed),
                catmull(value1.Z, value2.Z, value3.Z, value4.Z, amount, squared, cubed));
    }

    private static float catmull(
            float value1, float value2, float value3, float value4,
            float amount, float squared, float cubed) {
        return 0.5f * ((2.0f * value2) + ((0.0f - value1 + value3) * amount)
                + (((2.0f * value1) - (5.0f * value2) + (4.0f * value3) - value4) * squared)
                + (((0.0f - value1) + (3.0f * value2) - (3.0f * value3) + value4) * cubed));
    }

    public static Vector3 Hermite(
            Vector3 value1, Vector3 tangent1, Vector3 value2, Vector3 tangent2, float amount) {
        float squared = amount * amount;
        float cubed = amount * squared;
        float first = (2.0f * cubed) - (3.0f * squared) + 1.0f;
        float second = (-2.0f * cubed) + (3.0f * squared);
        float third = cubed - (2.0f * squared) + amount;
        float fourth = cubed - squared;
        return new Vector3(
                hermite(value1.X, tangent1.X, value2.X, tangent2.X, first, second, third, fourth),
                hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, first, second, third, fourth),
                hermite(value1.Z, tangent1.Z, value2.Z, tangent2.Z, first, second, third, fourth));
    }

    private static float hermite(
            float value1, float tangent1, float value2, float tangent2,
            float first, float second, float third, float fourth) {
        return (value1 * first) + (value2 * second) + (tangent1 * third) + (tangent2 * fourth);
    }

    public static Vector3 Transform(Vector3 position, Matrix matrix) {
        return new Vector3(
                (position.X * matrix.M11) + (position.Y * matrix.M21) + (position.Z * matrix.M31) + matrix.M41,
                (position.X * matrix.M12) + (position.Y * matrix.M22) + (position.Z * matrix.M32) + matrix.M42,
                (position.X * matrix.M13) + (position.Y * matrix.M23) + (position.Z * matrix.M33) + matrix.M43);
    }

    public static Vector3 TransformNormal(Vector3 normal, Matrix matrix) {
        return new Vector3(
                (normal.X * matrix.M11) + (normal.Y * matrix.M21) + (normal.Z * matrix.M31),
                (normal.X * matrix.M12) + (normal.Y * matrix.M22) + (normal.Z * matrix.M32),
                (normal.X * matrix.M13) + (normal.Y * matrix.M23) + (normal.Z * matrix.M33));
    }

    public static Vector3 Transform(Vector3 value, Quaternion rotation) {
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
        return new Vector3(
                (value.X * (1.0f - yy2 - zz2)) + (value.Y * (xy2 - wz2)) + (value.Z * (xz2 + wy2)),
                (value.X * (xy2 + wz2)) + (value.Y * (1.0f - xx2 - zz2)) + (value.Z * (yz2 - wx2)),
                (value.X * (xz2 - wy2)) + (value.Y * (yz2 + wx2)) + (value.Z * (1.0f - xx2 - yy2)));
    }

    public static void Transform(Vector3[] sourceArray, Matrix matrix, Vector3[] destinationArray) {
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
            Vector3[] sourceArray, int sourceIndex, Matrix matrix,
            Vector3[] destinationArray, int destinationIndex, int length) {
        checkTransformRange(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
        while (length > 0) {
            destinationArray[destinationIndex] = Transform(sourceArray[sourceIndex], matrix);
            sourceIndex++;
            destinationIndex++;
            length--;
        }
    }

    public static void Transform(Vector3[] sourceArray, Quaternion rotation, Vector3[] destinationArray) {
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
            Vector3[] sourceArray, int sourceIndex, Quaternion rotation,
            Vector3[] destinationArray, int destinationIndex, int length) {
        checkTransformRange(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
        while (length > 0) {
            destinationArray[destinationIndex] = Transform(sourceArray[sourceIndex], rotation);
            sourceIndex++;
            destinationIndex++;
            length--;
        }
    }

    public static void TransformNormal(Vector3[] sourceArray, Matrix matrix, Vector3[] destinationArray) {
        Objects.requireNonNull(sourceArray, "sourceArray");
        Objects.requireNonNull(destinationArray, "destinationArray");
        if (destinationArray.length < sourceArray.length) {
            throw new IllegalArgumentException("The destination array is too small.");
        }
        for (int i = 0; i < sourceArray.length; i++) {
            destinationArray[i] = TransformNormal(sourceArray[i], matrix);
        }
    }

    public static void TransformNormal(
            Vector3[] sourceArray, int sourceIndex, Matrix matrix,
            Vector3[] destinationArray, int destinationIndex, int length) {
        checkTransformRange(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
        while (length > 0) {
            destinationArray[destinationIndex] = TransformNormal(sourceArray[sourceIndex], matrix);
            sourceIndex++;
            destinationIndex++;
            length--;
        }
    }

    private static void checkTransformRange(
            Vector3[] sourceArray, int sourceIndex,
            Vector3[] destinationArray, int destinationIndex, int length) {
        Objects.requireNonNull(sourceArray, "sourceArray");
        Objects.requireNonNull(destinationArray, "destinationArray");
        if (sourceArray.length < ((long)sourceIndex + length)) {
            throw new IllegalArgumentException("The source array is too small.");
        }
        if (destinationArray.length < ((long)destinationIndex + length)) {
            throw new IllegalArgumentException("The destination array is too small.");
        }
    }

    public boolean equals(Vector3 other) {
        return other != null && X == other.X && Y == other.Y && Z == other.Z;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Vector3 value && equals(value);
    }

    @Override
    public int hashCode() {
        return FloatSemantics.hash(X) + FloatSemantics.hash(Y) + FloatSemantics.hash(Z);
    }

    @Override
    public String toString() {
        return "{X:" + X + " Y:" + Y + " Z:" + Z + '}';
    }
}
