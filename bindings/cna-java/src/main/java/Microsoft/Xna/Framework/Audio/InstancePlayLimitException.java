package Microsoft.Xna.Framework.Audio;

/** Playback could not begin because the platform instance limit was reached. */
public final class InstancePlayLimitException extends RuntimeException {
    private static final long serialVersionUID = 1L;
    public InstancePlayLimitException() { }
    public InstancePlayLimitException(String message) { super(message); }
    public InstancePlayLimitException(String message, RuntimeException inner) {
        super(message, inner);
    }
}
