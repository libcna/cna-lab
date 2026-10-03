package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.Media.VisualizationData;
import Microsoft.Xna.Framework.Media.Video;

import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.ArrayDeque;
import java.util.Collections;
import java.util.Deque;
import java.util.IdentityHashMap;
import java.util.Set;
import java.util.WeakHashMap;
import java.util.Map;

/** Internal JNI surface for CNA ABI 0.7 media-library, media-player, and video routes. */
public final class NativeMedia {
    public static final int ALBUM = 0;
    public static final int ARTIST = 1;
    public static final int GENRE = 2;
    public static final int PICTURE = 3;
    public static final int PICTURE_ALBUM = 4;
    public static final int PLAYLIST = 5;
    public static final int SONG = 6;

    public static final int NAME = 0;
    public static final int IS_DISPOSED = 0;

    private static final Deque<Integer> PENDING_EVENTS = new ArrayDeque<>();
    private static final Set<AutoCloseable> VIDEO_PLAYERS = Collections.newSetFromMap(
            new IdentityHashMap<>());
    private static final Set<AutoCloseable> MEDIA_OWNERS = Collections.newSetFromMap(
            new IdentityHashMap<>());
    private static final Map<Video, NativeHandle> VIDEOS = new WeakHashMap<>();
    private static volatile boolean acceptEvents = true;

    private NativeMedia() {
    }

    public static long game(String operation) {
        acceptEvents = true;
        return NativeBindings.currentGameValue(operation);
    }

    public static void beginGameLifetime() {
        synchronized (PENDING_EVENTS) {
            PENDING_EVENTS.clear();
            acceptEvents = true;
        }
    }

    public static Duration duration(long ticks) {
        long seconds = ticks / 10_000_000L;
        long nanos = (ticks % 10_000_000L) * 100L;
        return Duration.ofSeconds(seconds, nanos);
    }

    public static long ticks(Duration duration) {
        return Math.addExact(Math.multiplyExact(duration.getSeconds(), 10_000_000L),
                duration.getNano() / 100L);
    }

    public static byte[] utf8(String value) {
        return value.getBytes(StandardCharsets.UTF_8);
    }

    public static void check(String operation, int result) {
        if (result != 0) throw NativeBindings.failure(operation, result);
    }

    public static long handle(long[] output, String operation) {
        if (output[0] == 0L) throw NativeBindings.failure(operation, -1);
        return output[0];
    }

    public static long createLibrary(int sourceIndex) {
        long[] output = new long[1];
        check(sourceIndex < 0 ? "cna_media_library_create"
                        : "cna_media_library_create_from_source",
                nativeCreateLibrary(game("MediaLibrary"), sourceIndex, output));
        return handle(output, "cna_media_library_create");
    }

    public static int getMediaSourceCount() {
        int[] output = new int[1];
        check("cna_media_source_get_available_count", nativeGetMediaSourceCount(
                game("MediaSource.GetAvailableMediaSources"), output));
        return output[0];
    }

    public static int getMediaSourceType(int index) {
        int[] output = new int[1];
        check("cna_media_source_get_type_at", nativeGetMediaSourceType(
                game("MediaSource.GetAvailableMediaSources"), index, output));
        return output[0];
    }

    public static String getMediaSourceName(int index) {
        return string(nativeGetMediaSourceName(
                game("MediaSource.GetAvailableMediaSources"), index));
    }

    public static boolean getLibraryIsDisposed(long library) {
        int[] output = new int[1];
        check("cna_media_library_get_is_disposed", nativeGetLibraryInt(library, 0, output));
        return output[0] != 0;
    }

    public static int getLibrarySourceType(long library) {
        int[] output = new int[1];
        check("cna_media_library_get_media_source_type", nativeGetLibraryInt(library, 1, output));
        return output[0];
    }

    public static String getLibrarySourceName(long library) {
        return string(nativeGetLibraryString(library));
    }

    public static long getLibraryChild(long library, int relation) {
        long[] output = new long[1];
        int[] available = new int[1];
        check("cna_media_library child", nativeGetLibraryChild(library, relation, null,
                output, available));
        return available[0] == 0 ? 0L : handle(output, "cna_media_library child");
    }

