package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.time.Duration;

/** Album discovered by a platform media library. */
public final class Album implements AutoCloseable {
    private final MediaObjectCore core;
    Album(long handle) { core = new MediaObjectCore(handle, NativeMedia.ALBUM); }
    public String getName() { return NativeMedia.getObjectName(core.value(), core.kind()); }
    public Artist getArtist() { return core.relationship(10, 0, Artist::new, Artist::releaseHandleOnly); }
    public Genre getGenre() { return core.relationship(11, 1, Genre::new, Genre::releaseHandleOnly); }
    public Duration getDuration() { return NativeMedia.duration(NativeMedia.getObjectLong(core.value(), core.kind(), 0)); }
    public boolean getHasArt() { return NativeMedia.getObjectInt(core.value(), core.kind(), 1) != 0; }
    public SongCollection getSongs() { return core.child(2, SongCollection::new); }
    public InputStream GetAlbumArt() { return new ByteArrayInputStream(NativeMedia.getObjectBlob(core.value(), core.kind(), 0)); }
    public InputStream GetThumbnail() { return new ByteArrayInputStream(NativeMedia.getObjectBlob(core.value(), core.kind(), 1)); }
    public boolean getIsDisposed() { return core.isClosed() || NativeMedia.getObjectInt(core.value(), core.kind(), 0) != 0; }
    public final boolean equals(Album other) { return other != null && (other == this || NativeMedia.objectEquals(core.value(), other.core.value(), core.kind())); }
    @Override public boolean equals(Object obj) { return obj instanceof Album other && equals(other); }
    @Override public int hashCode() { return NativeMedia.objectHash(core.value(), core.kind()); }
    @Override public String toString() { return getName(); }
    public final void Dispose() { core.close(); }
    @Override public final void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
