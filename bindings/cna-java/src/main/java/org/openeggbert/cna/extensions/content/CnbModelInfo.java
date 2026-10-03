package org.openeggbert.cna.extensions.content;

/**
 * What a compiled model contains.
 *
 * @param BoneCount how many bones the model's own hierarchy names
 * @param PartCount how many renderable parts there are
 * @param MeshCount how many meshes group those parts
 * @param AnimationCount how many animation clips travel with the model
 * @param LightCount how many baked directional lights the file records
 * @param HasSkeleton whether a skinning skeleton is present, which is separate from the bones
 * @param AppliesGltfLightingPolicy whether the source asset's glTF lighting rules apply
 * @param HasBoneHierarchy whether the bones form a hierarchy rather than a flat list
 */
public record CnbModelInfo(
        int BoneCount,
        int PartCount,
        int MeshCount,
        int AnimationCount,
        int LightCount,
        boolean HasSkeleton,
        boolean AppliesGltfLightingPolicy,
        boolean HasBoneHierarchy) {
}
