package Microsoft.Xna.Framework;

import java.util.Objects;

/** Mutable XNA quaternion value. */
public final class Quaternion {

    public float X;
    public float Y;
    public float Z;
    public float W;

    public Quaternion() {
    }

    public Quaternion(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; }
    public Quaternion(Vector3 vectorPart, float scalarPart) { this(vectorPart.X, vectorPart.Y, vectorPart.Z, scalarPart); }
    public Quaternion(Quaternion value) { this(Objects.requireNonNull(value, "value").X, value.Y, value.Z, value.W); }

    public static Quaternion getIdentity() { return new Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }

    public float Length() { return (float)Math.sqrt(LengthSquared()); }
    public float LengthSquared() { return (X * X) + (Y * Y) + (Z * Z) + (W * W); }

    public void Normalize() {
        float factor = 1.0f / (float)Math.sqrt(LengthSquared());
        X *= factor; Y *= factor; Z *= factor; W *= factor;
    }

    public void Conjugate() { X = -X; Y = -Y; Z = -Z; }

    public static Quaternion Normalize(Quaternion quaternion) { Quaternion result = new Quaternion(quaternion); result.Normalize(); return result; }
    public static Quaternion Conjugate(Quaternion value) { return new Quaternion(-value.X, -value.Y, -value.Z, value.W); }

    public static Quaternion Inverse(Quaternion quaternion) {
        float inverse = 1.0f / quaternion.LengthSquared();
        return new Quaternion(-quaternion.X * inverse, -quaternion.Y * inverse, -quaternion.Z * inverse, quaternion.W * inverse);
    }

    public static Quaternion Add(Quaternion quaternion1, Quaternion quaternion2) {
        return new Quaternion(quaternion1.X + quaternion2.X, quaternion1.Y + quaternion2.Y, quaternion1.Z + quaternion2.Z, quaternion1.W + quaternion2.W);
    }

    public static Quaternion Subtract(Quaternion quaternion1, Quaternion quaternion2) {
        return new Quaternion(quaternion1.X - quaternion2.X, quaternion1.Y - quaternion2.Y, quaternion1.Z - quaternion2.Z, quaternion1.W - quaternion2.W);
    }

    public static Quaternion Negate(Quaternion quaternion) { return new Quaternion(-quaternion.X, -quaternion.Y, -quaternion.Z, -quaternion.W); }

    public static Quaternion Multiply(Quaternion quaternion1, Quaternion quaternion2) {
        float x = quaternion1.X;
        float y = quaternion1.Y;
        float z = quaternion1.Z;
        float w = quaternion1.W;
        float x2 = quaternion2.X;
        float y2 = quaternion2.Y;
        float z2 = quaternion2.Z;
        float w2 = quaternion2.W;
        float crossX = (y * z2) - (z * y2);
        float crossY = (z * x2) - (x * z2);
        float crossZ = (x * y2) - (y * x2);
        float dot = (x * x2) + (y * y2) + (z * z2);
        return new Quaternion(
                (x * w2) + (x2 * w) + crossX,
                (y * w2) + (y2 * w) + crossY,
                (z * w2) + (z2 * w) + crossZ,
                (w * w2) - dot);
    }

    public static Quaternion Multiply(Quaternion quaternion1, float scaleFactor) {
        return new Quaternion(quaternion1.X * scaleFactor, quaternion1.Y * scaleFactor, quaternion1.Z * scaleFactor, quaternion1.W * scaleFactor);
    }

    public static Quaternion Divide(Quaternion quaternion1, Quaternion quaternion2) {
        float x = quaternion1.X;
        float y = quaternion1.Y;
        float z = quaternion1.Z;
        float w = quaternion1.W;
        float lengthSquared = (quaternion2.X * quaternion2.X) + (quaternion2.Y * quaternion2.Y)
                + (quaternion2.Z * quaternion2.Z) + (quaternion2.W * quaternion2.W);
        float inverse = 1.0f / lengthSquared;
        float x2 = -quaternion2.X * inverse;
        float y2 = -quaternion2.Y * inverse;
        float z2 = -quaternion2.Z * inverse;
        float w2 = quaternion2.W * inverse;
        float crossX = (y * z2) - (z * y2);
        float crossY = (z * x2) - (x * z2);
        float crossZ = (x * y2) - (y * x2);
        float dot = (x * x2) + (y * y2) + (z * z2);
        return new Quaternion(
                (x * w2) + (x2 * w) + crossX,
                (y * w2) + (y2 * w) + crossY,
                (z * w2) + (z2 * w) + crossZ,
                (w * w2) - dot);
    }

    public static float Dot(Quaternion quaternion1, Quaternion quaternion2) {
        return (quaternion1.X * quaternion2.X) + (quaternion1.Y * quaternion2.Y) + (quaternion1.Z * quaternion2.Z) + (quaternion1.W * quaternion2.W);
    }

    public static Quaternion CreateFromAxisAngle(Vector3 axis, float angle) {
        float half = angle * 0.5f;
        float sine = (float)Math.sin(half);
        return new Quaternion(axis.X * sine, axis.Y * sine, axis.Z * sine, (float)Math.cos(half));
    }

