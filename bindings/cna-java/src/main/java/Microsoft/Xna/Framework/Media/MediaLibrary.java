package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.io.IOException;
import java.io.InputStream;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;

/** Platform media-library facade; empty collections are valid on headless Linux. */
public final class MediaLibrary implements AutoCloseable {
    private final Map<Integer, ChildEntry<?>> children = new LinkedHashMap<>();
    private long handle;
    private int nextTransientChild = 1000;
    private MediaSource mediaSource;

    public MediaLibrary() {
        handle = NativeMedia.createLibrary(-1);
        NativeMedia.registerMediaOwner(this);
    }

    public MediaLibrary(MediaSource mediaSource) {
        MediaSource selected = Objects.requireNonNull(mediaSource, "mediaSource");
        if (selected.getMediaSourceType() != MediaSourceType.LocalDevice) {
            throw new UnsupportedOperationException(
                    "XNA MediaLibrary accepts only a LocalDevice media source");
        }
        handle = NativeMedia.createLibrary(selected.index());
        this.mediaSource = selected;
        NativeMedia.registerMediaOwner(this);
    }

    public SongCollection getSongs() { return child(0, SongCollection::new); }
    public AlbumCollection getAlbums() { return child(1, AlbumCollection::new); }
    public ArtistCollection getArtists() { return child(2, ArtistCollection::new); }
    public GenreCollection getGenres() { return child(3, GenreCollection::new); }
    public PlaylistCollection getPlaylists() { return child(4, PlaylistCollection::new); }
    public PictureCollection getPictures() { return child(5, PictureCollection::new); }
    public PictureCollection getSavedPictures() { return child(6, PictureCollection::new); }

    public PictureAlbum getRootPictureAlbum() {
        @SuppressWarnings("unchecked")
        ChildEntry<PictureAlbum> existing = (ChildEntry<PictureAlbum>)children.get(7);
        if (existing != null) return existing.value();
        long value = NativeMedia.getLibraryChild(value(), 7);
        if (value == 0L) return null;
        PictureAlbum created = new PictureAlbum(value);
        children.put(7, new ChildEntry<>(created, created::releaseHandleOnly));
        return created;
    }

    public MediaSource getMediaSource() {
        value();
        if (mediaSource == null) {
            mediaSource = new MediaSource(-1,
                    MediaSourceType.fromValue(NativeMedia.getLibrarySourceType(value())),
                    NativeMedia.getLibrarySourceName(value()));
        }
        return mediaSource;
    }

    public boolean getIsDisposed() {
        return handle == 0L || NativeMedia.getLibraryIsDisposed(handle);
    }

    public Picture GetPictureFromToken(String token) {
        long value = NativeMedia.getLibraryPicture(value(), Objects.requireNonNull(token, "token"));
        return value == 0L ? null : track(new Picture(value));
    }

    public Picture SavePicture(String name, int[] imageBuffer) {
        value();
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(imageBuffer, "imageBuffer");
        byte[] bytes = new byte[imageBuffer.length];
        for (int index = 0; index < imageBuffer.length; index++) {
            int value = imageBuffer[index];
            if ((value & ~0xff) != 0) {
                throw new IllegalArgumentException("imageBuffer contains a non-byte value");
            }
            bytes[index] = (byte)value;
        }
        return save(name, bytes);
    }

    public Picture SavePicture(String name, InputStream source) {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(source, "source");
        value();
        try { return save(name, source.readAllBytes()); }
        catch (IOException exception) {
            throw new IllegalStateException("Could not read the picture source", exception);
        }
    }

    public final synchronized void Dispose() {
        if (handle == 0L) return;
        Throwable failure = null;
        AutoCloseable[] values = children.values().stream()
                .map(ChildEntry::cleanup).toArray(AutoCloseable[]::new);
        for (int index = values.length - 1; index >= 0; index--) {
            try { values[index].close(); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        if (failure == null) {
            try {
                NativeMedia.closeLibrary(handle);
                handle = 0L;
                children.clear();
                NativeMedia.unregisterMediaOwner(this);
            }
            catch (Throwable exception) { failure = exception; }
        }
        MediaObjectCoreFailure.rethrow(failure, "MediaLibrary");
    }

    @Override
    public final void close() {
        Dispose();
    }

    private Picture save(String name, byte[] bytes) {
        Objects.requireNonNull(name, "name");
        value();
        return track(new Picture(NativeMedia.saveLibraryPicture(handle, name, bytes)));
    }

    private synchronized <T extends AutoCloseable> T child(int relation, ChildFactory<T> factory) {
        @SuppressWarnings("unchecked") ChildEntry<T> existing =
                (ChildEntry<T>)children.get(relation);
        if (existing != null) return existing.value();
        T created = factory.create(NativeMedia.getLibraryChild(value(), relation));
        children.put(relation, new ChildEntry<>(created, created));
        return created;
    }

    private synchronized Picture track(Picture picture) {
        children.put(nextTransientChild++, new ChildEntry<>(picture, picture::releaseHandleOnly));
        return picture;
    }

    private synchronized long value() {
        if (handle == 0L) throw new IllegalStateException("MediaLibrary is already closed");
        return handle;
    }

    @FunctionalInterface private interface ChildFactory<T> { T create(long handle); }
    private record ChildEntry<T>(T value, AutoCloseable cleanup) { }
}
