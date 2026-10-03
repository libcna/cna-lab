package org.openeggbert.cna.extensions.input;

import java.time.Duration;
import java.util.List;

/**
 * A force that resists the axis rather than driving it.
 *
 * <p>A condition has no direction of its own: each axis carries its own two-sided resistance, so
 * direction is a property of {@link HapticAxisCondition} and not of the effect. It has no
 * envelope either, because there is nothing to ramp in.
 *
 * @param Kind which physical analogy the resistance follows
 * @param Length how long it lasts, or {@code null} to last until stopped
 * @param Delay how long to wait before it starts
 * @param Axes the per-axis conditions, in the device's axis order, at most three
 * @param Trigger the device button that replays it, or {@link HapticTrigger#NONE}
 */
public record ConditionHapticEffect(
        ConditionKind Kind,
        Duration Length,
        Duration Delay,
        List<HapticAxisCondition> Axes,
        HapticTrigger Trigger) implements HapticEffect {

    /** CNA's descriptor carries exactly three per-axis slots. */
    private static final int MAXIMUM_AXES = 3;

    /** Copies the axis list, and refuses more axes than CNA's descriptor can carry. */
    public ConditionHapticEffect {
        Axes = List.copyOf(Axes);
        if (Axes.size() > MAXIMUM_AXES) {
            throw new IllegalArgumentException(
                    "a condition effect carries at most " + MAXIMUM_AXES
                    + " axes; got " + Axes.size());
        }
    }

    /** Returns a condition over one axis, with no delay and no trigger. */
    public static ConditionHapticEffect of(
            ConditionKind kind, Duration length, HapticAxisCondition axis) {
        return new ConditionHapticEffect(kind, length, Duration.ZERO, List.of(axis),
                HapticTrigger.NONE);
    }

    @Override
    public long[] encode() {
        long[] effect = HapticEffectLayout.of(Kind.effectType(), Length());
        HapticEffectLayout.schedule(effect, Delay, Trigger);
        for (int axis = 0; axis < Axes.size(); axis++) {
            HapticAxisCondition condition = Axes.get(axis);
            effect[HapticEffectLayout.RIGHT_SATURATION + axis] = condition.RightSaturation();
            effect[HapticEffectLayout.LEFT_SATURATION + axis] = condition.LeftSaturation();
            effect[HapticEffectLayout.RIGHT_COEFFICIENT + axis] = condition.RightCoefficient();
            effect[HapticEffectLayout.LEFT_COEFFICIENT + axis] = condition.LeftCoefficient();
            effect[HapticEffectLayout.DEADBAND + axis] = condition.Deadband();
            effect[HapticEffectLayout.CENTER + axis] = condition.Center();
        }
        return effect;
    }
}
