package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Media.Video;
import Microsoft.Xna.Framework.Media.VideoSoundtrackType;

import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.time.Duration;
import java.util.Objects;

/**
 * A video as {@code .cnb} carries it: metadata and a streaming reference, never the frames.
 *
 * <p>The same choice a song makes, for the same reason -- see {@link CnbSong}. What is recorded
 * here in addition is the frame size, the rate and the soundtrack type, so a game can size its
 * render target and choose its audio path before opening the media.
 *
 * @param StreamReference the logical name of the media file to stream; never empty
 * @param Duration the recorded duration
 * @param Width the frame width in pixels
 * @param Height the frame height in pixels
 * @param FramesPerSecond the frame rate
 * @param SoundtrackType which soundtracks the media carries
 */
public record CnbVideo(
        String StreamReference,
        Duration Duration,
        int Width,
        int Height,
        float FramesPerSecond,
        VideoSoundtrackType SoundtrackType) {

    /**
     * Encodes a video as a complete {@code .cnb} file.
     *
     * @param streamReference the logical name of the media to stream; must not be empty
     * @param video the frame size, rate, duration and soundtrack type to record
     * @param contentName the source content name to record
     * @return the whole file
     * @throws CnbFormatException when the reference is empty, a dimension is out of range, or the
     *         frame rate is not positive and finite
     */
    public static byte[] encode(String streamReference, CnbVideo video, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(streamReference, "streamReference");
        Objects.requireNonNull(video, "video");
        Objects.requireNonNull(contentName, "contentName");
        byte[] reference = CnbExtension.utf8(streamReference);
        byte[] content = CnbExtension.utf8(contentName);
        long[] integral = {
            video.Duration().toMillis(), video.Width(), video.Height(),
            FacadeFactory.videoSoundtrackValue(video.SoundtrackType()), 0,
        };
        float[] floating = {video.FramesPerSecond()};
        long[] size = new long[1];
        int probe = NativeCnbRoutes.cnbEncodeVideo(
                reference, integral, floating, content, new byte[0], size);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("CnbVideo.encode", probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("CnbVideo.encode", NativeCnbRoutes.cnbEncodeVideo(
                reference, integral, floating, content, destination, written));
        return CnbExtension.trim(destination, written[0]);
    }

    /**
     * Creates an ordinary XNA {@link Video} over the resolved media.
     *
     * <p>Everything but the location crosses from the file. The location does not, for the reason
     * {@link CnbSong#toSong(java.net.URI)} gives: the reference in the file is logical.
     *
     * @param graphicsDevice the device the player will present frames on
     * @param fileName where the media actually is on this machine
     * @return the video, which a {@code VideoPlayer} can play
     */
    public Video toVideo(GraphicsDevice graphicsDevice, String fileName) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        Objects.requireNonNull(fileName, "fileName");
        return FacadeFactory.createVideo(graphicsDevice, fileName,
                Math.toIntExact(Duration.toMillis()), Width, Height, FramesPerSecond,
                FacadeFactory.videoSoundtrackValue(SoundtrackType));
    }

    static CnbVideo read(long handle) {
        long[] integral = new long[5];
        float[] floating = new float[1];
        CnbExtension.check("CnbDocument.decodeVideo",
                NativeCnbRoutes.cnbDecodeVideo(handle, integral, floating));
        String reference = CnbExtension.text("CnbDocument.decodeVideo",
                bytes -> NativeCnbRoutes.cnbDecodeVideoStreamReferenceSize(handle, bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbDecodeVideoStreamReference(handle, destination, bytes));
        return new CnbVideo(reference, java.time.Duration.ofMillis(integral[0]),
                (int) integral[1], (int) integral[2], floating[0],
                soundtrack(integral[3]));
    }

    private static VideoSoundtrackType soundtrack(long value) {
        for (VideoSoundtrackType type : VideoSoundtrackType.values()) {
            if (FacadeFactory.videoSoundtrackValue(type) == value) {
                return type;
            }
        }
        throw new CnbFormatException("the file names video soundtrack type " + value
                + ", which XNA has no constant for");
    }
}
