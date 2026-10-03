package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.util.Iterator;

/** Ordered read-only song view owned by a media library. */
public final class SongCollection implements Iterable<Song>, AutoCloseable {
    private final MediaCollectionCore<Song> core;
    SongCollection(long handle) { core = new MediaCollectionCore<>(handle, NativeMedia.SONG,
            new MediaCollectionCore.ItemFactory<>() {
                public Song create(long value) { return new Song(value); }
                public void release(Object value) { ((Song)value).releaseHandleOnly(); }
            }); }
    public int getCount() { return core.count(); }
    public Song get(int index) { return core.get(index); }
    public boolean getIsDisposed() { return core.isDisposed(); }
    public Iterator<Song> GetEnumerator() { return core.iterator(); }
    @Override public Iterator<Song> iterator() { return GetEnumerator(); }
    public void Dispose() { core.close(); }
    @Override public void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
    long nativeHandle() { return core.value(); }
}
