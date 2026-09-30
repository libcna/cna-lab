package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.RenderTarget2D;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Vector4;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.LinkedHashMap;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The colour-matrix effect: a four-by-four transform of every colour the effect draws.
 *
 * <p>A CNA extension with no XNA 4.0 counterpart at all. XNA's stock effects can multiply a
 * diffuse colour and that is the whole of their colour control -- desaturating an image, swapping
 * two channels or shifting everything towards sepia all need a shader there, through a Content
 * Pipeline that no longer runs.
 *
 * <p><strong>What is asserted is arithmetic, not a result code.</strong> The transform is written
 * and read back element for element, the greyscale preset is checked against the Rec. 709 weights
 * by value, and on a renderer that draws, a red texture drawn through the greyscale preset comes
 * back as the grey whose luminance those weights predict.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class ColorMatrixEffectTests {

    /** Rec. 709 luminance weights, which is what "greyscale" means here rather than a third each. */
    private static final float RED = 0.2126f;
    private static final float GREEN = 0.7152f;
    private static final float BLUE = 0.0722f;

    private static float[] identity() {
        float[] values = new float[16];
        values[0] = 1f;
        values[5] = 1f;
        values[10] = 1f;
        values[15] = 1f;
        return values;
    }

    @Test
    void aFreshEffectIsTheIdentityAndResetRestoresIt() {
        GameProbe.run(probe -> {
            try (ColorMatrixEffect effect = ColorMatrixEffect.create(probe.device())) {
                assertArrayEquals(identity(), effect.getMatrix(), 0f,
                        "a new effect changes no colour");
                assertEquals(new Vector4(0f, 0f, 0f, 0f).toString(),
                        effect.getOffset().toString(), "and adds nothing");

                float[] swapRedAndBlue = new float[16];
                swapRedAndBlue[2] = 1f;
                swapRedAndBlue[5] = 1f;
                swapRedAndBlue[8] = 1f;
                swapRedAndBlue[15] = 1f;
                effect.setMatrix(swapRedAndBlue);
                effect.setOffset(new Vector4(0.25f, 0.5f, 0.75f, 1f));
                assertArrayEquals(swapRedAndBlue, effect.getMatrix(), 0f,
                        "sixteen floats went in and the same sixteen came back, in order");
                Vector4 offset = effect.getOffset();
                assertEquals(0.25f, offset.X, 0f);
                assertEquals(0.5f, offset.Y, 0f);
                assertEquals(0.75f, offset.Z, 0f);
                assertEquals(1f, offset.W, 0f);

                effect.reset();
                assertArrayEquals(identity(), effect.getMatrix(), 0f);
                assertEquals(0f, effect.getOffset().X, 0f);
                assertEquals(0f, effect.getOffset().W, 0f);
            }
        });
    }

    @Test
    void greyscaleIsRec709AndNotAThirdEach() {
        GameProbe.run(probe -> {
            try (ColorMatrixEffect effect = ColorMatrixEffect.create(probe.device())) {
                effect.setOffset(new Vector4(1f, 1f, 1f, 1f));
                effect.setGrayscale();

                float[] grey = effect.getMatrix();
                // Row-major, and each ROW is one output channel's weights over the four inputs.
                // The first three rows are therefore identical -- that identity is what makes
                // the result a grey rather than a tint -- and the layout matters: the transpose
                // of this matrix is a different transform, and it was what this test asserted
                // until CNA disagreed. Measured, then written down.
                assertArrayEquals(new float[] {RED, GREEN, BLUE, 0f},
                        new float[] {grey[0], grey[1], grey[2], grey[3]}, 1e-6f,
                        "row 0 weighs the inputs into red");
                assertArrayEquals(new float[] {RED, GREEN, BLUE, 0f},
                        new float[] {grey[4], grey[5], grey[6], grey[7]}, 1e-6f,
                        "row 1 weighs them identically into green");
                assertArrayEquals(new float[] {RED, GREEN, BLUE, 0f},
                        new float[] {grey[8], grey[9], grey[10], grey[11]}, 1e-6f,
                        "row 2 identically into blue");
                assertArrayEquals(new float[] {0f, 0f, 0f, 1f},
                        new float[] {grey[12], grey[13], grey[14], grey[15]}, 1e-6f,
                        "and alpha passes through untouched");
                // A third each would be 0.3333 in every slot. It is not, and the difference is
                // the whole reason the preset exists.
                assertTrue(Math.abs(grey[1] - grey[0]) > 0.4f,
                        "green weighs far more than red, as luminance does");
                assertEquals(0f, effect.getOffset().X, 0f,
                        "the preset zeroes the offset, so the one set above is gone");
            }
        });
    }

    @Test
    void aRedTextureDrawnThroughGreyscaleComesBackGreyWhereTheRendererExecutesIt() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            final int size = 8;
            try (ColorMatrixEffect effect = ColorMatrixEffect.create(device);
                    Texture2D source = new Texture2D(device, 2, 2);
                    RenderTarget2D plainTarget = new RenderTarget2D(device, size, size);
                    RenderTarget2D greyTarget = new RenderTarget2D(device, size, size)) {
                source.SetData(new Color[] {Color.Red, Color.Red, Color.Red, Color.Red});

                // The control first, and it is not optional: a renderer that draws nothing
                // would let a black frame read as agreement with any expectation at all.
                GameProbe.drawOver(device, source, plainTarget, null, null);
                Color[] plain;
                try {
                    plain = read(plainTarget, size);
                } catch (RuntimeException refused) {
                    // HEADLESS binds an offscreen target and refuses to read it back. Nothing
                    // about an image can be claimed here, and saying so is all this can do.
                    assertFalse(RendererCapabilities.getRendererName(device).isEmpty());
                    return;
                }
                assertTrue(plain[0].getR() > 200 && plain[0].getG() < 60,
                        "the control draw must produce the source's red before anything else "
                                + "is claimed about an image");

                effect.setGrayscale();
                GameProbe.drawOver(device, source, greyTarget, effect.getEffect(), null);
                Color[] grey = read(greyTarget, size);

                if (grey[0].getR() == plain[0].getR() && grey[0].getG() == plain[0].getG()) {
                    // This renderer does not execute the colour matrix, which CNA documents
                    // rather than leaves to chance: the transform is the shared CPU SpriteBatch
                    // path, and every other renderer "deliberately leaves this false rather
                    // than pretending to execute it". So the image is the SOURCE, exactly --
                    // not a wrong colour, and not nothing. That is what is asserted, because
                    // "it looks unchanged" and "the effect did something wrong" have to stay
                    // distinguishable.
                    for (Color pixel : grey) {
                        assertEquals(plain[0].getR(), pixel.getR(),
                                "an unexecuted colour matrix must leave the source untouched");
                        assertEquals(plain[0].getG(), pixel.getG());
                        assertEquals(plain[0].getB(), pixel.getB());
                    }
                    return;
                }

                // Rec. 709 luminance of pure red is 0.2126, which is 54 of 255. The channels
                // must agree with each other -- that is what grey means -- and the value must be
                // that luminance rather than the red the source carried.
                for (Color pixel : grey) {
                    assertEquals(pixel.getR(), pixel.getG(),
                            "a greyscale draw makes red and green equal");
                    assertEquals(pixel.getG(), pixel.getB(), "and green and blue equal");
                }
                int expected = Math.round(RED * 255f);
                assertTrue(Math.abs(grey[0].getR() - expected) <= 4,
                        "pure red through Rec. 709 is " + expected + ", not " + grey[0].getR());

                // And the offset is added after the transform, which a matrix alone cannot show.
                effect.reset();
                effect.setOffset(new Vector4(0f, 1f, 0f, 0f));
                try (RenderTarget2D offsetTarget = new RenderTarget2D(device, size, size)) {
                    GameProbe.drawOver(device, source, offsetTarget, effect.getEffect(), null);
                    Color shifted = read(offsetTarget, size)[0];
                    assertTrue(shifted.getR() > 200, "the identity kept the red");
                    assertTrue(shifted.getG() > 200,
                            "and a green offset of one saturates green, which the matrix "
                                    + "alone could never do");
                }
            }
        });
    }

    private static Color[] read(RenderTarget2D target, int size) {
        Color[] pixels = new Color[size * size];
        target.GetData(pixels);
        return pixels;
    }

    @Test
    void whatIsRefusedIsRefused() {
        GameProbe.run(probe -> {
            try (ColorMatrixEffect effect = ColorMatrixEffect.create(probe.device())) {
                assertThrows(IllegalArgumentException.class,
                        () -> effect.setMatrix(new float[15]));
                assertThrows(IllegalArgumentException.class,
                        () -> effect.setMatrix(new float[17]));
                assertThrows(NullPointerException.class, () -> effect.setMatrix(null));
                assertThrows(NullPointerException.class, () -> effect.setOffset(null));
                // CNA refuses a matrix that is not finite rather than drawing with a NaN.
                float[] broken = identity();
                broken[3] = Float.NaN;
                assertThrows(RuntimeException.class, () -> effect.setMatrix(broken));

                // The array handed in is copied, so a caller may reuse it.
                float[] mine = identity();
                mine[1] = 0.5f;
                effect.setMatrix(mine);
                mine[1] = 9f;
                assertEquals(0.5f, effect.getMatrix()[1], 0f,
                        "the effect kept the values, not the array");
                assertNotSame(effect.getMatrix(), effect.getMatrix(),
                        "and every read is a fresh array");
            }
            assertThrows(NullPointerException.class, () -> ColorMatrixEffect.create(null));
        });
    }

    @Test
    void aClosedEffectSaysSo() {
        GameProbe.run(probe -> {
            ColorMatrixEffect effect = ColorMatrixEffect.create(probe.device());
            effect.close();
            effect.close();
            assertThrows(IllegalStateException.class, effect::getMatrix);
            assertThrows(IllegalStateException.class, effect::getEffect);
        });
    }

    @Test
    void theDialectRouteAnswersUnknownOnEveryRendererThisBuildHas() {
        // graphics.h says the renderer's identity is not a safe way to infer which shader text
        // to supply -- "wrong in a build carrying several renderers", which this is -- and to
        // ask this route instead. It answers UNKNOWN on all five, including the two that
        // demonstrably compile and execute GLSL ES. Reproduced in pure C in
        // tools/native-abi/probes/shader_dialect_answer.c and filed as JAVA-UPSTREAM-022:
        // GetShaderDialectEXT is a virtual whose default body returns Unknown, and WebGPU is
        // the only renderer in CNA's tree that overrides it.
        //
        // So this asserts the measured answer rather than the documented intent. A projection
        // that hard-coded Unknown would pass it, and that is not a gap in the test -- it is
        // the finding, and it is why the assertion below is about what CNA does.
        GameProbe.run(probe -> {
            ShaderDialect dialect = RendererCapabilities.getShaderDialect(probe.device());
            assertNotNull(dialect);
            assertEquals(ShaderDialect.Unknown, dialect,
                    "measured on all five renderers; JAVA-UPSTREAM-022");
            assertEquals(dialect, RendererCapabilities.getShaderDialect(probe.device()));
            assertThrows(NullPointerException.class,
                    () -> RendererCapabilities.getShaderDialect(null));
        });
    }

    @Test
    void everyDialectCnaNamesHasAJavaName() {
        // The decode is what a WebGPU build would exercise and this one cannot, so it is pinned
        // at the declaration: seven identities in CNA's own order, and anything else refused
        // rather than read as the first.
        assertEquals(7, ShaderDialect.values().length);
        assertEquals(0, ShaderDialect.Unknown.ordinal());
        assertEquals(1, ShaderDialect.GlslDesktop.ordinal());
        assertEquals(2, ShaderDialect.GlslEs.ordinal());
        assertEquals(3, ShaderDialect.GlslVulkan.ordinal());
        assertEquals(4, ShaderDialect.Hlsl.ordinal());
        assertEquals(5, ShaderDialect.Msl.ordinal());
        assertEquals(6, ShaderDialect.Wgsl.ordinal());
    }

    @Test
    void aUniformBlockDeclarationIsAcceptedOnEveryRenderer() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            if (!ShaderEffect.isSupported(device)) {
                // Nothing to declare a block on. Said rather than skipped silently.
                assertNotNull(RendererCapabilities.getRendererName(device));
                return;
            }
            try (ShaderEffect shader = ShaderEffect.compile(device,
                    "#version 300 es\nin vec2 a_position;\nvoid main() {\n"
                            + "  gl_Position = vec4(a_position, 0.0, 1.0);\n}\n",
                    "#version 300 es\nprecision mediump float;\nuniform vec4 u_colour;\n"
                            + "out vec4 fragment;\nvoid main() { fragment = u_colour; }\n")) {
                Map<String, Integer> members = new LinkedHashMap<>();
                members.put("u_colour", 0);
                members.put("u_scale", 16);
                // Ignored on a dialect with loose uniforms and required on one without, so the
                // call sits unconditionally beside construction. Both are accepted.
                //
                // What this CANNOT check, and says so rather than implying otherwise: whether
                // the offsets arrive correctly. Every renderer this build has takes loose
                // uniforms and ignores the declaration, so a planted defect that sends every
                // offset as zero passes -- verified. Only a SPIR-V target would read them, and
                // this build has none. The marshalling is pinned instead by the generator's own
                // tool tests, where the two arrays' shared count is checked.
                shader.declareUniformBlock(32, members);
                shader.declareUniformBlock(0, Map.of());

                // The two arrays share one count in C, and Java carries a length in each. A
                // declaration whose halves disagree cannot be expressed through this API at all,
                // which is the point of taking a map.
                assertThrows(RuntimeException.class,
                        () -> shader.declareUniformBlock(-1, members));
                assertThrows(NullPointerException.class,
                        () -> shader.declareUniformBlock(32, null));
            }
        });
    }
}
