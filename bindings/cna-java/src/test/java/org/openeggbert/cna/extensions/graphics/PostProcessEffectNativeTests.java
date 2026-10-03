package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * CNA's post-process effects, against the live runtime.
 *
 * <p>What these assert is that the effect is a real XNA {@code Effect} the game owns, and that
 * each knob round-trips through CNA. Nothing here claims a frame was rendered: the qualified
 * runtime is headless.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class PostProcessEffectNativeTests {

    @Test
    void theCrtAndDepthEffectsAreRealEffectsWhoseKnobsRoundTrip() {
        try (EffectGame game = new EffectGame()) {
            game.RunOneFrame();
            if (game.failure != null) {
                if (game.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(game.failure);
            }
            assertTrue(game.ran, "the probe must have run");
        }
    }

    private static final class EffectGame extends Game {

        private boolean ran;
        private Throwable failure;

        private EffectGame() {
            new Microsoft.Xna.Framework.GraphicsDeviceManager(this);
        }

        @Override
        protected void Update(GameTime gameTime) {
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
            if (!GraphicsExtension.isAvailable()) {
                // A build without CNA_CNAEXT refuses the effects rather than faking one.
                org.junit.jupiter.api.Assertions.assertThrows(
                        ExtensionNotSupportedException.class,
                        () -> new CrtEffect(getGraphicsDevice()));
                return;
            }

            try (CrtEffect crt = new CrtEffect(getGraphicsDevice())) {
                // It is a real XNA Effect: the game owns it and the ordinary effect surface
                // works on it, which is what lets SpriteBatch.Begin take it.
                assertNotNull(crt.getEffect());
                assertSameDevice(crt.getEffect());
                assertFalse(crt.getEffect().getIsDisposed());

                crt.setCurvature(0.25f);
                assertEquals(0.25f, crt.getCurvature());
                crt.setScanlineIntensity(0.5f);
                assertEquals(0.5f, crt.getScanlineIntensity());
                crt.setVignetteIntensity(0.75f);
                assertEquals(0.75f, crt.getVignetteIntensity());
                crt.setMaskIntensity(0.125f);
                assertEquals(0.125f, crt.getMaskIntensity());
                crt.setMaskType(CrtMaskType.ApertureGrille);
                assertEquals(CrtMaskType.ApertureGrille, crt.getMaskType());
                assertThrows(NullPointerException.class, () -> crt.setMaskType(null));
            }

            try (DepthEffect depth = new DepthEffect(getGraphicsDevice())) {
                assertNotNull(depth.getEffect());
                assertNotSame(depth.getEffect(), null);
                depth.setMode(DepthEffectMode.Grayscale4Bit);
                assertEquals(DepthEffectMode.Grayscale4Bit, depth.getMode());
                depth.setDitherMode(DitherMode.Bayer8x8);
                assertEquals(DitherMode.Bayer8x8, depth.getDitherMode());
                assertThrows(NullPointerException.class, () -> depth.setDitherMode(null));
            }

            // Closing an effect disposes it; closing twice is a no-op, as for every CNA-owned
            // resource in this binding.
            CrtEffect closed = new CrtEffect(getGraphicsDevice());
            closed.close();
            assertTrue(closed.getEffect().getIsDisposed());
            closed.close();

            assertThrows(NullPointerException.class, () -> new CrtEffect(null));
            assertThrows(NullPointerException.class, () -> new DepthEffect(null));
        }

        private void assertSameDevice(Microsoft.Xna.Framework.Graphics.Effect effect) {
            assertEquals(getGraphicsDevice(), effect.getGraphicsDevice());
        }
    }
}
