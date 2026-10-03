package Microsoft.Xna.Framework.Input;

import java.util.Objects;

/** Immutable snapshot of the two XNA controller triggers. */
public final class GamePadTriggers {

    private final float left;
    private final float right;

    public GamePadTriggers() {
        this(0.0f, 0.0f);
    }

    public GamePadTriggers(GamePadTriggers value) {
        this(Objects.requireNonNull(value, "value").left, value.right);
    }

    public GamePadTriggers(float leftTrigger, float rightTrigger) {
        left = Math.max(Math.min(leftTrigger, 1.0f), 0.0f);
        right = Math.max(Math.min(rightTrigger, 1.0f), 0.0f);
    }

    public float getLeft() {
        return left;
    }

    public float getRight() {
        return right;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof GamePadTriggers other
                && left == other.left && right == other.right;
    }

    @Override
    public int hashCode() {
        int hash = Float.floatToRawIntBits(left) ^ Float.floatToRawIntBits(right);
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        return "{Left:" + left + " Right:" + right + '}';
    }
}
