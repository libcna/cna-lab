package Microsoft.Xna.Framework.Input;

import java.util.Objects;

/** Immutable snapshot of the four XNA directional-pad buttons. */
public final class GamePadDPad {

    private final ButtonState up;
    private final ButtonState right;
    private final ButtonState down;
    private final ButtonState left;

    public GamePadDPad() {
        this(ButtonState.Released, ButtonState.Released,
                ButtonState.Released, ButtonState.Released);
    }

    public GamePadDPad(GamePadDPad value) {
        this(Objects.requireNonNull(value, "value").up, value.down, value.left, value.right);
    }

    public GamePadDPad(
            ButtonState upValue,
            ButtonState downValue,
            ButtonState leftValue,
            ButtonState rightValue) {
        up = Objects.requireNonNull(upValue, "upValue");
        right = Objects.requireNonNull(rightValue, "rightValue");
        down = Objects.requireNonNull(downValue, "downValue");
        left = Objects.requireNonNull(leftValue, "leftValue");
    }

    public ButtonState getDown() { return down; }
    public ButtonState getLeft() { return left; }
    public ButtonState getRight() { return right; }
    public ButtonState getUp() { return up; }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof GamePadDPad other
                && up == other.up && down == other.down
                && left == other.left && right == other.right;
    }

    @Override
    public int hashCode() {
        int hash = up.ordinal() ^ right.ordinal() ^ down.ordinal() ^ left.ordinal();
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        StringBuilder names = new StringBuilder();
        append(names, up, "Up");
        append(names, down, "Down");
        append(names, left, "Left");
        append(names, right, "Right");
        return "{DPad:" + (names.length() == 0 ? "None" : names) + '}';
    }

    int mask() {
        int result = 0;
        result |= bit(up, Buttons.DPadUp);
        result |= bit(down, Buttons.DPadDown);
        result |= bit(left, Buttons.DPadLeft);
        result |= bit(right, Buttons.DPadRight);
        return result;
    }

    private static int bit(ButtonState state, Buttons button) {
        return state == ButtonState.Pressed ? button.getValue() : 0;
    }

    private static void append(StringBuilder destination, ButtonState state, String name) {
        if (state == ButtonState.Pressed) {
            if (destination.length() != 0) {
                destination.append(' ');
            }
            destination.append(name);
        }
    }
}
