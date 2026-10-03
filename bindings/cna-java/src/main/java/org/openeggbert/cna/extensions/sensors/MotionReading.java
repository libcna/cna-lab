package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Vector3;

import java.time.OffsetDateTime;

/**
 * One fused motion sample: orientation, motion and gravity separated from each other.
 *
 * @param Timestamp when the host took the reading, with the offset the host reported
 * @param Attitude the fused device orientation
 * @param DeviceAcceleration acceleration per axis in g, with gravity already removed
 * @param DeviceRotationRate angular velocity per axis in radians per second
 * @param Gravity the gravity vector per axis, in g
 */
public record MotionReading(
        OffsetDateTime Timestamp,
        AttitudeReading Attitude,
        Vector3 DeviceAcceleration,
        Vector3 DeviceRotationRate,
        Vector3 Gravity) {

    /** The four integral leaves: this reading's timestamp and the attitude's. */
    long[] toIntegralLeaves() {
        long[] mine = SensorExtension.timestampLeaves(Timestamp);
        long[] attitude = SensorExtension.timestampLeaves(Attitude.Timestamp());
        return new long[] {mine[0], mine[1], attitude[0], attitude[1]};
    }

    /** The thirty-two floating leaves, in the order CNA's structure declares them. */
    float[] toFloatingLeaves() {
        float[] leaves = new float[32];
        leaves[0] = Attitude.Pitch();
        leaves[1] = Attitude.Roll();
        leaves[2] = Attitude.Yaw();
        leaves[3] = Attitude.Quaternion().X;
        leaves[4] = Attitude.Quaternion().Y;
        leaves[5] = Attitude.Quaternion().Z;
        leaves[6] = Attitude.Quaternion().W;
        float[] matrix = {
                Attitude.RotationMatrix().M11, Attitude.RotationMatrix().M12,
                Attitude.RotationMatrix().M13, Attitude.RotationMatrix().M14,
                Attitude.RotationMatrix().M21, Attitude.RotationMatrix().M22,
                Attitude.RotationMatrix().M23, Attitude.RotationMatrix().M24,
                Attitude.RotationMatrix().M31, Attitude.RotationMatrix().M32,
                Attitude.RotationMatrix().M33, Attitude.RotationMatrix().M34,
                Attitude.RotationMatrix().M41, Attitude.RotationMatrix().M42,
                Attitude.RotationMatrix().M43, Attitude.RotationMatrix().M44};
        System.arraycopy(matrix, 0, leaves, 7, 16);
        leaves[23] = DeviceAcceleration.X;
        leaves[24] = DeviceAcceleration.Y;
        leaves[25] = DeviceAcceleration.Z;
        leaves[26] = DeviceRotationRate.X;
        leaves[27] = DeviceRotationRate.Y;
        leaves[28] = DeviceRotationRate.Z;
        leaves[29] = Gravity.X;
        leaves[30] = Gravity.Y;
        leaves[31] = Gravity.Z;
        return leaves;
    }
}
