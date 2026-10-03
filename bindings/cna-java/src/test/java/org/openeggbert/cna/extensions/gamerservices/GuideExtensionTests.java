package org.openeggbert.cna.extensions.gamerservices;

import Microsoft.Xna.Framework.FrameworkDispatcher;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.PlayerIndex;
import Microsoft.Xna.Framework.GamerServices.Guide;
import Microsoft.Xna.Framework.GamerServices.MessageBoxIcon;
import System.IAsyncResult;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The half of the Guide CNA hands back to the game.
 *
 * <p>XNA's Guide draws its own keyboard and message box because an Xbox has a system overlay.
 * This runtime has none, so the request becomes the game's to show -- and that whole question has
 * no XNA member, which is why it is measured here rather than through {@code Guide}.
 *
 * <p>What is asserted is the round trip a game actually performs: ask for input, find it pending,
 * read exactly the strings that were asked for, answer it, and see the XNA operation complete
 * with that answer.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class GuideExtensionTests {

    @Test
    void aPendingRequestReachesTheGameAndItsAnswerReachesXna() {
        try (Game game = new Game()) {
            GuideProbe probe = new GuideProbe(game);
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

    private static final class GuideProbe extends Microsoft.Xna.Framework.GameComponent {

        private boolean ran;
        private Throwable failure;

        private GuideProbe(Game game) {
            super(game);
        }

        @Override
        public void Update(GameTime gameTime) {
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
            assertNull(GuideExtensions.getPendingKeyboardInput(),
                    "nothing is pending before anything is asked for");
            assertNull(GuideExtensions.getPendingMessageBox());

            keyboardInputIsHandedToTheGame();
            messageBoxChoiceReachesXna();
            xnaSeesTheAnswerThroughEndAndTheCallback();

            assertThrows(NullPointerException.class,
                    () -> GuideExtensions.ShowAchievements(null));
            assertThrows(NullPointerException.class,
                    () -> GuideExtensions.Draw(null, null, null));
        }

        private void keyboardInputIsHandedToTheGame() {
            Guide.BeginShowKeyboardInput(PlayerIndex.One, "Name your ship",
                    "Twelve characters or fewer", "Aurora", null, null);

            PendingKeyboardInput pending = GuideExtensions.getPendingKeyboardInput();
            assertNotNull(pending, "the request must reach the game, or nobody can draw it");
            assertEquals("Name your ship", pending.Title(),
                    "the title the game asked for is the title it has to draw");
            assertEquals("Twelve characters or fewer", pending.Description());
            assertEquals("Aurora", pending.Text(),
                    "the default text is what has been entered so far");

            // Discarding drops the request without answering it, which is what a screen
            // teardown does; cancelling is an answer and is asserted next.
            GuideExtensions.DiscardKeyboardInput();
            assertNull(GuideExtensions.getPendingKeyboardInput());

            Guide.BeginShowKeyboardInput(PlayerIndex.One, "Again", "Once more", "", null, null);
            assertNotNull(GuideExtensions.getPendingKeyboardInput());
            GuideExtensions.CancelKeyboardInput();
            assertNull(GuideExtensions.getPendingKeyboardInput(),
                    "an answered request is no longer pending");
        }

        private void messageBoxChoiceReachesXna() {
            Guide.BeginShowMessageBox(PlayerIndex.One, "Quit?", "Progress will be lost",
                    List.of("Quit", "Keep playing"), 1, MessageBoxIcon.Warning, null, null);

            PendingMessageBox pending = GuideExtensions.getPendingMessageBox();
            assertNotNull(pending, "the box must reach the game, or nobody can draw it");
            assertEquals(1, pending.FocusButton(),
                    "the host's focused button is the one thing the game does not already know");

            // The player chose the first button. XNA's own EndShowMessageBox is what has to see
            // it, because that is the API a game already wrote against.
            GuideExtensions.ClickMessageBox(0);
            assertNull(GuideExtensions.getPendingMessageBox());
        }

        private void xnaSeesTheAnswerThroughEndAndTheCallback() {
            // CNA's Guide is genuinely asynchronous: Begin returns with the screen up, and the
            // XNA result is incomplete until the player answers.
            List<IAsyncResult> called = new java.util.ArrayList<>();
            IAsyncResult keyboard = Guide.BeginShowKeyboardInput(PlayerIndex.One, "Name",
                    "Your name", "Ada", called::add, "keyboard-state");
            assertFalse(keyboard.getIsCompleted(), "nobody has answered yet");
            assertFalse(keyboard.getCompletedSynchronously());
            assertEquals("keyboard-state", keyboard.getAsyncState());
            GuideExtensions.CancelKeyboardInput();
            assertTrue(keyboard.getIsCompleted());
            FrameworkDispatcher.Update();
            assertEquals(List.of(keyboard), called,
                    "the dispatcher pump runs the XNA callback once the answer exists");
            assertNull(Guide.EndShowKeyboardInput(keyboard),
                    "a cancelled input ends with null, as XNA's does");
            assertThrows(IllegalStateException.class, () -> Guide.EndShowKeyboardInput(keyboard));

            IAsyncResult box = Guide.BeginShowMessageBox(PlayerIndex.One, "Quit?", "Really",
                    List.of("Quit", "Stay"), 0, MessageBoxIcon.Warning, null, null);
            GuideExtensions.ClickMessageBox(1);
            assertEquals(Integer.valueOf(1), Guide.EndShowMessageBox(box),
                    "EndShowMessageBox returns the button the player chose");
        }
    }
}
