package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Composable immutable projection of XNA's graphics-buffer clear flags. */
public final class ClearOptions {

    public static final ClearOptions Target = new ClearOptions(1);
    public static final ClearOptions DepthBuffer = new ClearOptions(2);
    public static final ClearOptions Stencil = new ClearOptions(4);

    private final int value;

    private ClearOptions(int value) {
        this.value = value;
    }

    public static ClearOptions FromValue(int value) {
        return switch (value) {
            case 1 -> Target;
            case 2 -> DepthBuffer;
            case 4 -> Stencil;
            default -> new ClearOptions(value);
        };
    }

    public int getValue() {
        return value;
    }

    public ClearOptions Or(ClearOptions other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(ClearOptions value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof ClearOptions other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
