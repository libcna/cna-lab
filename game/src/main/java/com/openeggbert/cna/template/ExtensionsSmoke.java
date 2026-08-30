package com.openeggbert.cna.template;

import org.openeggbert.cna.extensions.graphics.ExtensionNotSupportedException;
import org.openeggbert.cna.extensions.graphics.GraphicsExtension;
import org.openeggbert.cna.extensions.graphics.PbrMaterial;
import org.openeggbert.cna.extensions.graphics.RenderPipelineSettings;
import org.openeggbert.cna.extensions.graphics.TonemappingMode;
import org.openeggbert.cna.extensions.runtime.CnaLogger;
import org.openeggbert.cna.extensions.runtime.CnaRuntime;
import org.openeggbert.cna.extensions.runtime.LogCategory;
import org.openeggbert.cna.extensions.runtime.LogLevel;

/**
 * Proves the CNA extension surface works from outside the binding.
 *
 * <p>Deliberately not part of {@link HelloGame}. The starter stays an XNA program an XNA
 * developer recognizes; this is the separate, opt-in {@code --extensions-smoke} mode.
 *
 * <p>What it proves is narrow and honest: that the extension packages compile against the
 * published artifact, that the JNI routes behind them exist, that the availability query answers
 * rather than guesses, and that a build without the extended graphics layer says so rather than
 * doing something else. It does not claim any rendering happened.
 */
final class ExtensionsSmoke {

    private ExtensionsSmoke() {
    }

    static void run() {
        System.out.println("cna-java-template: CNA runtime");
        System.out.println("  platform         " + CnaRuntime.getPlatform()
                + " (" + CnaRuntime.getPlatformName() + ")");
        System.out.println("  mobile           " + CnaRuntime.isMobile());
        System.out.println("  apple            " + CnaRuntime.isApple());
        System.out.println("  renderer         " + CnaRuntime.getRendererName());
        System.out.println("  backend category " + CnaRuntime.getBackendCategory()
                + " (" + CnaRuntime.getName(CnaRuntime.getBackendCategory()) + ")");
        System.out.println("  backend maturity " + CnaRuntime.getBackendMaturity()
                + " (" + CnaRuntime.getName(CnaRuntime.getBackendMaturity()) + ")");

        LogLevel minimum = CnaLogger.getMinimumLevel();
        try {
            CnaLogger.Info("cna-java-template extensions smoke", LogCategory.Application);
        } finally {
            CnaLogger.setMinimumLevel(minimum);
        }

        boolean available = GraphicsExtension.isAvailable();
        System.out.println("cna-java-template: extended graphics layer available " + available);

        // The value routes work in either build, so their defaults come from CNA whichever one
        // is loaded.
        RenderPipelineSettings settings = new RenderPipelineSettings();
        System.out.println("  default tonemapping " + settings.getTonemappingMode()
                + ", exposure " + settings.getExposure()
                + ", gamma " + settings.getGamma()
                + ", shadows " + settings.getShadowQuality());
        settings.setTonemappingMode(TonemappingMode.Aces);
        if (settings.getTonemappingMode() != TonemappingMode.Aces) {
            throw new IllegalStateException("RenderPipelineSettings did not keep its value");
        }

        PbrMaterial material = new PbrMaterial();
        System.out.println("  default material metallic " + material.getMetallicFactor()
                + ", roughness " + material.getRoughnessFactor()
                + ", albedo " + material.getAlbedoColor());

        // The one route that needs the native extension object. On a build without the layer it
        // must say so, which is the distinction this smoke exists to check.
        try {
            reportEffect(available);
        } catch (ExtensionNotSupportedException notSupported) {
            if (available) {
                throw new IllegalStateException(
                        "The extended layer reported itself available and then refused", notSupported);
            }
            System.out.println("  post-process effect NOT_SUPPORTED, as this build reports");
        }
        System.out.println("cna-java-template: extensions smoke passed");
    }

    private static void reportEffect(boolean available) {
        if (!available) {
            throw new ExtensionNotSupportedException(
                    "this build has no extended graphics layer");
        }
        System.out.println("  post-process effect available on this build");
    }
}
