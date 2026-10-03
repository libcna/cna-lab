package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

/** Expectations in this class are transcribed from the XNA 4.0 IL packing operations. */
final class PackedVectorTests {

    private static final float TOLERANCE = 1.0e-4f;

    @Test
    void Bgr565UsesFiveSixFiveChannelOrderAndOpaqueInterfaceAlpha() {
        assertEquals(0xf800, new Bgr565(1.0f, 0.0f, 0.0f).getPackedValue());
        assertEquals(0x07e0, new Bgr565(0.0f, 1.0f, 0.0f).getPackedValue());
        assertEquals(0x001f, new Bgr565(0.0f, 0.0f, 1.0f).getPackedValue());
        Bgr565 value = new Bgr565(new Vector3(0.2f, 0.4f, 0.6f));
        assertEquals(1.0f, value.ToVector4().W);
        assertEquals(String.format("%04X", value.getPackedValue()), value.toString());
    }

    @Test
    void Bgra4444StoresBlueLowAndAlphaHigh() {
        assertEquals(0x000f, new Bgra4444(0.0f, 0.0f, 1.0f, 0.0f).getPackedValue());
        assertEquals(0x00f0, new Bgra4444(0.0f, 1.0f, 0.0f, 0.0f).getPackedValue());
        assertEquals(0x0f00, new Bgra4444(1.0f, 0.0f, 0.0f, 0.0f).getPackedValue());
        assertEquals(0xf000, new Bgra4444(0.0f, 0.0f, 0.0f, 1.0f).getPackedValue());
    }

    @Test
    void HalfVectorsUseXnaSaturationAndLowToHighComponentOrder() {
        HalfVector2 pair = new HalfVector2(1.0f, -2.0f);
        assertEquals(0xc000_3c00L, pair.getPackedValue());
        assertEquals(new Vector2(1.0f, -2.0f), pair.ToVector2());
        assertEquals(new Vector4(1.0f, -2.0f, 0.0f, 1.0f), pair.ToVector4());

        HalfVector4 four = new HalfVector4(1.0f, -2.0f, 0.5f, Float.POSITIVE_INFINITY);
        assertEquals(0x7fff_3800_c000_3c00L, four.getPackedValue());
        assertEquals(131_008.0f, four.ToVector4().W);
    }

    @Test
    void SignedNormalizedFormatsUseNearestEvenAndSpecialCaseRawMinimum() {
        NormalizedByte4 bytes = new NormalizedByte4(
                0.5f / 127.0f, -0.5f / 127.0f, 1.0f, -1.0f);
        assertEquals(0x817f_0000L, bytes.getPackedValue());

        NormalizedShort2 shorts = new NormalizedShort2(
                0.5f / 32_767.0f, -0.5f / 32_767.0f);
        assertEquals(0L, shorts.getPackedValue());

        NormalizedShort4 minimum = new NormalizedShort4();
        minimum.setPackedValue(0x8000_8000_8000_8000L);
        assertEquals(new Vector4(-1.0f), minimum.ToVector4());
    }

    @Test
    void UnsignedNormalizedFormatsPreserveWidthsAndDefaultNarrowComponents() {
        Rg32 rg = new Rg32(1.0f, 0.5f);
        assertEquals(0x8000_ffffL, rg.getPackedValue());
        assertEquals(0.0f, rg.ToVector4().Z);
        assertEquals(1.0f, rg.ToVector4().W);

        Rgba1010102 rgba10 = new Rgba1010102(1.0f, 0.0f, 0.0f, 0.5f);
        assertEquals(0x8000_03ffL, rgba10.getPackedValue());
        assertEquals(2.0f / 3.0f, rgba10.ToVector4().W, TOLERANCE);

        Rgba64 rgba64 = new Rgba64(0.0f, 0.5f, 1.0f, 0.25f);
        Vector4 expanded = rgba64.ToVector4();
        assertEquals(0.0f, expanded.X, TOLERANCE);
        assertEquals(0.5f, expanded.Y, TOLERANCE);
        assertEquals(1.0f, expanded.Z, TOLERANCE);
        assertEquals(0.25f, expanded.W, TOLERANCE);
    }

    @Test
    void Short4IsUnnormalizedNearestEvenAndClamped() {
        Short4 value = new Short4(0.5f, 1.5f, 40_000.0f, -40_000.0f);
        Vector4 expanded = value.ToVector4();
        assertEquals(0.0f, expanded.X);
        assertEquals(2.0f, expanded.Y);
        assertEquals(32_767.0f, expanded.Z);
        assertEquals(-32_768.0f, expanded.W);
    }

    @Test
    void PackedValueSettersCopiesEqualityHashAndStringsPreserveRawBits() {
        Bgr565 original = new Bgr565();
        original.setPackedValue(0xa55a);
        Bgr565 copy = new Bgr565(original);
        assertNotSame(original, copy);
        assertEquals(original, copy);
        assertTrue(original.equals(copy));
        assertEquals(original.hashCode(), copy.hashCode());
        assertEquals("A55A", copy.toString());
        assertThrows(IllegalArgumentException.class, () -> original.setPackedValue(0x1_0000));

        Rgba64 unsigned64 = new Rgba64();
        unsigned64.setPackedValue(-1L);
        assertEquals(-1L, unsigned64.getPackedValue());
        assertEquals("FFFFFFFFFFFFFFFF", unsigned64.toString());
    }

    @Test
    void AllSeventeenFormatsRoundTripThroughTheUntypedContract() {
        IPackedVector[] formats = {
                new Alpha8(), new Bgr565(), new Bgra4444(), new Bgra5551(),
                new Byte4(), new HalfSingle(), new HalfVector2(), new HalfVector4(),
                new NormalizedByte2(), new NormalizedByte4(),
                new NormalizedShort2(), new NormalizedShort4(),
                new Rg32(), new Rgba1010102(), new Rgba64(),
                new Short2(), new Short4()
        };
        for (IPackedVector format : formats) {
            format.PackFromVector4(new Vector4());
            Vector4 zero = format.ToVector4();
            format.PackFromVector4(new Vector4(1.0f));
            assertNotEquals(zero, format.ToVector4(), format.getClass().getSimpleName());
        }
    }
}
