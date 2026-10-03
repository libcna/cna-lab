package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Four unsigned-normalized components in B4G4R4A4 storage order. */
public final class Bgra4444 implements IPackedVectorOfT<Integer> {

    private int packedValue;

    public Bgra4444() {
    }

    public Bgra4444(Bgra4444 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public Bgra4444(float x, float y, float z, float w) {
        packedValue = pack(x, y, z, w);
    }

    public Bgra4444(Vector4 vector) {
        Vector4 value = Objects.requireNonNull(vector, "vector");
        packedValue = pack(value.X, value.Y, value.Z, value.W);
    }

    @Override
    public Integer getPackedValue() {
        return packedValue;
    }

    @Override
    public void setPackedValue(Integer value) {
        int packed = Objects.requireNonNull(value, "value");
        if ((packed & ~0xFFFF) != 0) {
            throw new IllegalArgumentException("value must be between 0 and 65535");
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
                PackUtils.unpackUNorm(15L, packedValue >>> 8),
                PackUtils.unpackUNorm(15L, packedValue >>> 4),
                PackUtils.unpackUNorm(15L, packedValue),
                PackUtils.unpackUNorm(15L, packedValue >>> 12));
    }

    private static int pack(float x, float y, float z, float w) {
        return (int)(PackUtils.packUNorm(15.0f, z)
                | (PackUtils.packUNorm(15.0f, y) << 4)
                | (PackUtils.packUNorm(15.0f, x) << 8)
                | (PackUtils.packUNorm(15.0f, w) << 12));
    }

    public boolean equals(Bgra4444 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Bgra4444 other && equals(other);
    }

    @Override
    public int hashCode() {
        return packedValue;
    }

    @Override
    public String toString() {
        return String.format(Locale.ROOT, "%04X", packedValue);
    }
}
