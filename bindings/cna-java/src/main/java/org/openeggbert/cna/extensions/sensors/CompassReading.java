package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Vector3;

import java.time.OffsetDateTime;

/**
 * One compass sample.
 *
 * <p>The three headings are doubles in CNA and stay doubles here: narrowing them to float at the
 * boundary would throw away precision the host actually reported.
 *
 * @param Timestamp when the host took the reading, with the offset the host reported
 * @param HeadingAccuracy how far the heading may be off, in degrees
 * @param MagneticHeading heading relative to magnetic north, in degrees
 * @param TrueHeading heading relative to true north, in degrees
 * @param MagnetometerReading the raw magnetometer vector per axis, in micro-teslas
 */
public record CompassReading(
        OffsetDateTime Timestamp,
        double HeadingAccuracy,
        double MagneticHeading,
        double TrueHeading,
        Vector3 MagnetometerReading) {

    /** The two integral leaves CNA's structure declares: the timestamp. */
    long[] toIntegralLeaves() {
        return SensorExtension.timestampLeaves(Timestamp);
    }

    /** The three floating leaves: the magnetometer vector. */
    float[] toFloatingLeaves() {
        return new float[] {MagnetometerReading.X, MagnetometerReading.Y, MagnetometerReading.Z};
    }

    /** The three double leaves, in declaration order: accuracy, magnetic heading, true heading. */
    double[] toDoubleLeaves() {
        return new double[] {HeadingAccuracy, MagneticHeading, TrueHeading};
    }
}
