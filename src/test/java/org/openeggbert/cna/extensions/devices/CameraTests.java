package org.openeggbert.cna.extensions.devices;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.function.Consumer;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The camera, against the live runtime, through CNA's own test backend.
 *
 * <p>The family was unbound while destroying a test-backend camera left a dangling process-wide
 * override that the next camera route dereferenced (JAVA-UPSTREAM-019). CNA fixed that, and this
 * suite opens, drives and closes a test camera twice in one process to show the teardown holds.
 * A build without CNA's device layer refuses every route, which is asserted instead.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CameraTests {

    @Test
    void theStateNamesAreCnasOwn() {
        assertEquals(6, CameraState.values().length);
        assertEquals(4, CameraState.Ready.ordinal());
        assertEquals(3, CameraPosition.values().length);
    }

    @Test
    void aTestCameraDeliversTheFrameItWasGivenIntoATextureOfItsSize() {
        inFrame(game -> {
            if (!DeviceExtension.isAvailable()) {
                assertThrows(DeviceNotSupportedException.class, Camera::OpenWithTestBackend);
                return;
            }
            for (int round = 0; round < 2; round++) {
                try (Camera camera = Camera.OpenWithTestBackend()) {
                    Color[] frame = new Color[4 * 3];
                    for (int index = 0; index < frame.length; index++) {
                        frame[index] = new Color(index * 17, 255 - index * 17, index * 3, 255);
                    }
                    camera.SetTestState(CameraState.Ready);
                    camera.SetTestFrame(4, 3, frame);
                    assertEquals(CameraState.Ready, camera.getState());
                    assertEquals(4, camera.getFrameWidth());
                    assertEquals(3, camera.getFrameHeight());

                    try (Texture2D wrong = new Texture2D(game.getGraphicsDevice(), 2, 2)) {
                        assertFalse(camera.TryAcquireFrame(wrong),
                                "a texture of another size is refused as an ordinary false");
                    }
                    try (Texture2D texture = new Texture2D(game.getGraphicsDevice(), 4, 3)) {
                        assertTrue(camera.TryAcquireFrame(texture));
                        Color[] copied = new Color[frame.length];
                        texture.GetData(copied);
                        assertArrayEquals(frame, copied, "the texture holds the frame");
                    }
                    Color[] fivePixels = java.util.Arrays.copyOf(frame, 5);
                    assertThrows(org.openeggbert.cna.internal.CnaNativeException.class,
                            () -> camera.SetTestFrame(4, 3, fivePixels),
                            "CNA refuses a pixel count that is not width times height");
                }
            }
            assertNotNull(Camera.getCameras());
            assertEquals(CameraPosition.Unknown, Camera.defaultPosition());
            Camera closed = Camera.OpenWithTestBackend();
            closed.close();
            closed.close();
            assertThrows(IllegalStateException.class, closed::getState);
        });
    }

    private static void inFrame(Consumer<Game> body) {
        try (Game game = new Game()) {
            new GraphicsDeviceManager(game);
            Throwable[] failure = new Throwable[1];
            boolean[] ran = new boolean[1];
            game.getComponents().add(new Microsoft.Xna.Framework.GameComponent(game) {
                @Override
                public void Update(GameTime gameTime) {
                    super.Update(gameTime);
                    if (ran[0]) {
                        return;
                    }
                    ran[0] = true;
                    try {
                        body.accept(game);
                    } catch (Throwable exception) {
                        failure[0] = exception;
                    }
                }
            });
            game.RunOneFrame();
            if (failure[0] instanceof RuntimeException runtime) {
                throw runtime;
            }
            if (failure[0] instanceof Error error) {
                throw error;
            }
            assertTrue(ran[0], "the probe must have run");
        }
    }
}
