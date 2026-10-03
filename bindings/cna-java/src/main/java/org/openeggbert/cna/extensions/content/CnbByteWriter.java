package org.openeggbert.cna.extensions.content;

import java.nio.charset.StandardCharsets;
import java.util.Objects;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

/**
 * Emits {@code .cnb} primitives in the container's own encoding.
 *
 * <p>The exact counterpart of {@link CnbReader}, and the reason to use it rather than a
 * {@code ByteBuffer}: CNA decomposes every integer into individual bytes and sends every float
 * through an integer first, so the bytes produced never depend on the host's byte order or its
 * floating-point storage order. Nothing here consults the clock, a random source or a pointer
 * value, which is what makes a built {@code .cnb} byte-deterministic.
 *
 * <p>Writing the same bytes from Java would be a second encoder for one format, and the failure
 * that costs is quiet: a file this projection wrote that CNA's own reader refuses, or worse,
 * accepts and reads differently. This is CNA's encoder.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op.
 */
public final class CnbByteWriter implements AutoCloseable {

    private final long handle;
    private boolean closed;

    private CnbByteWriter(long handle) {
        this.handle = handle;
    }

    /**
     * Creates an empty writer.
     *
     * @return the writer, which the caller closes
     */
    public static CnbByteWriter create() {
        CnbExtension.requireAvailable();
        long[] created = new long[1];
        CnbExtension.check("CnbByteWriter.create", NativeCnbRoutes.cnbByteWriterCreate(created));
        return new CnbByteWriter(created[0]);
    }