    public static long getLibraryPicture(long library, String token) {
        long[] output = new long[1];
        int[] available = new int[1];
        check("cna_media_library_get_picture_from_token", nativeGetLibraryChild(
                library, 9, utf8(token), output, available));
        return available[0] == 0 ? 0L : handle(output,
                "cna_media_library_get_picture_from_token");
    }

    public static long saveLibraryPicture(long library, String name, byte[] bytes) {
        long[] output = new long[1];
        check("cna_media_library_save_picture", nativeSaveLibraryPicture(
                library, utf8(name), bytes, output));
        return handle(output, "cna_media_library_save_picture");
    }

    public static void closeLibrary(long library) {
        check("cna_media_library_dispose", nativeCloseLibrary(library, false));
        check("cna_media_library_destroy", nativeCloseLibrary(library, true));
    }

    public static int getCollectionCount(long collection, int kind) {
        int[] output = new int[1];
        check("media collection count", nativeGetCollectionInt(collection, kind, 0, output));
        return output[0];
    }

    public static boolean getCollectionIsDisposed(long collection, int kind) {
        int[] output = new int[1];
        check("media collection disposed", nativeGetCollectionInt(collection, kind, 1, output));
        return output[0] != 0;
    }

    public static long getCollectionAt(long collection, int kind, int index) {
        long[] output = new long[1];
        check("media collection index", nativeGetCollectionAt(collection, kind, index, output));
        return handle(output, "media collection index");
    }

    public static void closeCollection(long collection, int kind) {
        check("media collection dispose", nativeCloseCollection(collection, kind, false));
        check("media collection destroy", nativeCloseCollection(collection, kind, true));
    }

    public static void releaseCollection(long collection, int kind) {
        check("media collection destroy", nativeCloseCollection(collection, kind, true));
    }

    public static String getObjectName(long object, int kind) {
        return string(nativeGetObjectString(object, kind, NAME));
    }

    public static int getObjectInt(long object, int kind, int property) {
        int[] output = new int[1];
        check("media object property", nativeGetObjectInt(object, kind, property, output));
        return output[0];
    }

    public static long getObjectLong(long object, int kind, int property) {
        long[] output = new long[1];
        check("media object property", nativeGetObjectLong(object, kind, property, output));
        return output[0];
    }

    public static long getObjectChild(long object, int kind, int relation) {
        long[] output = new long[1];
        int[] available = new int[1];
        check("media object relationship", nativeGetObjectChild(
                object, kind, relation, output, available));
        return available[0] == 0 ? 0L : handle(output, "media object relationship");
    }

    public static byte[] getObjectBlob(long object, int kind, int property) {
        byte[] value = nativeGetObjectBlob(object, kind, property);
        if (value == null) throw NativeBindings.failure("media object image", -1);
        return value;
    }

    public static boolean objectEquals(long left, long right, int kind) {
        int[] output = new int[1];
        check("media object equality", nativeObjectEquals(left, right, kind, output));
        return output[0] != 0;
    }

    public static int objectHash(long object, int kind) {
        int[] output = new int[1];
        check("media object hash", nativeObjectHash(object, kind, output));
        return output[0];
    }

    public static void closeObject(long object, int kind) {
        check("media object dispose", nativeCloseObject(object, kind, false));
        check("media object destroy", nativeCloseObject(object, kind, true));
    }

    public static void releaseObject(long object, int kind) {
        check("media object destroy", nativeCloseObject(object, kind, true));
    }

    public static long createSong(String name, String uri) {
        long[] output = new long[1];
        check("cna_song_create_from_uri", nativeCreateSong(
                game("Song.FromUri"), utf8(name), utf8(uri), output));
        return handle(output, "cna_song_create_from_uri");
    }

    public static int getMediaPlayerInt(int property) {
        int[] output = new int[1];
        check("cna_media_player property", nativeGetMediaPlayerInt(
                game("MediaPlayer"), property, output));
        return output[0];
    }

    public static void setMediaPlayerInt(int property, int value) {
        check("cna_media_player property", nativeSetMediaPlayerInt(
                game("MediaPlayer"), property, value));
    }

