package Microsoft.Xna.Framework.GamerServices;

/** Thrown when a network operation fails. */
public class NetworkException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public NetworkException() {
    }

    public NetworkException(String message, RuntimeException innerException) {
        super(message, innerException);
    }

    public NetworkException(String message) {
        super(message);
    }
}
