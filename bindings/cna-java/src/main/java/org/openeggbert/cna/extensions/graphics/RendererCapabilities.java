package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.EnumSet;
import java.util.Objects;
import java.util.Set;

/**
 * What the renderer behind a device can actually do, and which renderer it is.
 *
 * <p>A CNA extension. XNA's answer to this question is {@code GraphicsProfile}, a two-valued tier
 * chosen for the hardware of 2010; CNA runs on backends ranging from a CPU rasterizer to desktop
 * OpenGL 4.6, and a game that wants to know whether it may dispatch a compute shader has nowhere
 * in XNA to ask.
 *
 * <p><strong>Ask before you construct.</strong> A CNA object that needs a capability raises
 * {@link ExtensionNotSupportedException} rather than producing an object that refuses everything;
 * {@link #supports} is how a game chooses a path instead of catching one.
 *
 * <p><strong>The name is the renderer's, not the build's.</strong> CNA can be built with several
 * renderers compiled in and one chosen at run time, so {@link #getRendererName} answers about the
 * device in hand. That is the single most useful line in a bug report about rendering.
 */
public final class RendererCapabilities {

    /** CNA's own result for a buffer that could not hold the answer. */
    private static final int RESULT_BUFFER_TOO_SMALL = 14;

    private RendererCapabilities() {
    }

    /**
     * Reports whether the device's renderer has one capability.
     *
     * <p>A capability CNA recognises but the renderer lacks is a successful query that answers
     * {@code false}, not a failure: an absent capability is an answer.
     *
     * @param graphicsDevice the device to ask about
     * @param capability the capability to ask for
     * @return whether the renderer has it
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static boolean supports(GraphicsDevice graphicsDevice, GraphicsCapability capability) {
        GraphicsExtension.requireBackend();
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        Objects.requireNonNull(capability, "capability");
        boolean[] supported = new boolean[1];
        GraphicsExtension.check("RendererCapabilities.supports",
                NativeGraphicsExtensionRoutes.graphicsDeviceSupportsCapability(
                        NativeBindings.nativeGraphicsDeviceValue(graphicsDevice),
                        capability.toValue(), supported));
        return supported[0];
    }

    /**
     * Returns every capability the device's renderer has.
     *
     * <p>One call per capability, because that is the only question the C API answers; the set is
     * the convenience, not a cheaper route.
     *
     * @param graphicsDevice the device to ask about
     * @return the capabilities it has, which may be empty
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static Set<GraphicsCapability> supported(GraphicsDevice graphicsDevice) {
        Set<GraphicsCapability> answer = EnumSet.noneOf(GraphicsCapability.class);
        for (GraphicsCapability capability : GraphicsCapability.values()) {
            if (supports(graphicsDevice, capability)) {
                answer.add(capability);
            }
        }
        return answer;
    }

    /**
     * Returns the name of the renderer this device is really using.
     *
     * <p>CNA's own identity for it -- {@code OPENGLES3}, {@code HEADLESS}, {@code SOFTWARE} --
     * rather than the GPU's marketing name.
     *
     * @param graphicsDevice the device to ask about
     * @return the renderer's name
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static String getRendererName(GraphicsDevice graphicsDevice) {
        GraphicsExtension.requireBackend();
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        long device = NativeBindings.nativeGraphicsDeviceValue(graphicsDevice);
        long[] bytes = new long[1];
        GraphicsExtension.check("RendererCapabilities.getRendererName",
                NativeGraphicsExtensionRoutes.graphicsDeviceGetRendererNameSize(device, bytes));
        int length = Math.toIntExact(bytes[0]);
        if (length == 0) {
            return "";
        }
        byte[] destination = new byte[length];
        GraphicsExtension.check("RendererCapabilities.getRendererName",
                NativeGraphicsExtensionRoutes.graphicsDeviceCopyRendererName(
                        device, destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /**
     * Returns CNA's complete English capability report for the device.
     *
     * <p>Everything {@link #supported} answers and more, in CNA's own words, meant for a
     * diagnostics screen or a bug report rather than for a branch.
     *
     * @param graphicsDevice the device to ask about
     * @return the report
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static String getReport(GraphicsDevice graphicsDevice) {
        GraphicsExtension.requireBackend();
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        long device = NativeBindings.nativeGraphicsDeviceValue(graphicsDevice);
        long[] bytes = new long[1];
        // A zero-capacity probe reports the byte count and writes nothing, and CNA writes no
        // partial string, so BUFFER_TOO_SMALL is an expected answer rather than a failure.
        int probe = NativeGraphicsExtensionRoutes
                .graphicsDeviceCopyCapabilityReportExt(device, new byte[0], bytes);
        if (probe != RESULT_BUFFER_TOO_SMALL) {
            GraphicsExtension.check("RendererCapabilities.getReport", probe);
        }
        long[] sized = new long[1];
        GraphicsExtension.check("RendererCapabilities.getReport",
                NativeGraphicsExtensionRoutes.graphicsDeviceGetCapabilityReportSizeExt(
                        device, sized));
        int length = Math.toIntExact(sized[0]);
        if (length == 0) {
            return "";
        }
        byte[] destination = new byte[length];
        GraphicsExtension.check("RendererCapabilities.getReport",
                NativeGraphicsExtensionRoutes.graphicsDeviceCopyCapabilityReportExt(
                        device, destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /**
     * Returns the shading language a source-built effect must be written in for this device.
     *
     * <p>The question {@link ShaderEffect#compile} does not answer for itself. A renderer's name
     * is not a safe substitute: a build can carry several renderers in one library, and the one
     * that is running is chosen after the library loads.
     *
     * <p>EasyGL answers {@link ShaderDialect#GlslEs}; a renderer that accepts source without
     * executing it answers {@link ShaderDialect#Unknown}, which means "do not guess".
     * {@code JAVA-UPSTREAM-022} once found every renderer but WebGPU answering Unknown; CNA
     * fixed it, and {@code tools/native-abi/probes/shader_dialect_answer.c} re-measures it.
     *
     * @param graphicsDevice the device to ask
     * @return the dialect, or {@link ShaderDialect#Unknown} when the renderer declares none
     */
    public static ShaderDialect getShaderDialect(GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        int[] dialect = new int[1];
        GraphicsExtension.check("RendererCapabilities.getShaderDialect",
                NativeGraphicsExtensionRoutes.graphicsDeviceGetShaderDialectExt(
                        NativeBindings.nativeGraphicsDeviceValue(graphicsDevice), dialect));
        return ShaderDialect.of(dialect[0]);
    }
}
