package System.IO;

import java.util.Objects;

/** Immutable flags projection of CLR FileShare. */
public final class FileShare {
    public static final FileShare None = new FileShare(0);
    public static final FileShare Read = new FileShare(1);
    public static final FileShare Write = new FileShare(2);
    public static final FileShare ReadWrite = new FileShare(3);
    public static final FileShare Delete = new FileShare(4);
    public static final FileShare Inheritable = new FileShare(16);

    private static final int VALID_BITS = 1 | 2 | 4 | 16;
    private final int value;

    private FileShare(int value) {
        if ((value & ~VALID_BITS) != 0) throw new IllegalArgumentException("Unknown FileShare bits");
        this.value = value;
    }

    public static FileShare FromValue(int value) { return new FileShare(value); }
    public int getValue() { return value; }
    public FileShare Or(FileShare other) {
        return new FileShare(value | Objects.requireNonNull(other, "other").value);
    }
    public boolean Contains(FileShare other) {
        int selected = Objects.requireNonNull(other, "other").value;
        return (value & selected) == selected;
    }
    @Override public boolean equals(Object other) {
        return other instanceof FileShare share && share.value == value;
    }
    @Override public int hashCode() { return Integer.hashCode(value); }
}
