package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.EventArgs;

/**
 * Names the gamer who accepted a game invitation.
 *
 * <p>{@code IsCurrentSession} tells the title whether the invitation was for the session it is
 * already in, in which case it must not tear that session down.
 */
public class InviteAcceptedEventArgs extends EventArgs {

    private final SignedInGamer gamer;
    private final boolean isCurrentSession;

    public InviteAcceptedEventArgs(SignedInGamer gamer, boolean isCurrentSession) {
        this.gamer = gamer;
        this.isCurrentSession = isCurrentSession;
    }

    public final SignedInGamer getGamer() {
        return gamer;
    }

    public final boolean getIsCurrentSession() {
        return isCurrentSession;
    }
}
