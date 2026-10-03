package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Composable immutable projection of XNA's {@code [Flags]} sprite-effects enum. */
public final class SpriteEffects {

    public static final SpriteEffects None = new SpriteEffects(0);
    public static final SpriteEffects FlipHorizontally = new SpriteEffects(1);
    public static final SpriteEffects FlipVertically = new SpriteEffects(2);

    private final int value;

    private SpriteEffects(int value) {
        this.value = value;
    }

    public static SpriteEffects FromValue(int value) {
        return switch (value) {
            case 0 -> None;
            case 1 -> FlipHorizontally;
            case 2 -> FlipVertically;
            default -> new SpriteEffects(value);
        };
    }

    public int getValue() {
        return value;
    }

    public SpriteEffects Or(SpriteEffects other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(SpriteEffects value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof SpriteEffects other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
