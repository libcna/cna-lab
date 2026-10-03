package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.EventArgs;

/** Names the gamer who has left the session. */
public class GamerLeftEventArgs extends EventArgs {

    private final NetworkGamer gamer;

    public GamerLeftEventArgs(NetworkGamer gamer) {
        this.gamer = gamer;
    }

    public final NetworkGamer getGamer() {
        return gamer;
    }
}
