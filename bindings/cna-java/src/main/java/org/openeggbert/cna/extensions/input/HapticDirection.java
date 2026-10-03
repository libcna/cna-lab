package org.openeggbert.cna.extensions.input;

/**
 * Where a haptic effect's force comes from.
 *
 * <p>How many components carry meaning depends on {@link #Type()}: one for polar and steering
 * axis, two for spherical, three for cartesian. The rest are carried through unchanged, which is
 * what CNA does, so a value read back is the value that was sent.
 *
 * @param Type how to read the components
 * @param First the first component
 * @param Second the second component, meaningful for spherical and cartesian
 * @param Third the third component, meaningful for cartesian
 */
public record HapticDirection(HapticDirectionType Type, int First, int Second, int Third) {

    /** Straight ahead: a polar direction of zero. */
    public static final HapticDirection NORTH =
            new HapticDirection(HapticDirectionType.Polar, 0, 0, 0);

    /** Returns a polar direction, in hundredths of a degree clockwise from north. */
    public static HapticDirection polar(int hundredthsOfADegree) {
        return new HapticDirection(HapticDirectionType.Polar, hundredthsOfADegree, 0, 0);
    }

    /** Returns a cartesian direction. */
    public static HapticDirection cartesian(int x, int y, int z) {
        return new HapticDirection(HapticDirectionType.Cartesian, x, y, z);
    }
}
