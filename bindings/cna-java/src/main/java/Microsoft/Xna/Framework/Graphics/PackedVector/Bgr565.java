package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Three unsigned-normalized components in B5G6R5 storage order. */
public final class Bgr565 implements IPackedVectorOfT<Integer> {

    private int packedValue;

    public Bgr565() {
    }

    public Bgr565(Bgr565 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public Bgr565(float x, float y, float z) {
        packedValue = pack(x, y, z);
    }

    public Bgr565(Vector3 vector) {
        Vector3 value = Objects.requireNonNull(vector, "vector");
        packedValue = pack(value.X, value.Y, value.Z);
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
        packedValue = pack(value.X, value.Y, value.Z);
    }

    public Vector3 ToVector3() {
        return new Vector3(
                PackUtils.unpackUNorm(31L, packedValue >>> 11),
                PackUtils.unpackUNorm(63L, packedValue >>> 5),
                PackUtils.unpackUNorm(31L, packedValue));
    }

    @Override
    public Vector4 ToVector4() {
        return new Vector4(ToVector3(), 1.0f);
    }

    private static int pack(float x, float y, float z) {
        return (int)(PackUtils.packUNorm(31.0f, z)
                | (PackUtils.packUNorm(63.0f, y) << 5)
                | (PackUtils.packUNorm(31.0f, x) << 11));
    }

    public boolean equals(Bgr565 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Bgr565 other && equals(other);
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
