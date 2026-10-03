package Microsoft.Xna.Framework;

import java.util.Objects;

/** XNA view frustum with its original plane extraction and GJK intersection behavior. */
public class BoundingFrustum {

    public static final int CornerCount = 8;

    private Matrix matrix;
    private final Plane[] planes = new Plane[6];
    private final Vector3[] corners = new Vector3[CornerCount];
    private Gjk gjk;

    public BoundingFrustum(Matrix value) {
        setMatrix(value);
    }

    public final Plane getNear() { return new Plane(planes[0]); }
    public final Plane getFar() { return new Plane(planes[1]); }
    public final Plane getLeft() { return new Plane(planes[2]); }
    public final Plane getRight() { return new Plane(planes[3]); }
    public final Plane getTop() { return new Plane(planes[4]); }
    public final Plane getBottom() { return new Plane(planes[5]); }
    public final Matrix getMatrix() { return new Matrix(matrix); }

    public final void setMatrix(Matrix value) {
        Objects.requireNonNull(value, "value");
        matrix = new Matrix(value);
        planes[2] = new Plane(
                -value.M14 - value.M11,
                -value.M24 - value.M21,
                -value.M34 - value.M31,
                -value.M44 - value.M41);
        planes[3] = new Plane(
                -value.M14 + value.M11,
                -value.M24 + value.M21,
                -value.M34 + value.M31,
                -value.M44 + value.M41);
        planes[4] = new Plane(
                -value.M14 + value.M12,
                -value.M24 + value.M22,
                -value.M34 + value.M32,
                -value.M44 + value.M42);
        planes[5] = new Plane(
                -value.M14 - value.M12,
                -value.M24 - value.M22,
                -value.M34 - value.M32,
                -value.M44 - value.M42);
        planes[0] = new Plane(
                -value.M13,
                -value.M23,
                -value.M33,
                -value.M43);
        planes[1] = new Plane(
                -value.M14 + value.M13,
                -value.M24 + value.M23,
                -value.M34 + value.M33,
                -value.M44 + value.M43);
        for (Plane plane : planes) {
            float length = plane.Normal.Length();
            plane.Normal = Vector3.Divide(plane.Normal, length);
            plane.D /= length;
        }
        Ray ray = computeIntersectionLine(planes[0], planes[2]);
        corners[0] = computeIntersection(planes[4], ray);
        corners[3] = computeIntersection(planes[5], ray);
        ray = computeIntersectionLine(planes[3], planes[0]);
        corners[1] = computeIntersection(planes[4], ray);
        corners[2] = computeIntersection(planes[5], ray);
        ray = computeIntersectionLine(planes[2], planes[1]);
        corners[4] = computeIntersection(planes[4], ray);
        corners[7] = computeIntersection(planes[5], ray);
        ray = computeIntersectionLine(planes[1], planes[3]);
        corners[5] = computeIntersection(planes[4], ray);
        corners[6] = computeIntersection(planes[5], ray);
    }

    private static Ray computeIntersectionLine(Plane first, Plane second) {
        Vector3 direction = Vector3.Cross(first.Normal, second.Normal);
        float lengthSquared = direction.LengthSquared();
        Vector3 term1 = Vector3.Multiply(-first.D, second.Normal);
        Vector3 term2 = Vector3.Multiply(second.D, first.Normal);
        Vector3 position = Vector3.Divide(
                Vector3.Cross(Vector3.Add(term1, term2), direction), lengthSquared);
        return new Ray(position, direction);
    }

    private static Vector3 computeIntersection(Plane plane, Ray ray) {
        float distance = (-plane.D - Vector3.Dot(plane.Normal, ray.Position))
                / Vector3.Dot(plane.Normal, ray.Direction);
        return Vector3.Add(ray.Position, Vector3.Multiply(ray.Direction, distance));
    }

    public final Vector3[] GetCorners() {
        Vector3[] result = new Vector3[CornerCount];
        for (int i = 0; i < CornerCount; i++) {
            result[i] = new Vector3(corners[i]);
        }
        return result;
    }

    public final void GetCorners(Vector3[] corners) {
        Objects.requireNonNull(corners, "corners");
        if (corners.length < CornerCount) {
            throw new IllegalArgumentException("corners must contain at least eight elements.");
        }
        for (int i = 0; i < CornerCount; i++) {
            corners[i] = new Vector3(this.corners[i]);
        }
    }

