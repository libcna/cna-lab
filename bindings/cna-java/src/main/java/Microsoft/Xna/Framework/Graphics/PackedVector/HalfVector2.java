package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector4;

import java.util.Objects;

/** Two XNA saturated binary16 values packed into an unsigned 32-bit payload. */
public final class HalfVector2 implements IPackedVectorOfT<Long> {

    private long packedValue;

    public HalfVector2() {
    }

    public HalfVector2(HalfVector2 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public HalfVector2(float x, float y) {
        packedValue = pack(x, y);
    }

    public HalfVector2(Vector2 vector) {
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
        return new Vector2(
                HalfUtils.unpack((int)packedValue),
                HalfUtils.unpack((int)(packedValue >>> 16)));
    }

    @Override
    public Vector4 ToVector4() {
        return new Vector4(ToVector2(), 0.0f, 1.0f);
    }

    private static long pack(float x, float y) {
        return ((long)HalfUtils.pack(x) & 0xFFFFL)
                | (((long)HalfUtils.pack(y) & 0xFFFFL) << 16);
    }

    public boolean equals(HalfVector2 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof HalfVector2 other && equals(other);
    }

    @Override
    public int hashCode() {
        return (int)packedValue;
    }

    @Override
    public String toString() {
        return ToVector2().toString();
    }
}
