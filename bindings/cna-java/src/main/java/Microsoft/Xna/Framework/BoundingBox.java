package Microsoft.Xna.Framework;

import java.util.Objects;

/** Axis-aligned XNA bounding box. Constructor inputs are snapshotted. */
public final class BoundingBox {

    public static final int CornerCount = 8;

    public Vector3 Min;
    public Vector3 Max;

    public BoundingBox() {
        Min = new Vector3();
        Max = new Vector3();
    }

    public BoundingBox(Vector3 min, Vector3 max) {
        Min = new Vector3(Objects.requireNonNull(min, "min"));
        Max = new Vector3(Objects.requireNonNull(max, "max"));
    }

    public BoundingBox(BoundingBox value) {
        this(Objects.requireNonNull(value, "value").Min, value.Max);
    }

    public Vector3[] GetCorners() {
        return new Vector3[] {
            new Vector3(Min.X, Max.Y, Max.Z), new Vector3(Max.X, Max.Y, Max.Z),
            new Vector3(Max.X, Min.Y, Max.Z), new Vector3(Min.X, Min.Y, Max.Z),
            new Vector3(Min.X, Max.Y, Min.Z), new Vector3(Max.X, Max.Y, Min.Z),
            new Vector3(Max.X, Min.Y, Min.Z), new Vector3(Min.X, Min.Y, Min.Z)
        };
    }

    public void GetCorners(Vector3[] corners) {
        Objects.requireNonNull(corners, "corners");
        if (corners.length < CornerCount) {
            throw new IllegalArgumentException("corners must contain at least eight elements.");
        }
        Vector3[] values = GetCorners();
        System.arraycopy(values, 0, corners, 0, CornerCount);
    }

    public static BoundingBox CreateMerged(BoundingBox original, BoundingBox additional) {
        return new BoundingBox(
                Vector3.Min(original.Min, additional.Min),
                Vector3.Max(original.Max, additional.Max));
    }

    public static BoundingBox CreateFromSphere(BoundingSphere sphere) {
        return new BoundingBox(
                new Vector3(
                        sphere.Center.X - sphere.Radius,
                        sphere.Center.Y - sphere.Radius,
                        sphere.Center.Z - sphere.Radius),
                new Vector3(
                        sphere.Center.X + sphere.Radius,
                        sphere.Center.Y + sphere.Radius,
                        sphere.Center.Z + sphere.Radius));
    }

    public static BoundingBox CreateFromPoints(Iterable<Vector3> points) {
        Objects.requireNonNull(points, "points");
        boolean any = false;
        Vector3 min = new Vector3(Float.MAX_VALUE);
        Vector3 max = new Vector3(-Float.MAX_VALUE);
        for (Vector3 point : points) {
            Vector3 value = new Vector3(point);
            min = Vector3.Min(min, value);
            max = Vector3.Max(max, value);
            any = true;
        }
        if (!any) {
            throw new IllegalArgumentException("points must contain at least one point.");
        }
        return new BoundingBox(min, max);
    }

    public boolean Intersects(BoundingBox box) {
        if (Max.X < box.Min.X || Min.X > box.Max.X) {
            return false;
        }
        if (Max.Y < box.Min.Y || Min.Y > box.Max.Y) {
            return false;
        }
        if (Max.Z < box.Min.Z || Min.Z > box.Max.Z) {
            return false;
        }
        return true;
    }

    public boolean Intersects(BoundingFrustum frustum) {
        return Objects.requireNonNull(frustum, "frustum").Intersects(this);
    }

    public PlaneIntersectionType Intersects(Plane plane) {
        return plane.Intersects(this);
    }