    Vector3[] cornerArray() {
        return corners;
    }

    public final boolean Intersects(BoundingBox box) {
        ensureGjk();
        gjk.reset();
        Vector3 closest = Vector3.Subtract(corners[0], box.Min);
        if (closest.LengthSquared() < 1.0E-05f) {
            closest = Vector3.Subtract(corners[0], box.Max);
        }
        float previousDistance = Float.MAX_VALUE;
        float threshold = 0.0f;
        Vector3 direction = new Vector3();
        do {
            direction.X = -closest.X;
            direction.Y = -closest.Y;
            direction.Z = -closest.Z;
            Vector3 firstSupport = new Vector3();
            supportMapping(direction, firstSupport);
            Vector3 secondSupport = new Vector3();
            box.supportMapping(closest, secondSupport);
            Vector3 support = Vector3.Subtract(firstSupport, secondSupport);
            float dot = (closest.X * support.X) + (closest.Y * support.Y) + (closest.Z * support.Z);
            if (dot > 0.0f) {
                return false;
            }
            gjk.addSupportPoint(support);
            closest = gjk.getClosestPoint();
            float oldDistance = previousDistance;
            previousDistance = closest.LengthSquared();
            if (oldDistance - previousDistance <= 1.0E-05f * oldDistance) {
                return false;
            }
            threshold = 4.0E-05f * gjk.getMaxLengthSquared();
        } while (!gjk.isFullSimplex() && previousDistance >= threshold);
        return true;
    }

    public final boolean Intersects(BoundingFrustum frustum) {
        Objects.requireNonNull(frustum, "frustum");
        ensureGjk();
        gjk.reset();
        Vector3 closest = Vector3.Subtract(corners[0], frustum.corners[0]);
        if (closest.LengthSquared() < 1.0E-05f) {
            closest = Vector3.Subtract(corners[0], frustum.corners[1]);
        }
        float previousDistance = Float.MAX_VALUE;
        float threshold;
        Vector3 direction = new Vector3();
        do {
            direction.X = -closest.X;
            direction.Y = -closest.Y;
            direction.Z = -closest.Z;
            Vector3 firstSupport = new Vector3();
            supportMapping(direction, firstSupport);
            Vector3 secondSupport = new Vector3();
            frustum.supportMapping(closest, secondSupport);
            Vector3 support = Vector3.Subtract(firstSupport, secondSupport);
            float dot = (closest.X * support.X) + (closest.Y * support.Y) + (closest.Z * support.Z);
            if (dot > 0.0f) {
                return false;
            }
            gjk.addSupportPoint(support);
            closest = gjk.getClosestPoint();
            float oldDistance = previousDistance;
            previousDistance = closest.LengthSquared();
            threshold = 4.0E-05f * gjk.getMaxLengthSquared();
            if (oldDistance - previousDistance <= 1.0E-05f * oldDistance) {
                return false;
            }
        } while (!gjk.isFullSimplex() && previousDistance >= threshold);
        return true;
    }

    public final PlaneIntersectionType Intersects(Plane plane) {
        int sides = 0;
        for (Vector3 corner : corners) {
            float dot = Vector3.Dot(corner, plane.Normal);
            sides = !(dot + plane.D > 0.0f) ? sides | 2 : sides | 1;
            if (sides == 3) {
                return PlaneIntersectionType.Intersecting;
            }
        }
        return sides != 1 ? PlaneIntersectionType.Back : PlaneIntersectionType.Front;
    }

    public final Float Intersects(Ray ray) {
        if (Contains(ray.Position) == ContainmentType.Contains) {
            return 0.0f;
        }
        float entering = -Float.MAX_VALUE;
        float leaving = Float.MAX_VALUE;
        for (Plane plane : planes) {
            float directionDot = Vector3.Dot(ray.Direction, plane.Normal);
            float positionDot = Vector3.Dot(ray.Position, plane.Normal) + plane.D;
            if (Math.abs(directionDot) < 1.0E-05f) {
                if (positionDot > 0.0f) {
                    return null;
                }
                continue;
            }
            float distance = -positionDot / directionDot;
            if (directionDot < 0.0f) {
                if (distance > leaving) {
                    return null;
                }
                if (distance > entering) {
                    entering = distance;
                }
            } else {
                if (distance < entering) {
                    return null;
                }
                if (distance < leaving) {
                    leaving = distance;
                }
            }
        }
        float distance = entering >= 0.0f ? entering : leaving;
        return distance >= 0.0f ? distance : null;
    }

