package org.openeggbert.cna.extensions.content;

/**
 * Whether a glTF import diagnostic is a note or an observable loss of fidelity.
 *
 * <p>A CNA extension: XNA's content pipeline reported build errors and warnings to a build log
 * and the runtime never saw them. The ordinals are CNA's own
 * {@code CNA_GLTF_IMPORT_SEVERITY_*_EXT} values.
 */
public enum GltfImportSeverity {

    /** Something the importer did that a reader might want to know. */
    Information,

    /** Something the imported model does not represent as the source did. */
    Warning;

    static GltfImportSeverity of(int value) {
        GltfImportSeverity[] all = values();
        if (value < 0 || value >= all.length) {
            throw new IllegalStateException("CNA reported import severity " + value
                    + ", which this ABI does not name");
        }
        return all[value];
    }
}
