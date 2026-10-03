package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.GamerServices.Gamer;
import org.openeggbert.cna.internal.GamerHandles;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;

import java.time.Duration;
import java.util.Objects;

/** One gamer in a network session, local or remote. */
public class NetworkGamer extends Gamer {

    private final long handle;

    NetworkGamer(long handle) {
        this.handle = handle;
        GamerHandles.register(this, handle);
    }

    final long handle() {
        return handle;
    }

    public final boolean getHasLeftSession() {
        return flag("HasLeftSession",
                value -> NativeNetworkRoutes.networkGamerGetHasLeftSession(handle(), value));
    }

    public final boolean getHasVoice() {
        return flag("HasVoice",
                value -> NativeNetworkRoutes.networkGamerGetHasVoice(handle(), value));
    }

    /** Returns the session-unique gamer id, which XNA reports as an unsigned byte. */
    public final int getId() {
        byte[] value = new byte[1];
        NativeGamerServices.check("NetworkGamer.Id",
                NativeNetworkRoutes.networkGamerGetId(handle(), value));
        return value[0] & 0xFF;
    }

    public final boolean getIsGuest() {
        return flag("IsGuest",
                value -> NativeNetworkRoutes.networkGamerGetIsGuest(handle(), value));
    }

    public final boolean getIsHost() {
        return flag("IsHost",
                value -> NativeNetworkRoutes.networkGamerGetIsHost(handle(), value));
    }

    public final boolean getIsLocal() {
        return flag("IsLocal",
                value -> NativeNetworkRoutes.networkGamerGetIsLocal(handle(), value));
    }

    public final boolean getIsMutedByLocalUser() {
        return flag("IsMutedByLocalUser",
                value -> NativeNetworkRoutes.networkGamerGetIsMutedByLocalUser(handle(), value));
    }

    public final boolean getIsPrivateSlot() {
        return flag("IsPrivateSlot",
                value -> NativeNetworkRoutes.networkGamerGetIsPrivateSlot(handle(), value));
    }

    public final boolean getIsReady() {
        return flag("IsReady",
                value -> NativeNetworkRoutes.networkGamerGetIsReady(handle(), value));
    }

    public final void setIsReady(boolean value) {
        NativeGamerServices.check("NetworkGamer.IsReady",
                NativeNetworkRoutes.networkGamerSetIsReady(handle(), value));
    }

    public final boolean getIsTalking() {
        return flag("IsTalking",
                value -> NativeNetworkRoutes.networkGamerGetIsTalking(handle(), value));
    }

    public final NetworkMachine getMachine() {
        long[] machine = new long[1];
        NativeGamerServices.check("NetworkGamer.Machine",
                NativeNetworkRoutes.networkGamerCopyMachine(handle(), machine));
        return machine[0] == 0L ? null : new NetworkMachine(machine[0]);
    }

    protected final void setMachine(NetworkMachine value) {
        NativeGamerServices.check("NetworkGamer.Machine",
                NativeNetworkRoutes.networkGamerSetMachine(handle(),
                        Objects.requireNonNull(value, "value").handle()));
    }

    public final Duration getRoundtripTime() {
        long[] ticks = new long[1];
        NativeGamerServices.check("NetworkGamer.RoundtripTime",
                NativeNetworkRoutes.networkGamerGetRoundtripTicks(handle(), ticks));
        return NativeGamerServices.duration(ticks[0]);
    }

    public final NetworkSession getSession() {
        long[] session = new long[1];
        NativeGamerServices.check("NetworkGamer.Session",
                NativeNetworkRoutes.networkGamerGetSession(handle(), session));
        return session[0] == 0L ? null : NetworkSession.borrowed(session[0]);
    }

    private interface Flag {
        int read(boolean[] value);
    }

    private static boolean flag(String property, Flag reader) {
        boolean[] value = new boolean[1];
        NativeGamerServices.check("NetworkGamer." + property, reader.read(value));
        return value[0];
    }
}
