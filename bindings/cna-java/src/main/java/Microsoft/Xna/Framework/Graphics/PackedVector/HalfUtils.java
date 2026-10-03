package Microsoft.Xna.Framework.Graphics.PackedVector;

/** XNA's saturated 16-bit half conversion, including its exponent-31 unpack behavior. */
final class HalfUtils {

    private HalfUtils() {
    }

    static int pack(float value) {
        int bits = Float.floatToRawIntBits(value);
        int sign = (bits & 0x8000_0000) >>> 16;
        int magnitude = bits & 0x7FFF_FFFF;
        if (magnitude > 1_207_955_455) {
            return sign | 0x7FFF;
        }
        if (magnitude < 947_912_704) {
            int fraction = (magnitude & 0x7F_FFFF) | 0x80_0000;
            int shift = 113 - (magnitude >>> 23);
            magnitude = shift <= 31 ? fraction >>> shift : 0;
            return sign | (magnitude + 4095 + ((magnitude >>> 13) & 1) >>> 13);
        }
        return sign | (magnitude - 939_524_096 + 4095 + ((magnitude >>> 13) & 1) >>> 13);
    }

    static float unpack(int value) {
        int half = value & 0xFFFF;
        int bits;
        if ((half & 0x7C00) == 0) {
            if ((half & 0x03FF) != 0) {
                int exponent = -14;
                int fraction = half & 0x03FF;
                while ((fraction & 0x0400) == 0) {
                    exponent--;
                    fraction <<= 1;
                }
                fraction &= ~0x0400;
                bits = ((half & 0x8000) << 16)
                        | ((exponent + 127) << 23)
                        | (fraction << 13);
            } else {
                bits = (half & 0x8000) << 16;
            }
        } else {
            bits = ((half & 0x8000) << 16)
                    | ((((half >>> 10) & 0x1F) - 15 + 127) << 23)
                    | ((half & 0x03FF) << 13);
        }
        return Float.intBitsToFloat(bits);
    }
}
