package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Four packed unsigned 8-bit integer components. */
public final class Byte4 implements IPackedVectorOfT<Long> {

    private long packedValue;

    public Byte4() {
    }

    public Byte4(Byte4 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public Byte4(float x, float y, float z, float w) {
        packedValue = pack(x, y, z, w);
    }

    public Byte4(Vector4 vector) {
        Vector4 value = Objects.requireNonNull(vector, "vector");
        packedValue = pack(value.X, value.Y, value.Z, value.W);
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
        packedValue = pack(value.X, value.Y, value.Z, value.W);
    }

    @Override
    public Vector4 ToVector4() {
        return new Vector4(
                (float)(packedValue & 0xFFL),
                (float)((packedValue >>> 8) & 0xFFL),
                (float)((packedValue >>> 16) & 0xFFL),
                (float)((packedValue >>> 24) & 0xFFL));
    }

    private static long pack(float x, float y, float z, float w) {
        return (PackUtils.packUnsigned(255.0f, x)
                | (PackUtils.packUnsigned(255.0f, y) << 8)
                | (PackUtils.packUnsigned(255.0f, z) << 16)
                | (PackUtils.packUnsigned(255.0f, w) << 24)) & 0xFFFF_FFFFL;
    }

    public boolean equals(Byte4 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Byte4 other && equals(other);
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