    public static long getMediaPlayerLong(int property) {
        long[] output = new long[1];
        check("cna_media_player property", nativeGetMediaPlayerLong(
                game("MediaPlayer"), property, output));
        return output[0];
    }

    public static float getMediaPlayerFloat() {
        float[] output = new float[1];
        check("cna_media_player_get_volume", nativeGetMediaPlayerFloat(
                game("MediaPlayer.Volume"), output));
        return output[0];
    }

    public static void setMediaPlayerFloat(float value) {
        check("cna_media_player_set_volume", nativeSetMediaPlayerFloat(
                game("MediaPlayer.Volume"), value));
    }

    public static void mediaPlayerOperation(int operation, long handle, int index) {
        check("cna_media_player operation", nativeMediaPlayerOperation(
                game("MediaPlayer"), operation, handle, index));
    }

    public static long getMediaQueue() {
        long[] output = new long[1];
        check("cna_media_player_get_queue", nativeGetMediaQueue(
                game("MediaPlayer.Queue"), output));
        return handle(output, "cna_media_player_get_queue");
    }

    public static int getMediaQueueInt(long queue, int property) {
        int[] output = new int[1];
        check("cna_media_queue property", nativeGetMediaQueueInt(queue, property, output));
        return output[0];
    }

    public static void setMediaQueueIndex(long queue, int index) {
        check("cna_media_queue_set_active_song_index", nativeSetMediaQueueIndex(queue, index));
    }

    public static long getMediaQueueSong(long queue, int index, boolean active) {
        long[] output = new long[1];
        int[] available = new int[1];
        check(active ? "cna_media_queue_get_active_song" : "cna_media_queue_get_at",
                nativeGetMediaQueueSong(queue, index, active, output, available));
        return available[0] == 0 ? 0L : handle(output, "cna_media_queue song");
    }

    public static void releaseMediaQueue(long queue) {
        check("cna_media_queue_destroy", nativeReleaseMediaQueue(queue));
    }

    public static void getVisualizationData(VisualizationData data) {
        float[] frequencies = new float[256];
        float[] samples = new float[256];
        check("cna_media_player_get_visualization_data", nativeGetVisualizationData(
                game("MediaPlayer.GetVisualizationData"), frequencies, samples));
        FacadeFactory.setVisualizationData(data, frequencies, samples);
    }

    public static synchronized void subscribeEvents() {
        check("cna_media_player_subscribe_*", nativeSubscribeEvents());
    }

    /** Drives a canonical native event raise for JNI/callback qualification tests. */
    public static void raiseEventForQualification(int kind) {
        check("cna_media_player_raise_*", nativeRaiseMediaEvent(
                game("MediaPlayer event qualification"), kind));
    }

    @SuppressWarnings("unused")
    private static void nativeMediaEvent(int kind) {
        synchronized (PENDING_EVENTS) {
            if (acceptEvents) PENDING_EVENTS.addLast(kind);
        }
    }

    public static void dispatchPendingEvents() {
        while (true) {
            Integer kind;
            synchronized (PENDING_EVENTS) { kind = PENDING_EVENTS.pollFirst(); }
            if (kind == null) return;
            FacadeFactory.dispatchMediaPlayerEvent(kind);
        }
    }

    public static void closeAllForGameShutdown() {
        long game = NativeBindings.currentGameValue("Media shutdown");
        acceptEvents = false;
        Throwable failure = null;
        try { FacadeFactory.releaseMediaPlayerState(); }
        catch (Throwable exception) { failure = exception; }
        try { closeVideoPlayers(); }
        catch (Throwable exception) { failure = append(failure, exception); }
        try { closeMediaOwners(); }
        catch (Throwable exception) { failure = append(failure, exception); }
        try {
            check("cna_media_player_program_exit_ext", nativeMediaPlayerProgramExit(game));
        } catch (Throwable exception) { failure = append(failure, exception); }
        synchronized (PENDING_EVENTS) { PENDING_EVENTS.clear(); }
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        if (failure != null) throw new IllegalStateException("Media shutdown failed", failure);
    }

    public static void registerMediaOwner(AutoCloseable owner) {
        synchronized (MEDIA_OWNERS) { MEDIA_OWNERS.add(owner); }
    }

