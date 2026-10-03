package Microsoft.Xna.Framework;

import java.util.Objects;

/** Mutable two-dimensional XNA value; Java assignments alias, so APIs snapshot retained values. */
public final class Vector2 {

    public float X;
    public float Y;

    public Vector2() {
    }

    public Vector2(float value) {
        this(value, value);
    }

    public Vector2(float x, float y) {
        X = x;
        Y = y;
    }

    public Vector2(Vector2 value) {
        this(Objects.requireNonNull(value, "value").X, value.Y);
    }

    public static Vector2 getZero() { return new Vector2(); }
    public static Vector2 getOne() { return new Vector2(1.0f); }
    public static Vector2 getUnitX() { return new Vector2(1.0f, 0.0f); }
    public static Vector2 getUnitY() { return new Vector2(0.0f, 1.0f); }

    public float Length() {
        return (float)Math.sqrt((X * X) + (Y * Y));
    }

    public float LengthSquared() {
        return (X * X) + (Y * Y);
    }

    public void Normalize() {
        float factor = 1.0f / (float)Math.sqrt((X * X) + (Y * Y));
        X *= factor;
        Y *= factor;
    }

    public static Vector2 Normalize(Vector2 value) {
        float factor = 1.0f / (float)Math.sqrt((value.X * value.X) + (value.Y * value.Y));
        return new Vector2(value.X * factor, value.Y * factor);
    }

    public static Vector2 Add(Vector2 value1, Vector2 value2) {
        return new Vector2(value1.X + value2.X, value1.Y + value2.Y);
    }

    public static Vector2 Subtract(Vector2 value1, Vector2 value2) {
        return new Vector2(value1.X - value2.X, value1.Y - value2.Y);
    }

    public static Vector2 Multiply(Vector2 value1, Vector2 value2) {
        return new Vector2(value1.X * value2.X, value1.Y * value2.Y);
    }

    public static Vector2 Multiply(Vector2 value, float scaleFactor) {
        return new Vector2(value.X * scaleFactor, value.Y * scaleFactor);
    }

    public static Vector2 Multiply(float scaleFactor, Vector2 value) {
        return new Vector2(value.X * scaleFactor, value.Y * scaleFactor);
    }

    public static Vector2 Divide(Vector2 value1, Vector2 value2) {
        return new Vector2(value1.X / value2.X, value1.Y / value2.Y);
    }

    public static Vector2 Divide(Vector2 value1, float divider) {
        float factor = 1.0f / divider;
        return new Vector2(value1.X * factor, value1.Y * factor);
    }

    public static Vector2 Negate(Vector2 value) {
        return new Vector2(-value.X, -value.Y);
    }

    public static float Distance(Vector2 value1, Vector2 value2) {
        float x = value1.X - value2.X;
        float y = value1.Y - value2.Y;
        return (float)Math.sqrt((x * x) + (y * y));
    }

    public static float DistanceSquared(Vector2 value1, Vector2 value2) {
        float x = value1.X - value2.X;
        float y = value1.Y - value2.Y;
        return (x * x) + (y * y);
    }

    public static float Dot(Vector2 value1, Vector2 value2) {
        return (value1.X * value2.X) + (value1.Y * value2.Y);
    }

    public static Vector2 Reflect(Vector2 vector, Vector2 normal) {
        float dot = (vector.X * normal.X) + (vector.Y * normal.Y);
        return new Vector2(
                vector.X - (2.0f * dot * normal.X),
                vector.Y - (2.0f * dot * normal.Y));
    }

    public static Vector2 Min(Vector2 value1, Vector2 value2) {
        return new Vector2(
                value1.X < value2.X ? value1.X : value2.X,
                value1.Y < value2.Y ? value1.Y : value2.Y);
    }

    public static Vector2 Max(Vector2 value1, Vector2 value2) {
        return new Vector2(
                value1.X > value2.X ? value1.X : value2.X,
                value1.Y > value2.Y ? value1.Y : value2.Y);
    }

    public static Vector2 Clamp(Vector2 value1, Vector2 min, Vector2 max) {
        float x = value1.X;
        x = x > max.X ? max.X : x;
        x = x < min.X ? min.X : x;
        float y = value1.Y;
        y = y > max.Y ? max.Y : y;
        y = y < min.Y ? min.Y : y;
        return new Vector2(x, y);
    }

    public static Vector2 Lerp(Vector2 value1, Vector2 value2, float amount) {
        return new Vector2(
                value1.X + ((value2.X - value1.X) * amount),
                value1.Y + ((value2.Y - value1.Y) * amount));
    }

    public static Vector2 Barycentric(
            Vector2 value1, Vector2 value2, Vector2 value3, float amount1, float amount2) {
        return new Vector2(
                value1.X + (amount1 * (value2.X - value1.X)) + (amount2 * (value3.X - value1.X)),
                value1.Y + (amount1 * (value2.Y - value1.Y)) + (amount2 * (value3.Y - value1.Y)));
    }

    public static Vector2 SmoothStep(Vector2 value1, Vector2 value2, float amount) {
        amount = amount > 1.0f ? 1.0f : amount < 0.0f ? 0.0f : amount;
        amount = amount * amount * (3.0f - (2.0f * amount));
        return new Vector2(
                value1.X + ((value2.X - value1.X) * amount),
                value1.Y + ((value2.Y - value1.Y) * amount));
    }

