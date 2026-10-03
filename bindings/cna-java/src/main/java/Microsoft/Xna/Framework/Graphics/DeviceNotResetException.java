package Microsoft.Xna.Framework.Graphics;

/** Java projection of XNA's device-not-reset failure. */
public final class DeviceNotResetException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public DeviceNotResetException() {
    }

    public DeviceNotResetException(String message) {
        super(message);
    }

    public DeviceNotResetException(String message, RuntimeException inner) {
        super(message, inner);
    }
}