    public final boolean Intersects(BoundingSphere sphere) {
        ensureGjk();
        gjk.reset();
        Vector3 closest = Vector3.Subtract(corners[0], sphere.Center);
        if (closest.LengthSquared() < 1.0E-05f) {
            closest = Vector3.getUnitX();
        }
        float previousDistance = Float.MAX_VALUE;
        float threshold = 0.0f;
        Vector3 direction = new Vector3();
        do {
            direction.X = -closest.X;
            direction.Y = -closest.Y;
            direction.Z = -closest.Z;
            Vector3 firstSupport = new Vector3();
            supportMapping(direction, firstSupport);
            Vector3 secondSupport = new Vector3();
            sphere.supportMapping(closest, secondSupport);
            Vector3 support = Vector3.Subtract(firstSupport, secondSupport);
            float dot = (closest.X * support.X) + (closest.Y * support.Y) + (closest.Z * support.Z);
            if (dot > 0.0f) {
                return false;
            }
            gjk.addSupportPoint(support);
            closest = gjk.getClosestPoint();
            float oldDistance = previousDistance;
            previousDistance = closest.LengthSquared();
            if (oldDistance - previousDistance <= 1.0E-05f * oldDistance) {
                return false;
            }
            threshold = 4.0E-05f * gjk.getMaxLengthSquared();
        } while (!gjk.isFullSimplex() && previousDistance >= threshold);
        return true;
    }

    public final ContainmentType Contains(BoundingBox box) {
        boolean intersects = false;
        for (Plane plane : planes) {
            PlaneIntersectionType relation = box.Intersects(plane);
            if (relation == PlaneIntersectionType.Front) {
                return ContainmentType.Disjoint;
            }
            if (relation == PlaneIntersectionType.Intersecting) {
                intersects = true;
            }
        }
        return intersects ? ContainmentType.Intersects : ContainmentType.Contains;
    }

    public final ContainmentType Contains(BoundingFrustum frustum) {
        Objects.requireNonNull(frustum, "frustum");
        if (!Intersects(frustum)) {
            return ContainmentType.Disjoint;
        }
        for (Vector3 corner : frustum.corners) {
            if (Contains(corner) == ContainmentType.Disjoint) {
                return ContainmentType.Intersects;
            }
        }
        return ContainmentType.Contains;
    }

    public final ContainmentType Contains(Vector3 point) {
        for (Plane plane : planes) {
            float distance = (plane.Normal.X * point.X)
                    + (plane.Normal.Y * point.Y) + (plane.Normal.Z * point.Z) + plane.D;
            if (distance > 1.0E-05f) {
                return ContainmentType.Disjoint;
            }
        }
        return ContainmentType.Contains;
    }

    public final ContainmentType Contains(BoundingSphere sphere) {
        Vector3 center = sphere.Center;
        float radius = sphere.Radius;
        int inside = 0;
        for (Plane plane : planes) {
            float dot = (plane.Normal.X * center.X)
                    + (plane.Normal.Y * center.Y) + (plane.Normal.Z * center.Z);
            float distance = dot + plane.D;
            if (distance > radius) {
                return ContainmentType.Disjoint;
            }
            if (distance < -radius) {
                inside++;
            }
        }
        return inside == 6 ? ContainmentType.Contains : ContainmentType.Intersects;
    }

    private void ensureGjk() {
        if (gjk == null) {
            gjk = new Gjk();
        }
    }

    private void supportMapping(Vector3 direction, Vector3 result) {
        int selected = 0;
        float best = Vector3.Dot(corners[0], direction);
        for (int i = 1; i < corners.length; i++) {
            float dot = Vector3.Dot(corners[i], direction);
            if (dot > best) {
                selected = i;
                best = dot;
            }
        }
        Vector3 corner = corners[selected];
        result.X = corner.X;
        result.Y = corner.Y;
        result.Z = corner.Z;
    }

    public final boolean equals(BoundingFrustum other) {
        return other != null && matrix.equals(other.matrix);
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof BoundingFrustum value && equals(value);
    }

    @Override
    public int hashCode() {
        return matrix.hashCode();
    }

    @Override
    public String toString() {
        return "{Near:" + planes[0] + " Far:" + planes[1]
                + " Left:" + planes[2] + " Right:" + planes[3]
                + " Top:" + planes[4] + " Bottom:" + planes[5] + '}';
    }
}
