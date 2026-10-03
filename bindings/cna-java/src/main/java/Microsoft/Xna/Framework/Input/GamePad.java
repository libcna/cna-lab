package Microsoft.Xna.Framework.Input;

import Microsoft.Xna.Framework.PlayerIndex;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Static XNA game-pad facade backed by the current CNA game. */
public final class GamePad {

    private GamePad() {
    }

    public static GamePadCapabilities GetCapabilities(PlayerIndex playerIndex) {
        int[] values = NativeBindings.getGamePadCapabilities(index(playerIndex));
        boolean[] features = new boolean[24];
        for (int index = 0; index < features.length; index++) {
            features[index] = values[index + 2] != 0;
        }
        return GamePadCapabilities.fromNative(values[0] != 0, values[1], features);
    }

    public static GamePadState GetState(PlayerIndex playerIndex) {
        return getState(playerIndex, -1);
    }

    public static GamePadState GetState(
            PlayerIndex playerIndex, GamePadDeadZone deadZoneMode) {
        return getState(playerIndex,
                Objects.requireNonNull(deadZoneMode, "deadZoneMode").ordinal());
    }

    public static boolean SetVibration(
            PlayerIndex playerIndex, float leftMotor, float rightMotor) {
        return NativeBindings.setGamePadVibration(
                index(playerIndex), leftMotor, rightMotor);
    }

    private static GamePadState getState(PlayerIndex playerIndex, int deadZone) {
        int[] discrete = new int[3];
        float[] analog = new float[6];
        NativeBindings.getGamePadState(index(playerIndex), deadZone, discrete, analog);
        return GamePadState.fromNative(
                discrete[0] != 0,
                discrete[1],
                discrete[2],
                analog[0], analog[1], analog[2], analog[3], analog[4], analog[5]);
    }

    private static int index(PlayerIndex playerIndex) {
        return Objects.requireNonNull(playerIndex, "playerIndex").ordinal();
    }
}
