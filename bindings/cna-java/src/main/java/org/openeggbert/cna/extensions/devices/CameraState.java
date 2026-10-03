package org.openeggbert.cna.extensions.devices;

/** What a {@link Camera} is doing, in CNA's own order. */
public enum CameraState {
    /** The host has no camera service. */
    NotSupported,
    /** The camera is not open. */
    Closed,
    /** The camera is opening, which on most hosts includes waiting for permission. */
    Opening,
    /** The user or the host refused access. */
    Denied,
    /** Frames are being produced. */
    Ready,
    /** The camera was open and went away. */
    Lost;

    static CameraState fromValue(int value) {
        CameraState[] states = values();
        if (value < 0 || value >= states.length) {
            throw new IllegalStateException("CNA reported an unknown camera state " + value);
        }
        return states[value];
    }
}
