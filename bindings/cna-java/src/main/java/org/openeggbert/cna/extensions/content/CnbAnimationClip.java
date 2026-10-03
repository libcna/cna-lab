package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * A skeletal animation clip decoded out of a {@code .cnb} file whose whole asset is one clip.
 *
 * <p>A CNA extension: XNA 4.0 ships skinning only as a sample-level content processor, so it has
 * no clip type at all. The distinction from {@link CnbAnimation} is where the clip lives --
 * {@code CnbAnimation} is one of several a compiled model carries, and this is a file that holds
 * exactly one, with its tracks and every keyframe readable.
 *
 * <p>This is the decoded handle. {@link CnbClip#of(CnbAnimationClip)} turns it into a value, and
 * {@link CnbClip#encode} is what writes one back out -- so a clip round-trips through CNA's own
 * encoder and CNA's own decoder rather than through a second implementation of the schema.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op. Decoding does
 * not keep the document alive: close the clip first.
 */
public final class CnbAnimationClip implements AutoCloseable {

    private final long handle;
    private boolean closed;

    private CnbAnimationClip(long handle) {
        this.handle = handle;
    }

    /**
     * Decodes the clip a document's asset is.
     *
     * @param document the document to decode
     * @return the clip, which the caller closes
     * @throws CnbFormatException when the document's asset is not a clip
     */
    public static CnbAnimationClip decode(CnbDocument document) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(document, "document");
        long[] decoded = new long[1];
        CnbExtension.check("CnbAnimationClip.decode",
                NativeCnbRoutes.cnbDecodeAnimationClip(document.handle(), decoded));
        return new CnbAnimationClip(decoded[0]);
    }

    /**
     * Returns how long the clip runs.
     *
     * @return the duration in seconds
     */
    public double getDurationSeconds() {
        return state()[0];
    }

    /**
     * Returns how many bones the clip animates.
     *
     * @return the track count
     */
    public int getTrackCount() {
        return (int) state()[1];
    }

    /**
     * Returns which index space the tracks' bone indices live in.
     *
     * @return the target space
     */
    public CnbClipTargetSpace getTargetSpace() {
        return CnbClipTargetSpace.fromValue((long) state()[2]);
    }

    /**
     * Returns which bone one track drives.
     *
     * @param track the zero-based track index
     * @return the bone index, in the clip's own target space
     */
    public int getTrackBoneIndex(int track) {
        return track(track)[0];
    }

    /**
     * Returns how many keyframes one track holds.
     *
     * @param track the zero-based track index
     * @return the keyframe count
     */
    public int getTrackKeyframeCount(int track) {
        return track(track)[1];
    }

    /**
     * Returns every keyframe of one track, in time order.
     *
     * @param track the zero-based track index
     * @return the keyframes
     */
    public List<CnbKeyframe> getKeyframes(int track) {
        int count = getTrackKeyframeCount(track);
        float[] floating = new float[count * CnbKeyframes.FLOATS];
        double[] doubles = new double[count * CnbKeyframes.DOUBLES];
        long[] written = new long[1];
        CnbExtension.check("CnbAnimationClip.getKeyframes",
                NativeCnbRoutes.cnbAnimationClipCopyKeyframes(
                        open(), track, floating, doubles, written));
        List<CnbKeyframe> keyframes = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            keyframes.add(CnbKeyframes.read(floating, doubles, index));
        }
        return Collections.unmodifiableList(keyframes);
    }

    /** Releases the clip. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbAnimationClip.close",
                NativeCnbRoutes.cnbAnimationClipDestroy(handle));
    }

    /** Duration, track count and target space, which CNA answers in one call. */
    private double[] state() {
        double[] duration = new double[1];
        long[] tracks = new long[1];
        int[] space = new int[1];
        CnbExtension.check("CnbAnimationClip.get",
                NativeCnbRoutes.cnbAnimationClipGet(open(), duration, tracks, space));
        return new double[] {duration[0], tracks[0], space[0]};
    }

    private int[] track(int index) {
        if (index < 0) {
            throw new IllegalArgumentException("track index must not be negative: " + index);
        }
        int[] bone = new int[1];
        long[] keyframes = new long[1];
        CnbExtension.check("CnbAnimationClip.getTrack",
                NativeCnbRoutes.cnbAnimationClipGetTrack(open(), index, bone, keyframes));
        return new int[] {bone[0], Math.toIntExact(keyframes[0])};
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this CnbAnimationClip is closed");
        }
        return handle;
    }
}
