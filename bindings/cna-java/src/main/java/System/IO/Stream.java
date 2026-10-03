package System.IO;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Objects;

/** Java stream carrier used where the strict XNA signature names System.IO.Stream. */
public class Stream extends FilterInputStream {
    public Stream(InputStream input) {
        super(Objects.requireNonNull(input, "input"));
    }

    /** Constructor for native read/write stream implementations. */
    protected Stream() { super(InputStream.nullInputStream()); }

    public boolean getCanRead() { return true; }
    public boolean getCanWrite() { return false; }
    public boolean getCanSeek() { return false; }
    public long getLength() { throw new UnsupportedOperationException("Stream is not seekable"); }
    public long getPosition() { throw new UnsupportedOperationException("Stream is not seekable"); }
    public void setPosition(long value) { seek(value, SeekOrigin.Begin); }
    public void write(int value) throws IOException {
        write(new byte[]{(byte)value}, 0, 1);
    }
    public void write(byte[] buffer) throws IOException {
        Objects.requireNonNull(buffer, "buffer");
        write(buffer, 0, buffer.length);
    }
    public void write(byte[] buffer, int offset, int count) throws IOException {
        throw new UnsupportedOperationException("Stream is not writable");
    }
    public void flush() throws IOException { }
    public long seek(long offset, SeekOrigin origin) {
        throw new UnsupportedOperationException("Stream is not seekable");
    }
    public void setLength(long value) {
        throw new UnsupportedOperationException("Stream is not seekable");
    }
}
