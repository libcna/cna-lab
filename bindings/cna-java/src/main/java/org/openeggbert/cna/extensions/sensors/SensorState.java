package org.openeggbert.cna.extensions.sensors;

/**
 * What a sensor is currently doing, in CNA's own identity order.
 *
 * <p>The ordinals are CNA's {@code CNA_SENSOR_STATE_*} values, so a state read across the
 * boundary is this enum's constant at the same index.
 */
public enum SensorState {

    /** The host does not have this sensor. */
    NotSupported,

    /** The sensor is acquiring and its readings are real measurements. */
    Ready,

    /** The sensor is starting up and has not produced a reading yet. */
    Initializing,

    /** The sensor is present and started, but no data has arrived. */
    NoData,

    /** The sensor exists but this process may not read it. */
    NoPermissions,

    /** The sensor is present and switched off. */
    Disabled
}
