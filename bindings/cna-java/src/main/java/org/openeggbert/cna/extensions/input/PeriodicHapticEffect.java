package org.openeggbert.cna.extensions.input;

import java.time.Duration;

/**
 * A repeating wave.
 *
 * @param Wave which wave the effect follows
 * @param Direction where the force comes from
 * @param Length how long it plays, or {@code null} to play until stopped
 * @param Delay how long to wait before it starts
 * @param Period how long one cycle of the wave lasts
 * @param Magnitude the wave's peak; a negative value shifts the phase by 180 degrees
 * @param Offset the wave's mean level
 * @param Phase a positive phase shift, in hundredths of a degree
 * @param Trigger the device button that replays it, or {@link HapticTrigger#NONE}
 * @param Envelope how it ramps in and out, or {@link HapticEnvelope#NONE}
 */
public record PeriodicHapticEffect(
        PeriodicWave Wave,
        HapticDirection Direction,
        Duration Length,
        Duration Delay,
        Duration Period,
        int Magnitude,
        int Offset,
        int Phase,
        HapticTrigger Trigger,
        HapticEnvelope Envelope) implements HapticEffect {

    /** Returns a plain wave of one period and peak, with no delay, trigger or envelope. */
    public static PeriodicHapticEffect of(
            PeriodicWave wave, Duration length, Duration period, int magnitude) {
        return new PeriodicHapticEffect(wave, HapticDirection.NORTH, length, Duration.ZERO,
                period, magnitude, 0, 0, HapticTrigger.NONE, HapticEnvelope.NONE);
    }

    @Override
    public long[] encode() {
        long[] effect = HapticEffectLayout.of(Wave.effectType(), Length());
        HapticEffectLayout.direction(effect, Direction);
        HapticEffectLayout.schedule(effect, Delay, Trigger);
        HapticEffectLayout.envelope(effect, Envelope);
        effect[HapticEffectLayout.PERIOD] = HapticEffectLayout.milliseconds(Period, "period");
        effect[HapticEffectLayout.MAGNITUDE] = Magnitude;
        effect[HapticEffectLayout.OFFSET] = Offset;
        effect[HapticEffectLayout.PHASE] = Phase;
        return effect;
    }
}
