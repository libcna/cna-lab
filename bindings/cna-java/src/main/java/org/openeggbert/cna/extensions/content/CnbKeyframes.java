package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector3;

/**
 * How one {@link CnbKeyframe} crosses the native boundary.
 *
 * <p>CNA's keyframe is one structure with a {@code double} and ten {@code float}s, and the
 * generated adapter flattens it into one array per storage class in declaration order. Both the
 * order and the split are the generator's own, taken from the live header, so this is the one
 * place that knows it -- rather than every caller repeating it.
 */
final class CnbKeyframes {

    /** How many floats CNA's keyframe carries: a translation, a rotation and a scale. */
    static final int FLOATS = 10;

    /** How many doubles it carries: the time. */
    static final int DOUBLES = 1;

    private CnbKeyframes() {
    }

    static float[] floating(CnbKeyframe keyframe) {
        return new float[] {
                keyframe.Translation().X, keyframe.Translation().Y, keyframe.Translation().Z,
                keyframe.Rotation().X, keyframe.Rotation().Y, keyframe.Rotation().Z,
                keyframe.Rotation().W,
                keyframe.Scale().X, keyframe.Scale().Y, keyframe.Scale().Z};
    }

    static double[] doubles(CnbKeyframe keyframe) {
        return new double[] {keyframe.TimeSeconds()};
    }

    /** Reads one keyframe out of the flattened pair at an element index. */
    static CnbKeyframe read(float[] floating, double[] doubles, int index) {
        int base = index * FLOATS;
        return new CnbKeyframe(doubles[index * DOUBLES],
                new Vector3(floating[base], floating[base + 1], floating[base + 2]),
                new Quaternion(floating[base + 3], floating[base + 4], floating[base + 5],
                        floating[base + 6]),
                new Vector3(floating[base + 7], floating[base + 8], floating[base + 9]));
    }
}
