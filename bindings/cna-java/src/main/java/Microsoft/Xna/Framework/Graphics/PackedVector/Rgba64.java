package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Four unsigned-normalized 16-bit components packed as R16G16B16A16. */
public final class Rgba64 implements IPackedVectorOfT<Long> {

    private long packedValue;

    public Rgba64() {
    }

    public Rgba64(Rgba64 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public Rgba64(float x, float y, float z, float w) {
        packedValue = pack(x, y, z, w);
    }

    public Rgba64(Vector4 vector) {
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
                PackUtils.unpackUNorm(65_535L, packedValue),
                PackUtils.unpackUNorm(65_535L, packedValue >>> 16),
                PackUtils.unpackUNorm(65_535L, packedValue >>> 32),
                PackUtils.unpackUNorm(65_535L, packedValue >>> 48));
    }

    private static long pack(float x, float y, float z, float w) {
        return PackUtils.packUNorm(65_535.0f, x)
                | (PackUtils.packUNorm(65_535.0f, y) << 16)
                | (PackUtils.packUNorm(65_535.0f, z) << 32)
                | (PackUtils.packUNorm(65_535.0f, w) << 48);
    }

    public boolean equals(Rgba64 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Rgba64 other && equals(other);
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
