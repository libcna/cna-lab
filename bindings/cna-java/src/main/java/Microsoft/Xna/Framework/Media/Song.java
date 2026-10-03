package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.net.URI;
import java.time.Duration;
import java.util.Objects;

/** Playable song from a media library or a local file URI. */
public final class Song implements AutoCloseable {
    private final MediaObjectCore core;
    Song(long handle) { core = new MediaObjectCore(handle, NativeMedia.SONG); }
    public static Song FromUri(String name, URI uri) {
        Song song = new Song(NativeMedia.createSong(Objects.requireNonNull(name, "name"),
                Objects.requireNonNull(uri, "uri").toString()));
        NativeMedia.registerMediaOwner(song);
        return song;
    }
    public String getName() { return NativeMedia.getObjectName(core.value(), core.kind()); }
    public Album getAlbum() { return core.relationship(10, 0, Album::new, Album::releaseHandleOnly); }
    public Artist getArtist() { return core.relationship(11, 1, Artist::new, Artist::releaseHandleOnly); }
    public Genre getGenre() { return core.relationship(12, 2, Genre::new, Genre::releaseHandleOnly); }
    public Duration getDuration() { return NativeMedia.duration(NativeMedia.getObjectLong(core.value(), core.kind(), 0)); }
    public boolean getIsProtected() { return NativeMedia.getObjectInt(core.value(), core.kind(), 1) != 0; }
    public boolean getIsRated() { return NativeMedia.getObjectInt(core.value(), core.kind(), 2) != 0; }
    public int getPlayCount() { return NativeMedia.getObjectInt(core.value(), core.kind(), 3); }
    public int getRating() { return NativeMedia.getObjectInt(core.value(), core.kind(), 4); }
    public int getTrackNumber() { return NativeMedia.getObjectInt(core.value(), core.kind(), 5); }
    public boolean getIsDisposed() { return core.isClosed() || NativeMedia.getObjectInt(core.value(), core.kind(), 0) != 0; }
    public final boolean equals(Song other) { return other != null && (other == this || NativeMedia.objectEquals(core.value(), other.core.value(), core.kind())); }
    @Override public boolean equals(Object obj) { return obj instanceof Song other && equals(other); }
    @Override public int hashCode() { return NativeMedia.objectHash(core.value(), core.kind()); }
    @Override public String toString() { return getName(); }
    public final void Dispose() {
        core.close();
        NativeMedia.unregisterMediaOwner(this);
    }

    @Override
    public final void close() {
        Dispose();
    }
    void releaseHandleOnly() { core.releaseHandleOnly(); }
    long nativeHandle() { return core.value(); }
}
