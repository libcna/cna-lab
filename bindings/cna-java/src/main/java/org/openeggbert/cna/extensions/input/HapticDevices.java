package org.openeggbert.cna.extensions.input;

import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * Finds and opens the host's force-feedback devices.
 *
 * <p>A CNA extension. Three things can be haptic and they are reached differently: a standalone
 * device by its own identifier, a joystick by its joystick identifier, and the mouse. All three
 * hand back a {@link HapticDevice}, and all three succeed even when the hardware is not there --
 * the device that comes back reports {@link HapticDevice#getIsOpen()} false, which is CNA's own
 * contract and the reason opening never throws for absent hardware.
 */
public final class HapticDevices {

    private HapticDevices() {
    }

    /** Returns a fresh snapshot of the standalone haptic devices the host reports. */
    public static List<HapticDeviceInfo> enumerate() {
        long game = InputExtension.game("HapticDevices");
        int[] count = new int[1];
        InputExtension.check("HapticDevices.enumerate",
                NativeInputExtensionRoutes.hapticsGetCount(game, count));
        List<HapticDeviceInfo> devices = new ArrayList<>(count[0]);
        for (int index = 0; index < count[0]; index++) {
            int[] id = new int[1];
            InputExtension.check("HapticDevices.id",
                    NativeInputExtensionRoutes.hapticsGetIdAt(game, index, id));
            devices.add(new HapticDeviceInfo(id[0], nameAt(game, index)));
        }
        return List.copyOf(devices);
    }

    /**
     * Opens a standalone haptic device.
     *
     * @param deviceId an identifier from {@link HapticDeviceInfo#Id()}
     * @return the device, open or not; the caller closes it either way
     */
    public static HapticDevice open(int deviceId) {
        long[] device = new long[1];
        InputExtension.check("HapticDevices.open", NativeInputExtensionRoutes
                .hapticsOpen(InputExtension.game("HapticDevices"), deviceId, device));
        return new HapticDevice(device[0]);
    }

    /**
     * Opens the force feedback of a raw joystick.
     *
     * @param joystickId an identifier from {@link JoystickInfo#Id()}
     * @return the device, open or not; the caller closes it either way
     */
    public static HapticDevice openFromJoystick(int joystickId) {
        long[] device = new long[1];
        InputExtension.check("HapticDevices.openFromJoystick", NativeInputExtensionRoutes
                .hapticsOpenFromJoystick(InputExtension.game("HapticDevices"),
                        joystickId, device));
        return new HapticDevice(device[0]);
    }

    /**
     * Opens the mouse's force feedback.
     *
     * @return the device, open or not; the caller closes it either way
     */
    public static HapticDevice openFromMouse() {
        long[] device = new long[1];
        InputExtension.check("HapticDevices.openFromMouse", NativeInputExtensionRoutes
                .hapticsOpenFromMouse(InputExtension.game("HapticDevices"), device));
        return new HapticDevice(device[0]);
    }

    /**
     * Reports whether one raw joystick has force feedback, without opening it.
     *
     * @param joystickId an identifier from {@link JoystickInfo#Id()}
     * @return whether it is haptic
     */
    public static boolean isJoystickHaptic(int joystickId) {
        boolean[] haptic = new boolean[1];
        InputExtension.check("HapticDevices.isJoystickHaptic", NativeInputExtensionRoutes
                .hapticsGetIsJoystickHaptic(InputExtension.game("HapticDevices"),
                        joystickId, haptic));
        return haptic[0];
    }

    /** Reports whether the mouse has force feedback, without opening it. */
    public static boolean isMouseHaptic() {
        boolean[] haptic = new boolean[1];
        InputExtension.check("HapticDevices.isMouseHaptic", NativeInputExtensionRoutes
                .hapticsGetIsMouseHaptic(InputExtension.game("HapticDevices"), haptic));
        return haptic[0];
    }

    private static String nameAt(long game, int index) {
        long[] bytes = new long[1];
        InputExtension.check("HapticDevices.nameSize",
                NativeInputExtensionRoutes.hapticsGetNameSizeAt(game, index, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        InputExtension.check("HapticDevices.name",
                NativeInputExtensionRoutes.hapticsCopyNameAt(game, index, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }
}
