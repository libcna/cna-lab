package Microsoft.Xna.Framework.Media;

import Microsoft.Xna.Framework.Graphics.Texture2D;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeMedia;

import java.time.Duration;
import java.util.Objects;

/** Native video-player control path with a non-owning transient frame facade. */
public final class VideoPlayer implements AutoCloseable {
    private long handle;
    private Video video;
    private Texture2D frameTexture;
    private boolean looped;
    private boolean muted;
    private float volume = 1f;

    public VideoPlayer() {
        handle = NativeMedia.createVideoPlayer();
        NativeMedia.registerVideoPlayer(this);
    }

    public boolean getIsDisposed() {
        if (handle == 0L) return true;
        invalidateFrameTexture();
        return NativeMedia.getVideoPlayerInt(handle, 0) != 0;
    }

    public Video getVideo() { return video; }

    public MediaState getState() {
        requireOpen();
        invalidateFrameTexture();
        return MediaState.fromValue(NativeMedia.getVideoPlayerInt(handle, 1));
    }

    public Duration getPlayPosition() {
        requireOpen();
        invalidateFrameTexture();
        return video == null ? Duration.ZERO
                : NativeMedia.duration(NativeMedia.getVideoPlayerLong(handle, 0));
    }

    public boolean getIsLooped() { return looped; }
    public void setIsLooped(boolean value) {
        requireOpen();
        if (looped == value) return;
        invalidateFrameTexture();
        NativeMedia.setVideoPlayerInt(handle, 0, value ? 1 : 0);
        looped = value;
    }

    public boolean getIsMuted() { return muted; }
    public void setIsMuted(boolean value) {
        requireOpen();
        if (muted == value) return;
        invalidateFrameTexture();
        NativeMedia.setVideoPlayerInt(handle, 1, value ? 1 : 0);
        muted = value;
    }

    public float getVolume() { return volume; }
    public void setVolume(float value) {
        requireOpen();
        if (volume == value) return;
        if (value < 0f || value > 1f) {
            throw new IllegalArgumentException("value must be in [0, 1] or NaN");
        }
        invalidateFrameTexture();
        NativeMedia.setVideoPlayerFloat(handle, value);
        volume = value;
    }

    public void Play(Video video) {
        requireOpen();
        Video selected = Objects.requireNonNull(video, "video");
        invalidateFrameTexture();
        NativeMedia.videoPlayerOperation(handle, 0, NativeMedia.getVideoHandle(selected));
        this.video = selected;
    }

    public void Pause() { operation(1); }
    public void Resume() { operation(2); }
    public void Stop() { operation(3); }

    public Texture2D GetTexture() {
        requireOpen();
        invalidateFrameTexture();
        if (video == null) throw new IllegalStateException("No video has been played");
        long texture = NativeMedia.getVideoTexture(handle);
        if (texture == 0L) return null;
        frameTexture = NativeBindings.createBorrowedVideoTexture(
                video.graphicsDevice(), texture);
        return frameTexture;
    }

    public final synchronized void Dispose() {
        if (handle == 0L) return;
        invalidateFrameTexture();
        NativeMedia.closeVideoPlayer(handle);
        handle = 0L;
        NativeMedia.unregisterVideoPlayer(this);
    }

    @Override
    public final void close() {
        Dispose();
    }

    private void operation(int operation) {
        requireOpen();
        invalidateFrameTexture();
        if (video != null) NativeMedia.videoPlayerOperation(handle, operation, 0L);
    }

    private void invalidateFrameTexture() {
        if (frameTexture != null) {
            frameTexture.close();
            frameTexture = null;
        }
    }

    private void requireOpen() {
        if (handle == 0L) throw new IllegalStateException("VideoPlayer is already closed");
    }
}
