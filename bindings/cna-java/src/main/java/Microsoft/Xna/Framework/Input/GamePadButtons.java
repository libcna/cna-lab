package Microsoft.Xna.Framework.Input;

import java.util.Objects;

/** Immutable snapshot of the physical XNA controller buttons. */
public final class GamePadButtons {

    static final int NORMAL_BUTTON_MASK = 64511;

    private final ButtonState a;
    private final ButtonState b;
    private final ButtonState x;
    private final ButtonState y;
    private final ButtonState leftStick;
    private final ButtonState rightStick;
    private final ButtonState leftShoulder;
    private final ButtonState rightShoulder;
    private final ButtonState back;
    private final ButtonState start;
    private final ButtonState bigButton;

    public GamePadButtons() {
        this(Buttons.FromValue(0));
    }

    public GamePadButtons(GamePadButtons value) {
        this(Objects.requireNonNull(value, "value").mask());
    }

    public GamePadButtons(Buttons buttons) {
        this(Objects.requireNonNull(buttons, "buttons").getValue());
    }

    private GamePadButtons(int buttons) {
        a = state(buttons, Buttons.A);
        b = state(buttons, Buttons.B);
        x = state(buttons, Buttons.X);
        y = state(buttons, Buttons.Y);
        leftStick = state(buttons, Buttons.LeftStick);
        rightStick = state(buttons, Buttons.RightStick);
        leftShoulder = state(buttons, Buttons.LeftShoulder);
        rightShoulder = state(buttons, Buttons.RightShoulder);
        back = state(buttons, Buttons.Back);
        start = state(buttons, Buttons.Start);
        bigButton = state(buttons, Buttons.BigButton);
    }

    public ButtonState getA() { return a; }
    public ButtonState getB() { return b; }
    public ButtonState getBack() { return back; }
    public ButtonState getBigButton() { return bigButton; }
    public ButtonState getLeftShoulder() { return leftShoulder; }
    public ButtonState getLeftStick() { return leftStick; }
    public ButtonState getRightShoulder() { return rightShoulder; }
    public ButtonState getRightStick() { return rightStick; }
    public ButtonState getStart() { return start; }
    public ButtonState getX() { return x; }
    public ButtonState getY() { return y; }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof GamePadButtons other
                && a == other.a && b == other.b && x == other.x && y == other.y
                && leftShoulder == other.leftShoulder && leftStick == other.leftStick
                && rightShoulder == other.rightShoulder && rightStick == other.rightStick
                && back == other.back && start == other.start && bigButton == other.bigButton;
    }

    @Override
    public int hashCode() {
        int hash = a.ordinal() ^ b.ordinal() ^ x.ordinal() ^ y.ordinal()
                ^ leftStick.ordinal() ^ rightStick.ordinal()
                ^ leftShoulder.ordinal() ^ rightShoulder.ordinal()
                ^ back.ordinal() ^ start.ordinal() ^ bigButton.ordinal();
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        StringBuilder names = new StringBuilder();
        append(names, a, "A");
        append(names, b, "B");
        append(names, x, "X");
        append(names, y, "Y");
        append(names, leftShoulder, "LeftShoulder");
        append(names, rightShoulder, "RightShoulder");
        append(names, leftStick, "LeftStick");
        append(names, rightStick, "RightStick");
        append(names, start, "Start");
        append(names, back, "Back");
        append(names, bigButton, "BigButton");
        return "{Buttons:" + (names.length() == 0 ? "None" : names) + '}';
    }

    int mask() {
        int result = 0;
        result |= bit(a, Buttons.A);
        result |= bit(b, Buttons.B);
        result |= bit(x, Buttons.X);
        result |= bit(y, Buttons.Y);
        result |= bit(leftStick, Buttons.LeftStick);
        result |= bit(rightStick, Buttons.RightStick);
        result |= bit(leftShoulder, Buttons.LeftShoulder);
        result |= bit(rightShoulder, Buttons.RightShoulder);
        result |= bit(back, Buttons.Back);
        result |= bit(start, Buttons.Start);
        result |= bit(bigButton, Buttons.BigButton);
        return result;
    }

    private static ButtonState state(int buttons, Buttons button) {
        return (buttons & button.getValue()) == button.getValue()
                ? ButtonState.Pressed : ButtonState.Released;
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
