package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.nio.charset.StandardCharsets;
import java.util.Objects;

/**
 * A checked cursor over one chunk's bytes.
 *
 * <p>Every read is bounds-checked against what remains and against the document's limits, so a
 * malformed file cannot make a reader walk off the end or allocate a string it invented. That is
 * the whole reason to use this rather than a {@code ByteBuffer}: the limits travel with the
 * cursor, and {@link #readCount(long, String)} refuses an element count the remaining bytes
 * cannot possibly hold before anything is allocated for it.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op. A reader
 * opened over a document's chunk does not keep the document alive: close the reader first.
 */
public final class CnbReader implements AutoCloseable {

    private final long handle;
    private boolean closed;

    CnbReader(long handle) {
        this.handle = handle;
    }

    /**
     * Creates a reader over caller bytes.
     *
     * @param data the bytes to read; CNA copies them, so the array stays the caller's
     * @param context what the bytes are, for the diagnostic a failed read reports
     * @param limits what the reader refuses
     * @return the reader, which the caller closes
     */
    public static CnbReader of(byte[] data, String context, CnbReadLimits limits) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(data, "data");
        Objects.requireNonNull(context, "context");
        Objects.requireNonNull(limits, "limits");
        long[] reader = new long[1];
        CnbExtension.check("CnbReader.of", NativeCnbRoutes.cnbReaderCreate(
                data, CnbExtension.utf8(context), limits.encode(), reader));
        return new CnbReader(reader[0]);
    }

    /** Returns how many bytes are left to read. */
    public long getRemaining() {
        long[] remaining = new long[1];
        CnbExtension.check("CnbReader.getRemaining",
                NativeCnbRoutes.cnbReaderGetRemaining(open(), remaining));
        return remaining[0];
    }

    /** Returns how far into the chunk the cursor is. */
    public long getPosition() {
        long[] position = new long[1];
        CnbExtension.check("CnbReader.getPosition",
                NativeCnbRoutes.cnbReaderGetPosition(open(), position));
        return position[0];
    }

    /** Returns how many bytes the chunk holds in total. */
    public long getSize() {
        long[] size = new long[1];
        CnbExtension.check("CnbReader.getSize", NativeCnbRoutes.cnbReaderGetSize(open(), size));
        return size[0];
    }

    /** Returns what the reader calls these bytes in a diagnostic. */
    public String getContext() {
        long handle = open();
        return CnbExtension.text("CnbReader.getContext",
                bytes -> NativeCnbRoutes.cnbReaderGetContextSize(handle, bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbReaderCopyContext(handle, destination, bytes));
    }

    /** Reads one unsigned byte, widened so its value is never negative. */
    public int readUnsignedByte() {
        byte[] value = new byte[1];
        CnbExtension.check("CnbReader.readUnsignedByte",
                NativeCnbRoutes.cnbReaderReadU8(open(), value));
        return value[0] & 0xFF;
    }

    /** Reads one unsigned 16-bit value, widened so its value is never negative. */
    public int readUnsignedShort() {
        int[] value = new int[1];
        CnbExtension.check("CnbReader.readUnsignedShort",
                NativeCnbRoutes.cnbReaderReadU16(open(), value));
        return value[0];
    }

    /** Reads one unsigned 32-bit value, widened so its value is never negative. */
    public long readUnsignedInt() {
        int[] value = new int[1];
        CnbExtension.check("CnbReader.readUnsignedInt",
                NativeCnbRoutes.cnbReaderReadU32(open(), value));
        return value[0] & 0xFFFFFFFFL;
    }

    /**
     * Reads one unsigned 64-bit value.
     *
     * <p>Java has no unsigned long, so a value above {@code Long.MAX_VALUE} comes back negative;
     * {@link Long#toUnsignedString(long)} and {@link Long#compareUnsigned(long, long)} read it
     * correctly. Widening would need a {@code BigInteger} per read, which no CNB field needs.
     */
    public long readUnsignedLong() {
        long[] value = new long[1];
        CnbExtension.check("CnbReader.readUnsignedLong",
                NativeCnbRoutes.cnbReaderReadU64(open(), value));
        return value[0];
    }

    /** Reads one signed 32-bit value. */
    public int readInt() {
        int[] value = new int[1];
        CnbExtension.check("CnbReader.readInt", NativeCnbRoutes.cnbReaderReadI32(open(), value));
        return value[0];
    }

    /** Reads one 32-bit float. */
    public float readFloat() {
        float[] value = new float[1];
        CnbExtension.check("CnbReader.readFloat",
                NativeCnbRoutes.cnbReaderReadF32(open(), value));
        return value[0];
    }

    /** Reads one 64-bit float. */
    public double readDouble() {
        double[] value = new double[1];
        CnbExtension.check("CnbReader.readDouble",
                NativeCnbRoutes.cnbReaderReadF64(open(), value));
        return value[0];
    }

    /**
     * Reads one length-prefixed UTF-8 string.
     *
     * <p>The length is checked against the reader's string limit before anything is allocated, so
     * a file claiming a gigabyte-long name is refused rather than believed.
     */
    public String readString() {
        long handle = open();
        long[] bytes = new long[1];
        CnbExtension.check("CnbReader.readString",
                NativeCnbRoutes.cnbReaderReadString(handle, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        CnbExtension.check("CnbReader.readString",
                NativeCnbRoutes.cnbReaderCopyString(handle, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    /**
     * Reads an element count and checks it against what remains.
     *
     * <p>This is the read that keeps a malformed file from making the caller allocate: the count
     * is refused unless the remaining bytes could actually hold that many elements.
     *
     * @param elementByteSize how many bytes one element takes
     * @param whatIsBeingCounted what the elements are, for the diagnostic
     * @return the count
     */
    public int readCount(long elementByteSize, String whatIsBeingCounted) {
        Objects.requireNonNull(whatIsBeingCounted, "whatIsBeingCounted");
        int[] count = new int[1];
        CnbExtension.check("CnbReader.readCount", NativeCnbRoutes.cnbReaderReadCount(
                open(), elementByteSize, CnbExtension.utf8(whatIsBeingCounted), count));
        return count[0];
    }

    /**
     * Reads a fixed number of bytes.
     *
     * @param byteCount how many to read
     * @return the bytes
     */
    public byte[] readBytes(int byteCount) {
        byte[] destination = new byte[byteCount];
        long[] written = new long[1];
        CnbExtension.check("CnbReader.readBytes",
                NativeCnbRoutes.cnbReaderReadBytes(open(), byteCount, destination, written));
        return destination;
    }

    /**
     * Reads a seconds value and refuses anything a {@code TimeSpan} could not hold.
     *
     * <p>Narrower than {@link #readDouble()} on purpose: the canonical {@code TimeSpan} factory
     * throws for a NaN or an out-of-range value, and that exception is not what a content
     * subsystem's callers catch. This checks the range before the value could reach one, so a
     * malformed file surfaces as a content failure naming the file.
     *
     * @param what the noun the failure message uses, such as {@code "the clip duration"}
     * @return the value, which a {@code TimeSpan} can hold
     */
    public double readSeconds(String what) {
        Objects.requireNonNull(what, "what");
        double[] value = new double[1];
        CnbExtension.check("CnbReader.readSeconds", NativeCnbRoutes.cnbReaderReadSeconds(
                open(), what.getBytes(StandardCharsets.UTF_8), value));
        return value[0];
    }

    /**
     * Reads one keyframe, in the form {@link CnbByteWriter#writeKeyframe} writes it.
     *
     * @return the pose
     */
    public CnbKeyframe readKeyframe() {
        float[] floating = new float[CnbKeyframes.FLOATS];
        double[] doubles = new double[CnbKeyframes.DOUBLES];
        CnbExtension.check("CnbReader.readKeyframe",
                NativeCnbRoutes.cnbReaderReadKeyframe(open(), floating, doubles));
        return CnbKeyframes.read(floating, doubles, 0);
    }

    /**
     * Fails with a diagnostic built from this cursor's context, its offset and a detail.
     *
     * <p>Always throws. It exists so a schema decoder written in Java refuses in exactly the words
     * the cursor's own bounds checks use -- naming the chunk and saying how far in the trouble
     * was -- rather than in a second vocabulary a reader of the message has to learn.
     *
     * @param detail what was wrong
     * @throws CnbFormatException always
     */
    public void fail(String detail) {
        Objects.requireNonNull(detail, "detail");
        CnbExtension.check("CnbReader.fail", NativeCnbRoutes.cnbReaderFail(
                open(), detail.getBytes(StandardCharsets.UTF_8)));
        throw new IllegalStateException(
                "cna_cnb_reader_fail answered success, which it documents it never does");
    }

    /** Skips forward without reading. */
    public void skip(long byteCount) {
        CnbExtension.check("CnbReader.skip", NativeCnbRoutes.cnbReaderSkip(open(), byteCount));
    }

    /**
     * Requires that nothing is left.
     *
     * <p>A schema that read everything it expected and still has bytes left has misread the
     * chunk, so this is how a reader turns "I finished" into "I finished correctly".
     */
    public void requireExhausted() {
        CnbExtension.check("CnbReader.requireExhausted",
                NativeCnbRoutes.cnbReaderRequireExhausted(open()));
    }

    /** Releases the reader. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbReader.close", NativeCnbRoutes.cnbReaderDestroy(handle));
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnbReader is closed");
            }
        }
        return handle;
    }
}
