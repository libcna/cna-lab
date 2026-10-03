package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector4;
import Microsoft.Xna.Framework.Graphics.PackedVector.Alpha8;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgr565;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgra4444;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgra5551;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfSingle;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfVector2;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfVector4;
import Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedByte2;
import Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedByte4;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rg32;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rgba1010102;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rgba64;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

final class TextureDataCodecTests {

    @Test
    void CodecsUseExactCnaTagsNativeWidthsAndSurfaceFormats() {
        assertCodec(Color.class, SurfaceFormat.Color, 0, 4);
        assertCodec(Bgr565.class, SurfaceFormat.Bgr565, 1, 2);
        assertCodec(Integer.class, SurfaceFormat.Dxt1, 4, 1);
        assertCodec(Integer.class, SurfaceFormat.Dxt3, 4, 1);
        assertCodec(Integer.class, SurfaceFormat.Dxt5, 4, 1);
        assertCodec(Bgra5551.class, SurfaceFormat.Bgra5551, 2, 2);
        assertCodec(Bgra4444.class, SurfaceFormat.Bgra4444, 3, 2);
        assertCodec(NormalizedByte2.class, SurfaceFormat.NormalizedByte2, 5, 2);
        assertCodec(NormalizedByte4.class, SurfaceFormat.NormalizedByte4, 6, 4);
        assertCodec(Rgba1010102.class, SurfaceFormat.Rgba1010102, 7, 4);
        assertCodec(Rg32.class, SurfaceFormat.Rg32, 8, 4);
        assertCodec(Rgba64.class, SurfaceFormat.Rgba64, 9, 8);
        assertCodec(Alpha8.class, SurfaceFormat.Alpha8, 10, 1);
        assertCodec(Float.class, SurfaceFormat.Single, 11, 4);
        assertCodec(Vector2.class, SurfaceFormat.Vector2, 12, 8);
        assertCodec(Vector4.class, SurfaceFormat.Vector4, 13, 16);
        assertCodec(HalfSingle.class, SurfaceFormat.HalfSingle, 14, 2);
        assertCodec(HalfVector2.class, SurfaceFormat.HalfVector2, 15, 4);
        assertCodec(HalfVector4.class, SurfaceFormat.HalfVector4, 16, 8);
        assertCodec(HalfVector4.class, SurfaceFormat.HdrBlendable, 16, 8);

        assertThrows(UnsupportedOperationException.class,
                () -> TextureDataCodec.select(Color.class, SurfaceFormat.Vector4));
        assertThrows(UnsupportedOperationException.class,
                () -> TextureDataCodec.select(Object.class, SurfaceFormat.Color));
    }

    @Test
    void ColorCodecSnapshotsOnlyTheSelectedWindowAndDecodesFreshValues() {
        TextureDataCodec codec = TextureDataCodec.select(Color.class, SurfaceFormat.Color);
        Color[] source = {
                null,
                new Color(1, 2, 3, 4),
                new Color(250, 128, 64, 32),
                null
        };
        byte[] encoded = codec.encode(source, 1, 2);
        source[1].setR(99);

        Color before = new Color(9, 9, 9, 9);
        Color after = new Color(8, 8, 8, 8);
        Color[] destination = {before, null, null, after};
        codec.decodeInto(encoded, destination, 1, 2);

        assertSame(before, destination[0]);
        assertEquals(new Color(1, 2, 3, 4), destination[1]);
        assertEquals(new Color(250, 128, 64, 32), destination[2]);
        assertSame(after, destination[3]);
        assertNotSame(source[1], destination[1]);
    }

