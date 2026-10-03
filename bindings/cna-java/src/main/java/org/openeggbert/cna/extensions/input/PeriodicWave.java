package org.openeggbert.cna.extensions.input;

/** The wave a {@link PeriodicHapticEffect} follows. */
public enum PeriodicWave {

    /** A sine wave. */
    Sine(1, HapticFeature.Sine),

    /** A square wave. */
    Square(2, HapticFeature.Square),

    /** A triangle wave. */
    Triangle(3, HapticFeature.Triangle),

    /** A sawtooth rising to its peak. */
    SawtoothUp(4, HapticFeature.SawtoothUp),

    /** A sawtooth falling from its peak. */
    SawtoothDown(5, HapticFeature.SawtoothDown);

    private final int effectType;
    private final HapticFeature feature;

    PeriodicWave(int effectType, HapticFeature feature) {
        this.effectType = effectType;
        this.feature = feature;
    }

    /** Returns the capability a device must have to play this wave. */
    public HapticFeature getFeature() {
        return feature;
    }

    int effectType() {
        return effectType;
    }
}
