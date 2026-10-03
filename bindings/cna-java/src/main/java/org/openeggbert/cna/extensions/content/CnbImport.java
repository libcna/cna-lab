package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Color;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.nio.file.Path;
import java.util.Objects;

/**
 * Turns ordinary source assets into the compiled descriptions {@code .cnb} carries.
 *
 * <p>This is the ingest half of a content pipeline, and it is CNA's own: a PNG, a JPEG, a WAV or
 * a DDS cube map goes in, and what comes out is a {@link CnbTextureData} or a
 * {@link CnbSoundEffectData} that {@link Cnb} can write to a file or that a game can upload
 * directly. Nothing here touches a graphics or audio device.
 *
 * <p>It is what {@code docs/content-pipeline-decision.md} concluded should exist. XNA's Content
 * Pipeline is a build-time MSBuild system this projection deliberately does not reimplement;
 * being able to <em>read the formats artists actually hand over</em> is the part a Java game
 * genuinely needs, and CNA already had it.
 *
 * <p>Every method here reads the whole file or byte array and copies what it needs, so nothing
 * borrows the caller's array and the handle returned owns itself.
 */
public final class CnbImport {

    private CnbImport() {
    }

    /**
     * Decodes an image file into a single-level RGBA8 texture description.
     *
     * @param imagePath the PNG, JPEG or other format CNA's image loader decodes
     * @return the texture data, which the caller closes
     * @throws CnbFormatException when the file cannot be read or decoded, or decodes to nothing
     */
    public static CnbTextureData image(Path imagePath) {
        return image(imagePath, null);
    }

    /**
     * Decodes an image file, treating one colour as fully transparent.
     *
     * <p>The colour key is the content pipeline's oldest trick and XNA had it too: an artist
     * paints the background magenta and the importer turns those pixels transparent, because the
     * source format has no alpha channel to say so. Only the three colour channels are compared;
     * a pixel that matches becomes transparent black.
     *
     * @param imagePath the image to decode
     * @param colorKey the colour to make transparent, or null to keep every pixel
     * @return the texture data, which the caller closes
     * @throws CnbFormatException when the file cannot be read or decoded
     */
    public static CnbTextureData image(Path imagePath, Color colorKey) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(imagePath, "imagePath");
        byte[] key = new byte[3];
        if (colorKey != null) {
            key[0] = (byte) colorKey.getR();
            key[1] = (byte) colorKey.getG();
            key[2] = (byte) colorKey.getB();
        }
        long[] texture = new long[1];
        CnbExtension.check("CnbImport.image", NativeCnbRoutes.cnbImportImageAsTexture2d(
                CnbExtension.utf8(imagePath.toString()), key,
                new long[] {colorKey == null ? 0 : 1}, texture));
        return new CnbTextureData(texture[0]);
    }

    /**
     * Reads and decodes a WAV file as a compiled sound.
     *
     * @param wavPath the {@code .wav} to read
     * @return the sound data, which the caller closes
     * @throws CnbFormatException when the file is not a WAV, is truncated, or declares a format
     *         the container cannot carry
     */
    public static CnbSoundEffectData wav(Path wavPath) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(wavPath, "wavPath");
        long[] sound = new long[1];
        CnbExtension.check("CnbImport.wav", NativeCnbRoutes
                .cnbImportWavAsSoundEffect(CnbExtension.utf8(wavPath.toString()), sound));
        return new CnbSoundEffectData(sound[0]);
    }

    /**
     * Decodes WAV bytes already in memory.
     *
     * @param bytes the complete file contents
     * @param origin text naming the source in diagnostics, which may be empty
     * @return the sound data, which the caller closes
     * @throws CnbFormatException when the bytes are not a usable WAV
     */
    public static CnbSoundEffectData wav(byte[] bytes, String origin) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(bytes, "bytes");
        Objects.requireNonNull(origin, "origin");
        long[] sound = new long[1];
        CnbExtension.check("CnbImport.wav", NativeCnbRoutes.cnbDecodeWavAsSoundEffect(
                bytes, CnbExtension.utf8(origin), sound));
        return new CnbSoundEffectData(sound[0]);
    }

    /**
     * Decodes a DDS cube map file into a texture description with six faces.
     *
     * @param ddsPath the {@code .dds} to read
     * @return the texture data, which the caller closes
     * @throws CnbFormatException when the file is malformed, is not a cube map, or uses a format
     *         the container cannot carry
     */
    public static CnbTextureData ddsCube(Path ddsPath) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(ddsPath, "ddsPath");
        long[] texture = new long[1];
        CnbExtension.check("CnbImport.ddsCube", NativeCnbRoutes
                .cnbImportDdsAsTextureCube(CnbExtension.utf8(ddsPath.toString()), texture));
        return new CnbTextureData(texture[0]);
    }

    /**
     * Decodes DDS cube map bytes already in memory.
     *
     * @param bytes the complete file contents
     * @param origin text naming the source in diagnostics, which may be empty
     * @return the texture data, which the caller closes
     * @throws CnbFormatException when the bytes are not a usable DDS cube map
     */
    public static CnbTextureData ddsCube(byte[] bytes, String origin) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(bytes, "bytes");
        Objects.requireNonNull(origin, "origin");
        long[] texture = new long[1];
        CnbExtension.check("CnbImport.ddsCube", NativeCnbRoutes.cnbDecodeDdsAsTextureCube(
                bytes, CnbExtension.utf8(origin), texture));
        return new CnbTextureData(texture[0]);
    }
}
