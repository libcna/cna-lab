package Microsoft.Xna.Framework.Audio;

/** No microphone is connected for an operation that requires one. */
public final class NoMicrophoneConnectedException extends RuntimeException {
    private static final long serialVersionUID = 1L;
    public NoMicrophoneConnectedException() { }
    public NoMicrophoneConnectedException(String message) { super(message); }
    public NoMicrophoneConnectedException(String message, RuntimeException inner) {
        super(message, inner);
    }
}
