package Microsoft.Xna.Framework;

/** XNA 4.0 single-precision interpolation and angle helpers. */
public final class MathHelper {

    public static final float E = 2.71828175f;
    public static final float Log10E = 0.4342945f;
    public static final float Log2E = 1.442695f;
    public static final float Pi = 3.14159274f;
    public static final float PiOver2 = 1.57079637f;
    public static final float PiOver4 = 0.7853982f;
    public static final float TwoPi = 6.28318548f;

    private MathHelper() {
    }

    public static float Barycentric(float value1, float value2, float value3, float amount1, float amount2) {
        return value1 + (amount1 * (value2 - value1)) + (amount2 * (value3 - value1));
    }

    public static float CatmullRom(float value1, float value2, float value3, float value4, float amount) {
        float squared = amount * amount;
        float cubed = amount * squared;
        return 0.5f * ((2.0f * value2) + ((0.0f - value1 + value3) * amount)
                + (((2.0f * value1) - (5.0f * value2) + (4.0f * value3) - value4) * squared)
                + (((0.0f - value1) + (3.0f * value2) - (3.0f * value3) + value4) * cubed));
    }

    public static float Clamp(float value, float min, float max) {
        value = value > max ? max : value;
        return value < min ? min : value;
    }

    public static float Distance(float value1, float value2) {
        return Math.abs(value1 - value2);
    }

    public static float Hermite(float value1, float tangent1, float value2, float tangent2, float amount) {
        float squared = amount * amount;
        float cubed = squared * amount;
        float first = (2.0f * cubed) - (3.0f * squared) + 1.0f;
        float second = (-2.0f * cubed) + (3.0f * squared);
        float third = cubed - (2.0f * squared) + amount;
        float fourth = cubed - squared;
        return (value1 * first) + (value2 * second) + (tangent1 * third) + (tangent2 * fourth);
    }

    public static float Lerp(float value1, float value2, float amount) {
        return value1 + ((value2 - value1) * amount);
    }

    public static float Max(float value1, float value2) {
        return Math.max(value1, value2);
    }

    public static float Min(float value1, float value2) {
        return Math.min(value1, value2);
    }

    public static float SmoothStep(float value1, float value2, float amount) {
        float bounded = Clamp(amount, 0.0f, 1.0f);
        return Lerp(value1, value2, bounded * bounded * (3.0f - (2.0f * bounded)));
    }

    public static float ToDegrees(float radians) {
        return radians * 57.29578f;
    }

    public static float ToRadians(float degrees) {
        return degrees * 0.0174532924f;
    }

    public static float WrapAngle(float angle) {
        angle = (float)Math.IEEEremainder(angle, 6.2831854820251465d);
        if (angle <= -Pi) {
            angle += Pi * 2.0f;
        } else if (angle > Pi) {
            angle -= Pi * 2.0f;
        }
        return angle;
    }
}
