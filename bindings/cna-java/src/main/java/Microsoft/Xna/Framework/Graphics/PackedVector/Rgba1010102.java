package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Four unsigned-normalized components packed as R10G10B10A2. */
public final class Rgba1010102 implements IPackedVectorOfT<Long> {

    private long packedValue;

    public Rgba1010102() {
    }

    public Rgba1010102(Rgba1010102 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public Rgba1010102(float x, float y, float z, float w) {
        packedValue = pack(x, y, z, w);
    }

    public Rgba1010102(Vector4 vector) {
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
                PackUtils.unpackUNorm(1_023L, packedValue),
                PackUtils.unpackUNorm(1_023L, packedValue >>> 10),
                PackUtils.unpackUNorm(1_023L, packedValue >>> 20),
                PackUtils.unpackUNorm(3L, packedValue >>> 30));
    }

    private static long pack(float x, float y, float z, float w) {
        return (PackUtils.packUNorm(1_023.0f, x)
                | (PackUtils.packUNorm(1_023.0f, y) << 10)
                | (PackUtils.packUNorm(1_023.0f, z) << 20)
                | (PackUtils.packUNorm(3.0f, w) << 30)) & 0xFFFF_FFFFL;
    }

    public boolean equals(Rgba1010102 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Rgba1010102 other && equals(other);
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
