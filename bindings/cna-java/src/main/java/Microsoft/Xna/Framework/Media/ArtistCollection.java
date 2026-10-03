package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.util.Iterator;

/** Ordered read-only artist view owned by a media library. */
public final class ArtistCollection implements Iterable<Artist>, AutoCloseable {
    private final MediaCollectionCore<Artist> core;
    ArtistCollection(long handle) { core = new MediaCollectionCore<>(handle, NativeMedia.ARTIST,
            new MediaCollectionCore.ItemFactory<>() {
                public Artist create(long value) { return new Artist(value); }
                public void release(Object value) { ((Artist)value).releaseHandleOnly(); }
            }); }
    public int getCount() { return core.count(); }
    public Artist get(int index) { return core.get(index); }
    public boolean getIsDisposed() { return core.isDisposed(); }
    public Iterator<Artist> GetEnumerator() { return core.iterator(); }
    @Override public Iterator<Artist> iterator() { return GetEnumerator(); }
    public void Dispose() { core.close(); }
    @Override public void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
