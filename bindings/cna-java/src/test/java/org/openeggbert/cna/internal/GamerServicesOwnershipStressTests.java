package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GamerServices.AchievementCollection;
import Microsoft.Xna.Framework.GamerServices.FriendCollection;
import Microsoft.Xna.Framework.GamerServices.Gamer;
import Microsoft.Xna.Framework.GamerServices.GamerProfile;
import Microsoft.Xna.Framework.GamerServices.GamerServicesDispatcher;
import Microsoft.Xna.Framework.GamerServices.SignedInGamer;
import Microsoft.Xna.Framework.Net.NetworkSession;
import Microsoft.Xna.Framework.Net.NetworkSessionType;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Ownership and lifetime for the families added on top of the selected profile.
 *
 * <p>What these exercise is the part a structural check cannot see: closing twice, using a
 * closed object, tearing a session down while its subscriptions are live, and doing all of it
 * across several game lifetimes in one process. The dispatcher, the event queue and the
 * subscriptions are process-wide, so a leak between lifetimes would show here and nowhere else.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class GamerServicesOwnershipStressTests {

    private static final int LIFETIMES = 8;
    private static final int SESSIONS_PER_LIFETIME = 4;

    @Test
    void repeatedGameLifetimesDoNotLeakSessionsSubscriptionsOrEvents() {
        long droppedBefore = GamerEventPump.droppedEventCount();
        for (int lifetime = 0; lifetime < LIFETIMES; lifetime++) {
            try (Game game = new Game()) {
                StressProbe probe = new StressProbe(game);
                game.getComponents().add(probe);
                game.RunOneFrame();
                if (probe.failure != null) {
                    if (probe.failure instanceof RuntimeException runtime) {
                        throw runtime;
                    }
                    throw new IllegalStateException(probe.failure);
                }
                assertTrue(probe.ran, "lifetime " + lifetime + " did not run its probe");
                assertEquals(SESSIONS_PER_LIFETIME, probe.sessions,
                        "every session of lifetime " + lifetime + " must have been created");
            }
        }
        // The dispatcher survives a game, so a subscription leaked by one lifetime would keep
        // filling the queue during the next. Nothing may be dropped across all of them.
        assertEquals(droppedBefore, GamerEventPump.droppedEventCount(),
                "no event may be dropped across " + LIFETIMES + " game lifetimes");
    }

    /**
     * Drives the process-wide dispatcher as GamerServicesComponent does, initializing it only if
     * no earlier suite in this JVM has: XNA refuses a second initialization.
     */
    private static final class StressProbe extends Microsoft.Xna.Framework.GameComponent {

        private boolean ran;
        private int sessions;
        private Throwable failure;

        private StressProbe(Game game) {
            super(game);
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
                probe();
            } catch (Throwable exception) {
                failure = exception;
            }
        }

        private void probe() {
            assertTrue(GamerServicesDispatcher.getIsInitialized());

            for (int index = 0; index < SESSIONS_PER_LIFETIME; index++) {
                AtomicInteger started = new AtomicInteger();
                NetworkSession session = NetworkSession.Create(NetworkSessionType.Local, 1, 4);
                session.addGameStartedListener((sender, args) -> started.incrementAndGet());
                session.StartGame();
                session.Update();
                assertTrue(started.get() > 0, "the session's events must reach Java");

                // Closing twice is a no-op, and the disposed session reports itself disposed.
                session.Dispose();
                assertTrue(session.getIsDisposed());
                session.close();
                session.Dispose();

                // No callback may arrive for a session that has been torn down: the drain after
                // the next pump must not raise anything into the listener above.
                int afterDispose = started.get();
                GamerEventPump.drain();
                assertEquals(afterDispose, started.get(),
                        "a disposed session must not raise events");
                sessions++;
            }

            // The gamer graph is owned by CNA, and each projected object closes independently.
            for (SignedInGamer gamer : Gamer.getSignedInGamers()) {
                assertNotNull(gamer.getGamertag());
                try (AchievementCollection achievements = gamer.GetAchievements()) {
                    assertFalse(achievements.getIsDisposed());
                    achievements.Dispose();
                    assertTrue(achievements.getIsDisposed());
                    achievements.Dispose();
                }
                if (gamer.getIsSignedInToLive()) {
                    FriendCollection friends = gamer.GetFriends();
                    // Dispose then close then Dispose: all three are the same operation, and
                    // the second and third must be no-ops rather than a double free.
                    friends.Dispose();
                    friends.close();
                    friends.Dispose();
                    assertTrue(friends.getIsDisposed());
                } else {
                    // A local profile has no friends list, and CNA refuses rather than
                    // inventing an empty one.
                    org.junit.jupiter.api.Assertions.assertThrows(
                            Microsoft.Xna.Framework.GamerServices.GamerServicesNotAvailableException.class,
                            gamer::GetFriends);
                }
                try (GamerProfile profile = gamer.GetProfile()) {
                    assertNotNull(profile);
                    profile.Dispose();
                    assertTrue(profile.getIsDisposed());
                    profile.Dispose();
                }
            }
        }
    }
}
