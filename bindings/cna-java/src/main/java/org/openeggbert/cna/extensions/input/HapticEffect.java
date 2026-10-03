package org.openeggbert.cna.extensions.input;

import java.time.Duration;
import java.util.List;

/**
 * One force-feedback effect.
 *
 * <p>CNA carries every family in one flattened value where only the fields its family documents
 * are meaningful. That is the wrong shape for a caller: a periodic effect has no ramp start, and
 * a left/right effect has no direction. This is a sealed family instead, so each kind of effect
 * carries exactly the settings that apply to it and nothing else, and the flattening happens at
 * the boundary rather than in the game's code.
 *
 * <p>A device only plays an effect whose family it supports, which
 * {@link HapticCapabilities#Features()} lists and
 * {@link HapticDevice#isEffectSupported(HapticEffect)} answers for one effect.
 */
public sealed interface HapticEffect
        permits ConstantHapticEffect, PeriodicHapticEffect, RampHapticEffect,
                ConditionHapticEffect, LeftRightHapticEffect, CustomHapticEffect {

    /** How long the effect plays, or {@code null} to play until it is stopped. */
    Duration Length();

    /** Flattens this effect into the layout CNA's descriptor uses. */
    long[] encode();

    /** Returns the custom waveform samples, empty for every family but {@link CustomHapticEffect}. */
    default List<Integer> samples() {
        return List.of();
    }
}
