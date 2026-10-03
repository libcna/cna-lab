package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.PlayerIndex;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Input.Buttons;
import org.openeggbert.cna.extensions.devices.PowerState;
import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * What a modern game pad has and XNA's {@code GamePad} cannot describe.
 *
 * <p>A CNA extension, and deliberately a separate type rather than extra members on
 * {@code Microsoft.Xna.Framework.Input.GamePad}: XNA's pad is an Xbox 360 controller, and it has
 * no place for a light bar, a touchpad, a battery reading or the label printed on a face button.
 * The XNA surface stays exactly what Microsoft shipped, and this stands beside it, keyed by the
 * same {@link PlayerIndex}.
 *
 * <p>Every reading here belongs to a pad that is actually connected. On a host with no pad the
 * strings come back empty and the readings report absent, which is an ordinary answer.
 */
public final class GamePadExtensions {

    /** CNA reports an unknown battery percentage as -1, as the rest of the device layer does. */
    private static final int UNKNOWN_PERCENT = -1;

    private GamePadExtensions() {
    }

    /** Returns the host's display name for the pad, empty when there is none. */
    public static String getName(PlayerIndex playerIndex) {
        return text(playerIndex, Text.Name);
    }

    /** Returns the pad's device GUID, empty when the host has none. */
    public static String getGuid(PlayerIndex playerIndex) {
        return text(playerIndex, Text.Guid);
    }

    /** Returns the host's device path for the pad, empty when the host has none. */
    public static String getPath(PlayerIndex playerIndex) {
        return text(playerIndex, Text.Path);
    }

    /** Returns the pad's serial number, empty when the host will not report one. */
    public static String getSerial(PlayerIndex playerIndex) {
        return text(playerIndex, Text.Serial);
    }

    /** Returns the pad's firmware version, as the host reports it. */
    public static int getFirmwareVersion(PlayerIndex playerIndex) {
        int[] version = new int[1];
        check("getFirmwareVersion", NativeInputExtensionRoutes
                .gamepadGetFirmwareVersionExt(game(), index(playerIndex), version));
        return version[0];
    }

    /**
     * Returns Steam's own handle for the pad, or zero when Steam is not driving it.
     *
     * <p>Zero is Steam's own "no handle", not a missing answer, which is why it is a
     * {@code long} rather than {@code null}.
     */
    public static long getSteamHandle(PlayerIndex playerIndex) {
        long[] handle = new long[1];
        check("getSteamHandle", NativeInputExtensionRoutes
                .gamepadGetSteamHandleExt(game(), index(playerIndex), handle));
        return handle[0];
    }

    /** Returns how the pad is attached. */
    public static GamePadConnectionState getConnectionState(PlayerIndex playerIndex) {
        int[] state = new int[1];
        check("getConnectionState", NativeInputExtensionRoutes
                .gamepadGetConnectionStateExt(game(), index(playerIndex), state));
        return GamePadConnectionState.values()[state[0]];
    }

    /** Returns the player number the pad shows on itself, or -1 when it shows none. */
    public static int getPlayerNumber(PlayerIndex playerIndex) {
        int[] number = new int[1];
        check("getPlayerNumber", NativeInputExtensionRoutes
                .gamepadGetPlayerIndexExt(game(), index(playerIndex), number));
        return number[0];
    }

    /**
     * Sets the player number the pad shows on itself.
     *
     * @param playerIndex which pad to set
     * @param playerNumber the number to display
     * @return whether the pad applied it; one with no such indicator reports false
     */
    public static boolean setPlayerNumber(PlayerIndex playerIndex, int playerNumber) {
        boolean[] applied = new boolean[1];
        check("setPlayerNumber", NativeInputExtensionRoutes.gamepadSetPlayerIndexExt(
                game(), index(playerIndex), playerNumber, applied));
        return applied[0];
    }

