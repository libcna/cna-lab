package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Four signed-normalized 16-bit components. */
public final class NormalizedShort4 implements IPackedVectorOfT<Long> {

    private long packedValue;

    public NormalizedShort4() {
    }

    public NormalizedShort4(NormalizedShort4 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public NormalizedShort4(float x, float y, float z, float w) {
        packedValue = pack(x, y, z, w);
    }

    public NormalizedShort4(Vector4 vector) {
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
                PackUtils.unpackSNorm(65_535L, packedValue),
                PackUtils.unpackSNorm(65_535L, packedValue >>> 16),
                PackUtils.unpackSNorm(65_535L, packedValue >>> 32),
                PackUtils.unpackSNorm(65_535L, packedValue >>> 48));
    }

    private static long pack(float x, float y, float z, float w) {
        return PackUtils.packSNorm(65_535L, x)
                | (PackUtils.packSNorm(65_535L, y) << 16)
                | (PackUtils.packSNorm(65_535L, z) << 32)
                | (PackUtils.packSNorm(65_535L, w) << 48);
    }

    public boolean equals(NormalizedShort4 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof NormalizedShort4 other && equals(other);
    }

    @Override
    public int hashCode() {
        return Long.hashCode(packedValue);
    }

    @Override
    public String toString() {
        return String.format(Locale.ROOT, "%016X", packedValue);
    }
}
