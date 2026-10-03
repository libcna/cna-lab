package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.GamerEventPump;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.function.BiConsumer;

/**
 * Enumerates the mice, keyboards and touch devices the host reports, and reports hot plug.
 *
 * <p>A CNA extension: XNA 4.0 has no device enumeration at all. It is metadata, not a second way
 * to read input -- {@code Mouse.GetState} and {@code Keyboard.GetState} stay merged across every
 * device, which is XNA's shape and CNA's.
 *
 * <p><strong>An enumeration is a snapshot.</strong> Each call takes a fresh one, so an index is
 * only meaningful until the device set changes; the identifier in {@link InputDeviceInfo} is what
 * survives, and it is what a hot-plug event carries. Nothing here is cached, so a device that was
 * unplugged between two calls is simply absent from the second.
 *
 * <p>Hot-plug events arrive during {@code FrameworkDispatcher.Update}, on the game thread, like
 * every other CNA event this binding delivers. CNA's hot-plug events are static, so the native
 * subscription belongs to the process and outlives any one game; adding a Java listener does not
 * create a new one.
 */
public final class InputDevices {

    private static final int KIND_MOUSE_CONNECTED = 31;
    private static final int KIND_MOUSE_DISCONNECTED = 32;
    private static final int KIND_KEYBOARD_CONNECTED = 33;
    private static final int KIND_KEYBOARD_DISCONNECTED = 34;

    private static final List<BiConsumer<InputDeviceKind, Long>> CONNECTED =
            new CopyOnWriteArrayList<>();
    private static final List<BiConsumer<InputDeviceKind, Long>> DISCONNECTED =
            new CopyOnWriteArrayList<>();

    static {
        GamerEventPump.addInputHandler(InputDevices::dispatch);
    }

    private InputDevices() {
    }

    /** Returns a fresh snapshot of the mice the host reports. */
    public static List<InputDeviceInfo> getMice() {
        return enumerate(InputDeviceKind.Mouse);
    }

    /** Returns a fresh snapshot of the keyboards the host reports. */
    public static List<InputDeviceInfo> getKeyboards() {
        return enumerate(InputDeviceKind.Keyboard);
    }

    /** Returns a fresh snapshot of the touch devices the host reports. */
    public static List<InputDeviceInfo> getTouchDevices() {
        return enumerate(InputDeviceKind.TouchDevice);
    }

    /** Returns a fresh snapshot of the devices of one kind. */
    public static List<InputDeviceInfo> enumerate(InputDeviceKind kind) {
        Objects.requireNonNull(kind, "kind");
        long game = DeviceExtension.game("InputDevices");
        int[] count = new int[1];
        DeviceExtension.check("InputDevices.count", switch (kind) {
            case Mouse -> NativeDeviceExtensionRoutes.inputDevicesGetMouseCount(game, count);
            case Keyboard -> NativeDeviceExtensionRoutes.inputDevicesGetKeyboardCount(game, count);
            case TouchDevice ->
                    NativeDeviceExtensionRoutes.inputDevicesGetTouchDeviceCount(game, count);
        });
        List<InputDeviceInfo> devices = new ArrayList<>(count[0]);
        for (int index = 0; index < count[0]; index++) {
            long[] info = new long[1];
            DeviceExtension.check("InputDevices.info", switch (kind) {
                case Mouse ->
                        NativeDeviceExtensionRoutes.inputDevicesGetMouseInfoAt(game, index, info);
                case Keyboard ->
                        NativeDeviceExtensionRoutes.inputDevicesGetKeyboardInfoAt(game, index, info);
                case TouchDevice -> NativeDeviceExtensionRoutes
                        .inputDevicesGetTouchDeviceInfoAt(game, index, info);
            });
            devices.add(new InputDeviceInfo(info[0], name(kind, game, index)));
        }
        return List.copyOf(devices);
    }

    /**
     * Adds a listener for a device being connected.
     *
     * <p>The listener receives the kind and the host's device identifier, and runs on the game
     * thread during {@code FrameworkDispatcher.Update}.
     *
     * @param listener called with the device kind and the connected device's identifier
     */
    public static void addConnectedListener(BiConsumer<InputDeviceKind, Long> listener) {
        CONNECTED.add(Objects.requireNonNull(listener, "listener"));
        subscribe();
    }

