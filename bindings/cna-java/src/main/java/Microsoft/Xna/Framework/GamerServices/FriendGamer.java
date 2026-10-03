package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

/** A gamer on a signed-in gamer's friends list, with that friendship's current state. */
public final class FriendGamer extends Gamer {

    FriendGamer(long handle) {
        super(handle);
    }

    public boolean getFriendRequestReceivedFrom() {
        return info()[0] != 0L;
    }

    public boolean getFriendRequestSentTo() {
        return info()[1] != 0L;
    }

    public boolean getHasVoice() {
        return info()[2] != 0L;
    }

    public boolean getInviteAccepted() {
        return info()[3] != 0L;
    }

    public boolean getInviteReceivedFrom() {
        return info()[4] != 0L;
    }

    public boolean getInviteRejected() {
        return info()[5] != 0L;
    }

    public boolean getInviteSentTo() {
        return info()[6] != 0L;
    }

    public boolean getIsAway() {
        return info()[7] != 0L;
    }

    public boolean getIsBusy() {
        return info()[8] != 0L;
    }

    public boolean getIsJoinable() {
        return info()[9] != 0L;
    }

    public boolean getIsOnline() {
        return info()[10] != 0L;
    }

    public boolean getIsPlaying() {
        return info()[11] != 0L;
    }

    /** Returns the presence text this friend publishes, which is empty when they publish none. */
    public String getPresence() {
        return NativeGamerServices.text("FriendGamer.Presence",
                out -> NativeGamerServicesRoutes.friendGamerGetPresenceSize(handle(), out),
                (buffer, out) -> NativeGamerServicesRoutes.friendGamerCopyPresence(
                        handle(), buffer, out));
    }

    private long[] info() {
        long[] values = new long[12];
        NativeGamerServices.check("FriendGamer",
                NativeGamerServicesRoutes.friendGamerGetInfo(handle(), new byte[4], values));
        return values;
    }
}
