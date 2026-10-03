package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.PlayerIndex;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.util.Objects;

/**
 * The gamers signed in on this machine, addressable by player slot as well as by index.
 *
 * <p>This collection is not a snapshot: it reads the platform on each access, which is what
 * XNA's {@code Gamer.SignedInGamers} does, so a gamer who signs out between two reads
 * disappears from it.
 */
public final class SignedInGamerCollection extends GamerCollection<SignedInGamer> {

    private SignedInGamerCollection() {
        super(SignedInGamerCollection::slot, SignedInGamerCollection::count);
    }

    private static SignedInGamer slot(int index) {
        long[] gamer = new long[1];
        NativeGamerServices.check("Gamer.SignedInGamers",
                NativeGamerServicesRoutes.gamerGetSignedInGamerAt(index, gamer));
        return new SignedInGamer(gamer[0]);
    }

    private static int count() {
        int[] count = new int[1];
        NativeGamerServices.check("Gamer.SignedInGamers",
                NativeGamerServicesRoutes.gamerGetSignedInGamerCount(count));
        return count[0];
    }

    static SignedInGamerCollection current() {
        NativeGamerServices.requireAvailable("Gamer.SignedInGamers");
        return new SignedInGamerCollection();
    }

    /** Returns the gamer in this player slot, or {@code null} when the slot is empty. */
    public SignedInGamer get(PlayerIndex index) {
        Objects.requireNonNull(index, "index");
        boolean[] present = new boolean[1];
        long[] gamer = new long[1];
        NativeGamerServices.check("SignedInGamerCollection.get",
                NativeGamerServicesRoutes.gamerGetSignedInGamerAtPlayerIndex(
                        index.ordinal(), present, gamer));
        return present[0] && gamer[0] != 0L ? new SignedInGamer(gamer[0]) : null;
    }
}
