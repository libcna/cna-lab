package Microsoft.Xna.Framework.Input;

import Microsoft.Xna.Framework.MathHelper;
import Microsoft.Xna.Framework.Vector2;

import java.util.Objects;

/** Binary32 port of XNA's controller dead-zone transformations. */
final class GamePadDeadZoneUtils {

    private static final int LEFT_STICK_DEAD_ZONE_SIZE = 7849;
    private static final int RIGHT_STICK_DEAD_ZONE_SIZE = 8689;
    private static final int TRIGGER_DEAD_ZONE_SIZE = 30;

    private GamePadDeadZoneUtils() {
    }

    static Vector2 applyLeftStickDeadZone(int x, int y, GamePadDeadZone mode) {
        return applyStickDeadZone(x, y, mode, LEFT_STICK_DEAD_ZONE_SIZE);
    }

    static Vector2 applyRightStickDeadZone(int x, int y, GamePadDeadZone mode) {
        return applyStickDeadZone(x, y, mode, RIGHT_STICK_DEAD_ZONE_SIZE);
    }

    static float applyTriggerDeadZone(int value, GamePadDeadZone mode) {
        Objects.requireNonNull(mode, "mode");
        return applyLinearDeadZone(
                value, 255.0f, mode == GamePadDeadZone.None ? 0.0f : TRIGGER_DEAD_ZONE_SIZE);
    }

    private static Vector2 applyStickDeadZone(
            int x, int y, GamePadDeadZone mode, int deadZoneSize) {
        Objects.requireNonNull(mode, "mode");
        if (mode == GamePadDeadZone.IndependentAxes) {
            return new Vector2(
                    applyLinearDeadZone(x, 32767.0f, deadZoneSize),
                    applyLinearDeadZone(y, 32767.0f, deadZoneSize));
        }
        if (mode == GamePadDeadZone.Circular) {
            // XNA performs the square-and-add as Int32 before converting to Single.
            float length = (float) Math.sqrt((x * x) + (y * y));
            float adjustedLength = applyLinearDeadZone(length, 32767.0f, deadZoneSize);
            float scale = adjustedLength > 0.0f ? adjustedLength / length : 0.0f;
            return new Vector2(
                    MathHelper.Clamp(x * scale, -1.0f, 1.0f),
                    MathHelper.Clamp(y * scale, -1.0f, 1.0f));
        }
        return new Vector2(
                applyLinearDeadZone(x, 32767.0f, 0.0f),
                applyLinearDeadZone(y, 32767.0f, 0.0f));
    }

    private static float applyLinearDeadZone(float value, float maxValue, float deadZoneSize) {
        if (value < -deadZoneSize) {
            value += deadZoneSize;
        } else {
            if (!(value > deadZoneSize)) {
                return 0.0f;
            }
            value -= deadZoneSize;
        }
        return MathHelper.Clamp(value / (maxValue - deadZoneSize), -1.0f, 1.0f);
    }
}
