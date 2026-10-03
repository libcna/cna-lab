package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.time.Instant;

/** Picture discovered by a platform media library. */
public final class Picture implements AutoCloseable {
    private final MediaObjectCore core;
    Picture(long handle) { core = new MediaObjectCore(handle, NativeMedia.PICTURE); }
    public String getName() { return NativeMedia.getObjectName(core.value(), core.kind()); }
    public PictureAlbum getAlbum() { return core.relationship(10, 0,
            PictureAlbum::new, PictureAlbum::releaseHandleOnly); }
    public Instant getDate() {
        long ticks = NativeMedia.getObjectLong(core.value(), core.kind(), 0);
        return Instant.ofEpochSecond(Math.floorDiv(ticks, 10_000_000L),
                Math.floorMod(ticks, 10_000_000L) * 100L);
    }
    public int getWidth() { return NativeMedia.getObjectInt(core.value(), core.kind(), 1); }
    public int getHeight() { return NativeMedia.getObjectInt(core.value(), core.kind(), 2); }
    public InputStream GetImage() { return new ByteArrayInputStream(NativeMedia.getObjectBlob(core.value(), core.kind(), 0)); }
    public InputStream GetThumbnail() { return new ByteArrayInputStream(NativeMedia.getObjectBlob(core.value(), core.kind(), 1)); }
    public boolean getIsDisposed() { return core.isClosed() || NativeMedia.getObjectInt(core.value(), core.kind(), 0) != 0; }
    public final boolean equals(Picture other) { return other != null && (other == this || NativeMedia.objectEquals(core.value(), other.core.value(), core.kind())); }
    @Override public boolean equals(Object obj) { return obj instanceof Picture other && equals(other); }
    @Override public int hashCode() { return NativeMedia.objectHash(core.value(), core.kind()); }
    @Override public String toString() { return getName(); }
    public final void Dispose() { core.close(); }
    @Override public final void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
