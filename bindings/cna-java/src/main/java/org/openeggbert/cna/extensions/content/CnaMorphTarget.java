package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Vector3;

import java.util.List;
import java.util.Objects;

/**
 * One blend shape: how far each vertex moves towards it.
 *
 * <p>A CNA extension, and glTF's morph target. The deltas are added to the base pose in
 * proportion to the target's weight, so a weight of zero contributes nothing and a weight of one
 * contributes the whole delta.
 *
 * <p>Normal deltas are optional -- a target that moves vertices without changing their shading
 * carries none -- which is why an empty list here is a real answer rather than a missing one.
 *
 * @param PositionDeltas one per vertex of the base pose
 * @param NormalDeltas one per vertex, or empty
 */
public record CnaMorphTarget(List<Vector3> PositionDeltas, List<Vector3> NormalDeltas) {

    /** Copies the lists, so a target is a value rather than a view of a caller's lists. */
    public CnaMorphTarget {
        PositionDeltas = List.copyOf(Objects.requireNonNull(PositionDeltas, "PositionDeltas"));
        NormalDeltas = List.copyOf(Objects.requireNonNull(NormalDeltas, "NormalDeltas"));
    }
}
