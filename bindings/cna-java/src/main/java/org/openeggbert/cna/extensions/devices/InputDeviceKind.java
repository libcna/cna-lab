package org.openeggbert.cna.extensions.devices;

/** Which family of input device an enumeration or a hot-plug event is about. */
public enum InputDeviceKind {

    /** A mouse or another pointing device the host reports as one. */
    Mouse,

    /** A keyboard. */
    Keyboard,

    /**
     * A touch device.
     *
     * <p>Touch devices are enumerated but have no hot-plug event: CNA raises connection events
     * for mice and keyboards only, so {@link InputDevices#addConnectedListener} never reports
     * one.
     */
    TouchDevice
}
