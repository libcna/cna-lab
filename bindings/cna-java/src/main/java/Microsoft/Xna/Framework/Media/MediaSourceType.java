package Microsoft.Xna.Framework.Media;

/** Kind of platform media source. */
public enum MediaSourceType {
    LocalDevice(0),
    WindowsMediaConnect(4);

    private final int value;

    MediaSourceType(int value) { this.value = value; }

    public int getValue() { return value; }

    static MediaSourceType fromValue(int value) {
        for (MediaSourceType type : values()) if (type.value == value) return type;
        throw new IllegalStateException("CNA returned unknown MediaSourceType " + value);
    }
}
