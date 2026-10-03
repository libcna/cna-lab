package Microsoft.Xna.Framework.Graphics;

/** Java projection of XNA's no-suitable-graphics-device failure. */
public final class NoSuitableGraphicsDeviceException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public NoSuitableGraphicsDeviceException() {
    }

    public NoSuitableGraphicsDeviceException(String message) {
        super(message);
    }

    public NoSuitableGraphicsDeviceException(String message, RuntimeException inner) {
        super(message, inner);
    }
}
