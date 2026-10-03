package org.openeggbert.cna.internal;

import System.IO.SeekOrigin;
import System.IO.Stream;

import java.io.IOException;
import java.util.Objects;

/** Owned native storage-stream implementation hidden behind System.IO.Stream. */
public final class NativeStorageStream extends Stream {
    private final long identity;
    private final Runnable released;
    private long handle;

    public NativeStorageStream(long handle, Runnable released) {
        if (handle == 0L) throw new IllegalArgumentException("Storage stream handle is zero");
        this.handle = handle;
        this.identity = handle;
        this.released = Objects.requireNonNull(released, "released");
    }

    @Override public boolean getCanRead() {
        return NativeStorage.getStreamCapability(requireHandle(), 0);
    }
    @Override public boolean getCanWrite() {
        return NativeStorage.getStreamCapability(requireHandle(), 1);
    }
    @Override public boolean getCanSeek() {
        return NativeStorage.getStreamCapability(requireHandle(), 2);
    }
    @Override public long getLength() { return NativeStorage.getStreamLong(requireHandle(), 1); }
    @Override public long getPosition() { return NativeStorage.getStreamLong(requireHandle(), 0); }
    @Override public void setPosition(long value) { seek(value, SeekOrigin.Begin); }

    @Override public int read() throws IOException {
        byte[] value = new byte[1];
        int read = read(value, 0, 1);
        return read == 0 ? -1 : value[0] & 0xff;
    }

    @Override public int read(byte[] buffer, int offset, int count) throws IOException {
        Objects.requireNonNull(buffer, "buffer");
        Objects.checkFromIndexSize(offset, count, buffer.length);
        if (count == 0) return 0;
        return NativeStorage.readStream(requireHandle(), buffer, offset, count);
    }

    @Override public void write(byte[] buffer, int offset, int count) throws IOException {
        Objects.requireNonNull(buffer, "buffer");
        Objects.checkFromIndexSize(offset, count, buffer.length);
        if (count != 0) NativeStorage.writeStream(requireHandle(), buffer, offset, count);
    }

    @Override public void flush() throws IOException {
        NativeStorage.flushStream(requireHandle());
    }

    @Override public long seek(long offset, SeekOrigin origin) {
        return NativeStorage.seekStream(requireHandle(), offset,
                Objects.requireNonNull(origin, "origin").ordinal());
    }

    @Override public void setLength(long value) {
        if (value < 0L) throw new IllegalArgumentException("value");
        NativeStorage.setStreamLength(requireHandle(), value);
    }

    @Override public synchronized void close() {
        if (handle == 0L) return;
        NativeStorage.closeStream(handle);
        handle = 0L;
        released.run();
    }

    public boolean matchesReleasedHandle(long value) { return identity == value; }

    private synchronized long requireHandle() {
        if (handle == 0L) throw new IllegalStateException("Storage stream is already closed");
        return handle;
    }
}
