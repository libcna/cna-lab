package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import System.IO.BinaryWriter;

import java.io.ByteArrayOutputStream;
import java.util.Objects;

/**
 * Builds one outgoing network packet.
 *
 * <p>XNA's writer is a managed {@code BinaryWriter} over a growable buffer, and so is this one:
 * the packet is assembled in Java and crosses the boundary once, when
 * {@link LocalNetworkGamer#SendData(PacketWriter, SendDataOptions)} hands it to CNA. Keeping
 * the buffer managed is what makes {@code Position} and {@code Length} mean exactly what they
 * mean in XNA, with one position rather than a Java one and a native one to keep in step.
 *
 * <p>The value overloads write in XNA's own order and layout: a colour as its packed RGBA, a
 * vector component by component, a matrix in {@code M11..M44} order.
 */
public class PacketWriter extends BinaryWriter {

    private final Buffer buffer;

    public PacketWriter() {
        this(0);
    }

    public PacketWriter(int capacity) {
        this(new Buffer(capacity));
    }

    private PacketWriter(Buffer buffer) {
        super(buffer);
        this.buffer = buffer;
    }

    public final void Write(Color value) {
        Objects.requireNonNull(value, "value");
        // XNA writes the packed RGBA as one UInt32. The project maps CLR UInt32 to a Java long
        // so the whole range is representable, so the four bytes are written explicitly.
        Write(value.getPackedValue().intValue());
    }

    public final void Write(Matrix value) {
        Objects.requireNonNull(value, "value");
        Write(value.M11);
        Write(value.M12);
        Write(value.M13);
        Write(value.M14);
        Write(value.M21);
        Write(value.M22);
        Write(value.M23);
        Write(value.M24);
        Write(value.M31);
        Write(value.M32);
        Write(value.M33);
        Write(value.M34);
        Write(value.M41);
        Write(value.M42);
        Write(value.M43);
        Write(value.M44);
    }

    public final void Write(Quaternion value) {
        Objects.requireNonNull(value, "value");
        Write(value.X);
        Write(value.Y);
        Write(value.Z);
        Write(value.W);
    }

    public final void Write(Vector2 value) {
        Objects.requireNonNull(value, "value");
        Write(value.X);
        Write(value.Y);
    }

    public final void Write(Vector3 value) {
        Objects.requireNonNull(value, "value");
        Write(value.X);
        Write(value.Y);
        Write(value.Z);
    }

    public final void Write(Vector4 value) {
        Objects.requireNonNull(value, "value");
        Write(value.X);
        Write(value.Y);
        Write(value.Z);
        Write(value.W);
    }

    @Override
    public void Write(double value) {
        super.Write(value);
    }

    @Override
    public void Write(float value) {
        super.Write(value);
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

    /** Returns the bytes written so far. Used when the packet crosses to CNA. */
    final byte[] data() {
        return buffer.data();
    }

    final void reset() {
        buffer.reset();
    }

    /**
     * The writer's growable buffer.
     *
     * <p>It is an {@code OutputStream} so {@code BinaryWriter} can write through it, and it
     * supports seeking because XNA's {@code Position} is settable: writing after a seek
     * overwrites in place rather than appending.
     */
    private static final class Buffer extends ByteArrayOutputStream {

        private int position;

        Buffer(int capacity) {
            super(Math.max(capacity, 32));
        }

        @Override
        public synchronized void write(int value) {
            if (position < count) {
                buf[position++] = (byte) value;
                return;
            }
            super.write(value);
            position = count;
        }

        @Override
        public synchronized void write(byte[] bytes, int offset, int length) {
            for (int index = 0; index < length; index++) {
                write(bytes[offset + index]);
            }
        }

        synchronized int length() {
            return count;
        }

        synchronized int position() {
            return position;
        }

        synchronized void position(int value) {
            if (value < 0 || value > count) {
                throw new IndexOutOfBoundsException(
                        "position " + value + ", length " + count);
            }
            position = value;
        }

        synchronized byte[] data() {
            return toByteArray();
        }

        @Override
        public synchronized void reset() {
            super.reset();
            position = 0;
        }
    }
}
