package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Objects;

/** Four XNA saturated binary16 values packed into an unsigned 64-bit payload. */
public final class HalfVector4 implements IPackedVectorOfT<Long> {

    private long packedValue;

    public HalfVector4() {
    }

    public HalfVector4(HalfVector4 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public HalfVector4(float x, float y, float z, float w) {
        packedValue = pack(x, y, z, w);
    }

    public HalfVector4(Vector4 vector) {
        Vector4 value = Objects.requireNonNull(vector, "vector");
        packedValue = pack(value.X, value.Y, value.Z, value.W);
    }

    @Override
    public Long getPackedValue() {
        return packedValue;
    }

    @Override
    public void setPackedValue(Long value) {
        packedValue = Objects.requireNonNull(value, "value");
    }

    @Override
    public void PackFromVector4(Vector4 vector) {
        Vector4 value = Objects.requireNonNull(vector, "vector");
        packedValue = pack(value.X, value.Y, value.Z, value.W);
    }

    @Override
    public Vector4 ToVector4() {
        return new Vector4(
                HalfUtils.unpack((int)packedValue),
                HalfUtils.unpack((int)(packedValue >>> 16)),
                HalfUtils.unpack((int)(packedValue >>> 32)),
                HalfUtils.unpack((int)(packedValue >>> 48)));
    }

    private static long pack(float x, float y, float z, float w) {
        return ((long)HalfUtils.pack(x) & 0xFFFFL)
                | (((long)HalfUtils.pack(y) & 0xFFFFL) << 16)
                | (((long)HalfUtils.pack(z) & 0xFFFFL) << 32)
                | (((long)HalfUtils.pack(w) & 0xFFFFL) << 48);
    }

    public boolean equals(HalfVector4 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof HalfVector4 other && equals(other);
    }

    @Override
    public int hashCode() {
        return Long.hashCode(packedValue);
    }

    @Override
    public String toString() {
        return ToVector4().toString();
    }
}