    public static void unregisterMediaOwner(AutoCloseable owner) {
        synchronized (MEDIA_OWNERS) { MEDIA_OWNERS.remove(owner); }
    }

    private static void closeMediaOwners() {
        AutoCloseable[] snapshot;
        synchronized (MEDIA_OWNERS) { snapshot = MEDIA_OWNERS.toArray(AutoCloseable[]::new); }
        Throwable failure = null;
        for (int index = snapshot.length - 1; index >= 0; index--) {
            try { snapshot[index].close(); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        if (failure != null) throw new IllegalStateException("Media shutdown failed", failure);
    }

    private static Throwable append(Throwable current, Throwable added) {
        if (current == null) return added;
        current.addSuppressed(added);
        return current;
    }

    public static long createVideo(byte[] fileName, int durationMs, int width, int height,
            float framesPerSecond, int soundtrackType) {
        long[] output = new long[1];
        check("cna_video_create_with_metadata", nativeCreateVideo(
                game("Video"), fileName, durationMs, width, height,
                framesPerSecond, soundtrackType, output));
        return handle(output, "cna_video_create_with_metadata");
    }

    public static void registerVideo(Video video, long handle) {
        synchronized (VIDEOS) {
            if (VIDEOS.put(video, new NativeHandle(handle, NativeHandle.Ownership.OWNED,
                    NativeMedia::destroyVideo)) != null) {
                throw new IllegalStateException("Video was registered twice");
            }
        }
    }

    public static long getVideoHandle(Video video) {
        NativeHandle handle;
        synchronized (VIDEOS) { handle = VIDEOS.get(video); }
        if (handle == null) throw new IllegalStateException("Video content is no longer loaded");
        return handle.requireValue();
    }

    public static void closeVideo(Video video) {
        NativeHandle handle;
        synchronized (VIDEOS) { handle = VIDEOS.get(video); }
        if (handle == null) return;
        handle.close();
        synchronized (VIDEOS) { VIDEOS.remove(video); }
    }

    public static void destroyVideo(long video) {
        check("cna_video_destroy", nativeDestroyVideo(video));
    }

    public static long createVideoPlayer() {
        long[] output = new long[1];
        check("cna_video_player_create", nativeCreateVideoPlayer(
                game("VideoPlayer"), output));
        return handle(output, "cna_video_player_create");
    }

    public static void registerVideoPlayer(AutoCloseable player) {
        synchronized (VIDEO_PLAYERS) { VIDEO_PLAYERS.add(player); }
    }

    public static void unregisterVideoPlayer(AutoCloseable player) {
        synchronized (VIDEO_PLAYERS) { VIDEO_PLAYERS.remove(player); }
    }

    private static void closeVideoPlayers() {
        AutoCloseable[] snapshot;
        synchronized (VIDEO_PLAYERS) {
            snapshot = VIDEO_PLAYERS.toArray(AutoCloseable[]::new);
        }
        Throwable failure = null;
        for (int index = snapshot.length - 1; index >= 0; index--) {
            try { snapshot[index].close(); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        if (failure != null) throw new IllegalStateException("VideoPlayer shutdown failed", failure);
    }

    public static int getVideoPlayerInt(long player, int property) {
        int[] output = new int[1];
        check("cna_video_player property", nativeGetVideoPlayerInt(player, property, output));
        return output[0];
    }

    public static void setVideoPlayerInt(long player, int property, int value) {
        check("cna_video_player property", nativeSetVideoPlayerInt(player, property, value));
    }

    public static long getVideoPlayerLong(long player, int property) {
        long[] output = new long[1];
        check("cna_video_player property", nativeGetVideoPlayerLong(player, property, output));
        return output[0];
    }

    public static void setVideoPlayerFloat(long player, float value) {
        check("cna_video_player_set_volume", nativeSetVideoPlayerFloat(player, value));
    }

    public static void videoPlayerOperation(long player, int operation, long video) {
        check("cna_video_player operation", nativeVideoPlayerOperation(player, operation, video));
    }

    public static long getVideoTexture(long player) {
        long[] output = new long[1];
        int[] available = new int[1];
        check("cna_video_player_get_texture", nativeGetVideoTexture(player, output, available));
        return available[0] == 0 ? 0L : handle(output, "cna_video_player_get_texture");
    }

    public static void closeVideoPlayer(long player) {
        check("cna_video_player_dispose", nativeCloseVideoPlayer(player, false));
        check("cna_video_player_destroy", nativeCloseVideoPlayer(player, true));
    }

    private static String string(byte[] bytes) {
        if (bytes == null) throw NativeBindings.failure("media string", -1);
        return new String(bytes, StandardCharsets.UTF_8);
    }

    private static native int nativeCreateLibrary(long game, int sourceIndex, long[] output);
    private static native int nativeGetMediaSourceCount(long game, int[] output);
    private static native int nativeGetMediaSourceType(long game, int index, int[] output);
    private static native byte[] nativeGetMediaSourceName(long game, int index);
    private static native int nativeGetLibraryInt(long library, int property, int[] output);
    private static native byte[] nativeGetLibraryString(long library);
    private static native int nativeGetLibraryChild(long library, int relation, byte[] token,
            long[] output, int[] available);
    private static native int nativeSaveLibraryPicture(long library, byte[] name, byte[] bytes,
            long[] output);
    private static native int nativeCloseLibrary(long library, boolean destroy);
    private static native int nativeGetCollectionInt(long collection, int kind, int property,
            int[] output);
    private static native int nativeGetCollectionAt(long collection, int kind, int index,
            long[] output);
    private static native int nativeCloseCollection(long collection, int kind, boolean destroy);
    private static native byte[] nativeGetObjectString(long object, int kind, int property);
    private static native int nativeGetObjectInt(long object, int kind, int property, int[] output);
    private static native int nativeGetObjectLong(long object, int kind, int property, long[] output);
    private static native int nativeGetObjectChild(long object, int kind, int relation,
            long[] output, int[] available);
    private static native byte[] nativeGetObjectBlob(long object, int kind, int property);
    private static native int nativeObjectEquals(long left, long right, int kind, int[] output);
    private static native int nativeObjectHash(long object, int kind, int[] output);
    private static native int nativeCloseObject(long object, int kind, boolean destroy);
    private static native int nativeCreateSong(long game, byte[] name, byte[] uri, long[] output);
    private static native int nativeGetMediaPlayerInt(long game, int property, int[] output);
    private static native int nativeSetMediaPlayerInt(long game, int property, int value);
    private static native int nativeGetMediaPlayerLong(long game, int property, long[] output);
    private static native int nativeGetMediaPlayerFloat(long game, float[] output);
    private static native int nativeSetMediaPlayerFloat(long game, float value);
    private static native int nativeMediaPlayerOperation(long game, int operation, long handle,
            int index);
    private static native int nativeGetMediaQueue(long game, long[] output);
    private static native int nativeGetMediaQueueInt(long queue, int property, int[] output);
    private static native int nativeSetMediaQueueIndex(long queue, int index);
    private static native int nativeGetMediaQueueSong(long queue, int index, boolean active,
            long[] output, int[] available);
    private static native int nativeReleaseMediaQueue(long queue);
    private static native int nativeGetVisualizationData(long game, float[] frequencies,
            float[] samples);
    private static native int nativeSubscribeEvents();
    private static native int nativeMediaPlayerProgramExit(long game);
    private static native int nativeRaiseMediaEvent(long game, int kind);
    private static native int nativeCreateVideo(long game, byte[] fileName, int durationMs,
            int width, int height, float framesPerSecond, int soundtrackType, long[] output);
    private static native int nativeDestroyVideo(long video);
    private static native int nativeCreateVideoPlayer(long game, long[] output);
    private static native int nativeGetVideoPlayerInt(long player, int property, int[] output);
    private static native int nativeSetVideoPlayerInt(long player, int property, int value);
    private static native int nativeGetVideoPlayerLong(long player, int property, long[] output);
    private static native int nativeSetVideoPlayerFloat(long player, float value);
    private static native int nativeVideoPlayerOperation(long player, int operation, long video);
    private static native int nativeGetVideoTexture(long player, long[] output, int[] available);
    private static native int nativeCloseVideoPlayer(long player, boolean destroy);
}
