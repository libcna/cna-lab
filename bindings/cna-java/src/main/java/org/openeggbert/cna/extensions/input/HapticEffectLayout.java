package org.openeggbert.cna.extensions.input;

import java.time.Duration;

/**
 * The one place that knows CNA's flattened effect layout.
 *
 * <p>The generated declaration of {@code hapticDeviceCreateEffect} documents the leaf order; the
 * indices below are that order, named. Keeping them here means a family record never carries a
 * magic index, and a change upstream is one edit rather than six.
 *
 * <p>This class is not application API.
 */
final class HapticEffectLayout {

    static final int LEAVES = 44;

    static final int TYPE = 0;
    static final int DIRECTION_TYPE = 2;
    static final int DIRECTION_FIRST = 3;
    static final int LENGTH = 6;
    static final int DELAY = 7;
    static final int BUTTON = 8;
    static final int INTERVAL = 9;
    static final int LEVEL = 10;
    static final int PERIOD = 11;
    static final int MAGNITUDE = 12;
    static final int OFFSET = 13;
    static final int PHASE = 14;
    static final int RAMP_START = 15;
    static final int RAMP_END = 16;
    static final int RIGHT_SATURATION = 17;
    static final int LEFT_SATURATION = 20;
    static final int RIGHT_COEFFICIENT = 23;
    static final int LEFT_COEFFICIENT = 26;
    static final int DEADBAND = 29;
    static final int CENTER = 32;
    static final int LARGE_MAGNITUDE = 35;
    static final int SMALL_MAGNITUDE = 36;
    static final int CUSTOM_PERIOD = 37;
    static final int CUSTOM_CHANNELS = 38;
    static final int ATTACK_LENGTH = 40;
    static final int ATTACK_LEVEL = 41;
    static final int FADE_LENGTH = 42;
    static final int FADE_LEVEL = 43;

    /** CNA's "play forever" length, which is the widest unsigned 32-bit value. */
    static final long INFINITE_LENGTH = 0xFFFFFFFFL;

    private HapticEffectLayout() {
    }

    /** Starts a descriptor of one family. */
    static long[] of(int type, Duration length) {
        long[] effect = new long[LEAVES];
        effect[TYPE] = type;
        effect[LENGTH] = length == null ? INFINITE_LENGTH : milliseconds(length, "length");
        return effect;
    }

    /** Writes the direction, which every family but the conditions and left/right uses. */
    static void direction(long[] effect, HapticDirection direction) {
        effect[DIRECTION_TYPE] = direction.Type().ordinal();
        effect[DIRECTION_FIRST] = direction.First();
        effect[DIRECTION_FIRST + 1] = direction.Second();
        effect[DIRECTION_FIRST + 2] = direction.Third();
    }

    /** Writes the delay and the button trigger. */
    static void schedule(long[] effect, Duration delay, HapticTrigger trigger) {
        effect[DELAY] = milliseconds(delay, "delay");
        effect[BUTTON] = trigger.Button();
        effect[INTERVAL] = milliseconds(trigger.Interval(), "interval");
    }

    /** Writes the attack and fade ramps. */
    static void envelope(long[] effect, HapticEnvelope envelope) {
        effect[ATTACK_LENGTH] = milliseconds(envelope.AttackLength(), "attackLength");
        effect[ATTACK_LEVEL] = envelope.AttackLevel();
        effect[FADE_LENGTH] = milliseconds(envelope.FadeLength(), "fadeLength");
        effect[FADE_LEVEL] = envelope.FadeLevel();
    }

    /** Converts a duration to the milliseconds CNA counts, refusing one it cannot carry. */
    static long milliseconds(Duration duration, String name) {
        long value = duration.toMillis();
        if (value < 0) {
            throw new IllegalArgumentException(name + " must not be negative: " + duration);
        }
        if (value > INFINITE_LENGTH) {
            throw new IllegalArgumentException(name + " is longer than CNA can carry: " + duration);
        }
        return value;
    }
}
