package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;

import java.time.OffsetDateTime;

/**
 * One device-orientation sample, carried inside a {@link MotionReading}.
 *
 * <p>The three angles, the quaternion and the matrix are three spellings of the same orientation,
 * which is how CNA reports it; none is derived here.
 *
 * @param Timestamp when the host took the reading, with the offset the host reported
 * @param Pitch rotation around the X axis, in radians
 * @param Roll rotation around the Y axis, in radians
 * @param Yaw rotation around the Z axis, in radians
 * @param Quaternion the same orientation as a quaternion
 * @param RotationMatrix the same orientation as a rotation matrix
 */
public record AttitudeReading(
        OffsetDateTime Timestamp,
        float Pitch,
        float Roll,
        float Yaw,
        Quaternion Quaternion,
        Matrix RotationMatrix) {
}
