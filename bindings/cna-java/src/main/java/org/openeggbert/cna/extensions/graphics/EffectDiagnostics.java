package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.Effect;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeShaderEffectRoutes;

import java.nio.charset.StandardCharsets;
import java.util.Objects;

/**
 * What an effect is made of, asked of any effect rather than only of one this binding built.
 *
 * <p>A CNA extension. XNA's {@code Effect} tells a game nothing about itself beyond its parameters
 * and techniques: whether it came from compiled bytecode or from source, whether a renderer is
 * behind it at all, and what its source says are all invisible. CNA answers all three, and on a
 * renderer that compiles shaders the answers are worth having -- a shader-editing tool needs the
 * source it is editing, and a game whose effect draws nothing needs to know whether there is a
 * program behind it.
 *
 * <p>Static, because these are questions <em>about</em> an XNA type rather than members of it: an
 * {@code Effect} lives in {@code Microsoft.Xna.Framework.Graphics} and nothing CNA-specific may
 * appear on it.
 */
public final class EffectDiagnostics {

    /** CNA's own result for a buffer that could not hold the answer. */
    private static final int RESULT_BUFFER_TOO_SMALL = 14;

    private EffectDiagnostics() {
    }

    /**
     * Reports whether an effect came from compiled bytecode.
     *
     * <p>{@code true} means its parameters, techniques and passes were reflected out of that
     * bytecode rather than built by hand -- and that cloning it clones the compiled runtime with
     * it. A stock effect or one built from source answers {@code false}.
     *
     * @param effect the effect
     * @return whether a compiled runtime is present
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static boolean isCompiled(Effect effect) {
        GraphicsExtension.requireBackend();
        boolean[] compiled = new boolean[1];
        GraphicsExtension.check("EffectDiagnostics.isCompiled",
                NativeShaderEffectRoutes.effectGetIsCompiledExt(handle(effect), compiled));
        return compiled[0];
    }

    /**
     * Reports whether a renderer-specific program is behind the effect.
     *
     * <p>A different question from {@link #isCompiled}, and the one that separates an effect that
     * can draw from one that cannot: a renderer with no shader compiler leaves an effect with no
     * program however the effect was made.
     *
     * @param effect the effect
     * @return whether a live compiled program exists
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static boolean hasRenderer(Effect effect) {
        GraphicsExtension.requireBackend();
        boolean[] present = new boolean[1];
        GraphicsExtension.check("EffectDiagnostics.hasRenderer",
                NativeShaderEffectRoutes.effectHasRenderer(handle(effect), present));
        return present[0];
    }

    /**
     * Returns the vertex shader the effect was built from.
     *
     * @param effect the effect
     * @return the source, which is empty for an effect that was not built from any
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static String getVertexSource(Effect effect) {
        return source("EffectDiagnostics.getVertexSource", handle(effect),
                NativeShaderEffectRoutes::effectGetVertexSourceByteCount,
                NativeShaderEffectRoutes::effectCopyVertexSource);
    }

    /**
     * Returns the fragment shader the effect was built from.
     *
     * @param effect the effect
     * @return the source, which is empty for an effect that was not built from any
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static String getFragmentSource(Effect effect) {
        return source("EffectDiagnostics.getFragmentSource", handle(effect),
                NativeShaderEffectRoutes::effectGetFragmentSourceByteCount,
                NativeShaderEffectRoutes::effectCopyFragmentSource);
    }

    /** How many bytes a source is. */
    @FunctionalInterface
    private interface SizeRoute {
        int call(long effect, long[] answer);
    }

    /** The copy-out half of the same pair. */
    @FunctionalInterface
    private interface CopyRoute {
        int call(long effect, byte[] destination, long[] written);
    }

    private static String source(String operation, long effect, SizeRoute size, CopyRoute copy) {
        GraphicsExtension.requireBackend();
        long[] bytes = new long[1];
        GraphicsExtension.check(operation, size.call(effect, bytes));
        int length = Math.toIntExact(bytes[0]);
        if (length == 0) {
            return "";
        }
        byte[] destination = new byte[length];
        int result = copy.call(effect, destination, bytes);
        if (result == RESULT_BUFFER_TOO_SMALL) {
            // The source grew between the two calls, which nothing here can do about but which
            // must not be reported as a truncated shader: CNA writes no partial string, so the
            // buffer is untouched and asking again is the only correct answer.
            length = Math.toIntExact(bytes[0]);
            destination = new byte[length];
            result = copy.call(effect, destination, bytes);
        }
        GraphicsExtension.check(operation, result);
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    private static long handle(Effect effect) {
        Objects.requireNonNull(effect, "effect");
        return NativeBindings.nativeResourceHandle(effect);
    }
}
