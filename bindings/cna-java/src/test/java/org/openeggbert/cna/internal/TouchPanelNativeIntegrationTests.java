package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.DisplayOrientation;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.WindowHandle;
import Microsoft.Xna.Framework.Input.Touch.GestureSample;
import Microsoft.Xna.Framework.Input.Touch.GestureType;
import Microsoft.Xna.Framework.Input.Touch.TouchCollection;
import Microsoft.Xna.Framework.Input.Touch.TouchLocation;
import Microsoft.Xna.Framework.Input.Touch.TouchLocationState;
import Microsoft.Xna.Framework.Input.Touch.TouchPanel;
import Microsoft.Xna.Framework.Input.Touch.TouchPanelCapabilities;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.time.Duration;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class TouchPanelNativeIntegrationTests {

    @Test
    void touchPanelUsesNativeSnapshotsQueueAndGestureRecognitionRoutes() {
        try (TouchGame game = new TouchGame()) {
            assertThrows(IllegalStateException.class, TouchPanel::GetState);
            game.RunOneFrame();
            assertTrue(game.completed);
            assertTrue(game.recognizerRouteReached);
        }
    }

    private static final class TouchGame extends Game {
        private boolean completed;
        private boolean recognizerRouteReached;

        @Override
        protected void Update(GameTime gameTime) {
            NativeBindings.resetTouchPanelForTests();
            try {
                assertEquals(0, TouchPanel.getDisplayWidth());
                assertEquals(0, TouchPanel.getDisplayHeight());
                assertEquals(DisplayOrientation.Default, TouchPanel.getDisplayOrientation());
                assertEquals(GestureType.None, TouchPanel.getEnabledGestures());
                assertFalse(TouchPanel.getIsGestureAvailable());
                assertEquals(WindowHandle.Zero, TouchPanel.getWindowHandle());

                TouchPanel.setDisplayWidth(640);
                TouchPanel.setDisplayHeight(480);
                TouchPanel.setDisplayOrientation(DisplayOrientation.Portrait);
                assertEquals(640, TouchPanel.getDisplayWidth());
                assertEquals(480, TouchPanel.getDisplayHeight());
                assertEquals(DisplayOrientation.Portrait, TouchPanel.getDisplayOrientation());
                assertThrows(CnaNativeException.class,
                        () -> TouchPanel.setDisplayOrientation(DisplayOrientation.FromValue(8)));
                assertEquals(DisplayOrientation.Portrait, TouchPanel.getDisplayOrientation());

                TouchPanel.setWindowHandle(getWindow().getHandle());
                assertEquals(getWindow().getHandle(), TouchPanel.getWindowHandle());

                TouchPanelCapabilities disconnected = TouchPanel.GetCapabilities();
                assertFalse(disconnected.getIsConnected());
                assertEquals(0, disconnected.getMaximumTouchCount());
                NativeBindings.setTouchDeviceExistsForTests(true);
                TouchPanelCapabilities connected = TouchPanel.GetCapabilities();
                assertTrue(connected.getIsConnected());
                assertEquals(4, connected.getMaximumTouchCount());

                TouchCollection empty = TouchPanel.GetState();
                assertTrue(empty.getIsConnected());
                assertEquals(0, empty.getCount());
                NativeBindings.setTouchFingerForTests(0, 5, 12.0f, 34.0f);
                assertThrows(CnaNativeException.class,
                        () -> NativeBindings.setTouchFingerForTests(-1, 5, 0.0f, 0.0f));

                NativeBindings.updateTouchPanelForTests();
                TouchCollection pressedState = TouchPanel.GetState();
                assertEquals(1, pressedState.getCount());
                TouchLocation pressed = pressedState.get(0);
                assertEquals(5, pressed.getId());
                assertEquals(TouchLocationState.Pressed, pressed.getState());
                assertEquals(new Vector2(12.0f, 34.0f), pressed.getPosition());
                assertFalse(pressed.TryGetPreviousLocation().getSucceeded());

                NativeBindings.setTouchFingerForTests(0, -1, 12.0f, 34.0f);
                NativeBindings.updateTouchPanelForTests();
                TouchLocation released = TouchPanel.GetState().get(0);
                assertEquals(TouchLocationState.Released, released.getState());
                assertTrue(released.TryGetPreviousLocation().getSucceeded());
                assertEquals(TouchLocationState.Pressed,
                        released.TryGetPreviousLocation().getPreviousLocation().getState());

                NativeBindings.resetTouchPanelForTests();
                assertThrows(CnaNativeException.class, TouchPanel::ReadGesture);
                GestureType selected = GestureType.Tap.Or(GestureType.Flick);
                TouchPanel.setEnabledGestures(selected);
                assertEquals(selected, TouchPanel.getEnabledGestures());
                assertTrue(selected.Contains(GestureType.Tap));
                assertThrows(CnaNativeException.class,
                        () -> TouchPanel.setEnabledGestures(GestureType.FromValue(1024)));
                assertEquals(selected, TouchPanel.getEnabledGestures());

                float[] sampleVectors = {
                        12.0f, 34.0f, 56.0f, 78.0f,
                        1.0f, 2.0f, 3.0f, 4.0f
                };
                NativeBindings.enqueueTouchGestureForTests(
                        GestureType.Flick.getValue(), 4242L, sampleVectors);
                assertTrue(TouchPanel.getIsGestureAvailable());
                GestureSample sample = TouchPanel.ReadGesture();
                assertEquals(GestureType.Flick, sample.getGestureType());
                assertEquals(Duration.ofNanos(424_200L), sample.getTimestamp());
                assertEquals(new Vector2(12.0f, 34.0f), sample.getPosition());
                assertEquals(new Vector2(56.0f, 78.0f), sample.getPosition2());
                assertEquals(new Vector2(1.0f, 2.0f), sample.getDelta());
                assertEquals(new Vector2(3.0f, 4.0f), sample.getDelta2());
                Vector2 mutableSnapshot = sample.getPosition();
                mutableSnapshot.X = 999.0f;
                assertEquals(new Vector2(12.0f, 34.0f), sample.getPosition());
                assertFalse(TouchPanel.getIsGestureAvailable());
                assertThrows(CnaNativeException.class, TouchPanel::ReadGesture);

                NativeBindings.resetTouchPanelForTests();
                TouchPanel.setDisplayWidth(640);
                TouchPanel.setDisplayHeight(480);
                TouchPanel.setEnabledGestures(GestureType.Tap);
                NativeBindings.raiseTouchEventForTests(
                        6, TouchLocationState.Pressed.ordinal(),
                        0.5f, 0.5f, 0.0f, 0.0f);
                NativeBindings.raiseTouchEventForTests(
                        6, TouchLocationState.Released.ordinal(),
                        0.5f, 0.5f, 0.0f, 0.0f);
                NativeBindings.updateTouchPanelForTests();
                assertEquals(0, TouchPanel.GetState().getCount(), "gesture event leaked into raw state");
                if (TouchPanel.getIsGestureAvailable()) {
                    GestureSample recognized = TouchPanel.ReadGesture();
                    assertTrue(GestureType.FromValue(1023).Contains(
                            recognized.getGestureType()));
                }
                recognizerRouteReached = true;
                completed = true;
            } finally {
                NativeBindings.resetTouchPanelForTests();
            }
        }
    }
}
