package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.GamerServices.NetworkException;

/** Thrown when joining a network session fails, naming why. */
public class NetworkSessionJoinException extends NetworkException {

    private static final long serialVersionUID = 1L;

    private NetworkSessionJoinError joinError = NetworkSessionJoinError.SessionNotFound;

    public NetworkSessionJoinException() {
    }

    public NetworkSessionJoinException(String message, RuntimeException innerException) {
        super(message, innerException);
    }

    public NetworkSessionJoinException(String message, NetworkSessionJoinError joinError) {
        super(message);
        this.joinError = joinError;
    }

    public NetworkSessionJoinException(String message) {
        super(message);
    }

    public final NetworkSessionJoinError getJoinError() {
        return joinError;
    }

    public final void setJoinError(NetworkSessionJoinError value) {
        joinError = value;
    }
}
