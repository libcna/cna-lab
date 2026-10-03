package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Point;
import org.openeggbert.cna.extensions.devices.PowerState;
import org.openeggbert.cna.internal.GamerEventPump;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.function.IntConsumer;

/**
 * Raw joysticks: every axis, button, POV hat and trackball the host reports, unnormalised.
 *
 * <p>A CNA extension. XNA 4.0 has only {@code GamePad}, which is an Xbox controller's shape: four
 * named face buttons, two sticks, two triggers. A wheel, a flight stick, a dance pad or a
 * six-axis HOTAS does not fit it, and forcing one to would either drop inputs or invent them.
 * This surface reports what the device actually has.
 *
 * <p><strong>Raw joystick state is not game-pad state.</strong> A device the host also maps as a
 * game pad -- {@link JoystickType#GamePad} -- can be read through
 * {@code Microsoft.Xna.Framework.Input.GamePad} as well, and the two readings are independent:
 * nothing here normalises, applies a dead zone, or renames a button.
 *
 * <p>Enumeration is a snapshot, as it is for every other CNA device list. The identifier in
 * {@link JoystickInfo} is what the other calls take and what a hot-plug event carries; an index
 * is only meaningful until the device set changes.
 */
public final class Joysticks {

    private static final int KIND_CONNECTED = 35;
    private static final int KIND_DISCONNECTED = 36;

    /** CNA reports an unknown battery percentage as -1, as the host device layer does. */
    private static final int UNKNOWN_PERCENT = -1;

    private static final List<IntConsumer> CONNECTED = new CopyOnWriteArrayList<>();
    private static final List<IntConsumer> DISCONNECTED = new CopyOnWriteArrayList<>();

    static {
        GamerEventPump.addInputHandler(Joysticks::dispatch);
    }

    private Joysticks() {
    }

    /** Returns a fresh snapshot of the raw joysticks the host reports. */
    public static List<JoystickInfo> enumerate() {
        long game = InputExtension.game("Joysticks");
        int[] count = new int[1];
        InputExtension.check("Joysticks.enumerate",
                NativeInputExtensionRoutes.joysticksGetCount(game, count));
        List<JoystickInfo> joysticks = new ArrayList<>(count[0]);
        for (int index = 0; index < count[0]; index++) {
            long[] info = new long[2];
            InputExtension.check("Joysticks.info",
                    NativeInputExtensionRoutes.joysticksGetInfoAt(game, index, info));
            joysticks.add(new JoystickInfo((int) info[0],
                    JoystickType.values()[(int) info[1]], nameAt(game, index)));
        }
        return List.copyOf(joysticks);
    }

    /**
     * Returns the fixed hardware shape of one joystick.
     *
     * @param joystickId the host's joystick instance identifier, from {@link JoystickInfo#Id()}
     * @return its capabilities; a device that is not connected reports {@code IsConnected} false
     *     and zero counts rather than failing
     */
    public static JoystickCapabilities getCapabilities(int joystickId) {
        long game = InputExtension.game("Joysticks");
        byte[] reserved = new byte[3];
        long[] values = new long[8];
        InputExtension.check("Joysticks.getCapabilities", NativeInputExtensionRoutes
                .joysticksGetCapabilities(game, joystickId, reserved, values));
        int percent = (int) values[6];
        return new JoystickCapabilities(
                text(game, joystickId, false),
                text(game, joystickId, true),
                (int) values[0], (int) values[1], (int) values[2], (int) values[3],
                JoystickType.values()[(int) values[4]],
                PowerState.values()[(int) values[5]],
                percent == UNKNOWN_PERCENT ? null : percent,
                values[7] != 0L);
    }

    /**
     * Captures one joystick's axes, buttons, hats and trackballs at a single instant.
     *
     * <p>An identifier that is not connected is not an error: the capture succeeds and every list
     * is empty, which is what CNA's own query does.
     *
     * <p>The native snapshot is released before this returns, so the record that comes back owns
     * nothing. <strong>Capturing consumes trackball motion</strong>, because ball values are
     * relative to the previous capture.
     *
     * @param joystickId the host's joystick instance identifier
     * @return an immutable snapshot
     */
    public static JoystickState captureState(int joystickId) {
        long game = InputExtension.game("Joysticks");
        long[] state = new long[1];
        InputExtension.check("Joysticks.captureState",
                NativeInputExtensionRoutes.joysticksCaptureState(game, joystickId, state));
        long handle = state[0];
        try {
            return readSnapshot(handle);
        } finally {
            InputExtension.check("Joysticks.captureState",
                    NativeInputExtensionRoutes.joystickStateDestroy(handle));
        }
    }

    /**
     * Adds a listener for a joystick being connected.
     *
     * <p>The listener receives the host's joystick identifier and runs on the game thread during
     * {@code FrameworkDispatcher.Update}.
     *
     * @param listener called with the connected joystick's identifier
     */
    public static void addConnectedListener(IntConsumer listener) {
        CONNECTED.add(Objects.requireNonNull(listener, "listener"));
        subscribe();
    }

