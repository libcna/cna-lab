package Microsoft.Xna.Framework.Graphics;

/** Java projection of XNA's device-lost failure. */
public final class DeviceLostException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public DeviceLostException() {
    }

    public DeviceLostException(String message) {
        super(message);
    }

    public DeviceLostException(String message, RuntimeException inner) {
        super(message, inner);
    }
}