    public static Vector2 CatmullRom(
            Vector2 value1, Vector2 value2, Vector2 value3, Vector2 value4, float amount) {
        float squared = amount * amount;
        float cubed = amount * squared;
        return new Vector2(
                0.5f * ((2.0f * value2.X) + ((0.0f - value1.X + value3.X) * amount)
                        + (((2.0f * value1.X) - (5.0f * value2.X) + (4.0f * value3.X) - value4.X) * squared)
                        + (((0.0f - value1.X) + (3.0f * value2.X) - (3.0f * value3.X) + value4.X) * cubed)),
                0.5f * ((2.0f * value2.Y) + ((0.0f - value1.Y + value3.Y) * amount)
                        + (((2.0f * value1.Y) - (5.0f * value2.Y) + (4.0f * value3.Y) - value4.Y) * squared)
                        + (((0.0f - value1.Y) + (3.0f * value2.Y) - (3.0f * value3.Y) + value4.Y) * cubed)));
    }

    public static Vector2 Hermite(
            Vector2 value1, Vector2 tangent1, Vector2 value2, Vector2 tangent2, float amount) {
        float squared = amount * amount;
        float cubed = amount * squared;
        float first = (2.0f * cubed) - (3.0f * squared) + 1.0f;
        float second = (-2.0f * cubed) + (3.0f * squared);
        float third = cubed - (2.0f * squared) + amount;
        float fourth = cubed - squared;
        return new Vector2(
                (value1.X * first) + (value2.X * second) + (tangent1.X * third) + (tangent2.X * fourth),
                (value1.Y * first) + (value2.Y * second) + (tangent1.Y * third) + (tangent2.Y * fourth));
    }

    public static Vector2 Transform(Vector2 position, Matrix matrix) {
        return new Vector2(
                (position.X * matrix.M11) + (position.Y * matrix.M21) + matrix.M41,
                (position.X * matrix.M12) + (position.Y * matrix.M22) + matrix.M42);
    }

    public static Vector2 TransformNormal(Vector2 normal, Matrix matrix) {
        return new Vector2(
                (normal.X * matrix.M11) + (normal.Y * matrix.M21),
                (normal.X * matrix.M12) + (normal.Y * matrix.M22));
    }

    public static Vector2 Transform(Vector2 value, Quaternion rotation) {
        float x2 = rotation.X + rotation.X;
        float y2 = rotation.Y + rotation.Y;
        float z2 = rotation.Z + rotation.Z;
        float wz2 = rotation.W * z2;
        float xx2 = rotation.X * x2;
        float xy2 = rotation.X * y2;
        float yy2 = rotation.Y * y2;
        float zz2 = rotation.Z * z2;
        return new Vector2(
                (value.X * (1.0f - yy2 - zz2)) + (value.Y * (xy2 - wz2)),
                (value.X * (xy2 + wz2)) + (value.Y * (1.0f - xx2 - zz2)));
    }

    public static void Transform(Vector2[] sourceArray, Matrix matrix, Vector2[] destinationArray) {
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
            Vector2[] sourceArray, int sourceIndex, Matrix matrix,
            Vector2[] destinationArray, int destinationIndex, int length) {
        checkTransformRange(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
        while (length > 0) {
            destinationArray[destinationIndex] = Transform(sourceArray[sourceIndex], matrix);
            sourceIndex++;
            destinationIndex++;
            length--;
        }
    }

    public static void Transform(Vector2[] sourceArray, Quaternion rotation, Vector2[] destinationArray) {
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
            Vector2[] sourceArray, int sourceIndex, Quaternion rotation,
            Vector2[] destinationArray, int destinationIndex, int length) {
        checkTransformRange(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
        while (length > 0) {
            destinationArray[destinationIndex] = Transform(sourceArray[sourceIndex], rotation);
            sourceIndex++;
            destinationIndex++;
            length--;
        }
    }

    public static void TransformNormal(Vector2[] sourceArray, Matrix matrix, Vector2[] destinationArray) {
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
            Vector2[] sourceArray, int sourceIndex, Matrix matrix,
            Vector2[] destinationArray, int destinationIndex, int length) {
        checkTransformRange(sourceArray, sourceIndex, destinationArray, destinationIndex, length);
        while (length > 0) {
            destinationArray[destinationIndex] = TransformNormal(sourceArray[sourceIndex], matrix);
            sourceIndex++;
            destinationIndex++;
            length--;
        }
    }

    private static void checkTransformRange(
            Vector2[] sourceArray, int sourceIndex,
            Vector2[] destinationArray, int destinationIndex, int length) {
        Objects.requireNonNull(sourceArray, "sourceArray");
        Objects.requireNonNull(destinationArray, "destinationArray");
        if (sourceArray.length < ((long)sourceIndex + length)) {
            throw new IllegalArgumentException("The source array is too small.");
        }
        if (destinationArray.length < ((long)destinationIndex + length)) {
            throw new IllegalArgumentException("The destination array is too small.");
        }
    }

    public boolean equals(Vector2 other) {
        return other != null && X == other.X && Y == other.Y;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Vector2 value && equals(value);
    }

    @Override
    public int hashCode() {
        return FloatSemantics.hash(X) + FloatSemantics.hash(Y);
    }

    @Override
    public String toString() {
        return "{X:" + X + " Y:" + Y + '}';
    }
}
