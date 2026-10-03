package Microsoft.Xna.Framework.Input;

import java.util.Objects;

/** Composable immutable projection of XNA's controller-button flags. */
public final class Buttons {

    public static final Buttons DPadUp = new Buttons(1);
    public static final Buttons DPadDown = new Buttons(2);
    public static final Buttons DPadLeft = new Buttons(4);
    public static final Buttons DPadRight = new Buttons(8);
    public static final Buttons Start = new Buttons(16);
    public static final Buttons Back = new Buttons(32);
    public static final Buttons LeftStick = new Buttons(64);
    public static final Buttons RightStick = new Buttons(128);
    public static final Buttons LeftShoulder = new Buttons(256);
    public static final Buttons RightShoulder = new Buttons(512);
    public static final Buttons BigButton = new Buttons(2048);
    public static final Buttons A = new Buttons(4096);
    public static final Buttons B = new Buttons(8192);
    public static final Buttons X = new Buttons(16384);
    public static final Buttons Y = new Buttons(32768);
    public static final Buttons RightTrigger = new Buttons(4194304);
    public static final Buttons LeftTrigger = new Buttons(8388608);
    public static final Buttons RightThumbstickUp = new Buttons(16777216);
    public static final Buttons RightThumbstickDown = new Buttons(33554432);
    public static final Buttons RightThumbstickRight = new Buttons(67108864);
    public static final Buttons RightThumbstickLeft = new Buttons(134217728);
    public static final Buttons LeftThumbstickLeft = new Buttons(2097152);
    public static final Buttons LeftThumbstickUp = new Buttons(268435456);
    public static final Buttons LeftThumbstickDown = new Buttons(536870912);
    public static final Buttons LeftThumbstickRight = new Buttons(1073741824);

    private final int value;

    private Buttons(int value) {
        this.value = value;
    }

    public static Buttons FromValue(int value) {
        return switch (value) {
            case 1 -> DPadUp;
            case 2 -> DPadDown;
            case 4 -> DPadLeft;
            case 8 -> DPadRight;
            case 16 -> Start;
            case 32 -> Back;
            case 64 -> LeftStick;
            case 128 -> RightStick;
            case 256 -> LeftShoulder;
            case 512 -> RightShoulder;
            case 2048 -> BigButton;
            case 4096 -> A;
            case 8192 -> B;
            case 16384 -> X;
            case 32768 -> Y;
            case 2097152 -> LeftThumbstickLeft;
            case 4194304 -> RightTrigger;
            case 8388608 -> LeftTrigger;
            case 16777216 -> RightThumbstickUp;
            case 33554432 -> RightThumbstickDown;
            case 67108864 -> RightThumbstickRight;
            case 134217728 -> RightThumbstickLeft;
            case 268435456 -> LeftThumbstickUp;
            case 536870912 -> LeftThumbstickDown;
            case 1073741824 -> LeftThumbstickRight;
            default -> new Buttons(value);
        };
    }

    public int getValue() {
        return value;
    }

    public Buttons Or(Buttons other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(Buttons value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof Buttons other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
