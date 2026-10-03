package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.time.Duration;
import java.util.Objects;

/**
 * The host's vibration motor, for a handheld or a controller that has one.
 *
 * <p>A CNA extension. XNA had {@code Microsoft.Devices.VibrateController} on Windows Phone only,
 * and {@code GamePad.SetVibration} for a controller; this is the host's own motor.
 *
 * <p>{@link #getIsSupported()} is what a game checks first: a machine without a motor says so
 * rather than accepting a request that does nothing.
 */
public final class VibrateController {

    private VibrateController() {
    }

    public static boolean getIsSupported() {
        boolean[] supported = new boolean[1];
        DeviceExtension.check("VibrateController.IsSupported",
                NativeDeviceExtensionRoutes.vibrateControllerGetIsSupportedExt(
                        DeviceExtension.game("VibrateController"), supported));
        return supported[0];
    }

    /** Returns the host's name for the vibrating device, empty when it has none. */
    public static String getDeviceName() {
        long game = DeviceExtension.game("VibrateController");
        return NativeGamerServices.text("VibrateController.DeviceName",
                out -> NativeDeviceExtensionRoutes.vibrateControllerGetDeviceNameSizeExt(game, out),
                (buffer, out) -> NativeDeviceExtensionRoutes.vibrateControllerCopyDeviceNameExt(
                        game, buffer, out));
    }

    /** Vibrates for a duration at the host's default intensity. */
    public static void Start(Duration duration) {
        DeviceExtension.check("VibrateController.Start",
                NativeDeviceExtensionRoutes.vibrateControllerStart(
                        DeviceExtension.game("VibrateController"), ticks(duration)));
    }

    /** Vibrates for a duration at an intensity between zero and one. */
    public static void Start(Duration duration, float intensity) {
        DeviceExtension.check("VibrateController.Start",
                NativeDeviceExtensionRoutes.vibrateControllerStartWithIntensityExt(
                        DeviceExtension.game("VibrateController"), ticks(duration), intensity));
    }

    /** Vibrates both motors independently, as a two-motor controller allows. */
    public static void Start(Duration duration, float largeMotor, float smallMotor) {
        DeviceExtension.check("VibrateController.Start",
                NativeDeviceExtensionRoutes.vibrateControllerStartLeftRightExt(
                        DeviceExtension.game("VibrateController"),
                        largeMotor, smallMotor, ticks(duration)));
    }

    public static void Stop() {
        DeviceExtension.check("VibrateController.Stop",
                NativeDeviceExtensionRoutes.vibrateControllerStop(
                        DeviceExtension.game("VibrateController")));
    }

    private static long ticks(Duration duration) {
        return NativeGamerServices.ticks(Objects.requireNonNull(duration, "duration"));
    }
}
