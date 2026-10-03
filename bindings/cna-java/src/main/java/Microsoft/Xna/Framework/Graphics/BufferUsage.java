package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Immutable flags projection describing how a GPU buffer may be used. */
public final class BufferUsage {

    public static final BufferUsage None = new BufferUsage(0);
    public static final BufferUsage WriteOnly = new BufferUsage(1);

    private final int value;

    private BufferUsage(int value) {
        this.value = value;
    }

    public static BufferUsage FromValue(int value) {
        return switch (value) {
            case 0 -> None;
            case 1 -> WriteOnly;
            default -> new BufferUsage(value);
        };
    }

    public int getValue() {
        return value;
    }

    public BufferUsage Or(BufferUsage other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(BufferUsage value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof BufferUsage other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
