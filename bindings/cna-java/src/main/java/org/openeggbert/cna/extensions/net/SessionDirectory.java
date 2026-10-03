package org.openeggbert.cna.extensions.net;

import Microsoft.Xna.Framework.Net.AvailableNetworkSession;
import Microsoft.Xna.Framework.Net.NetworkSession;
import Microsoft.Xna.Framework.Net.NetworkSessionType;

import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;

import java.nio.charset.StandardCharsets;
import java.util.Objects;

/**
 * Sessions a title already knows the address of.
 *
 * <p>A CNA extension, and the counterpart to {@link AvailableSessionExtensions}. XNA has exactly
 * one way to obtain something {@link NetworkSession#Join} accepts: run a search and pick a result.
 * That was the whole story on Xbox Live, where the service found the session. Off it, a title
 * routinely learns of a session some other way -- a player types a host, a launcher passes one on
 * the command line, the title's own web service returns a lobby list, a favourite is remembered
 * between runs -- and XNA gives it no way to turn that into something joinable.
 *
 * <p>{@link #atEndpoint} does. It describes a session at an address, and the result is an ordinary
 * {@link AvailableNetworkSession} that {@code NetworkSession.Join} takes like any other.
 *
 * <p><strong>What it is not.</strong> This is a description, not a discovery: nothing here
 * contacts the host, so the gamer counts a searched session carries are zero rather than
 * invented, and a title that wants them has to join and ask the session itself. CNA's own
 * quality of service still reports itself <em>available</em> with no measurement behind it,
 * which is CNA's answer and not something this projection corrects. Describing a host that is
 * not listening is not an error either -- the join is where that is found out.
 */
public final class SessionDirectory {

    private SessionDirectory() {
    }

    /**
     * Describes a session at a known address so it can be joined.
     *
     * @param endpoint where the host is listening
     * @param sessionType the kind of session to join it as
     * @param hostGamertag a label for the host, which may be empty when the title has none; it is
     *        carried through to {@link AvailableNetworkSession#getHostGamertag()} and is not
     *        verified against the host
     * @return a joinable description of that session
     */
    public static AvailableNetworkSession atEndpoint(
            SessionEndpoint endpoint, NetworkSessionType sessionType, String hostGamertag) {
        NativeGamerServices.requireAvailable("SessionDirectory.atEndpoint");
        Objects.requireNonNull(endpoint, "endpoint");
        Objects.requireNonNull(sessionType, "sessionType");
        Objects.requireNonNull(hostGamertag, "hostGamertag");
        Objects.requireNonNull(endpoint.Host(), "endpoint.Host");
        if (endpoint.Host().isEmpty()) {
            throw new IllegalArgumentException("a session's host must not be empty");
        }
        if (endpoint.Port() < 0 || endpoint.Port() > 0xFFFF) {
            throw new IllegalArgumentException(
                    "a session's port must be a 16-bit value, not " + endpoint.Port());
        }
        // The counts and the quality of service stay zero: nothing here has spoken to the host,
        // and reporting a number that was never measured would be worse than reporting none.
        long[] info = {0, 0, 0, sessionType.ordinal(), endpoint.Port(), 0};
        long[] session = new long[1];
        NativeGamerServices.check("SessionDirectory.atEndpoint",
                NativeNetworkRoutes.availableNetworkSessionCreateExt(
                        new byte[6], info,
                        hostGamertag.getBytes(StandardCharsets.UTF_8),
                        endpoint.Host().getBytes(StandardCharsets.UTF_8),
                        new byte[7], new long[5], session));
        return FacadeFactory.createAvailableNetworkSession(session[0]);
    }
}
