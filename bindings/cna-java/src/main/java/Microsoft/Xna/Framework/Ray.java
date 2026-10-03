package Microsoft.Xna.Framework;

import java.util.Objects;

/** XNA ray. Constructor inputs are snapshotted. */
public final class Ray {

    public Vector3 Position;
    public Vector3 Direction;

    public Ray() {
        Position = new Vector3();
        Direction = new Vector3();
    }

    public Ray(Vector3 position, Vector3 direction) {
        Position = new Vector3(Objects.requireNonNull(position, "position"));
        Direction = new Vector3(Objects.requireNonNull(direction, "direction"));
    }

    public Ray(Ray value) {
        this(Objects.requireNonNull(value, "value").Position, value.Direction);
    }

    public Float Intersects(BoundingBox box) {
        return box.Intersects(this);
    }

    public Float Intersects(BoundingFrustum frustum) {
        return Objects.requireNonNull(frustum, "frustum").Intersects(this);
    }

    public Float Intersects(Plane plane) {
        float denominator = (plane.Normal.X * Direction.X)
                + (plane.Normal.Y * Direction.Y) + (plane.Normal.Z * Direction.Z);
        if (Math.abs(denominator) < 1.0E-05f) {
            return null;
        }
        float positionDot = (plane.Normal.X * Position.X)
                + (plane.Normal.Y * Position.Y) + (plane.Normal.Z * Position.Z);
        float distance = (-plane.D - positionDot) / denominator;
        if (distance < 0.0f) {
            if (distance < -1.0E-05f) {
                return null;
            }
            distance = 0.0f;
        }
        return distance;
    }

    public Float Intersects(BoundingSphere sphere) {
        float x = sphere.Center.X - Position.X;
        float y = sphere.Center.Y - Position.Y;
        float z = sphere.Center.Z - Position.Z;
        float distanceSquared = (x * x) + (y * y) + (z * z);
        float radiusSquared = sphere.Radius * sphere.Radius;
        if (distanceSquared <= radiusSquared) {
            return 0.0f;
        }
        float along = (x * Direction.X) + (y * Direction.Y) + (z * Direction.Z);
        if (along < 0.0f) {
            return null;
        }
        float perpendicularSquared = distanceSquared - (along * along);
        if (perpendicularSquared > radiusSquared) {
            return null;
        }
        float offset = (float)Math.sqrt(radiusSquared - perpendicularSquared);
        return along - offset;
    }

    public boolean equals(Ray other) {
        return other != null && Position.equals(other.Position) && Direction.equals(other.Direction);
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Ray value && equals(value);
    }

    @Override
    public int hashCode() {
        return Position.hashCode() + Direction.hashCode();
    }

    @Override
    public String toString() {
        return "{Position:" + Position + " Direction:" + Direction + '}';
    }
}
