package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Objects;

/** XNA 16-bit half value, using XNA's saturation and unpack rules. */
public final class HalfSingle implements IPackedVectorOfT<Integer> {

    private int packedValue;

    public HalfSingle() {
    }

    public HalfSingle(HalfSingle value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public HalfSingle(float value) {
        packedValue = HalfUtils.pack(value);
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
        packedValue = HalfUtils.pack(Objects.requireNonNull(vector, "vector").X);
    }

    public float ToSingle() {
        return HalfUtils.unpack(packedValue);
    }

    @Override
    public Vector4 ToVector4() {
        return new Vector4(ToSingle(), 0.0f, 0.0f, 1.0f);
    }

    public boolean equals(HalfSingle other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof HalfSingle other && equals(other);
    }

    @Override
    public int hashCode() {
        return packedValue;
    }

    @Override
    public String toString() {
        String text = Float.toString(ToSingle());
        return text.endsWith(".0") ? text.substring(0, text.length() - 2) : text;
    }
}