    public static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll) {
        float halfRoll = roll * 0.5f;
        float sinRoll = (float)Math.sin(halfRoll);
        float cosRoll = (float)Math.cos(halfRoll);
        float halfPitch = pitch * 0.5f;
        float sinPitch = (float)Math.sin(halfPitch);
        float cosPitch = (float)Math.cos(halfPitch);
        float halfYaw = yaw * 0.5f;
        float sinYaw = (float)Math.sin(halfYaw);
        float cosYaw = (float)Math.cos(halfYaw);
        return new Quaternion(
                (cosYaw * sinPitch * cosRoll) + (sinYaw * cosPitch * sinRoll),
                (sinYaw * cosPitch * cosRoll) - (cosYaw * sinPitch * sinRoll),
                (cosYaw * cosPitch * sinRoll) - (sinYaw * sinPitch * cosRoll),
                (cosYaw * cosPitch * cosRoll) + (sinYaw * sinPitch * sinRoll));
    }

    public static Quaternion CreateFromRotationMatrix(Matrix matrix) {
        float trace = matrix.M11 + matrix.M22 + matrix.M33;
        if (trace > 0.0f) {
            float root = (float)Math.sqrt(trace + 1.0f);
            float w = root * 0.5f;
            root = 0.5f / root;
            return new Quaternion(
                    (matrix.M23 - matrix.M32) * root,
                    (matrix.M31 - matrix.M13) * root,
                    (matrix.M12 - matrix.M21) * root,
                    w);
        }
        if (matrix.M11 >= matrix.M22 && matrix.M11 >= matrix.M33) {
            float root = (float)Math.sqrt(1.0f + matrix.M11 - matrix.M22 - matrix.M33);
            float inverse = 0.5f / root;
            return new Quaternion(
                    0.5f * root,
                    (matrix.M12 + matrix.M21) * inverse,
                    (matrix.M13 + matrix.M31) * inverse,
                    (matrix.M23 - matrix.M32) * inverse);
        }
        if (matrix.M22 > matrix.M33) {
            float root = (float)Math.sqrt(1.0f + matrix.M22 - matrix.M11 - matrix.M33);
            float inverse = 0.5f / root;
            return new Quaternion(
                    (matrix.M21 + matrix.M12) * inverse,
                    0.5f * root,
                    (matrix.M32 + matrix.M23) * inverse,
                    (matrix.M31 - matrix.M13) * inverse);
        }
        float root = (float)Math.sqrt(1.0f + matrix.M33 - matrix.M11 - matrix.M22);
        float inverse = 0.5f / root;
        return new Quaternion(
                (matrix.M31 + matrix.M13) * inverse,
                (matrix.M32 + matrix.M23) * inverse,
                0.5f * root,
                (matrix.M12 - matrix.M21) * inverse);
    }

    public static Quaternion Concatenate(Quaternion value1, Quaternion value2) {
        float x = value2.X;
        float y = value2.Y;
        float z = value2.Z;
        float w = value2.W;
        float x2 = value1.X;
        float y2 = value1.Y;
        float z2 = value1.Z;
        float w2 = value1.W;
        float crossX = (y * z2) - (z * y2);
        float crossY = (z * x2) - (x * z2);
        float crossZ = (x * y2) - (y * x2);
        float dot = (x * x2) + (y * y2) + (z * z2);
        return new Quaternion(
                (x * w2) + (x2 * w) + crossX,
                (y * w2) + (y2 * w) + crossY,
                (z * w2) + (z2 * w) + crossZ,
                (w * w2) - dot);
    }

    public static Quaternion Lerp(Quaternion quaternion1, Quaternion quaternion2, float amount) {
        float inverse = 1.0f - amount;
        Quaternion result = Dot(quaternion1, quaternion2) >= 0.0f
                ? new Quaternion(
                        (inverse * quaternion1.X) + (amount * quaternion2.X),
                        (inverse * quaternion1.Y) + (amount * quaternion2.Y),
                        (inverse * quaternion1.Z) + (amount * quaternion2.Z),
                        (inverse * quaternion1.W) + (amount * quaternion2.W))
                : new Quaternion(
                        (inverse * quaternion1.X) - (amount * quaternion2.X),
                        (inverse * quaternion1.Y) - (amount * quaternion2.Y),
                        (inverse * quaternion1.Z) - (amount * quaternion2.Z),
                        (inverse * quaternion1.W) - (amount * quaternion2.W));
        result.Normalize();
        return result;
    }

    public static Quaternion Slerp(Quaternion quaternion1, Quaternion quaternion2, float amount) {
        float dot = Dot(quaternion1, quaternion2);
        boolean negate = dot < 0.0f;
        if (negate) {
            dot = -dot;
        }
        float leftScale;
        float rightScale;
        if (dot > 0.999999f) {
            leftScale = 1.0f - amount;
            rightScale = negate ? -amount : amount;
        } else {
            float angle = (float)Math.acos(dot);
            float inverseSin = 1.0f / (float)Math.sin(angle);
            leftScale = (float)Math.sin((1.0f - amount) * angle) * inverseSin;
            rightScale = (negate ? -1.0f : 1.0f) * (float)Math.sin(amount * angle) * inverseSin;
        }
        return new Quaternion(
                (leftScale * quaternion1.X) + (rightScale * quaternion2.X),
                (leftScale * quaternion1.Y) + (rightScale * quaternion2.Y),
                (leftScale * quaternion1.Z) + (rightScale * quaternion2.Z),
                (leftScale * quaternion1.W) + (rightScale * quaternion2.W));
    }

    public boolean equals(Quaternion other) {
        return other != null && X == other.X && Y == other.Y && Z == other.Z && W == other.W;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Quaternion value && equals(value);
    }

    @Override
    public int hashCode() { return FloatSemantics.hash(X) + FloatSemantics.hash(Y) + FloatSemantics.hash(Z) + FloatSemantics.hash(W); }

    @Override
    public String toString() { return "{X:" + X + " Y:" + Y + " Z:" + Z + " W:" + W + '}'; }
}
