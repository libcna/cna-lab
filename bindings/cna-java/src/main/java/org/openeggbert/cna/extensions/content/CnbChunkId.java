package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.nio.charset.StandardCharsets;

/**
 * A four-character chunk identifier.
 *
 * <p>Packed little-endian so its bytes read left to right in a hex dump, which is why it is a
 * value here rather than a string: two identifiers compare as integers and there is exactly one
 * spelling of each.
 *
 * <p>Every byte must be printable ASCII. <strong>An identifier beginning with an uppercase letter
 * is reserved for CNA's own schemas</strong>; a game defining its own {@code .cnb} schema starts
 * its identifiers with a lowercase letter, and {@link #isWellFormed()} says whether a value is a
 * legal identifier at all.
 *
 * @param Value the packed identifier, as CNA stores it
 */
public record CnbChunkId(int Value) {

    /** {@code CMET}: the asset's canonical type name and its source content name. */
    public static final CnbChunkId METADATA = new CnbChunkId(0x54454D43);

    /** {@code XREF}: the external assets this file refers to by logical name. */
    public static final CnbChunkId EXTERNAL_REFERENCES = new CnbChunkId(0x46455258);

    /**
     * Returns the identifier four characters name.
     *
     * @param text exactly four printable ASCII characters
     * @return the packed identifier
     */
    public static CnbChunkId of(String text) {
        CnbExtension.requireAvailable();
        byte[] bytes = text.getBytes(StandardCharsets.US_ASCII);
        if (bytes.length != 4) {
            throw new IllegalArgumentException(
                    "a chunk identifier is exactly four characters; got '" + text + "'");
        }
        int[] id = new int[1];
        CnbExtension.check("CnbChunkId.of", NativeCnbRoutes.cnbMakeChunkId(
                bytes[0], bytes[1], bytes[2], bytes[3], id));
        return new CnbChunkId(id[0]);
    }

    /** Reports whether every byte is printable ASCII, which a legal identifier requires. */
    public boolean isWellFormed() {
        CnbExtension.requireAvailable();
        boolean[] wellFormed = new boolean[1];
        CnbExtension.check("CnbChunkId.isWellFormed",
                NativeCnbRoutes.cnbIsWellFormedChunkId(Value, wellFormed));
        return wellFormed[0];
    }

    /** Returns the four characters, as CNA renders them. */
    @Override
    public String toString() {
        CnbExtension.requireAvailable();
        return CnbExtension.text("CnbChunkId.toString",
                bytes -> NativeCnbRoutes.cnbGetChunkIdStringSize(Value, bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbCopyChunkIdString(Value, destination, bytes));
    }
}
