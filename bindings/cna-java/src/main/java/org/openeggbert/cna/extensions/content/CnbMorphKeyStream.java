package org.openeggbert.cna.extensions.content;

/**
 * Which of a morph weight key's three value arrays a route is addressing.
 *
 * <p>The tangents only mean anything for a cubic-spline weight track; a stepped or linear one
 * carries weights alone. The numbers are wire format.
 */
public enum CnbMorphKeyStream {

    /** The weights themselves. */
    Weights,

    /** The incoming tangents, for a cubic-spline track. */
    InTangent,

    /** The outgoing tangents, for a cubic-spline track. */
    OutTangent
}
