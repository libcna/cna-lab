package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Graphics.BlendState;
import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.RenderTarget2D;
import Microsoft.Xna.Framework.Graphics.SamplerState;
import Microsoft.Xna.Framework.Graphics.SpriteBatch;
import Microsoft.Xna.Framework.Graphics.SpriteSortMode;
import Microsoft.Xna.Framework.Graphics.Texture2D;

import java.util.function.Consumer;

import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Runs one test body inside a real game frame.
 *
 * <p>Most of the extended graphics layer needs a real graphics device, and the C API only lends
 * one out during a lifecycle callback -- so a test for any of those families has to be a game.
 * This is that game, shared by every suite that needs one rather than copied into each.
 *
 * <p>A failure inside the body is captured rather than thrown through CNA's native frame, which
 * would cross a JNI boundary with an exception in flight, and is rethrown afterwards.
 */
final class GameProbe extends Game implements AutoCloseable {

    private final Consumer<GameProbe> body;
    private boolean ran;
    private Throwable failure;

    private GameProbe(Consumer<GameProbe> body) {
        this.body = body;
        new GraphicsDeviceManager(this);
    }

    /** Runs one body inside a single frame and rethrows whatever it threw. */
    static void run(Consumer<GameProbe> body) {
        try (GameProbe probe = new GameProbe(body)) {
            probe.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                if (probe.failure instanceof Error error) {
                    throw error;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    /** The game's own device, borrowed for the frame. */
    GraphicsDevice device() {
        return getGraphicsDevice();
    }

    /**
     * Stretches {@code source} over the whole of {@code target} through XNA's own SpriteBatch.
     *
     * <p>Opaque blending, so every pixel of the target is what the draw wrote. A null effect is
     * SpriteBatch's own; a custom one sees SpriteBatch's vertex layout -- position, texture
     * coordinate and colour at locations nought, one and two, and a {@code projection} uniform.
     */
    static void drawOver(GraphicsDevice device, Texture2D source, RenderTarget2D target,
            Effect effect, SamplerState sampler) {
        device.SetRenderTarget(target);
        device.Clear(Color.Transparent);
        try (SpriteBatch batch = new SpriteBatch(device)) {
            batch.Begin(SpriteSortMode.Immediate, BlendState.Opaque, sampler, null, null, effect);
            batch.Draw(source, new Rectangle(0, 0, target.getWidth(), target.getHeight()),
                    Color.White);
            batch.End();
        } finally {
            device.SetRenderTarget(null);
        }
    }

    @Override
    protected void Update(GameTime gameTime) {
        super.Update(gameTime);
        if (ran) {
            return;
        }
        ran = true;
        try {
            body.accept(this);
        } catch (Throwable exception) {
            failure = exception;
        }
    }
}
