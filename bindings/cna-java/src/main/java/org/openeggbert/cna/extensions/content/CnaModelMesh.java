package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.BoundingSphere;

import java.util.List;

/**
 * One mesh of a CNA model, as a value.
 *
 * @param Name the mesh's name, empty when the asset gave it none
 * @param ParentBoneIndex the index of the bone the mesh hangs from, or -1 when it has none
 * @param BoundingSphere the sphere the asset recorded for the mesh
 * @param Parts the mesh's draw ranges, in the model's own order
 * @param EffectTypeNames the distinct effects the mesh's parts draw with, in CNA's order
 */
public record CnaModelMesh(
        String Name,
        int ParentBoneIndex,
        BoundingSphere BoundingSphere,
        List<CnaModelMeshPart> Parts,
        List<String> EffectTypeNames) {

    /** Copies both lists, so a snapshot cannot be changed after it is taken. */
    public CnaModelMesh {
        Parts = List.copyOf(Parts);
        EffectTypeNames = List.copyOf(EffectTypeNames);
    }
}
