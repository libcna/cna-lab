package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

/**
 * Reports whether this CNA build carries the device extensions, and holds their shared plumbing.
 *
 * <p>The device layer is an opt-in CNA build option, declared in every build so the exported ABI
 * never changes shape. A build without it answers {@code CNA_RESULT_NOT_SUPPORTED}, which stays
 * its own identity here rather than being flattened into an ordinary failure.
 */
public final class DeviceExtension {

    private static final int RESULT_SUCCESS = 0;
    private static final int RESULT_NOT_SUPPORTED = 6;

    private DeviceExtension() {
    }

    /**
     * Reports whether the loaded CNA build contains the device extensions.
     *
     * <p>Answers {@code false} rather than failing with no native backend loaded, so a game can
     * ask before it does anything native.
     */
    public static boolean isAvailable() {
        if (!NativeBindings.isAvailable()) {
            return false;
        }
        boolean[] available = new boolean[1];
        check("DeviceExtension.isAvailable",
                NativeDeviceExtensionRoutes.devicesExtIsAvailable(available));
        return available[0];
    }

    /**
     * Returns the running game's native handle.
     *
     * <p>Every device capability reaches the host through the platform the game created, so
     * there is nothing to ask before a game exists. That is CNA's shape, not an added
     * restriction.
     */
    static long game(String owner) {
        NativeBindings.requireAvailable();
        return NativeBindings.currentGameHandleValue(owner);
    }

    /** Maps one CNA result, keeping {@code NOT_SUPPORTED} as its own identity. */
    static void check(String operation, int result) {
        if (result == RESULT_SUCCESS) {
            return;
        }
        if (result == RESULT_NOT_SUPPORTED) {
            throw new DeviceNotSupportedException(operation
                    + " is not supported by this CNA build or by this host");
        }
        throw NativeBindings.failure(operation, result);
    }
}
