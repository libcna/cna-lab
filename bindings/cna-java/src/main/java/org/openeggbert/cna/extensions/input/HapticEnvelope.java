package org.openeggbert.cna.extensions.input;

import java.time.Duration;

/**
 * How an effect ramps in at its start and out at its end.
 *
 * @param AttackLength how long the ramp-in lasts
 * @param AttackLevel the effect level the ramp-in starts from
 * @param FadeLength how long the ramp-out lasts
 * @param FadeLevel the effect level the ramp-out ends at
 */
public record HapticEnvelope(
        Duration AttackLength, int AttackLevel, Duration FadeLength, int FadeLevel) {

    /** No ramp at either end: the effect starts and stops at full level. */
    public static final HapticEnvelope NONE =
            new HapticEnvelope(Duration.ZERO, 0, Duration.ZERO, 0);
}
