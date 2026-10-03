package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** BGRA-ordered 5/5/5/1 unsigned-normalized value. */
public final class Bgra5551 implements IPackedVectorOfT<Integer> {

    private int packedValue;

    public Bgra5551() {
    }

    public Bgra5551(Bgra5551 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public Bgra5551(float x, float y, float z, float w) {
        packedValue = pack(x, y, z, w);
    }

    public Bgra5551(Vector4 vector) {
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
                PackUtils.unpackUNorm(31L, packedValue >>> 10),
                PackUtils.unpackUNorm(31L, packedValue >>> 5),
                PackUtils.unpackUNorm(31L, packedValue),
                PackUtils.unpackUNorm(1L, packedValue >>> 15));
    }

    private static int pack(float x, float y, float z, float w) {
        return (int)((PackUtils.packUNorm(31.0f, x) << 10)
                | (PackUtils.packUNorm(31.0f, y) << 5)
                | PackUtils.packUNorm(31.0f, z)
                | (PackUtils.packUNorm(1.0f, w) << 15));
    }

    public boolean equals(Bgra5551 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Bgra5551 other && equals(other);
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
