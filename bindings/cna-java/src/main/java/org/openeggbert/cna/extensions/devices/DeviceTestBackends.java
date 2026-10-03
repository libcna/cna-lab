package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.time.Duration;
import java.util.List;
import java.util.Objects;

/**
 * CNA's own stand-in backends for the host capabilities no automated test can complete.
 *
 * <p>A message box waits for a person, a file dialog waits for a person, and a vibration motor
 * either exists or does not. A game that wants any of the three under test needs the host taken
 * out of the loop, and CNA provides exactly that: each backend answers immediately and records
 * what it was asked for. These are part of CNA's published extension ABI rather than a private
 * seam, which is why they are here rather than hidden -- a consumer's own tests need them for
 * the same reason this projection's do.
 *
 * <p><strong>Every backend here is process-wide.</strong> CNA's own is, so this switch is too:
 * it is not scoped to the game that validates the call, and installing one affects every message
 * box or dialog anywhere in the process until it is removed. Tests that install one should
 * remove it again.
 *
 * <p>A tray is the exception and is not here: CNA takes its backend as a construction argument
 * rather than a switch, so it is {@link SystemTray#CreateForTests(String)}.
 */
public final class DeviceTestBackends {

    private DeviceTestBackends() {
    }

    /**
     * Installs the message-box backend, which answers without showing anything.
     *
     * <p>Installing resets the log.
     *
     * @param chosenButton the index the backend answers for a button-answering request
     */
    public static void installMessageBoxBackend(int chosenButton) {
        DeviceExtension.check("DeviceTestBackends.installMessageBoxBackend",
                NativeDeviceExtensionRoutes.messageBoxSetTestBackendExt(
                        DeviceExtension.game("DeviceTestBackends"), true, chosenButton));
    }

    /** Restores the platform's own message boxes. */
    public static void removeMessageBoxBackend() {
        DeviceExtension.check("DeviceTestBackends.removeMessageBoxBackend",
                NativeDeviceExtensionRoutes.messageBoxSetTestBackendExt(
                        DeviceExtension.game("DeviceTestBackends"), false, 0));
    }

    /**
     * Reads what the installed message-box backend has been asked to show.
     *
     * @return the log
     * @throws DeviceNotSupportedException when this build has no device extensions
     * @throws RuntimeException when no message-box backend is installed, which CNA reports as
     *         {@code INVALID_STATE} rather than as an empty log
     */
    public static MessageBoxTestLog messageBoxLog() {
        long[] values = new long[4];
        DeviceExtension.check("DeviceTestBackends.messageBoxLog",
                NativeDeviceExtensionRoutes.messageBoxGetTestLogExt(
                        DeviceExtension.game("DeviceTestBackends"), values));
        return new MessageBoxTestLog((int) values[0], (int) values[1],
                MessageBoxType.of((int) values[2]), (int) values[3]);
    }

    /**
     * Installs the file-dialog backend, which answers with these paths and shows nothing.
     *
     * @param results the paths every dialog answers with; an empty list is what a cancelled
     *        dialog reports, which is how the canonical dialog reports one
     */
    public static void installFileDialogBackend(List<String> results) {
        Objects.requireNonNull(results, "results");
        byte[][] paths = new byte[results.size()][];
        for (int index = 0; index < paths.length; index++) {
            paths[index] = NativeGamerServices.utf8(
                    Objects.requireNonNull(results.get(index), "result"));
        }
        DeviceExtension.check("DeviceTestBackends.installFileDialogBackend",
                NativeDeviceExtensionRoutes.fileDialogSetTestBackendExt(
                        DeviceExtension.game("DeviceTestBackends"), true, paths));
    }

    /** Restores the platform's own file dialogs. */
    public static void removeFileDialogBackend() {
        DeviceExtension.check("DeviceTestBackends.removeFileDialogBackend",
                NativeDeviceExtensionRoutes.fileDialogSetTestBackendExt(
                        DeviceExtension.game("DeviceTestBackends"), false, new byte[0][]));
    }

    /**
     * Installs the vibration backend, which records requests and moves no motor.
     *
     * @param supported what {@link VibrateController#getIsSupported()} then answers
     * @param deviceName what {@link VibrateController#getDeviceName()} then answers
     */
    public static void installVibrationBackend(boolean supported, String deviceName) {
        Objects.requireNonNull(deviceName, "deviceName");
        DeviceExtension.check("DeviceTestBackends.installVibrationBackend",
                NativeDeviceExtensionRoutes.vibrateControllerSetTestBackendExt(
                        DeviceExtension.game("DeviceTestBackends"), true, supported,
                        NativeGamerServices.utf8(deviceName)));
    }

    /** Restores the platform's own vibration. */
    public static void removeVibrationBackend() {
        DeviceExtension.check("DeviceTestBackends.removeVibrationBackend",
                NativeDeviceExtensionRoutes.vibrateControllerSetTestBackendExt(
                        DeviceExtension.game("DeviceTestBackends"), false, false, new byte[0]));
    }

    /**
     * Reads what the installed vibration backend has been asked for.
     *
     * @return the log
     */
    public static VibrationTestLog vibrationLog() {
        long[] integral = new long[5];
        float[] floating = new float[4];
        DeviceExtension.check("DeviceTestBackends.vibrationLog",
                NativeDeviceExtensionRoutes.vibrateControllerGetTestLogExt(
                        DeviceExtension.game("DeviceTestBackends"), integral, floating));
        return new VibrationTestLog((int) integral[0], (int) integral[1], (int) integral[2],
                Duration.ofNanos(integral[4] * 100L), floating[0], floating[1], floating[2]);
    }

    /**
     * What the message-box backend was asked to show.
     *
     * @param simpleCalls how many dismiss-only boxes were requested
     * @param choiceCalls how many button-answering boxes were requested
     * @param lastType the severity of the most recent request
     * @param lastButtonCount how many labels the most recent button-answering request had
     */
    public record MessageBoxTestLog(int simpleCalls, int choiceCalls, MessageBoxType lastType,
            int lastButtonCount) {
    }

    /**
     * What the vibration backend was asked for.
     *
     * @param startCalls how many single-intensity starts were requested
     * @param stopCalls how many stops were requested
     * @param leftRightCalls how many two-motor starts were requested
     * @param lastDuration the most recent request's duration
     * @param lastIntensity the most recent single-intensity request's intensity
     * @param lastLargeMotor the most recent two-motor request's large motor
     * @param lastSmallMotor the most recent two-motor request's small motor
     */
    public record VibrationTestLog(int startCalls, int stopCalls, int leftRightCalls,
            Duration lastDuration, float lastIntensity, float lastLargeMotor,
            float lastSmallMotor) {
    }
}
