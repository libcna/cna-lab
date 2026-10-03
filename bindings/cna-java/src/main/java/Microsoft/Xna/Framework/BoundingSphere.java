package Microsoft.Xna.Framework;

import java.util.Arrays;
import java.util.Iterator;
import java.util.Objects;

/** XNA bounding sphere. Constructor inputs are snapshotted. */
public final class BoundingSphere {

    public Vector3 Center;
    public float Radius;

    public BoundingSphere() {
        Center = new Vector3();
    }

    public BoundingSphere(Vector3 center, float radius) {
        if (radius < 0.0f) {
            throw new IllegalArgumentException("radius must not be negative.");
        }
        Center = new Vector3(Objects.requireNonNull(center, "center"));
        Radius = radius;
    }

    public BoundingSphere(BoundingSphere value) {
        this(Objects.requireNonNull(value, "value").Center, value.Radius);
    }

    public static BoundingSphere CreateMerged(BoundingSphere original, BoundingSphere additional) {
        Vector3 difference = Vector3.Subtract(additional.Center, original.Center);
        float distance = difference.Length();
        float radius = original.Radius;
        float additionalRadius = additional.Radius;
        if (radius + additionalRadius >= distance) {
            if (radius - additionalRadius >= distance) {
                return new BoundingSphere(original);
            }
            if (additionalRadius - radius >= distance) {
                return new BoundingSphere(additional);
            }
        }
        Vector3 direction = Vector3.Multiply(difference, 1.0f / distance);
        float minimum = MathHelper.Min(0.0f - radius, distance - additionalRadius);
        float maximum = MathHelper.Max(radius, distance + additionalRadius);
        float mergedRadius = (maximum - minimum) * 0.5f;
        Vector3 center = Vector3.Add(
                original.Center, Vector3.Multiply(direction, mergedRadius + minimum));
        return new BoundingSphere(center, mergedRadius);
    }

    public static BoundingSphere CreateFromBoundingBox(BoundingBox box) {
        Vector3 center = Vector3.Lerp(box.Min, box.Max, 0.5f);
        float radius = Vector3.Distance(box.Min, box.Max) * 0.5f;
        return new BoundingSphere(center, radius);
    }

    public static BoundingSphere CreateFromPoints(Iterable<Vector3> points) {
        Objects.requireNonNull(points, "points");
        Iterator<Vector3> iterator = points.iterator();
        if (!iterator.hasNext()) {
            throw new IllegalArgumentException("points must contain at least one point.");
        }
        Vector3 initial = new Vector3(iterator.next());
        Vector3 minX = new Vector3(initial);
        Vector3 maxX = new Vector3(initial);
        Vector3 minY = new Vector3(initial);
        Vector3 maxY = new Vector3(initial);
        Vector3 minZ = new Vector3(initial);
        Vector3 maxZ = new Vector3(initial);
        for (Vector3 item : points) {
            Vector3 point = new Vector3(item);
            if (point.X < minX.X) { minX = point; }
            if (point.X > maxX.X) { maxX = point; }
            if (point.Y < minY.Y) { minY = point; }
            if (point.Y > maxY.Y) { maxY = point; }
            if (point.Z < minZ.Z) { minZ = point; }
            if (point.Z > maxZ.Z) { maxZ = point; }
        }
        float xSpan = Vector3.Distance(maxX, minX);
        float ySpan = Vector3.Distance(maxY, minY);
        float zSpan = Vector3.Distance(maxZ, minZ);
        Vector3 center;
        float radius;
        if (xSpan > ySpan) {
            if (xSpan > zSpan) {
                center = Vector3.Lerp(maxX, minX, 0.5f);
                radius = xSpan * 0.5f;
            } else {
                center = Vector3.Lerp(maxZ, minZ, 0.5f);
                radius = zSpan * 0.5f;
            }
        } else if (ySpan > zSpan) {
            center = Vector3.Lerp(maxY, minY, 0.5f);
            radius = ySpan * 0.5f;
        } else {
            center = Vector3.Lerp(maxZ, minZ, 0.5f);
            radius = zSpan * 0.5f;
        }
        for (Vector3 point : points) {
            Vector3 offset = new Vector3(
                    point.X - center.X, point.Y - center.Y, point.Z - center.Z);
            float distance = offset.Length();
            if (distance > radius) {
                radius = (radius + distance) * 0.5f;
                center = Vector3.Add(center,
                        Vector3.Multiply(offset, 1.0f - (radius / distance)));
            }
        }
        return new BoundingSphere(center, radius);
    }

