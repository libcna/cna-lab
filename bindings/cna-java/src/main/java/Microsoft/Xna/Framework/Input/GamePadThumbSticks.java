package Microsoft.Xna.Framework.Input;

import Microsoft.Xna.Framework.Vector2;

import java.util.Objects;

/** Immutable snapshot of the two XNA controller thumb sticks. */
public final class GamePadThumbSticks {

    private final Vector2 left;
    private final Vector2 right;

    public GamePadThumbSticks() {
        this(new Vector2(), new Vector2());
    }

    public GamePadThumbSticks(GamePadThumbSticks value) {
        this(Objects.requireNonNull(value, "value").left, value.right);
    }

    public GamePadThumbSticks(Vector2 leftThumbstick, Vector2 rightThumbstick) {
        Vector2 leftValue = new Vector2(Objects.requireNonNull(leftThumbstick, "leftThumbstick"));
        Vector2 rightValue = new Vector2(Objects.requireNonNull(rightThumbstick, "rightThumbstick"));
        leftValue = Vector2.Min(leftValue, Vector2.getOne());
        left = Vector2.Max(leftValue, Vector2.Negate(Vector2.getOne()));
        rightValue = Vector2.Min(rightValue, Vector2.getOne());
        right = Vector2.Max(rightValue, Vector2.Negate(Vector2.getOne()));
    }

    public Vector2 getLeft() {
        return new Vector2(left);
    }

    public Vector2 getRight() {
        return new Vector2(right);
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof GamePadThumbSticks other
                && left.equals(other.left) && right.equals(other.right);
    }

    @Override
    public int hashCode() {
        int hash = Float.floatToRawIntBits(left.X) ^ Float.floatToRawIntBits(left.Y)
                ^ Float.floatToRawIntBits(right.X) ^ Float.floatToRawIntBits(right.Y);
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        return "{Left:" + left + " Right:" + right + '}';
    }
}
