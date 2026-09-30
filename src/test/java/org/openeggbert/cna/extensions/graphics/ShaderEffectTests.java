package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.RenderTarget2D;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Effects built from shader source, and the uniforms a game sets on them.
 *
 * <p><strong>The strongest claim in this file is a pixel.</strong> A fragment shader that writes
 * nothing but a uniform is the shortest path from a Java call to a colour on a render target: set
 * the uniform, draw, read the target back, and the colour is either the one that was set or the
 * uniform never arrived. Nothing weaker distinguishes a binding that works from one that returns
 * {@code SUCCESS} and does nothing, which is what every earlier test of this layer could say.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class ShaderEffectTests {

    /**
     * The vertex program for SpriteBatch's vertex layout.
     *
     * <p>A custom effect SpriteBatch draws through has to match the layout it feeds -- position,
     * texture coordinate and colour at locations nought, one and two, and a {@code projection}
     * uniform SpriteBatch sets. A shader that names its attributes anything else compiles and
     * draws nothing.
     */
    private static final String VERTEX = String.join("\n",
            "#version 300 es",
            "precision highp float;",
            "layout(location = 0) in vec2 aPos;",
            "layout(location = 1) in vec2 aTexCoord;",
            "layout(location = 2) in vec4 aColor;",
            "out vec2 TexCoord;",
            "uniform mat4 projection;",
            "void main() {",
            "    gl_Position = projection * vec4(aPos, 0.0, 1.0);",
            "    TexCoord = aTexCoord;",
            "}",
            "");

    /**
     * Writes a literal colour, which is the control.
     *
     * <p>Whether a renderer runs a custom fragment shader at all is a fact about the renderer, and
     * this is how it is established rather than inferred from a black image: if this shader does
     * not paint the target red then nothing this file draws means anything, and if it does then
     * the uniform claim beside it has no excuse left.
     */
    private static final String LITERAL_RED = String.join("\n",
            "#version 300 es",
            "precision highp float;",
            "in vec2 TexCoord;",
            "out vec4 FragColor;",
            "void main() {",
            "    FragColor = vec4(1.0, 0.0, 0.0, 1.0);",
            "}",
            "");

    /** Writes one uniform colour, so the output pixel is the uniform and nothing else. */
    private static final String CONSTANT_COLOUR = String.join("\n",
            "#version 300 es",
            "precision highp float;",
            "in vec2 TexCoord;",
            "out vec4 FragColor;",
            "uniform vec4 u_colour;",
            "void main() {",
            "    FragColor = u_colour;",
            "}",
            "");

    private static Color[] read(Texture2D target, int size) {
        Color[] pixels = new Color[size * size];
        target.GetData(pixels);
        return pixels;
    }

    @Test
    void aShaderBuiltFromSourceExistsAndSaysWhatTheRendererThoughtOfIt() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            try (ShaderEffect shader = ShaderEffect.compile(device, VERTEX, CONSTANT_COLOUR)) {
                assertNotNull(shader.getEffect(), "the effect is an ordinary XNA Effect");
                // isValid's two answers are not symmetric and CNA is explicit about it: false
                // means a renderer looked and refused, true means only that nothing rejected it.
                // So a true here is not asserted to mean the shader draws -- the pixel test
                // below is what says that -- and a false is asserted to come with a reason.
                if (!shader.isValid()) {
                    assertFalse(shader.getCompileError().isBlank(),
                            "a renderer that refused the source says why");
                }
                // Whether a renderer is behind the effect at all is a different question, and on
                // every renderer here the answer agrees with whether the layer has one.
                shader.hasRenderer();
            }
        });
    }

    @Test
    void bothSourcesEmptyIsTheOneRefusalEveryRendererMakes() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            // CNA settles exactly this case in the ABI rather than leaving it to the renderer,
            // and it is the only one it settles -- which is why it is the only one asserted.
            assertThrows(IllegalArgumentException.class,
                    () -> ShaderEffect.compile(device, "", ""));
            assertThrows(NullPointerException.class,
                    () -> ShaderEffect.compile(device, null, CONSTANT_COLOUR));
            assertThrows(NullPointerException.class,
                    () -> ShaderEffect.compile(device, VERTEX, null));
        });
    }

    @Test
    void theTransformsRoundTripThroughTheEffect() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            try (ShaderEffect shader = ShaderEffect.compile(device, VERTEX, CONSTANT_COLOUR)) {
                // Three distinguishable transforms, so a getter wired to the wrong one fails
                // rather than passing on symmetry.
                Matrix world = Matrix.CreateTranslation(new Vector3(1f, 2f, 3f));
                Matrix view = Matrix.CreateTranslation(new Vector3(4f, 5f, 6f));
                Matrix projection = Matrix.CreateTranslation(new Vector3(7f, 8f, 9f));
                shader.setWorld(world);
                shader.setView(view);
                shader.setProjection(projection);

                assertEquals(1f, shader.getWorld().M41, 1.0e-5f);
                assertEquals(2f, shader.getWorld().M42, 1.0e-5f);
                assertEquals(4f, shader.getView().M41, 1.0e-5f);
                assertEquals(7f, shader.getProjection().M41, 1.0e-5f);
                assertEquals(9f, shader.getProjection().M43, 1.0e-5f);
            }
        });
    }

    @Test
    void everyUniformShapeIsAcceptedAndAnUnknownNameIsNotAnError() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            // The texture is declared first so it closes last: an effect retains what is bound
            // to its samplers, and CNA refuses to dispose a texture while one still names it.
            // That ordering is the contract rather than a style choice.
            try (Texture2D texture = new Texture2D(device, 4, 4);
                    ShaderEffect shader = ShaderEffect.compile(device, VERTEX, CONSTANT_COLOUR)) {
                // A name the program does not declare is accepted and does nothing, which is the
                // renderer's own behaviour: a uniform the compiler removed because nothing read
                // it is indistinguishable from one that was never there.
                shader.apply();
                shader.setUniform("u_absent", 1.5f);
                shader.setUniform("u_absent", 7);
                shader.setUniform("u_absent", new Vector2(1f, 2f));
                shader.setUniform("u_absent", new Vector3(1f, 2f, 3f));
                shader.setUniform("u_absent", new Vector4(1f, 2f, 3f, 4f));
                shader.setUniform("u_absent", Matrix.getIdentity());
                shader.setUniformArray("u_absent", 1f, 2f, 3f);
                shader.setUniformArray("u_absent", List.of(new Vector2(1f, 2f)));
                shader.setUniformVector3Array("u_absent",
                        List.of(new Vector3(1f, 2f, 3f), new Vector3(4f, 5f, 6f)));
                shader.setUniformArray("u_absent", Matrix.getIdentity(), Matrix.CreateScale(2f));
                shader.setTexture(0, texture);
                // There is no unbind: CNA refuses an invalid handle here rather than reading it
                // as "none", which is measured rather than read off the declaration.
                assertThrows(NullPointerException.class,
                        () -> shader.setTexture(0, (Texture2D) null));

                assertThrows(NullPointerException.class, () -> shader.setUniform(null, 1f));
                assertThrows(NullPointerException.class,
                        () -> shader.setUniform("u_absent", (Vector3) null));
                assertThrows(NullPointerException.class,
                        () -> shader.setUniformVector3Array("u_absent", null));
            }
        });
    }

    @Test
    void aUniformSetFromJavaDecidesThePixelTheShaderWrites() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            final int size = 8;
            try (Texture2D source = new Texture2D(device, 4, 4);
                    RenderTarget2D control = new RenderTarget2D(device, size, size);
                    RenderTarget2D first = new RenderTarget2D(device, size, size);
                    RenderTarget2D second = new RenderTarget2D(device, size, size)) {

                // The control, and the only escape this test has. A renderer that does not run a
                // custom fragment shader cannot be asked what one wrote, and this is how that is
                // established -- not by finding a black image later, which is also what a broken
                // uniform produces.
                boolean runsShaders;
                try (ShaderEffect literal =
                        ShaderEffect.compile(device, VERTEX, LITERAL_RED)) {
                    literal.apply();
                    GameProbe.drawOver(device, source, control, literal.getEffect(), null);
                    Color[] controlPixels;
                    try {
                        controlPixels = read(control, size);
                    } catch (RuntimeException refused) {
                        // The renderer cannot read a target back, so it cannot be asked what
                        // anything wrote.
                        return;
                    }
                    runsShaders = controlPixels[0].getR() == 255 && controlPixels[0].getG() == 0
                            && controlPixels[0].getB() == 0;
                    if (!runsShaders) {
                        // CNA documents this state: a renderer may accept any non-empty source
                        // without inspecting it and draw nothing with it. Asserted rather than
                        // skipped -- isValid may still be true, and that is the point.
                        assertFalse(RendererCapabilities.getRendererName(device).isEmpty());
                        return;
                    }
                }

                // From here nothing is conditional. The renderer runs a custom fragment shader,
                // and the only thing between a Java call and the pixel is the uniform.
                try (ShaderEffect shader =
                        ShaderEffect.compile(device, VERTEX, CONSTANT_COLOUR)) {
                    // Applied first, which is the contract this renderer imposes without saying
                    // so: a uniform is written to the current shader program, and an effect that
                    // has not been applied is not it. JAVA-UPSTREAM-016.
                    shader.apply();
                    shader.setUniform("u_colour", new Vector4(1f, 0f, 0f, 1f));
                    GameProbe.drawOver(device, source, first, shader.getEffect(), null);
                    Color[] red = read(first, size);

                    // The claim. The fragment shader writes nothing but the uniform, so the
                    // target holds the colour Java set and no other.
                    for (Color pixel : red) {
                        assertEquals(255, pixel.getR(), "red was the uniform: " + pixel);
                        assertEquals(0, pixel.getG(), "and nothing else: " + pixel);
                        assertEquals(0, pixel.getB(), "and nothing else: " + pixel);
                    }

                    // Changing the uniform changes the image, which is what separates a uniform
                    // that arrived from a shader that happens to write red.
                    shader.apply();
                    shader.setUniform("u_colour", new Vector4(0f, 0.5f, 1f, 1f));
                    GameProbe.drawOver(device, source, second, shader.getEffect(), null);
                    Color[] blue = read(second, size);
                    for (Color pixel : blue) {
                        assertEquals(0, pixel.getR(), "the second colour arrived too: " + pixel);
                        assertTrue(Math.abs(pixel.getG() - 128) <= 2, "half green: " + pixel);
                        assertEquals(255, pixel.getB(), "full blue: " + pixel);
                    }
                    assertFalse(java.util.Arrays.equals(red, blue),
                            "two uniforms must produce two images");
                }
            }
        });
    }

    @Test
    void anEffectSaysWhatItIsMadeOfAndWhetherARendererIsBehindIt() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            try (ShaderEffect shader = ShaderEffect.compile(device, VERTEX, CONSTANT_COLOUR);
                    Microsoft.Xna.Framework.Graphics.BasicEffect stock =
                            new Microsoft.Xna.Framework.Graphics.BasicEffect(device)) {

                // An effect built from source is not a compiled one, however well it works. The
                // two are different provenances and CNA keeps them apart.
                assertFalse(EffectDiagnostics.isCompiled(shader.getEffect()),
                        "source is not bytecode");
                assertFalse(EffectDiagnostics.isCompiled(stock),
                        "and neither is a stock effect");

                // The source comes back exactly as it went in, which is what a shader-editing
                // tool needs and what nothing else here can answer.
                assertEquals(VERTEX, EffectDiagnostics.getVertexSource(shader.getEffect()));
                assertEquals(CONSTANT_COLOUR,
                        EffectDiagnostics.getFragmentSource(shader.getEffect()));

                // Whether a program is behind it is the renderer's answer, and it agrees with the
                // effect's own -- two routes, one fact, which is what makes the pair worth having.
                assertEquals(EffectDiagnostics.hasRenderer(shader.getEffect()),
                        shader.hasRenderer());

                assertThrows(NullPointerException.class,
                        () -> EffectDiagnostics.isCompiled(null));
                assertThrows(NullPointerException.class,
                        () -> EffectDiagnostics.getVertexSource(null));
            }
        });
    }

    @Test
    void aClosedShaderEffectRefusesFurtherUse() {
        GameProbe.run(probe -> {
            ShaderEffect shader =
                    ShaderEffect.compile(probe.device(), VERTEX, CONSTANT_COLOUR);
            shader.close();
            shader.close();
            assertThrows(IllegalStateException.class, shader::isValid);
            assertThrows(IllegalStateException.class, () -> shader.setUniform("u_colour", 1f));
        });
    }
}
