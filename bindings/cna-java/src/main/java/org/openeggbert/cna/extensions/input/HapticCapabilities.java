package org.openeggbert.cna.extensions.input;

import java.util.Set;

/**
 * The fixed hardware shape of a haptic device.
 *
 * @param Name the host's display name for the device
 * @param Features the effect families and global capabilities the device supports
 * @param AxisCount how many axes the device reports
 * @param MaxEffects how many effects can be stored, or {@code null} when the device is closed or
 *     the host will not say; absent is not zero, because zero would mean "stores none"
 * @param MaxEffectsPlaying how many effects can play at once, or {@code null} for the same reason
 * @param IsOpen whether the handle is actually attached to hardware
 * @param RumbleSupported whether the simple rumble shortcut works on this device
 */
public record HapticCapabilities(
        String Name,
        Set<HapticFeature> Features,
        int AxisCount,
        Integer MaxEffects,
        Integer MaxEffectsPlaying,
        boolean IsOpen,
        boolean RumbleSupported) {

    /** Copies the feature set, so capabilities cannot be changed after they are read. */
    public HapticCapabilities {
        Features = Set.copyOf(Features);
    }
}
