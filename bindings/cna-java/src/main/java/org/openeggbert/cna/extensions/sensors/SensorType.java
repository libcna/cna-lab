package org.openeggbert.cna.extensions.sensors;

/**
 * What kind of motion sensor the host reported, in CNA's own identity order.
 *
 * <p>The left and right variants belong to a controller that carries a sensor on each side.
 */
public enum SensorType {

    /** The host reported a sensor whose kind it does not name. */
    Unknown,

    /** An accelerometer, reporting acceleration in metres per second squared. */
    Accelerometer,

    /** A gyroscope, reporting angular velocity in radians per second. */
    Gyroscope,

    /** The left-hand accelerometer of a dual-sensor controller. */
    AccelerometerLeft,

    /** The left-hand gyroscope of a dual-sensor controller. */
    GyroscopeLeft,

    /** The right-hand accelerometer of a dual-sensor controller. */
    AccelerometerRight,

    /** The right-hand gyroscope of a dual-sensor controller. */
    GyroscopeRight
}
