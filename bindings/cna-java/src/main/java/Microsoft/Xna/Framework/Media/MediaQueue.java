package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.util.LinkedHashMap;
import java.util.Map;

/** Stable facade over the process-wide media-player queue. */
public final class MediaQueue {
    private final Map<Integer, Song> songs = new LinkedHashMap<>();
    private long handle;

    MediaQueue(long handle) { this.handle = handle; }

    public int getCount() { return NativeMedia.getMediaQueueInt(value(), 0); }

    public int getActiveSongIndex() {
        return getCount() == 0 ? -1 : NativeMedia.getMediaQueueInt(value(), 1);
    }

    public void setActiveSongIndex(int value) {
        int count = getCount();
        int bounded = Math.max(0, value);
        bounded = Math.min(bounded, count - 1);
        NativeMedia.setMediaQueueIndex(value(), bounded);
    }

    public Song getActiveSong() {
        int index = getActiveSongIndex();
        return index < 0 ? null : get(index);
    }

    public synchronized Song get(int index) {
        int count = getCount();
        if (index < 0 || index >= count) {
            throw new IndexOutOfBoundsException("index " + index + " is outside [0, " + count + ")");
        }
        Song cached = songs.get(index);
        if (cached != null) return cached;
        long song = NativeMedia.getMediaQueueSong(value(), index, false);
        Song created = new Song(song);
        songs.put(index, created);
        return created;
    }

    synchronized void invalidateElements() {
        Throwable failure = null;
        Song[] values = songs.values().toArray(Song[]::new);
        for (int index = values.length - 1; index >= 0; index--) {
            try { values[index].releaseHandleOnly(); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        if (failure == null) songs.clear();
        MediaObjectCoreFailure.rethrow(failure, "MediaQueue elements");
    }

    synchronized void release() {
        if (handle == 0L) return;
        invalidateElements();
        NativeMedia.releaseMediaQueue(handle);
        handle = 0L;
    }

    private synchronized long value() {
        if (handle == 0L) throw new IllegalStateException("MediaQueue belongs to a closed Game");
        return handle;
    }
}
