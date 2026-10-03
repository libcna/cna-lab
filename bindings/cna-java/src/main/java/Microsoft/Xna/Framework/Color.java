package Microsoft.Xna.Framework;

import Microsoft.Xna.Framework.Graphics.PackedVector.IPackedVectorOfT;

import java.util.Objects;

/** Mutable XNA RGBA value. Named instances are frozen to prevent shared-state corruption. */
public final class Color implements IPackedVectorOfT<Long> {

    public static final Color Transparent = named(0, 0, 0, 0);
    public static final Color AliceBlue = named(240, 248, 255);
    public static final Color AntiqueWhite = named(250, 235, 215);
    public static final Color Aqua = named(0, 255, 255);
    public static final Color Aquamarine = named(127, 255, 212);
    public static final Color Azure = named(240, 255, 255);
    public static final Color Beige = named(245, 245, 220);
    public static final Color Bisque = named(255, 228, 196);
    public static final Color Black = named(0, 0, 0);
    public static final Color BlanchedAlmond = named(255, 235, 205);
    public static final Color Blue = named(0, 0, 255);
    public static final Color BlueViolet = named(138, 43, 226);
    public static final Color Brown = named(165, 42, 42);
    public static final Color BurlyWood = named(222, 184, 135);
    public static final Color CadetBlue = named(95, 158, 160);
    public static final Color Chartreuse = named(127, 255, 0);
    public static final Color Chocolate = named(210, 105, 30);
    public static final Color Coral = named(255, 127, 80);
    public static final Color CornflowerBlue = named(100, 149, 237);
    public static final Color Cornsilk = named(255, 248, 220);
    public static final Color Crimson = named(220, 20, 60);
    public static final Color Cyan = named(0, 255, 255);
    public static final Color DarkBlue = named(0, 0, 139);
    public static final Color DarkCyan = named(0, 139, 139);
    public static final Color DarkGoldenrod = named(184, 134, 11);
    public static final Color DarkGray = named(169, 169, 169);
    public static final Color DarkGreen = named(0, 100, 0);
    public static final Color DarkKhaki = named(189, 183, 107);
    public static final Color DarkMagenta = named(139, 0, 139);
    public static final Color DarkOliveGreen = named(85, 107, 47);
    public static final Color DarkOrange = named(255, 140, 0);
    public static final Color DarkOrchid = named(153, 50, 204);
    public static final Color DarkRed = named(139, 0, 0);
    public static final Color DarkSalmon = named(233, 150, 122);
    public static final Color DarkSeaGreen = named(143, 188, 143);
    public static final Color DarkSlateBlue = named(72, 61, 139);
    public static final Color DarkSlateGray = named(47, 79, 79);
    public static final Color DarkTurquoise = named(0, 206, 209);
    public static final Color DarkViolet = named(148, 0, 211);
    public static final Color DeepPink = named(255, 20, 147);
    public static final Color DeepSkyBlue = named(0, 191, 255);
    public static final Color DimGray = named(105, 105, 105);
    public static final Color DodgerBlue = named(30, 144, 255);
    public static final Color Firebrick = named(178, 34, 34);
    public static final Color FloralWhite = named(255, 250, 240);
    public static final Color ForestGreen = named(34, 139, 34);
    public static final Color Fuchsia = named(255, 0, 255);
    public static final Color Gainsboro = named(220, 220, 220);
    public static final Color GhostWhite = named(248, 248, 255);
    public static final Color Gold = named(255, 215, 0);
    public static final Color Goldenrod = named(218, 165, 32);
    public static final Color Gray = named(128, 128, 128);
    public static final Color Green = named(0, 128, 0);
    public static final Color GreenYellow = named(173, 255, 47);
    public static final Color Honeydew = named(240, 255, 240);
    public static final Color HotPink = named(255, 105, 180);
    public static final Color IndianRed = named(205, 92, 92);
    public static final Color Indigo = named(75, 0, 130);
    public static final Color Ivory = named(255, 255, 240);
    public static final Color Khaki = named(240, 230, 140);
    public static final Color Lavender = named(230, 230, 250);
    public static final Color LavenderBlush = named(255, 240, 245);
    public static final Color LawnGreen = named(124, 252, 0);
    public static final Color LemonChiffon = named(255, 250, 205);
    public static final Color LightBlue = named(173, 216, 230);
    public static final Color LightCoral = named(240, 128, 128);
    public static final Color LightCyan = named(224, 255, 255);
    public static final Color LightGoldenrodYellow = named(250, 250, 210);
    public static final Color LightGray = named(211, 211, 211);
    public static final Color LightGreen = named(144, 238, 144);
    public static final Color LightPink = named(255, 182, 193);
    public static final Color LightSalmon = named(255, 160, 122);
    public static final Color LightSeaGreen = named(32, 178, 170);
    public static final Color LightSkyBlue = named(135, 206, 250);
    public static final Color LightSlateGray = named(119, 136, 153);
    public static final Color LightSteelBlue = named(176, 196, 222);
    public static final Color LightYellow = named(255, 255, 224);
    public static final Color Lime = named(0, 255, 0);
    public static final Color LimeGreen = named(50, 205, 50);
    public static final Color Linen = named(250, 240, 230);
    public static final Color Magenta = named(255, 0, 255);
    public static final Color Maroon = named(128, 0, 0);
    public static final Color MediumAquamarine = named(102, 205, 170);
    public static final Color MediumBlue = named(0, 0, 205);
    public static final Color MediumOrchid = named(186, 85, 211);
    public static final Color MediumPurple = named(147, 112, 219);
    public static final Color MediumSeaGreen = named(60, 179, 113);
    public static final Color MediumSlateBlue = named(123, 104, 238);
    public static final Color MediumSpringGreen = named(0, 250, 154);
    public static final Color MediumTurquoise = named(72, 209, 204);
    public static final Color MediumVioletRed = named(199, 21, 133);
    public static final Color MidnightBlue = named(25, 25, 112);
    public static final Color MintCream = named(245, 255, 250);
    public static final Color MistyRose = named(255, 228, 225);
    public static final Color Moccasin = named(255, 228, 181);
    public static final Color NavajoWhite = named(255, 222, 173);
    public static final Color Navy = named(0, 0, 128);
    public static final Color OldLace = named(253, 245, 230);
    public static final Color Olive = named(128, 128, 0);
    public static final Color OliveDrab = named(107, 142, 35);
    public static final Color Orange = named(255, 165, 0);
    public static final Color OrangeRed = named(255, 69, 0);
    public static final Color Orchid = named(218, 112, 214);
    public static final Color PaleGoldenrod = named(238, 232, 170);
    public static final Color PaleGreen = named(152, 251, 152);
    public static final Color PaleTurquoise = named(175, 238, 238);
    public static final Color PaleVioletRed = named(219, 112, 147);
    public static final Color PapayaWhip = named(255, 239, 213);
    public static final Color PeachPuff = named(255, 218, 185);
    public static final Color Peru = named(205, 133, 63);
    public static final Color Pink = named(255, 192, 203);
    public static final Color Plum = named(221, 160, 221);
    public static final Color PowderBlue = named(176, 224, 230);
    public static final Color Purple = named(128, 0, 128);
    public static final Color Red = named(255, 0, 0);
    public static final Color RosyBrown = named(188, 143, 143);
    public static final Color RoyalBlue = named(65, 105, 225);
    public static final Color SaddleBrown = named(139, 69, 19);
    public static final Color Salmon = named(250, 128, 114);
    public static final Color SandyBrown = named(244, 164, 96);
    public static final Color SeaGreen = named(46, 139, 87);
    public static final Color SeaShell = named(255, 245, 238);
    public static final Color Sienna = named(160, 82, 45);
    public static final Color Silver = named(192, 192, 192);
    public static final Color SkyBlue = named(135, 206, 235);
    public static final Color SlateBlue = named(106, 90, 205);
    public static final Color SlateGray = named(112, 128, 144);
    public static final Color Snow = named(255, 250, 250);
    public static final Color SpringGreen = named(0, 255, 127);
    public static final Color SteelBlue = named(70, 130, 180);
    public static final Color Tan = named(210, 180, 140);
    public static final Color Teal = named(0, 128, 128);
    public static final Color Thistle = named(216, 191, 216);
    public static final Color Tomato = named(255, 99, 71);
    public static final Color Turquoise = named(64, 224, 208);
    public static final Color Violet = named(238, 130, 238);
    public static final Color Wheat = named(245, 222, 179);
    public static final Color White = named(255, 255, 255);
    public static final Color WhiteSmoke = named(245, 245, 245);
    public static final Color Yellow = named(255, 255, 0);
    public static final Color YellowGreen = named(154, 205, 50);

