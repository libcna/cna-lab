package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * A bone hierarchy and its bind pose: what a skinned mesh is deformed by.
 *
 * <p>A CNA extension. XNA 4.0 ships skinning only as a sample-level content processor, so a
 * skeleton has no XNA type at all -- the {@code SkinnedModel} sample defined its own, and every
 * game that used it carried a copy.
 *
 * <p>The three lists are parallel and one bone long each. {@link #RootPrefix} may be empty, which
 * is what a skeleton whose root needs no extra transform looks like; when it is present it is one
 * matrix per bone, and CNA refuses any other length.
 *
 * @param ParentBoneIndices each bone's parent, or -1 for a root
 * @param BindPoseLocal each bone's local bind-pose transform
 * @param InverseBindPoseGlobal each bone's inverse global bind-pose transform
 * @param RootPrefix an optional per-bone prefix, empty when the skeleton has none
 */
public record CnaSkeleton(List<Integer> ParentBoneIndices, List<Matrix> BindPoseLocal,
        List<Matrix> InverseBindPoseGlobal, List<Matrix> RootPrefix) {

    /** How many floats one matrix occupies where CNA takes an array of them. */
    static final int MATRIX_FLOATS = 16;

    /** Copies the lists and refuses three that do not describe the same bones. */
    public CnaSkeleton {
        ParentBoneIndices = List.copyOf(Objects.requireNonNull(ParentBoneIndices,
                "ParentBoneIndices"));
        BindPoseLocal = List.copyOf(Objects.requireNonNull(BindPoseLocal, "BindPoseLocal"));
        InverseBindPoseGlobal = List.copyOf(Objects.requireNonNull(InverseBindPoseGlobal,
                "InverseBindPoseGlobal"));
        RootPrefix = List.copyOf(Objects.requireNonNull(RootPrefix, "RootPrefix"));
        if (BindPoseLocal.size() != ParentBoneIndices.size()
                || InverseBindPoseGlobal.size() != ParentBoneIndices.size()) {
            throw new IllegalArgumentException(
                    "a skeleton needs one bind pose and one inverse bind pose per bone: "
                            + ParentBoneIndices.size() + " bones, " + BindPoseLocal.size()
                            + " bind poses, " + InverseBindPoseGlobal.size() + " inverses");
        }
        if (!RootPrefix.isEmpty() && RootPrefix.size() != ParentBoneIndices.size()) {
            throw new IllegalArgumentException(
                    "a root prefix is empty or one matrix per bone, not " + RootPrefix.size());
        }
    }

    /**
     * Returns how many bones the skeleton has.
     *
     * @return the bone count
     */
    public int boneCount() {
        return ParentBoneIndices.size();
    }

    int[] parents() {
        int[] parents = new int[ParentBoneIndices.size()];
        for (int index = 0; index < parents.length; index++) {
            parents[index] = ParentBoneIndices.get(index);
        }
        return parents;
    }

    /** Flattens a list of matrices into CNA's own row order, sixteen floats each. */
    static float[] matrices(List<Matrix> matrices) {
        float[] leaves = new float[matrices.size() * MATRIX_FLOATS];
        for (int index = 0; index < matrices.size(); index++) {
            Matrix matrix = matrices.get(index);
            int base = index * MATRIX_FLOATS;
            leaves[base] = matrix.M11;
            leaves[base + 1] = matrix.M12;
            leaves[base + 2] = matrix.M13;
            leaves[base + 3] = matrix.M14;
            leaves[base + 4] = matrix.M21;
            leaves[base + 5] = matrix.M22;
            leaves[base + 6] = matrix.M23;
            leaves[base + 7] = matrix.M24;
            leaves[base + 8] = matrix.M31;
            leaves[base + 9] = matrix.M32;
            leaves[base + 10] = matrix.M33;
            leaves[base + 11] = matrix.M34;
            leaves[base + 12] = matrix.M41;
            leaves[base + 13] = matrix.M42;
            leaves[base + 14] = matrix.M43;
            leaves[base + 15] = matrix.M44;
        }
        return leaves;
    }

    /** Reads matrices back out of the same flattening. */
    static List<Matrix> matricesOf(float[] leaves, int count) {
        List<Matrix> matrices = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            int base = index * MATRIX_FLOATS;
            matrices.add(new Matrix(
                    leaves[base], leaves[base + 1], leaves[base + 2], leaves[base + 3],
                    leaves[base + 4], leaves[base + 5], leaves[base + 6], leaves[base + 7],
                    leaves[base + 8], leaves[base + 9], leaves[base + 10], leaves[base + 11],
                    leaves[base + 12], leaves[base + 13], leaves[base + 14], leaves[base + 15]));
        }
        return Collections.unmodifiableList(matrices);
    }
}
