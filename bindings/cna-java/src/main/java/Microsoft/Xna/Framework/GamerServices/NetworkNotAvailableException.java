package Microsoft.Xna.Framework.GamerServices;

/** Thrown when no network connection is available for the requested operation. */
public class NetworkNotAvailableException extends NetworkException {

    private static final long serialVersionUID = 1L;

    public NetworkNotAvailableException() {
    }

    public NetworkNotAvailableException(String message, RuntimeException innerException) {
        super(message, innerException);
    }

    public NetworkNotAvailableException(String message) {
        super(message);
    }
}
