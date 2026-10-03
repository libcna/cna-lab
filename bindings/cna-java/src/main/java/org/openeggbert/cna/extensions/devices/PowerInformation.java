package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.time.Duration;

/**
 * The host's power state, for a game that wants to lower its own cost on battery.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. A percentage or a remaining time the host will
 * not report comes back as absent rather than as zero, because zero means empty.
 */
public final class PowerInformation {

    /** CNA reports an unknown percentage or remaining time as -1. */
    private static final int UNKNOWN = -1;

    private PowerInformation() {
    }

    public static PowerState getState() {
        int[] state = new int[1];
        DeviceExtension.check("PowerInformation.getState",
                NativeDeviceExtensionRoutes.powerGetStateExt(
                        DeviceExtension.game("PowerInformation"), state));
        return PowerState.values()[state[0]];
    }

    /**
     * Returns the battery charge as a percentage, or {@code null} when the host will not say.
     *
     * <p>Absent is not zero: zero means an empty battery, and a machine with no battery or no
     * answer is a different thing.
     */
    public static Integer getBatteryPercent() {
        int[] percent = new int[1];
        DeviceExtension.check("PowerInformation.getBatteryPercent",
                NativeDeviceExtensionRoutes.powerGetBatteryPercentExt(
                        DeviceExtension.game("PowerInformation"), percent));
        return percent[0] == UNKNOWN ? null : percent[0];
    }

    /** Returns the remaining runtime, or {@code null} when the host will not say. */
    public static Duration getRemainingRuntime() {
        int[] seconds = new int[1];
        DeviceExtension.check("PowerInformation.getRemainingRuntime",
                NativeDeviceExtensionRoutes.powerGetSecondsRemainingExt(
                        DeviceExtension.game("PowerInformation"), seconds));
        return seconds[0] == UNKNOWN ? null : Duration.ofSeconds(seconds[0]);
    }

    /**
     * Reads the state, the remaining runtime and the charge in one call.
     *
     * <p>Not a convenience over the three getters above: it is the only way to get three values
     * that describe the same instant. Asked one at a time, a machine unplugged between the first
     * and the third answers {@code Charging} with no remaining runtime, which is a state no
     * machine is ever in.
     *
     * @return the three values, as one reading
     */
    public static PowerSnapshot getSnapshot() {
        int[] state = new int[1];
        int[] seconds = new int[1];
        int[] percent = new int[1];
        DeviceExtension.check("PowerInformation.getSnapshot",
                NativeDeviceExtensionRoutes.powerGetInfo(
                        DeviceExtension.game("PowerInformation"), state, seconds, percent));
        return new PowerSnapshot(PowerState.values()[state[0]],
                seconds[0] == UNKNOWN ? null : Duration.ofSeconds(seconds[0]),
                percent[0] == UNKNOWN ? null : percent[0]);
    }

    /**
     * The host's power state, remaining runtime and charge, all describing one instant.
     *
     * @param state the power state, which CNA always reports
     * @param remainingRuntime the runtime left, or {@code null} when the host will not say
     * @param batteryPercent the charge from 0 through 100, or {@code null} when the host will
     *        not say
     */
    public record PowerSnapshot(PowerState state, Duration remainingRuntime,
            Integer batteryPercent) {
    }
}
