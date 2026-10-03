package org.openeggbert.cna.extensions.input;

import java.time.Duration;

/**
 * A force that changes linearly from one strength to another.
 *
 * @param Direction where the force comes from
 * @param Length how long the ramp takes, or {@code null} to play until stopped
 * @param Delay how long to wait before it starts
 * @param Start the strength it begins at
 * @param End the strength it finishes at
 * @param Trigger the device button that replays it, or {@link HapticTrigger#NONE}
 * @param Envelope how it ramps in and out, or {@link HapticEnvelope#NONE}
 */
public record RampHapticEffect(
        HapticDirection Direction,
        Duration Length,
        Duration Delay,
        int Start,
        int End,
        HapticTrigger Trigger,
        HapticEnvelope Envelope) implements HapticEffect {

    /** Returns a ramp between two strengths, with no delay, trigger or envelope. */
    public static RampHapticEffect of(
            HapticDirection direction, Duration length, int start, int end) {
        return new RampHapticEffect(direction, length, Duration.ZERO, start, end,
                HapticTrigger.NONE, HapticEnvelope.NONE);
    }

    @Override
    public long[] encode() {
        long[] effect = HapticEffectLayout.of(6, Length());
        HapticEffectLayout.direction(effect, Direction);
        HapticEffectLayout.schedule(effect, Delay, Trigger);
        HapticEffectLayout.envelope(effect, Envelope);
        effect[HapticEffectLayout.RAMP_START] = Start;
        effect[HapticEffectLayout.RAMP_END] = End;
        return effect;
    }
}
