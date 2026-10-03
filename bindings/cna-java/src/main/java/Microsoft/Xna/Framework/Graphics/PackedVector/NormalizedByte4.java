package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Four signed-normalized 8-bit components. */
public final class NormalizedByte4 implements IPackedVectorOfT<Long> {

    private long packedValue;

    public NormalizedByte4() {
    }

    public NormalizedByte4(NormalizedByte4 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public NormalizedByte4(float x, float y, float z, float w) {
        packedValue = pack(x, y, z, w);
    }

    public NormalizedByte4(Vector4 vector) {
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
                PackUtils.unpackSNorm(255L, packedValue),
                PackUtils.unpackSNorm(255L, packedValue >>> 8),
                PackUtils.unpackSNorm(255L, packedValue >>> 16),
                PackUtils.unpackSNorm(255L, packedValue >>> 24));
    }

    private static long pack(float x, float y, float z, float w) {
        return (PackUtils.packSNorm(255L, x)
                | (PackUtils.packSNorm(255L, y) << 8)
                | (PackUtils.packSNorm(255L, z) << 16)
                | (PackUtils.packSNorm(255L, w) << 24)) & 0xFFFF_FFFFL;
    }

    public boolean equals(NormalizedByte4 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof NormalizedByte4 other && equals(other);
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
