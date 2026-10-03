package org.openeggbert.cna.extensions.content;

import java.util.List;
import java.util.Objects;

/**
 * One bone's keyframes within an animation clip.
 *
 * <p>A CNA extension: XNA 4.0 ships skinning only as a sample-level content processor, so it has
 * no track type at all.
 *
 * @param BoneIndex which bone the track drives, in the clip's own target space; an index the
 *        skeleton does not have is skipped rather than refused, which is CNA's behaviour and not
 *        a tolerance added here
 * @param Keyframes the poses, in time order
 */
public record CnbBoneTrack(int BoneIndex, List<CnbKeyframe> Keyframes) {

    /** Copies the keyframes, so the track is a value rather than a view of a caller's list. */
    public CnbBoneTrack {
        Keyframes = List.copyOf(Objects.requireNonNull(Keyframes, "Keyframes"));
    }
}
