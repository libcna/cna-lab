package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Vector3;

import java.time.OffsetDateTime;

/**
 * One accelerometer sample.
 *
 * @param Timestamp when the host took the reading, with the offset the host reported
 * @param Acceleration acceleration per axis in g, so a device at rest reads about one g along
 *     whichever axis points down
 */
public record AccelerometerReading(OffsetDateTime Timestamp, Vector3 Acceleration) {
}
