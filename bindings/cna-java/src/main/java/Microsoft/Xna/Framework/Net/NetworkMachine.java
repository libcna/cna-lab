package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.GamerServices.GamerCollection;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;



/**
 * One console in a session, and the gamers signed in on it.
 *
 * <p>{@code RemoveFromSession} is how a host evicts a whole machine, which is why the API is on
 * the machine rather than on each of its gamers.
 */
public final class NetworkMachine {

    private final long handle;

    NetworkMachine(long handle) {
        this.handle = handle;
    }

    long handle() {
        return handle;
    }

    public void RemoveFromSession() {
        NativeGamerServices.check("NetworkMachine.RemoveFromSession",
                NativeNetworkRoutes.networkMachineRemoveFromSession(handle));
    }

    /**
     * Returns the gamers signed in on this machine.
     *
     * <p>CNA reads them from the machine rather than from a gamer-collection handle, so the
     * collection is built over that live view.
     */
    public GamerCollection<NetworkGamer> getGamers() {
        return new MachineGamers(handle);
    }

    private static final class MachineGamers extends GamerCollection<NetworkGamer> {

        MachineGamers(long machine) {
            super(index -> {
                long[] gamer = new long[1];
                NativeGamerServices.check("NetworkMachine.Gamers",
                        NativeNetworkRoutes.networkMachineGetGamer(machine, index, gamer));
                return new NetworkGamer(gamer[0]);
            }, () -> {
                int[] count = new int[1];
                NativeGamerServices.check("NetworkMachine.Gamers",
                        NativeNetworkRoutes.networkMachineGetGamerCount(machine, count));
                return count[0];
            });
        }
    }
}