    /** Returns how the pad is powered. */
    public static PowerState getPowerState(PlayerIndex playerIndex) {
        int[] state = new int[1];
        int[] percent = new int[1];
        check("getPowerState", NativeInputExtensionRoutes
                .gamepadGetPowerInfoExt(game(), index(playerIndex), state, percent));
        return PowerState.values()[state[0]];
    }

    /**
     * Returns the pad's remaining charge as a percentage.
     *
     * @param playerIndex which pad to ask
     * @return the percentage, or {@code null} when the pad will not say; absent is not zero,
     *     because zero means empty
     */
    public static Integer getBatteryPercent(PlayerIndex playerIndex) {
        int[] state = new int[1];
        int[] percent = new int[1];
        check("getBatteryPercent", NativeInputExtensionRoutes
                .gamepadGetPowerInfoExt(game(), index(playerIndex), state, percent));
        return percent[0] == UNKNOWN_PERCENT ? null : percent[0];
    }

    /**
     * Returns what is physically printed on one button of this pad.
     *
     * <p>The XNA {@link Buttons} value is the identity of the button; the label is what to draw
     * in a prompt, so a PlayStation player is told to press cross rather than A.
     *
     * @param playerIndex which pad to ask
     * @param button the XNA button whose label is wanted
     * @return the printed label, or {@link GamePadButtonLabel#Unknown} when the host cannot say
     */
    public static GamePadButtonLabel getButtonLabel(PlayerIndex playerIndex, Buttons button) {
        Objects.requireNonNull(button, "button");
        int[] label = new int[1];
        check("getButtonLabel", NativeInputExtensionRoutes.gamepadGetButtonLabelExt(
                game(), index(playerIndex), button.getValue(), label));
        return GamePadButtonLabel.values()[label[0]];
    }

    /**
     * Returns the pad's own gyroscope reading, in radians per second.
     *
     * @param playerIndex which pad to ask
     * @return the angular velocity, or {@code null} when the pad has no gyroscope
     */
    public static Vector3 getAngularVelocity(PlayerIndex playerIndex) {
        float[] values = new float[3];
        boolean[] available = new boolean[1];
        check("getAngularVelocity", NativeInputExtensionRoutes
                .gamepadGetGyroExt(game(), index(playerIndex), values, available));
        return available[0] ? new Vector3(values[0], values[1], values[2]) : null;
    }

    /**
     * Returns the pad's own accelerometer reading, in metres per second squared.
     *
     * @param playerIndex which pad to ask
     * @return the acceleration, or {@code null} when the pad has no accelerometer
     */
    public static Vector3 getAcceleration(PlayerIndex playerIndex) {
        float[] values = new float[3];
        boolean[] available = new boolean[1];
        check("getAcceleration", NativeInputExtensionRoutes
                .gamepadGetAccelerometerExt(game(), index(playerIndex), values, available));
        return available[0] ? new Vector3(values[0], values[1], values[2]) : null;
    }

    /**
     * Sets the colour of the pad's light bar.
     *
     * @param playerIndex which pad to set
     * @param color the colour to show; a pad with no light bar ignores it
     */
    public static void setLightBar(PlayerIndex playerIndex, Color color) {
        Objects.requireNonNull(color, "color");
        check("setLightBar", NativeInputExtensionRoutes.gamepadSetLightBarExt(
                game(), index(playerIndex),
                new long[] {color.getR(), color.getG(), color.getB(), color.getA()}));
    }

    /**
     * Drives the motors inside the triggers, which XNA's {@code SetVibration} cannot reach.
     *
     * <p>{@code GamePad.SetVibration} drives the two handle motors and nothing else, because
     * that is all an Xbox 360 pad had. These are separate motors on a later pad, and using them
     * does not disturb the handle motors.
     *
     * @param playerIndex which pad to shake
     * @param leftTrigger the left trigger's motor, from zero through one
     * @param rightTrigger the right trigger's motor, from zero through one
     * @return whether the pad applied it; one without trigger motors reports false
     */
    public static boolean SetTriggerVibration(
            PlayerIndex playerIndex, float leftTrigger, float rightTrigger) {
        boolean[] applied = new boolean[1];
        check("SetTriggerVibration", NativeInputExtensionRoutes.gamepadSetTriggerVibrationExt(
                game(), index(playerIndex), leftTrigger, rightTrigger, applied));
        return applied[0];
    }

