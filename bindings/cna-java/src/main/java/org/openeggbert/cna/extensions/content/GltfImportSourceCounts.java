package org.openeggbert.cna.extensions.content;

/**
 * The shape of the source scene a model was imported from.
 *
 * <p>A CNA extension. These twelve are recorded state: they describe what the importer read, and
 * nothing in the model can recompute them once the source is gone. They are separated from the
 * rest of {@link GltfImportReport} because CNA's own split is exactly this -- these are written,
 * the other five are derived from the diagnostics and are refused on the way in.
 *
 * @param NodeCount nodes imported from the represented source scene
 * @param MeshInstanceCount mesh placements imported from those nodes
 * @param DistinctMeshCount distinct source meshes those placements refer to
 * @param SharedMeshCount how many of those are referred to by more than one placement
 * @param MaxNodeDepth the longest imported root-to-leaf node chain
 * @param CameraNodeCount imported scene nodes that reference a camera
 * @param LightNodeCount imported scene nodes that reference a punctual light
 * @param ImportedLightCount punctual lights that reached a CNA effect light slot
 * @param PrimitiveCount source primitives this model represents, excluding material variants
 * @param SkinCount independent skins this model represents
 * @param AnimationCount source animations inspected while producing this model
 * @param ClipCount animation clips this model actually retained
 */
public record GltfImportSourceCounts(long NodeCount, long MeshInstanceCount,
        long DistinctMeshCount, long SharedMeshCount, long MaxNodeDepth, long CameraNodeCount,
        long LightNodeCount, long ImportedLightCount, long PrimitiveCount, long SkinCount,
        long AnimationCount, long ClipCount) {

    /** A scene nothing was imported from, which is what a model from another path reads back. */
    public static final GltfImportSourceCounts EMPTY =
            new GltfImportSourceCounts(0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L);

    /** Refuses a negative count, which CNA's unsigned counts cannot mean. */
    public GltfImportSourceCounts {
        requireNotNegative(NodeCount, "NodeCount");
        requireNotNegative(MeshInstanceCount, "MeshInstanceCount");
        requireNotNegative(DistinctMeshCount, "DistinctMeshCount");
        requireNotNegative(SharedMeshCount, "SharedMeshCount");
        requireNotNegative(MaxNodeDepth, "MaxNodeDepth");
        requireNotNegative(CameraNodeCount, "CameraNodeCount");
        requireNotNegative(LightNodeCount, "LightNodeCount");
        requireNotNegative(ImportedLightCount, "ImportedLightCount");
        requireNotNegative(PrimitiveCount, "PrimitiveCount");
        requireNotNegative(SkinCount, "SkinCount");
        requireNotNegative(AnimationCount, "AnimationCount");
        requireNotNegative(ClipCount, "ClipCount");
    }

    private static void requireNotNegative(long value, String name) {
        if (value < 0L) {
            throw new IllegalArgumentException(name + " must not be negative, but is " + value);
        }
    }
}
