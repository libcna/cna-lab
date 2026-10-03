package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.Objects;

/**
 * The parts of the {@code .cnb} container that belong to no single file.
 *
 * <p>Chunk compression, the rules a logical name has to satisfy, and the overflow-checked
 * arithmetic a reader does on a file's own numbers. All of it is CNA's, because a Java
 * reimplementation would be a second answer to a wire-format question that has one.
 */
public final class CnbFormat {

    private CnbFormat() {
    }

    /**
     * Compresses bytes with one codec.
     *
     * @param raw the bytes to compress
     * @param codec how to compress them, which {@link CnbCompression#isSupported()} says this
     *     build can actually do
     * @param level the codec's own level
     * @return the compressed bytes
     */
    public static byte[] compress(byte[] raw, CnbCompression codec, int level) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(raw, "raw");
        Objects.requireNonNull(codec, "codec");
        long[] size = new long[1];
        CnbExtension.check("CnbFormat.compress",
                NativeCnbRoutes.cnbGetCompressedByteCount(raw, codec.ordinal(), level, size));
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("CnbFormat.compress", NativeCnbRoutes
                .cnbCopyCompressed(raw, codec.ordinal(), level, destination, written));
        if (written[0] == destination.length) {
            return destination;
        }
        byte[] exact = new byte[Math.toIntExact(written[0])];
        System.arraycopy(destination, 0, exact, 0, exact.length);
        return exact;
    }

    /**
     * Decompresses bytes with one codec.
     *
     * <p>Both sizes are required, and that is the point: a compressed block that claims to expand
     * to more than the caller allows is refused before anything is allocated, which is what stops
     * a small hostile file from exhausting memory.
     *
     * @param stored the compressed bytes
     * @param codec how they were compressed
     * @param uncompressedSize how many bytes the file says they become
     * @param maximumUncompressedSize the most the caller is willing to allocate
     * @return the decompressed bytes
     */
    public static byte[] decompress(byte[] stored, CnbCompression codec,
            long uncompressedSize, long maximumUncompressedSize) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(stored, "stored");
        Objects.requireNonNull(codec, "codec");
        if (uncompressedSize > maximumUncompressedSize) {
            throw new CnbFormatException("the block claims to expand to " + uncompressedSize
                    + " bytes, past the " + maximumUncompressedSize + " allowed");
        }
        byte[] destination = new byte[Math.toIntExact(uncompressedSize)];
        long[] written = new long[1];
        CnbExtension.check("CnbFormat.decompress", NativeCnbRoutes.cnbCopyDecompressed(
                stored, codec.ordinal(), uncompressedSize, maximumUncompressedSize,
                destination, written));
        return destination;
    }

    /**
     * Returns why a logical name is not usable, or the empty string when it is.
     *
     * <p>A name is how one asset refers to another, so its rules are the format's rather than the
     * file system's; asking CNA is how a tool reports the reason rather than guessing at it.
     *
     * @param logicalName the name to check
     * @return the problem, empty when there is none
     */
    public static String logicalNameProblem(String logicalName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(logicalName, "logicalName");
        byte[] encoded = CnbExtension.utf8(logicalName);
        return CnbExtension.text("CnbFormat.logicalNameProblem",
                bytes -> NativeCnbRoutes.cnbGetLogicalNameProblemSize(encoded, bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbCopyLogicalNameProblem(encoded, destination, bytes));
    }

    /** Reports whether some text is well-formed UTF-8, which every string in the format must be. */
    public static boolean isWellFormedUtf8(byte[] utf8) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(utf8, "utf8");
        boolean[] wellFormed = new boolean[1];
        CnbExtension.check("CnbFormat.isWellFormedUtf8",
                NativeCnbRoutes.cnbIsWellFormedUtf8(utf8, wellFormed));
        return wellFormed[0];
    }

    /**
     * Adds two of a file's own numbers, refusing an overflow.
     *
     * <p>A malformed file's offset plus size is exactly where an unchecked add wraps and a bounds
     * check then passes, so the format does this arithmetic checked and so does this.
     *
     * @param first the first value
     * @param second the second value
     * @return the sum
     * @throws CnbFormatException when the sum does not fit
     */
    public static long checkedAdd(long first, long second) {
        CnbExtension.requireAvailable();
        long[] sum = new long[1];
        CnbExtension.check("CnbFormat.checkedAdd",
                NativeCnbRoutes.cnbCheckedAdd(first, second, sum));
        return sum[0];
    }

    /**
     * Multiplies two of a file's own numbers, refusing an overflow.
     *
     * @param first the first value
     * @param second the second value
     * @return the product
     * @throws CnbFormatException when the product does not fit
     */
    public static long checkedMultiply(long first, long second) {
        CnbExtension.requireAvailable();
        long[] product = new long[1];
        CnbExtension.check("CnbFormat.checkedMultiply",
                NativeCnbRoutes.cnbCheckedMultiply(first, second, product));
        return product[0];
    }
}
