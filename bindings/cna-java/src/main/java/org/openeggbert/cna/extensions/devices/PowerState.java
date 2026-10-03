package org.openeggbert.cna.extensions.devices;

/**
 * The host's power state.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. {@code Unknown} and {@code Error} are real
 * answers, not placeholders -- a machine that will not say is different from one on battery.
 */
public enum PowerState {
    Error,
    Unknown,
    OnBattery,
    NoBattery,
    Charging,
    Charged
}
