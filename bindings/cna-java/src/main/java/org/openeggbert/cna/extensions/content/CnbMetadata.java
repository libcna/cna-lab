package org.openeggbert.cna.extensions.content;

/**
 * What the {@code CMET} chunk says an asset is.
 *
 * <p>The chunk is optional, so {@link #Present()} is a real answer and not a placeholder: a file
 * without it is well formed and simply carries no names.
 *
 * @param Present whether the file carried a metadata chunk at all
 * @param AssetTypeName the asset's canonical type name, empty when absent
 * @param ContentName the source content name the asset was built from, empty when absent
 * @param Flags the chunk's own flag bits, as the file recorded them
 */
public record CnbMetadata(
        boolean Present, String AssetTypeName, String ContentName, int Flags) {
}
