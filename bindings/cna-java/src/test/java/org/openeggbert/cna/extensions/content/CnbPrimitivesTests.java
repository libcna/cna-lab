package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The {@code .cnb} primitive writer, the cursor's last three reads, and the external-reference
 * table a file records.
 *
 * <p>Three asymmetries were left in this projection, each one a reader with no writer. The cursor
 * could read every primitive and nothing could write one; a document could report its whole
 * external-reference table and the writer could not set it; a keyframe could be read and not
 * written. All three sat behind "the .cnb container, its schemas, its writers and its loader
 * registry are a CNA content format with no XNA 4.0 counterpart", which explains what the format
 * is and not why half of it was missing.
 *
 * <p><strong>Every assertion here is a round trip through CNA's own encoder and CNA's own
 * cursor.</strong> That is the point: writing the bytes from Java would be a second encoder for
 * one format, and the way that fails is quiet -- a file this projection wrote that CNA's reader
 * refuses, or worse, accepts and reads differently.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnbPrimitivesTests {

    private static CnbReader readerOver(byte[] bytes) {
        return CnbReader.of(bytes, "test payload", CnbReadLimits.standard());
    }

    @Test
    void everyPrimitiveComesBackOutOfTheCursorThatWroteIt() {
        byte[] payload;
        try (CnbByteWriter writer = CnbByteWriter.create()) {
            assertEquals(0L, writer.size(), "a fresh writer holds nothing");
            writer.writeUnsignedByte(0xFE)
                    .writeUnsignedShort(0xBEEF)
                    .writeUnsignedInt(0xDEADBEEFL)
                    .writeUnsignedLong(-1L)
                    .writeInt(-123456)
                    .writeFloat(0.15625f)
                    .writeDouble(Math.PI)
                    .writeString("clip é🎮")
                    .writeBytes(new byte[] {1, 2, 3})
                    .writeZeros(4);
            // 1 + 2 + 4 + 8 + 4 + 4 + 8, then the string as a four-byte length and eleven
            // UTF-8 bytes -- "clip " is five, the acute e is two and the emoji is four -- then
            // three raw bytes and four zeros. The string's prefix width is measured rather than
            // assumed: it is a fixed 32-bit count here, not the seven-bit form an .xnb uses,
            // and the first version of this test guessed the other one.
            assertEquals(1 + 2 + 4 + 8 + 4 + 4 + 8 + 4 + 11 + 3 + 4, writer.size(),
                    "each primitive occupies exactly the width the format gives it");
            payload = writer.toByteArray();
            assertEquals(payload.length, writer.size(),
                    "copying leaves the writer holding what it had");
        }

        try (CnbReader reader = readerOver(payload)) {
            assertEquals(0xFE, reader.readUnsignedByte());
            assertEquals(0xBEEF, reader.readUnsignedShort());
            assertEquals(0xDEADBEEFL, reader.readUnsignedInt());
            assertEquals(-1L, reader.readUnsignedLong(), "all sixty-four bits survive");
            assertEquals(-123456, reader.readInt());
            assertEquals(0.15625f, reader.readFloat(), 0f);
            assertEquals(Math.PI, reader.readDouble(), 0d);
            assertEquals("clip é🎮", reader.readString(),
                    "an astral character survives the length-prefixed UTF-8 form");
            assertArrayEquals(new byte[] {1, 2, 3}, reader.readBytes(3));
            assertArrayEquals(new byte[4], reader.readBytes(4));
            reader.requireExhausted();
        }
    }

    @Test
    void eachPrimitiveOccupiesTheWidthTheFormatGivesIt() {
        // One at a time, so a width that moved names itself rather than shifting a total.
        assertEquals(1L, widthOf(writer -> writer.writeUnsignedByte(1)));
        assertEquals(2L, widthOf(writer -> writer.writeUnsignedShort(1)));
        assertEquals(4L, widthOf(writer -> writer.writeUnsignedInt(1L)));
        assertEquals(8L, widthOf(writer -> writer.writeUnsignedLong(1L)));
        assertEquals(4L, widthOf(writer -> writer.writeInt(1)));
        assertEquals(4L, widthOf(writer -> writer.writeFloat(1f)));
        assertEquals(8L, widthOf(writer -> writer.writeDouble(1d)));
        assertEquals(4L, widthOf(writer -> writer.writeString("")),
                "an empty string is its four-byte length and nothing else");
        assertEquals(4L + 1L, widthOf(writer -> writer.writeString("a")));
        assertEquals(4L + 4L, widthOf(writer -> writer.writeString("\uD83C\uDFAE")),
                "a character outside the basic plane is four UTF-8 bytes, not two");
        assertEquals(0L, widthOf(writer -> writer.writeZeros(0)));
        assertEquals(7L, widthOf(writer -> writer.writeZeros(7)));
        // A keyframe is a double and ten floats, which is what makes the flattened pair the
        // generated adapter takes the right shape.
        assertEquals(8L + 10L * 4L, widthOf(writer -> writer.writeKeyframe(
                new CnbKeyframe(0d, new Vector3(0f, 0f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f)))));
    }

    private static long widthOf(java.util.function.Consumer<CnbByteWriter> write) {
        try (CnbByteWriter writer = CnbByteWriter.create()) {
            write.accept(writer);
            return writer.size();
        }
    }

    @Test
    void takeEmptiesTheWriterAndCopyDoesNot() {
        try (CnbByteWriter writer = CnbByteWriter.create()) {
            writer.writeInt(7);
            byte[] copied = writer.toByteArray();
            assertEquals(4L, writer.size());
            byte[] taken = writer.take();
            assertArrayEquals(copied, taken, "taking hands over the same bytes copying saw");
            assertEquals(0L, writer.size(), "and leaves the writer empty");

            // Which is what makes one writer usable for several chunks in a row.
            writer.writeInt(9);
            assertEquals(4L, writer.size());
            try (CnbReader reader = readerOver(writer.take())) {
                assertEquals(9, reader.readInt());
            }
        }
    }

    @Test
    void aWriterCanBeSeededWithBytesAlreadyWritten() {
        byte[] head;
        try (CnbByteWriter first = CnbByteWriter.create()) {
            first.writeString("head");
            head = first.take();
        }
        try (CnbByteWriter second = CnbByteWriter.of(head)) {
            assertEquals(head.length, second.size(),
                    "the seeded bytes are the writer's, not a separate prefix");
            second.writeInt(42);
            try (CnbReader reader = readerOver(second.take())) {
                assertEquals("head", reader.readString());
                assertEquals(42, reader.readInt());
                reader.requireExhausted();
            }
        }
        // And the seed array stays the caller's: CNA copies it.
        byte[] mine = {9, 9, 9};
        try (CnbByteWriter third = CnbByteWriter.of(mine)) {
            mine[0] = 1;
            assertArrayEquals(new byte[] {9, 9, 9}, third.toByteArray());
        }
    }

    @Test
    void aKeyframeSurvivesTheRoundTripPoseForPose() {
        CnbKeyframe pose = new CnbKeyframe(1.25,
                new Vector3(1f, 2f, 3f),
                new Quaternion(0.1f, 0.2f, 0.3f, 0.4f),
                new Vector3(4f, 5f, 6f));
        byte[] payload;
        try (CnbByteWriter writer = CnbByteWriter.create()) {
            writer.writeKeyframe(pose);
            payload = writer.take();
        }
        try (CnbReader reader = readerOver(payload)) {
            CnbKeyframe read = reader.readKeyframe();
            // Field by field rather than by equals: a projection that swapped the rotation's
            // components, or the translation with the scale, would still round-trip through a
            // record comparison of two values it built the same wrong way.
            assertEquals(1.25, read.TimeSeconds(), 0d);
            assertEquals(1f, read.Translation().X, 0f);
            assertEquals(2f, read.Translation().Y, 0f);
            assertEquals(3f, read.Translation().Z, 0f);
            assertEquals(0.1f, read.Rotation().X, 0f);
            assertEquals(0.2f, read.Rotation().Y, 0f);
            assertEquals(0.3f, read.Rotation().Z, 0f);
            assertEquals(0.4f, read.Rotation().W, 0f);
            assertEquals(4f, read.Scale().X, 0f);
            assertEquals(5f, read.Scale().Y, 0f);
            assertEquals(6f, read.Scale().Z, 0f);
            reader.requireExhausted();
        }
    }

    @Test
    void aSecondsReadRefusesWhatATimeSpanCannotHold() {
        byte[] good;
        byte[] infinite;
        byte[] notANumber;
        try (CnbByteWriter writer = CnbByteWriter.create()) {
            writer.writeDouble(2.5);
            good = writer.take();
            writer.writeDouble(Double.POSITIVE_INFINITY);
            infinite = writer.take();
            writer.writeDouble(Double.NaN);
            notANumber = writer.take();
        }
        try (CnbReader reader = readerOver(good)) {
            assertEquals(2.5, reader.readSeconds("the clip duration"), 0d);
        }
        // readDouble takes these happily; readSeconds is the narrower read, and the difference
        // is the whole reason it exists.
        try (CnbReader reader = readerOver(infinite)) {
            assertEquals(Double.POSITIVE_INFINITY, reader.readDouble(), 0d);
        }
        try (CnbReader reader = readerOver(infinite)) {
            assertThrows(CnbFormatException.class, () -> reader.readSeconds("the duration"));
        }
        try (CnbReader reader = readerOver(notANumber)) {
            assertThrows(CnbFormatException.class, () -> reader.readSeconds("the duration"));
        }
    }

    @Test
    void aDecodersOwnRefusalReadsLikeTheCursorsOwn() {
        byte[] payload;
        try (CnbByteWriter writer = CnbByteWriter.create()) {
            writer.writeInt(1).writeInt(2);
            payload = writer.take();
        }
        try (CnbReader reader = readerOver(payload)) {
            reader.readInt();
            CnbFormatException failure = assertThrows(CnbFormatException.class,
                    () -> reader.fail("the version is not one this schema knows"));
            String message = failure.getMessage();
            assertTrue(message.contains("the version is not one this schema knows"),
                    "the detail survives: " + message);
            assertTrue(message.contains("test payload"),
                    "and so does the cursor's own name for the bytes: " + message);
        }
    }

    @Test
    void whatTheWriterRefusesIsRefusedBeforeItReachesCna() {
        try (CnbByteWriter writer = CnbByteWriter.create()) {
            assertThrows(IllegalArgumentException.class, () -> writer.writeUnsignedByte(256));
            assertThrows(IllegalArgumentException.class, () -> writer.writeUnsignedByte(-1));
            assertThrows(IllegalArgumentException.class, () -> writer.writeUnsignedShort(0x10000));
            assertThrows(IllegalArgumentException.class,
                    () -> writer.writeUnsignedInt(0x1_0000_0000L));
            assertThrows(IllegalArgumentException.class, () -> writer.writeZeros(-1));
            assertThrows(NullPointerException.class, () -> writer.writeString(null));
            assertThrows(NullPointerException.class, () -> writer.writeBytes(null));
            assertThrows(NullPointerException.class, () -> writer.writeKeyframe(null));
            assertEquals(0L, writer.size(), "and none of them wrote anything");
        }
    }

    @Test
    void aClosedWriterSaysSo() {
        CnbByteWriter writer = CnbByteWriter.create();
        writer.writeInt(1);
        writer.close();
        writer.close();
        assertThrows(IllegalStateException.class, writer::size);
        assertThrows(IllegalStateException.class, () -> writer.writeInt(2));
        assertThrows(IllegalStateException.class, writer::take);
    }

    @Test
    void anExternalReferenceTableSurvivesBeingWrittenAndReadBack() {
        List<CnbExternalReference> table = List.of(
                new CnbExternalReference("textures/atlas", CnbAssetType.TEXTURE_2D, 0),
                new CnbExternalReference("audio/theme", CnbAssetType.SONG, 0),
                new CnbExternalReference("models/hero", CnbAssetType.MODEL, 0));
        byte[] file;
        try (CnbWriter writer = CnbWriter.of(CnbAssetType.CURVE, 1)) {
            writer.setExternalReferences(table);
            writer.addChunk(CnbChunkId.of("TEST"), new byte[] {1, 2, 3, 4}, false);
            file = writer.build();
        }
        try (CnbDocument document = CnbDocument.parse(file, "xref round trip", CnbReadLimits.standard())) {
            List<CnbExternalReference> read = document.getExternalReferences();
            assertEquals(3, read.size());
            // Position is identity here: a schema refers to an entry by its index, so an order
            // the writer did not preserve would silently repoint every reference.
            for (int index = 0; index < table.size(); index++) {
                assertEquals(table.get(index).LogicalName(), read.get(index).LogicalName(),
                        "entry " + index + " kept its name and its place");
                assertEquals(table.get(index).ExpectedAssetType().Id(),
                        read.get(index).ExpectedAssetType().Id());
                assertEquals(table.get(index).Flags(), read.get(index).Flags());
            }
            assertNotEquals(read.get(0).ExpectedAssetType().Id(),
                    read.get(1).ExpectedAssetType().Id(),
                    "the three entries differ, so an order that collapsed would be visible");
        }
    }

    @Test
    void theReferenceFlagsAreReservedAndCnaSaysSo() {
        // Measured rather than assumed: CnbExternalReference carries a flags field because the
        // file format has one, and CNA refuses a non-zero value when it assembles the file. A
        // reader can therefore report flags a future format writes, and a writer today cannot
        // invent them.
        try (CnbWriter writer = CnbWriter.of(CnbAssetType.CURVE, 1)) {
            writer.addChunk(CnbChunkId.of("TEST"), new byte[] {0}, false);
            writer.setExternalReferences(List.of(
                    new CnbExternalReference("later", CnbAssetType.CURVE, 1)));
            CnbFormatException refused = assertThrows(CnbFormatException.class, writer::build);
            assertTrue(refused.getMessage().contains("reserved"),
                    "and says why: " + refused.getMessage());
        }
    }

    @Test
    void settingTheTableAgainReplacesItRatherThanAppending() {
        byte[] file;
        try (CnbWriter writer = CnbWriter.of(CnbAssetType.CURVE, 1)) {
            writer.setExternalReferences(List.of(
                    new CnbExternalReference("first", CnbAssetType.TEXTURE_2D, 0),
                    new CnbExternalReference("second", CnbAssetType.TEXTURE_2D, 0)));
            writer.setExternalReferences(List.of(
                    new CnbExternalReference("only", CnbAssetType.CURVE, 0)));
            writer.addChunk(CnbChunkId.of("TEST"), new byte[] {0}, false);
            file = writer.build();
        }
        try (CnbDocument document = CnbDocument.parse(file, "xref replace", CnbReadLimits.standard())) {
            assertEquals(1, document.getExternalReferences().size(),
                    "the second call replaced the table rather than adding to it");
            assertEquals("only", document.getExternalReferences().get(0).LogicalName());
        }

        // An empty list clears it, which is the same operation with nothing appended.
        byte[] cleared;
        try (CnbWriter writer = CnbWriter.of(CnbAssetType.CURVE, 1)) {
            writer.setExternalReferences(List.of(
                    new CnbExternalReference("gone", CnbAssetType.CURVE, 0)));
            writer.setExternalReferences(List.of());
            writer.addChunk(CnbChunkId.of("TEST"), new byte[] {0}, false);
            cleared = writer.build();
        }
        try (CnbDocument document = CnbDocument.parse(cleared, "xref clear", CnbReadLimits.standard())) {
            assertTrue(document.getExternalReferences().isEmpty());
        }
    }

    @Test
    void aReferenceNameTheFormatRefusesIsRefusedWhenTheFileIsAssembled() {
        try (CnbWriter writer = CnbWriter.of(CnbAssetType.CURVE, 1)) {
            writer.addChunk(CnbChunkId.of("TEST"), new byte[] {0}, false);
            // CNA validates each name with the same function its reader applies, when the file
            // is assembled rather than when the entry is added -- so this is what fails, and
            // that is worth pinning: a writer that accepted a name its own reader refuses would
            // produce a file nothing can read.
            writer.setExternalReferences(List.of(
                    new CnbExternalReference("../escapes/the/root", CnbAssetType.CURVE, 0)));
            assertThrows(RuntimeException.class, writer::build);
        }
        assertThrows(NullPointerException.class, () -> {
            try (CnbWriter writer = CnbWriter.of(CnbAssetType.CURVE, 1)) {
                writer.setExternalReferences(null);
            }
        });
    }

    /** Two tracks with different keyframe counts, so a flattening that lost the split shows. */
    private static CnbClip threeBoneClip() {
        return new CnbClip(2.5, List.of(
                new CnbBoneTrack(0, List.of(
                        new CnbKeyframe(0d, new Vector3(0f, 0f, 0f),
                                new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f)),
                        new CnbKeyframe(1.25, new Vector3(1f, 2f, 3f),
                                new Quaternion(0.1f, 0.2f, 0.3f, 0.4f), new Vector3(2f, 2f, 2f)),
                        new CnbKeyframe(2.5, new Vector3(4f, 5f, 6f),
                                new Quaternion(0.5f, 0.6f, 0.7f, 0.8f), new Vector3(3f, 3f, 3f)))),
                new CnbBoneTrack(2, List.of(
                        new CnbKeyframe(0.5, new Vector3(7f, 8f, 9f),
                                new Quaternion(0f, 1f, 0f, 0f), new Vector3(0.5f, 0.5f, 0.5f))))));
    }

    @Test
    void aClipSurvivesEncodingAndDecodingTrackForTrack() {
        CnbClip written = threeBoneClip();
        byte[] file = written.encode(CnbClipTargetSpace.JointPalette, "walk");
        try (CnbDocument document = CnbDocument.parse(file, "clip", CnbReadLimits.standard());
                CnbAnimationClip decoded = CnbAnimationClip.decode(document)) {
            assertEquals(CnbAssetType.ANIMATION_CLIP.Id(), document.getAssetType().Id());
            assertEquals(2.5, decoded.getDurationSeconds(), 0d);
            assertEquals(2, decoded.getTrackCount());
            assertEquals(CnbClipTargetSpace.JointPalette, decoded.getTargetSpace());

            // The split between the tracks is the thing a flattening can lose, so it is asserted
            // before the values: two tracks with three and one keyframes, not one with four.
            assertEquals(0, decoded.getTrackBoneIndex(0));
            assertEquals(3, decoded.getTrackKeyframeCount(0));
            assertEquals(2, decoded.getTrackBoneIndex(1));
            assertEquals(1, decoded.getTrackKeyframeCount(1));

            CnbClip read = CnbClip.of(decoded);
            assertEquals(written, read,
                    "the whole clip is the value that went in, tracks and poses and all");
        }
    }

    @Test
    void aClipAddedToAModelIsTheModelsAfterwards() {
        try (CnbModelData model = CnbModelData.create()) {
            assertEquals(0, model.getAnimations().size());
            assertEquals(0, model.addAnimation("walk", threeBoneClip(),
                    CnbClipTargetSpace.JointPalette));
            assertEquals(1, model.addAnimation("run", new CnbClip(1.0, List.of()),
                    CnbClipTargetSpace.SceneNode));

            List<CnbAnimation> animations = model.getAnimations();
            assertEquals(2, animations.size());
            assertEquals("walk", animations.get(0).Name());
            assertEquals(2.5, animations.get(0).DurationSeconds(), 0d);
            assertEquals(2, animations.get(0).TrackCount());
            assertEquals(CnbClipTargetSpace.JointPalette, animations.get(0).TargetSpace());
            assertEquals("run", animations.get(1).Name());
            assertEquals(0, animations.get(1).TrackCount(),
                    "a clip with no tracks is a clip, not a refusal");
            assertEquals(CnbClipTargetSpace.SceneNode, animations.get(1).TargetSpace());
        }
    }

    @Test
    void aClipWhoseCountsDoNotAddUpCannotBeBuiltAtAll() {
        // The flattening is done by CnbClip rather than by the caller, so the four arrays cannot
        // disagree through this API -- which is the point of taking a list of tracks. What the
        // adapter still checks is the case a direct caller of the internal route could produce,
        // and it checks it before allocating anything, because a sum that does not add up would
        // make CNA read past an array's end from inside C.
        long[] size = new long[1];
        assertEquals(1, org.openeggbert.cna.internal.NativeBindings.cnbEncodeAnimationClip(
                        1.0, new int[] {0}, new int[] {2}, new double[] {0d},
                        new float[CnbKeyframes.FLOATS], 0, new byte[0], new byte[0], size),
                "two keyframes promised and one supplied is INVALID_ARGUMENT");
        assertEquals(1, org.openeggbert.cna.internal.NativeBindings.cnbEncodeAnimationClip(
                        1.0, new int[] {0, 1}, new int[] {1}, new double[] {0d},
                        new float[CnbKeyframes.FLOATS], 0, new byte[0], new byte[0], size),
                "two tracks and one count is INVALID_ARGUMENT");
        assertEquals(1, org.openeggbert.cna.internal.NativeBindings.cnbEncodeAnimationClip(
                        1.0, new int[] {0}, new int[] {1}, new double[] {0d},
                        new float[CnbKeyframes.FLOATS - 1], 0, new byte[0], new byte[0], size),
                "one keyframe short of its ten floats is INVALID_ARGUMENT");
        assertEquals(1, org.openeggbert.cna.internal.NativeBindings.cnbEncodeAnimationClip(
                        1.0, new int[] {0}, new int[] {-1}, new double[0],
                        new float[0], 0, new byte[0], new byte[0], size),
                "a negative keyframe count is INVALID_ARGUMENT rather than a huge unsigned one");

        assertThrows(NullPointerException.class, () -> new CnbClip(1.0, null));
        assertThrows(NullPointerException.class, () -> new CnbBoneTrack(0, null));
        assertThrows(NullPointerException.class,
                () -> threeBoneClip().encode(null, "walk"));
    }
}
