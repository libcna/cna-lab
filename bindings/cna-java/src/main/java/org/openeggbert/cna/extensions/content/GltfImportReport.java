package org.openeggbert.cna.extensions.content;

import java.util.Objects;

/**
 * What an importer read, and what it did to it.
 *
 * <p>A CNA extension. XNA's content pipeline reported this kind of thing to a build log the
 * runtime never saw, so a game could not ask at run time whether the model it just loaded is the
 * model the artist made. Here the model carries its own provenance.
 *
 * <p><strong>The two halves are not the same kind of thing.</strong>
 * {@linkplain #SourceCounts() The source counts} are recorded state. The five that follow are
 * derived from the model's {@linkplain CnaModel#getGltfImportDiagnostics() diagnostics} every
 * time they are asked for -- they are answered here only so that a caller does not have to walk
 * the list to learn whether anything was lost -- and
 * {@link CnaModel#setGltfImportSourceCounts} accordingly takes the first half alone.
 *
 * @param SourceCounts the shape of the scene the model was imported from
 * @param DiagnosticCount how many diagnostics the import recorded, which bounds the diagnostic
 *     index
 * @param WarningCount how many of those are warnings, counting entries rather than occurrences
 * @param DroppedFeatureCount the summed occurrences of dropped data and unsupported features
 * @param ApproximationCount the summed occurrences of approximations
 * @param AnythingLost whether at least one warning is present, so the model may differ from its
 *     source
 */
public record GltfImportReport(GltfImportSourceCounts SourceCounts, long DiagnosticCount,
        long WarningCount, long DroppedFeatureCount, long ApproximationCount,
        boolean AnythingLost) {

    /** How many integral leaves CNA's structure declares, after its size and version. */
    static final int LEAVES = 17;

    /** How many of those leaves are written rather than derived. */
    static final int WRITTEN_LEAVES = 12;

    /** Refuses a report with no counts at all, which is not the same as empty counts. */
    public GltfImportReport {
        Objects.requireNonNull(SourceCounts, "SourceCounts");
    }

    static GltfImportReport of(long[] leaves) {
        return new GltfImportReport(
                new GltfImportSourceCounts(leaves[0], leaves[1], leaves[2], leaves[3], leaves[4],
                        leaves[5], leaves[6], leaves[7], leaves[8], leaves[9], leaves[10],
                        leaves[11]),
                leaves[12], leaves[13], leaves[14], leaves[15], leaves[16] != 0L);
    }

    /**
     * Lays the twelve written counts out for CNA, leaving the five derived ones zero.
     *
     * <p>CNA refuses a report that carries a derived value rather than dropping it quietly, so
     * the trailing zeros are the contract and not merely a convenience.
     */
    static long[] toLeaves(GltfImportSourceCounts counts) {
        long[] leaves = new long[LEAVES];
        leaves[0] = counts.NodeCount();
        leaves[1] = counts.MeshInstanceCount();
        leaves[2] = counts.DistinctMeshCount();
        leaves[3] = counts.SharedMeshCount();
        leaves[4] = counts.MaxNodeDepth();
        leaves[5] = counts.CameraNodeCount();
        leaves[6] = counts.LightNodeCount();
        leaves[7] = counts.ImportedLightCount();
        leaves[8] = counts.PrimitiveCount();
        leaves[9] = counts.SkinCount();
        leaves[10] = counts.AnimationCount();
        leaves[11] = counts.ClipCount();
        return leaves;
    }
}
