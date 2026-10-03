package Microsoft.Xna.Framework.GamerServices;

/** Thrown when the gamer services dispatcher has not been initialized, or the platform provides no gamer services. */
public class GamerServicesNotAvailableException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public GamerServicesNotAvailableException() {
    }

    public GamerServicesNotAvailableException(String message, RuntimeException innerException) {
        super(message, innerException);
    }

    public GamerServicesNotAvailableException(String message) {
        super(message);
    }
}
