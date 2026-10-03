package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Two packed signed 16-bit integer components. */
public final class Short2 implements IPackedVectorOfT<Long> {

    private long packedValue;

    public Short2() {
    }

    public Short2(Short2 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public Short2(float x, float y) {
        packedValue = pack(x, y);
    }

    public Short2(Vector2 vector) {
        Vector2 value = Objects.requireNonNull(vector, "vector");
        packedValue = pack(value.X, value.Y);
    }

    @Override
    public Long getPackedValue() {
        return packedValue;
    }

    @Override
    public void setPackedValue(Long value) {
        long packed = Objects.requireNonNull(value, "value");
        if ((packed & ~0xFFFF_FFFFL) != 0L) {
            throw new IllegalArgumentException("value must be between 0 and 4294967295");
        }
        packedValue = packed;
    }

    @Override
    public void PackFromVector4(Vector4 vector) {
        Vector4 value = Objects.requireNonNull(vector, "vector");
        packedValue = pack(value.X, value.Y);
    }

    public Vector2 ToVector2() {
        return new Vector2((short)packedValue, (short)(packedValue >>> 16));
    }

    @Override
    public Vector4 ToVector4() {
        Vector2 value = ToVector2();
        return new Vector4(value.X, value.Y, 0.0f, 1.0f);
    }

    private static long pack(float x, float y) {
        return (PackUtils.packSigned(65_535L, x)
                | (PackUtils.packSigned(65_535L, y) << 16)) & 0xFFFF_FFFFL;
    }

    public boolean equals(Short2 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Short2 other && equals(other);
    }

    @Override
    public int hashCode() {
        return (int)packedValue;
    }

    @Override
    public String toString() {
        return String.format(Locale.ROOT, "%08X", packedValue);
    }
}
