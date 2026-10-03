package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.NativeBindings;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * A skeletal animation clip, as a value a game can build, encode and read back.
 *
 * <p>A CNA extension. XNA 4.0 ships skinning only as a sample-level content processor, so a clip
 * has no XNA type at all -- which is why this lives here rather than in the strict projection.
 *
 * <p><strong>This is the shape CNA takes and the shape Java can hold, reconciled.</strong> CNA's
 * own route takes a pointer graph: a clip descriptor pointing at track descriptors, each pointing
 * at keyframes. That is what the JNI generator refuses, and correctly -- nothing in the C says
 * which keyframes belong to which track. The lifetimes are not unknown though, only underivable:
 * CNA documents every array as borrowed for the call and deeply copies what it keeps. So the
 * graph is flattened here, checked in the adapter, rebuilt for the duration of one call, and
 * freed after it.
 *
 * @param DurationSeconds how long the clip runs
 * @param Tracks one track per animated bone
 */
public record CnbClip(double DurationSeconds, List<CnbBoneTrack> Tracks) {

    /** Copies the tracks, so a clip is a value rather than a view of a caller's list. */
    public CnbClip {
        Tracks = List.copyOf(Objects.requireNonNull(Tracks, "Tracks"));
    }

    /**
     * Encodes this clip as a whole {@code .cnb} file.
     *
     * <p>CNA's own encoder, so the bytes carry the schema and the version CNA's reader expects.
     *
     * @param targetSpace which index space the tracks' bone indices live in
     * @param contentName the logical content name to record
     * @return the whole file
     */
    public byte[] encode(CnbClipTargetSpace targetSpace, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(targetSpace, "targetSpace");
        Objects.requireNonNull(contentName, "contentName");
        Flattened flat = flatten();
        byte[] name = CnbExtension.utf8(contentName);
        long[] size = new long[1];
        // Ask with no buffer first: CNA reports the size it needs and writes nothing, which is
        // the format's own two-call protocol rather than a guess at how big the file will be.
        int probe = NativeBindings.cnbEncodeAnimationClip(DurationSeconds, flat.boneIndices,
                flat.keyframeCounts, flat.times, flat.values, targetSpace.ordinal(), name,
                new byte[0], size);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("CnbClip.encode", probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("CnbClip.encode", NativeBindings.cnbEncodeAnimationClip(
                DurationSeconds, flat.boneIndices, flat.keyframeCounts, flat.times, flat.values,
                targetSpace.ordinal(), name, destination, written));
        return destination;
    }

    /**
     * Reads a clip back out of a decoded {@code .cnb} clip file.
     *
     * @param clip the decoded clip
     * @return the same clip as a value
     */
    public static CnbClip of(CnbAnimationClip clip) {
        Objects.requireNonNull(clip, "clip");
        int trackCount = clip.getTrackCount();
        List<CnbBoneTrack> tracks = new ArrayList<>(trackCount);
        for (int track = 0; track < trackCount; track++) {
            tracks.add(new CnbBoneTrack(clip.getTrackBoneIndex(track), clip.getKeyframes(track)));
        }
        return new CnbClip(clip.getDurationSeconds(), tracks);
    }

    /** The four parallel arrays the native boundary takes. */
    Flattened flatten() {
        int trackCount = Tracks.size();
        int keyframeCount = 0;
        for (CnbBoneTrack track : Tracks) {
            keyframeCount += track.Keyframes().size();
        }
        Flattened flat = new Flattened(new int[trackCount], new int[trackCount],
                new double[keyframeCount], new float[keyframeCount * CnbKeyframes.FLOATS]);
        int keyframe = 0;
        for (int index = 0; index < trackCount; index++) {
            CnbBoneTrack track = Tracks.get(index);
            flat.boneIndices[index] = track.BoneIndex();
            flat.keyframeCounts[index] = track.Keyframes().size();
            for (CnbKeyframe pose : track.Keyframes()) {
                flat.times[keyframe] = pose.TimeSeconds();
                System.arraycopy(CnbKeyframes.floating(pose), 0, flat.values,
                        keyframe * CnbKeyframes.FLOATS, CnbKeyframes.FLOATS);
                keyframe++;
            }
        }
        return flat;
    }

    /** The clip's pointer graph, flattened the one way that loses nothing. */
    record Flattened(int[] boneIndices, int[] keyframeCounts, double[] times, float[] values) {
    }
}
