package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Curve;
import Microsoft.Xna.Framework.CurveContinuity;
import Microsoft.Xna.Framework.CurveLoopType;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.junit.jupiter.api.io.TempDir;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The {@code .cnj} compiler: the build step, end to end.
 *
 * <p>A JSON document goes in and a {@code .cnb} file comes out, which is then read back through
 * CNA's own parser and compared to what the JSON said. The documents here are authored by this
 * test, which is right for the same reason the importer fixtures are: the source format is the
 * thing under test, and a CNA-authored input would prove nothing about whether an artist's
 * document compiles.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnjCompilerTests {

    private static Path write(Path directory, String name, String json) throws IOException {
        Path file = directory.resolve(name);
        Files.write(file, json.getBytes(StandardCharsets.UTF_8));
        return file;
    }

    @Test
    void aCurveDocumentCompilesToACurveAGameEvaluates(@TempDir Path directory)
            throws IOException {
        Path document = write(directory, "ease.cnj",
                "{\"cnjVersion\":1,\"type\":\"Curve\",\"preLoop\":\"Constant\","
                + "\"postLoop\":\"Linear\",\"keys\":["
                + "{\"position\":0,\"value\":2.5},"
                + "{\"position\":1,\"value\":7.5,\"continuity\":\"Step\"}]}");

        byte[] file;
        try (CnjResult result = Cnj.compile(document)) {
            assertEquals(CnbAssetType.CURVE, result.getAssetType());
            // CNA names the type as the runtime does, not as the document wrote
            // it: the document says "Curve" and the file records the .NET type
            // name a reader would look for.
            assertEquals("Microsoft.Xna.Framework.Curve", result.getAssetTypeName());
            // The document is always the first absorbed file, and a curve pulls in nothing else,
            // so a build watching this list watches exactly one path.
            assertEquals(List.of("ease.cnj"), result.getAbsorbedFiles());
            assertEquals(List.of(), result.getExternalReferences());
            file = result.getBytes();
        }

        try (CnbDocument parsed = CnbDocument.parse(
                     file, "ease.cnb", CnbReadLimits.standard())) {
            Curve curve = parsed.decodeCurve();
            assertEquals(CurveLoopType.Constant, curve.getPreLoop());
            assertEquals(CurveLoopType.Linear, curve.getPostLoop());
            assertEquals(2, curve.getKeys().getCount());
            assertEquals(0.0f, curve.getKeys().get(0).getPosition());
            assertEquals(2.5f, curve.getKeys().get(0).getValue());
            assertEquals(7.5f, curve.getKeys().get(1).getValue());
            assertEquals(CurveContinuity.Step, curve.getKeys().get(1).getContinuity());
            // The whole point of a build step: what the artist wrote is what the game evaluates.
            assertEquals(2.5f, curve.Evaluate(0.0f));
            assertEquals(7.5f, curve.Evaluate(1.0f));
        }
    }

    @Test
    void aDocumentThatNamesAnUnknownTypeIsRefusedByName(@TempDir Path directory)
            throws IOException {
        Path document = write(directory, "typo.cnj",
                "{\"cnjVersion\":1,\"type\":\"Curv\",\"keys\":[]}");
        // Refused rather than compiled to an empty file. A typo in a type has to be a build
        // error; an asset that loads to nothing is the failure this prevents, and it would show
        // up in the game rather than in the build that produced it.
        CnbFormatException refused = assertThrows(CnbFormatException.class,
                () -> Cnj.compile(document));
        assertTrue(refused.getMessage().contains("Curv"),
                "the refusal must name the type it did not recognise: " + refused.getMessage());

        Path malformed = write(directory, "broken.cnj", "{\"cnjVersion\":1,");
        assertThrows(RuntimeException.class, () -> Cnj.compile(malformed));
        assertThrows(RuntimeException.class,
                () -> Cnj.compile(directory.resolve("absent.cnj")));
        assertThrows(NullPointerException.class, () -> Cnj.compile(null));
    }

    @Test
    void aSpriteFontDocumentAbsorbsItsAtlas(@TempDir Path directory) throws IOException {
        // A BMP the compiler will read and swallow: two glyphs side by side in a 4x8 image.
        Path atlas = directory.resolve("font.bmp");
        Files.write(atlas, bmp(4, 8));
        Path document = write(directory, "font.cnj",
                "{\"cnjVersion\":1,\"type\":\"SpriteFont\",\"texture\":\"font.bmp\","
                + "\"lineSpacing\":10,\"spacing\":1.0,\"defaultCharacter\":\"?\",\"glyphs\":["
                + "{\"char\":63,\"source\":[2,0,2,8],\"crop\":[0,0,2,8],"
                + "\"kerning\":[1.0,4.0,2.0]},"
                + "{\"char\":65,\"source\":[0,0,2,8],\"crop\":[0,0,2,8],"
                + "\"kerning\":[0.0,5.0,0.0]}]}");

        try (CnjResult result = Cnj.compile(document)) {
            assertEquals(CnbAssetType.SPRITE_FONT, result.getAssetType());
            // The bitmap is now inside the .cnb and no longer needs to ship, which is what
            // "absorbed" means and what a build script needs in order to know what to watch.
            assertEquals(List.of("font.cnj", "font.bmp"), result.getAbsorbedFiles());
            assertFalse(result.getExternalReferences().contains("font.bmp"),
                    "an absorbed file is not also an external reference");

            byte[] file = result.getBytes();
            try (CnbDocument parsed = CnbDocument.parse(
                         file, "font.cnb", CnbReadLimits.standard());
                 CnbSpriteFontData font = parsed.decodeSpriteFont()) {
                assertEquals(10, font.getLineSpacing());
                assertEquals(1.0f, font.getSpacing());
                assertEquals('?', font.getDefaultCharacter());
                // Sorted by character, which is why '?' comes before 'A'.
                assertEquals(List.of('?', 'A'),
                        font.getGlyphs().stream().map(CnbGlyph::Character).toList());
            }
        }
    }

    @Test
    void aModelDocumentCanBeCompiledToADescriptionRatherThanAFile(@TempDir Path directory)
            throws IOException {
        Path document = write(directory, "empty.cnj",
                "{\"cnjVersion\":1,\"type\":\"Model\",\"meshes\":[]}");
        try (CnjModelResult result = Cnj.buildModel(document, null)) {
            // Unlike Cnj.compile, which always lists the document first, building a description
            // absorbs nothing: no file has been written, so nothing has been swallowed into one.
            // A model that pulled in a sidecar would list that sidecar; this one pulls in none.
            assertEquals(List.of(), result.getAbsorbedFiles());
            try (CnbModelData model = result.takeModel()) {
                CnbModelInfo info = model.getInfo();
                assertEquals(0, info.MeshCount());
                assertEquals(0, info.PartCount());
                // A model with no meshes still has a root bone: the compiler gives every model a
                // node hierarchy, so "empty" means no geometry rather than no structure.
                assertEquals(1, info.BoneCount());

                // The model can be changed before it is written, which is why this exists at all
                // rather than only the compile-to-bytes path.
                model.addBone("inserted", 0, Microsoft.Xna.Framework.Matrix.getIdentity());
                assertEquals(2, model.getInfo().BoneCount());
            }
            // The model is moved out, not lent, so a second take finds nothing to move and the
            // result can be closed without taking the first one's model with it.
            assertThrows(RuntimeException.class, result::takeModel);
        }
    }

    @Test
    void aModelWithAnAnimationCarriesItsTracksAndKeyframes(@TempDir Path directory)
            throws IOException {
        write(directory, "wave.cnj",
                "{\"cnjVersion\":1,\"type\":\"AnimationClip\",\"duration\":2.0,"
                + "\"targetSpace\":\"SceneNode\",\"tracks\":[{\"boneIndex\":0,\"keys\":["
                + "{\"time\":0.0,\"translation\":[0,0,0],\"rotation\":[0,0,0,1],"
                + "\"scale\":[1,1,1]},"
                + "{\"time\":2.0,\"translation\":[4,0,0],\"rotation\":[0,0,0,1],"
                + "\"scale\":[1,1,1]}]}]}");
        Path model = write(directory, "hero.cnj",
                "{\"cnjVersion\":1,\"type\":\"Model\",\"meshes\":[],"
                + "\"animations\":[{\"name\":\"wave\",\"clip\":\"wave.cnj\"}]}");

        // The content root is explicit here, and has to be: a model naming a sidecar is refused
        // without one, because a relative path has nothing to be relative to.
        assertThrows(CnbFormatException.class, () -> Cnj.buildModel(model, null));
        try (CnjModelResult result = Cnj.buildModel(model, directory);
             CnbModelData compiled = result.takeModel()) {
            // The clip sidecar is what the compile absorbed, which is what a build watches.
            assertEquals(List.of("wave.cnj"), result.getAbsorbedFiles());

            assertEquals(1, compiled.getInfo().AnimationCount());
            CnbAnimation animation = compiled.getAnimations().get(0);
            assertEquals("wave", animation.Name());
            assertEquals(2.0, animation.DurationSeconds());
            assertEquals(1, animation.TrackCount());
            // Scene-node rather than joint-palette: this model has no skeleton, and the two
            // spaces select different things, so the one the document declared has to survive.
            assertEquals(CnbClipTargetSpace.SceneNode, animation.TargetSpace());

            int[] track = compiled.getAnimationTrack(0, 0);
            assertEquals(0, track[0], "the track drives bone 0");
            assertEquals(2, track[1]);

            List<CnbKeyframe> keyframes = compiled.readAnimationKeyframes(0, 0);
            assertEquals(2, keyframes.size());
            assertEquals(0.0, keyframes.get(0).TimeSeconds());
            assertEquals(2.0, keyframes.get(1).TimeSeconds());
            assertEquals(new Vector3(0f, 0f, 0f), keyframes.get(0).Translation());
            assertEquals(new Vector3(4f, 0f, 0f), keyframes.get(1).Translation());
            assertEquals(1.0f, keyframes.get(1).Rotation().W);
            assertEquals(new Vector3(1f, 1f, 1f), keyframes.get(1).Scale());
        }
    }

    @Test
    void aClosedResultRefusesEveryOperation(@TempDir Path directory) throws IOException {
        Path document = write(directory, "ease.cnj",
                "{\"cnjVersion\":1,\"type\":\"Curve\",\"preLoop\":\"Constant\","
                + "\"postLoop\":\"Constant\",\"keys\":[{\"position\":0,\"value\":1}]}");
        CnjResult result = Cnj.compile(document);
        result.close();
        result.close();
        assertThrows(IllegalStateException.class, result::getBytes);
        assertThrows(IllegalStateException.class, result::getAbsorbedFiles);
    }

    /** A 24-bit bottom-up BMP of one solid colour, written from the format's layout. */
    private static byte[] bmp(int width, int height) {
        int rowBytes = ((width * 3 + 3) / 4) * 4;
        int pixelBytes = rowBytes * height;
        byte[] file = new byte[54 + pixelBytes];
        java.nio.ByteBuffer buffer = java.nio.ByteBuffer.wrap(file)
                .order(java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.put((byte) 'B').put((byte) 'M');
        buffer.putInt(file.length);
        buffer.putInt(0);
        buffer.putInt(54);
        buffer.putInt(40);          // DIB header size
        buffer.putInt(width);
        buffer.putInt(height);
        buffer.putShort((short) 1);
        buffer.putShort((short) 24);
        buffer.putInt(0);           // BI_RGB
        buffer.putInt(pixelBytes);
        buffer.putInt(2835);
        buffer.putInt(2835);
        buffer.putInt(0);
        buffer.putInt(0);
        for (int index = 54; index < file.length; index++) {
            file[index] = (byte) 0xFF;
        }
        return file;
    }
}
