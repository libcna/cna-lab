package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.Objects;

/** Encodes a whole asset as a {@code .cnb} file, using CNA's own writer for that asset. */
public final class Cnb {

    private Cnb() {
    }

    /**
     * Encodes a texture as a complete Texture2D {@code .cnb} file.
     *
     * <p>This is CNA's own encoder, so the bytes carry the schema and the version CNA's reader
     * expects. A fixture built any other way would only prove that a hand-written writer and a
     * hand-written reader agree with each other.
     *
     * @param texture the texture to encode
     * @param contentName the source content name to record
     * @return the whole file
     */
    public static byte[] encodeTexture2D(CnbTextureData texture, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(texture, "texture");
        Objects.requireNonNull(contentName, "contentName");
        byte[] name = CnbExtension.utf8(contentName);
        long[] size = new long[1];
        // Ask with no buffer: CNA reports the size it needs and writes nothing, which is the
        // format's own two-call protocol rather than a guess at how big the file will be.
        int probe = NativeCnbRoutes.cnbEncodeTexture2d(
                texture.handle(), name, new byte[0], size);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("Cnb.encodeTexture2D", probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("Cnb.encodeTexture2D", NativeCnbRoutes
                .cnbEncodeTexture2d(texture.handle(), name, destination, written));
        return CnbExtension.trim(destination, written[0]);
    }

    /**
     * Encodes a sound as a complete SoundEffect {@code .cnb} file.
     *
     * <p>CNA's own encoder again, and it is stricter than the description: a loop region outside
     * the sound, a shape its samples do not fill, or a format the v1 container reserves but has
     * no codec for is refused here rather than written out for a reader to trip over.
     *
     * @param sound the sound to encode
     * @param contentName the source content name to record
     * @return the whole file
     * @throws CnbFormatException when the description and the samples disagree
     */
    public static byte[] encodeSoundEffect(CnbSoundEffectData sound, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(sound, "sound");
        Objects.requireNonNull(contentName, "contentName");
        byte[] name = CnbExtension.utf8(contentName);
        long[] size = new long[1];
        int probe = NativeCnbRoutes.cnbEncodeSoundEffect(
                sound.handle(), name, new byte[0], size);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("Cnb.encodeSoundEffect", probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("Cnb.encodeSoundEffect", NativeCnbRoutes
                .cnbEncodeSoundEffect(sound.handle(), name, destination, written));
        return CnbExtension.trim(destination, written[0]);
    }

    /**
     * Encodes a sprite font as a complete SpriteFont {@code .cnb} file.
     *
     * @param font the font to encode, atlas and glyph table together
     * @param contentName the source content name to record
     * @return the whole file
     * @throws CnbFormatException when the font has no atlas, or a glyph the format cannot carry
     */
    public static byte[] encodeSpriteFont(CnbSpriteFontData font, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(font, "font");
        Objects.requireNonNull(contentName, "contentName");
        byte[] name = CnbExtension.utf8(contentName);
        long[] size = new long[1];
        int probe = NativeCnbRoutes.cnbEncodeSpriteFont(
                font.handle(), name, new byte[0], size);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("Cnb.encodeSpriteFont", probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("Cnb.encodeSpriteFont", NativeCnbRoutes
                .cnbEncodeSpriteFont(font.handle(), name, destination, written));
        return CnbExtension.trim(destination, written[0]);
    }

    /**
     * Encodes a model as a complete Model {@code .cnb} file.
     *
     * <p>The encoder is where a model's description meets its payload: a part whose declared
     * vertex count and stride do not multiply out to the bytes it holds, a mesh naming a part
     * that does not exist, or a skeleton disagreeing with the bones is refused here rather than
     * written into a file something else would read.
     *
     * @param model the model to encode
     * @param contentName the source content name to record
     * @return the whole file
     * @throws CnbFormatException when the model's parts, meshes or skeleton disagree
     */
    public static byte[] encodeModel(CnbModelData model, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(model, "model");
        Objects.requireNonNull(contentName, "contentName");
        byte[] name = CnbExtension.utf8(contentName);
        long[] size = new long[1];
        int probe = NativeCnbRoutes.cnbEncodeModel(model.handle(), name, new byte[0], size);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("Cnb.encodeModel", probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("Cnb.encodeModel", NativeCnbRoutes
                .cnbEncodeModel(model.handle(), name, destination, written));
        return CnbExtension.trim(destination, written[0]);
    }

    /**
     * Encodes a cube texture as a complete TextureCube {@code .cnb} file.
     *
     * @param texture a texture with six faces, a depth of one and square levels
     * @param contentName the source content name to record
     * @return the whole file
     * @throws CnbFormatException when the texture is not shaped like a cube map
     */
    public static byte[] encodeTextureCube(CnbTextureData texture, String contentName) {
        return encode("Cnb.encodeTextureCube", texture, contentName,
                NativeCnbRoutes::cnbEncodeTextureCube);
    }

    /**
     * Encodes a volume texture as a complete Texture3D {@code .cnb} file.
     *
     * @param texture a texture with one face and any positive depth
     * @param contentName the source content name to record
     * @return the whole file
     * @throws CnbFormatException when the texture is not shaped like a volume
     */
    public static byte[] encodeTexture3D(CnbTextureData texture, String contentName) {
        return encode("Cnb.encodeTexture3D", texture, contentName,
                NativeCnbRoutes::cnbEncodeTexture3d);
    }

    /** The size-then-write protocol every texture encoder shares. */
    private static byte[] encode(
            String operation, CnbTextureData texture, String contentName, Encoder encoder) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(texture, "texture");
        Objects.requireNonNull(contentName, "contentName");
        byte[] name = CnbExtension.utf8(contentName);
        long[] size = new long[1];
        int probe = encoder.encode(texture.handle(), name, new byte[0], size);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check(operation, probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check(operation,
                encoder.encode(texture.handle(), name, destination, written));
        return CnbExtension.trim(destination, written[0]);
    }

    /** One of CNA's texture encoders, which all share the same shape. */
    private interface Encoder {
        int encode(long texture, byte[] contentName, byte[] destination, long[] written);
    }
}
