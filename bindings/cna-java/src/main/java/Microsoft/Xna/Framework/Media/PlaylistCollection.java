package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.util.Iterator;

/** Ordered read-only playlist view owned by a media library. */
public final class PlaylistCollection implements Iterable<Playlist>, AutoCloseable {
    private final MediaCollectionCore<Playlist> core;
    PlaylistCollection(long handle) { core = new MediaCollectionCore<>(handle, NativeMedia.PLAYLIST,
            new MediaCollectionCore.ItemFactory<>() {
                public Playlist create(long value) { return new Playlist(value); }
                public void release(Object value) { ((Playlist)value).releaseHandleOnly(); }
            }); }
    public int getCount() { return core.count(); }
    public Playlist get(int index) { return core.get(index); }
    public boolean getIsDisposed() { return core.isDisposed(); }
    public Iterator<Playlist> GetEnumerator() { return core.iterator(); }
    @Override public Iterator<Playlist> iterator() { return GetEnumerator(); }
    public void Dispose() { core.close(); }
    @Override public void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
