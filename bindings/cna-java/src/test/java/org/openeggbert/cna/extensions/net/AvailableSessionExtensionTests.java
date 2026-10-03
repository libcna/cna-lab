package org.openeggbert.cna.extensions.net;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Net.AvailableNetworkSession;
import Microsoft.Xna.Framework.Net.AvailableNetworkSessionCollection;
import Microsoft.Xna.Framework.Net.NetworkSession;
import Microsoft.Xna.Framework.Net.NetworkSessionType;
import org.junit.jupiter.api.Test;

import java.util.List;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Where a session is, which XNA's discovered session has no member for.
 *
 * <p>The two halves are tested together because each is the other's evidence: a description built
 * from an endpoint is read back through the accessors, so a mistake in either shows. A search
 * cannot supply the evidence here -- see {@link #aSearchOnThisRuntimeFindsNothing()} -- and a
 * description is not a substitute for one, only for the accessors.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class AvailableSessionExtensionTests {

    @Test
    void aSessionDescribedByAddressReadsBackAsOne() {
        run(AvailableSessionExtensionTests::describedSession);
    }

    @Test
    void aDescribedSessionGivesItsHandleBack() {
        run(AvailableSessionExtensionTests::handlesGoBack);
    }

    @Test
    void aSearchOnThisRuntimeFindsNothing() {
        run(AvailableSessionExtensionTests::search);
    }

    private static void run(Runnable body) {
        try (Game game = new Game()) {
            Probe probe = new Probe(game, body);
            game.getComponents().add(probe);
            game.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    /** Runs one body inside a frame, because CNA's session layer needs a live game. */
    /**
     * Drives the process-wide dispatcher as GamerServicesComponent does, initializing it only if
     * no earlier suite in this JVM has: XNA refuses a second initialization.
     */
    private static final class Probe extends Microsoft.Xna.Framework.GameComponent {

        private final Runnable body;
        private boolean ran;
        private Throwable failure;

        private Probe(Game game, Runnable body) {
            super(game);
            this.body = body;
        }

        @Override
        public void Initialize() {
            if (!Microsoft.Xna.Framework.GamerServices.GamerServicesDispatcher.getIsInitialized()) {
                Microsoft.Xna.Framework.GamerServices.GamerServicesDispatcher.Initialize(
                        getGame().getServices());
            }
            super.Initialize();
        }

        @Override
        public void Update(GameTime gameTime) {
            Microsoft.Xna.Framework.GamerServices.GamerServicesDispatcher.Update();
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                body.run();
            } catch (Throwable exception) {
                failure = exception;
            }
        }
    }

    private static void describedSession() {
        SessionEndpoint endpoint = new SessionEndpoint("127.0.0.1", 27015);
        AvailableNetworkSession described = SessionDirectory.atEndpoint(
                endpoint, NetworkSessionType.SystemLink, "Host");

        // The whole point of the slice: the address goes in and comes back out, through the
        // XNA type that has no member for it.
        assertEquals(endpoint, AvailableSessionExtensions.getConnectEndpoint(described));
        assertEquals(NetworkSessionType.SystemLink,
                AvailableSessionExtensions.getSessionType(described));
        assertEquals("Host", described.getHostGamertag());

        // Nothing has spoken to the host, so the counts are zero rather than invented.
        assertEquals(0, described.getCurrentGamerCount());
        assertEquals(0, described.getOpenPublicGamerSlots());
        assertEquals(0, described.getOpenPrivateGamerSlots());

        // CNA reports quality of service as available even though nothing measured it. That
        // is CNA's own answer for a session built rather than found, and it is asserted here
        // rather than corrected, because a projection that quietly said false would be
        // reporting a measurement of its own.
        assertTrue(described.getQualityOfService().getIsAvailable(),
                "CNA reports availability true for a described session");

        AvailableNetworkSession empty = SessionDirectory.atEndpoint(
                new SessionEndpoint("::1", 0), NetworkSessionType.PlayerMatch, "");
        assertEquals("", empty.getHostGamertag());
        assertEquals(new SessionEndpoint("::1", 0),
                AvailableSessionExtensions.getConnectEndpoint(empty));

        assertThrows(IllegalArgumentException.class, () -> SessionDirectory.atEndpoint(
                new SessionEndpoint("", 27015), NetworkSessionType.SystemLink, ""));
        assertThrows(IllegalArgumentException.class, () -> SessionDirectory.atEndpoint(
                new SessionEndpoint("127.0.0.1", 65536), NetworkSessionType.SystemLink, ""));
        assertThrows(NullPointerException.class,
                () -> AvailableSessionExtensions.getConnectEndpoint(null));
        assertThrows(NullPointerException.class,
                () -> AvailableSessionExtensions.getSessionType(null));

        // A described session is what Join takes. CNA's SystemLink transport now contacts the
        // host (GS-007n), and nothing listens at this address on the qualification host, so the
        // join is refused with XNA's own exception rather than succeeding against nobody.
        assertThrows(Microsoft.Xna.Framework.Net.NetworkSessionJoinException.class,
                () -> NetworkSession.Join(described));
    }

    /**
     * Proves a discovered session hands its native handle back.
     *
     * <p>Every {@code AvailableNetworkSession} owns one: CNA documents the collection accessor as
     * returning "an independent copy" that "stays valid after the collection is disposed", and
     * the direct constructor returns a new handle too. XNA's type is not disposable, so a game
     * has nowhere to release it, and until this was written nothing did -- every discovered
     * session a search returned leaked. The release is deferred because CNA refuses it from any
     * thread but the one that created the object.
     */
    private static void handlesGoBack() {
        int before = org.openeggbert.cna.internal.NativeDeferredRelease.pendingCount();
        for (int index = 0; index < 32; index++) {
            AvailableNetworkSession discarded = SessionDirectory.atEndpoint(
                    new SessionEndpoint("127.0.0." + (index + 1), 27015),
                    NetworkSessionType.SystemLink, "Host" + index);
            assertEquals(27015, AvailableSessionExtensions
                    .getConnectEndpoint(discarded).Port());
        }
        int released = 0;
        for (int attempt = 0; attempt < 50 && released == 0; attempt++) {
            System.gc();
            try {
                Thread.sleep(10L);
            } catch (InterruptedException interrupted) {
                Thread.currentThread().interrupt();
                break;
            }
            int owed = org.openeggbert.cna.internal.NativeDeferredRelease.pendingCount();
            if (owed > before) {
                Microsoft.Xna.Framework.FrameworkDispatcher.Update();
                released = owed - before;
            }
        }
        assertTrue(released > 0,
                "a collected discovered session must hand its native handle back");
        assertEquals(0, org.openeggbert.cna.internal.NativeDeferredRelease.pendingCount(),
                "the drain must leave nothing owed");
    }

    private static void search() {
        // Recorded rather than asserted away: this qualification runs with no peer on the
        // network, so every search returns nothing and no real discovered session exists to
        // read. A machine with a second host would find one; the accessors are the same
        // either way, which is what the endpoint probe establishes.
        assertThrows(RuntimeException.class,
                () -> NetworkSession.Find(NetworkSessionType.Local, 1, null),
                "CNA refuses a search for local sessions, which are not discoverable");
        // SystemLink searches the local network, and with no peer finds nothing.
        try (AvailableNetworkSessionCollection found =
                     NetworkSession.Find(NetworkSessionType.SystemLink, 1, null)) {
            assertEquals(0, found.size(), "a SystemLink search found a session on a network with "
                    + "no peer, which this qualification cannot have");
        }
        // PlayerMatch and Ranked need a configured CNA account service; with none CNA refuses,
        // which is XNA's GamerServicesNotAvailable rather than an empty result.
        for (NetworkSessionType type : List.of(NetworkSessionType.PlayerMatch,
                NetworkSessionType.Ranked)) {
            assertThrows(Microsoft.Xna.Framework.GamerServices.GamerServicesNotAvailableException.class,
                    () -> NetworkSession.Find(type, 1, null), type.toString());
        }
    }
}
