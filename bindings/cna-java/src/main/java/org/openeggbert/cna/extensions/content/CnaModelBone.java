package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;

import java.util.List;

/**
 * One node of a CNA model's bone hierarchy, as a value.
 *
 * <p>This is a snapshot. CNA hands out an owned view for every bone, parent link and child list,
 * and the walk that built this destroyed every one of them, so nothing here holds a native
 * handle or a lifetime. The parent and children are indices into the model's own bone list,
 * which is what survives a snapshot; an object reference would only be as good as the walk.
 *
 * @param Index the bone's index in the model's bone list
 * @param Name the bone's name, empty when the asset gave it none
 * @param Transform the bone's transform relative to its parent
 * @param ParentIndex the parent's index, or -1 for the root
 * @param ChildIndices the children's indices, in the model's own order
 */
public record CnaModelBone(
        int Index, String Name, Matrix Transform, int ParentIndex, List<Integer> ChildIndices) {

    /** Copies the child list, so a snapshot cannot be changed after it is taken. */
    public CnaModelBone {
        ChildIndices = List.copyOf(ChildIndices);
    }
}
