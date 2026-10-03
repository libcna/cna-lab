package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;

import java.util.function.Consumer;

import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Runs one body inside a frame, with a graphics device.
 *
 * <p>The {@code .cnb} layer needs no game, but a skinned model's parts do: a vertex buffer, an
 * index buffer and a texture are all device resources, and the C API only lends a device out
 * during a lifecycle callback.
 */
final class CnaSkinnedModelProbe {

    private CnaSkinnedModelProbe() {
    }

    static void run(Consumer<GraphicsDevice> body) {
        try (Probe probe = new Probe(body)) {
            probe.RunOneFrame();
            if (probe.failure instanceof RuntimeException runtime) {
                throw runtime;
            }
            if (probe.failure instanceof Error error) {
                throw error;
            }
            if (probe.failure != null) {
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    private static final class Probe extends Game {

        private final Consumer<GraphicsDevice> body;
        private boolean ran;
        private Throwable failure;

        private Probe(Consumer<GraphicsDevice> body) {
            this.body = body;
            new GraphicsDeviceManager(this);
        }

        @Override
        protected void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                body.accept(getGraphicsDevice());
            } catch (Throwable exception) {
                failure = exception;
            }
        }
    }
}
