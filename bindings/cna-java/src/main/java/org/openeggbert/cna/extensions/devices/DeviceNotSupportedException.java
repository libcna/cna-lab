package org.openeggbert.cna.extensions.devices;

/**
 * Raised when a device capability exists in the ABI but not in this build or on this host.
 *
 * <p>Kept distinct from an ordinary failure so a game can fall back -- to no vibration, to no
 * tray icon -- without swallowing a real error.
 */
public class DeviceNotSupportedException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public DeviceNotSupportedException(String message) {
        super(message);
    }
}
