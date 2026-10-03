package Microsoft.Xna.Framework;

/** CLR {@code Single.Equals}/{@code GetHashCode} behavior shared by Java value facades. */
final class FloatSemantics {

    private FloatSemantics() {
    }

    static boolean equals(float left, float right) {
        return left == right || Float.isNaN(left) && Float.isNaN(right);
    }

    static int hash(float value) {
        if (value == 0.0f) {
            return 0;
        }
        return Float.floatToIntBits(value);
    }
}
