package Microsoft.Xna.Framework.Input;

import java.util.Objects;

/** Immutable XNA mouse snapshot. */
public final class MouseState {

    private final int x;
    private final int y;
    private final int scrollWheelValue;
    private final ButtonState leftButton;
    private final ButtonState middleButton;
    private final ButtonState rightButton;
    private final ButtonState xButton1;
    private final ButtonState xButton2;

    public MouseState() {
        this(0, 0, 0, ButtonState.Released, ButtonState.Released,
                ButtonState.Released, ButtonState.Released, ButtonState.Released);
    }

    public MouseState(MouseState value) {
        this(Objects.requireNonNull(value, "value").x, value.y, value.scrollWheelValue,
                value.leftButton, value.middleButton, value.rightButton,
                value.xButton1, value.xButton2);
    }

    public MouseState(
            int x,
            int y,
            int scrollWheel,
            ButtonState leftButton,
            ButtonState middleButton,
            ButtonState rightButton,
            ButtonState xButton1,
            ButtonState xButton2) {
        this.x = x;
        this.y = y;
        scrollWheelValue = scrollWheel;
        this.leftButton = Objects.requireNonNull(leftButton, "leftButton");
        this.middleButton = Objects.requireNonNull(middleButton, "middleButton");
        this.rightButton = Objects.requireNonNull(rightButton, "rightButton");
        this.xButton1 = Objects.requireNonNull(xButton1, "xButton1");
        this.xButton2 = Objects.requireNonNull(xButton2, "xButton2");
    }

    MouseState(int[] nativeState) {
        this(nativeState[0], nativeState[1], nativeState[2],
                button(nativeState[3], 0), button(nativeState[3], 1),
                button(nativeState[3], 2), button(nativeState[3], 3),
                button(nativeState[3], 4));
    }

    public ButtonState getLeftButton() {
        return leftButton;
    }

    public ButtonState getMiddleButton() {
        return middleButton;
    }

    public ButtonState getRightButton() {
        return rightButton;
    }

    public int getScrollWheelValue() {
        return scrollWheelValue;
    }

    public int getX() {
        return x;
    }

    public ButtonState getXButton1() {
        return xButton1;
    }

    public ButtonState getXButton2() {
        return xButton2;
    }

    public int getY() {
        return y;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof MouseState other
                && x == other.x && y == other.y
                && scrollWheelValue == other.scrollWheelValue
                && leftButton == other.leftButton
                && middleButton == other.middleButton
                && rightButton == other.rightButton
                && xButton1 == other.xButton1
                && xButton2 == other.xButton2;
    }

    @Override
    public int hashCode() {
        return x ^ y ^ leftButton.ordinal() ^ rightButton.ordinal()
                ^ middleButton.ordinal() ^ xButton1.ordinal() ^ xButton2.ordinal()
                ^ scrollWheelValue;
    }

    @Override
    public String toString() {
        StringBuilder buttons = new StringBuilder();
        appendPressed(buttons, leftButton, "Left");
        appendPressed(buttons, rightButton, "Right");
        appendPressed(buttons, middleButton, "Middle");
        appendPressed(buttons, xButton1, "XButton1");
        appendPressed(buttons, xButton2, "XButton2");
        if (buttons.length() == 0) {
            buttons.append("None");
        }
        return "{X:" + x + " Y:" + y + " Buttons:" + buttons
                + " Wheel:" + scrollWheelValue + "}";
    }

    private static ButtonState button(int flags, int bit) {
        return (flags & (1 << bit)) == 0 ? ButtonState.Released : ButtonState.Pressed;
    }

    private static void appendPressed(
            StringBuilder destination, ButtonState state, String name) {
        if (state == ButtonState.Pressed) {
            if (destination.length() != 0) {
                destination.append(' ');
            }
            destination.append(name);
        }
    }
}
