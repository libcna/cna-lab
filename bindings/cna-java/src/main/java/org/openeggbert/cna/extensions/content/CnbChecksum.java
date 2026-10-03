package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.Objects;

/**
 * The CRC-32C every {@code .cnb} chunk carries.
 *
 * <p>CRC-32C, not the CRC-32 {@code java.util.zip} computes: a different polynomial and a
 * different answer for the same bytes, so a Java-side checksum would not match the file. This is
 * CNA's own, and it uses the processor's instruction where there is one.
 */
public final class CnbChecksum {

    private CnbChecksum() {
    }

    /**
     * Returns the checksum of some bytes.
     *
     * @param data the bytes to sum
     * @return the checksum, widened so its value is never negative
     */
    public static long of(byte[] data) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(data, "data");
        int[] checksum = new int[1];
        CnbExtension.check("CnbChecksum.of", NativeCnbRoutes.cnbCrc32c(data, checksum));
        return checksum[0] & 0xFFFFFFFFL;
    }

    /**
     * Continues a checksum over a further block.
     *
     * @param previous the checksum so far
     * @param data the next block
     * @return the checksum of everything summed so far
     */
    public static long continued(long previous, byte[] data) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(data, "data");
        int[] checksum = new int[1];
        CnbExtension.check("CnbChecksum.continued",
                NativeCnbRoutes.cnbCrc32cContinue((int) previous, data, checksum));
        return checksum[0] & 0xFFFFFFFFL;
    }

    /**
     * Returns the checksum computed without the processor's instruction.
     *
     * <p>Same answer as {@link #of(byte[])} on every input; it exists so a caller can prove that,
     * which is the only way to tell a broken hardware path from a broken file.
     *
     * @param data the bytes to sum
     * @return the checksum
     */
    public static long portable(byte[] data) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(data, "data");
        int[] checksum = new int[1];
        CnbExtension.check("CnbChecksum.portable",
                NativeCnbRoutes.cnbCrc32cPortable(data, checksum));
        return checksum[0] & 0xFFFFFFFFL;
    }

    /** Reports whether this build uses the processor's CRC-32C instruction. */
    public static boolean usesHardware() {
        CnbExtension.requireAvailable();
        boolean[] hardware = new boolean[1];
        CnbExtension.check("CnbChecksum.usesHardware",
                NativeCnbRoutes.cnbCrc32cUsesHardware(hardware));
        return hardware[0];
    }
}
