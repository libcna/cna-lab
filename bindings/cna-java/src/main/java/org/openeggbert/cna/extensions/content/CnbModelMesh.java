package org.openeggbert.cna.extensions.content;

import java.util.List;

/**
 * One mesh: a name, a bone it hangs from, and the parts it draws.
 *
 * @param Name the mesh's name
 * @param ParentBone the bone index the mesh follows, or -1 when it follows none
 * @param PartIndices the model parts this mesh is made of, in order
 */
public record CnbModelMesh(String Name, int ParentBone, List<Integer> PartIndices) {

    /** Copies the part indices so the record cannot be changed through the list it was given. */
    public CnbModelMesh {
        PartIndices = List.copyOf(PartIndices);
    }
}
