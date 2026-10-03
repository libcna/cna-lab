package org.openeggbert.cna.extensions.input;

import java.util.EnumSet;
import java.util.Set;

/**
 * One thing a haptic device can do: an effect family, or a global capability.
 *
 * <p>CNA reports these as a bit set. They are an enum here so a capability is named rather than
 * masked, and {@link HapticCapabilities#Features()} hands back a {@link Set} of them.
 */
public enum HapticFeature {

    /** Constant-force effects: {@link ConstantHapticEffect}. */
    Constant(0x00000001),

    /** Sine-wave periodic effects. */
    Sine(0x00000002),

    /** Square-wave periodic effects. */
    Square(0x00000004),

    /** Triangle-wave periodic effects. */
    Triangle(0x00000008),

    /** Upward-sawtooth periodic effects. */
    SawtoothUp(0x00000010),

    /** Downward-sawtooth periodic effects. */
    SawtoothDown(0x00000020),

    /** Ramp effects: {@link RampHapticEffect}. */
    Ramp(0x00000040),

    /** Spring condition effects. */
    Spring(0x00000080),

    /** Damper condition effects. */
    Damper(0x00000100),

    /** Inertia condition effects. */
    Inertia(0x00000200),

    /** Friction condition effects. */
    Friction(0x00000400),

    /** Explicit large- and small-motor effects: {@link LeftRightHapticEffect}. */
    LeftRight(0x00000800),

    /** Custom raw-sample effects: {@link CustomHapticEffect}. */
    Custom(0x00008000),

    /** The overall effect gain can be set. */
    Gain(0x00010000),

    /** The autocentring strength can be set. */
    Autocenter(0x00020000),

    /** Whether an effect is playing can be queried. */
    Status(0x00040000),

    /** Effects can be paused and resumed. */
    Pause(0x00080000);

    private final int mask;

    HapticFeature(int mask) {
        this.mask = mask;
    }

    /** Returns CNA's own bit for this capability. */
    public int getMask() {
        return mask;
    }

    /** Decodes CNA's bit set. A bit CNA adds later is ignored rather than guessed at. */
    static Set<HapticFeature> decode(long features) {
        EnumSet<HapticFeature> decoded = EnumSet.noneOf(HapticFeature.class);
        for (HapticFeature feature : values()) {
            if ((features & feature.mask) != 0) {
                decoded.add(feature);
            }
        }
        return java.util.Collections.unmodifiableSet(decoded);
    }
}
