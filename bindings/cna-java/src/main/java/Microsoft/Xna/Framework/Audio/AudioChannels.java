package Microsoft.Xna.Framework.Audio;

/** Number of interleaved PCM channels. */
public enum AudioChannels {
    Mono(1),
    Stereo(2);

    private final int value;

    AudioChannels(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
