package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import System.IO.BinaryReader;

import java.io.InputStream;

/**
 * Reads one received network packet.
 *
 * <p>The counterpart of {@link PacketWriter}, and managed for the same reason: XNA's reader is
 * a {@code BinaryReader} over the received bytes, and keeping it managed gives {@code Position}
 * and {@code Length} exactly XNA's meaning over one cursor. The bytes arrive from CNA once,
 * when {@link LocalNetworkGamer#ReceiveData(PacketReader)} fills the reader.
 *
 * <p>The value overloads read what {@link PacketWriter} wrote, in the same order.
 */
public class PacketReader extends BinaryReader {

    private final Buffer buffer;

    public PacketReader() {
        this(0);
    }

    public PacketReader(int capacity) {
        this(new Buffer(capacity));
    }

    private PacketReader(Buffer buffer) {
        super(buffer);
        this.buffer = buffer;
    }

    public final Color ReadColor() {
        return new Color(ReadByte(), ReadByte(), ReadByte(), ReadByte());
    }

    @Override
    public double ReadDouble() {
        return super.ReadDouble();
    }

    public final Matrix ReadMatrix() {
        Matrix value = new Matrix();
        value.M11 = ReadSingle();
        value.M12 = ReadSingle();
        value.M13 = ReadSingle();
        value.M14 = ReadSingle();
        value.M21 = ReadSingle();
        value.M22 = ReadSingle();
        value.M23 = ReadSingle();
        value.M24 = ReadSingle();
        value.M31 = ReadSingle();
        value.M32 = ReadSingle();
        value.M33 = ReadSingle();
        value.M34 = ReadSingle();
        value.M41 = ReadSingle();
        value.M42 = ReadSingle();
        value.M43 = ReadSingle();
        value.M44 = ReadSingle();
        return value;
    }

    public final Quaternion ReadQuaternion() {
        return new Quaternion(ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle());
    }

    @Override
    public float ReadSingle() {
        return super.ReadSingle();
    }

    public final Vector2 ReadVector2() {
        return new Vector2(ReadSingle(), ReadSingle());
    }

    public final Vector3 ReadVector3() {
        return new Vector3(ReadSingle(), ReadSingle(), ReadSingle());
    }

    public final Vector4 ReadVector4() {
        return new Vector4(ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle());
    }

    public final int getLength() {
        return buffer.length();
    }

    public final int getPosition() {
        return buffer.position();
    }

    public final void setPosition(int value) {
        buffer.position(value);
    }

    /** Replaces the reader's contents with a freshly received packet. */
    final void fill(byte[] data, int length) {
        buffer.fill(data, length);
    }

    /** The reader's seekable byte source; XNA's {@code Position} is settable. */
    private static final class Buffer extends InputStream {

        private byte[] data;
        private int length;
        private int position;

        Buffer(int capacity) {
            data = new byte[Math.max(capacity, 32)];
        }

        @Override
        public synchronized int read() {
            return position < length ? data[position++] & 0xFF : -1;
        }

        @Override
        public synchronized int read(byte[] destination, int offset, int count) {
            if (position >= length) {
                return -1;
            }
            int copied = Math.min(count, length - position);
            System.arraycopy(data, position, destination, offset, copied);
            position += copied;
            return copied;
        }

        synchronized void fill(byte[] source, int size) {
            if (data.length < size) {
                data = new byte[size];
            }
            System.arraycopy(source, 0, data, 0, size);
            length = size;
            position = 0;
        }

        synchronized int length() {
            return length;
        }

        synchronized int position() {
            return position;
        }

        synchronized void position(int value) {
            if (value < 0 || value > length) {
                throw new IndexOutOfBoundsException("position " + value + ", length " + length);
            }
            position = value;
        }
    }
}
