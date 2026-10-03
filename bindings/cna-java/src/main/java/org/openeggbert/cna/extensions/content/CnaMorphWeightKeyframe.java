package org.openeggbert.cna.extensions.content;

import java.util.Objects;

/**
 * One moment of a morph-target weight animation.
 *
 * <p>A CNA extension: XNA 4.0's {@code Model} has no morph data at all, so a weight keyframe has
 * no XNA counterpart.
 *
 * <p>The tangents are optional and are read only when the track says
 * {@linkplain CnaMorphWeightTrack#CubicSpline() cubic spline}; a step or linear track carries
 * empty ones. When they are present, glTF's rule applies: one tangent per weight.
 *
 * @param TimeSeconds when in the animation this applies
 * @param Weights one weight per morph target
 * @param InTangents the incoming Hermite tangents, or empty
 * @param OutTangents the outgoing Hermite tangents, or empty
 */
public record CnaMorphWeightKeyframe(double TimeSeconds, float[] Weights, float[] InTangents,
        float[] OutTangents) {

    /** Copies the arrays, so a keyframe is a value rather than a view of a caller's arrays. */
    public CnaMorphWeightKeyframe {
        Weights = Objects.requireNonNull(Weights, "Weights").clone();
        InTangents = Objects.requireNonNull(InTangents, "InTangents").clone();
        OutTangents = Objects.requireNonNull(OutTangents, "OutTangents").clone();
    }
}
