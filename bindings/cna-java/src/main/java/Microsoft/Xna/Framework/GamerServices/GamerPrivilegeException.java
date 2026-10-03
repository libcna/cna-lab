package Microsoft.Xna.Framework.GamerServices;

/** Thrown when a gamer's privileges forbid the requested operation. */
public class GamerPrivilegeException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public GamerPrivilegeException() {
    }

    public GamerPrivilegeException(String message, RuntimeException innerException) {
        super(message, innerException);
    }

    public GamerPrivilegeException(String message) {
        super(message);
    }
}
