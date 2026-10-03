package Microsoft.Xna.Framework.Media;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeMedia;

import java.time.Duration;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Process-global XNA media player backed by CNA's one native player and queue. */
public final class MediaPlayer {
    private static final CopyOnWriteArrayList<EventHandler<EventArgs>> ACTIVE_LISTENERS =
            new CopyOnWriteArrayList<>();
    private static final CopyOnWriteArrayList<EventHandler<EventArgs>> STATE_LISTENERS =
            new CopyOnWriteArrayList<>();
    private static boolean eventsSubscribed;
    private static MediaQueue queue;

    private MediaPlayer() { }

    public static void addActiveSongChangedListener(EventHandler<EventArgs> listener) {
        EventHandler<EventArgs> selected = Objects.requireNonNull(listener, "listener");
        subscribeEvents();
        ACTIVE_LISTENERS.add(selected);
    }

    public static void removeActiveSongChangedListener(EventHandler<EventArgs> listener) {
        ACTIVE_LISTENERS.remove(listener);
    }

    public static void addMediaStateChangedListener(EventHandler<EventArgs> listener) {
        EventHandler<EventArgs> selected = Objects.requireNonNull(listener, "listener");
        subscribeEvents();
        STATE_LISTENERS.add(selected);
    }

    public static void removeMediaStateChangedListener(EventHandler<EventArgs> listener) {
        STATE_LISTENERS.remove(listener);
    }

    public static MediaState getState() { return MediaState.fromValue(NativeMedia.getMediaPlayerInt(4)); }
    public static boolean getGameHasControl() { return NativeMedia.getMediaPlayerInt(0) != 0; }
    public static boolean getIsMuted() { return NativeMedia.getMediaPlayerInt(1) != 0; }
    public static void setIsMuted(boolean value) { NativeMedia.setMediaPlayerInt(1, value ? 1 : 0); }
    public static boolean getIsRepeating() { return NativeMedia.getMediaPlayerInt(2) != 0; }
    public static void setIsRepeating(boolean value) { NativeMedia.setMediaPlayerInt(2, value ? 1 : 0); }
    public static boolean getIsShuffled() { return NativeMedia.getMediaPlayerInt(3) != 0; }
    public static void setIsShuffled(boolean value) { NativeMedia.setMediaPlayerInt(3, value ? 1 : 0); }
    public static boolean getIsVisualizationEnabled() { return NativeMedia.getMediaPlayerInt(5) != 0; }
    public static void setIsVisualizationEnabled(boolean value) { NativeMedia.setMediaPlayerInt(5, value ? 1 : 0); }
    public static Duration getPlayPosition() { return NativeMedia.duration(NativeMedia.getMediaPlayerLong(0)); }
    public static float getVolume() { return NativeMedia.getMediaPlayerFloat(); }
    public static void setVolume(float value) {
        float clamped = value < 0f ? 0f : value;
        clamped = clamped > 1f ? 1f : clamped;
        NativeMedia.setMediaPlayerFloat(clamped);
    }

    public static synchronized MediaQueue getQueue() {
        if (queue == null) queue = new MediaQueue(NativeMedia.getMediaQueue());
        return queue;
    }

    public static void Play(Song song) {
        Song selected = Objects.requireNonNull(song, "song");
        NativeMedia.mediaPlayerOperation(0, selected.nativeHandle(), 0);
        invalidateQueue();
    }

    public static void Play(SongCollection songs) {
        SongCollection selected = Objects.requireNonNull(songs, "songs");
        if (selected.getCount() == 0) throw new IllegalArgumentException("songs must not be empty");
        NativeMedia.mediaPlayerOperation(1, selected.nativeHandle(), 0);
        invalidateQueue();
    }

    public static void Play(SongCollection songs, int index) {
        SongCollection selected = Objects.requireNonNull(songs, "songs");
        int count = selected.getCount();
        if (count == 0) throw new IllegalArgumentException("songs must not be empty");
        if (index < 0 || index >= count) throw new IndexOutOfBoundsException("index");
        NativeMedia.mediaPlayerOperation(2, selected.nativeHandle(), index);
        invalidateQueue();
    }

    public static void Pause() { NativeMedia.mediaPlayerOperation(3, 0L, 0); }
    public static void Resume() { NativeMedia.mediaPlayerOperation(4, 0L, 0); }
    public static void Stop() { NativeMedia.mediaPlayerOperation(5, 0L, 0); }
    public static void MoveNext() { NativeMedia.mediaPlayerOperation(6, 0L, 0); }
    public static void MovePrevious() { NativeMedia.mediaPlayerOperation(7, 0L, 0); }

    public static void GetVisualizationData(VisualizationData visualizationData) {
        NativeMedia.getVisualizationData(Objects.requireNonNull(
                visualizationData, "visualizationData"));
    }

    static void dispatchNativeEvent(int kind) {
        CopyOnWriteArrayList<EventHandler<EventArgs>> listeners =
                kind == 0 ? ACTIVE_LISTENERS : STATE_LISTENERS;
        Throwable failure = null;
        for (EventHandler<EventArgs> listener : listeners) {
            try { listener.invoke(null, EventArgs.Empty); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        MediaObjectCoreFailure.rethrow(failure, "MediaPlayer event callback");
    }

    static synchronized void releaseGameScopedState() {
        if (queue != null) {
            queue.release();
            queue = null;
        }
    }

    private static synchronized void subscribeEvents() {
        if (!eventsSubscribed) {
            NativeMedia.subscribeEvents();
            eventsSubscribed = true;
        }
    }

    private static synchronized void invalidateQueue() {
        if (queue != null) queue.invalidateElements();
    }
}
