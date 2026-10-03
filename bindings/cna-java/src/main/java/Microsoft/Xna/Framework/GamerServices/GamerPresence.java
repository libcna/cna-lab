package Microsoft.Xna.Framework.GamerServices;

import java.util.Objects;

/**
 * The presence string and numeric value a signed-in gamer publishes to other players.
 *
 * <p>The object a {@link SignedInGamer} returns writes through: setting the mode or the value
 * publishes it, which is what assigning to XNA's {@code SignedInGamer.Presence} members does.
 */
public final class GamerPresence {

    private final SignedInGamer owner;
    private GamerPresenceMode presenceMode;
    private int presenceValue;

    private GamerPresence(SignedInGamer owner, GamerPresenceMode mode, int value) {
        this.owner = owner;
        presenceMode = mode;
        presenceValue = value;
    }

    static GamerPresence attached(SignedInGamer owner, long[] values) {
        return new GamerPresence(owner, GamerPresenceMode.values()[(int) values[0]],
                (int) values[1]);
    }

    public GamerPresenceMode getPresenceMode() {
        return presenceMode;
    }

    public void setPresenceMode(GamerPresenceMode value) {
        presenceMode = Objects.requireNonNull(value, "value");
        publish();
    }

    public int getPresenceValue() {
        return presenceValue;
    }

    public void setPresenceValue(int value) {
        presenceValue = value;
        publish();
    }

    private void publish() {
        if (owner != null) {
            owner.publishPresence(presenceMode, presenceValue);
        }
    }
}
