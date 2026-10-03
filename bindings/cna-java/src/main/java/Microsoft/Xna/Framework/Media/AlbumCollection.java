package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.util.Iterator;

/** Ordered read-only album view owned by a media-library relationship. */
public final class AlbumCollection implements Iterable<Album>, AutoCloseable {
    private final MediaCollectionCore<Album> core;
    AlbumCollection(long handle) { core = new MediaCollectionCore<>(handle, NativeMedia.ALBUM,
            new MediaCollectionCore.ItemFactory<>() {
                public Album create(long value) { return new Album(value); }
                public void release(Object value) { ((Album)value).releaseHandleOnly(); }
            }); }
    public int getCount() { return core.count(); }
    public Album get(int index) { return core.get(index); }
    public boolean getIsDisposed() { return core.isDisposed(); }
    public Iterator<Album> GetEnumerator() { return core.iterator(); }
    @Override public Iterator<Album> iterator() { return GetEnumerator(); }
    public void Dispose() { core.close(); }
    @Override public void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
    long nativeHandle() { return core.value(); }
}
