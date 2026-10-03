package System.IO;

/** File access selection with the exact CLR numeric identities. */
public enum FileAccess {
    Read(1), Write(2), ReadWrite(3);

    private final int value;
    FileAccess(int value) { this.value = value; }
    public int getValue() { return value; }
}
