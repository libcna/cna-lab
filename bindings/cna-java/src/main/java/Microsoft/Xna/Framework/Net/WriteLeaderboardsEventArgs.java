package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.EventArgs;

/**
 * Asks the title to write one gamer's leaderboard entries.
 *
 * <p>{@code IsLeaving} distinguishes a gamer who is leaving the session, whose entries must be
 * written now, from the end-of-game write for everyone still present.
 */
public final class WriteLeaderboardsEventArgs extends EventArgs {

    private final NetworkGamer gamer;
    private final boolean isLeaving;

    WriteLeaderboardsEventArgs(NetworkGamer gamer, boolean isLeaving) {
        this.gamer = gamer;
        this.isLeaving = isLeaving;
    }

    public NetworkGamer getGamer() {
        return gamer;
    }

    public boolean getIsLeaving() {
        return isLeaving;
    }
}
