package org.openeggbert.cna.extensions.runtime;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/** What CNA reports about the runtime, asserted against what CNA actually says. */
final class CnaRuntimeTests {

    @Test
    void logLevelCarriesCnasOwnNumbersRatherThanJavaOrdinals() {
        // Experiment is 100, not 6, so the value cannot come from the ordinal.
        assertEquals(0, LogLevel.Fatal.getValue());
        assertEquals(5, LogLevel.Trace.getValue());
        assertEquals(100, LogLevel.Experiment.getValue());
        assertEquals(7, LogLevel.values().length);
        assertEquals(9, LogCategory.values().length);
        assertEquals(4, CnaPlatform.values().length);
        assertEquals(4, DesktopOperatingSystem.values().length);
        assertEquals(5, GraphicsBackendCategory.values().length);
        assertEquals(5, GraphicsBackendMaturity.values().length);
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void theRuntimeDescribesTheBuildThatIsActuallyLoaded() {
        assertEquals(CnaPlatform.Desktop, CnaRuntime.getPlatform());
        assertFalse(CnaRuntime.isMobile());
        assertNotNull(CnaRuntime.getPlatformName());
        assertFalse(CnaRuntime.getPlatformName().isEmpty());
        assertEquals(DesktopOperatingSystem.Linux, CnaRuntime.getDesktopOperatingSystem());

        // The compiled-in renderer, which on these single-renderer builds is also the running
        // one; the device answers the same question independently.
        assertTrue(java.util.List.of("HEADLESS", "OPENGLES3", "OPENGL33", "SOFTWARE", "VULKAN")
                .contains(CnaRuntime.getRendererName()), CnaRuntime.getRendererName());
        assertNotNull(CnaRuntime.getBackendCategory());
        assertNotNull(CnaRuntime.getBackendMaturity());
        assertFalse(CnaRuntime.getName(CnaRuntime.getBackendCategory()).isEmpty());
        assertFalse(CnaRuntime.getName(CnaRuntime.getBackendMaturity()).isEmpty());
        assertThrows(NullPointerException.class,
                () -> CnaRuntime.getName((GraphicsBackendCategory) null));
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void theTitleRoundTripsAndIsRestored() {
        String title = CnaRuntime.getTitle();
        assertNotNull(title);
        try {
            CnaRuntime.setTitle("cna-java extension probe");
            assertEquals("cna-java extension probe", CnaRuntime.getTitle());
        } finally {
            CnaRuntime.setTitle(title);
        }
        assertEquals(title, CnaRuntime.getTitle());
        assertThrows(NullPointerException.class, () -> CnaRuntime.setTitle(null));
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void theLoggerAcceptsEveryLevelAndItsThresholdIsRestored() {
        LogLevel minimum = CnaLogger.getMinimumLevel();
        assertNotNull(minimum);
        try {
            CnaLogger.setMinimumLevel(LogLevel.Trace);
            assertEquals(LogLevel.Trace, CnaLogger.getMinimumLevel());
            for (LogLevel level : LogLevel.values()) {
                CnaLogger.Log(level, "cna-java extension probe", LogCategory.Test);
            }
            // A conditional message that does not hold must still be accepted, not refused.
            CnaLogger.Log(LogLevel.Info, "not written", LogCategory.Test, false);
            CnaLogger.Info("cna-java extension probe", LogCategory.Application);
        } finally {
            CnaLogger.setMinimumLevel(minimum);
        }
        assertEquals(minimum, CnaLogger.getMinimumLevel());
        assertThrows(NullPointerException.class,
                () -> CnaLogger.Log(null, "message", LogCategory.Test));
        assertThrows(NullPointerException.class,
                () -> CnaLogger.Info(null, LogCategory.Test));
        assertThrows(NullPointerException.class,
                () -> CnaLogger.Info("message", null));
    }

    @Test
    void everyRuntimeQueryNeedsTheNativeBackend() {
        // With no backend loaded these must fail rather than answer with a guess. With one
        // loaded they answer, which the tests above assert.
        assertTrue(nativeEnabled() || throwsWithoutBackend());
    }

    private static boolean throwsWithoutBackend() {
        try {
            CnaRuntime.getPlatform();
            return false;
        } catch (RuntimeException | LinkageError expected) {
            return true;
        }
    }

    private static boolean nativeEnabled() {
        return System.getenv("CNA_NATIVE_LIBRARY") != null;
    }
}
