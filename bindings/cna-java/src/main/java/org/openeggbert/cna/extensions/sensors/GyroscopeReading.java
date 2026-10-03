package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Vector3;

import java.time.OffsetDateTime;

/**
 * One gyroscope sample.
 *
 * @param Timestamp when the host took the reading, with the offset the host reported
 * @param RotationRate angular velocity per axis in radians per second
 */
public record GyroscopeReading(OffsetDateTime Timestamp, Vector3 RotationRate) {
}