    /** Removes a connection listener. */
    public static void removeConnectedListener(BiConsumer<InputDeviceKind, Long> listener) {
        CONNECTED.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Adds a listener for a device being disconnected.
     *
     * @param listener called with the device kind and the disconnected device's identifier
     */
    public static void addDisconnectedListener(BiConsumer<InputDeviceKind, Long> listener) {
        DISCONNECTED.add(Objects.requireNonNull(listener, "listener"));
        subscribe();
    }

    /** Removes a disconnection listener. */
    public static void removeDisconnectedListener(BiConsumer<InputDeviceKind, Long> listener) {
        DISCONNECTED.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Raises the host's own connection event for one device identifier.
     *
     * <p>This is CNA's route, not a Java simulation: it invokes the same event the platform layer
     * invokes on real hot plug, so every subscriber sees it, and it is how a game exercises its
     * own device wiring on a machine where nothing can actually be unplugged. Only mice and
     * keyboards have such an event.
     *
     * @param kind the device family to report, never {@link InputDeviceKind#TouchDevice}
     * @param deviceId the identifier to report
     */
    public static void RaiseConnected(InputDeviceKind kind, int deviceId) {
        long game = DeviceExtension.game("InputDevices");
        DeviceExtension.check("InputDevices.RaiseConnected", switch (hotPluggable(kind)) {
            case Mouse ->
                    NativeDeviceExtensionRoutes.inputDevicesRaiseMouseConnectedExt(game, deviceId);
            case Keyboard -> NativeDeviceExtensionRoutes
                    .inputDevicesRaiseKeyboardConnectedExt(game, deviceId);
            default -> throw new IllegalStateException(kind.name());
        });
    }

    /**
     * Raises the host's own disconnection event for one device identifier.
     *
     * @param kind the device family to report, never {@link InputDeviceKind#TouchDevice}
     * @param deviceId the identifier to report
     */
    public static void RaiseDisconnected(InputDeviceKind kind, int deviceId) {
        long game = DeviceExtension.game("InputDevices");
        DeviceExtension.check("InputDevices.RaiseDisconnected", switch (hotPluggable(kind)) {
            case Mouse -> NativeDeviceExtensionRoutes
                    .inputDevicesRaiseMouseDisconnectedExt(game, deviceId);
            case Keyboard -> NativeDeviceExtensionRoutes
                    .inputDevicesRaiseKeyboardDisconnectedExt(game, deviceId);
            default -> throw new IllegalStateException(kind.name());
        });
    }

    private static InputDeviceKind hotPluggable(InputDeviceKind kind) {
        Objects.requireNonNull(kind, "kind");
        if (kind == InputDeviceKind.TouchDevice) {
            throw new IllegalArgumentException(
                    "CNA raises hot-plug events for mice and keyboards only");
        }
        return kind;
    }

    private static String name(InputDeviceKind kind, long game, int index) {
        long[] bytes = new long[1];
        DeviceExtension.check("InputDevices.nameSize", switch (kind) {
            case Mouse ->
                    NativeDeviceExtensionRoutes.inputDevicesGetMouseNameSizeAt(game, index, bytes);
            case Keyboard -> NativeDeviceExtensionRoutes
                    .inputDevicesGetKeyboardNameSizeAt(game, index, bytes);
            case TouchDevice -> NativeDeviceExtensionRoutes
                    .inputDevicesGetTouchDeviceNameSizeAt(game, index, bytes);
        });
        byte[] destination = new byte[(int) bytes[0]];
        DeviceExtension.check("InputDevices.name", switch (kind) {
            case Mouse -> NativeDeviceExtensionRoutes
                    .inputDevicesCopyMouseNameAt(game, index, destination, bytes);
            case Keyboard -> NativeDeviceExtensionRoutes
                    .inputDevicesCopyKeyboardNameAt(game, index, destination, bytes);
            case TouchDevice -> NativeDeviceExtensionRoutes
                    .inputDevicesCopyTouchDeviceNameAt(game, index, destination, bytes);
        });
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private static void subscribe() {
        NativeBindings.requireAvailable();
        GamerEventPump.ensureInputDevicesSubscribed();
    }

    private static void dispatch(long kind, long session, long first, long second, long flag) {
        List<BiConsumer<InputDeviceKind, Long>> listeners;
        InputDeviceKind device;
        switch ((int) kind) {
            case KIND_MOUSE_CONNECTED -> {
                listeners = CONNECTED;
                device = InputDeviceKind.Mouse;
            }
            case KIND_MOUSE_DISCONNECTED -> {
                listeners = DISCONNECTED;
                device = InputDeviceKind.Mouse;
            }
            case KIND_KEYBOARD_CONNECTED -> {
                listeners = CONNECTED;
                device = InputDeviceKind.Keyboard;
            }
            case KIND_KEYBOARD_DISCONNECTED -> {
                listeners = DISCONNECTED;
                device = InputDeviceKind.Keyboard;
            }
            default -> {
                return;
            }
        }
        for (BiConsumer<InputDeviceKind, Long> listener : listeners) {
            listener.accept(device, first);
        }
    }
}
