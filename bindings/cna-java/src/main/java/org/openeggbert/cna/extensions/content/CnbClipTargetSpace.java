package org.openeggbert.cna.extensions.content;

/**
 * Which index space a clip's bone indices live in.
 *
 * <p><strong>The two must never be silently interchanged.</strong> A joint-palette clip's indices
 * select entries of the skinning skeleton; a scene-node clip's select the model's own bones.
 * Applying one as the other poses the wrong bones without failing, which is why the descriptor
 * does not carry it and every route that needs it asks for it separately.
 */
public enum CnbClipTargetSpace {

    /** Indices into the skinning skeleton's joints. */
    JointPalette,

    /** Indices into the model's own bone hierarchy. */
    SceneNode;

    static CnbClipTargetSpace fromValue(long value) {
        CnbClipTargetSpace[] values = values();
        if (value < 0 || value >= values.length) {
            throw new CnbFormatException("the file names clip target space " + value
                    + ", which this build has no constant for");
        }
        return values[(int) value];
    }
}