    public static BoundingSphere CreateFromFrustum(BoundingFrustum frustum) {
        Objects.requireNonNull(frustum, "frustum");
        return CreateFromPoints(Arrays.asList(frustum.cornerArray()));
    }

    public boolean Intersects(BoundingBox box) {
        return box.Intersects(this);
    }

    public boolean Intersects(BoundingFrustum frustum) {
        return Objects.requireNonNull(frustum, "frustum").Intersects(this);
    }

    public PlaneIntersectionType Intersects(Plane plane) {
        return plane.Intersects(this);
    }

    public Float Intersects(Ray ray) {
        return ray.Intersects(this);
    }

    public boolean Intersects(BoundingSphere sphere) {
        float distanceSquared = Vector3.DistanceSquared(Center, sphere.Center);
        float radius = Radius;
        float otherRadius = sphere.Radius;
        return ((radius * radius) + (2.0f * radius * otherRadius)
                + (otherRadius * otherRadius)) > distanceSquared;
    }

    public ContainmentType Contains(BoundingBox box) {
        if (!box.Intersects(this)) {
            return ContainmentType.Disjoint;
        }
        float radiusSquared = Radius * Radius;
        for (Vector3 corner : box.GetCorners()) {
            Vector3 offset = new Vector3(
                    Center.X - corner.X, Center.Y - corner.Y, Center.Z - corner.Z);
            if (offset.LengthSquared() > radiusSquared) {
                return ContainmentType.Intersects;
            }
        }
        return ContainmentType.Contains;
    }

    public ContainmentType Contains(BoundingFrustum frustum) {
        Objects.requireNonNull(frustum, "frustum");
        if (!frustum.Intersects(this)) {
            return ContainmentType.Disjoint;
        }
        float radiusSquared = Radius * Radius;
        for (Vector3 corner : frustum.cornerArray()) {
            Vector3 offset = new Vector3(
                    corner.X - Center.X, corner.Y - Center.Y, corner.Z - Center.Z);
            if (offset.LengthSquared() > radiusSquared) {
                return ContainmentType.Intersects;
            }
        }
        return ContainmentType.Contains;
    }

    public ContainmentType Contains(Vector3 point) {
        float distanceSquared = Vector3.DistanceSquared(point, Center);
        return distanceSquared < (Radius * Radius)
                ? ContainmentType.Contains : ContainmentType.Disjoint;
    }

    public ContainmentType Contains(BoundingSphere sphere) {
        float distance = Vector3.Distance(Center, sphere.Center);
        float radius = Radius;
        float otherRadius = sphere.Radius;
        if (!(radius + otherRadius >= distance)) {
            return ContainmentType.Disjoint;
        }
        if (!(radius - otherRadius >= distance)) {
            return ContainmentType.Intersects;
        }
        return ContainmentType.Contains;
    }

    public BoundingSphere Transform(Matrix matrix) {
        Vector3 center = Vector3.Transform(Center, matrix);
        float first = (matrix.M11 * matrix.M11)
                + (matrix.M12 * matrix.M12) + (matrix.M13 * matrix.M13);
        float second = (matrix.M21 * matrix.M21)
                + (matrix.M22 * matrix.M22) + (matrix.M23 * matrix.M23);
        float third = (matrix.M31 * matrix.M31)
                + (matrix.M32 * matrix.M32) + (matrix.M33 * matrix.M33);
        float maximum = Math.max(first, Math.max(second, third));
        return new BoundingSphere(center, Radius * (float)Math.sqrt(maximum));
    }

    void supportMapping(Vector3 direction, Vector3 result) {
        float length = direction.Length();
        float factor = Radius / length;
        result.X = Center.X + (direction.X * factor);
        result.Y = Center.Y + (direction.Y * factor);
        result.Z = Center.Z + (direction.Z * factor);
    }

    public boolean equals(BoundingSphere other) {
        return other != null && Center.equals(other.Center) && Radius == other.Radius;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof BoundingSphere value && equals(value);
    }

    @Override
    public int hashCode() {
        return Center.hashCode() + FloatSemantics.hash(Radius);
    }

    @Override
    public String toString() {
        return "{Center:" + Center + " Radius:" + Radius + '}';
    }
}