    private int packedValue;
    private final boolean frozen;

    public Color() {
        this(0, false);
    }

    public Color(int r, int g, int b) {
        this(r, g, b, 255);
    }

    public Color(int r, int g, int b, int a) {
        this(pack(clampByte(r), clampByte(g), clampByte(b), clampByte(a)), false);
    }

    public Color(float r, float g, float b) {
        this(r, g, b, 1.0f);
    }

    public Color(float r, float g, float b, float a) {
        this(pack(packUnit(r), packUnit(g), packUnit(b), packUnit(a)), false);
    }

    public Color(Vector3 vector) {
        this(Objects.requireNonNull(vector, "vector").X, vector.Y, vector.Z, 1.0f);
    }

    public Color(Vector4 vector) {
        this(Objects.requireNonNull(vector, "vector").X, vector.Y, vector.Z, vector.W);
    }

    /** Copies a color snapshot; copies of named colors are ordinary mutable values. */
    public Color(Color value) {
        this(Objects.requireNonNull(value, "value").packedValue, false);
    }

    private Color(int packedValue, boolean frozen) {
        this.packedValue = packedValue;
        this.frozen = frozen;
    }

    public int getR() {
        return packedValue & 0xff;
    }

    public void setR(int value) {
        ensureMutable();
        packedValue = (packedValue & 0xffffff00) | requireByte(value, "value");
    }

