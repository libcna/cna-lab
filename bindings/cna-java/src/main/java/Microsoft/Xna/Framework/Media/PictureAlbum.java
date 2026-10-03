package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;

/** Node in a media library's picture-album tree. */
public final class PictureAlbum implements AutoCloseable {
    private final MediaObjectCore core;
    PictureAlbum(long handle) { core = new MediaObjectCore(handle, NativeMedia.PICTURE_ALBUM); }
    public String getName() { return NativeMedia.getObjectName(core.value(), core.kind()); }
    public PictureAlbum getParent() { return core.relationship(10, 0,
            PictureAlbum::new, PictureAlbum::releaseHandleOnly); }
    public PictureAlbumCollection getAlbums() { return core.child(1, PictureAlbumCollection::new); }
    public PictureCollection getPictures() { return core.child(2, PictureCollection::new); }
    public boolean getIsDisposed() { return core.isClosed() || NativeMedia.getObjectInt(core.value(), core.kind(), 0) != 0; }
    public final boolean equals(PictureAlbum other) { return other != null && (other == this || NativeMedia.objectEquals(core.value(), other.core.value(), core.kind())); }
    @Override public boolean equals(Object obj) { return obj instanceof PictureAlbum other && equals(other); }
    @Override public int hashCode() { return NativeMedia.objectHash(core.value(), core.kind()); }
    @Override public String toString() { return getName(); }
    public final void Dispose() { core.close(); }
    @Override public final void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
