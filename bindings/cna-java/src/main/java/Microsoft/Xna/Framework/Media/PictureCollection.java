package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.util.Iterator;

/** Ordered read-only picture view owned by a media library. */
public final class PictureCollection implements Iterable<Picture>, AutoCloseable {
    private final MediaCollectionCore<Picture> core;
    PictureCollection(long handle) { core = new MediaCollectionCore<>(handle, NativeMedia.PICTURE,
            new MediaCollectionCore.ItemFactory<>() {
                public Picture create(long value) { return new Picture(value); }
                public void release(Object value) { ((Picture)value).releaseHandleOnly(); }
            }); }
    public int getCount() { return core.count(); }
    public Picture get(int index) { return core.get(index); }
    public boolean getIsDisposed() { return core.isDisposed(); }
    public Iterator<Picture> GetEnumerator() { return core.iterator(); }
    @Override public Iterator<Picture> iterator() { return GetEnumerator(); }
    public void Dispose() { core.close(); }
    @Override public void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
