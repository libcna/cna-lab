package Microsoft.Xna.Framework.Graphics.PackedVector;

/** Binary32-grouped packing helpers matching XNA's internal PackUtils. */
final class PackUtils {

    private PackUtils() {
    }

    static long packUnsigned(float bitmask, float value) {
        return ((long)clampAndRound(value, 0.0f, bitmask)) & 0xFFFF_FFFFL;
    }

    static long packSigned(long bitmask, float value) {
        float maximum = (float)(bitmask >>> 1);
        float minimum = -maximum - 1.0f;
        return ((long)(int)clampAndRound(value, minimum, maximum)) & bitmask;
    }

    static long packUNorm(float bitmask, float value) {
        value *= bitmask;
        return ((long)clampAndRound(value, 0.0f, bitmask)) & 0xFFFF_FFFFL;
    }

    static float unpackUNorm(long bitmask, long value) {
        return (float)(value & bitmask) / (float)bitmask;
    }

    static long packSNorm(long bitmask, float value) {
        float maximum = (float)(bitmask >>> 1);
        value *= maximum;
        return ((long)(int)clampAndRound(value, -maximum, maximum)) & bitmask;
    }

    static float unpackSNorm(long bitmask, long value) {
        long sign = (bitmask + 1L) >>> 1;
        long masked = value & bitmask;
        if ((masked & sign) != 0L) {
            if (masked == sign) {
                return -1.0f;
            }
            masked |= ~bitmask;
        }
        float maximum = (float)(bitmask >>> 1);
        return (float)(int)masked / maximum;
    }

    private static double clampAndRound(float value, float minimum, float maximum) {
        if (Float.isNaN(value)) {
            return 0.0;
        }
        if (Float.isInfinite(value)) {
            return value < 0.0f ? minimum : maximum;
        }
        if (value < minimum) {
            return minimum;
        }
        if (value > maximum) {
            return maximum;
        }
        return Math.rint(value);
    }
}
