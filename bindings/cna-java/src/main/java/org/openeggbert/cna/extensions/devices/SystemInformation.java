package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

/**
 * What CNA can tell a game about the machine it is on.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. A game uses it to size its own worker pool or
 * its asset budget instead of guessing.
 */
public final class SystemInformation {

    private SystemInformation() {
    }

    /** Returns the number of logical CPU cores the host reports. */
    public static int getLogicalCpuCoreCount() {
        int[] count = new int[1];
        DeviceExtension.check("SystemInformation.getLogicalCpuCoreCount",
                NativeDeviceExtensionRoutes.systemInfoGetLogicalCpuCoreCountExt(
                        DeviceExtension.game("SystemInformation"), count));
        return count[0];
    }

    /** Returns the host's system memory in megabytes. */
    public static int getSystemRamMegabytes() {
        int[] megabytes = new int[1];
        DeviceExtension.check("SystemInformation.getSystemRamMegabytes",
                NativeDeviceExtensionRoutes.systemInfoGetSystemRamMegabytesExt(
                        DeviceExtension.game("SystemInformation"), megabytes));
        return megabytes[0];
    }

    /**
     * Reports whether the game is running on real hardware or in an emulator.
     *
     * <p>The one device query CNA answers without a game: it reads the build's own target rather
     * than the running platform, so there is nothing to reach through.
     *
     * @return what CNA says this build runs on
     */
    public static DeviceType getDeviceType() {
        int[] kind = new int[1];
        DeviceExtension.check("SystemInformation.getDeviceType",
                NativeDeviceExtensionRoutes.environmentGetDeviceType(kind));
        return DeviceType.of(kind[0]);
    }
}
