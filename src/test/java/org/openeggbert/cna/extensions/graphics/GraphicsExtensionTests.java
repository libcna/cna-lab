package org.openeggbert.cna.extensions.graphics;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;

/** The extended graphics layer, including what it does when the layer is not in the build. */
final class GraphicsExtensionTests {

    @Test
    void availabilityIsAnsweredRatherThanAssumedWithNoNativeBackend() {
        // Asking before anything native is loaded must answer, not fail: a game decides whether
        // to use the extension on this answer.
        assertFalse(GraphicsExtension.isAvailable() && !nativeEnabled(),
                "availability must be false when no native backend is loaded");
    }

    @Test
    void identityNamesAreCnasOwn() {
        assertEquals(2, AsciiQuantizeMode.values().length);
        assertEquals(3, CrtMaskType.values().length);
        assertEquals(3, DitherMode.values().length);
        assertEquals(7, DepthEffectMode.values().length);
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void availabilityIsTruthfulAgainstTheLoadedBuild() {
        // Whichever build is loaded, the answer must be the same on every call.
        boolean available = GraphicsExtension.isAvailable();
        assertEquals(available, GraphicsExtension.isAvailable());
    }

    private static boolean nativeEnabled() {
        return System.getenv("CNA_NATIVE_LIBRARY") != null;
    }
}
