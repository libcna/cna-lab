package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;

import java.util.List;

/**
 * A compiled model's skinning skeleton.
 *
 * <p>Separate from the model's bones, and not the same thing: the bones are the asset's own node
 * hierarchy, and this is the joint set a skinned part is weighted against. A model can have
 * bones and no skeleton, which is what {@link CnbModelInfo#HasSkeleton()} distinguishes.
 *
 * <p>The root prefix is optional. It is the transform a source format put above the root joint --
 * a scene-level scale or axis conversion -- and a model whose source carried none has it absent
 * rather than set to identity, because "no prefix" and "an identity prefix" are different facts
 * about where the asset came from.
 *
 * @param Hierarchy one parent index per joint, or -1 for a root
 * @param BindPose each joint's local bind transform
 * @param InverseBindPose each joint's inverse global bind transform
 * @param RootPrefix the transform above the root joint, or null when the source carried none
 */
public record CnbSkeleton(
        List<Integer> Hierarchy,
        List<Matrix> BindPose,
        List<Matrix> InverseBindPose,
        List<Matrix> RootPrefix) {

    /** Copies the lists, and refuses a skeleton whose parts do not describe the same joints. */
    public CnbSkeleton {
        Hierarchy = List.copyOf(Hierarchy);
        BindPose = List.copyOf(BindPose);
        InverseBindPose = List.copyOf(InverseBindPose);
        RootPrefix = RootPrefix == null ? null : List.copyOf(RootPrefix);
        if (BindPose.size() != Hierarchy.size() || InverseBindPose.size() != Hierarchy.size()
                || (RootPrefix != null && RootPrefix.size() != Hierarchy.size())) {
            throw new IllegalArgumentException(
                    "a skeleton's hierarchy and matrix sets must all describe the same joints");
        }
    }

    /** Returns how many joints the skeleton has. */
    public int getJointCount() {
        return Hierarchy.size();
    }
}
