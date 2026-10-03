package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

import java.util.Locale;
import java.util.Objects;

/** Single unsigned-normalized 8-bit alpha value. */
public final class Alpha8 implements IPackedVectorOfT<Integer> {

    private int packedValue;

    public Alpha8() {
    }

    public Alpha8(Alpha8 value) {
        packedValue = Objects.requireNonNull(value, "value").packedValue;
    }

    public Alpha8(float alpha) {
        packedValue = pack(alpha);
    }

    @Override
    public Integer getPackedValue() {
        return packedValue;
    }

    @Override
    public void setPackedValue(Integer value) {
        int packed = Objects.requireNonNull(value, "value");
        if ((packed & ~0xFF) != 0) {
            throw new IllegalArgumentException("value must be between 0 and 255");
        }
        packedValue = packed;
    }

    @Override
    public void PackFromVector4(Vector4 vector) {
        packedValue = pack(Objects.requireNonNull(vector, "vector").W);
    }

    public float ToAlpha() {
        return PackUtils.unpackUNorm(255L, packedValue);
    }

    @Override
    public Vector4 ToVector4() {
        return new Vector4(0.0f, 0.0f, 0.0f, ToAlpha());
    }

    private static int pack(float alpha) {
        return (int)PackUtils.packUNorm(255.0f, alpha);
    }

    public boolean equals(Alpha8 other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Alpha8 other && equals(other);
    }

    @Override
    public int hashCode() {
        return packedValue;
    }

    @Override
    public String toString() {
        return String.format(Locale.ROOT, "%02X", packedValue);
    }
}