    public Float Intersects(Ray ray) {
        float minimum = 0.0f;
        float maximum = Float.MAX_VALUE;
        float[] positions = {ray.Position.X, ray.Position.Y, ray.Position.Z};
        float[] directions = {ray.Direction.X, ray.Direction.Y, ray.Direction.Z};
        float[] minima = {Min.X, Min.Y, Min.Z};
        float[] maxima = {Max.X, Max.Y, Max.Z};
        for (int index = 0; index < 3; index++) {
            if (Math.abs(directions[index]) < 1.0E-06f) {
                if (positions[index] < minima[index] || positions[index] > maxima[index]) {
                    return null;
                }
            } else {
                float inverse = 1.0f / directions[index];
                float first = (minima[index] - positions[index]) * inverse;
                float second = (maxima[index] - positions[index]) * inverse;
                if (first > second) {
                    float swap = first;
                    first = second;
                    second = swap;
                }
                minimum = MathHelper.Max(first, minimum);
                maximum = MathHelper.Min(second, maximum);
                if (minimum > maximum) {
                    return null;
                }
            }
        }
        return minimum;
    }

    public boolean Intersects(BoundingSphere sphere) {
        Vector3 nearest = Vector3.Clamp(sphere.Center, Min, Max);
        float distanceSquared = Vector3.DistanceSquared(sphere.Center, nearest);
        return !(distanceSquared > (sphere.Radius * sphere.Radius));
    }

    public ContainmentType Contains(BoundingBox box) {
        if (Max.X < box.Min.X || Min.X > box.Max.X
                || Max.Y < box.Min.Y || Min.Y > box.Max.Y
                || Max.Z < box.Min.Z || Min.Z > box.Max.Z) {
            return ContainmentType.Disjoint;
        }
        if (!(Min.X <= box.Min.X) || !(box.Max.X <= Max.X)
                || !(Min.Y <= box.Min.Y) || !(box.Max.Y <= Max.Y)
                || !(Min.Z <= box.Min.Z) || !(box.Max.Z <= Max.Z)) {
            return ContainmentType.Intersects;
        }
        return ContainmentType.Contains;
    }

    public ContainmentType Contains(BoundingFrustum frustum) {
        Objects.requireNonNull(frustum, "frustum");
        if (!frustum.Intersects(this)) {
            return ContainmentType.Disjoint;
        }
        for (Vector3 point : frustum.cornerArray()) {
            if (Contains(point) == ContainmentType.Disjoint) {
                return ContainmentType.Intersects;
            }
        }
        return ContainmentType.Contains;
    }

    public ContainmentType Contains(Vector3 point) {
        if (!(Min.X <= point.X) || !(point.X <= Max.X)
                || !(Min.Y <= point.Y) || !(point.Y <= Max.Y)
                || !(Min.Z <= point.Z) || !(point.Z <= Max.Z)) {
            return ContainmentType.Disjoint;
        }
        return ContainmentType.Contains;
    }

    public ContainmentType Contains(BoundingSphere sphere) {
        Vector3 nearest = Vector3.Clamp(sphere.Center, Min, Max);
        float distanceSquared = Vector3.DistanceSquared(sphere.Center, nearest);
        float radius = sphere.Radius;
        if (distanceSquared > (radius * radius)) {
            return ContainmentType.Disjoint;
        }
        if (!(Min.X + radius <= sphere.Center.X)
                || !(sphere.Center.X <= Max.X - radius)
                || !(Max.X - Min.X > radius)
                || !(Min.Y + radius <= sphere.Center.Y)
                || !(sphere.Center.Y <= Max.Y - radius)
                || !(Max.Y - Min.Y > radius)
                || !(Min.Z + radius <= sphere.Center.Z)
                || !(sphere.Center.Z <= Max.Z - radius)
                || !(Max.X - Min.X > radius)) {
            return ContainmentType.Intersects;
        }
        return ContainmentType.Contains;
    }

    void supportMapping(Vector3 direction, Vector3 result) {
        result.X = direction.X >= 0.0f ? Max.X : Min.X;
        result.Y = direction.Y >= 0.0f ? Max.Y : Min.Y;
        result.Z = direction.Z >= 0.0f ? Max.Z : Min.Z;
    }

    public boolean equals(BoundingBox other) {
        return other != null && Min.equals(other.Min) && Max.equals(other.Max);
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof BoundingBox value && equals(value);
    }

    @Override
    public int hashCode() {
        return Min.hashCode() + Max.hashCode();
    }

    @Override
    public String toString() {
        return "{Min:" + Min + " Max:" + Max + '}';
    }
}
