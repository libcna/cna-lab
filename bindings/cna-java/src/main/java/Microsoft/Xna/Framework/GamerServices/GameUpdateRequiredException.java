package Microsoft.Xna.Framework.GamerServices;

/** Thrown when a title update must be installed before the requested Live operation can run. */
public class GameUpdateRequiredException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public GameUpdateRequiredException() {
    }

    public GameUpdateRequiredException(String message, RuntimeException innerException) {
        super(message, innerException);
    }

    public GameUpdateRequiredException(String message) {
        super(message);
    }
}
