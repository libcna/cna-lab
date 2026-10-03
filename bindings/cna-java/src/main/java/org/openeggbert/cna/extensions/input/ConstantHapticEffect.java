package org.openeggbert.cna.extensions.input;

import java.time.Duration;

/**
 * A steady push from one direction.
 *
 * @param Direction where the force comes from
 * @param Length how long it plays, or {@code null} to play until stopped
 * @param Delay how long to wait before it starts
 * @param Level how hard it pushes, from -32768 to 32767
 * @param Trigger the device button that replays it, or {@link HapticTrigger#NONE}
 * @param Envelope how it ramps in and out, or {@link HapticEnvelope#NONE}
 */
public record ConstantHapticEffect(
        HapticDirection Direction,
        Duration Length,
        Duration Delay,
        int Level,
        HapticTrigger Trigger,
        HapticEnvelope Envelope) implements HapticEffect {

    /** Returns a constant push of one strength, for as long as the caller asks. */
    public static ConstantHapticEffect of(HapticDirection direction, Duration length, int level) {
        return new ConstantHapticEffect(direction, length, Duration.ZERO, level,
                HapticTrigger.NONE, HapticEnvelope.NONE);
    }

    @Override
    public long[] encode() {
        long[] effect = HapticEffectLayout.of(0, Length());
        HapticEffectLayout.direction(effect, Direction);
        HapticEffectLayout.schedule(effect, Delay, Trigger);
        HapticEffectLayout.envelope(effect, Envelope);
        effect[HapticEffectLayout.LEVEL] = Level;
        return effect;
    }
}
