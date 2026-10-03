package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.EventArgs;

/** Names the gamer who was hosting the session and the one now hosting it. */
public class HostChangedEventArgs extends EventArgs {

    private final NetworkGamer oldHost;
    private final NetworkGamer newHost;

    public HostChangedEventArgs(NetworkGamer oldHost, NetworkGamer newHost) {
        this.oldHost = oldHost;
        this.newHost = newHost;
    }

    public final NetworkGamer getNewHost() {
        return newHost;
    }

    public final NetworkGamer getOldHost() {
        return oldHost;
    }
}
