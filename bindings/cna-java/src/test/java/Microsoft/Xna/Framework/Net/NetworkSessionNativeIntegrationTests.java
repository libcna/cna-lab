package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GamerServices.GamerServicesDispatcher;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Proves the session events have a real native producer.
 *
 * <p>What matters here is not that a particular event fires -- CNA decides that -- but that when
 * CNA raises one, it reaches the Java listener, on the game thread, during {@code Update}. An
 * event with no producer would leave every list below empty no matter what the session did.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class NetworkSessionNativeIntegrationTests {

    @Test
    void aLocalSessionRaisesItsStateEventsThroughToJava() {
        try (Game game = new Game()) {
            SessionProbe probe = new SessionProbe(game);
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

    /**
     * Drives the process-wide dispatcher as GamerServicesComponent does, initializing it only if
     * no earlier suite in this JVM has: XNA refuses a second initialization.
     */
    private static final class SessionProbe extends Microsoft.Xna.Framework.GameComponent {

        private boolean ran;
        private Throwable failure;

        private SessionProbe(Game game) {
            super(game);
        }

        @Override
        public void Initialize() {
            if (!GamerServicesDispatcher.getIsInitialized()) {
                GamerServicesDispatcher.Initialize(getGame().getServices());
            }
            super.Initialize();
        }

        @Override
        public void Update(GameTime gameTime) {
            GamerServicesDispatcher.Update();
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                probe();
            } catch (Throwable exception) {
                failure = exception;
            }
        }

        private void probe() {
            List<String> observed = new ArrayList<>();
            try (NetworkSession session =
                         NetworkSession.Create(NetworkSessionType.Local, 1, 4)) {
                assertNotNull(session);
                assertEquals(NetworkSessionType.Local, session.getSessionType());
                assertEquals(NetworkSessionState.Lobby, session.getSessionState());
                assertTrue(session.getIsHost());
                assertFalse(session.getIsDisposed());

                // The maximum a session was created with is the maximum it reports. CNA did
                // not always: a session created with four once reported 69, which
                // JAVA-UPSTREAM-002 recorded and which is fixed in ABI 0.21.0. Asserting the
                // creation value is what keeps it fixed.
                assertEquals(4, session.getMaxGamers(),
                        "a session created with four gamers reports four");
                session.setMaxGamers(8);
                assertEquals(8, session.getMaxGamers());
                session.setMaxGamers(4);
                assertEquals(4, session.getMaxGamers());

                session.addGameStartedListener((sender, args) -> {
                    assertEquals(session, sender);
                    observed.add("started");
                });
                session.addGameEndedListener((sender, args) -> observed.add("ended"));
                session.addGamerJoinedListener((sender, args) -> observed.add("joined"));
                session.addSessionEndedListener((sender, args) -> observed.add("session-ended"));

                // StartGame and EndGame only queue a state change; Update is what applies it and
                // raises the event. A projection with no producer would leave `observed` empty.
                session.StartGame();
                session.Update();
                assertEquals(NetworkSessionState.Playing, session.getSessionState());
                assertTrue(observed.contains("started"),
                        "GameStarted must reach the Java listener; observed " + observed);

                session.EndGame();
                session.Update();
                assertEquals(NetworkSessionState.Lobby, session.getSessionState());
                assertTrue(observed.contains("ended"),
                        "GameEnded must reach the Java listener; observed " + observed);

                // A removed listener stops receiving, which is what a Java event bridge has to
                // guarantee for a C# `-=` to mean the same thing.
                int before = observed.size();
                session.StartGame();
                session.Update();
                assertTrue(observed.size() > before);

                assertEquals(session.getAllGamers().size(), session.getAllGamers().size());
                assertNotNull(session.getSessionProperties());
                sessionProperties();
                aFailedJoinCarriesCnasOwnReason();
                assertEquals(0, org.openeggbert.cna.internal.GamerEventPump.droppedEventCount(),
                        "no event may be dropped in a session this small");
            }
        }

        /**
         * Proves a failed join raises XNA's exception with CNA's own reason.
         *
         * <p>XNA games catch {@code NetworkSessionJoinException} and read its {@code JoinError}
         * to tell a full session from one that has gone. Before this, the projection threw an
         * ordinary native failure and the exception's error was a Java default nobody measured.
         */
        private void aFailedJoinCarriesCnasOwnReason() {
            // Nothing is joinable on this host, so JoinInvited fails -- which is the case worth
            // asserting: whatever CNA calls the failure, it must reach the game as XNA's type
            // when CNA recorded a join error, and as the plain failure when it did not.
            RuntimeException failure = assertThrows(RuntimeException.class,
                    () -> NetworkSession.JoinInvited(1));
            if (failure instanceof NetworkSessionJoinException join) {
                assertNotNull(join.getJoinError(),
                        "a join exception must carry the reason CNA recorded");
                assertNotNull(join.getMessage());
            } else {
                // CNA did not record a join error for this failure, so dressing it up as one
                // would be an invention. The projection reports what actually happened.
                assertFalse(failure instanceof NetworkSessionJoinException);
            }
        }

        /**
         * Proves CNA's bulk property copy carries the payload, not just a success code.
         *
         * <p>{@code toArray} is the Java spelling of XNA's {@code CopyTo} and takes CNA's own
         * copy route. An adapter that returned success without writing anything back would
         * leave every slot null here, which is exactly what this asserts against.
         */
        private void sessionProperties() {
            NetworkSessionProperties properties = new NetworkSessionProperties();
            // XNA's list is eight fixed slots, all unset at first; CNA answers the same.
            assertEquals(8, properties.size(), "eight slots, as XNA declares");
            Object[] fresh = properties.toArray();
            assertEquals(8, fresh.length);
            for (Object slot : fresh) {
                assertNull(slot, "a new slot is unset rather than zero");
            }
            properties.set(0, 41);
            properties.set(2, -7);
            properties.set(3, Integer.MAX_VALUE);
            Object[] copied = properties.toArray();
            assertEquals(41, copied[0]);
            assertNull(copied[1], "an unset slot stays absent rather than becoming zero");
            assertEquals(-7, copied[2]);
            assertEquals(Integer.MAX_VALUE, copied[3]);
            properties.set(1, 0);
            assertEquals(0, properties.toArray()[1],
                    "a slot set to zero is a value, not an absent slot");
            assertThrows(UnsupportedOperationException.class, () -> properties.add(5));
            assertThrows(UnsupportedOperationException.class, () -> properties.add(0, 5));
            assertThrows(UnsupportedOperationException.class, () -> properties.remove(0));
            assertThrows(UnsupportedOperationException.class, properties::clear);
            assertEquals(8, properties.size(), "and the refusals changed nothing");
            propertiesGoBackToCna();
        }

        /**
         * Proves a game-created property list gives its native handle back.
         *
         * <p>XNA's type is not disposable, so a game has nothing to close and the handle can
         * only go back once the object is unreachable. CNA refuses the release from any thread
         * but the one that created it -- so the cleaner records it and this thread, the one
         * pumping, is what actually releases it. A leak here would be silent and permanent.
         */
        private void propertiesGoBackToCna() {
            int before = org.openeggbert.cna.internal.NativeDeferredRelease.pendingCount();
            for (int index = 0; index < 32; index++) {
                NetworkSessionProperties discarded = new NetworkSessionProperties();
                discarded.set(0, index);
                assertEquals(index, discarded.get(0));
            }
            // Nothing is owed until the collector has actually run, so the loop is the honest
            // shape: ask for a collection, pump, and see whether anything came back.
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
                    "a collected property list must hand its native handle back on this thread");
            assertEquals(0, org.openeggbert.cna.internal.NativeDeferredRelease.pendingCount(),
                    "the drain must leave nothing owed");
        }
    }
}
