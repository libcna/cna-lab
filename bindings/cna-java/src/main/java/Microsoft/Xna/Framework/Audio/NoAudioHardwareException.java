package Microsoft.Xna.Framework.Audio;

/** The runtime could not provide an audio output device. */
public final class NoAudioHardwareException extends RuntimeException {
    private static final long serialVersionUID = 1L;
    public NoAudioHardwareException() { }
    public NoAudioHardwareException(String message) { super(message); }
    public NoAudioHardwareException(String message, RuntimeException inner) {
        super(message, inner);
    }
}
