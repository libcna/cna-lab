package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;

/**
 * The per-gamer set of leaderboard entries a title writes during an arbitrated session.
 *
 * <p>A title asks for one entry per leaderboard it scores, fills the entry's columns, and the
 * session writes them when the game ends. Asking twice for the same leaderboard returns the
 * same entry, which is what lets a title accumulate into it across a match.
 */
public final class LeaderboardWriter {

    private final Map<String, LeaderboardEntry> entries = new LinkedHashMap<>();

    public LeaderboardWriter() {
    }

    public LeaderboardEntry GetLeaderboard(LeaderboardIdentity leaderboardId) {
        Objects.requireNonNull(leaderboardId, "leaderboardId");
        String key = leaderboardId.getKey() + "/" + leaderboardId.getGameMode();
        synchronized (entries) {
            LeaderboardEntry existing = entries.get(key);
            if (existing != null) {
                return existing;
            }
            long[] entry = new long[1];
            NativeGamerServices.check("LeaderboardWriter.GetLeaderboard",
                    NativeGamerServicesRoutes.leaderboardEntryCreateExt(0L, 0L, 0, entry));
            LeaderboardEntry created = new LeaderboardEntry(entry[0]);
            entries.put(key, created);
            return created;
        }
    }
}
