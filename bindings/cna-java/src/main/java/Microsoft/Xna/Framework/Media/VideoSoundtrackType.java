package Microsoft.Xna.Framework.Media;

/** Semantic role of a video's audio track. */
public enum VideoSoundtrackType {
    Music(0),
    Dialog(1),
    MusicAndDialog(2);

    private final int value;

    VideoSoundtrackType(int value) { this.value = value; }

    /**
     * Returns CNA's own identifier for this role.
     *
     * <p>Package-private on purpose: XNA's enumeration has no such member, and the numbers
     * are CNA's, not part of the API this type projects.
     */
    int value() { return value; }

    static VideoSoundtrackType fromValue(int value) {
        for (VideoSoundtrackType type : values()) if (type.value == value) return type;
        throw new IllegalStateException("CNA returned unknown VideoSoundtrackType " + value);
    }
}
