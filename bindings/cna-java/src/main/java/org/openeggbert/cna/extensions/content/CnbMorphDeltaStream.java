package org.openeggbert.cna.extensions.content;

/**
 * Which per-vertex stream one morph target's deltas belong to.
 *
 * <p>A blend shape can displace positions alone, or carry its own normal and tangent corrections
 * so a strongly deformed shape still lights correctly. The numbers are wire format.
 */
public enum CnbMorphDeltaStream {

    /** Per-vertex position displacements. */
    Position,

    /** Per-vertex normal displacements. */
    Normal,

    /** Per-vertex tangent displacements. */
    Tangent
}
