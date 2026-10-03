package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.util.Iterator;

/** Ordered read-only picture-album view owned by a media library. */
public final class PictureAlbumCollection implements Iterable<PictureAlbum>, AutoCloseable {
    private final MediaCollectionCore<PictureAlbum> core;
    PictureAlbumCollection(long handle) { core = new MediaCollectionCore<>(handle,
            NativeMedia.PICTURE_ALBUM, new MediaCollectionCore.ItemFactory<>() {
                public PictureAlbum create(long value) { return new PictureAlbum(value); }
                public void release(Object value) { ((PictureAlbum)value).releaseHandleOnly(); }
            }); }
    public int getCount() { return core.count(); }
    public PictureAlbum get(int index) { return core.get(index); }
    public boolean getIsDisposed() { return core.isDisposed(); }
    public Iterator<PictureAlbum> GetEnumerator() { return core.iterator(); }
    @Override public Iterator<PictureAlbum> iterator() { return GetEnumerator(); }
    public void Dispose() { core.close(); }
    @Override public void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
