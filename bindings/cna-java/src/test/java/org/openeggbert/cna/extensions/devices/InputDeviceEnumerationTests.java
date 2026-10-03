package org.openeggbert.cna.extensions.devices;

import Microsoft.Xna.Framework.FrameworkDispatcher;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.ArrayList;
import java.util.List;
import java.util.function.BiConsumer;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Device enumeration and hot plug, against the live runtime.
 *
 * <p>The hot-plug path is proved rather than assumed: CNA exposes the same event the platform
 * layer raises on real hot plug, so the test raises it and asserts the identifier arrives at a
 * Java listener, on the game thread. Nothing is simulated on the Java side.
 */
final class InputDeviceEnumerationTests {

    @Test
    void enumerationRequiresAKind() {
        assertThrows(NullPointerException.class, () -> InputDevices.enumerate(null));
        assertThrows(NullPointerException.class, () -> InputDevices.addConnectedListener(null));
        assertThrows(NullPointerException.class, () -> InputDevices.removeConnectedListener(null));
        assertThrows(NullPointerException.class, () -> InputDevices.addDisconnectedListener(null));
        assertEquals(3, InputDeviceKind.values().length);
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void devicesEnumerateAndHotPlugReachesJava() {
        runProbe();
        // The native hot-plug subscription is process-wide and outlives a game, so a second
        // game in the same process must still see events. A subscription silently torn down
        // with the first game would leave the second one deaf, which is exactly what this
        // second run rules out.
        runProbe();
    }

    private static void runProbe() {
        try (Game game = new Game()) {
            DeviceProbe probe = new DeviceProbe(game);
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

    private static final class DeviceProbe extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private DeviceProbe(Game game) {
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

        /**
         * Asserts that a list ends with an expected tail.
         *
         * <p>The events a test raises are the last ones raised, so they are the last ones
         * delivered; whatever a host's own hardware contributed arrived before them and cannot
         * be predicted from here.
         */
        private static void assertEndsWith(List<String> expected, List<String> observed,
                String message) {
            assertTrue(observed.size() >= expected.size(),
                    message + " -- got " + observed);
            assertEquals(expected, observed.subList(observed.size() - expected.size(),
                    observed.size()), message + " -- got " + observed);
        }

        private void probe() {
            // Zero devices is an ordinary answer on a headless host, so what is asserted is
            // that the snapshot is well formed and self-consistent, not that hardware exists.
            for (InputDeviceKind kind : InputDeviceKind.values()) {
                List<InputDeviceInfo> devices = InputDevices.enumerate(kind);
                assertNotNull(devices);
                for (InputDeviceInfo device : devices) {
                    assertNotNull(device.Name(), "a name is empty, never null");
                }
                assertThrows(UnsupportedOperationException.class,
                        () -> devices.add(new InputDeviceInfo(1, "x")));
            }
            assertEquals(InputDevices.getMice(), InputDevices.enumerate(InputDeviceKind.Mouse));
            assertEquals(InputDevices.getKeyboards(),
                    InputDevices.enumerate(InputDeviceKind.Keyboard));
            assertEquals(InputDevices.getTouchDevices(),
                    InputDevices.enumerate(InputDeviceKind.TouchDevice));

            List<String> observed = new ArrayList<>();
            BiConsumer<InputDeviceKind, Long> connected =
                    (kind, id) -> observed.add("+" + kind + ":" + id);
            BiConsumer<InputDeviceKind, Long> disconnected =
                    (kind, id) -> observed.add("-" + kind + ":" + id);
            InputDevices.addConnectedListener(connected);
            InputDevices.addDisconnectedListener(disconnected);
            try {
                // CNA's own route raises the same event the platform layer raises. The record
                // is queued by the JNI callback and delivered by the pump, so nothing arrives
                // until Update runs -- which is what puts a listener on the game thread.
                InputDevices.RaiseConnected(InputDeviceKind.Mouse, 7);
                InputDevices.RaiseDisconnected(InputDeviceKind.Keyboard, 9);
                assertEquals(List.of(), observed, "no event may arrive before the pump runs");
                FrameworkDispatcher.Update();
                // The last two, rather than the only two. A platform with real input hardware
                // reports its own devices through the same subscription, and those arrive first
                // because they were queued when the subscription was taken -- so a test that
                // demanded an exact list was describing a host with no mouse rather than the
                // ordering CNA guarantees.
                assertEndsWith(List.of("+Mouse:7", "-Keyboard:9"), observed,
                        "both events must arrive, in the order CNA raised them");

                // A removed listener stops receiving, and the other one keeps working.
                observed.clear();
                InputDevices.removeConnectedListener(connected);
                InputDevices.RaiseConnected(InputDeviceKind.Mouse, 11);
                InputDevices.RaiseDisconnected(InputDeviceKind.Mouse, 12);
                FrameworkDispatcher.Update();
                assertEndsWith(List.of("-Mouse:12"), observed,
                        "the removed listener saw nothing and the other one saw its event");
                assertFalse(observed.contains("+Mouse:11"),
                        "a removed listener must receive nothing at all");
            } finally {
                InputDevices.removeConnectedListener(connected);
                InputDevices.removeDisconnectedListener(disconnected);
            }

            // Touch devices are enumerated but have no hot-plug event, and saying so is better
            // than raising a mouse event under a touch-device name.
            assertThrows(IllegalArgumentException.class,
                    () -> InputDevices.RaiseConnected(InputDeviceKind.TouchDevice, 1));
            assertThrows(IllegalArgumentException.class,
                    () -> InputDevices.RaiseDisconnected(InputDeviceKind.TouchDevice, 1));

            releasingTheSubscriptionStopsAndResumesDelivery();
        }

        /**
         * Proves the four native registrations can be released and taken again.
         *
         * <p>Releasing is the case a lifetime bug hides in: the queue, the handlers and the
         * registrations are three separate things, and a release that left a stale callback
         * pointer behind would show up here as an event arriving after the release, or as a
         * crash. Re-subscribing afterwards proves the release did not poison the state.
         */
        private void releasingTheSubscriptionStopsAndResumesDelivery() {
            List<String> observed = new ArrayList<>();
            BiConsumer<InputDeviceKind, Long> listener =
                    (kind, id) -> observed.add(kind + ":" + id);
            InputDevices.addConnectedListener(listener);
            try {
                org.openeggbert.cna.internal.GamerEventPump.releaseInputDevices();
                InputDevices.RaiseConnected(InputDeviceKind.Mouse, 21);
                FrameworkDispatcher.Update();
                assertEquals(List.of(), observed,
                        "a released subscription must deliver nothing");

                // Adding a listener subscribes again, so delivery resumes.
                InputDevices.addConnectedListener(listener);
                InputDevices.removeConnectedListener(listener);
                InputDevices.RaiseConnected(InputDeviceKind.Keyboard, 22);
                FrameworkDispatcher.Update();
                assertEndsWith(List.of("Keyboard:22"), observed,
                        "one listener remains registered, so exactly one record arrives");
                assertFalse(observed.contains("Mouse:21"),
                        "and the event raised while released is not replayed");
            } finally {
                InputDevices.removeConnectedListener(listener);
                InputDevices.removeConnectedListener(listener);
            }
        }
    }
}
