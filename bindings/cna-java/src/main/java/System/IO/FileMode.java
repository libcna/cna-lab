package System.IO;

/** File creation/open behavior with the exact CLR numeric identities. */
public enum FileMode {
    CreateNew(1), Create(2), Open(3), OpenOrCreate(4), Truncate(5), Append(6);

    private final int value;
    FileMode(int value) { this.value = value; }
    public int getValue() { return value; }
}
