package Microsoft.Xna.Framework;

import java.util.Objects;

/** Composable immutable projection of XNA's {@code [Flags]} display-orientation enum. */
public final class DisplayOrientation {

    public static final DisplayOrientation Default = new DisplayOrientation(0);
    public static final DisplayOrientation LandscapeLeft = new DisplayOrientation(1);
    public static final DisplayOrientation LandscapeRight = new DisplayOrientation(2);
    public static final DisplayOrientation Portrait = new DisplayOrientation(4);

    private final int value;

    private DisplayOrientation(int value) {
        this.value = value;
    }

    public static DisplayOrientation FromValue(int value) {
        return switch (value) {
            case 0 -> Default;
            case 1 -> LandscapeLeft;
            case 2 -> LandscapeRight;
            case 4 -> Portrait;
            default -> new DisplayOrientation(value);
        };
    }

    public int getValue() {
        return value;
    }

    public DisplayOrientation Or(DisplayOrientation other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(DisplayOrientation value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof DisplayOrientation other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
