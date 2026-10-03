package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.time.Duration;

/** Playlist discovered by a platform media library. */
public final class Playlist implements AutoCloseable {
    private final MediaObjectCore core;
    Playlist(long handle) { core = new MediaObjectCore(handle, NativeMedia.PLAYLIST); }
    public String getName() { return NativeMedia.getObjectName(core.value(), core.kind()); }
    public Duration getDuration() { return NativeMedia.duration(NativeMedia.getObjectLong(core.value(), core.kind(), 0)); }
    public SongCollection getSongs() { return core.child(0, SongCollection::new); }
    public boolean getIsDisposed() { return core.isClosed() || NativeMedia.getObjectInt(core.value(), core.kind(), 0) != 0; }
    public final boolean equals(Playlist other) { return other != null && (other == this || NativeMedia.objectEquals(core.value(), other.core.value(), core.kind())); }
    @Override public boolean equals(Object obj) { return obj instanceof Playlist other && equals(other); }
    @Override public int hashCode() { return NativeMedia.objectHash(core.value(), core.kind()); }
    @Override public String toString() { return getName(); }
    public final void Dispose() { core.close(); }
    @Override public final void close() { Dispose(); }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
}