    public int getG() {
        return (packedValue >>> 8) & 0xff;
    }

    public void setG(int value) {
        ensureMutable();
        packedValue = (packedValue & 0xffff00ff) | (requireByte(value, "value") << 8);
    }

    public int getB() {
        return (packedValue >>> 16) & 0xff;
    }

    public void setB(int value) {
        ensureMutable();
        packedValue = (packedValue & 0xff00ffff) | (requireByte(value, "value") << 16);
    }

    public int getA() {
        return (packedValue >>> 24) & 0xff;
    }

    public void setA(int value) {
        ensureMutable();
        packedValue = (packedValue & 0x00ffffff) | (requireByte(value, "value") << 24);
    }

    /** Unsigned XNA packed value represented in Java's positive {@code long} range. */
    @Override
    public Long getPackedValue() {
        return Integer.toUnsignedLong(packedValue);
    }

    @Override
    public void setPackedValue(Long value) {
        ensureMutable();
        Objects.requireNonNull(value, "value");
        if (value < 0L || value > 0xffff_ffffL) {
            throw new IllegalArgumentException("value must fit an unsigned 32-bit integer");
        }
        packedValue = value.intValue();
    }

    @Override
    public void PackFromVector4(Vector4 vector) {
        ensureMutable();
        Objects.requireNonNull(vector, "vector");
        packedValue = pack(
                packUnit(vector.X), packUnit(vector.Y), packUnit(vector.Z), packUnit(vector.W));
    }

