package org.openeggbert.cna.extensions.net;

import Microsoft.Xna.Framework.Net.AvailableNetworkSession;
import Microsoft.Xna.Framework.Net.NetworkSessionType;

import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;

import java.util.Objects;

/**
 * What a discovered session knows about itself that XNA's {@link AvailableNetworkSession} has no
 * member for.
 *
 * <p>A CNA extension. XNA's discovered session describes <em>who</em> is hosting and what joining
 * would cost -- the host's gamertag, the open slots, the quality of service -- and says nothing
 * about <em>where</em>, because on Xbox Live the transport found the session and the title never
 * needed the address. Off that network a title often does: a system-link lobby that shows which
 * machine a session is on, a tool that reports what a search actually found, a game that wants to
 * remember a host between runs.
 *
 * <p>These are read-only observations about a session the search returned. The handle behind one
 * belongs to the collection the search produced, so nothing here takes ownership of anything, and
 * a session read after its collection is gone is the caller's mistake, exactly as it is for the
 * XNA members.
 */
public final class AvailableSessionExtensions {

    private AvailableSessionExtensions() {
    }

    /**
     * Returns the host and port the session was found at.
     *
     * @param session a session a search returned
     * @return the endpoint, whose host may be empty when the transport does not name one
     */
    public static SessionEndpoint getConnectEndpoint(AvailableNetworkSession session) {
        long handle = handle(session);
        String host = NativeGamerServices.text("AvailableSessionExtensions.getConnectEndpoint",
                out -> NativeNetworkRoutes
                        .availableNetworkSessionGetConnectAddressSizeExt(handle, out),
                (buffer, out) -> NativeNetworkRoutes
                        .availableNetworkSessionCopyConnectAddressExt(handle, buffer, out));
        int[] port = new int[1];
        NativeGamerServices.check("AvailableSessionExtensions.getConnectEndpoint",
                NativeNetworkRoutes.availableNetworkSessionGetConnectPortExt(handle, port));
        return new SessionEndpoint(host, port[0]);
    }

    /**
     * Returns the session type the discovered session advertises.
     *
     * <p>The value is XNA's own {@link NetworkSessionType}; it is only the <em>question</em> that
     * XNA's discovered session has no member for. A search is given a type to look for, so a
     * title that searches for one type already knows the answer -- this is for the one that
     * searches more than once, or that displays what it found.
     *
     * @param session a session a search returned
     * @return the advertised type
     */
    public static NetworkSessionType getSessionType(AvailableNetworkSession session) {
        int[] value = new int[1];
        NativeGamerServices.check("AvailableSessionExtensions.getSessionType", NativeNetworkRoutes
                .availableNetworkSessionGetSessionTypeExt(handle(session), value));
        NetworkSessionType[] values = NetworkSessionType.values();
        if (value[0] < 0 || value[0] >= values.length) {
            throw new IllegalStateException(
                    "CNA reported network session type " + value[0]
                    + ", which XNA has no constant for");
        }
        return values[value[0]];
    }

    private static long handle(AvailableNetworkSession session) {
        NativeGamerServices.requireAvailable("AvailableSessionExtensions");
        Objects.requireNonNull(session, "session");
        return FacadeFactory.availableNetworkSessionHandle(session);
    }
}
