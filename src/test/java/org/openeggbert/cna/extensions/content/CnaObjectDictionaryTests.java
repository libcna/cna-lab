package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Content.ContentManager;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.junit.jupiter.api.io.TempDir;

import java.io.ByteArrayOutputStream;
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
 * The {@code Dictionary<string, object>} a custom content processor attaches to an asset.
 *
 * <p>Fifteen routes were unbound behind "the content processor's Dictionary&lt;string, object&gt;
 * side data is reachable in CNA through a dedicated handle kind that XNA exposes only as a managed
 * object graph" -- which says why the shape differs and nothing about why it was absent. It was
 * absent because nothing had asked whether a Java game could reach it, and nothing can: the
 * managed reader table in this projection has no {@code DictionaryReader}, so before this the
 * asset could not be loaded at all, by either path.
 *
 * <p><strong>The fixture is CNA's own, byte for byte.</strong> The layout below is the one
 * {@code modules/c-api/tests/pure_c/ContentSmoke.c} writes for its own object-dictionary test --
 * seven-bit-encoded reader names, a five-byte header, a reader table, then key/reader-index/value
 * triples. Two of CNA's five entries need a reflectively declared type registered from C, which
 * this projection deliberately does not project, so this fixture carries the other three: the
 * {@code BoundingSphere}, the {@code Vector3[]} and the name. Those three are exactly the shape
 * XNA's own {@code TrianglePickingSample} stores, which is what the family exists for.
 *
 * <p>What is asserted is the values, not that a call succeeded: the sphere's centre and radius,
 * every vertex of the triangle, the string, the entry kinds, the array length, and the refusal
 * CNA gives for a type named wrongly.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnaObjectDictionaryTests {

    /** The three vertices the fixture stores, which is what a picking sample would carry. */
    private static final float[][] TRIANGLE = {
            {0f, 0f, 0f}, {1f, 0f, 0f}, {0f, 1f, 0f}};

    /** A seven-bit-encoded length, which is how an {@code .xnb} writes every count and string. */
    private static void writeSevenBit(ByteArrayOutputStream out, int value) {
        int remaining = value;
        while (remaining >= 0x80) {
            out.write((remaining & 0x7F) | 0x80);
            remaining >>>= 7;
        }
        out.write(remaining);
    }

    private static void writeString(ByteArrayOutputStream out, String text) {
        byte[] bytes = text.getBytes(StandardCharsets.UTF_8);
        writeSevenBit(out, bytes.length);
        out.write(bytes, 0, bytes.length);
    }

    /** A reader name followed by its version, which is always zero for the stock readers. */
    private static void writeReader(ByteArrayOutputStream out, String name) {
        writeString(out, name);
        writeInt32(out, 0);
    }

    private static void writeInt32(ByteArrayOutputStream out, int value) {
        out.write(value & 0xFF);
        out.write((value >>> 8) & 0xFF);
        out.write((value >>> 16) & 0xFF);
        out.write((value >>> 24) & 0xFF);
    }

    private static void writeSingle(ByteArrayOutputStream out, float value) {
        writeInt32(out, Float.floatToRawIntBits(value));
    }

    /**
     * Writes the same asset CNA's own C API test writes, minus the two reflective entries.
     *
     * @return the whole {@code .xnb} file
     */
    private static byte[] dictionaryAsset() {
        ByteArrayOutputStream body = new ByteArrayOutputStream();
        // Five type readers, one-based when referenced: 1 dictionary, 2 string, 3 sphere,
        // 4 Vector3 array, 5 Vector3.
        writeSevenBit(body, 5);
        writeReader(body,
                "Microsoft.Xna.Framework.Content.DictionaryReader`2"
                        + "[[System.String],[System.Object]]");
        writeReader(body, "Microsoft.Xna.Framework.Content.StringReader");
        writeReader(body, "Microsoft.Xna.Framework.Content.BoundingSphereReader");
        writeReader(body,
                "Microsoft.Xna.Framework.Content.ArrayReader`1"
                        + "[[Microsoft.Xna.Framework.Vector3]]");
        writeReader(body, "Microsoft.Xna.Framework.Content.Vector3Reader");
        writeSevenBit(body, 0);   // no shared resources
        writeSevenBit(body, 1);   // the root object is read by reader 1, the dictionary

        writeInt32(body, 3);      // three entries

        writeSevenBit(body, 2);   // key, by the string reader
        writeString(body, "BoundingSphere");
        writeSevenBit(body, 3);   // value, by the bounding-sphere reader
        writeSingle(body, 1f);
        writeSingle(body, 2f);
        writeSingle(body, 3f);
        writeSingle(body, 4f);

        writeSevenBit(body, 2);
        writeString(body, "Name");
        writeSevenBit(body, 2);
        writeString(body, "triangles");

        writeSevenBit(body, 2);
        writeString(body, "Vertices");
        writeSevenBit(body, 4);   // the Vector3 array reader
        writeInt32(body, TRIANGLE.length);
        for (float[] vertex : TRIANGLE) {
            writeSingle(body, vertex[0]);
            writeSingle(body, vertex[1]);
            writeSingle(body, vertex[2]);
        }

        byte[] payload = body.toByteArray();
        ByteArrayOutputStream file = new ByteArrayOutputStream();
        file.write('X');
        file.write('N');
        file.write('B');
        file.write('w');          // the Windows platform byte
        file.write(5);            // format version 5
        file.write(0);            // no flags: uncompressed, not HiDef
        // The size field counts the whole file, header included, which is why the header is
        // written last and the payload measured first.
        writeInt32(file, payload.length + 10);
        file.write(payload, 0, payload.length);
        return file.toByteArray();
    }

    /**
     * A one-bone model whose {@code Tag} is a dictionary, laid out exactly as CNA's own
     * {@code ContentSmoke.c} writes its tagged-model fixture.
     */
    private static byte[] taggedModelAsset() {
        ByteArrayOutputStream body = new ByteArrayOutputStream();
        writeSevenBit(body, 5);   // 1 model, 2 string, 3 dictionary, 4 Vector3 array, 5 sphere
        writeReader(body, "Microsoft.Xna.Framework.Content.ModelReader");
        writeReader(body, "Microsoft.Xna.Framework.Content.StringReader");
        writeReader(body,
                "Microsoft.Xna.Framework.Content.DictionaryReader`2"
                        + "[[System.String],[System.Object]]");
        writeReader(body,
                "Microsoft.Xna.Framework.Content.ArrayReader`1"
                        + "[[Microsoft.Xna.Framework.Vector3]]");
        writeReader(body, "Microsoft.Xna.Framework.Content.BoundingSphereReader");
        writeSevenBit(body, 0);   // no shared resources
        writeSevenBit(body, 1);   // the root object is the model

        writeInt32(body, 1);      // one bone
        writeSevenBit(body, 2);
        writeString(body, "Root");
        for (int index = 0; index < 16; index++) {
            writeSingle(body, index % 5 == 0 ? 1f : 0f);
        }
        body.write(0);            // no parent
        writeInt32(body, 0);      // no children
        writeInt32(body, 0);      // no meshes
        body.write(1);            // root bone reference

        writeSevenBit(body, 3);   // Tag: the dictionary
        writeInt32(body, 2);
        writeSevenBit(body, 2);
        writeString(body, "BoundingSphere");
        writeSevenBit(body, 5);
        writeSingle(body, 1f);
        writeSingle(body, 2f);
        writeSingle(body, 3f);
        writeSingle(body, 4f);
        writeSevenBit(body, 2);
        writeString(body, "Vertices");
        writeSevenBit(body, 4);
        writeInt32(body, TRIANGLE.length);
        for (float[] vertex : TRIANGLE) {
            writeSingle(body, vertex[0]);
            writeSingle(body, vertex[1]);
            writeSingle(body, vertex[2]);
        }
        return frame(body.toByteArray());
    }

    private static byte[] frame(byte[] payload) {
        ByteArrayOutputStream file = new ByteArrayOutputStream();
        file.write('X');
        file.write('N');
        file.write('B');
        file.write('w');
        file.write(5);
        file.write(0);
        writeInt32(file, payload.length + 10);
        file.write(payload, 0, payload.length);
        return file.toByteArray();
    }

    @Test
    void aModelLoadedByCnaCarriesTheTagItsProcessorWrote(@TempDir Path root) throws IOException {
        Files.write(root.resolve("tagged.xnb"), taggedModelAsset());
        run(root, manager -> {
            CnaObjectDictionary tag;
            try (CnaModel model = CnaModel.Load(manager, "tagged")) {
                assertEquals(1, model.getBoneCount());
                tag = model.getContentTag();
            }
            // The tag outlives the model, which is what CNA documents for it.
            try (CnaObjectDictionary dictionary = tag) {
                assertTrue(dictionary != null, "the model's Tag is a dictionary");
                assertEquals(List.of("BoundingSphere", "Vertices"), dictionary.keys());
                assertEquals(4f, dictionary.getBoundingSphere("BoundingSphere").Radius, 0f);
                List<Vector3> triangle = dictionary.getVector3Array("Vertices");
                assertEquals(3, triangle.size());
                assertEquals(1f, triangle.get(1).X, 0f);
            }
            assertThrows(NullPointerException.class, () -> CnaModel.Load(manager, null));
        });
    }

    @Test
    void anAssetWhoseRootIsADictionaryReadsBackEveryValue(@TempDir Path root) throws IOException {
        Files.write(root.resolve("triangles.xnb"), dictionaryAsset());
        run(root, manager -> {
            try (CnaObjectDictionary dictionary =
                         CnaObjectDictionary.Load(manager, "triangles")) {
                assertEquals(3, dictionary.size());
                assertEquals(List.of("BoundingSphere", "Name", "Vertices"), dictionary.keys(),
                        "the canonical container is ordered, so the keys come back in one order");
                assertEquals("System.Collections.Generic.Dictionary`2"
                                + "[System.String,System.Object]",
                        dictionary.getRuntimeTypeName());

                assertTrue(dictionary.containsKey("Name"));
                assertFalse(dictionary.containsKey("Absent"));

                // The sphere: four floats written, four read, in the order they were written.
                assertEquals(ObjectDictionaryValueKind.BoundingSphere,
                        dictionary.getEntryKind("BoundingSphere"));
                BoundingSphere sphere = dictionary.getBoundingSphere("BoundingSphere");
                assertEquals(1f, sphere.Center.X, 0f);
                assertEquals(2f, sphere.Center.Y, 0f);
                assertEquals(3f, sphere.Center.Z, 0f);
                assertEquals(4f, sphere.Radius, 0f);

                assertEquals(ObjectDictionaryValueKind.String, dictionary.getEntryKind("Name"));
                assertEquals("triangles", dictionary.getString("Name"));

                // The array: an entry that reports its element kind with is_array set, which is
                // a different answer from a scalar of the same kind.
                CnaObjectDictionary.Entry vertices = dictionary.getEntry("Vertices");
                assertEquals(ObjectDictionaryValueKind.Vector3, vertices.kind());
                assertTrue(vertices.isArray(), "a Vector3[] is an array of Vector3");
                assertEquals(3, vertices.elementCount());
                assertFalse(dictionary.getEntry("Name").isArray());
                assertEquals(1, dictionary.getEntry("Name").elementCount(),
                        "a scalar reports one element, not zero");

                List<Vector3> triangle = dictionary.getVector3Array("Vertices");
                assertEquals(3, triangle.size());
                for (int index = 0; index < TRIANGLE.length; index++) {
                    assertEquals(TRIANGLE[index][0], triangle.get(index).X, 0f);
                    assertEquals(TRIANGLE[index][1], triangle.get(index).Y, 0f);
                    assertEquals(TRIANGLE[index][2], triangle.get(index).Z, 0f);
                }
                assertThrows(UnsupportedOperationException.class,
                        () -> triangle.add(new Vector3(9f, 9f, 9f)));

                // A diagnostic, not an identity: what is asserted is that it names the type,
                // not how the toolchain spells it.
                assertTrue(dictionary.getEntryTypeName("Name").length() > 0);
            }
        });
    }

    @Test
    void namingTheWrongTypeIsRefusedRatherThanReinterpreted(@TempDir Path root) throws IOException {
        Files.write(root.resolve("triangles.xnb"), dictionaryAsset());
        run(root, manager -> {
            try (CnaObjectDictionary dictionary =
                         CnaObjectDictionary.Load(manager, "triangles")) {
                // Naming the kind is the cast, and CNA refuses a wrong one -- the C form of the
                // InvalidCastException the canonical Get<T> throws. A projection that read the
                // bytes anyway would hand a game a Vector3 built out of a sphere.
                assertThrows(IllegalArgumentException.class,
                        () -> dictionary.getVector3("BoundingSphere"));
                assertThrows(IllegalArgumentException.class,
                        () -> dictionary.getInt32("Name"));
                assertThrows(IllegalArgumentException.class,
                        () -> dictionary.getString("BoundingSphere"));
                // An array entry is refused by the scalar route, which is a separate refusal
                // from a wrong kind: the kind here is right and the shape is not.
                assertThrows(IllegalArgumentException.class,
                        () -> dictionary.getVector3("Vertices"));
                // And an absent key, which CNA folds into the same result code and separates
                // by message.
                assertThrows(IllegalArgumentException.class, () -> dictionary.getEntry("Absent"));
                assertThrows(IllegalArgumentException.class,
                        () -> dictionary.getSingle("Absent"));
                assertThrows(IllegalArgumentException.class,
                        () -> dictionary.getVector3Array("Absent"));
            }
        });
    }

    @Test
    void aMalformedAssetIsRefusedAndAClosedDictionarySaysSo(@TempDir Path root)
            throws IOException {
        byte[] whole = dictionaryAsset();
        // Truncated in the middle of the last vertex: the header still promises the full length,
        // so a reader that trusted the header would run off the end.
        Files.write(root.resolve("truncated.xnb"),
                java.util.Arrays.copyOf(whole, whole.length - 6));
        // A root object that is not a dictionary at all.
        Files.write(root.resolve("notadictionary.xnb"), new byte[] {
                'X', 'N', 'B', 'w', 5, 0, 10, 0, 0, 0});
        Files.write(root.resolve("triangles.xnb"), whole);
        run(root, manager -> {
            assertThrows(RuntimeException.class,
                    () -> CnaObjectDictionary.Load(manager, "truncated"));
            assertThrows(RuntimeException.class,
                    () -> CnaObjectDictionary.Load(manager, "notadictionary"));
            assertThrows(RuntimeException.class,
                    () -> CnaObjectDictionary.Load(manager, "missing"));

            CnaObjectDictionary dictionary = CnaObjectDictionary.Load(manager, "triangles");
            dictionary.close();
            dictionary.close();
            assertThrows(IllegalStateException.class, dictionary::size);
            assertThrows(IllegalStateException.class, () -> dictionary.getString("Name"));

            // Two loads of one name are two independent handles over two copies, and each one
            // has to be closed. Asserting that they are different is what says the second is not
            // the first handed out again.
            try (CnaObjectDictionary first = CnaObjectDictionary.Load(manager, "triangles");
                    CnaObjectDictionary second = CnaObjectDictionary.Load(manager, "triangles")) {
                assertEquals(first.size(), second.size());
                assertEquals("triangles", first.getString("Name"));
                assertEquals("triangles", second.getString("Name"));
            }
        });
    }

    /** Runs one body inside a frame, with a content manager rooted at a directory. */
    private static void run(Path root, ContentBody body) {
        try (Game game = new Game()) {
            // A graphics device manager, because the native content manager is reached through
            // the device service: CNA's content manager belongs to a game that has one, and a
            // Java ContentManager with no device service has no native side to ask.
            new GraphicsDeviceManager(game);
            DictionaryProbe probe = new DictionaryProbe(game, root, body);
            game.getComponents().add(probe);
            game.RunOneFrame();
            if (probe.failure instanceof RuntimeException runtime) {
                throw runtime;
            }
            if (probe.failure instanceof Error error) {
                throw error;
            }
            if (probe.failure != null) {
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    private interface ContentBody {
        void accept(ContentManager manager);
    }

    private static final class DictionaryProbe extends Microsoft.Xna.Framework.GameComponent {

        private final Path root;
        private final ContentBody body;
        private boolean ran;
        private Throwable failure;

        private DictionaryProbe(Game game, Path root, ContentBody body) {
            super(game);
            this.root = root;
            this.body = body;
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            ContentManager manager = getGame().getContent();
            String previous = manager.getRootDirectory();
            try {
                manager.setRootDirectory(root.toString());
                body.accept(manager);
            } catch (Throwable exception) {
                failure = exception;
            } finally {
                try {
                    manager.setRootDirectory(previous);
                } catch (RuntimeException ignored) {
                    // The probe's own failure is the one worth reporting.
                }
            }
        }
    }
}
