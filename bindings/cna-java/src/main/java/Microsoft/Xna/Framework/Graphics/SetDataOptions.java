package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Immutable flags projection for dynamic-buffer upload behavior. */
public final class SetDataOptions {

    public static final SetDataOptions None = new SetDataOptions(0);
    public static final SetDataOptions Discard = new SetDataOptions(1);
    public static final SetDataOptions NoOverwrite = new SetDataOptions(2);

    private final int value;

    private SetDataOptions(int value) {
        this.value = value;
    }

    public static SetDataOptions FromValue(int value) {
        return switch (value) {
            case 0 -> None;
            case 1 -> Discard;
            case 2 -> NoOverwrite;
            default -> new SetDataOptions(value);
        };
    }

    public int getValue() {
        return value;
    }

    public SetDataOptions Or(SetDataOptions other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(SetDataOptions value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof SetDataOptions other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
