package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.EventArgs;

/** Names the gamer who has joined the session. */
public class GamerJoinedEventArgs extends EventArgs {

    private final NetworkGamer gamer;

    public GamerJoinedEventArgs(NetworkGamer gamer) {
        this.gamer = gamer;
    }

    public final NetworkGamer getGamer() {
        return gamer;
    }
}
