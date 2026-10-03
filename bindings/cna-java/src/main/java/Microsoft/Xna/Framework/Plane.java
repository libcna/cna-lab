package Microsoft.Xna.Framework;

import java.util.Objects;

/** XNA plane represented by a normal and distance. */
public final class Plane {

    public Vector3 Normal;
    public float D;

    public Plane() {
        Normal = new Vector3();
    }

    public Plane(float a, float b, float c, float d) {
        Normal = new Vector3(a, b, c);
        D = d;
    }

    public Plane(Vector3 normal, float d) {
        Normal = new Vector3(Objects.requireNonNull(normal, "normal"));
        D = d;
    }

    public Plane(Vector4 value) {
        this(Objects.requireNonNull(value, "value").X, value.Y, value.Z, value.W);
    }

    public Plane(Plane value) {
        this(Objects.requireNonNull(value, "value").Normal, value.D);
    }

    public Plane(Vector3 point1, Vector3 point2, Vector3 point3) {
        float x1 = point2.X - point1.X;
        float y1 = point2.Y - point1.Y;
        float z1 = point2.Z - point1.Z;
        float x2 = point3.X - point1.X;
        float y2 = point3.Y - point1.Y;
        float z2 = point3.Z - point1.Z;
        float x = (y1 * z2) - (z1 * y2);
        float y = (z1 * x2) - (x1 * z2);
        float z = (x1 * y2) - (y1 * x2);
        float lengthSquared = (x * x) + (y * y) + (z * z);
        float inverse = 1.0f / (float)Math.sqrt(lengthSquared);
        Normal = new Vector3(x * inverse, y * inverse, z * inverse);
        D = -((Normal.X * point1.X) + (Normal.Y * point1.Y) + (Normal.Z * point1.Z));
    }

    public float Dot(Vector4 value) {
        return (Normal.X * value.X) + (Normal.Y * value.Y)
                + (Normal.Z * value.Z) + (D * value.W);
    }

    public float DotCoordinate(Vector3 value) {
        return (Normal.X * value.X) + (Normal.Y * value.Y) + (Normal.Z * value.Z) + D;
    }

    public float DotNormal(Vector3 value) {
        return (Normal.X * value.X) + (Normal.Y * value.Y) + (Normal.Z * value.Z);
    }

    public void Normalize() {
        float lengthSquared = (Normal.X * Normal.X) + (Normal.Y * Normal.Y) + (Normal.Z * Normal.Z);
        if (!(Math.abs(lengthSquared - 1.0f) < 1.1920929E-07f)) {
            float inverse = 1.0f / (float)Math.sqrt(lengthSquared);
            Normal.X *= inverse;
            Normal.Y *= inverse;
            Normal.Z *= inverse;
            D *= inverse;
        }
    }

    public static Plane Normalize(Plane value) {
        float lengthSquared = (value.Normal.X * value.Normal.X)
                + (value.Normal.Y * value.Normal.Y) + (value.Normal.Z * value.Normal.Z);
        if (Math.abs(lengthSquared - 1.0f) < 1.1920929E-07f) {
            return new Plane(value);
        }
        float inverse = 1.0f / (float)Math.sqrt(lengthSquared);
        return new Plane(
                value.Normal.X * inverse,
                value.Normal.Y * inverse,
                value.Normal.Z * inverse,
                value.D * inverse);
    }

    public static Plane Transform(Plane plane, Matrix matrix) {
        Matrix inverse = Matrix.Invert(matrix);
        float x = plane.Normal.X;
        float y = plane.Normal.Y;
        float z = plane.Normal.Z;
        float d = plane.D;
        return new Plane(
                (x * inverse.M11) + (y * inverse.M12) + (z * inverse.M13) + (d * inverse.M14),
                (x * inverse.M21) + (y * inverse.M22) + (z * inverse.M23) + (d * inverse.M24),
                (x * inverse.M31) + (y * inverse.M32) + (z * inverse.M33) + (d * inverse.M34),
                (x * inverse.M41) + (y * inverse.M42) + (z * inverse.M43) + (d * inverse.M44));
    }

    public static Plane Transform(Plane plane, Quaternion rotation) {
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
        float x = plane.Normal.X;
        float y = plane.Normal.Y;
        float z = plane.Normal.Z;
        return new Plane(
                (x * r11) + (y * r12) + (z * r13),
                (x * r21) + (y * r22) + (z * r23),
                (x * r31) + (y * r32) + (z * r33),
                plane.D);
    }

    public PlaneIntersectionType Intersects(BoundingBox box) {
        Vector3 negative = new Vector3(
                Normal.X >= 0.0f ? box.Min.X : box.Max.X,
                Normal.Y >= 0.0f ? box.Min.Y : box.Max.Y,
                Normal.Z >= 0.0f ? box.Min.Z : box.Max.Z);
        Vector3 positive = new Vector3(
                Normal.X >= 0.0f ? box.Max.X : box.Min.X,
                Normal.Y >= 0.0f ? box.Max.Y : box.Min.Y,
                Normal.Z >= 0.0f ? box.Max.Z : box.Min.Z);
        float distance = (Normal.X * negative.X) + (Normal.Y * negative.Y) + (Normal.Z * negative.Z);
        if (distance + D > 0.0f) {
            return PlaneIntersectionType.Front;
        }
        distance = (Normal.X * positive.X) + (Normal.Y * positive.Y) + (Normal.Z * positive.Z);
        if (distance + D < 0.0f) {
            return PlaneIntersectionType.Back;
        }
        return PlaneIntersectionType.Intersecting;
    }

    public PlaneIntersectionType Intersects(BoundingFrustum frustum) {
        return Objects.requireNonNull(frustum, "frustum").Intersects(this);
    }

    public PlaneIntersectionType Intersects(BoundingSphere sphere) {
        float dot = (sphere.Center.X * Normal.X)
                + (sphere.Center.Y * Normal.Y) + (sphere.Center.Z * Normal.Z);
        float distance = dot + D;
        if (distance > sphere.Radius) {
            return PlaneIntersectionType.Front;
        }
        if (distance < -sphere.Radius) {
            return PlaneIntersectionType.Back;
        }
        return PlaneIntersectionType.Intersecting;
    }

    public boolean equals(Plane other) {
        return other != null && Normal.equals(other.Normal) && D == other.D;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Plane value && equals(value);
    }

    @Override
    public int hashCode() {
        return Normal.hashCode() + FloatSemantics.hash(D);
    }

    @Override
    public String toString() {
        return "{Normal:" + Normal + " D:" + D + '}';
    }
}