    /**
     * Creates a writer that already holds these bytes, so more can be appended after them.
     *
     * @param initial the bytes to start from; CNA copies them, so the array stays the caller's
     * @return the writer, which the caller closes
     */
    public static CnbByteWriter of(byte[] initial) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(initial, "initial");
        long[] created = new long[1];
        CnbExtension.check("CnbByteWriter.of",
                NativeCnbRoutes.cnbByteWriterCreateFromBytes(initial.clone(), created));
        return new CnbByteWriter(created[0]);
    }

    /**
     * Returns how many bytes have been written so far.
     *
     * @return the size
     */
    public long size() {
        long[] value = new long[1];
        CnbExtension.check("CnbByteWriter.size",
                NativeCnbRoutes.cnbByteWriterGetSize(open(), value));
        return value[0];
    }

    /** Writes one unsigned byte. */
    public CnbByteWriter writeUnsignedByte(int value) {
        requireRange(value, 0, 0xFF, "an unsigned byte");
        CnbExtension.check("CnbByteWriter.writeUnsignedByte",
                NativeCnbRoutes.cnbByteWriterWriteU8(open(), (byte) value));
        return this;
    }

    /** Writes one unsigned 16-bit value, least significant byte first. */
    public CnbByteWriter writeUnsignedShort(int value) {
        requireRange(value, 0, 0xFFFF, "an unsigned 16-bit value");
        CnbExtension.check("CnbByteWriter.writeUnsignedShort",
                NativeCnbRoutes.cnbByteWriterWriteU16(open(), value));
        return this;
    }

    /**
     * Writes one unsigned 32-bit value.
     *
     * @param value the value, which must fit in 32 unsigned bits
     * @return this writer
     */
    public CnbByteWriter writeUnsignedInt(long value) {
        if (value < 0L || value > 0xFFFFFFFFL) {
            throw new IllegalArgumentException(
                    value + " is not an unsigned 32-bit value");
        }
        CnbExtension.check("CnbByteWriter.writeUnsignedInt",
                NativeCnbRoutes.cnbByteWriterWriteU32(open(), (int) value));
        return this;
    }

    /**
     * Writes one unsigned 64-bit value.
     *
     * <p>Java has no unsigned {@code long}, so the bits are written as given: a negative argument
     * is the same 64 bits CNA reads back as a large unsigned value, exactly as
     * {@link CnbReader#readUnsignedLong()} reports them.
     *
     * @param value the 64 bits to write
     * @return this writer
     */
    public CnbByteWriter writeUnsignedLong(long value) {
        CnbExtension.check("CnbByteWriter.writeUnsignedLong",
                NativeCnbRoutes.cnbByteWriterWriteU64(open(), value));
        return this;
    }

    /** Writes one signed 32-bit value. */
    public CnbByteWriter writeInt(int value) {
        CnbExtension.check("CnbByteWriter.writeInt",
                NativeCnbRoutes.cnbByteWriterWriteI32(open(), value));
        return this;
    }

    /** Writes one 32-bit float, through its integer bits. */
    public CnbByteWriter writeFloat(float value) {
        CnbExtension.check("CnbByteWriter.writeFloat",
                NativeCnbRoutes.cnbByteWriterWriteF32(open(), value));
        return this;
    }

    /** Writes one 64-bit float, through its integer bits. */
    public CnbByteWriter writeDouble(double value) {
        CnbExtension.check("CnbByteWriter.writeDouble",
                NativeCnbRoutes.cnbByteWriterWriteF64(open(), value));
        return this;
    }

    /**
     * Writes a length-prefixed UTF-8 string, in the form {@link CnbReader#readString()} reads.
     *
     * @param value the text
     * @return this writer
     */
    public CnbByteWriter writeString(String value) {
        Objects.requireNonNull(value, "value");
        CnbExtension.check("CnbByteWriter.writeString", NativeCnbRoutes.cnbByteWriterWriteString(
                open(), value.getBytes(StandardCharsets.UTF_8)));
        return this;
    }

    /**
     * Writes raw bytes with no length prefix.
     *
     * @param bytes the bytes; CNA copies them
     * @return this writer
     */
    public CnbByteWriter writeBytes(byte[] bytes) {
        Objects.requireNonNull(bytes, "bytes");
        CnbExtension.check("CnbByteWriter.writeBytes",
                NativeCnbRoutes.cnbByteWriterWriteBytes(open(), bytes.clone()));
        return this;
    }

    /**
     * Writes a run of zero bytes, which is how a chunk pads to an alignment.
     *
     * @param byteCount how many zeros
     * @return this writer
     */
    public CnbByteWriter writeZeros(long byteCount) {
        if (byteCount < 0L) {
            throw new IllegalArgumentException("cannot write " + byteCount + " zeros");
        }
        CnbExtension.check("CnbByteWriter.writeZeros",
                NativeCnbRoutes.cnbByteWriterWriteZeros(open(), byteCount));
        return this;
    }

    /**
     * Writes one keyframe, in the form {@link CnbReader} reads it back.
     *
     * @param keyframe the pose to write
     * @return this writer
     */
    public CnbByteWriter writeKeyframe(CnbKeyframe keyframe) {
        Objects.requireNonNull(keyframe, "keyframe");
        CnbExtension.check("CnbByteWriter.writeKeyframe",
                NativeCnbRoutes.cnbByteWriterWriteKeyframe(open(),
                        CnbKeyframes.floating(keyframe), CnbKeyframes.doubles(keyframe)));
        return this;
    }

    /**
     * Copies what has been written so far, leaving the writer holding it.
     *
     * @return the bytes
     */
    public byte[] toByteArray() {
        byte[] destination = new byte[Math.toIntExact(size())];
        long[] written = new long[1];
        CnbExtension.check("CnbByteWriter.toByteArray",
                NativeCnbRoutes.cnbByteWriterCopyBytes(open(), destination, written));
        return destination;
    }

    /**
     * Takes what has been written and empties the writer.
     *
     * <p>Different from {@link #toByteArray()} in the one way that matters to a caller building
     * several chunks from one writer: afterwards {@link #size()} is zero and the next write starts
     * a new payload.
     *
     * @return the bytes
     */
    public byte[] take() {
        byte[] destination = new byte[Math.toIntExact(size())];
        long[] written = new long[1];
        CnbExtension.check("CnbByteWriter.take",
                NativeCnbRoutes.cnbByteWriterTake(open(), destination, written));
        return destination;
    }

    /** Releases the writer. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbByteWriter.close", NativeCnbRoutes.cnbByteWriterDestroy(handle));
    }

    private static void requireRange(int value, int low, int high, String what) {
        if (value < low || value > high) {
            throw new IllegalArgumentException(value + " is not " + what);
        }
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this CnbByteWriter is closed");
        }
        return handle;
    }
}