    @Test
    void ScalarVectorAndPackedCodecsRoundTripBinaryRepresentations() {
        assertRoundTrip(
                TextureDataCodec.select(Float.class, SurfaceFormat.Single),
                new Float[]{-0.0f, Float.intBitsToFloat(0x7fc01234)}, 0, 2);
        assertRoundTrip(
                TextureDataCodec.select(Vector2.class, SurfaceFormat.Vector2),
                new Vector2[]{new Vector2(1.25f, -2.5f)}, 0, 1);
        assertRoundTrip(
                TextureDataCodec.select(Vector4.class, SurfaceFormat.Vector4),
                new Vector4[]{new Vector4(1.0f, 2.0f, 3.0f, 4.0f)}, 0, 1);

        Bgra5551 bgra = new Bgra5551();
        bgra.setPackedValue(0xa55a);
        assertRoundTrip(
                TextureDataCodec.select(Bgra5551.class, SurfaceFormat.Bgra5551),
                new Bgra5551[]{bgra}, 0, 1);

        NormalizedByte2 normalized = new NormalizedByte2();
        normalized.setPackedValue(0x81fe);
        assertRoundTrip(
                TextureDataCodec.select(
                        NormalizedByte2.class, SurfaceFormat.NormalizedByte2),
                new NormalizedByte2[]{normalized}, 0, 1);

        Alpha8 alpha = new Alpha8();
        alpha.setPackedValue(0xe7);
        assertRoundTrip(
                TextureDataCodec.select(Alpha8.class, SurfaceFormat.Alpha8),
                new Alpha8[]{alpha}, 0, 1);

        HalfSingle half = new HalfSingle();
        half.setPackedValue(0x3555);
        assertRoundTrip(
                TextureDataCodec.select(HalfSingle.class, SurfaceFormat.HalfSingle),
                new HalfSingle[]{half}, 0, 1);

        Bgr565 bgr = new Bgr565();
        bgr.setPackedValue(0xa55a);
        assertRoundTrip(TextureDataCodec.select(Bgr565.class, SurfaceFormat.Bgr565),
                new Bgr565[]{bgr}, 0, 1);
        Bgra4444 bgra4444 = new Bgra4444();
        bgra4444.setPackedValue(0x5aa5);
        assertRoundTrip(TextureDataCodec.select(Bgra4444.class, SurfaceFormat.Bgra4444),
                new Bgra4444[]{bgra4444}, 0, 1);
        NormalizedByte4 normalized4 = new NormalizedByte4();
        normalized4.setPackedValue(0x89ab_cdefL);
        assertRoundTrip(TextureDataCodec.select(
                        NormalizedByte4.class, SurfaceFormat.NormalizedByte4),
                new NormalizedByte4[]{normalized4}, 0, 1);
        Rgba1010102 rgba10 = new Rgba1010102();
        rgba10.setPackedValue(0xfedc_ba98L);
        assertRoundTrip(TextureDataCodec.select(
                        Rgba1010102.class, SurfaceFormat.Rgba1010102),
                new Rgba1010102[]{rgba10}, 0, 1);
        Rg32 rg = new Rg32();
        rg.setPackedValue(0x7654_3210L);
        assertRoundTrip(TextureDataCodec.select(Rg32.class, SurfaceFormat.Rg32),
                new Rg32[]{rg}, 0, 1);
        Rgba64 rgba64 = new Rgba64();
        rgba64.setPackedValue(0xfedc_ba98_7654_3210L);
        assertRoundTrip(TextureDataCodec.select(Rgba64.class, SurfaceFormat.Rgba64),
                new Rgba64[]{rgba64}, 0, 1);
        HalfVector2 half2 = new HalfVector2();
        half2.setPackedValue(0xc000_3c00L);
        assertRoundTrip(TextureDataCodec.select(
                        HalfVector2.class, SurfaceFormat.HalfVector2),
                new HalfVector2[]{half2}, 0, 1);
        HalfVector4 half4 = new HalfVector4();
        half4.setPackedValue(0x4400_3800_c000_3c00L);
        assertRoundTrip(TextureDataCodec.select(
                        HalfVector4.class, SurfaceFormat.HalfVector4),
                new HalfVector4[]{half4}, 0, 1);
    }

    @Test
    void JavaIntegerByteProjectionValidatesUnsignedRange() {
        TextureDataCodec codec = TextureDataCodec.select(Integer.class, SurfaceFormat.Dxt1);
        Integer[] source = {0, 1, 127, 128, 255};
        byte[] encoded = codec.encode(source, 0, source.length);
        Integer[] decoded = new Integer[source.length];
        codec.decodeInto(encoded, decoded, 0, decoded.length);
        assertArrayEquals(source, decoded);

        assertThrows(IllegalArgumentException.class,
                () -> codec.encode(new Integer[]{-1}, 0, 1));
        assertThrows(IllegalArgumentException.class,
                () -> codec.encode(new Integer[]{256}, 0, 1));
    }

    private static void assertCodec(
            Class<?> componentType,
            SurfaceFormat format,
            int dataType,
            int elementSize) {
        TextureDataCodec codec = TextureDataCodec.select(componentType, format);
        assertEquals(dataType, codec.dataType());
        assertEquals(elementSize, codec.elementSize());
    }

    private static <T> void assertRoundTrip(
            TextureDataCodec codec,
            T[] source,
            int startIndex,
            int elementCount) {
        byte[] encoded = codec.encode(source, startIndex, elementCount);
        @SuppressWarnings("unchecked")
        T[] decoded = (T[])java.lang.reflect.Array.newInstance(
                source.getClass().getComponentType(), source.length);
        codec.decodeInto(encoded, decoded, startIndex, elementCount);
        assertArrayEquals(source, decoded);
    }
}
