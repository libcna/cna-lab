package org.openeggbert.cna.extensions.content;

import java.util.List;

/**
 * A skin binding a model's meshes to a skeleton.
 *
 * <p>A CNA extension. XNA 4.0 shipped skinning only as a sample-level content processor, so its
 * {@code Model} has nowhere to put one.
 *
 * @param Name the skin's name, empty when the asset gave it none
 * @param HasSkinningData whether the skin carries a skeleton CNA can pose
 * @param MeshIndices the indices, in the model's mesh list, of the meshes this skin drives
 */
public record CnaModelSkin(String Name, boolean HasSkinningData, List<Long> MeshIndices) {

    /** Copies the mesh list, so a snapshot cannot be changed after it is taken. */
    public CnaModelSkin {
        MeshIndices = List.copyOf(MeshIndices);
    }
}
