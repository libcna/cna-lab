package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.nio.charset.StandardCharsets;

/**
 * How one chunk of a {@code .cnb} file is stored.
 *
 * <p><strong>The numbers are wire format and frozen.</strong> Codec 2 is Zstandard in every
 * {@code .cnb} ever written, whether or not the build reading it can decompress one. Whether this
 * build actually can is a different question, and {@link #isSupported()} is the one that answers
 * it -- a codec having a name here is not a promise that it works.
 */
public enum CnbCompression {

    /** Stored uncompressed. Always available, and the default. */
    None,

    /** LZ4. The identifier is assigned; no build implements it yet. */
    Lz4,

    /** Zstandard. Available when CNA was built with libzstd. */
    Zstd,

    /** Deflate. */
    Deflate;

    /**
     * Reports whether this build can actually use the codec.
     *
     * <p>Asking rather than assuming is the point: a file may name a codec this build cannot
     * decompress, and finding out at read time is worse than finding out now.
     */
    public boolean isSupported() {
        boolean[] supported = new boolean[1];
        CnbExtension.check("CnbCompression.isSupported",
                NativeCnbRoutes.cnbIsCompressionSupported(ordinal(), supported));
        return supported[0];
    }

    /** Returns CNA's own name for the codec. */
    public String getName() {
        long[] bytes = new long[1];
        CnbExtension.check("CnbCompression.getName",
                NativeCnbRoutes.cnbGetCompressionNameSize(ordinal(), bytes));
        byte[] destination = new byte[(int) bytes[0]];
        CnbExtension.check("CnbCompression.getName",
                NativeCnbRoutes.cnbCopyCompressionName(ordinal(), destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    /** Returns the codec one wire value names, refusing a value this build has no constant for. */
    static CnbCompression fromValue(long value) {
        CnbCompression[] values = values();
        if (value < 0 || value >= values.length) {
            throw new CnbFormatException(
                    "the file names compression codec " + value + ", which this build has no "
                    + "constant for; the numbers are wire format, so this is a newer writer");
        }
        return values[(int) value];
    }
}
