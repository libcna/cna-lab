package org.openeggbert.cna.extensions.input;

/** Which physical analogy a {@link ConditionHapticEffect} resists with. */
public enum ConditionKind {

    /** Resistance proportional to how far the axis is from its centre. */
    Spring(7, HapticFeature.Spring),

    /** Resistance proportional to how fast the axis is moving. */
    Damper(8, HapticFeature.Damper),

    /** Resistance proportional to how fast the axis is accelerating. */
    Inertia(9, HapticFeature.Inertia),

    /** Resistance to the axis moving at all. */
    Friction(10, HapticFeature.Friction);

    private final int effectType;
    private final HapticFeature feature;

    ConditionKind(int effectType, HapticFeature feature) {
        this.effectType = effectType;
        this.feature = feature;
    }

    /** Returns the capability a device must have to play this condition. */
    public HapticFeature getFeature() {
        return feature;
    }

    int effectType() {
        return effectType;
    }
}