    public Vector3 ToVector3() {
        return new Vector3(getR() / 255.0f, getG() / 255.0f, getB() / 255.0f);
    }

    public Vector4 ToVector4() {
        return new Vector4(getR() / 255.0f, getG() / 255.0f, getB() / 255.0f, getA() / 255.0f);
    }

    public static Color FromNonPremultiplied(int r, int g, int b, int a) {
        int alpha = clampByte(a);
        return new Color(
                clampByte((int)((long)r * alpha / 255L)),
                clampByte((int)((long)g * alpha / 255L)),
                clampByte((int)((long)b * alpha / 255L)),
                alpha);
    }

    public static Color FromNonPremultiplied(Vector4 vector) {
        Objects.requireNonNull(vector, "vector");
        return new Color(vector.X * vector.W, vector.Y * vector.W, vector.Z * vector.W, vector.W);
    }

    public static Color Lerp(Color value1, Color value2, float amount) {
        Objects.requireNonNull(value1, "value1");
        Objects.requireNonNull(value2, "value2");
        int fraction = packFixed16(amount);
        return new Color(
                value1.getR() + (((value2.getR() - value1.getR()) * fraction) >> 16),
                value1.getG() + (((value2.getG() - value1.getG()) * fraction) >> 16),
                value1.getB() + (((value2.getB() - value1.getB()) * fraction) >> 16),
                value1.getA() + (((value2.getA() - value1.getA()) * fraction) >> 16));
    }

    public static Color Multiply(Color value, float scale) {
        Objects.requireNonNull(value, "value");
        float scaled = scale * 65_536.0f;
        int fixedScale;
        if (Float.isNaN(scaled) || scaled <= 0.0f) {
            fixedScale = 0;
        } else if (scaled >= 16_777_215.0f) {
            fixedScale = 16_777_215;
        } else {
            fixedScale = (int)scaled;
        }
        return new Color(
                Math.min(255, (value.getR() * fixedScale) >> 16),
                Math.min(255, (value.getG() * fixedScale) >> 16),
                Math.min(255, (value.getB() * fixedScale) >> 16),
                Math.min(255, (value.getA() * fixedScale) >> 16));
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof Color color && packedValue == color.packedValue;
    }

    public boolean equals(Color other) {
        return other != null && packedValue == other.packedValue;
    }

    @Override
    public int hashCode() {
        return packedValue;
    }

    @Override
    public String toString() {
        return "{R:" + getR() + " G:" + getG() + " B:" + getB() + " A:" + getA() + '}';
    }

    private static Color named(int red, int green, int blue, int alpha) {
        return new Color(pack(red, green, blue, alpha), true);
    }

    private static Color named(int red, int green, int blue) {
        return named(red, green, blue, 255);
    }

    private static int pack(int red, int green, int blue, int alpha) {
        return red | (green << 8) | (blue << 16) | (alpha << 24);
    }

    private static int packUnit(float value) {
        float scaled = value * 255.0f;
        if (Float.isNaN(scaled) || scaled <= 0.0f) {
            return 0;
        }
        if (scaled >= 255.0f) {
            return 255;
        }
        return (int)Math.rint(scaled);
    }

    private static int packFixed16(float amount) {
        float scaled = amount * 65_536.0f;
        if (Float.isNaN(scaled) || scaled <= 0.0f) {
            return 0;
        }
        if (scaled >= 16_777_215.0f) {
            return 16_777_215;
        }
        return (int)scaled;
    }

    private static int clampByte(int value) {
        return Math.max(0, Math.min(255, value));
    }

    private static int requireByte(int value, String name) {
        if (value < 0 || value > 255) {
            throw new IllegalArgumentException(name + " must be between 0 and 255");
        }
        return value;
    }

    private void ensureMutable() {
        if (frozen) {
            throw new UnsupportedOperationException("Named Color values are immutable snapshots");
        }
    }
}
