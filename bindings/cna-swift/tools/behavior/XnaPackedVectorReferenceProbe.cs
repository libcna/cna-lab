// Compile this probe against the pinned Microsoft XNA Framework 4.0 Windows
// runtime assembly. The binary is intentionally not part of this repository.
using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics.PackedVector;

internal static class XnaPackedVectorReferenceProbe
{
    private static uint FloatBits(float value)
    {
        return BitConverter.ToUInt32(BitConverter.GetBytes(value), 0);
    }

    private static float FromBits(uint value)
    {
        return BitConverter.ToSingle(BitConverter.GetBytes(value), 0);
    }

    private static void Vector(string name, ulong packed, Vector4 value, string text, int hash)
    {
        Console.WriteLine(
            "{0} packed={1:X16} decoded={2:X8},{3:X8},{4:X8},{5:X8} text={6} hash={7:X8}",
            name, packed, FloatBits(value.X), FloatBits(value.Y), FloatBits(value.Z),
            FloatBits(value.W), text, unchecked((uint)hash));
    }

    private static void Main()
    {
        Alpha8 alpha = new Alpha8(0.5f);
        Vector("Alpha8", alpha.PackedValue, ((IPackedVector)alpha).ToVector4(), alpha.ToString(), alpha.GetHashCode());
        Bgr565 bgr = new Bgr565(1f, 0.5f, 0.25f);
        Vector("Bgr565", bgr.PackedValue, ((IPackedVector)bgr).ToVector4(), bgr.ToString(), bgr.GetHashCode());
        Bgra4444 bgra4444 = new Bgra4444(1f, 0.5f, 0.25f, 0.75f);
        Vector("Bgra4444", bgra4444.PackedValue, bgra4444.ToVector4(), bgra4444.ToString(), bgra4444.GetHashCode());
        Bgra5551 bgra5551 = new Bgra5551(1f, 0.5f, 0.25f, 0.5f);
        Vector("Bgra5551", bgra5551.PackedValue, bgra5551.ToVector4(), bgra5551.ToString(), bgra5551.GetHashCode());
        Byte4 bytes = new Byte4(-1f, 0.5f, 127.5f, 256f);
        Vector("Byte4", bytes.PackedValue, bytes.ToVector4(), bytes.ToString(), bytes.GetHashCode());
        HalfSingle half = new HalfSingle(1f);
        Vector("HalfSingle", half.PackedValue, ((IPackedVector)half).ToVector4(), half.ToString(), half.GetHashCode());
        HalfVector2 half2 = new HalfVector2(-0f, 1f);
        Vector("HalfVector2", half2.PackedValue, ((IPackedVector)half2).ToVector4(), half2.ToString(), half2.GetHashCode());
        HalfVector4 half4 = new HalfVector4(1f, -2f, 0.5f, -0f);
        Vector("HalfVector4", half4.PackedValue, half4.ToVector4(), half4.ToString(), half4.GetHashCode());
        NormalizedByte2 nb2 = new NormalizedByte2(-1f, 0.5f);
        Vector("NormalizedByte2", nb2.PackedValue, ((IPackedVector)nb2).ToVector4(), nb2.ToString(), nb2.GetHashCode());
        NormalizedByte4 nb4 = new NormalizedByte4(-1f, -0.5f, 0.5f, 1f);
        Vector("NormalizedByte4", nb4.PackedValue, nb4.ToVector4(), nb4.ToString(), nb4.GetHashCode());
        NormalizedShort2 ns2 = new NormalizedShort2(-1f, 0.5f);
        Vector("NormalizedShort2", ns2.PackedValue, ((IPackedVector)ns2).ToVector4(), ns2.ToString(), ns2.GetHashCode());
        NormalizedShort4 ns4 = new NormalizedShort4(-1f, -0.5f, 0.5f, 1f);
        Vector("NormalizedShort4", ns4.PackedValue, ns4.ToVector4(), ns4.ToString(), ns4.GetHashCode());
        Rg32 rg = new Rg32(0.5f, 1f);
        Vector("Rg32", rg.PackedValue, ((IPackedVector)rg).ToVector4(), rg.ToString(), rg.GetHashCode());
        Rgba1010102 rgba10 = new Rgba1010102(1f, 0.5f, 0.25f, 0.5f);
        Vector("Rgba1010102", rgba10.PackedValue, rgba10.ToVector4(), rgba10.ToString(), rgba10.GetHashCode());
        Rgba64 rgba64 = new Rgba64(1f, 0.5f, 0.25f, 0.75f);
        Vector("Rgba64", rgba64.PackedValue, rgba64.ToVector4(), rgba64.ToString(), rgba64.GetHashCode());
        Short2 short2 = new Short2(-32768f, 32767f);
        Vector("Short2", short2.PackedValue, ((IPackedVector)short2).ToVector4(), short2.ToString(), short2.GetHashCode());
        Short4 short4 = new Short4(-32768f, -0.5f, 0.5f, 32767f);
        Vector("Short4", short4.PackedValue, short4.ToVector4(), short4.ToString(), short4.GetHashCode());

        uint[] specialInputs = {
            0x00000000, 0x80000000, 0x33000000, 0x33800000, 0x387FC000,
            0x38800000, 0x477FE000, 0x477FF000, 0x47800000,
            0x7F800000, 0xFF800000, 0x7FC00001, 0xFFC12345
        };
        foreach (uint inputBits in specialInputs)
        {
            HalfSingle item = new HalfSingle(FromBits(inputBits));
            Console.WriteLine("HALF_PACK input={0:X8} packed={1:X4} decoded={2:X8}",
                inputBits, item.PackedValue, FloatBits(item.ToSingle()));
        }

        ushort[] halfPatterns = {
            0x0000, 0x8000, 0x0001, 0x03FF, 0x0400, 0x7BFF,
            0x7C00, 0x7C01, 0x7FFF, 0xFC00, 0xFFFF
        };
        foreach (ushort bits in halfPatterns)
        {
            HalfSingle item = new HalfSingle(0f);
            item.PackedValue = bits;
            HalfSingle encoded = new HalfSingle(item.ToSingle());
            Console.WriteLine("HALF_DECODE packed={0:X4} decoded={1:X8} repacked={2:X4}",
                bits, FloatBits(item.ToSingle()), encoded.PackedValue);
        }

        int stable = 0;
        int changed = 0;
        for (int raw = 0; raw <= ushort.MaxValue; raw++)
        {
            HalfSingle item = new HalfSingle(0f);
            item.PackedValue = (ushort)raw;
            HalfSingle encoded = new HalfSingle(item.ToSingle());
            if (encoded.PackedValue == (ushort)raw) stable++; else changed++;
        }
        Console.WriteLine("HALF_EXHAUSTIVE iterations=65536 stable={0} changed={1}", stable, changed);

        float belowHalf = FromBits(0x3EFFFFFE);
        float exactHalf = FromBits(0x3F000000);
        float aboveHalf = FromBits(0x3F000001);
        Console.WriteLine("ALPHA_THRESHOLD below={0:X2} exact={1:X2} above={2:X2}",
            new Alpha8(belowHalf).PackedValue, new Alpha8(exactHalf).PackedValue,
            new Alpha8(aboveHalf).PackedValue);
        Console.WriteLine("BGRA5551_THRESHOLD below={0:X4} exact={1:X4} above={2:X4}",
            new Bgra5551(0f, 0f, 0f, belowHalf).PackedValue,
            new Bgra5551(0f, 0f, 0f, exactHalf).PackedValue,
            new Bgra5551(0f, 0f, 0f, aboveHalf).PackedValue);
    }
}
