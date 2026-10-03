package org.openeggbert.cna.extensions.content;

/**
 * One part's morph-target state: blend shapes and the weights that drive them.
 *
 * <p>A morph target is a per-vertex displacement -- a face's smile, a wing's flex -- applied on
 * top of the part's own geometry. {@code TargetCount} says how many the part carries,
 * {@code WeightCount} how many are being blended at once, and the weight track is the animation
 * over those weights.
 *
 * @param VertexCount how many vertices each target's deltas cover
 * @param TargetCount how many blend shapes the part carries
 * @param WeightCount how many weights the part blends at once
 * @param WeightTrackKeyCount how many keys the weight animation has
 * @param RecomputeFlatNormals whether normals are recomputed after blending rather than blended
 * @param WeightTrackStepInterpolation whether the weight track steps rather than interpolates
 * @param WeightTrackCubicSpline whether the weight track is a cubic spline, which is what makes
 *        its in and out tangents meaningful
 */
public record CnbMorphInfo(
        int VertexCount,
        int TargetCount,
        int WeightCount,
        int WeightTrackKeyCount,
        boolean RecomputeFlatNormals,
        boolean WeightTrackStepInterpolation,
        boolean WeightTrackCubicSpline) {
}
