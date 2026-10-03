package System.IO;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.Objects;

/** Little-endian BinaryReader subset required by XNA's ContentReader protocol. */
public class BinaryReader implements AutoCloseable {

    private final InputStream input;

    protected BinaryReader(InputStream input) {
        this.input = Objects.requireNonNull(input, "input");
    }

    public boolean ReadBoolean() { return ReadByte() != 0; }
    public int ReadByte() { return readUnsignedByte(); }
    public byte ReadSByte() { return (byte)readUnsignedByte(); }
    public short ReadInt16() { return littleEndian(2).getShort(); }
    public int ReadUInt16() { return Short.toUnsignedInt(ReadInt16()); }
    public int ReadInt32() { return littleEndian(4).getInt(); }
    public long ReadUInt32() { return Integer.toUnsignedLong(ReadInt32()); }
    public long ReadInt64() { return littleEndian(8).getLong(); }

    public long ReadUInt64() {
        long value = ReadInt64();
        if (value < 0) {
            throw new ArithmeticException("Unsigned 64-bit content value exceeds Java long");
        }
        return value;
    }

    public float ReadSingle() { return Float.intBitsToFloat(ReadInt32()); }
    public double ReadDouble() { return Double.longBitsToDouble(ReadInt64()); }
    public char ReadChar() { return (char)ReadUInt16(); }

    public String ReadString() {
        int length = Read7BitEncodedInt();
        if (length < 0) {
            throw new IllegalStateException("BinaryReader string length is negative");
        }
        return new String(ReadBytesExact(length), StandardCharsets.UTF_8);
    }

    public int Read(byte[] destination, int offset, int count) {
        Objects.checkFromIndexSize(offset, count, destination.length);
        try {
            return input.read(destination, offset, count);
        } catch (IOException exception) {
            throw new UncheckedIOException(exception);
        }
    }

    protected final int Read7BitEncodedInt() {
        int result = 0;
        for (int shift = 0; shift < 35; shift += 7) {
            int value = ReadByte();
            result |= (value & 0x7f) << shift;
            if ((value & 0x80) == 0) {
                return result;
            }
        }
        throw new IllegalStateException("Invalid 7-bit encoded integer");
    }

    protected final byte[] ReadBytesExact(int count) {
        if (count < 0) {
            throw new IllegalArgumentException("Byte count must not be negative");
        }
        byte[] output = new byte[count];
        int offset = 0;
        try {
            while (offset < count) {
                int read = input.read(output, offset, count - offset);
                if (read < 0) {
                    throw new EOFException("Unexpected end of content stream");
                }
                offset += read;
            }
            return output;
        } catch (IOException exception) {
            throw new UncheckedIOException(exception);
        }
    }

    @Override
    public void close() {
        try {
            input.close();
        } catch (IOException exception) {
            throw new UncheckedIOException(exception);
        }
    }

    private int readUnsignedByte() {
        try {
            int value = input.read();
            if (value < 0) {
                throw new EOFException("Unexpected end of content stream");
            }
            return value;
        } catch (IOException exception) {
            throw new UncheckedIOException(exception);
        }
    }

    private ByteBuffer littleEndian(int count) {
        return ByteBuffer.wrap(ReadBytesExact(count)).order(ByteOrder.LITTLE_ENDIAN);
    }

}
