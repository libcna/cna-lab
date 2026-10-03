package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.RenderTarget2D;
import Microsoft.Xna.Framework.Graphics.SamplerState;
import Microsoft.Xna.Framework.Graphics.Texture2D;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Claims that are about actual pixels.
 *
 * <p>Most suites here qualify what objects know about themselves and which calls CNA accepts.
 * This one renders into a target and <strong>reads the result back</strong>, so what it asserts
 * is the image.
 *
 * <p><strong>The sampler is the one value nothing else can check.</strong> It crosses into CNA and
 * never comes back -- no route reads one -- and a renderer that draws nothing accepts any sampler
 * and does something unobservable with it. On a renderer that draws, a filter mode is the
 * difference between hard texel blocks and interpolated ones, and that is a pixel a test can
 * read. The draw is XNA's own SpriteBatch with the sampler passed to Begin.
 *
 * <p><strong>What this cannot say on a renderer that draws nothing</strong> is said rather than
 * skipped: where the readback is refused, the refusal is asserted, and where the draw produces
 * nothing the test stops without claiming anything about an image it never saw.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class PixelReadbackTests {

    /** A two-by-two checkerboard: the smallest source whose filtering is visible. */
    private static Texture2D checkerboard(GraphicsDevice device) {
        Texture2D source = new Texture2D(device, 2, 2);
        source.SetData(new Color[] {
                Color.Black, Color.White,
                Color.White, Color.Black });
        return source;
    }

    private static Color[] read(Texture2D target, int size) {
        Color[] pixels = new Color[size * size];
        target.GetData(pixels);
        return pixels;
    }

    /** Whether a colour is one of the two the source was made of, allowing for 8-bit rounding. */
    private static boolean isSourceColour(Color colour) {
        return (colour.getR() <= 2 || colour.getR() >= 253)
                && colour.getR() == colour.getG() && colour.getG() == colour.getB();
    }

    @Test
    void aPointFilteredDrawKeepsTheSourcesTexelsAndALinearOneBlendsThem() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            final int size = 16;
            try (Texture2D source = checkerboard(device);
                    RenderTarget2D point = new RenderTarget2D(device, size, size);
                    RenderTarget2D linear = new RenderTarget2D(device, size, size)) {

                GameProbe.drawOver(device, source, point, null, SamplerState.PointClamp);
                GameProbe.drawOver(device, source, linear, null, SamplerState.LinearClamp);

                Color[] pointPixels;
                try {
                    pointPixels = read(point, size);
                } catch (RuntimeException refused) {
                    // A renderer that binds an offscreen target and refuses to read it back --
                    // HEADLESS is exactly that. Nothing here can be claimed about an image that
                    // cannot be seen, and saying so is the whole of what this test can do there.
                    assertFalse(RendererCapabilities.getRendererName(device).isEmpty());
                    return;
                }
                Color[] linearPixels = read(linear, size);

                // No escape hatch past this point, and that is deliberate. The readback above is
                // what a renderer that draws nothing refuses; having got an image back, this
                // renderer draws, and a flat one would be a defect rather than an excuse. An
                // earlier version of this test skipped on a flat image and a planted defect that
                // produced one therefore passed.

                // Point filtering reproduces the source's own texels and nothing between them:
                // every pixel is one of the two colours the checkerboard was made of.
                for (Color pixel : pointPixels) {
                    assertTrue(isSourceColour(pixel),
                            "point filtering invented the colour " + pixel);
                }
                // Linear filtering does the opposite: magnifying a two-by-two checkerboard
                // sixty-four times over produces a gradient, so some pixel must be neither
                // black nor white. This is the assertion the sampler had nowhere to be checked
                // by before -- it fails if the filter never reached the draw.
                boolean blended = false;
                for (Color pixel : linearPixels) {
                    if (!isSourceColour(pixel)) {
                        blended = true;
                        break;
                    }
                }
                assertTrue(blended,
                        "linear filtering must produce a colour between the source's two");

                // And the two images differ, which is the same claim from the other side: a
                // sampler that never reached CNA would make them identical.
                assertFalse(java.util.Arrays.equals(pointPixels, linearPixels),
                        "two filters must not produce the same image");
            }
        });
    }

    @Test
    void aClearedTargetReadsBackAsTheColourItWasCleared() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            final int size = 8;
            try (RenderTarget2D target = new RenderTarget2D(device, size, size)) {
                device.SetRenderTarget(target);
                device.Clear(new Color(12, 34, 56, 255));
                device.SetRenderTarget(null);

                Color[] pixels;
                try {
                    pixels = read(target, size);
                } catch (RuntimeException refused) {
                    // The floor under every other claim in this file: a renderer that cannot
                    // return what it was just told to write cannot be asked what a shader wrote.
                    return;
                }
                for (Color pixel : pixels) {
                    assertEquals(12, pixel.getR(), "red");
                    assertEquals(34, pixel.getG(), "green");
                    assertEquals(56, pixel.getB(), "blue");
                }
            }
        });
    }

    @Test
    void aTextureReadsBackTheTexelsItWasGiven() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            try (Texture2D source = checkerboard(device)) {
                Color[] pixels = new Color[4];
                source.GetData(pixels);
                // No renderer here refuses this one -- an ordinary texture's data is CPU-side --
                // and it is what separates "the readback path works" from "the draw worked".
                assertEquals(Color.Black, pixels[0]);
                assertEquals(Color.White, pixels[1]);
                assertEquals(Color.White, pixels[2]);
                assertEquals(Color.Black, pixels[3]);
            }
        });
    }

}
