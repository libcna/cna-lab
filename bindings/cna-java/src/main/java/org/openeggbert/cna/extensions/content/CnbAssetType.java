package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

/**
 * What kind of asset a {@code .cnb} file holds.
 *
 * <p>An asset type is an integer on the wire, and CNA reserves ranges: its own types are small
 * numbers, one range is reserved for future CNA use, and everything at or above the custom range
 * belongs to games defining their own schemas. This is a value rather than an enum for exactly
 * that reason -- a custom type has no constant and must still be representable.
 *
 * @param Id the asset type identifier as it appears in the file
 */
public record CnbAssetType(int Id) {

    /** No asset type; a file carrying this is malformed. */
    public static final CnbAssetType INVALID = new CnbAssetType(0);

    /** A two-dimensional texture. */
    public static final CnbAssetType TEXTURE_2D = new CnbAssetType(1);

    /** A three-dimensional texture. */
    public static final CnbAssetType TEXTURE_3D = new CnbAssetType(2);

    /** A cube texture. */
    public static final CnbAssetType TEXTURE_CUBE = new CnbAssetType(3);

    /** A sprite font. */
    public static final CnbAssetType SPRITE_FONT = new CnbAssetType(4);

    /** A model. */
    public static final CnbAssetType MODEL = new CnbAssetType(5);

    /** An animation clip. */
    public static final CnbAssetType ANIMATION_CLIP = new CnbAssetType(6);

    /** A curve. */
    public static final CnbAssetType CURVE = new CnbAssetType(7);

    /** A sound effect. */
    public static final CnbAssetType SOUND_EFFECT = new CnbAssetType(8);

    /** A song. */
    public static final CnbAssetType SONG = new CnbAssetType(9);

    /** A video. */
    public static final CnbAssetType VIDEO = new CnbAssetType(10);

    /** An effect. */
    public static final CnbAssetType EFFECT = new CnbAssetType(11);

    /**
     * Mints the custom asset type a game-defined type name gets.
     *
     * <p><strong>This does not look a name up; it hashes it.</strong> The identifier is the
     * name's FNV-1a-32 hash with the custom-range bit set, so two different names can in
     * principle collide -- which is exactly why the optional metadata chunk carries the type name
     * as well, so a loader can compare the name and report a mismatch rather than decode the
     * wrong asset. Passing one of CNA's own type names here yields a custom identifier, not
     * {@link #TEXTURE_2D}.
     *
     * @param typeName the game's own type name, such as {@code "MyGame.Level"}; never empty
     * @return the minted custom identifier
     */
    public static CnbAssetType custom(String typeName) {
        CnbExtension.requireAvailable();
        int[] id = new int[1];
        CnbExtension.check("CnbAssetType.custom", NativeCnbRoutes.cnbAssetTypeIdFromName(
                CnbExtension.utf8(typeName), id));
        return new CnbAssetType(id[0]);
    }

    /** Reports whether the identifier belongs to a game's own schema rather than to CNA. */
    public boolean isCustom() {
        CnbExtension.requireAvailable();
        boolean[] custom = new boolean[1];
        CnbExtension.check("CnbAssetType.isCustom",
                NativeCnbRoutes.cnbIsCustomAssetTypeId(Id, custom));
        return custom[0];
    }

    /**
     * Returns CNA's own name for the type.
     *
     * @return the name, empty for a custom type CNA has no name for
     */
    public String getName() {
        CnbExtension.requireAvailable();
        return CnbExtension.text("CnbAssetType.getName",
                bytes -> NativeCnbRoutes.cnbGetAssetTypeNameSize(Id, bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbCopyAssetTypeName(Id, destination, bytes));
    }
}
