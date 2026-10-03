package org.openeggbert.cna.extensions.content;

/**
 * An asset a {@code .cnb} file refers to by logical name rather than embedding.
 *
 * @param LogicalName the name the referring file uses, which the content manager resolves
 * @param ExpectedAssetType what the referring file believes the target is
 * @param Flags the reference's own flag bits, as the file recorded them
 */
public record CnbExternalReference(
        String LogicalName, CnbAssetType ExpectedAssetType, int Flags) {
}
