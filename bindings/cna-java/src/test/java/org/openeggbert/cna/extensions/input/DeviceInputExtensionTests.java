package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.FrameworkDispatcher;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.PlayerIndex;
import Microsoft.Xna.Framework.Point;
import Microsoft.Xna.Framework.Input.Buttons;
import Microsoft.Xna.Framework.Input.Keys;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.function.IntConsumer;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The game-pad, keyboard and mouse capabilities XNA has no shape for.
 *
 * <p>None of these belongs on {@code Microsoft.Xna.Framework.Input}: XNA's pad is an Xbox 360
 * controller with no light bar, touchpad, battery or button labels, its keyboard cannot name a
 * key in the player's layout, and its mouse cannot be captured or read across the desktop.
 */
final class DeviceInputExtensionTests {

    @Test
    void identitiesAreCnasOwn() {
        assertEquals(3, GamePadConnectionState.values().length);
        assertEquals(2, GamePadConnectionState.Wireless.ordinal());
        assertEquals(9, GamePadButtonLabel.values().length);
        assertEquals(5, GamePadButtonLabel.Cross.ordinal());
        assertEquals(0x08, KeyModifier.Gui.getMask());
        assertEquals(Set.of(KeyModifier.Shift, KeyModifier.Caps), KeyModifier.decode(0x11));
        assertEquals(Set.of(), KeyModifier.decode(0));
        // A modifier bit CNA adds later must not become a wrong constant.
        assertEquals(Set.of(), KeyModifier.decode(0x8000));
        assertThrows(UnsupportedOperationException.class,
                () -> KeyModifier.decode(1).add(KeyModifier.Num));
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void theHostAnswersForEveryOneOfThem() {
        try (Game game = new Game()) {
            Probe probe = new Probe(game);
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

    private static final class Probe extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private Probe(Game game) {
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
            gamePad();
            keyboard();
            mouse();
            touch();
        }

        private void gamePad() {
            PlayerIndex one = PlayerIndex.One;
            // No pad is connected here, so what is asserted is that every reading is
            // well formed and that an absent measurement is absent rather than zero.
            assertNotNull(GamePadExtensions.getName(one));
            assertNotNull(GamePadExtensions.getGuid(one));
            assertNotNull(GamePadExtensions.getPath(one));
            assertNotNull(GamePadExtensions.getSerial(one));
            assertNotNull(GamePadExtensions.getConnectionState(one));
            assertNotNull(GamePadExtensions.getPowerState(one));
            assertNotNull(GamePadExtensions.getButtonLabel(one, Buttons.A));
            assertEquals(GamePadExtensions.getFirmwareVersion(one),
                    GamePadExtensions.getFirmwareVersion(one));
            assertEquals(GamePadExtensions.getSteamHandle(one),
                    GamePadExtensions.getSteamHandle(one));
            assertEquals(GamePadExtensions.getPlayerNumber(one),
                    GamePadExtensions.getPlayerNumber(one));
            assertEquals(GamePadExtensions.setPlayerNumber(one, 1),
                    GamePadExtensions.setPlayerNumber(one, 1));
            assertEquals(GamePadExtensions.SetTriggerVibration(one, 0.5f, 0.5f),
                    GamePadExtensions.SetTriggerVibration(one, 0.0f, 0.0f));
            GamePadExtensions.setLightBar(one, Color.Red);

            List<GamePadTouchpadFinger> fingers = GamePadExtensions.getTouchpadFingers(one, 0);
            assertNotNull(fingers);
            assertThrows(UnsupportedOperationException.class,
                    () -> fingers.add(new GamePadTouchpadFinger(true, 0, 0, 0)));
            assertEquals(GamePadExtensions.getTouchpadCount(one),
                    GamePadExtensions.getTouchpadCount(one));

            assertThrows(NullPointerException.class, () -> GamePadExtensions.getName(null));
            assertThrows(NullPointerException.class,
                    () -> GamePadExtensions.setLightBar(one, null));
            assertThrows(NullPointerException.class,
                    () -> GamePadExtensions.getButtonLabel(one, null));
        }

        private void keyboard() {
            // The scancode round trip is the real content: a name has to come back as the key
            // it names. Whatever the host calls the key, feeding its own name back must give
            // the same key, or a controls screen that saved a binding cannot restore it.
            String name = KeyboardExtensions.getScancodeName(Keys.A);
            assertNotNull(name);
            if (!name.isEmpty()) {
                assertEquals(Keys.A, KeyboardExtensions.getScancodeFromName(name),
                        "a scancode name must resolve back to its own scancode");
            }
            String keyName = KeyboardExtensions.getKeyName(Keys.A);
            assertNotNull(keyName);
            if (!keyName.isEmpty()) {
                assertEquals(Keys.A, KeyboardExtensions.getKeyFromName(keyName),
                        "a key name must resolve back to its own key");
            }
            assertNotNull(KeyboardExtensions.getKeyFromScancode(Keys.A));
            assertEquals(Keys.None, KeyboardExtensions.getKeyFromName("no such key"),
                    "a name the host does not know is None, not a wrong key");
            assertNotNull(KeyboardExtensions.getModifiers());

            assertThrows(NullPointerException.class,
                    () -> KeyboardExtensions.getKeyName(null));
            assertThrows(NullPointerException.class,
                    () -> KeyboardExtensions.getKeyFromName(null));
            assertThrows(NullPointerException.class,
                    () -> KeyboardExtensions.getKeyFromScancode(null));
        }

        private void touch() {
            // Real touch hardware and emulated touch are different claims, and CNA separates
            // them where XNA's IsConnected cannot. Turning emulation on must be visible, or
            // a desktop developer cannot exercise a touch path at all.
            boolean before = TouchPanelExtensions.getMouseTouchEmulationEnabled();
            TouchPanelExtensions.setMouseTouchEmulationEnabled(!before);
            assertEquals(!before, TouchPanelExtensions.getMouseTouchEmulationEnabled(),
                    "the emulation flag is CNA's own state and must round-trip");
            TouchPanelExtensions.setMouseTouchEmulationEnabled(before);
            assertEquals(before, TouchPanelExtensions.getMouseTouchEmulationEnabled());
            assertEquals(TouchPanelExtensions.getTouchDeviceExists(),
                    TouchPanelExtensions.getTouchDeviceExists());
        }

        private void mouse() {
            boolean before = MouseExtensions.getIsRelativeMouseMode();
            MouseExtensions.setIsRelativeMouseMode(!before);
            // Whether the host honours the request is the host's business; what the projection
            // guarantees is that it reports back what the host actually did, not what was asked.
            boolean after = MouseExtensions.getIsRelativeMouseMode();
            MouseExtensions.setIsRelativeMouseMode(before);
            assertEquals(before, MouseExtensions.getIsRelativeMouseMode(),
                    "the mode has to return to what it was, whatever the host allowed: " + after);

            assertEquals(MouseExtensions.setCapture(true), MouseExtensions.setCapture(false));
            assertNotNull(MouseExtensions.getGlobalPosition());
            assertEquals(MouseExtensions.WarpGlobal(new Point(1, 1)),
                    MouseExtensions.WarpGlobal(new Point(2, 2)));
            assertThrows(NullPointerException.class, () -> MouseExtensions.WarpGlobal(null));

            List<Integer> clicks = new ArrayList<>();
            IntConsumer listener = clicks::add;
            MouseExtensions.addClickedListener(listener);
            try {
                MouseExtensions.RaiseClicked(2);
                assertEquals(List.of(), clicks, "no event may arrive before the pump runs");
                FrameworkDispatcher.Update();
                assertEquals(List.of(2), clicks);
            } finally {
                MouseExtensions.removeClickedListener(listener);
            }
            MouseExtensions.RaiseClicked(3);
            FrameworkDispatcher.Update();
            assertEquals(List.of(2), clicks, "a removed listener receives nothing more");
            assertThrows(NullPointerException.class,
                    () -> MouseExtensions.addClickedListener(null));
        }
    }
}
