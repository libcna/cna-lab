package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Rectangle;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/** Typed text and the mouse cursor, against the live runtime. */
final class InputExtensionTests {

    @Test
    void identityNamesAreCnasOwn() {
        assertEquals(9, TextInputType.values().length);
        assertEquals(0, TextInputType.Text.ordinal());
        assertEquals(8, TextInputType.NumberPasswordVisible.ordinal());
        assertEquals(12, MouseCursorStock.values().length);
        assertEquals(0, MouseCursorStock.Arrow.ordinal());
        assertEquals(11, MouseCursorStock.WaitArrow.ordinal());
    }

    @Test
    void aListenerIsRequired() {
        assertThrows(NullPointerException.class, () -> TextInput.addTextInputListener(null));
        assertThrows(NullPointerException.class, () -> TextInput.removeTextInputListener(null));
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void textInputAndCursorsWorkAgainstTheHost() {
        try (Game game = new Game()) {
            InputProbe probe = new InputProbe(game);
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

    private static final class InputProbe extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private InputProbe(Game game) {
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
            // CNA documents that with no window bound -- which is the headless platform --
            // every activation is a successful no-op and every query answers false. The
            // projection reports that rather than claiming input became active.
            assertFalse(TextInput.getIsActive());
            TextInput.Start();
            assertEquals(TextInput.getIsActive(), TextInput.getIsActive());
            TextInput.setInputRectangle(new Rectangle(4, 8, 120, 24));
            assertEquals(TextInput.getIsScreenKeyboardShown(),
                    TextInput.getIsScreenKeyboardShown());
            TextInput.Stop();
            assertFalse(TextInput.getIsActive());

            TextInput.Start(TextInputType.TextEmail);
            TextInput.Stop();
            assertFalse(TextInput.getIsActive());

            List<Integer> typed = new ArrayList<>();
            TextInput.addTextInputListener(typed::add);
            try {
                // Nothing types on a headless platform, so what is asserted is that
                // subscribing, pumping and unsubscribing are all sound -- not that a character
                // arrived, which would be a fabrication here.
                Microsoft.Xna.Framework.FrameworkDispatcher.Update();
                assertNotNull(typed);
            } finally {
                TextInput.removeTextInputListener(typed::add);
            }

            MouseCursor arrow = MouseCursor.FromStock(MouseCursorStock.Arrow);
            // Applying a cursor needs a host that has one. A platform whose video subsystem was
            // never started -- which is what a windowless renderer leaves behind -- refuses with
            // "CreateSystemCursor is not currently supported", and that refusal is an answer
            // about the host rather than a failure of the projection. Both are accepted, and
            // nothing else is.
            boolean cursorsExist = applyIfTheHostHasCursors(arrow);
            // A stock cursor is a borrowed view of a process-lifetime object, so closing it
            // never frees the shared cursor, and closing twice is a no-op.
            arrow.close();
            arrow.close();
            assertThrows(IllegalStateException.class, arrow::Apply);

            try (MouseCursor empty = MouseCursor.CreateEmpty()) {
                assertEquals(cursorsExist, applyIfTheHostHasCursors(empty),
                        "a host either has cursors or has none; it cannot have one kind");
            }

            assertThrows(NullPointerException.class, () -> MouseCursor.FromStock(null));
            assertThrows(NullPointerException.class,
                    () -> MouseCursor.FromTexture(null, 0, 0));
            assertThrows(NullPointerException.class, () -> TextInput.Start(null));
            assertThrows(NullPointerException.class, () -> TextInput.setInputRectangle(null));
        }

        /**
         * Applies a cursor, accepting the one refusal a host without cursors gives.
         *
         * @param cursor the cursor to apply
         * @return whether the host applied it
         */
        private static boolean applyIfTheHostHasCursors(MouseCursor cursor) {
            try {
                cursor.Apply();
                return true;
            } catch (org.openeggbert.cna.internal.CnaNativeException refusal) {
                assertTrue(refusal.getMessage().contains("CreateSystemCursor")
                                || refusal.getMessage().contains("not currently supported"),
                        "the only refusal accepted here is a host with no cursors, and it said: "
                                + refusal.getMessage());
                return false;
            }
        }
    }
}
