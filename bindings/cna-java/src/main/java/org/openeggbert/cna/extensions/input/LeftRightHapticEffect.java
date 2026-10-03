package org.openeggbert.cna.extensions.input;

import java.time.Duration;

/**
 * The two rumble motors of a game pad, driven explicitly.
 *
 * <p>This is the only family with no direction, no delay and no envelope: a rumble motor has one
 * speed and nothing else. It is the same hardware XNA drives through
 * {@code GamePad.SetVibration}, which takes two normalised floats and no duration; this reports
 * CNA's own shape instead of restating that one.
 *
 * @param Length how long the motors run, or {@code null} to run until stopped
 * @param LargeMagnitude the low-frequency motor's strength, from 0 to 65535
 * @param SmallMagnitude the high-frequency motor's strength, from 0 to 65535
 */
public record LeftRightHapticEffect(
        Duration Length, int LargeMagnitude, int SmallMagnitude) implements HapticEffect {

    @Override
    public long[] encode() {
        long[] effect = HapticEffectLayout.of(11, Length());
        effect[HapticEffectLayout.LARGE_MAGNITUDE] = LargeMagnitude;
        effect[HapticEffectLayout.SMALL_MAGNITUDE] = SmallMagnitude;
        return effect;
    }
}
