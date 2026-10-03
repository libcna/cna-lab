package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.FrameworkDispatcher;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Point;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.ArrayList;
import java.util.List;
import java.util.function.IntConsumer;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Raw joysticks, against the live runtime.
 *
 * <p>This host has no joystick. That is the branch the tests exercise: an identifier that is not
 * connected must capture successfully with everything empty, which is CNA's documented answer and
 * not a Java invention, and hot plug must still reach a Java listener because CNA raises the
 * event itself.
 */
final class JoystickExtensionTests {

    @Test
    void identitiesAreCnasOwnAndAHatIsNotABitSet() {
        assertEquals(10, JoystickType.values().length);
        assertEquals(0, JoystickType.Unknown.ordinal());
        assertEquals(9, JoystickType.Throttle.ordinal());
        assertEquals(9, JoystickHatPosition.values().length);
        // RightUp is its own identity, not Right combined with Up. Reading it as a bit set
        // would give 2 | 1 == 3, which is Down: a wrong direction, silently.
        assertEquals(5, JoystickHatPosition.RightUp.ordinal());
        assertEquals(3, JoystickHatPosition.Down.ordinal());
    }

    @Test
    void aSnapshotIsAValueThatCopiesItsLists() {
        List<Short> axes = new ArrayList<>(List.of((short) -32768, (short) 32767));
        JoystickState state = new JoystickState(axes, List.of(true),
                List.of(JoystickHatPosition.LeftDown), List.of(new Point(1, -1)));
        axes.clear();
        assertEquals(2, state.Axes().size(), "the record copied the list it was given");
        assertThrows(UnsupportedOperationException.class, () -> state.Axes().add((short) 0));
        assertFalse(state.isEmpty());
        assertTrue(new JoystickState(List.of(), List.of(), List.of(), List.of()).isEmpty());
        assertEquals(state, new JoystickState(List.of((short) -32768, (short) 32767),
                List.of(true), List.of(JoystickHatPosition.LeftDown), List.of(new Point(1, -1))));
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void joysticksAnswerTruthfullyOnAHostWithNone() {
        try (Game game = new Game()) {
            JoystickProbe probe = new JoystickProbe(game);
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

    private static final class JoystickProbe extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private JoystickProbe(Game game) {
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
            List<JoystickInfo> joysticks = Joysticks.enumerate();
            assertNotNull(joysticks);
            assertThrows(UnsupportedOperationException.class,
                    () -> joysticks.add(new JoystickInfo(1, JoystickType.Wheel, "x")));
            for (JoystickInfo joystick : joysticks) {
                assertNotNull(joystick.Type());
                assertNotNull(joystick.Name());
                JoystickCapabilities capabilities = Joysticks.getCapabilities(joystick.Id());
                JoystickState state = Joysticks.captureState(joystick.Id());
                assertEquals(capabilities.AxisCount(), state.Axes().size(),
                        "the snapshot must report exactly the axes the device has");
                assertEquals(capabilities.ButtonCount(), state.Buttons().size());
                assertEquals(capabilities.HatCount(), state.Hats().size());
                assertEquals(capabilities.BallCount(), state.Balls().size());
            }

            // An identifier nothing is connected to is not an error: CNA documents that the
            // capture succeeds with every array empty, and the projection reports exactly that
            // rather than inventing a failure or a zeroed device.
            int absent = 4242;
            JoystickCapabilities absentCapabilities = Joysticks.getCapabilities(absent);
            assertNotNull(absentCapabilities.Name());
            assertNotNull(absentCapabilities.Guid());
            assertNotNull(absentCapabilities.Power());
            assertFalse(absentCapabilities.IsConnected());
            JoystickState absentState = Joysticks.captureState(absent);
            assertTrue(absentState.isEmpty(),
                    "an unconnected identifier captures empty, not a device of zeros");

            List<String> observed = new ArrayList<>();
            IntConsumer connected = id -> observed.add("+" + id);
            IntConsumer disconnected = id -> observed.add("-" + id);
            Joysticks.addConnectedListener(connected);
            Joysticks.addDisconnectedListener(disconnected);
            try {
                Joysticks.RaiseConnected(3);
                Joysticks.RaiseDisconnected(3);
                assertEquals(List.of(), observed, "no event may arrive before the pump runs");
                FrameworkDispatcher.Update();
                assertEquals(List.of("+3", "-3"), observed);

                observed.clear();
                Joysticks.removeDisconnectedListener(disconnected);
                Joysticks.RaiseConnected(5);
                Joysticks.RaiseDisconnected(5);
                FrameworkDispatcher.Update();
                assertEquals(List.of("+5"), observed);
            } finally {
                Joysticks.removeConnectedListener(connected);
                Joysticks.removeDisconnectedListener(disconnected);
            }

            assertThrows(NullPointerException.class, () -> Joysticks.addConnectedListener(null));
            assertThrows(NullPointerException.class,
                    () -> Joysticks.addDisconnectedListener(null));
        }
    }
}