    /** Removes a connection listener. */
    public static void removeConnectedListener(IntConsumer listener) {
        CONNECTED.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Adds a listener for a joystick being disconnected.
     *
     * @param listener called with the disconnected joystick's identifier
     */
    public static void addDisconnectedListener(IntConsumer listener) {
        DISCONNECTED.add(Objects.requireNonNull(listener, "listener"));
        subscribe();
    }

    /** Removes a disconnection listener. */
    public static void removeDisconnectedListener(IntConsumer listener) {
        DISCONNECTED.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Raises the host's own joystick-connected event for one identifier.
     *
     * <p>CNA's route, not a Java simulation: it invokes the same event the platform layer invokes
     * on real hot plug, which is how a game exercises its own wiring with no controller to plug
     * in.
     *
     * @param joystickId the identifier to report
     */
    public static void RaiseConnected(int joystickId) {
        InputExtension.check("Joysticks.RaiseConnected", NativeInputExtensionRoutes
                .joysticksRaiseConnectedExt(InputExtension.game("Joysticks"), joystickId));
    }

    /**
     * Raises the host's own joystick-disconnected event for one identifier.
     *
     * @param joystickId the identifier to report
     */
    public static void RaiseDisconnected(int joystickId) {
        InputExtension.check("Joysticks.RaiseDisconnected", NativeInputExtensionRoutes
                .joysticksRaiseDisconnectedExt(InputExtension.game("Joysticks"), joystickId));
    }

    private static JoystickState readSnapshot(long handle) {
        int[] count = new int[1];
        long[] copied = new long[1];

        InputExtension.check("JoystickState.axes",
                NativeInputExtensionRoutes.joystickStateGetAxisCount(handle, count));
        short[] axes = new short[count[0]];
        InputExtension.check("JoystickState.axes",
                NativeInputExtensionRoutes.joystickStateCopyAxes(handle, axes, copied));
        List<Short> axisValues = new ArrayList<>(axes.length);
        for (short axis : axes) {
            axisValues.add(axis);
        }

        InputExtension.check("JoystickState.buttons",
                NativeInputExtensionRoutes.joystickStateGetButtonCount(handle, count));
        boolean[] buttons = new boolean[count[0]];
        InputExtension.check("JoystickState.buttons",
                NativeInputExtensionRoutes.joystickStateCopyButtons(handle, buttons, copied));
        List<Boolean> buttonValues = new ArrayList<>(buttons.length);
        for (boolean button : buttons) {
            buttonValues.add(button);
        }

        InputExtension.check("JoystickState.hats",
                NativeInputExtensionRoutes.joystickStateGetHatCount(handle, count));
        int[] hats = new int[count[0]];
        InputExtension.check("JoystickState.hats",
                NativeInputExtensionRoutes.joystickStateCopyHats(handle, hats, copied));
        List<JoystickHatPosition> hatValues = new ArrayList<>(hats.length);
        for (int hat : hats) {
            hatValues.add(JoystickHatPosition.values()[hat]);
        }

        InputExtension.check("JoystickState.balls",
                NativeInputExtensionRoutes.joystickStateGetBallCount(handle, count));
        long[] balls = new long[count[0] * 2];
        InputExtension.check("JoystickState.balls",
                NativeInputExtensionRoutes.joystickStateCopyBalls(handle, balls, copied));
        List<Point> ballValues = new ArrayList<>(count[0]);
        for (int index = 0; index < count[0]; index++) {
            ballValues.add(new Point((int) balls[index * 2], (int) balls[index * 2 + 1]));
        }

        return new JoystickState(axisValues, buttonValues, hatValues, ballValues);
    }

    private static String nameAt(long game, int index) {
        long[] bytes = new long[1];
        InputExtension.check("Joysticks.nameSize",
                NativeInputExtensionRoutes.joysticksGetNameSizeAt(game, index, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        InputExtension.check("Joysticks.name",
                NativeInputExtensionRoutes.joysticksCopyNameAt(game, index, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private static String text(long game, int joystickId, boolean guid) {
        long[] bytes = new long[1];
        InputExtension.check("Joysticks.capabilitiesTextSize", guid
                ? NativeInputExtensionRoutes
                        .joysticksGetCapabilitiesGuidSize(game, joystickId, bytes)
                : NativeInputExtensionRoutes
                        .joysticksGetCapabilitiesNameSize(game, joystickId, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        InputExtension.check("Joysticks.capabilitiesText", guid
                ? NativeInputExtensionRoutes
                        .joysticksCopyCapabilitiesGuid(game, joystickId, destination, bytes)
                : NativeInputExtensionRoutes
                        .joysticksCopyCapabilitiesName(game, joystickId, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private static void subscribe() {
        NativeBindings.requireAvailable();
        GamerEventPump.ensureJoysticksSubscribed();
    }

    private static void dispatch(long kind, long session, long first, long second, long flag) {
        List<IntConsumer> listeners = switch ((int) kind) {
            case KIND_CONNECTED -> CONNECTED;
            case KIND_DISCONNECTED -> DISCONNECTED;
            default -> List.of();
        };
        for (IntConsumer listener : listeners) {
            listener.accept((int) first);
        }
    }
}
