package System.IO;

import java.io.IOException;
import java.io.OutputStream;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.Objects;

/**
 * Little-endian {@code BinaryWriter} subset, the counterpart of {@link BinaryReader}.
 *
 * <p>It exists because XNA's {@code PacketWriter} derives from the CLR's {@code BinaryWriter}
 * and inherits its whole write surface. Only the primitives XNA's networking and content
 * formats actually write are projected; the CLR type's culture, encoding and stream-ownership
 * options are not part of that contract.
 */
public class BinaryWriter implements AutoCloseable {

    private final OutputStream output;

    protected BinaryWriter(OutputStream output) {
        this.output = Objects.requireNonNull(output, "output");
    }

    public void Write(boolean value) {
        writeByte(value ? 1 : 0);
    }

    /**
     * Writes a 32-bit integer, as the CLR's {@code Write(Int32)} does.
     *
     * <p>The project maps CLR {@code Byte} to a checked Java {@code int} everywhere else, but
     * here that would collide with {@code Write(Int32)} on one Java descriptor. A single byte
     * therefore keeps the Java {@code byte} type, matching {@link BinaryReader#ReadSByte()},
     * and this overload keeps the Int32 meaning a caller reading the CLR name expects.
     */
    public void Write(int value) {
        writeBytes(littleEndian(4).putInt(value).array());
    }

    /** Writes one byte, as the CLR's {@code Write(Byte)} does. */
    public void Write(byte value) {
        writeBytes(new byte[] {value});
    }

    public void Write(short value) {
        writeBytes(littleEndian(2).putShort(value).array());
    }

    public void Write(long value) {
        writeBytes(littleEndian(8).putLong(value).array());
    }

    public void Write(float value) {
        writeBytes(littleEndian(4).putInt(Float.floatToRawIntBits(value)).array());
    }

    public void Write(double value) {
        writeBytes(littleEndian(8).putLong(Double.doubleToRawLongBits(value)).array());
    }

    public void Write(char value) {
        writeBytes(littleEndian(2).putShort((short) value).array());
    }

    /** Writes a length-prefixed UTF-8 string, using the CLR's seven-bit encoded length. */
    public void Write(String value) {
        byte[] bytes = Objects.requireNonNull(value, "value").getBytes(StandardCharsets.UTF_8);
        Write7BitEncodedInt(bytes.length);
        writeBytes(bytes);
    }

    public void Write(byte[] buffer, int index, int count) {
        Objects.checkFromIndexSize(index, count, buffer.length);
        byte[] slice = new byte[count];
        System.arraycopy(buffer, index, slice, 0, count);
        writeBytes(slice);
    }

    public void Flush() {
        try {
            output.flush();
        } catch (IOException exception) {
            throw new UncheckedIOException(exception);
        }
    }

    @Override
    public void close() {
        try {
            output.close();
        } catch (IOException exception) {
            throw new UncheckedIOException(exception);
        }
    }

    /** Writes an int32 in the CLR's seven-bit encoded form, as {@code Write(String)} needs. */
    protected final void Write7BitEncodedInt(int value) {
        int remaining = value;
        while ((remaining & ~0x7F) != 0) {
            writeByte((remaining & 0x7F) | 0x80);
            remaining >>>= 7;
        }
        writeByte(remaining & 0x7F);
    }

    private void writeByte(int value) {
        writeBytes(new byte[] {(byte) value});
    }

    /** Writes raw bytes with no length prefix. */
    protected final void writeBytes(byte[] bytes) {
        try {
            output.write(bytes);
        } catch (IOException exception) {
            throw new UncheckedIOException(exception);
        }
    }

    private static ByteBuffer littleEndian(int size) {
        return ByteBuffer.allocate(size).order(ByteOrder.LITTLE_ENDIAN);
    }
}
