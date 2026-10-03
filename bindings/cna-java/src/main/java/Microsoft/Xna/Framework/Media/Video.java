package Microsoft.Xna.Framework.Media;

import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import org.openeggbert.cna.internal.NativeMedia;

import java.time.Duration;

/** Video content metadata plus the native decoder asset used by {@link VideoPlayer}. */
public final class Video {
    private final GraphicsDevice graphicsDevice;
    private final Duration duration;
    private final int width;
    private final int height;
    private final float framesPerSecond;
    private final VideoSoundtrackType soundtrackType;

    Video(GraphicsDevice graphicsDevice, String fileName, int durationMilliseconds,
            int width, int height, float framesPerSecond, int soundtrackType) {
        this.graphicsDevice = graphicsDevice;
        this.duration = Duration.ofMillis(durationMilliseconds);
        this.width = width;
        this.height = height;
        this.framesPerSecond = framesPerSecond;
        this.soundtrackType = VideoSoundtrackType.fromValue(soundtrackType);
        long handle = NativeMedia.createVideo(NativeMedia.utf8(fileName), durationMilliseconds,
                width, height, framesPerSecond, soundtrackType);
        NativeMedia.registerVideo(this, handle);
    }

    public Duration getDuration() { return duration; }
    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public float getFramesPerSecond() { return framesPerSecond; }
    public VideoSoundtrackType getVideoSoundtrackType() { return soundtrackType; }
    GraphicsDevice graphicsDevice() { return graphicsDevice; }
}
