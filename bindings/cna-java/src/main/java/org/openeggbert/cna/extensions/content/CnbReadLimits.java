package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

/**
 * What a reader refuses before it allocates anything.
 *
 * <p>These are the defence against a malformed or hostile file: a header claiming a four-gigabyte
 * chunk must be refused at the header, not after the allocation. {@link #standard()} is CNA's own
 * default set, and a caller tightening them is the normal case -- a game that knows its largest
 * asset can say so.
 *
 * @param MaxFileSize the largest file that will be read at all
 * @param MaxChunkSize the largest single chunk
 * @param MaxTotalUncompressedSize the largest total after decompression, which is what stops a
 *     small file expanding without bound
 * @param MaxChunkCount the most chunks a file may declare
 * @param MaxStringBytes the longest string a reader will accept
 * @param MaxArrayElementCount the most elements one counted array may declare
 * @param MaxChunkAlignment the largest alignment a chunk may ask for
 */
public record CnbReadLimits(
        long MaxFileSize,
        long MaxChunkSize,
        long MaxTotalUncompressedSize,
        long MaxChunkCount,
        long MaxStringBytes,
        long MaxArrayElementCount,
        long MaxChunkAlignment) {

    /** Returns CNA's own default limits. */
    public static CnbReadLimits standard() {
        CnbExtension.requireAvailable();
        long[] values = new long[7];
        CnbExtension.check("CnbReadLimits.standard",
                NativeCnbRoutes.cnbReadLimitsInit(values));
        return new CnbReadLimits(values[0], values[1], values[2], values[3], values[4],
                values[5], values[6]);
    }

    long[] encode() {
        return new long[] {MaxFileSize, MaxChunkSize, MaxTotalUncompressedSize, MaxChunkCount,
            MaxStringBytes, MaxArrayElementCount, MaxChunkAlignment};
    }

    static CnbReadLimits decode(long[] values) {
        return new CnbReadLimits(values[0], values[1], values[2], values[3], values[4],
                values[5], values[6]);
    }
}
