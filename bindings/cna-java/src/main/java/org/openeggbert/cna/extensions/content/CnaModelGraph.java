package org.openeggbert.cna.extensions.content;

import java.util.List;

/**
 * A whole CNA model's structure, as one immutable snapshot.
 *
 * @param Bones every bone, in the model's own order
 * @param Meshes every mesh, in the model's own order
 * @param RootBoneIndex the root bone's index, or -1 when the model has no root
 */
public record CnaModelGraph(
        List<CnaModelBone> Bones, List<CnaModelMesh> Meshes, int RootBoneIndex) {

    /** Copies both lists, so a snapshot cannot be changed after it is taken. */
    public CnaModelGraph {
        Bones = List.copyOf(Bones);
        Meshes = List.copyOf(Meshes);
    }
}
