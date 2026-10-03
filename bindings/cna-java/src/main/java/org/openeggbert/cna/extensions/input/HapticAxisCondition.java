package org.openeggbert.cna.extensions.input;

/**
 * How a condition effect resists on one axis.
 *
 * <p>Every value here is per axis, and the two sides are independent: a wheel can be made to
 * resist turning left harder than turning right.
 *
 * @param RightSaturation the largest force the positive side may reach
 * @param LeftSaturation the largest force the negative side may reach
 * @param RightCoefficient how fast force grows on the positive side
 * @param LeftCoefficient how fast force grows on the negative side
 * @param Deadband how wide the band around the centre is where no force is applied
 * @param Center where that band sits on the axis
 */
public record HapticAxisCondition(
        int RightSaturation,
        int LeftSaturation,
        int RightCoefficient,
        int LeftCoefficient,
        int Deadband,
        int Center) {

    /** Returns a symmetric condition: both sides saturate and grow the same way. */
    public static HapticAxisCondition symmetric(int saturation, int coefficient) {
        return new HapticAxisCondition(saturation, saturation, coefficient, coefficient, 0, 0);
    }
}