    /** Returns how many touchpads the pad has. */
    public static int getTouchpadCount(PlayerIndex playerIndex) {
        int[] count = new int[1];
        check("getTouchpadCount", NativeInputExtensionRoutes
                .gamepadGetTouchpadCountExt(game(), index(playerIndex), count));
        return count[0];
    }

    /**
     * Returns every finger one touchpad can report, whether or not it is touching.
     *
     * <p>A touchpad reports a fixed number of finger slots; a slot nobody is touching has
     * {@link GamePadTouchpadFinger#IsDown()} false. A slot the host cannot read at all is left
     * out of the list rather than reported as a finger at the origin.
     *
     * @param playerIndex which pad to ask
     * @param touchpad the zero-based touchpad index, below {@link #getTouchpadCount}
     * @return one entry per readable finger slot
     */
    public static List<GamePadTouchpadFinger> getTouchpadFingers(
            PlayerIndex playerIndex, int touchpad) {
        long game = game();
        int player = index(playerIndex);
        int[] count = new int[1];
        check("getTouchpadFingerCount", NativeInputExtensionRoutes
                .gamepadGetTouchpadFingerCountExt(game, player, touchpad, count));
        List<GamePadTouchpadFinger> fingers = new ArrayList<>(count[0]);
        for (int finger = 0; finger < count[0]; finger++) {
            long[] integral = new long[1];
            float[] floating = new float[3];
            boolean[] available = new boolean[1];
            check("getTouchpadFinger", NativeInputExtensionRoutes.gamepadGetTouchpadFingerExt(
                    game, player, touchpad, finger, new byte[3], integral, floating, available));
            if (available[0]) {
                fingers.add(new GamePadTouchpadFinger(
                        integral[0] != 0L, floating[0], floating[1], floating[2]));
            }
        }
        return List.copyOf(fingers);
    }

    private enum Text {
        Name, Guid, Path, Serial
    }

    private static String text(PlayerIndex playerIndex, Text which) {
        long game = game();
        int player = index(playerIndex);
        long[] bytes = new long[1];
        check("textSize", switch (which) {
            case Name -> NativeInputExtensionRoutes.gamepadGetNameSizeExt(game, player, bytes);
            case Guid -> NativeInputExtensionRoutes.gamepadGetGuidSizeExt(game, player, bytes);
            case Path -> NativeInputExtensionRoutes.gamepadGetPathSizeExt(game, player, bytes);
            case Serial ->
                    NativeInputExtensionRoutes.gamepadGetSerialSizeExt(game, player, bytes);
        });
        byte[] destination = new byte[(int) bytes[0]];
        check("text", switch (which) {
            case Name ->
                    NativeInputExtensionRoutes.gamepadCopyNameExt(game, player, destination, bytes);
            case Guid ->
                    NativeInputExtensionRoutes.gamepadCopyGuidExt(game, player, destination, bytes);
            case Path ->
                    NativeInputExtensionRoutes.gamepadCopyPathExt(game, player, destination, bytes);
            case Serial -> NativeInputExtensionRoutes
                    .gamepadCopySerialExt(game, player, destination, bytes);
        });
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private static int index(PlayerIndex playerIndex) {
        return Objects.requireNonNull(playerIndex, "playerIndex").ordinal();
    }

    private static long game() {
        return InputExtension.game("GamePadExtensions");
    }

    private static void check(String operation, int result) {
        InputExtension.check("GamePadExtensions." + operation, result);
    }
}
