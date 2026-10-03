package org.openeggbert.cna.extensions.input;

/** How a {@link HapticDirection}'s three components are to be read, in CNA's identity order. */
public enum HapticDirectionType {

    /** One polar angle in hundredths of a degree, clockwise from north. Only the first is used. */
    Polar,

    /** An (X, Y, Z) cartesian vector. All three are used. */
    Cartesian,

    /** Two spherical rotation angles. The first two are used. */
    Spherical,

    /** The device's steering-wheel axis. Only the first is used. */
    SteeringAxis
}
