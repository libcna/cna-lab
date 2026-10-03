package org.openeggbert.cna.extensions.content;

/**
 * What an importer did with the data a diagnostic concerns.
 *
 * <p>A CNA extension. The five outcomes are what makes a diagnostic actionable rather than
 * merely alarming: data that was generated is not data that was dropped. The ordinals are CNA's
 * own {@code CNA_GLTF_IMPORT_KIND_*_EXT} values.
 */
public enum GltfImportKind {

    /** A note about something the importer did, with nothing lost. */
    Information,

    /** The importer produced data the source did not carry, such as derived normals. */
    GeneratedData,

    /** The source's own data was malformed and was repaired or ignored. */
    InvalidSourceData,

    /** The data was represented, but not exactly. */
    Approximation,

    /** The data was recognised and not carried over. */
    DroppedData,

    /** The feature is one this importer does not implement. */
    UnsupportedFeature;

    static GltfImportKind of(int value) {
        GltfImportKind[] all = values();
        if (value < 0 || value >= all.length) {
            throw new IllegalStateException("CNA reported import kind " + value
                    + ", which this ABI does not name");
        }
        return all[value];
    }
}
