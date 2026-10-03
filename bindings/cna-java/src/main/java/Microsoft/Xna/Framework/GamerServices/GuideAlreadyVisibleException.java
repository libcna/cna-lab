package Microsoft.Xna.Framework.GamerServices;

/** Thrown when a Guide screen is requested while another one is already visible. */
public class GuideAlreadyVisibleException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public GuideAlreadyVisibleException() {
    }

    public GuideAlreadyVisibleException(String message, RuntimeException innerException) {
        super(message, innerException);
    }

    public GuideAlreadyVisibleException(String message) {
        super(message);
    }
}
