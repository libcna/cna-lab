package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.EventArgs;

/** Reports why the session ended. */
public class NetworkSessionEndedEventArgs extends EventArgs {

    private final NetworkSessionEndReason endReason;

    public NetworkSessionEndedEventArgs(NetworkSessionEndReason endReason) {
        this.endReason = endReason;
    }

    public final NetworkSessionEndReason getEndReason() {
        return endReason;
    }
}
