package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.Texture3D;
import Microsoft.Xna.Framework.Graphics.TextureCube;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeShaderEffectRoutes;

import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * An effect built from shader source, and the uniforms a game sets on it.
 *
 * <p>A CNA extension, and the one XNA has no route to at all. An XNA {@code Effect} comes from a
 * compiled {@code .xnb} produced by a Content Pipeline that no longer runs on most machines, and
 * its parameters are reached through {@code Effect.Parameters["name"]}. This is the other way in:
 * hand CNA a vertex and a fragment shader as text, and set uniforms on the result by name and by
 * type.
 *
 * <p><strong>Creation succeeding is not the source compiling, and CNA is explicit about it.</strong>
 * A renderer decides for itself whether to compile at construction and whether to look at the
 * source at all, and this ABI does not normalize that. {@link #isValid()} is the question to ask
 * afterwards, and its two answers are not symmetric: {@code false} means a renderer looked and
 * refused, while {@code true} means only that nothing rejected it -- the CPU rasterizer accepts
 * any non-empty text and reports {@code true} for source that cannot draw. {@link #isSupported}
 * is the question to ask <em>before</em>.
 *
 * <p><strong>The effect is an ordinary graphics resource.</strong> {@link #getEffect()} hands back
 * an XNA {@link Effect} that {@code SpriteBatch.Begin} takes, and the game owns it: disposing the
 * effect is what releases it.
 *
 * <p><strong>Uniforms may be set in any order.</strong> A uniform set before the effect is
 * applied reaches the shader: SpriteBatch applies the effect it is given, as XNA's does, and
 * {@link #apply()} is only needed for a draw that does not. CNA's GL renderers once wrote a
 * uniform to whichever program happened to be current, so one set before the apply was lost
 * ({@code JAVA-UPSTREAM-016}); CNA fixed that, and
 * {@code tools/native-abi/probes/shader_effect_uniform_binding.c} measures all three orders.
 *
 * <p><strong>The dialect is the renderer's.</strong> CNA's own shaders are GLSL ES -- they open
 * with {@code #version 300 es} -- and that is what compiles on every renderer here that compiles
 * anything.
 */
public final class ShaderEffect implements AutoCloseable {

    /** CNA's own result for a buffer that could not hold the answer. */
    private static final int RESULT_BUFFER_TOO_SMALL = 14;

    private final Effect effect;
    private boolean closed;

    private ShaderEffect(Effect effect) {
        this.effect = effect;
    }

    /**
     * Reports whether a device's renderer takes effects built from source at all.
     *
     * <p>A different capability from the one that gates a compiled {@code .xnb} effect: a renderer
     * can support source-based effects and refuse compiled ones, and the CPU rasterizer does
     * exactly that. Asking the wrong one reports a game as blocked that is not.
     *
     * @param graphicsDevice the device to ask about
     * @return whether {@link #compile} can produce something usable
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static boolean isSupported(GraphicsDevice graphicsDevice) {
        return RendererCapabilities.supports(graphicsDevice, GraphicsCapability.CustomEffects);
    }

    /**
     * Builds an effect from a vertex and a fragment shader.
     *
     * @param graphicsDevice the device to compile on
     * @param vertexSource the vertex shader as text
     * @param fragmentSource the fragment shader as text
     * @return the effect, which the caller closes
     * @throws IllegalArgumentException when both sources are empty, which is the one refusal CNA
     *         makes identically on every renderer
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static ShaderEffect compile(GraphicsDevice graphicsDevice, String vertexSource,
            String fragmentSource) {
        GraphicsExtension.requireBackend();
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        Objects.requireNonNull(vertexSource, "vertexSource");
        Objects.requireNonNull(fragmentSource, "fragmentSource");
        long[] created = new long[1];
        GraphicsExtension.check("ShaderEffect.compile", NativeShaderEffectRoutes.shaderEffectCreate(
                NativeBindings.nativeGraphicsDeviceValue(graphicsDevice), utf8(vertexSource),
                utf8(fragmentSource), created));
        return new ShaderEffect(FacadeFactory.createOwnedEffect(graphicsDevice, created[0]));
    }

    /**
     * Returns the effect a draw takes.
     *
     * <p>An ordinary XNA {@link Effect}, which {@code SpriteBatch.Begin} accepts. It is this
     * object's, not a fresh view, and closing this closes it.
     *
     * @return the effect
     */
    public Effect getEffect() {
        open();
        return effect;
    }

    /**
     * Makes this effect's shader program the current one.
     *
     * <p>XNA's {@code effect.CurrentTechnique.Passes[0].Apply()}, for a draw that does not apply
     * the effect itself. SpriteBatch does, so a sprite draw needs no call here.
     */
    public void apply() {
        open();
        effect.getCurrentTechnique().getPasses().get(0).Apply();
    }

    /**
     * Reports what the renderer concluded about the source.
     *
     * <p><strong>The two answers are not symmetric.</strong> {@code false} is the strong one: a
     * renderer looked at the source and refused it. {@code true} means only that nothing rejected
     * it, which on a renderer that does not inspect source at all is true of text that cannot draw.
     *
     * @return the renderer's verdict
     */
    public boolean isValid() {
        boolean[] valid = new boolean[1];
        GraphicsExtension.check("ShaderEffect.isValid",
                NativeShaderEffectRoutes.shaderEffectIsValid(handle(), valid));
        return valid[0];
    }

    /**
     * Reports whether a renderer backs this effect at all.
     *
     * <p>Distinct from {@link #isValid()}: an effect with no renderer behind it has nothing that
     * could have judged its source, so the two questions come apart on exactly the renderers where
     * it matters.
     *
     * @return whether the effect has a renderer
     */
    public boolean hasRenderer() {
        boolean[] present = new boolean[1];
        GraphicsExtension.check("ShaderEffect.hasRenderer",
                NativeShaderEffectRoutes.shaderEffectHasRenderer(handle(), present));
        return present[0];
    }

    /**
     * Returns the compiler's diagnostics.
     *
     * @return the log, which is empty when the renderer had nothing to say
     */
    public String getCompileError() {
        long effectHandle = handle();
        long[] bytes = new long[1];
        // A zero-capacity probe reports the byte count and writes nothing, and CNA writes no
        // partial string, so BUFFER_TOO_SMALL is an expected answer rather than a failure.
        int probe = NativeShaderEffectRoutes
                .shaderEffectCopyCompileErrorExt(effectHandle, new byte[0], bytes);
        if (probe != RESULT_BUFFER_TOO_SMALL) {
            GraphicsExtension.check("ShaderEffect.getCompileError", probe);
        }
        int length = Math.toIntExact(bytes[0]);
        if (length == 0) {
            return "";
        }
        byte[] destination = new byte[length];
        GraphicsExtension.check("ShaderEffect.getCompileError", NativeShaderEffectRoutes
                .shaderEffectCopyCompileErrorExt(effectHandle, destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /**
     * Sets the world transform CNA's own shaders name.
     *
     * @param value the transform
     */
    public void setWorld(Matrix value) {
        GraphicsExtension.check("ShaderEffect.setWorld", NativeShaderEffectRoutes
                .shaderEffectSetWorld(handle(), EngineValues.floats(value, "value")));
    }

    /** @return the world transform */
    public Matrix getWorld() {
        return matrix("ShaderEffect.getWorld", NativeShaderEffectRoutes::shaderEffectGetWorld);
    }

    /**
     * Sets the view transform.
     *
     * @param value the transform
     */
    public void setView(Matrix value) {
        GraphicsExtension.check("ShaderEffect.setView", NativeShaderEffectRoutes
                .shaderEffectSetView(handle(), EngineValues.floats(value, "value")));
    }

    /** @return the view transform */
    public Matrix getView() {
        return matrix("ShaderEffect.getView", NativeShaderEffectRoutes::shaderEffectGetView);
    }

    /**
     * Sets the projection transform.
     *
     * @param value the transform
     */
    public void setProjection(Matrix value) {
        GraphicsExtension.check("ShaderEffect.setProjection", NativeShaderEffectRoutes
                .shaderEffectSetProjection(handle(), EngineValues.floats(value, "value")));
    }

    /** @return the projection transform */
    public Matrix getProjection() {
        return matrix("ShaderEffect.getProjection",
                NativeShaderEffectRoutes::shaderEffectGetProjection);
    }

    /**
     * Sets a named {@code float} uniform.
     *
     * <p>Call {@link #apply()} first, or the value goes nowhere -- see this class's own
     * documentation.
     *
     * <p>A name the program does not declare is accepted and does nothing, which is the renderer's
     * behaviour rather than a check this could add: a uniform the compiler removed because nothing
     * read it is indistinguishable from one that was never declared.
     *
     * @param name the uniform's name
     * @param value the value
     */
    public void setUniform(String name, float value) {
        GraphicsExtension.check("ShaderEffect.setUniform", NativeShaderEffectRoutes
                .shaderEffectSetUniformFloat(handle(), utf8(name), value));
    }

    /**
     * Sets a named {@code int} uniform.
     *
     * @param name the uniform's name
     * @param value the value
     */
    public void setUniform(String name, int value) {
        GraphicsExtension.check("ShaderEffect.setUniform", NativeShaderEffectRoutes
                .shaderEffectSetUniformInt32(handle(), utf8(name), value));
    }

    /**
     * Sets a named {@code vec2} uniform.
     *
     * @param name the uniform's name
     * @param value the value
     */
    public void setUniform(String name, Vector2 value) {
        Objects.requireNonNull(value, "value");
        GraphicsExtension.check("ShaderEffect.setUniform", NativeShaderEffectRoutes
                .shaderEffectSetUniformVector2(handle(), utf8(name),
                        new float[] {value.X, value.Y}));
    }

    /**
     * Sets a named {@code vec3} uniform.
     *
     * @param name the uniform's name
     * @param value the value
     */
    public void setUniform(String name, Vector3 value) {
        Objects.requireNonNull(value, "value");
        GraphicsExtension.check("ShaderEffect.setUniform", NativeShaderEffectRoutes
                .shaderEffectSetUniformVector3(handle(), utf8(name),
                        new float[] {value.X, value.Y, value.Z}));
    }

    /**
     * Sets a named {@code vec4} uniform.
     *
     * @param name the uniform's name
     * @param value the value
     */
    public void setUniform(String name, Vector4 value) {
        Objects.requireNonNull(value, "value");
        GraphicsExtension.check("ShaderEffect.setUniform", NativeShaderEffectRoutes
                .shaderEffectSetUniformVector4(handle(), utf8(name),
                        new float[] {value.X, value.Y, value.Z, value.W}));
    }

    /**
     * Sets a named {@code mat4} uniform.
     *
     * @param name the uniform's name
     * @param value the value
     */
    public void setUniform(String name, Matrix value) {
        Objects.requireNonNull(value, "value");
        GraphicsExtension.check("ShaderEffect.setUniform", NativeShaderEffectRoutes
                .shaderEffectSetUniformMatrix(handle(), utf8(name),
                        EngineValues.floats(value, "value")));
    }

    /**
     * Declares the std140 uniform block this effect's parameters live in.
     *
     * <p><strong>Required on a dialect with no loose uniforms, and harmless everywhere else.</strong>
     * Every SPIR-V target -- {@link ShaderDialect#GlslVulkan} and what compiles through it --
     * has no non-block uniforms at all, so a {@code setUniform} call on such a renderer has
     * nowhere to write until the block is declared. On GLSL and GLSL ES the declaration is
     * ignored, which is why it can sit unconditionally beside the effect's construction rather
     * than behind a test of the dialect. {@link RendererCapabilities#getShaderDialect} is how to
     * ask which one is running, when a game wants to know.
     *
     * @param blockSizeBytes the whole block's size in bytes, std140-padded; must not be negative
     * @param members each member's name and its byte offset from the start of the block; an
     *        empty map clears any previous declaration
     */
    public void declareUniformBlock(int blockSizeBytes, Map<String, Integer> members) {
        Objects.requireNonNull(members, "members");
        byte[][] names = new byte[members.size()][];
        int[] offsets = new int[members.size()];
        int index = 0;
        for (Map.Entry<String, Integer> member : members.entrySet()) {
            names[index] = utf8(Objects.requireNonNull(member.getKey(), "member name"));
            offsets[index] = Objects.requireNonNull(member.getValue(), "member offset");
            index++;
        }
        GraphicsExtension.check("ShaderEffect.declareUniformBlock", NativeShaderEffectRoutes
                .shaderEffectDeclareUniformBlockExt(handle(), blockSizeBytes, names, offsets));
    }

    /**
     * Sets a named array of {@code float} uniforms.
     *
     * @param name the uniform's name
     * @param values the values
     */
    public void setUniformArray(String name, float... values) {
        Objects.requireNonNull(values, "values");
        GraphicsExtension.check("ShaderEffect.setUniformArray", NativeShaderEffectRoutes
                .shaderEffectSetUniformFloatArray(handle(), utf8(name), values.clone()));
    }

    /**
     * Sets a named array of {@code vec2} uniforms.
     *
     * @param name the uniform's name
     * @param values the vectors
     */
    public void setUniformArray(String name, List<Vector2> values) {
        Objects.requireNonNull(values, "values");
        float[] packed = new float[Math.multiplyExact(values.size(), 2)];
        for (int index = 0; index < values.size(); index++) {
            Vector2 value = Objects.requireNonNull(values.get(index), "values");
            packed[index * 2] = value.X;
            packed[index * 2 + 1] = value.Y;
        }
        GraphicsExtension.check("ShaderEffect.setUniformArray", NativeShaderEffectRoutes
                .shaderEffectSetUniformVector2Array(handle(), utf8(name), packed));
    }

    /**
     * Sets a named array of {@code vec3} uniforms.
     *
     * <p>Three tightly packed floats per element, not four. A caller that padded to a
     * {@code vec4} would be describing a different array than the shader declares, which is the
     * mistake CNA's own header calls out -- and the packing happens here so a caller cannot make
     * it.
     *
     * @param name the uniform's name
     * @param values the vectors
     */
    public void setUniformVector3Array(String name, List<Vector3> values) {
        Objects.requireNonNull(values, "values");
        float[] packed = new float[Math.multiplyExact(values.size(), 3)];
        for (int index = 0; index < values.size(); index++) {
            Vector3 value = Objects.requireNonNull(values.get(index), "values");
            packed[index * 3] = value.X;
            packed[index * 3 + 1] = value.Y;
            packed[index * 3 + 2] = value.Z;
        }
        GraphicsExtension.check("ShaderEffect.setUniformVector3Array", NativeShaderEffectRoutes
                .shaderEffectSetUniformVec3Array(handle(), utf8(name), packed));
    }

    /**
     * Sets a named array of {@code mat4} uniforms.
     *
     * <p>Sixteen floats per matrix, in the order {@link Matrix} itself stores them -- which is what
     * makes this the route a skinning shader's bone transforms go through.
     *
     * @param name the uniform's name
     * @param values the matrices
     */
    public void setUniformArray(String name, Matrix... values) {
        Objects.requireNonNull(values, "values");
        GraphicsExtension.check("ShaderEffect.setUniformArray", NativeShaderEffectRoutes
                .shaderEffectSetUniformMat4Array(handle(), utf8(name),
                        EngineValues.matrices(java.util.Arrays.asList(values), "values")));
    }

    /**
     * Binds a two-dimensional texture to a numbered sampler unit.
     *
     * <p><strong>The effect retains it, and CNA enforces that.</strong> Disposing a texture while
     * a shader effect still names it is refused -- *"retained by an active SpriteBatch,
     * SpriteFont, effect, model or render-target scope"* -- so the effect has to be closed first.
     * That is stronger than the word "borrowed" suggests and is measured rather than read off the
     * declaration; a game using try-with-resources declares the texture before the effect so the
     * effect closes first.
     *
     * <p><strong>There is no unbind.</strong> CNA refuses an invalid handle here rather than
     * treating it as "none" -- measured, not read off the declaration -- so a game that wants a
     * unit to stop sampling something binds a different texture to it. Passing {@code null} is a
     * caller mistake and is refused before anything native happens.
     *
     * @param unit the texture unit
     * @param texture the texture
     */
    public void setTexture(int unit, Texture2D texture) {
        Objects.requireNonNull(texture, "texture");
        GraphicsExtension.check("ShaderEffect.setTexture", NativeShaderEffectRoutes
                .shaderEffectSetTexture2d(handle(), unit, handleOf(texture)));
    }

    /**
     * Binds a volume texture to a numbered sampler unit.
     *
     * @param unit the texture unit
     * @param texture the texture
     */
    public void setTexture(int unit, Texture3D texture) {
        Objects.requireNonNull(texture, "texture");
        GraphicsExtension.check("ShaderEffect.setTexture", NativeShaderEffectRoutes
                .shaderEffectSetTexture3d(handle(), unit, handleOf(texture)));
    }

    /**
     * Binds a cube map to a numbered sampler unit.
     *
     * @param unit the texture unit
     * @param texture the texture
     */
    public void setTexture(int unit, TextureCube texture) {
        Objects.requireNonNull(texture, "texture");
        GraphicsExtension.check("ShaderEffect.setTexture", NativeShaderEffectRoutes
                .shaderEffectSetTextureCube(handle(), unit, handleOf(texture)));
    }

    /** Disposes the effect. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        effect.Dispose();
    }

    /** One matrix CNA answers about this effect. */
    @FunctionalInterface
    private interface MatrixRoute {
        int call(long effect, float[] answer);
    }

    private Matrix matrix(String operation, MatrixRoute route) {
        float[] leaves = new float[EngineValues.MATRIX_LEAVES];
        GraphicsExtension.check(operation, route.call(handle(), leaves));
        return EngineValues.matrix(leaves, 0);
    }

    private static long handleOf(Object texture) {
        return NativeBindings.nativeResourceHandle(
                (Microsoft.Xna.Framework.Graphics.GraphicsResource) texture);
    }

    private static byte[] utf8(String value) {
        return Objects.requireNonNull(value, "name").getBytes(StandardCharsets.UTF_8);
    }

    private long handle() {
        open();
        return NativeBindings.nativeResourceHandle(effect);
    }

    private void open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This ShaderEffect is closed");
            }
        }
    }
}
