package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.PlayerIndex;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.time.Duration;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Exercises the GamerServices projection against a live CNA runtime.
 *
 * <p>The qualified runtime is HEADLESS with no Live service, so what these assert is the
 * behaviour a machine with no signed-in gamer actually has: an empty roster, an invisible
 * Guide, and every value route answering rather than failing. Nothing here fabricates a gamer.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class GamerServicesNativeIntegrationTests {

    @Test
    void dispatcherStateAndGuideValuesRoundTripThroughCna() {
        try (Game game = new Game()) {
            ProbeComponent probe = new ProbeComponent(game);
            game.getComponents().add(probe);
            game.RunOneFrame();
            // A failure inside the update callback is contained at the native boundary, which
            // is correct for production but would hide the assertion that failed. The probe
            // therefore captures it and the test rethrows it with its own stack intact.
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                if (probe.failure instanceof Error error) {
                    throw error;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe component must have been updated");
        }
    }

    /**
     * Drives the dispatcher the way GamerServicesComponent does, without being one.
     *
     * <p>The dispatcher is process-wide and XNA refuses a second initialization, while this test
     * JVM runs many suites; so the probe initializes only when nothing has yet. The component's
     * own first-initialization-and-refusal contract is asserted in a child JVM by
     * GamerServicesComponentTests.
     */
    private static final class ProbeComponent extends Microsoft.Xna.Framework.GameComponent {

        private boolean ran;
        private Throwable failure;

        private ProbeComponent(Game game) {
            super(game);
        }

        @Override
        public void Initialize() {
            if (!GamerServicesDispatcher.getIsInitialized()) {
                GamerServicesDispatcher.setWindowHandle(getGame().getWindow().getHandle());
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

            assertTrue(GamerServicesDispatcher.getIsInitialized());
            assertNotNull(GamerServicesDispatcher.getWindowHandle());

            // CNA's gamer services publish a local roster on this runtime rather than an
            // empty one. Whatever that roster is, the projection must agree with it: the
            // indexed view, the player-slot view and each gamer's own state all come from the
            // same native list, and Java fabricates none of it.
            SignedInGamerCollection gamers = Gamer.getSignedInGamers();
            assertEquals(gamers.size() == 0, gamers.isEmpty());
            for (int index = 0; index < gamers.size(); index++) {
                SignedInGamer gamer = gamers.get(index);
                assertNotNull(gamer);
                assertNotNull(gamer.getGamertag());
                assertNotNull(gamer.getDisplayName());
                assertNotNull(gamer.toString());
                assertFalse(gamer.getIsDisposed(), "signed-in gamer IsDisposed");
                assertNotNull(gamer.getPlayerIndex());
                assertEquals(gamer.getGamertag(), gamers.get(gamer.getPlayerIndex()).getGamertag());

                GamerPrivileges privileges = gamer.getPrivileges();
                assertNotNull(privileges.getAllowCommunication());
                assertNotNull(privileges.getAllowProfileViewing());

                GameDefaults defaults = gamer.getGameDefaults();
                assertNotNull(defaults.getGameDifficulty());
                assertNotNull(defaults.getControllerSensitivity());
                assertNotNull(defaults.getPrimaryColor());
                // Color is a struct in XNA, so each read is a copy a caller cannot alias.
                assertNotSame(defaults.getPrimaryColor(), defaults.getPrimaryColor());

                GamerPresence presence = gamer.getPresence();
                assertNotNull(presence.getPresenceMode());

                try (AchievementCollection achievements = gamer.GetAchievements()) {
                    assertEquals(achievements.size(), achievements.getCount());
                    assertFalse(achievements.getIsDisposed(), "achievements IsDisposed");
                    assertThrows(UnsupportedOperationException.class, achievements::clear);
                }
                if (!gamer.getIsSignedInToLive()) {
                    // A local profile has no friends list: CNA refuses rather than inventing an
                    // empty one (ABI 0.32), and the projection surfaces that refusal.
                    assertThrows(GamerServicesNotAvailableException.class, gamer::GetFriends);
                    continue;
                }
                try (FriendCollection friends = gamer.GetFriends()) {
                    assertFalse(friends.getIsDisposed(), "friends IsDisposed");
                    assertEquals(friends.size(), friends.GetEnumerator() == null ? -1
                            : friends.size());
                }
            }
            // A slot beyond the roster is empty rather than a fabricated gamer.
            for (PlayerIndex slot : PlayerIndex.values()) {
                SignedInGamer gamer = gamers.get(slot);
                if (gamer == null) {
                    continue;
                }
                assertTrue(slot.ordinal() < 4);
            }

            assertFalse(Guide.getIsVisible(), "Guide IsVisible");

            // A Guide setting is a request to the platform, not a Java field: CNA accepts the
            // call and the platform decides. The screen saver is one the headless platform
            // does not honour, so what the projection must guarantee is that the property
            // keeps answering consistently and that restoring it restores it -- not that a
            // request always takes effect.
            boolean screenSaver = Guide.getIsScreenSaverEnabled();
            Guide.setIsScreenSaverEnabled(!screenSaver);
            assertEquals(Guide.getIsScreenSaverEnabled(), Guide.getIsScreenSaverEnabled());
            Guide.setIsScreenSaverEnabled(screenSaver);
            assertEquals(screenSaver, Guide.getIsScreenSaverEnabled());

            // The notification position and the trial-mode simulation are CNA's own state, so
            // these do round-trip exactly.
            NotificationPosition position = Guide.getNotificationPosition();
            Guide.setNotificationPosition(NotificationPosition.BottomCenter);
            assertEquals(NotificationPosition.BottomCenter, Guide.getNotificationPosition());
            Guide.setNotificationPosition(position);
            assertEquals(position, Guide.getNotificationPosition());

            boolean simulate = Guide.getSimulateTrialMode();
            Guide.setSimulateTrialMode(!simulate);
            assertEquals(!simulate, Guide.getSimulateTrialMode());
            Guide.setSimulateTrialMode(simulate);
            assertEquals(simulate, Guide.getSimulateTrialMode());

            Guide.DelayNotifications(Duration.ofSeconds(1));

            // With no account service configured, CNA refuses the social screens rather than
            // showing an empty one (ABI 0.32): the XNA answer is GamerServicesNotAvailable.
            assertThrows(GamerServicesNotAvailableException.class,
                    () -> Guide.ShowFriends(PlayerIndex.One));
            assertEquals(Guide.getIsVisible(), Guide.getIsVisible());

            leaderboardColumnsRoundTrip();
        }

        private void leaderboardColumnsRoundTrip() {
            LeaderboardIdentity identity =
                    LeaderboardIdentity.Create(LeaderboardKey.BestScoreLifeTime, 3);
            LeaderboardWriter writer = new LeaderboardWriter();
            LeaderboardEntry entry = writer.GetLeaderboard(identity);
            // XNA returns the same entry for the same leaderboard, so a title can accumulate
            // into it across a match.
            assertEquals(entry, writer.GetLeaderboard(identity));

            entry.setRating(4_242L);
            assertEquals(4_242L, entry.getRating());

            PropertyDictionary columns = entry.getColumns();
            assertEquals(0, columns.getCount());
            columns.SetValue("score", 17);
            columns.SetValue("time", Duration.ofSeconds(90));
            columns.SetValue("mode", "ranked");
            columns.SetValue("outcome", LeaderboardOutcome.Win);
            columns.SetValue("ratio", 0.5f);

            assertEquals(5, columns.getCount());
            assertEquals(17, columns.GetValueInt32("score"));
            assertEquals(Duration.ofSeconds(90), columns.GetValueTimeSpan("time"));
            assertEquals("ranked", columns.GetValueString("mode"));
            assertEquals(LeaderboardOutcome.Win, columns.GetValueOutcome("outcome"));
            assertEquals(0.5f, columns.GetValueSingle("ratio"));

            // The XNA-named members and the java.util.Map bridge reach one dictionary.
            assertTrue(columns.ContainsKey("score"));
            assertTrue(columns.containsKey("score"));
            assertEquals(17, columns.get("score"));
            assertEquals(17, columns.TryGetValue("score"));
            assertEquals(null, columns.TryGetValue("absent"));
            // CNA reports the columns by key. CLR's Dictionary does not specify an order, so
            // the projection reports CNA's rather than imposing one of its own.
            assertEquals(List.of("mode", "outcome", "ratio", "score", "time"),
                    List.copyOf(columns.keySet()));
            assertEquals(5, columns.entrySet().size());

            columns.put("score", 21);
            assertEquals(21, columns.GetValueInt32("score"));
            assertEquals(21, columns.remove("score"));
            assertFalse(columns.ContainsKey("score"));

            assertThrows(IllegalArgumentException.class, () -> columns.put("bad", this));
            columns.clear();
            assertTrue(columns.isEmpty(), "columns empty after clear");

            ownedHandlesGoBackToCna();
        }

        /**
         * Proves the handles these value types own are given back.
         *
         * <p>Neither type is disposable in XNA, so a game has nothing to close and a leak here
         * would be silent and permanent. Every leaderboard entry CNA hands out is an owned copy,
         * so reading a page twice is two handles, not one; and every route that produces an
         * avatar description hands back an owned handle too.
         */
        private void ownedHandlesGoBackToCna() {
            int before = org.openeggbert.cna.internal.NativeDeferredRelease.pendingCount();
            LeaderboardWriter writer = new LeaderboardWriter();
            for (int index = 0; index < 16; index++) {
                LeaderboardIdentity identity = new LeaderboardIdentity();
                identity.setKey("leak-probe-" + index);
                assertNotNull(writer.GetLeaderboard(identity));
                assertNotNull(AvatarDescription.CreateRandom());
            }
            writer = null;

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
                    "a collected entry or description must hand its handle back on this thread");
            assertEquals(0, org.openeggbert.cna.internal.NativeDeferredRelease.pendingCount(),
                    "the drain must leave nothing owed");
        }
    }
}
