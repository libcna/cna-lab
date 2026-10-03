package org.openeggbert.cna.extensions.graphics;

import org.openeggbert.cna.internal.CnaNativeException;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

/**
 * Reports whether this CNA build carries the extended graphics layer.
 *
 * <p>The layer is an opt-in CNA build option. Its declarations exist in every build so the
 * exported ABI never changes shape, and the routes that need a native extension object answer
 * {@code CNA_RESULT_NOT_SUPPORTED} when it is absent. That distinction is preserved rather than
 * flattened: {@link #isAvailable()} answers truthfully, and a route on a build without the layer
 * raises {@link ExtensionNotSupportedException} instead of quietly doing something else.
 */
public final class GraphicsExtension {

    private static final int RESULT_SUCCESS = 0;
    private static final int RESULT_INVALID_ARGUMENT = 1;
    private static final int RESULT_INVALID_STATE = 3;
    private static final int RESULT_NOT_SUPPORTED = 6;
    private static final int RESULT_BUFFER_TOO_SMALL = 14;

    private GraphicsExtension() {
    }

    /**
     * Reports whether the loaded CNA build contains the extended graphics layer.
     *
     * <p>Answers {@code false} rather than failing when no native backend is loaded at all, so a
     * game can ask before it does anything native.
     */
    public static boolean isAvailable() {
        if (!NativeBindings.isAvailable()) {
            return false;
        }
        boolean[] available = new boolean[1];
        check("GraphicsExtension.isAvailable",
                NativeGraphicsExtensionRoutes.graphicsExtIsAvailable(available));
        return available[0];
    }

    /**
     * Loads the native bridge, so an external consumer can reach this package first.
     *
     * <p>Nothing else in it has loaded the bridge: a game may ask CNA about its extended layer
     * before it creates a {@code Game}, and every route here would otherwise fail to link.
     */
    static void requireBackend() {
        NativeBindings.requireAvailable();
    }

    /**
     * Maps one CNA result for the extension surface.
     *
     * <p>{@code NOT_SUPPORTED} keeps its own identity: a build without the extended layer is a
     * different thing from a call that failed, and a game that catches the first can fall back
     * without swallowing the second. {@code INVALID_ARGUMENT} and {@code INVALID_STATE} become
     * the Java exceptions that already mean those things, each carrying the native failure as
     * its cause, so a caller never has to catch an internal type for an ordinary mistake.
     */
    static void check(String operation, int result) {
        if (result == RESULT_SUCCESS) {
            return;
        }
        if (result == RESULT_NOT_SUPPORTED) {
            // CNA answers NOT_SUPPORTED to two different questions -- a build compiled without
            // the extended layer, and a renderer that has the layer but cannot do this one
            // thing -- and does not separate them, so neither is named alone.
            throw new ExtensionNotSupportedException(operation
                    + " is not available: either this build has no extended graphics layer, or"
                    + " this renderer cannot do it");
        }
        CnaNativeException failure = NativeBindings.failure(operation, result);
        if (result == RESULT_INVALID_ARGUMENT) {
            // A value the caller passed, not a native fault. Java has a name for that, and a
            // caller catching IllegalArgumentException should not have to know that the check
            // happened on the other side of JNI.
            throw new IllegalArgumentException(failure.getMessage(), failure);
        }
        if (result == RESULT_INVALID_STATE) {
            // The same argument one step along: an object asked to do something its current
            // configuration forbids is what IllegalStateException names, and a caller should
            // not have to catch an internal type to handle it. The native result stays
            // reachable as the cause.
            throw new IllegalStateException(failure.getMessage(), failure);
        }
        throw failure;
    }
}
