package Microsoft.Xna.Framework.Media;

/** Playback state reported by XNA media and video players. */
public enum MediaState {
    Stopped(0),
    Playing(1),
    Paused(2);

    private final int value;

    MediaState(int value) { this.value = value; }

    static MediaState fromValue(int value) {
        for (MediaState state : values()) if (state.value == value) return state;
        throw new IllegalStateException("CNA returned unknown MediaState " + value);
    }
}
