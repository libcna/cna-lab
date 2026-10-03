package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.NativeBindings;

import java.util.List;
import java.util.Objects;

/**
 * How a set of morph-target weights changes over time.
 *
 * <p>A CNA extension. XNA has no morph data and therefore no track for its weights; this is
 * glTF's, which is where morph targets in CNA come from.
 *
 * <p>Interpolation is stated rather than inferred, and the two flags are not alternatives to
 * linear: {@code StepInterpolation} holds the lower keyframe's value, {@code CubicSpline} uses
 * each keyframe's tangents, and neither means linear between the two.
 *
 * @param Keyframes the weight values over time, in time order
 * @param StepInterpolation whether evaluation holds the lower keyframe's value
 * @param CubicSpline whether evaluation uses the keyframes' tangents
 */
public record CnaMorphWeightTrack(List<CnaMorphWeightKeyframe> Keyframes,
        boolean StepInterpolation, boolean CubicSpline) {

    /** Copies the keyframes, so a track is a value. */
    public CnaMorphWeightTrack {
        Keyframes = List.copyOf(Objects.requireNonNull(Keyframes, "Keyframes"));
    }

    /** A track with nothing in it, which is what morph data with no animation carries. */
    public static CnaMorphWeightTrack empty() {
        return new CnaMorphWeightTrack(List.of(), false, false);
    }

    /**
     * Evaluates the track at a time, without any morph data to attach it to.
     *
     * @param timeSeconds when to evaluate
     * @return one weight per morph target, as the track states them at that moment
     */
    public float[] evaluate(double timeSeconds) {
        CnbExtension.requireAvailable();
        Flattened flat = flatten();
        long[] written = new long[1];
        int probe = NativeBindings.morphWeightTrackEvaluate(flat.times(), flat.weightCounts(),
                flat.weights(), flat.inCounts(), flat.inTangents(), flat.outCounts(),
                flat.outTangents(), StepInterpolation(), CubicSpline(), timeSeconds,
                new float[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("CnaMorphWeightTrack.evaluate", probe);
        }
        float[] destination = new float[Math.toIntExact(written[0])];
        CnbExtension.check("CnaMorphWeightTrack.evaluate",
                NativeBindings.morphWeightTrackEvaluate(flat.times(), flat.weightCounts(),
                        flat.weights(), flat.inCounts(), flat.inTangents(), flat.outCounts(),
                        flat.outTangents(), StepInterpolation(), CubicSpline(), timeSeconds,
                        destination, written));
        return destination;
    }

    /** The seven parallel arrays the native boundary takes. */
    Flattened flatten() {
        int count = Keyframes.size();
        double[] times = new double[count];
        int[] weightCounts = new int[count];
        int[] inCounts = new int[count];
        int[] outCounts = new int[count];
        int weightTotal = 0;
        int inTotal = 0;
        int outTotal = 0;
        for (int index = 0; index < count; index++) {
            CnaMorphWeightKeyframe keyframe = Keyframes.get(index);
            times[index] = keyframe.TimeSeconds();
            weightCounts[index] = keyframe.Weights().length;
            inCounts[index] = keyframe.InTangents().length;
            outCounts[index] = keyframe.OutTangents().length;
            weightTotal += weightCounts[index];
            inTotal += inCounts[index];
            outTotal += outCounts[index];
        }
        float[] weights = new float[weightTotal];
        float[] inTangents = new float[inTotal];
        float[] outTangents = new float[outTotal];
        int weightAt = 0;
        int inAt = 0;
        int outAt = 0;
        for (CnaMorphWeightKeyframe keyframe : Keyframes) {
            System.arraycopy(keyframe.Weights(), 0, weights, weightAt,
                    keyframe.Weights().length);
            System.arraycopy(keyframe.InTangents(), 0, inTangents, inAt,
                    keyframe.InTangents().length);
            System.arraycopy(keyframe.OutTangents(), 0, outTangents, outAt,
                    keyframe.OutTangents().length);
            weightAt += keyframe.Weights().length;
            inAt += keyframe.InTangents().length;
            outAt += keyframe.OutTangents().length;
        }
        return new Flattened(times, weightCounts, weights, inCounts, inTangents, outCounts,
                outTangents);
    }

    /** The track's keyframe graph, flattened the one way that loses nothing. */
    record Flattened(double[] times, int[] weightCounts, float[] weights, int[] inCounts,
            float[] inTangents, int[] outCounts, float[] outTangents) {
    }
}
