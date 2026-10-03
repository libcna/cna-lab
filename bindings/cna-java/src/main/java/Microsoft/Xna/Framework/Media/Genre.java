package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;

/** Genre discovered by a platform media library. */
public final class Genre implements AutoCloseable {
    private final MediaObjectCore core;
    Genre(long handle) { core = new MediaObjectCore(handle, NativeMedia.GENRE); }
    public String getName() { return NativeMedia.getObjectName(core.value(), core.kind()); }
    public AlbumCollection getAlbums() { return core.child(0, AlbumCollection::new); }
    public SongCollection getSongs() { return core.child(1, SongCollection::new); }
    public boolean getIsDisposed() { return core.isClosed() || NativeMedia.getObjectInt(core.value(), core.kind(), 0) != 0; }
    public final boolean equals(Genre other) { return other != null && (other == this || NativeMedia.objectEquals(core.value(), other.core.value(), core.kind())); }
    @Override public boolean equals(Object obj) { return obj instanceof Genre other && equals(other); }
    @Override public int hashCode() { return NativeMedia.objectHash(core.value(), core.kind()); }
    @Override public String toString() { return getName(); }
    public final void Dispose() { core.close(); }
    @Override public final void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
