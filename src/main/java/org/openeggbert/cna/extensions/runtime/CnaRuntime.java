package org.openeggbert.cna.extensions.runtime;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeRuntimeExtensionRoutes;

import java.util.Objects;

/**
 * What the loaded CNA build is: its platform, its renderer, and how mature that renderer is.
 *
 * <p>A CNA extension. XNA never told a game which backend it got, and a game that wants to
 * adapt -- to skip an effect on a software rasterizer, or to warn about an experimental
 * backend -- has nothing in XNA to ask. None of this needs a graphics device or a running game.
 */
public final class CnaRuntime {

    private CnaRuntime() {
    }

    /** Returns the platform family CNA was compiled for. */
    public static CnaPlatform getPlatform() {
        NativeBindings.requireAvailable();
        int[] platform = new int[1];
        check("CnaRuntime.getPlatform",
                NativeRuntimeExtensionRoutes.platformGetCurrent(platform));
        return CnaPlatform.values()[platform[0]];
    }

    /** Returns CNA's own name for the platform. */
    public static String getPlatformName() {
        return text("CnaRuntime.getPlatformName",
                NativeRuntimeExtensionRoutes::platformGetCurrentNameSizeExt,
                NativeRuntimeExtensionRoutes::platformCopyCurrentNameExt);
    }

    /**
     * Returns the desktop operating system.
     *
     * @throws IllegalStateException when the platform is not a desktop, because CNA refuses the
     *     question there rather than returning a fallback
     */
    public static DesktopOperatingSystem getDesktopOperatingSystem() {
        NativeBindings.requireAvailable();
        int[] os = new int[1];
        check("CnaRuntime.getDesktopOperatingSystem",
                NativeRuntimeExtensionRoutes.desktopOsGetCurrent(os));
        return DesktopOperatingSystem.values()[os[0]];
    }

    public static boolean isMobile() {
        NativeBindings.requireAvailable();
        boolean[] mobile = new boolean[1];
        check("CnaRuntime.isMobile",
                NativeRuntimeExtensionRoutes.platformGetIsMobileExt(mobile));
        return mobile[0];
    }

    public static boolean isApple() {
        NativeBindings.requireAvailable();
        boolean[] apple = new boolean[1];
        check("CnaRuntime.isApple",
                NativeRuntimeExtensionRoutes.platformGetIsAppleExt(apple));
        return apple[0];
    }

    /** Returns CNA's name for the renderer this build was compiled with. */
    public static String getRendererName() {
        return text("CnaRuntime.getRendererName",
                NativeRuntimeExtensionRoutes::graphicsRendererGetCurrentNameSize,
                NativeRuntimeExtensionRoutes::graphicsRendererCopyCurrentName);
    }

    /**
     * Returns how the renderer this build was configured with produces pixels.
     *
     * <p><strong>Not necessarily the renderer that is running.</strong> On a build with several
     * renderers compiled in, CNA's route behind this answers about the compile-time default rather
     * than the one chosen at startup, which is how CNA documents it. For the running renderer use
     * {@code GraphicsRenderer.getCategory(GraphicsRenderer.getActive())}.
     */
    public static GraphicsBackendCategory getBackendCategory() {
        NativeBindings.requireAvailable();
        int[] category = new int[1];
        check("CnaRuntime.getBackendCategory",
                NativeRuntimeExtensionRoutes.graphicsBackendGetCurrentCategory(category));
        return GraphicsBackendCategory.values()[category[0]];
    }

    /**
     * Returns how far the renderer this build was configured with has been taken.
     *
     * <p>Carries the same caveat as {@link #getBackendCategory()}: on a multi-renderer build this
     * describes the compile-time default, not the running renderer.
     */
    public static GraphicsBackendMaturity getBackendMaturity() {
        NativeBindings.requireAvailable();
        int[] maturity = new int[1];
        check("CnaRuntime.getBackendMaturity",
                NativeRuntimeExtensionRoutes.graphicsBackendGetCurrentMaturity(maturity));
        return GraphicsBackendMaturity.values()[maturity[0]];
    }

    /** Returns CNA's own name for one backend category. */
    public static String getName(GraphicsBackendCategory category) {
        NativeBindings.requireAvailable();
        int value = Objects.requireNonNull(category, "category").ordinal();
        return text("CnaRuntime.getName",
                out -> NativeRuntimeExtensionRoutes.graphicsBackendCategoryGetNameSize(value, out),
                (buffer, out) -> NativeRuntimeExtensionRoutes.graphicsBackendCategoryCopyName(
                        value, buffer, out));
    }

    /** Returns CNA's own name for one backend maturity. */
    public static String getName(GraphicsBackendMaturity maturity) {
        NativeBindings.requireAvailable();
        int value = Objects.requireNonNull(maturity, "maturity").ordinal();
        return text("CnaRuntime.getName",
                out -> NativeRuntimeExtensionRoutes.graphicsBackendMaturityGetNameSize(value, out),
                (buffer, out) -> NativeRuntimeExtensionRoutes.graphicsBackendMaturityCopyName(
                        value, buffer, out));
    }

    /**
     * Returns the title CNA reports for this application.
     *
     * <p>CNA has no size query for it, so this asks with a buffer and grows once if the answer
     * did not fit, rather than assuming a length.
     */
    public static String getTitle() {
        NativeBindings.requireAvailable();
        byte[] destination = new byte[256];
        long[] written = new long[1];
        check("CnaRuntime.getTitle",
                NativeRuntimeExtensionRoutes.assemblyCopyTitleExt(destination, written));
        if (written[0] > destination.length) {
            destination = new byte[Math.toIntExact(written[0])];
            check("CnaRuntime.getTitle",
                    NativeRuntimeExtensionRoutes.assemblyCopyTitleExt(destination, written));
        }
        return NativeGamerServices.string(destination,
                Math.min(Math.toIntExact(written[0]), destination.length));
    }

    public static void setTitle(String value) {
        NativeBindings.requireAvailable();
        check("CnaRuntime.setTitle", NativeRuntimeExtensionRoutes.assemblySetTitleExt(
                NativeGamerServices.utf8(Objects.requireNonNull(value, "value"))));
    }

    private static String text(String operation, NativeGamerServices.SizeQuery size,
            NativeGamerServices.CopyQuery copy) {
        NativeBindings.requireAvailable();
        return NativeGamerServices.text(operation, size, copy);
    }

    /**
     * Loads the native bridge and maps one CNA result.
     *
     * <p>Loading here rather than at each call site is what lets a game ask CNA about itself
     * before it has created a {@code Game}: nothing else in this package has loaded the bridge
     * yet, and an external consumer reaches these routes first.
     */
    private static void check(String operation, int result) {
        if (result != 0) {
            throw NativeBindings.failure(operation, result);
        }
    }
}
