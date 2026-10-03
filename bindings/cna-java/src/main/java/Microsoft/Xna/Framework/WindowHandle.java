package Microsoft.Xna.Framework;

/**
 * Opaque Java projection of XNA's platform {@code IntPtr} window token.
 *
 * <p>The numeric token is deliberately unavailable to application code. It is meaningful only
 * for round-tripping to other mapped XNA window properties.</p>
 */
public final class WindowHandle {

    public static final WindowHandle Zero = new WindowHandle(0L);

    private final long value;

    WindowHandle(long value) {
        this.value = value;
    }

    public boolean getIsZero() {
        return value == 0L;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof WindowHandle other && value == other.value;
    }

    @Override
    public int hashCode() {
        return Long.hashCode(value);
    }

    long value() {
        return value;
    }
}
