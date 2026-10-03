package Microsoft.Xna.Framework.Input.Touch;

import java.util.Objects;

/** Immutable bit-set projection of XNA's gesture-selection flags. */
public final class GestureType {

    public static final GestureType None = new GestureType(0);
    public static final GestureType Tap = new GestureType(1);
    public static final GestureType DoubleTap = new GestureType(2);
    public static final GestureType Hold = new GestureType(4);
    public static final GestureType HorizontalDrag = new GestureType(8);
    public static final GestureType VerticalDrag = new GestureType(16);
    public static final GestureType FreeDrag = new GestureType(32);
    public static final GestureType Pinch = new GestureType(64);
    public static final GestureType Flick = new GestureType(128);
    public static final GestureType DragComplete = new GestureType(256);
    public static final GestureType PinchComplete = new GestureType(512);

    private final int value;

    private GestureType(int value) {
        this.value = value;
    }

    public static GestureType FromValue(int value) {
        return switch (value) {
            case 0 -> None;
            case 1 -> Tap;
            case 2 -> DoubleTap;
            case 4 -> Hold;
            case 8 -> HorizontalDrag;
            case 16 -> VerticalDrag;
            case 32 -> FreeDrag;
            case 64 -> Pinch;
            case 128 -> Flick;
            case 256 -> DragComplete;
            case 512 -> PinchComplete;
            default -> new GestureType(value);
        };
    }

    public int getValue() {
        return value;
    }

    public GestureType Or(GestureType other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(GestureType value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof GestureType other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
