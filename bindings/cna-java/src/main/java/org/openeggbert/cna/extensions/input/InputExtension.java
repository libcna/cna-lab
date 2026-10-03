package org.openeggbert.cna.extensions.input;

import org.openeggbert.cna.internal.NativeBindings;

/**
 * Shared plumbing for the input extensions.
 *
 * <p>These reach the host through the platform the game created, so they need a running game.
 * A route the host does not implement answers {@code CNA_RESULT_NOT_SUPPORTED}, which keeps its
 * own identity here rather than being flattened into an ordinary failure.
 */
final class InputExtension {

    private static final int RESULT_SUCCESS = 0;
    private static final int RESULT_NOT_SUPPORTED = 6;

    private InputExtension() {
    }

    static long game(String owner) {
        NativeBindings.requireAvailable();
        return NativeBindings.currentGameHandleValue(owner);
    }

    static void check(String operation, int result) {
        if (result == RESULT_SUCCESS) {
            return;
        }
        if (result == RESULT_NOT_SUPPORTED) {
            throw new InputNotSupportedException(operation
                    + " is not supported by this CNA build or by this host");
        }
        throw NativeBindings.failure(operation, result);
    }
}
