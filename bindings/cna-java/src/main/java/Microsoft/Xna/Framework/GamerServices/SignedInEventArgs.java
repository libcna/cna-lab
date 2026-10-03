package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.EventArgs;

/** Names the gamer who has just signed in. */
public class SignedInEventArgs extends EventArgs {

    private final SignedInGamer gamer;

    public SignedInEventArgs(SignedInGamer gamer) {
        this.gamer = gamer;
    }

    public final SignedInGamer getGamer() {
        return gamer;
    }
}
