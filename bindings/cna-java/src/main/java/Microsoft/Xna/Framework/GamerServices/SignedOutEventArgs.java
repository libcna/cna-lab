package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.EventArgs;

/** Names the gamer who has just signed out. */
public class SignedOutEventArgs extends EventArgs {

    private final SignedInGamer gamer;

    public SignedOutEventArgs(SignedInGamer gamer) {
        this.gamer = gamer;
    }

    public final SignedInGamer getGamer() {
        return gamer;
    }
}
