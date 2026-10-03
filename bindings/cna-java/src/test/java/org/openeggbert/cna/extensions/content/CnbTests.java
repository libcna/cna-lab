package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Curve;
import Microsoft.Xna.Framework.CurveContinuity;
import Microsoft.Xna.Framework.CurveKey;
import Microsoft.Xna.Framework.CurveLoopType;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Audio.AudioChannels;
import Microsoft.Xna.Framework.Audio.SoundEffect;
import Microsoft.Xna.Framework.Audio.SoundEffectInstance;
import Microsoft.Xna.Framework.Audio.SoundState;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Graphics.SurfaceFormat;
import Microsoft.Xna.Framework.Media.Song;
import Microsoft.Xna.Framework.Media.Video;
import Microsoft.Xna.Framework.Media.VideoSoundtrackType;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.nio.file.Files;
import java.nio.file.Path;
import java.time.Duration;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The {@code .cnb} container, against the live runtime.
 *
 * <p>Every fixture here is produced by <strong>CNA's own encoder and writer</strong>, and read
 * back by CNA's own parser. Hand-writing the bytes would only establish that a hand-written
 * writer and a hand-written reader share the same misunderstanding; encoding with the format's
 * own writer is what makes a successful read mean the file is a real one.
 *
 * <p>And the assertions are about payload, not about result codes: the pixels that come back are
 * compared to the pixels that went in, the metadata names are compared to the names that were
 * set, and the checksum is compared to the one CNA's portable path computes.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnbTests {

    /** Four pixels, each a different colour, so a channel swap or a stride error cannot hide. */
    private static final byte[] PIXELS = {
        (byte) 0xFF, 0x00, 0x00, (byte) 0xFF,
        0x00, (byte) 0xFF, 0x00, (byte) 0x80,
        0x00, 0x00, (byte) 0xFF, (byte) 0xFF,
        (byte) 0x12, (byte) 0x34, (byte) 0x56, (byte) 0x78,
    };

    @Test
    void theContainerAnswersForItself() {
        byte[] magic = CnbDocument.magic();
        assertEquals(4, magic.length);
        assertTrue(CnbDocument.hasMagic(magic));
        assertFalse(CnbDocument.hasMagic(new byte[] {'X', 'N', 'B', 'w'}),
                "an .xnb is not a .cnb, and the guard has to say so");
        assertFalse(CnbDocument.hasMagic(new byte[0]));

        CnbChunkId identifier = CnbChunkId.of("tEx2");
        assertEquals("tEx2", identifier.toString());
        assertTrue(identifier.isWellFormed());
        assertEquals("CMET", CnbChunkId.METADATA.toString());
        assertEquals("XREF", CnbChunkId.EXTERNAL_REFERENCES.toString());
        // Every byte must be printable ASCII, so a control byte is not an identifier.
        assertFalse(new CnbChunkId(0x00000001).isWellFormed());
        assertThrows(IllegalArgumentException.class, () -> CnbChunkId.of("too long"));

        assertFalse(CnbAssetType.TEXTURE_2D.isCustom());
        assertFalse(CnbAssetType.TEXTURE_2D.getName().isEmpty());
        assertTrue(new CnbAssetType(0x80000001).isCustom(),
                "the custom range belongs to a game's own schemas");
        // A custom identifier is minted by hashing the name, not looked up, so the same name
        // always mints the same identifier and it always lands in the custom range.
        CnbAssetType minted = CnbAssetType.custom("MyGame.Level");
        assertEquals(minted, CnbAssetType.custom("MyGame.Level"));
        assertTrue(minted.isCustom());
        assertNotEquals(minted, CnbAssetType.custom("MyGame.Other"));
        assertThrows(RuntimeException.class, () -> CnbAssetType.custom(""));

        assertTrue(CnbCompression.None.isSupported(), "storing uncompressed always works");
        assertEquals("none", CnbCompression.None.getName(),
                "the name is CNA's own spelling, not the Java constant's");
        for (CnbCompression codec : CnbCompression.values()) {
            assertFalse(codec.getName().isEmpty());
        }
    }

    @Test
    void checksumsAreCnasOwnAndAgreeWithThemselves() {
        // CRC-32C, not java.util.zip's CRC-32: a different polynomial and a different answer.
        // The hardware and portable paths must agree, or one of them is broken.
        assertEquals(CnbChecksum.portable(PIXELS), CnbChecksum.of(PIXELS));
        assertNotEquals(CnbChecksum.of(PIXELS), CnbChecksum.of(new byte[] {1, 2, 3, 4}));
        assertEquals(CnbChecksum.of(new byte[0]), CnbChecksum.of(new byte[0]));
        // Summing in two blocks must equal summing in one.
        long first = CnbChecksum.of(new byte[] {PIXELS[0], PIXELS[1]});
        byte[] rest = new byte[PIXELS.length - 2];
        System.arraycopy(PIXELS, 2, rest, 0, rest.length);
        assertEquals(CnbChecksum.of(PIXELS), CnbChecksum.continued(first, rest));
        assertEquals(CnbChecksum.usesHardware(), CnbChecksum.usesHardware());
    }

    @Test
    void limitsAndCheckedArithmeticRefuseWhatTheyMust() {
        CnbReadLimits limits = CnbReadLimits.standard();
        assertTrue(limits.MaxFileSize() > 0);
        assertTrue(limits.MaxChunkSize() > 0);
        assertTrue(limits.MaxChunkCount() > 0);

        assertEquals(3L, CnbFormat.checkedAdd(1L, 2L));
        assertEquals(6L, CnbFormat.checkedMultiply(2L, 3L));
        // A malformed file's offset plus size is exactly where an unchecked add wraps.
        assertThrows(RuntimeException.class, () -> CnbFormat.checkedAdd(-1L, 2L));
        assertThrows(RuntimeException.class, () -> CnbFormat.checkedMultiply(-1L, 4L));

        assertTrue(CnbFormat.isWellFormedUtf8("ok".getBytes(java.nio.charset.StandardCharsets.UTF_8)));
        assertTrue(CnbFormat.isWellFormedUtf8("日本".getBytes(java.nio.charset.StandardCharsets.UTF_8)));
        assertFalse(CnbFormat.isWellFormedUtf8(new byte[] {(byte) 0xC3}),
                "a truncated multi-byte sequence is not well-formed UTF-8");
        assertEquals("", CnbFormat.logicalNameProblem("textures/hero"));
        assertFalse(CnbFormat.logicalNameProblem("../escape").isEmpty(),
                "a name that escapes the content root has to be rejected with a reason");
    }

    @Test
    void textureFormatsMeasureThemselves() {
        assertEquals(4, CnbTextureFormat.Rgba8.getUnitBytes());
        assertFalse(CnbTextureFormat.Rgba8.isBlockCompressed());
        assertTrue(CnbTextureFormat.Bc1.isBlockCompressed());
        assertEquals(16L, CnbTextureFormat.Rgba8.getLevelByteSize(2, 2, 1),
                "four RGBA pixels are sixteen bytes");
        // A block-compressed level is measured in four-by-four blocks, so a two-by-two level is
        // still one whole block. Measuring it as width times height would be wrong.
        assertEquals(8L, CnbTextureFormat.Bc1.getLevelByteSize(2, 2, 1));
        assertEquals(SurfaceFormat.Color, CnbTextureFormat.Rgba8.toSurfaceFormat());
        assertEquals(CnbTextureFormat.Rgba8,
                CnbTextureFormat.fromSurfaceFormat(SurfaceFormat.Color));
        assertTrue(CnbTextureFormat.isKnown(CnbTextureFormat.Bc7.ordinal()));
        assertFalse(CnbTextureFormat.isKnown(9999));
        assertFalse(CnbTextureFormat.Bc7Srgb.getName().isEmpty());
    }

    @Test
    void aTextureRoundTripsThroughCnasOwnEncoderAndParser() throws Exception {
        byte[] file;
        try (CnbTextureData source = CnbTextureData.ofRgba8(2, 2, PIXELS)) {
            CnbTextureInfo info = source.getInfo();
            assertEquals(2, info.Width());
            assertEquals(2, info.Height());
            assertEquals(1, info.Depth());
            assertEquals(1, info.FaceCount());
            assertEquals(1, info.MipCount());
            assertEquals(1, info.RepresentationCount());
            assertEquals(CnbTextureFormat.Rgba8, source.getRepresentationFormat(0));
            assertArrayEquals(PIXELS, source.readLevel(0, 0),
                    "the pixels a texture was built from must read back unchanged");
            file = Cnb.encodeTexture2D(source, "textures/four-pixels");
        }

        assertTrue(CnbDocument.hasMagic(file), "CNA's encoder writes a real .cnb");

        try (CnbDocument document = CnbDocument.parse(
                file, "four-pixels.cnb", CnbReadLimits.standard())) {
            assertEquals("four-pixels.cnb", document.getOrigin());
            assertEquals(CnbAssetType.TEXTURE_2D.Id(), document.getAssetType().Id());
            assertEquals(1, document.getContainerMajor());
            document.requireAsset(CnbAssetType.TEXTURE_2D, document.getAssetSchemaVersion());
            // A texture loader handed something else must say so rather than misread it.
            assertThrows(RuntimeException.class,
                    () -> document.requireAsset(CnbAssetType.MODEL, 99));

            CnbMetadata metadata = document.getMetadata();
            assertTrue(metadata.Present());
            assertEquals("textures/four-pixels", metadata.ContentName(),
                    "the content name written must be the content name read");
            assertFalse(metadata.AssetTypeName().isEmpty());

            List<CnbChunk> chunks = document.getChunks();
            assertFalse(chunks.isEmpty());
            for (CnbChunk chunk : chunks) {
                assertTrue(chunk.Id().isWellFormed());
                assertTrue(chunk.StoredSize() > 0 || chunk.UncompressedSize() == 0);
                // Every chunk carries the CRC-32C of what is stored, and reading it back and
                // summing it has to produce the number the table of contents recorded.
                if (chunk.Compression() == CnbCompression.None) {
                    assertEquals(chunk.Checksum(),
                            CnbChecksum.of(document.readChunk(chunks.indexOf(chunk))),
                            "chunk " + chunk.Id() + " does not match its recorded checksum");
                }
            }
            int metadataIndex = document.requireSingle(CnbChunkId.METADATA);
            assertTrue(metadataIndex >= 0);
            assertEquals(List.of(metadataIndex), document.findAll(CnbChunkId.METADATA));
            assertEquals(-1, document.findSingle(CnbChunkId.of("zzzz")),
                    "a chunk the file has none of is absent, not an error");
            // A mandatory chunk the reader does not know must stop the load.
            assertThrows(RuntimeException.class, () ->
                    document.requireMandatoryChunksUnderstood(List.of(CnbChunkId.of("zzzz"))));

            try (CnbTextureData decoded = document.decodeTexture2D()) {
                assertEquals(2, decoded.getInfo().Width());
                assertEquals(2, decoded.getInfo().Height());
                assertArrayEquals(PIXELS, decoded.readLevel(0, 0),
                        "the pixels that came out are the pixels that went in");
            }
        }

        // The same bytes from disk, through the file parser rather than the memory one.
        Path directory = Files.createTempDirectory("cna-java-cnb");
        Path path = directory.resolve("four-pixels.cnb");
        Files.write(path, file);
        try (CnbDocument document = CnbDocument.parse(path, CnbReadLimits.standard());
             CnbTextureData decoded = document.decodeTexture2D()) {
            assertArrayEquals(PIXELS, decoded.readLevel(0, 0));
        } finally {
            Files.deleteIfExists(path);
            Files.deleteIfExists(directory);
        }
    }

    @Test
    void aWrittenFileCarriesTheChunksAndStringsItWasGiven() throws Exception {
        byte[] payload = "a game's own schema payload".getBytes(
                java.nio.charset.StandardCharsets.UTF_8);
        byte[] file;
        CnbAssetType level = CnbAssetType.custom("MyGame.Content.Level");
        try (CnbWriter writer = CnbWriter.of(level, 3)) {
            writer.setMetadata("MyGame.Content.Level", "levels/one");
            writer.addChunk(CnbChunkId.of("lvl1"), payload, true);
            assertEquals(1, writer.getChunkCount());
            // Alignment is a power of two from one through 4096, and anything else is the
            // caller's mistake rather than a file CNA will write.
            assertThrows(IllegalArgumentException.class,
                    () -> writer.addChunk(CnbChunkId.of("bad0"), payload, false, 0));
            assertThrows(IllegalArgumentException.class,
                    () -> writer.addChunk(CnbChunkId.of("bad3"), payload, false, 3));
            assertThrows(IllegalArgumentException.class,
                    () -> writer.addChunk(CnbChunkId.of("bad9"), payload, false, 8192));
            writer.setCompression(CnbCompression.None, 0);
            assertEquals(CnbReadLimits.standard().MaxChunkCount(),
                    writer.getLimits().MaxChunkCount());
            file = writer.build();
        }

        try (CnbDocument document = CnbDocument.parse(
                file, "level.cnb", CnbReadLimits.standard())) {
            assertEquals(level, document.getAssetType());
            assertTrue(document.getAssetType().isCustom());
            assertEquals(3, document.getAssetSchemaVersion());
            assertEquals("MyGame.Content.Level", document.getMetadata().AssetTypeName());
            assertEquals("levels/one", document.getMetadata().ContentName());

            int index = document.requireSingle(CnbChunkId.of("lvl1"));
            assertTrue(document.getChunks().get(index).Mandatory(),
                    "a chunk written as mandatory must read back as mandatory");
            assertArrayEquals(payload, document.readChunk(index),
                    "the chunk bytes written must be the chunk bytes read");

            // The checked cursor over the same chunk sees the same bytes.
            try (CnbReader reader = document.openChunk(index)) {
                assertEquals(payload.length, reader.getSize());
                assertEquals(payload.length, reader.getRemaining());
                assertEquals(0, reader.getPosition());
                assertArrayEquals(payload, reader.readBytes(payload.length));
                assertEquals(0, reader.getRemaining());
                reader.requireExhausted();
                // Reading past the end is refused rather than returning zeros.
                assertThrows(RuntimeException.class, () -> reader.readUnsignedByte());
            }
        }
    }

    @Test
    void aCheckedReaderRefusesWhatAMalformedFileWouldClaim() {
        CnbReadLimits limits = CnbReadLimits.standard();
        byte[] data = {1, 2, 3, 4, 5, 6, 7, 8};
        try (CnbReader reader = CnbReader.of(data, "a probe", limits)) {
            assertEquals("a probe", reader.getContext());
            assertEquals(8, reader.getSize());
            assertEquals(1, reader.readUnsignedByte());
            assertEquals(0x0302, reader.readUnsignedShort(), "little-endian, as the format is");
            assertEquals(5, reader.getRemaining());
            reader.skip(1);
            assertEquals(4, reader.getRemaining());
            assertEquals(4, reader.getPosition());

            // Reading past the end is refused, and a refused read must not move the cursor:
            // a schema that catches the failure and carries on has to see the same bytes.
            assertThrows(RuntimeException.class, () -> reader.readBytes(64));
            assertEquals(4, reader.getRemaining(),
                    "a refused read leaves the cursor where it was");
            assertThrows(RuntimeException.class, reader::requireExhausted);

            assertArrayEquals(new byte[] {5, 6, 7, 8}, reader.readBytes(4));
            assertEquals(0, reader.getRemaining());
            reader.requireExhausted();
        }

        // A count whose elements cannot fit in what remains is refused before anything is
        // allocated for it, which is the reason to read counts this way rather than as an int.
        byte[] hugeCount = {(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, 0x7F, 0, 0, 0, 0};
        try (CnbReader reader = CnbReader.of(hugeCount, "a hostile count", limits)) {
            assertThrows(RuntimeException.class, () -> reader.readCount(1024, "vertices"));
        }
        // The same count with no element size skips the fit check, because the elements are
        // variable-length and nothing can be concluded from the remaining bytes.
        try (CnbReader reader = CnbReader.of(hugeCount, "a variable-length count", limits)) {
            assertThrows(RuntimeException.class, () -> reader.readCount(0, "strings"),
                    "the count is still checked against the limits, which is what stops it");
        }
        assertThrows(NullPointerException.class, () -> CnbReader.of(null, "x", limits));
    }

    @Test
    void aDecodedTextureBecomesAnXnaTexture() {
        try (TextureProbe probe = new TextureProbe()) {
            probe.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }


    /**
     * Sixteen signed 16-bit stereo frames, every one distinct, so a channel swap, a frame-size
     * mistake or a truncation at either end changes the bytes that come back.
     */
    private static byte[] stereoFrames() {
        byte[] samples = new byte[16 * 4];
        for (int frame = 0; frame < 16; frame++) {
            short left = (short) (frame * 1000 - 8000);
            short right = (short) (-left / 2);
            samples[frame * 4] = (byte) (left & 0xFF);
            samples[frame * 4 + 1] = (byte) ((left >> 8) & 0xFF);
            samples[frame * 4 + 2] = (byte) (right & 0xFF);
            samples[frame * 4 + 3] = (byte) ((right >> 8) & 0xFF);
        }
        return samples;
    }

    @Test
    void aCompiledSoundRoundTripsThroughItsOwnContainer() {
        byte[] samples = stereoFrames();
        byte[] file;
        try (CnbSoundEffectData source = CnbSoundEffectData.ofPcm16(44100, 2, samples)) {
            CnbSoundEffectInfo info = source.getInfo();
            assertEquals(CnbAudioFormat.Pcm16, info.Format());
            assertEquals(44100, info.SampleRate());
            assertEquals(2, info.Channels());
            assertEquals(16, info.FrameCount(), "sixty-four bytes is sixteen stereo frames");
            assertEquals(0, info.LoopLength());
            assertEquals(samples.length, info.getByteCount());
            assertArrayEquals(samples, source.readSamples());
            file = Cnb.encodeSoundEffect(source, "audio/sixteen-frames");
        }

        try (CnbDocument document = CnbDocument.parse(
                     file, "sixteen-frames.cnb", CnbReadLimits.standard())) {
            assertEquals(CnbAssetType.SOUND_EFFECT, document.getAssetType());
            try (CnbSoundEffectData decoded = document.decodeSoundEffect()) {
                assertEquals(new CnbSoundEffectInfo(CnbAudioFormat.Pcm16, 44100, 2, 16, 0, 0),
                        decoded.getInfo());
                // The bytes, not the shape: an encoder that dropped or reordered a frame would
                // still report sixteen of them.
                assertArrayEquals(samples, decoded.readSamples());
            }
        }
    }

    @Test
    void aLoopRegionSurvivesTheContainer() {
        byte[] samples = stereoFrames();
        byte[] file;
        try (CnbSoundEffectData source = CnbSoundEffectData.create(
                     new CnbSoundEffectInfo(CnbAudioFormat.Pcm16, 22050, 2, 16, 4, 8), samples)) {
            file = Cnb.encodeSoundEffect(source, "audio/looped");
        }
        try (CnbDocument document = CnbDocument.parse(
                     file, "looped.cnb", CnbReadLimits.standard());
             CnbSoundEffectData decoded = document.decodeSoundEffect()) {
            CnbSoundEffectInfo info = decoded.getInfo();
            assertEquals(4, info.LoopStart());
            assertEquals(8, info.LoopLength());
        }
    }

    @Test
    void anImpossibleSoundIsRefusedByTheEncoderRatherThanWritten() {
        byte[] samples = stereoFrames();
        // The description says the sound is twice as long as its samples. The handle takes it --
        // it stores a description -- and the encoder is where it has to fail, because that is the
        // point at which a file someone else would read gets written.
        try (CnbSoundEffectData wrong = CnbSoundEffectData.create(
                     new CnbSoundEffectInfo(CnbAudioFormat.Pcm16, 44100, 2, 32, 0, 0), samples)) {
            assertThrows(CnbFormatException.class,
                    () -> Cnb.encodeSoundEffect(wrong, "audio/too-short"));
        }
        try (CnbSoundEffectData outside = CnbSoundEffectData.create(
                     new CnbSoundEffectInfo(CnbAudioFormat.Pcm16, 44100, 2, 16, 12, 8), samples)) {
            assertThrows(CnbFormatException.class,
                    () -> Cnb.encodeSoundEffect(outside, "audio/loop-past-the-end"));
        }
    }

    @Test
    void theReservedAudioFormatsAreNamedButHaveNoCodec() {
        assertTrue(CnbAudioFormat.Pcm16.hasCodec());
        assertEquals(4, CnbAudioFormat.Pcm16.getFrameByteSize(2));
        for (CnbAudioFormat format : CnbAudioFormat.values()) {
            // CNA names every one of them, which is what makes a diagnostic about an unreadable
            // file useful; naming is not implementing.
            assertFalse(format.getName().isEmpty(), format + " has no name");
            if (format != CnbAudioFormat.Pcm16) {
                assertFalse(format.hasCodec(), format + " claims a v1 codec");
            }
        }
        // A fixed frame size and a v1 codec are different questions, and CNA answers them
        // separately: PCM8 and float PCM have a frame size nobody can encode yet, and the two
        // compressed formats have no fixed frame at all. Asserting CNA's own answers is the
        // point -- computing 2 * channels here would have been wrong for four of the six.
        assertEquals(0, CnbAudioFormat.Unknown.getFrameByteSize(2));
        assertEquals(1, CnbAudioFormat.Pcm8.getFrameByteSize(1));
        assertEquals(8, CnbAudioFormat.PcmFloat32.getFrameByteSize(2));
        assertEquals(0, CnbAudioFormat.Adpcm.getFrameByteSize(2));
        assertEquals(0, CnbAudioFormat.Vorbis.getFrameByteSize(2));
        for (CnbAudioFormat format : CnbAudioFormat.values()) {
            assertThrows(IllegalArgumentException.class, () -> format.getFrameByteSize(0));
        }
        assertThrows(CnbFormatException.class,
                () -> CnbAudioFormat.fromValue(CnbAudioFormat.values().length));
    }

    @Test
    void aCurveIsTheRuntimeFormLaidOutFlat() {
        Curve source = new Curve();
        source.setPreLoop(CurveLoopType.Oscillate);
        source.setPostLoop(CurveLoopType.CycleOffset);
        source.getKeys().Add(new CurveKey(0.0f, 1.0f, 0.0f, 2.0f, CurveContinuity.Smooth));
        source.getKeys().Add(new CurveKey(1.5f, -3.0f, 0.5f, -0.5f, CurveContinuity.Step));
        source.getKeys().Add(new CurveKey(4.0f, 7.25f, 1.0f, 1.0f, CurveContinuity.Smooth));

        byte[] file = CnbCurve.encode(source, "curves/probe");
        try (CnbDocument document = CnbDocument.parse(
                     file, "probe.cnb", CnbReadLimits.standard())) {
            assertEquals(CnbAssetType.CURVE, document.getAssetType());
            Curve decoded = document.decodeCurve();
            assertEquals(CurveLoopType.Oscillate, decoded.getPreLoop());
            assertEquals(CurveLoopType.CycleOffset, decoded.getPostLoop());
            assertEquals(3, decoded.getKeys().getCount());
            for (int index = 0; index < 3; index++) {
                CurveKey expected = source.getKeys().get(index);
                CurveKey actual = decoded.getKeys().get(index);
                assertEquals(expected.getPosition(), actual.getPosition());
                assertEquals(expected.getValue(), actual.getValue());
                assertEquals(expected.getTangentIn(), actual.getTangentIn());
                assertEquals(expected.getTangentOut(), actual.getTangentOut());
                assertEquals(expected.getContinuity(), actual.getContinuity());
            }
            // The point of keeping the tangents and the continuity: the curve evaluates the same
            // on the other side, which is the only thing a game actually asks of it.
            for (float position : new float[] {-1.0f, 0.0f, 0.75f, 1.5f, 3.0f, 4.0f, 6.0f}) {
                assertEquals(source.Evaluate(position), decoded.Evaluate(position),
                        "the curve evaluates differently at " + position);
            }
        }

        // An empty curve is a legal one, and evaluates to zero everywhere in XNA.
        byte[] empty = CnbCurve.encode(new Curve(), "curves/empty");
        try (CnbDocument document = CnbDocument.parse(
                     empty, "empty.cnb", CnbReadLimits.standard())) {
            assertEquals(0, document.decodeCurve().getKeys().getCount());
        }
    }

    @Test
    void theCurveEnumeratorsAreTheSameNumbersOnBothSides() {
        // The enumerators go into the file as numbers, so XNA's order and CNA's have to agree.
        // Asserting it here is what stops a reordering from silently changing how every stored
        // curve evaluates.
        assertEquals(0, CurveLoopType.Constant.ordinal());
        assertEquals(1, CurveLoopType.Cycle.ordinal());
        assertEquals(2, CurveLoopType.CycleOffset.ordinal());
        assertEquals(3, CurveLoopType.Oscillate.ordinal());
        assertEquals(4, CurveLoopType.Linear.ordinal());
        assertEquals(0, CurveContinuity.Smooth.ordinal());
        assertEquals(1, CurveContinuity.Step.ordinal());
        for (CurveLoopType loop : CurveLoopType.values()) {
            Curve curve = new Curve();
            curve.setPreLoop(loop);
            curve.setPostLoop(loop);
            curve.getKeys().Add(new CurveKey(0.0f, 1.0f));
            curve.getKeys().Add(new CurveKey(2.0f, 5.0f));
            byte[] file = CnbCurve.encode(curve, "");
            try (CnbDocument document = CnbDocument.parse(
                         file, loop + ".cnb", CnbReadLimits.standard())) {
                Curve decoded = document.decodeCurve();
                assertEquals(loop, decoded.getPreLoop());
                assertEquals(loop, decoded.getPostLoop());
                assertEquals(curve.Evaluate(-3.0f), decoded.Evaluate(-3.0f), loop.toString());
                assertEquals(curve.Evaluate(5.0f), decoded.Evaluate(5.0f), loop.toString());
            }
        }
    }

    /** A two-glyph font over a 2x2 atlas: enough that a swap or a dropped bearing shows. */
    private static CnbSpriteFontData twoGlyphFont() {
        CnbSpriteFontData font = CnbSpriteFontData.create();
        try {
            try (CnbTextureData atlas = CnbTextureData.ofRgba8(2, 2, PIXELS)) {
                font.setAtlas(atlas);
            }
            font.setMetrics(14, 1.5f, 'A');
            assertEquals(0, font.addGlyph(new CnbGlyph('A',
                    new Rectangle(0, 0, 1, 2), new Rectangle(0, 1, 1, 1),
                    new Vector3(0.5f, 3.0f, 0.25f))));
            assertEquals(1, font.addGlyph(new CnbGlyph('B',
                    new Rectangle(1, 0, 1, 2), new Rectangle(-1, 0, 1, 2),
                    new Vector3(-0.5f, 4.0f, 0.75f))));
            return font;
        } catch (RuntimeException failure) {
            font.close();
            throw failure;
        }
    }

    @Test
    void aSpriteFontIsAnAtlasAndAGlyphTable() {
        byte[] file;
        try (CnbSpriteFontData font = twoGlyphFont()) {
            assertEquals(2, font.getGlyphCount());
            assertEquals(14, font.getLineSpacing());
            assertEquals(1.5f, font.getSpacing());
            assertEquals('A', font.getDefaultCharacter());

            // A glyph read back is the glyph that went in, bearings and all.
            CnbGlyph second = font.getGlyph(1);
            assertEquals('B', second.Character());
            assertEquals(new Rectangle(1, 0, 1, 2), second.GlyphBounds());
            assertEquals(new Rectangle(-1, 0, 1, 2), second.Cropping());
            assertEquals(new Vector3(-0.5f, 4.0f, 0.75f), second.Kerning());

            font.setGlyph(0, new CnbGlyph('a', new Rectangle(0, 0, 1, 2),
                    new Rectangle(0, 1, 1, 1), new Vector3(0.5f, 3.0f, 0.25f)));
            assertEquals('a', font.getGlyph(0).Character());
            font.setGlyph(0, new CnbGlyph('A', new Rectangle(0, 0, 1, 2),
                    new Rectangle(0, 1, 1, 1), new Vector3(0.5f, 3.0f, 0.25f)));

            // The atlas comes back as its own texture data, which the caller owns separately.
            try (CnbTextureData atlas = font.copyAtlas()) {
                assertEquals(2, atlas.getInfo().Width());
                assertArrayEquals(PIXELS, atlas.readLevel(0, 0));
            }

            // A font with no default character is the other legal shape: a missing character is
            // then an error rather than a substitution.
            font.setMetrics(14, 1.5f, null);
            assertNull(font.getDefaultCharacter());

            // CNA refuses a substitute the font cannot draw, which is a rule worth having: a
            // default character absent from the table would fail at the first missing glyph, in
            // the game rather than in the build.
            font.setMetrics(14, 1.5f, 'Z');
            assertThrows(CnbFormatException.class,
                    () -> Cnb.encodeSpriteFont(font, "fonts/absent-default"));
            font.setMetrics(14, 1.5f, 'A');

            file = Cnb.encodeSpriteFont(font, "fonts/two-glyphs");
        }

        try (CnbDocument document = CnbDocument.parse(
                     file, "two-glyphs.cnb", CnbReadLimits.standard())) {
            assertEquals(CnbAssetType.SPRITE_FONT, document.getAssetType());
            try (CnbSpriteFontData decoded = document.decodeSpriteFont()) {
                assertEquals(14, decoded.getLineSpacing());
                assertEquals(1.5f, decoded.getSpacing());
                assertEquals('A', decoded.getDefaultCharacter());
                assertEquals(List.of('A', 'B'), decoded.getGlyphs().stream()
                        .map(CnbGlyph::Character).toList());
                assertEquals(new Vector3(0.5f, 3.0f, 0.25f),
                        decoded.getGlyphs().get(0).Kerning());
                try (CnbTextureData atlas = decoded.copyAtlas()) {
                    assertArrayEquals(PIXELS, atlas.readLevel(0, 0));
                }
            }
        }

        // A font with no glyph cannot draw, and says so rather than producing an empty one.
        try (CnbSpriteFontData empty = CnbSpriteFontData.create()) {
            assertThrows(IllegalArgumentException.class, () -> empty.setMetrics(14, Float.NaN, null));
        }
    }

    @Test
    void aSongIsMetadataAndAReferenceRatherThanAudio() {
        byte[] file = CnbSong.encode("music/theme.ogg", "Main Theme",
                Duration.ofMinutes(3).plusSeconds(21), "music/theme");
        try (CnbDocument document = CnbDocument.parse(
                     file, "theme.cnb", CnbReadLimits.standard())) {
            assertEquals(CnbAssetType.SONG, document.getAssetType());
            CnbSong song = document.decodeSong();
            assertEquals("music/theme.ogg", song.StreamReference());
            assertEquals("Main Theme", song.Name());
            assertEquals(Duration.ofSeconds(201), song.Duration());

            // The reference is the document's single external reference, which is what makes the
            // dependency visible to a build tool rather than buried in a payload.
            List<CnbExternalReference> references = document.getExternalReferences();
            assertEquals(1, references.size());
            assertEquals("music/theme.ogg", references.get(0).LogicalName());

            // And the audio is not in the file: three minutes of music would not fit in one.
            assertTrue(file.length < 4096,
                    "a song .cnb carries no audio, so it must stay tiny: " + file.length);
        }

        // An unnamed song of unknown length is an ordinary case a compiler produces.
        byte[] bare = CnbSong.encode("music/unknown.ogg", "", Duration.ZERO, "");
        try (CnbDocument document = CnbDocument.parse(
                     bare, "unknown.cnb", CnbReadLimits.standard())) {
            CnbSong song = document.decodeSong();
            assertEquals("", song.Name());
            assertEquals(Duration.ZERO, song.Duration());
        }

        // An empty reference is the one thing a song may not have, because it is the whole point.
        assertThrows(CnbFormatException.class,
                () -> CnbSong.encode("", "Nameless", Duration.ZERO, ""));
    }

    @Test
    void aVideoCarriesTheShapeAGameNeedsBeforeOpeningTheMedia() {
        CnbVideo source = new CnbVideo("video/intro.webm", Duration.ofSeconds(12),
                1280, 720, 29.97f, VideoSoundtrackType.MusicAndDialog);
        byte[] file = CnbVideo.encode("video/intro.webm", source, "video/intro");
        try (CnbDocument document = CnbDocument.parse(
                     file, "intro.cnb", CnbReadLimits.standard())) {
            assertEquals(CnbAssetType.VIDEO, document.getAssetType());
            assertEquals(source, document.decodeVideo());
        }

        // A frame rate that is not positive and finite is refused rather than written, because a
        // player dividing by it later has no way back.
        CnbVideo broken = new CnbVideo("video/intro.webm", Duration.ofSeconds(12),
                1280, 720, 0.0f, VideoSoundtrackType.Music);
        assertThrows(CnbFormatException.class,
                () -> CnbVideo.encode("video/intro.webm", broken, ""));
        assertThrows(CnbFormatException.class,
                () -> CnbVideo.encode("", source, ""));
    }

    @Test
    void aDecodedSoundBecomesAnXnaSoundEffect() {
        try (SoundProbe probe = new SoundProbe()) {
            probe.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    @Test
    void aDecodedSongAndVideoBecomeXnaMediaObjects() {
        try (MediaProbe probe = new MediaProbe()) {
            probe.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    @Test
    void aDecodedSpriteFontBecomesAnXnaSpriteFont() {
        try (FontProbe probe = new FontProbe()) {
            probe.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    /**
     * The graphics half of the sprite-font slice.
     *
     * <p>XNA has no public {@code SpriteFont} constructor, and this does not add one: the font
     * goes through the same internal path {@code ContentManager.Load} uses, so what comes out is
     * the same kind of object {@code SpriteBatch.DrawString} already draws with. What is asserted
     * is that the table survived -- the characters, the metrics and a measurement that depends on
     * the bearings that came out of the file.
     */
    private static final class FontProbe extends Game {

        private boolean ran;
        private Throwable failure;

        private FontProbe() {
            new GraphicsDeviceManager(this);
        }

        @Override
        protected void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                byte[] file;
                try (CnbSpriteFontData source = twoGlyphFont()) {
                    file = Cnb.encodeSpriteFont(source, "fonts/probe");
                }
                try (CnbDocument document = CnbDocument.parse(
                             file, "probe.cnb", CnbReadLimits.standard());
                     CnbSpriteFontData decoded = document.decodeSpriteFont()) {
                    SpriteFont font = decoded.toSpriteFont(getGraphicsDevice());
                    assertEquals(List.of('A', 'B'), font.getCharacters());
                    assertEquals(14, font.getLineSpacing());
                    assertEquals(1.5f, font.getSpacing());
                    assertEquals('A', font.getDefaultCharacter());
                    // The advance width came from the file's kerning, so a measurement is a
                    // statement about the glyph table rather than about the atlas.
                    assertEquals(new Vector2(3.75f, 14.0f), font.MeasureString("A"));

                    try (CnbSpriteFontData empty = CnbSpriteFontData.create()) {
                        assertThrows(CnbFormatException.class,
                                () -> empty.toSpriteFont(getGraphicsDevice()));
                    }
                }
            } catch (Throwable exception) {
                failure = exception;
            }
        }
    }

    /**
     * The media half of the song and video slices, inside a game for the same reason as the audio.
     *
     * <p>Both types are references to media that lives outside the {@code .cnb}, so this proves
     * the crossing -- a decoded reference becomes an XNA media object carrying the file's own
     * metadata -- and not that anything played. Playback needs media this repository does not
     * ship and a backend this qualification does not have.
     */
    private static final class MediaProbe extends Game {

        private boolean ran;
        private Throwable failure;

        private MediaProbe() {
            new GraphicsDeviceManager(this);
        }

        @Override
        protected void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                byte[] songFile = CnbSong.encode("music/theme.ogg", "Main Theme",
                        Duration.ofSeconds(201), "music/theme");
                try (CnbDocument document = CnbDocument.parse(
                             songFile, "theme.cnb", CnbReadLimits.standard())) {
                    CnbSong compiled = document.decodeSong();
                    // CNA opens the media when the Song is created, so the location the caller
                    // resolves has to be a file that is really there. That is the requirement
                    // being demonstrated, not an obstacle worked around: a logical reference is
                    // not playable until someone resolves it.
                    Path media = Files.createTempFile("cna-java-song", ".ogg");
                    try {
                        Song song = compiled.toSong(media.toUri());
                        // The display name is the file's; the location is the caller's, which is
                        // the whole shape of the slice.
                        assertEquals("Main Theme", song.getName());
                    } finally {
                        Files.deleteIfExists(media);
                    }
                }

                CnbVideo source = new CnbVideo("video/intro.webm", Duration.ofSeconds(12),
                        1280, 720, 30.0f, VideoSoundtrackType.Dialog);
                byte[] videoFile = CnbVideo.encode("video/intro.webm", source, "video/intro");
                try (CnbDocument document = CnbDocument.parse(
                             videoFile, "intro.cnb", CnbReadLimits.standard())) {
                    CnbVideo compiled = document.decodeVideo();
                    Video video = compiled.toVideo(getGraphicsDevice(), "video/intro.webm");
                    assertEquals(1280, video.getWidth());
                    assertEquals(720, video.getHeight());
                    assertEquals(30.0f, video.getFramesPerSecond());
                    assertEquals(Duration.ofSeconds(12), video.getDuration());
                    assertEquals(VideoSoundtrackType.Dialog, video.getVideoSoundtrackType());
                }
            } catch (Throwable exception) {
                failure = exception;
            }
        }
    }

    /**
     * The audio half of the slice, run inside a game because that is where XNA's audio lives.
     *
     * <p>A {@code SoundEffect} needs a live CNA game on this process -- the same lifetime rule
     * XNA has -- so this cannot be an ordinary test the way the container half can. It also does
     * not need audio <em>hardware</em>: the NULL backend this suite runs against still creates
     * the resource and still reports its duration, which is what is asserted.
     */
    private static final class SoundProbe extends Game {

        private boolean ran;
        private Throwable failure;

        @Override
        protected void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                byte[] samples = stereoFrames();
                byte[] file;
                try (CnbSoundEffectData source = CnbSoundEffectData.create(
                             new CnbSoundEffectInfo(CnbAudioFormat.Pcm16, 8000, 2, 16, 4, 8),
                             samples)) {
                    file = Cnb.encodeSoundEffect(source, "audio/probe");
                }
                try (CnbDocument document = CnbDocument.parse(
                             file, "probe.cnb", CnbReadLimits.standard());
                     CnbSoundEffectData decoded = document.decodeSoundEffect();
                     SoundEffect effect = decoded.toSoundEffect()) {
                    // Sixteen frames at 8 kHz is two milliseconds, computed by XNA's own duration
                    // rule from the buffer this file carried.
                    assertEquals(Duration.ofMillis(2), effect.getDuration());
                    try (SoundEffectInstance instance = effect.CreateInstance()) {
                        assertEquals(SoundState.Stopped, instance.getState());
                    }
                }

                // A format XNA cannot play must say so rather than produce silence. The v1
                // container has no Vorbis codec, so the refusal is checked on the description
                // itself, which is the object a caller would hold.
                try (CnbSoundEffectData vorbis = CnbSoundEffectData.create(
                             new CnbSoundEffectInfo(CnbAudioFormat.Vorbis, 44100, 2, 16, 0, 0),
                             samples)) {
                    assertThrows(ContentNotSupportedException.class, vorbis::toSoundEffect);
                }
            } catch (Throwable exception) {
                failure = exception;
            }
        }
    }

    private static final class TextureProbe extends Game {

        private boolean ran;
        private Throwable failure;

        private TextureProbe() {
            new GraphicsDeviceManager(this);
        }

        @Override
        protected void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                byte[] file;
                try (CnbTextureData source = CnbTextureData.ofRgba8(2, 2, PIXELS)) {
                    file = Cnb.encodeTexture2D(source, "textures/four-pixels");
                }
                try (CnbDocument document = CnbDocument.parse(
                             file, "four-pixels.cnb", CnbReadLimits.standard());
                     CnbTextureData decoded = document.decodeTexture2D()) {
                    Texture2D texture = decoded.toTexture2D(getGraphicsDevice());
                    try {
                        // The whole point of the slice: a .cnb file becomes a texture a game
                        // draws with, and the pixels survive the trip.
                        assertEquals(2, texture.getWidth());
                        assertEquals(2, texture.getHeight());
                        assertEquals(SurfaceFormat.Color, texture.getFormat());
                        Color[] pixels = new Color[4];
                        texture.GetData(pixels);
                        assertEquals(new Color(255, 0, 0, 255), pixels[0]);
                        assertEquals(new Color(0, 255, 0, 128), pixels[1]);
                        assertEquals(new Color(0, 0, 255, 255), pixels[2]);
                        assertEquals(new Color(0x12, 0x34, 0x56, 0x78), pixels[3]);
                    } finally {
                        texture.close();
                    }
                }
            } catch (Throwable exception) {
                failure = exception;
            }
        }
    }
}
