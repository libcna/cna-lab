package Microsoft.Xna.Framework.Content;

/** Java projection of XNA's content-load failure. */
public class ContentLoadException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public ContentLoadException() {
    }

    public ContentLoadException(String message) {
        super(message);
    }

    public ContentLoadException(String message, RuntimeException innerException) {
        super(message, innerException);
    }
}
