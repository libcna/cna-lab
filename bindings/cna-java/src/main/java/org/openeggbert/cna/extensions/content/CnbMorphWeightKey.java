package org.openeggbert.cna.extensions.content;

/**
 * One key of a part's morph weight track: when, and how many values it carries.
 *
 * <p>The values themselves are read separately, through
 * {@link CnbModelData#readMorphWeightKeyValues}, because a key has three arrays and only a
 * cubic-spline track uses the tangent two.
 *
 * @param TimeSeconds when in the track this key applies
 * @param WeightCount how many weights the key sets
 * @param InTangentCount how many incoming tangents it carries, zero unless the track is a spline
 * @param OutTangentCount how many outgoing tangents it carries
 */
public record CnbMorphWeightKey(
        double TimeSeconds, int WeightCount, int InTangentCount, int OutTangentCount) {
}
