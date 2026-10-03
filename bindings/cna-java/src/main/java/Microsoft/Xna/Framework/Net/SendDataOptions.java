package Microsoft.Xna.Framework.Net;

import java.util.Objects;

/**
 * Composable immutable projection of XNA's {@code [Flags]} packet delivery options.
 *
 * <p>{@code ReliableInOrder} is the combination of {@code Reliable} and {@code InOrder}, which
 * is why this is a flags enum rather than an ordinary one, and why Java projects it as a value
 * class instead of an enum: a Java enum constant cannot represent an unnamed combination.
 */
public final class SendDataOptions {

    public static final SendDataOptions None = new SendDataOptions(0);
    public static final SendDataOptions Reliable = new SendDataOptions(1);
    public static final SendDataOptions InOrder = new SendDataOptions(2);
    public static final SendDataOptions ReliableInOrder = new SendDataOptions(3);
    public static final SendDataOptions Chat = new SendDataOptions(4);

    private final int value;

    private SendDataOptions(int value) {
        this.value = value;
    }

    public static SendDataOptions FromValue(int value) {
        return switch (value) {
            case 0 -> None;
            case 1 -> Reliable;
            case 2 -> InOrder;
            case 3 -> ReliableInOrder;
            case 4 -> Chat;
            default -> new SendDataOptions(value);
        };
    }

    public int getValue() {
        return value;
    }

    public SendDataOptions Or(SendDataOptions other) {
        return FromValue(value | Objects.requireNonNull(other, "other").value);
    }

    public boolean Contains(SendDataOptions value) {
        int mask = Objects.requireNonNull(value, "value").value;
        return (this.value & mask) == mask;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof SendDataOptions other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Integer.hashCode(value);
    }
}
