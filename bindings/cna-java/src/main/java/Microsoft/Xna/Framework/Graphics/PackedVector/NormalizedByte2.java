package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Two signed-normalized 8-bit components. */
public final class NormalizedByte2 implements IPackedVectorOfT<Integer> {

    private int packedValue;

    public NormalizedByte2() {
    }

    public NormalizedByte2(NormalizedByte2 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public NormalizedByte2(float x, float y) {
        packedValue = pack(x, y);
    }

    public NormalizedByte2(Vector2 vector) {
        Vector2 value = Objects.requireNonNull(vector, "vector");
        packedValue = pack(value.X, value.Y);
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
        packedValue = pack(value.X, value.Y);
    }

    public Vector2 ToVector2() {
        return new Vector2(
                PackUtils.unpackSNorm(255L, packedValue),
                PackUtils.unpackSNorm(255L, packedValue >>> 8));
    }

    @Override
    public Vector4 ToVector4() {
        Vector2 value = ToVector2();
        return new Vector4(value.X, value.Y, 0.0f, 1.0f);
    }

    private static int pack(float x, float y) {
        return (int)(PackUtils.packSNorm(255L, x)
                | (PackUtils.packSNorm(255L, y) << 8));
    }

    public boolean equals(NormalizedByte2 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof NormalizedByte2 other && equals(other);
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
