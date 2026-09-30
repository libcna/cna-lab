package org.openeggbert.cna.extensions.devices;

/** Which way a camera faces, as the host reports it. */
public enum CameraPosition {
    /** The host does not say. */
    Unknown,
    /** Facing the user. */
    FrontFacing,
    /** Facing away from the user. */
    BackFacing;

    static CameraPosition fromValue(long value) {
        CameraPosition[] positions = values();
        return value >= 0 && value < positions.length ? positions[(int) value] : Unknown;
    }
}
