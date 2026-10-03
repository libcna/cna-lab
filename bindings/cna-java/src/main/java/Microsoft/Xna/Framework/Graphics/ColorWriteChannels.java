package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Composable immutable projection of XNA's color-write channel flags. */
public final class ColorWriteChannels {

    public static final ColorWriteChannels None = new ColorWriteChannels(0);
    public static final ColorWriteChannels Red = new ColorWriteChannels(1);
    public static final ColorWriteChannels Green = new ColorWriteChannels(2);
    public static final ColorWriteChannels Blue = new ColorWriteChannels(4);
    public static final ColorWriteChannels Alpha = new ColorWriteChannels(8);
    public static final ColorWriteChannels All = new ColorWriteChannels(15);

    private final int value;

    private ColorWriteChannels(int value) {
        this.value = value;
    }

    public static ColorWriteChannels FromValue(int value) {
        return switch (value) {
            case 0 -> None;
            case 1 -> Red;
            case 2 -> Green;
            case 4 -> Blue;
            case 8 -> Alpha;
            case 15 -> All;
            default -> new ColorWriteChannels(value);
        };
    }

    public int getValue() {
        return value;
    }

    public ColorWriteChannels Or(ColorWriteChannels other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(ColorWriteChannels value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof ColorWriteChannels other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
