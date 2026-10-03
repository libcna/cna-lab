package org.openeggbert.cna.extensions.devices;

/**
 * Whether the game is running on real hardware or in an emulator.
 *
 * <p>A CNA extension: XNA 4.0 answers a related question only on Windows Phone, and only for its
 * own device families. The ordinals are CNA's own {@code CNA_DEVICE_TYPE_*} values.
 */
public enum DeviceType {

    /** Real hardware. */
    Device,

    /** An emulator or simulator. */
    Emulator;

    static DeviceType of(int value) {
        DeviceType[] all = values();
        if (value < 0 || value >= all.length) {
            throw new IllegalStateException("CNA reported device type " + value
                    + ", which this ABI does not name");
        }
        return all[value];
    }
}
