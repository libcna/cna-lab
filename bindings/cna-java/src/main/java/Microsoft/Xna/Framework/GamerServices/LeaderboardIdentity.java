package Microsoft.Xna.Framework.GamerServices;

import java.util.Objects;

/**
 * Mutable value naming one leaderboard and the game mode it scores.
 *
 * <p>XNA declares this as a struct. {@code Create} is the factory the framework supplies; the
 * parameterless constructor produces the zeroed struct, whose key is {@code null} and whose
 * game mode is zero.
 */
public final class LeaderboardIdentity {

    private String key;
    private int gameMode;

    public LeaderboardIdentity() {
    }

    public LeaderboardIdentity(LeaderboardIdentity value) {
        LeaderboardIdentity source = Objects.requireNonNull(value, "value");
        key = source.key;
        gameMode = source.gameMode;
    }

    public static LeaderboardIdentity Create(LeaderboardKey key, int gameMode) {
        LeaderboardIdentity identity = new LeaderboardIdentity();
        identity.key = Objects.requireNonNull(key, "key").name();
        identity.gameMode = gameMode;
        return identity;
    }

    public static LeaderboardIdentity Create(LeaderboardKey key) {
        return Create(key, 0);
    }

    public int getGameMode() {
        return gameMode;
    }

    public void setGameMode(int value) {
        gameMode = value;
    }

    public String getKey() {
        return key;
    }

    public void setKey(String value) {
        key = value;
    }
}
