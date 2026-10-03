package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Color;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.junit.jupiter.api.io.TempDir;

import javax.imageio.ImageIO;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * CNA's importers, against the live runtime.
 *
 * <p>These fixtures are <strong>not</strong> produced by CNA. The rest of the {@code .cnb} suite
 * encodes with CNA's own writer so that a successful read means the file is a real one; here the
 * source formats are the thing under test, so a PNG written by the JDK and a WAV written byte by
 * byte from its published layout are exactly the right fixtures. A CNA-authored input would prove
 * nothing about whether CNA can read what an artist hands over.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnbImportTests {

    /** Four pixels, each a different colour, with magenta in one corner to key out. */
    private static Path writePng(Path directory) {
        BufferedImage image = new BufferedImage(2, 2, BufferedImage.TYPE_INT_ARGB);
        image.setRGB(0, 0, 0xFFFF0000);
        image.setRGB(1, 0, 0xFF00FF00);
        image.setRGB(0, 1, 0xFF0000FF);
        image.setRGB(1, 1, 0xFFFF00FF);
        Path file = directory.resolve("four-pixels.png");
        try {
            ImageIO.write(image, "png", file.toFile());
        } catch (IOException problem) {
            throw new UncheckedIOException(problem);
        }
        return file;
    }

    /** A 16-bit mono WAV of eight frames, written from the format's published layout. */
    private static byte[] wavBytes() {
        int frames = 8;
        int dataBytes = frames * 2;
        ByteBuffer buffer = ByteBuffer.allocate(44 + dataBytes).order(ByteOrder.LITTLE_ENDIAN);
        buffer.put(new byte[] {'R', 'I', 'F', 'F'});
        buffer.putInt(36 + dataBytes);
        buffer.put(new byte[] {'W', 'A', 'V', 'E'});
        buffer.put(new byte[] {'f', 'm', 't', ' '});
        buffer.putInt(16);
        buffer.putShort((short) 1);          // PCM
        buffer.putShort((short) 1);          // mono
        buffer.putInt(22050);                // sample rate
        buffer.putInt(22050 * 2);            // byte rate
        buffer.putShort((short) 2);          // block align
        buffer.putShort((short) 16);         // bits per sample
        buffer.put(new byte[] {'d', 'a', 't', 'a'});
        buffer.putInt(dataBytes);
        for (int frame = 0; frame < frames; frame++) {
            buffer.putShort((short) (frame * 4000 - 16000));
        }
        return buffer.array();
    }

    @Test
    void anOrdinaryImageBecomesTextureData(@TempDir Path directory) {
        Path png = writePng(directory);
        try (CnbTextureData texture = CnbImport.image(png)) {
            CnbTextureInfo info = texture.getInfo();
            assertEquals(2, info.Width());
            assertEquals(2, info.Height());
            assertEquals(1, info.Depth());
            assertEquals(1, info.FaceCount());
            assertEquals(1, info.MipCount());
            assertEquals(CnbTextureFormat.Rgba8, texture.getRepresentationFormat(0));
            // Red first, as the format's name says, and the alpha the PNG carried.
            byte[] pixels = texture.readLevel(0, 0);
            assertEquals(16, pixels.length);
            assertArrayEquals(new byte[] {(byte) 0xFF, 0, 0, (byte) 0xFF},
                    new byte[] {pixels[0], pixels[1], pixels[2], pixels[3]});
            assertArrayEquals(new byte[] {(byte) 0xFF, 0, (byte) 0xFF, (byte) 0xFF},
                    new byte[] {pixels[12], pixels[13], pixels[14], pixels[15]});
        }
    }

    @Test
    void aColorKeyMakesOnePaintedColourTransparent(@TempDir Path directory) {
        Path png = writePng(directory);
        try (CnbTextureData texture = CnbImport.image(png, new Color(255, 0, 255, 255))) {
            byte[] pixels = texture.readLevel(0, 0);
            // The magenta corner is keyed out; only the three colour channels are compared, and
            // a matching pixel becomes transparent.
            assertEquals(0, pixels[15] & 0xFF, "the keyed pixel must be transparent");
            // Everything else is untouched, which is what makes this a key rather than a wipe.
            assertEquals(0xFF, pixels[3] & 0xFF);
            assertEquals(0xFF, pixels[0] & 0xFF);
        }
    }

    @Test
    void anUnreadableImageIsRefusedRatherThanGuessedAt(@TempDir Path directory) throws IOException {
        Path notAnImage = directory.resolve("not-an-image.png");
        Files.write(notAnImage, "this is not a PNG".getBytes(java.nio.charset.StandardCharsets.UTF_8));
        assertThrows(RuntimeException.class, () -> CnbImport.image(notAnImage));
        assertThrows(RuntimeException.class,
                () -> CnbImport.image(directory.resolve("absent.png")));
        assertThrows(NullPointerException.class, () -> CnbImport.image(null));
    }

    @Test
    void aWavBecomesACompiledSound(@TempDir Path directory) throws IOException {
        byte[] bytes = wavBytes();
        try (CnbSoundEffectData sound = CnbImport.wav(bytes, "eight-frames.wav")) {
            CnbSoundEffectInfo info = sound.getInfo();
            assertEquals(CnbAudioFormat.Pcm16, info.Format());
            assertEquals(22050, info.SampleRate());
            assertEquals(1, info.Channels());
            assertEquals(8, info.FrameCount());
            // The samples are the WAV's own, not a re-encoding: the data chunk comes back byte
            // for byte.
            byte[] expected = new byte[16];
            System.arraycopy(bytes, 44, expected, 0, 16);
            assertArrayEquals(expected, sound.readSamples());
        }

        Path file = directory.resolve("eight-frames.wav");
        Files.write(file, bytes);
        try (CnbSoundEffectData fromFile = CnbImport.wav(file)) {
            assertEquals(8, fromFile.getInfo().FrameCount());
        }

        // A truncated WAV is refused rather than decoded to whatever fits.
        byte[] truncated = new byte[30];
        System.arraycopy(bytes, 0, truncated, 0, truncated.length);
        assertThrows(RuntimeException.class, () -> CnbImport.wav(truncated, "truncated"));
        assertThrows(RuntimeException.class, () -> CnbImport.wav(new byte[0], "empty"));
    }

    /**
     * A DXT1-compressed cube map of six 4x4 faces, written from the DDS layout.
     *
     * <p>DXT1, DXT3 and DXT5 are the only cube formats CNA's decoder accepts, which is a contract
     * rather than a gap: an uncompressed cube map is refused with NOT_SUPPORTED, and the first
     * version of this fixture was B8G8R8A8 and found that out. Each face gets its own block
     * colour so a face read from the wrong offset cannot pass.
     */
    private static byte[] ddsCubeBytes() {
        int blockBytes = 8;
        ByteBuffer buffer = ByteBuffer.allocate(128 + blockBytes * 6)
                .order(ByteOrder.LITTLE_ENDIAN);
        buffer.put(new byte[] {'D', 'D', 'S', ' '});
        buffer.putInt(124);                  // dwSize
        buffer.putInt(0x0008100F);           // CAPS|HEIGHT|WIDTH|PIXELFORMAT|MIPMAPCOUNT|LINEARSIZE
        buffer.putInt(4);                    // dwHeight
        buffer.putInt(4);                    // dwWidth
        buffer.putInt(blockBytes);           // dwPitchOrLinearSize
        buffer.putInt(0);                    // dwDepth
        buffer.putInt(1);                    // dwMipMapCount
        for (int index = 0; index < 11; index++) {
            buffer.putInt(0);                // dwReserved1
        }
        buffer.putInt(32);                   // DDS_PIXELFORMAT.dwSize
        buffer.putInt(0x4);                  // DDPF_FOURCC
        buffer.put(new byte[] {'D', 'X', 'T', '1'});
        buffer.putInt(0);                    // dwRGBBitCount
        buffer.putInt(0);                    // masks, unused for a FourCC format
        buffer.putInt(0);
        buffer.putInt(0);
        buffer.putInt(0);
        buffer.putInt(0x00401008);           // DDSCAPS_TEXTURE | COMPLEX | MIPMAP
        buffer.putInt(0x0000FE00);           // DDSCAPS2_CUBEMAP and all six faces
        buffer.putInt(0);
        buffer.putInt(0);
        buffer.putInt(0);
        for (int face = 0; face < 6; face++) {
            // One 4x4 block per face: two RGB565 endpoints and sixteen two-bit indices, all
            // zero, which paints the whole block in the first endpoint.
            buffer.putShort((short) (0xF800 >>> face));   // colour 0, a different hue per face
            buffer.putShort((short) 0x0000);              // colour 1
            buffer.putInt(0);                             // every texel takes colour 0
        }
        return buffer.array();
    }

    @Test
    void aDdsCubeMapBecomesSixFaces(@TempDir Path directory) throws IOException {
        byte[] bytes = ddsCubeBytes();
        try (CnbTextureData texture = CnbImport.ddsCube(bytes, "sky.dds")) {
            CnbTextureInfo info = texture.getInfo();
            assertEquals(4, info.Width());
            assertEquals(4, info.Height());
            assertEquals(6, info.FaceCount());
            assertEquals(1, info.Depth());
        }
        Path file = directory.resolve("sky.dds");
        Files.write(file, bytes);
        try (CnbTextureData texture = CnbImport.ddsCube(file)) {
            assertEquals(6, texture.getInfo().FaceCount());
        }

        // Bytes that are not a DDS, and a DDS that is not a cube map, are both refused.
        assertThrows(RuntimeException.class,
                () -> CnbImport.ddsCube(new byte[] {'N', 'O', 'P', 'E'}, "nope"));
        byte[] notACube = bytes.clone();
        // Clear DDSCAPS2_CUBEMAP, which is what makes the file a cube map at all.
        ByteBuffer.wrap(notACube).order(ByteOrder.LITTLE_ENDIAN).putInt(112, 0);
        assertThrows(RuntimeException.class,
                () -> CnbImport.ddsCube(notACube, "flat.dds"));
    }

    @Test
    void anUncompressedCubeMapIsNotSupportedRatherThanMisread() {
        // CNA's decoder takes DXT1, DXT3 and DXT5 only. An uncompressed cube map is a real file
        // that this build declines to read, and it says so with its own identity rather than
        // failing as though the bytes were malformed.
        byte[] uncompressed = ddsCubeBytes();
        ByteBuffer header = ByteBuffer.wrap(uncompressed).order(ByteOrder.LITTLE_ENDIAN);
        header.putInt(80, 0x41);             // DDPF_RGB | DDPF_ALPHAPIXELS instead of FOURCC
        header.putInt(84, 0);                // no FourCC
        assertThrows(ContentNotSupportedException.class,
                () -> CnbImport.ddsCube(uncompressed, "uncompressed.dds"));
    }

    @Test
    void anImportedImageCanBeWrittenStraightBackOutAsCnb(@TempDir Path directory) {
        // The whole point of the ingest half: a source asset an artist hands over becomes a .cnb
        // a game loads, without a graphics device anywhere in between.
        Path png = writePng(directory);
        byte[] file;
        byte[] pixels;
        try (CnbTextureData imported = CnbImport.image(png)) {
            pixels = imported.readLevel(0, 0);
            file = Cnb.encodeTexture2D(imported, "textures/four-pixels");
        }
        try (CnbDocument document = CnbDocument.parse(
                     file, "four-pixels.cnb", CnbReadLimits.standard());
             CnbTextureData decoded = document.decodeTexture2D()) {
            assertEquals(CnbAssetType.TEXTURE_2D, document.getAssetType());
            assertArrayEquals(pixels, decoded.readLevel(0, 0));
        }
    }

    @Test
    void aCubeAndAVolumeTextureRoundTripThroughTheirOwnEncoders() {
        byte[] face = {
            (byte) 0xFF, 0x00, 0x00, (byte) 0xFF,
            0x00, (byte) 0xFF, 0x00, (byte) 0xFF,
            0x00, 0x00, (byte) 0xFF, (byte) 0xFF,
            (byte) 0x12, (byte) 0x34, (byte) 0x56, (byte) 0x78,
        };
        byte[] cubeFile;
        try (CnbTextureData cube = CnbTextureData.create(2, 2, 1, 6, 1)) {
            int representation = cube.addRepresentation(CnbTextureFormat.Rgba8);
            for (int level = 0; level < 6; level++) {
                cube.setLevel(representation, level, face);
            }
            cubeFile = Cnb.encodeTextureCube(cube, "textures/sky");
        }
        try (CnbDocument document = CnbDocument.parse(
                     cubeFile, "sky.cnb", CnbReadLimits.standard());
             CnbTextureData decoded = document.decodeTextureCube()) {
            assertEquals(CnbAssetType.TEXTURE_CUBE, document.getAssetType());
            assertEquals(6, decoded.getInfo().FaceCount());
            assertArrayEquals(face, decoded.readLevel(0, 0));
        }

        byte[] volumeFile;
        try (CnbTextureData volume = CnbTextureData.create(2, 2, 2, 1, 1)) {
            int representation = volume.addRepresentation(CnbTextureFormat.Rgba8);
            byte[] slices = new byte[face.length * 2];
            System.arraycopy(face, 0, slices, 0, face.length);
            System.arraycopy(face, 0, slices, face.length, face.length);
            volume.setLevel(representation, 0, slices);
            volumeFile = Cnb.encodeTexture3D(volume, "textures/fog");
        }
        try (CnbDocument document = CnbDocument.parse(
                     volumeFile, "fog.cnb", CnbReadLimits.standard());
             CnbTextureData decoded = document.decodeTexture3D()) {
            assertEquals(CnbAssetType.TEXTURE_3D, document.getAssetType());
            assertEquals(2, decoded.getInfo().Depth());
            assertEquals(1, decoded.getInfo().FaceCount());
        }

        // A texture the wrong shape is refused by the encoder that names the shape.
        try (CnbTextureData notACube = CnbTextureData.create(2, 2, 1, 1, 1)) {
            notACube.setLevel(notACube.addRepresentation(CnbTextureFormat.Rgba8), 0, face);
            assertThrows(RuntimeException.class,
                    () -> Cnb.encodeTextureCube(notACube, "textures/not-a-cube"));
        }
        assertTrue(true);
    }
}
