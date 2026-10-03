package Microsoft.Xna.Framework.Content;

import Microsoft.Xna.Framework.GameServiceContainer;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import System.Action;
import System.Resources.ResourceManager;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;
import org.openeggbert.cna.content.ContentTypeReaderRegistry;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

@SuppressWarnings("try")
final class ContentReaderTests {

    private static final String ASSET_READER = "fixtures.AssetReader, Fixtures";
    private static final String THROWING_READER = "fixtures.ThrowingReader, Fixtures";
    private static final String SHARED_OWNER_READER = "fixtures.SharedOwnerReader, Fixtures";
    private static final String DISPOSABLE_READER = "fixtures.DisposableReader, Fixtures";
    private static final String SINGLETON_READER = "fixtures.SingletonReader, Fixtures";
    private static final String EXTERNAL_OWNER_READER = "fixtures.ExternalOwnerReader, Fixtures";
    private static final String EXISTING_OWNER_READER = "fixtures.ExistingOwnerReader, Fixtures";
    private static final String MUTABLE_READER = "fixtures.MutableReader, Fixtures";
    private static final String BAD_MUTABLE_READER = "fixtures.BadMutableReader, Fixtures";
    private static final String PARTIAL_FAILURE_READER =
            "fixtures.PartialFailureReader, Fixtures";
    private static final String STRING_READER =
            "Microsoft.Xna.Framework.Content.StringReader, Microsoft.Xna.Framework";
    private static final String TEXTURE_2D_READER =
            "Microsoft.Xna.Framework.Content.Texture2DReader, Microsoft.Xna.Framework.Graphics";

    @Test
    void customReaderLoadsThroughOrdinaryManagerPathAndCachesIdentity(
            @TempDir Path root) throws Exception {
        AtomicReference<AssetReader> activated = new AtomicReference<>();
        try (AutoCloseable registration = ContentTypeReaderRegistry.register(
                ASSET_READER, () -> {
                    AssetReader reader = new AssetReader();
                    activated.set(reader);
                    return reader;
                })) {
            writeAsset(root, "custom", xnb(
                    readers(entry(ASSET_READER, 0)), 0,
                    output -> {
                        output.seven(1);
                        output.string("managed-xnb");
                        output.int32(73);
                    }));

            try (ContentManager content = manager(root)) {
                TestAsset first = content.Load(TestAsset.class, "custom");
                TestAsset second = content.Load(TestAsset.class, "custom");
                assertEquals("managed-xnb", first.name);
                assertEquals(73, first.value);
                assertSame(first, second);
                assertNotNull(activated.get());
                assertTrue(activated.get().initialized);
                assertThrows(ContentLoadException.class,
                        () -> content.Load(String.class, "custom"));
            }
        }
    }

    @Test
    void headerAndReaderTableValidationRejectsEveryMalformedFixture(
            @TempDir Path root) throws Exception {
        byte[] valid = xnb(readers(entry(STRING_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.string("valid");
                });
        writeAsset(root, "valid", valid);

        byte[] badMagic = valid.clone();
        badMagic[0] = 'Z';
        writeAsset(root, "bad-magic", badMagic);
        byte[] badPlatform = valid.clone();
        badPlatform[3] = 'x';
        writeAsset(root, "bad-platform", badPlatform);
        byte[] badVersion = valid.clone();
        badVersion[4] = 4;
        writeAsset(root, "bad-version", badVersion);
        byte[] badSize = valid.clone();
        badSize[6]++;
        writeAsset(root, "bad-size", badSize);
        writeAsset(root, "truncated", new byte[] {'X', 'N', 'B', 'w', 5});
        writeAsset(root, "reader-count", rawXnb(output -> output.seven(4097)));
        writeAsset(root, "unknown-reader", xnb(
                readers(entry("fixtures.UnknownReader, Fixtures", 0)), 0,
                output -> output.seven(0)));
        writeAsset(root, "version-mismatch", xnb(
                readers(entry(STRING_READER, 7)), 0,
                output -> output.seven(0)));
        writeAsset(root, "invalid-index", xnb(
                readers(entry(STRING_READER, 0)), 0,
                output -> output.seven(2)));

        try (ContentManager content = manager(root)) {
            assertEquals("valid", content.Load(String.class, "valid"));
            assertBad(content, "bad-magic", "magic");
            assertBad(content, "bad-platform", "platform");
            assertBad(content, "bad-version", "version");
            assertBad(content, "bad-size", "size");
            assertBad(content, "truncated", "truncated");
            assertBad(content, "reader-count", "reader count");
            assertBad(content, "unknown-reader", "Unknown content type reader");
            assertBad(content, "version-mismatch", "version mismatch");
            assertBad(content, "invalid-index", "reader index");
        }
    }

    @Test
    void lzxSingleAndMultiFrameAssetsUseTheOrdinaryReaderAndCache(
            @TempDir Path root) throws Exception {
        byte[] single = xnb(readers(entry(STRING_READER, 0)), 0, output -> {
            output.seven(1);
            output.string("single compressed frame");
        });
        byte[] multi = xnb(readers(entry(STRING_READER, 0)), 0, output -> {
            output.seven(1);
            output.string("two persistent LZX frames");
        });
        writeAsset(root, "single-lzx", compressXnb(single));
        writeAsset(root, "multi-lzx", compressXnb(multi, 20));

        try (ContentManager content = manager(root)) {
            String singleValue = content.Load(String.class, "single-lzx");
            assertEquals("single compressed frame", singleValue);
            String first = content.Load(String.class, "multi-lzx");
            String second = content.Load(String.class, "multi-lzx");
            assertEquals("two persistent LZX frames", first);
            assertSame(first, second);
        }
    }

    @Test
    void lzxDefaultFrameAndExactOutputLengthAreValidated() {
        byte[] payload = new byte[0x8000];
        for (int i = 0; i < payload.length; i++) {
            payload[i] = (byte) (i * 37 + 11);
        }
        byte[] block = lzxUncompressedBlock(payload, true);
        byte[] framed = new byte[2 + block.length];
        framed[0] = (byte) (block.length >>> 8);
        framed[1] = (byte) block.length;
        System.arraycopy(block, 0, framed, 2, block.length);
        assertArrayEquals(payload,
                XnbLzxDecompression.decompress(framed, payload.length, "default-frame"));

        assertTrue(assertThrows(ContentLoadException.class,
                () -> XnbLzxDecompression.decompress(framed, payload.length - 1,
                        "too-long")).getMessage().contains("exceeds"));
        assertTrue(assertThrows(ContentLoadException.class,
                () -> XnbLzxDecompression.decompress(framed, payload.length + 1,
                        "too-short")).getMessage().contains("decompressed size"));
    }

    @Test
    void lzxFramingRejectsTruncationMalformedLengthsTrailingDataAndDecoderFailure() {
        byte[] validBlock = lzxUncompressedBlock(new byte[] {1, 2, 3, 4}, true);
        byte[] valid = extendedLzxFrame(validBlock, 4);

        assertLzxBad(new byte[] {(byte) 0xff}, 4, "header");
        assertLzxBad(new byte[] {(byte) 0xff, 0, 4, 0}, 4, "header");

        byte[] truncatedBlock = valid.clone();
        truncatedBlock[3] = 0x7f;
        truncatedBlock[4] = (byte) 0xff;
        assertLzxBad(truncatedBlock, 4, "truncated LZX block");

        byte[] zeroBlock = valid.clone();
        zeroBlock[3] = 0;
        zeroBlock[4] = 0;
        assertLzxBad(zeroBlock, 4, "block length");

        byte[] zeroFrame = valid.clone();
        zeroFrame[1] = 0;
        zeroFrame[2] = 0;
        assertLzxBad(zeroFrame, 4, "frame length");

        byte[] oversizedFrame = valid.clone();
        oversizedFrame[1] = 0;
        oversizedFrame[2] = 5;
        assertLzxBad(oversizedFrame, 4, "exceeds");

        byte[] trailing = java.util.Arrays.copyOf(valid, valid.length + 1);
        trailing[trailing.length - 1] = 1;
        assertLzxBad(trailing, 4, "header");

        byte[] invalidDecoderBlock = new byte[] {0, 0, 0, 0};
        assertLzxBad(extendedLzxFrame(invalidDecoderBlock, 1), 1, "decompression");
    }

    @Test
    void compressedReaderFailureCleansPartialResourcesAndUnloadClosesCachedAsset(
            @TempDir Path root) throws Exception {
        CloseTracked.closeCount.set(0);
        byte[] partial = xnb(
                readers(entry(PARTIAL_FAILURE_READER, 0), entry(DISPOSABLE_READER, 0)),
                0, output -> {
                    output.seven(1);
                    output.int32(81);
                });
        byte[] disposable = xnb(readers(entry(DISPOSABLE_READER, 0)), 0, output -> {
            output.seven(1);
            output.int32(82);
        });
        writeAsset(root, "compressed-partial", compressXnb(partial));
        writeAsset(root, "compressed-disposable", compressXnb(disposable));

        try (AutoCloseable partialRegistration = ContentTypeReaderRegistry.register(
                PARTIAL_FAILURE_READER, PartialFailureReader::new);
             AutoCloseable disposableRegistration = ContentTypeReaderRegistry.register(
                     DISPOSABLE_READER, DisposableReader::new);
             ContentManager content = manager(root)) {
            assertBad(content, "compressed-partial", "intentional partial failure");
            assertEquals(1, CloseTracked.closeCount.get());

            CloseTracked first = content.Load(CloseTracked.class, "compressed-disposable");
            assertSame(first, content.Load(CloseTracked.class, "compressed-disposable"));
            content.Unload();
            assertEquals(2, CloseTracked.closeCount.get());
        }
    }

    @Test
    void xnbNativeResourceRequestUsesManagedReaderDispatchBeforeLooseLoading(
            @TempDir Path root) throws Exception {
        writeAsset(root, "texture", xnb(
                readers(entry(TEXTURE_2D_READER, 0)), 0, output -> {
                    output.seven(1);
                    output.int32(0);
                    output.int32(0);
                    output.int32(1);
                    output.int32(1);
                }));
        try (ContentManager content = manager(root)) {
            ContentLoadException failure = assertThrows(
                    ContentLoadException.class,
                    () -> content.Load(Texture2D.class, "texture"));
            assertTrue(failureMessages(failure).contains("dimensions"));
        }
    }

    @Test
    void activationReaderFailureAndWrongRequestedTypeAreReportedAsContentFailures(
            @TempDir Path root) throws Exception {
        writeAsset(root, "activation", xnb(
                readers(entry(THROWING_READER, 0)), 0, output -> output.seven(0)));
        writeAsset(root, "throwing", xnb(
                readers(entry(THROWING_READER, 0)), 0, output -> output.seven(1)));
        writeAsset(root, "wrong-type", xnb(
                readers(entry(ASSET_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.string("wrong");
                    output.int32(1);
                }));

        try (AutoCloseable activation = ContentTypeReaderRegistry.register(
                THROWING_READER, () -> {
                    throw new IllegalStateException("activation exploded");
                }); ContentManager content = manager(root)) {
            assertBad(content, "activation", "activate");
        }
        try (AutoCloseable throwing = ContentTypeReaderRegistry.register(
                THROWING_READER, ThrowingReader::new);
             AutoCloseable asset = ContentTypeReaderRegistry.register(
                     ASSET_READER, AssetReader::new);
             ContentManager content = manager(root)) {
            assertBad(content, "throwing", "failed while loading");
            ContentLoadException mismatch = assertThrows(ContentLoadException.class,
                    () -> content.Load(String.class, "wrong-type"));
            assertTrue(mismatch.getMessage().contains("not java.lang.String"));
        }
    }

    @Test
    void sharedResourcesAreDeferredTypeCheckedAndOwnedByManager(
            @TempDir Path root) throws Exception {
        CloseTracked.closeCount.set(0);
        writeAsset(root, "shared", xnb(
                readers(entry(SHARED_OWNER_READER, 0), entry(DISPOSABLE_READER, 0)), 1,
                output -> {
                    output.seven(1);
                    output.seven(1);
                    output.seven(2);
                    output.int32(19);
                }));
        writeAsset(root, "bad-shared-index", xnb(
                readers(entry(SHARED_OWNER_READER, 0), entry(DISPOSABLE_READER, 0)), 1,
                output -> {
                    output.seven(1);
                    output.seven(2);
                    output.seven(2);
                    output.int32(20);
                }));
        try (AutoCloseable owner = ContentTypeReaderRegistry.register(
                SHARED_OWNER_READER, SharedOwnerReader::new);
             AutoCloseable disposable = ContentTypeReaderRegistry.register(
                     DISPOSABLE_READER, DisposableReader::new)) {
            ContentManager content = manager(root);
            SharedOwner loaded = content.Load(SharedOwner.class, "shared");
            assertTrue(loaded.fixupRan);
            assertNotNull(loaded.value);
            assertEquals(19, loaded.value.value);
            assertEquals(0, CloseTracked.closeCount.get());
            assertBad(content, "bad-shared-index", "shared resource index");
            content.Unload();
            assertEquals(1, CloseTracked.closeCount.get());
            content.close();
        }
    }

    @Test
    void duplicateDisposableOccurrencesAreRecordedAndReleasedPerRead(
            @TempDir Path root) throws Exception {
        CloseTracked singleton = new CloseTracked(31);
        CloseTracked.closeCount.set(0);
        writeAsset(root, "duplicate", xnb(
                readers(entry(SINGLETON_READER, 0)), 1,
                output -> {
                    output.seven(1);
                    output.seven(1);
                }));
        try (AutoCloseable registration = ContentTypeReaderRegistry.register(
                SINGLETON_READER, () -> new SingletonReader(singleton))) {
            ContentManager content = manager(root);
            assertSame(singleton, content.Load(CloseTracked.class, "duplicate"));
            content.Unload();
            assertEquals(2, CloseTracked.closeCount.get());
            content.close();
        }
    }

    @Test
    void externalReferencesResolveRelativeToParentAndUseTheManagerCache(
            @TempDir Path root) throws Exception {
        writeAsset(root, "folder/child", xnb(
                readers(entry(STRING_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.string("external-value");
                }));
        writeAsset(root, "folder/parent", xnb(
                readers(entry(EXTERNAL_OWNER_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.string("child");
                }));
        try (AutoCloseable registration = ContentTypeReaderRegistry.register(
                EXTERNAL_OWNER_READER, ExternalOwnerReader::new);
             ContentManager content = manager(root)) {
            ExternalOwner loaded = content.Load(ExternalOwner.class, "folder/parent");
            assertEquals("external-value", loaded.value);
            assertSame(loaded.value, content.Load(String.class, "folder/child"));
        }
    }

    @Test
    void existingInstanceIdentityIsRequiredAndPartialFailureCleansResources(
            @TempDir Path root) throws Exception {
        CloseTracked.closeCount.set(0);
        writeAsset(root, "existing", xnb(
                readers(entry(EXISTING_OWNER_READER, 0), entry(MUTABLE_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.int32(44);
                }));
        writeAsset(root, "bad-existing", xnb(
                readers(entry(EXISTING_OWNER_READER, 0), entry(BAD_MUTABLE_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.int32(45);
                }));
        writeAsset(root, "partial", xnb(
                readers(entry(PARTIAL_FAILURE_READER, 0), entry(DISPOSABLE_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.int32(52);
                }));
        writeAsset(root, "after-failure", xnb(
                readers(entry(STRING_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.string("still-usable");
                }));

        try (AutoCloseable existingOwner = ContentTypeReaderRegistry.register(
                EXISTING_OWNER_READER, ExistingOwnerReader::new);
             AutoCloseable mutable = ContentTypeReaderRegistry.register(
                     MUTABLE_READER, MutableReader::new);
             AutoCloseable badMutable = ContentTypeReaderRegistry.register(
                     BAD_MUTABLE_READER, BadMutableReader::new);
             AutoCloseable partial = ContentTypeReaderRegistry.register(
                     PARTIAL_FAILURE_READER, PartialFailureReader::new);
             AutoCloseable disposable = ContentTypeReaderRegistry.register(
                     DISPOSABLE_READER, DisposableReader::new);
             ContentManager content = manager(root)) {
            ExistingOwner loaded = content.Load(ExistingOwner.class, "existing");
            assertTrue(loaded.identityPreserved);
            assertEquals(44, loaded.value.value);
            assertBad(content, "bad-existing", "constructed a new value");
            assertBad(content, "partial", "intentional partial failure");
            assertEquals(1, CloseTracked.closeCount.get());
            assertEquals("still-usable", content.Load(String.class, "after-failure"));
        }
    }

    @Test
    void explicitDisposableRecorderAndResourceContentManagerRemainIndependent(
            @TempDir Path root) throws Exception {
        CloseTracked.closeCount.set(0);
        byte[] fixture = xnb(readers(entry(DISPOSABLE_READER, 0)), 0,
                output -> {
                    output.seven(1);
                    output.int32(65);
                });
        writeAsset(root, "recorded", fixture);
        List<AutoCloseable> recorded = new ArrayList<>();
        try (AutoCloseable registration = ContentTypeReaderRegistry.register(
                DISPOSABLE_READER, DisposableReader::new)) {
            try (ExposedContentManager content = new ExposedContentManager(root)) {
                CloseTracked value = content.readWithRecorder(
                        CloseTracked.class, "recorded", recorded::add);
                assertEquals(65, value.value);
                assertEquals(List.of(value), recorded);
            }
            assertEquals(0, CloseTracked.closeCount.get());
            recorded.get(0).close();
            assertEquals(1, CloseTracked.closeCount.get());

            try (ResourceContentManager resources = new ResourceContentManager(
                    type -> null, new ResourceManager(Map.of("embedded", fixture)))) {
                CloseTracked embedded = resources.Load(CloseTracked.class, "embedded");
                assertEquals(65, embedded.value);
            }
            assertEquals(2, CloseTracked.closeCount.get());
        }
    }

    private static ContentManager manager(Path root) {
        return new ContentManager(new GameServiceContainer(), root.toString());
    }

    private static void assertBad(
            ContentManager content, String name, String expectedMessagePart) {
        ContentLoadException failure = assertThrows(
                ContentLoadException.class, () -> content.Load(Object.class, name));
        String messages = failureMessages(failure);
        assertTrue(messages.contains(expectedMessagePart), () -> messages);
    }

    private static ReaderEntry entry(String name, int version) {
        return new ReaderEntry(name, version);
    }

    private static ReaderEntry[] readers(ReaderEntry... entries) {
        return entries;
    }

    private static byte[] xnb(
            ReaderEntry[] readers, int sharedCount, Payload payload) {
        Writer output = new Writer();
        output.seven(readers.length);
        for (ReaderEntry reader : readers) {
            output.string(reader.name);
            output.int32(reader.version);
        }
        output.seven(sharedCount);
        payload.write(output);
        return frame(output.toByteArray());
    }

    private static byte[] rawXnb(Payload payload) {
        Writer output = new Writer();
        payload.write(output);
        return frame(output.toByteArray());
    }

    private static byte[] frame(byte[] body) {
        byte[] result = new byte[10 + body.length];
        result[0] = 'X';
        result[1] = 'N';
        result[2] = 'B';
        result[3] = 'w';
        result[4] = 5;
        int size = result.length;
        result[6] = (byte)size;
        result[7] = (byte)(size >>> 8);
        result[8] = (byte)(size >>> 16);
        result[9] = (byte)(size >>> 24);
        System.arraycopy(body, 0, result, 10, body.length);
        return result;
    }

    private static byte[] compressXnb(byte[] uncompressed) {
        return compressXnb(uncompressed, -1);
    }

    private static byte[] compressXnb(byte[] uncompressed, int splitAt) {
        byte[] body = java.util.Arrays.copyOfRange(uncompressed, 10, uncompressed.length);
        ByteArrayOutputStream framed = new ByteArrayOutputStream();
        if (splitAt < 0) {
            framed.writeBytes(extendedLzxFrame(lzxUncompressedBlock(body, true), body.length));
        } else {
            if (splitAt <= 0 || splitAt >= body.length || (splitAt & 1) != 0) {
                throw new IllegalArgumentException("splitAt must be an even position inside the payload");
            }
            byte[] first = java.util.Arrays.copyOfRange(body, 0, splitAt);
            byte[] second = java.util.Arrays.copyOfRange(body, splitAt, body.length);
            framed.writeBytes(extendedLzxFrame(lzxUncompressedBlock(first, true), first.length));
            framed.writeBytes(extendedLzxFrame(lzxUncompressedBlock(second, false), second.length));
        }

        byte[] compressedPayload = framed.toByteArray();
        byte[] result = new byte[14 + compressedPayload.length];
        result[0] = 'X';
        result[1] = 'N';
        result[2] = 'B';
        result[3] = 'w';
        result[4] = 5;
        result[5] = (byte) 0x80;
        int totalSize = result.length;
        result[6] = (byte) totalSize;
        result[7] = (byte) (totalSize >>> 8);
        result[8] = (byte) (totalSize >>> 16);
        result[9] = (byte) (totalSize >>> 24);
        result[10] = (byte) body.length;
        result[11] = (byte) (body.length >>> 8);
        result[12] = (byte) (body.length >>> 16);
        result[13] = (byte) (body.length >>> 24);
        System.arraycopy(compressedPayload, 0, result, 14, compressedPayload.length);
        return result;
    }

    private static byte[] extendedLzxFrame(byte[] block, int frameSize) {
        if (frameSize <= 0 || frameSize > 0x8000 || block.length > 0xffff) {
            throw new IllegalArgumentException("invalid generated LZX frame");
        }
        byte[] framed = new byte[5 + block.length];
        framed[0] = (byte) 0xff;
        framed[1] = (byte) (frameSize >>> 8);
        framed[2] = (byte) frameSize;
        framed[3] = (byte) (block.length >>> 8);
        framed[4] = (byte) block.length;
        System.arraycopy(block, 0, framed, 5, block.length);
        return framed;
    }

    private static byte[] lzxUncompressedBlock(byte[] payload, boolean firstBlock) {
        if (payload.length <= 0 || payload.length > 0x8000) {
            throw new IllegalArgumentException("invalid generated LZX block length");
        }
        int headerBits = firstBlock
                ? 3 << 28 | payload.length << 4
                : 3 << 29 | payload.length << 5;
        byte[] block = new byte[16 + payload.length];
        block[0] = (byte) (headerBits >>> 16);
        block[1] = (byte) (headerBits >>> 24);
        block[2] = (byte) headerBits;
        block[3] = (byte) (headerBits >>> 8);
        block[4] = 1;
        block[8] = 1;
        block[12] = 1;
        System.arraycopy(payload, 0, block, 16, payload.length);
        return block;
    }

    private static void assertLzxBad(byte[] compressed, int decompressedSize,
            String expectedMessagePart) {
        ContentLoadException failure = assertThrows(ContentLoadException.class,
                () -> XnbLzxDecompression.decompress(
                        compressed, decompressedSize, "malformed"));
        assertTrue(failure.getMessage().contains(expectedMessagePart), failure::getMessage);
    }

    private static String failureMessages(Throwable failure) {
        StringBuilder result = new StringBuilder();
        Throwable current = failure;
        while (current != null) {
            if (result.length() != 0) result.append(" -> ");
            result.append(current.getMessage());
            current = current.getCause();
        }
        return result.toString();
    }

    private static void writeAsset(Path root, String name, byte[] bytes) throws IOException {
        Path path = root.resolve(name + ".xnb");
        Files.createDirectories(path.getParent());
        Files.write(path, bytes);
    }

    private record ReaderEntry(String name, int version) {
    }

    @FunctionalInterface
    private interface Payload {
        void write(Writer output);
    }

    private static final class Writer {
        private final ByteArrayOutputStream output = new ByteArrayOutputStream();

        private void seven(int value) {
            int remaining = value;
            do {
                int next = remaining & 0x7f;
                remaining >>>= 7;
                if (remaining != 0) next |= 0x80;
                output.write(next);
            } while (remaining != 0);
        }

        private void int32(int value) {
            output.write(value);
            output.write(value >>> 8);
            output.write(value >>> 16);
            output.write(value >>> 24);
        }

        private void string(String value) {
            byte[] encoded = value.getBytes(StandardCharsets.UTF_8);
            seven(encoded.length);
            output.writeBytes(encoded);
        }

        private byte[] toByteArray() {
            return output.toByteArray();
        }
    }

    private static final class TestAsset {
        private final String name;
        private final int value;

        private TestAsset(String name, int value) {
            this.name = name;
            this.value = value;
        }
    }

    private static final class AssetReader extends ContentTypeReaderOfT<TestAsset> {
        private boolean initialized;

        @Override
        protected void Initialize(ContentTypeReaderManager manager) {
            super.Initialize(manager);
            assertSame(this, manager.GetTypeReader(TestAsset.class));
            initialized = true;
        }

        @Override
        protected TestAsset ReadTyped(ContentReader input, TestAsset existingInstance) {
            assertNull(existingInstance);
            return new TestAsset(input.ReadString(), input.ReadInt32());
        }
    }

    private static final class ThrowingReader extends ContentTypeReaderOfT<TestAsset> {
        @Override
        protected TestAsset ReadTyped(ContentReader input, TestAsset existingInstance) {
            throw new IllegalStateException("reader body exploded");
        }
    }

    private static final class SharedOwner {
        private CloseTracked value;
        private boolean fixupRan;
    }

    private static final class SharedOwnerReader extends ContentTypeReaderOfT<SharedOwner> {
        @Override
        protected SharedOwner ReadTyped(ContentReader input, SharedOwner existingInstance) {
            SharedOwner value = new SharedOwner();
            input.ReadSharedResource(CloseTracked.class, shared -> {
                value.value = shared;
                value.fixupRan = true;
            });
            assertFalse(value.fixupRan);
            return value;
        }
    }

    private static final class CloseTracked implements AutoCloseable {
        private static final AtomicInteger closeCount = new AtomicInteger();
        private final int value;

        private CloseTracked(int value) {
            this.value = value;
        }

        @Override
        public void close() {
            closeCount.incrementAndGet();
        }
    }

    private static final class DisposableReader extends ContentTypeReaderOfT<CloseTracked> {
        @Override
        protected CloseTracked ReadTyped(ContentReader input, CloseTracked existingInstance) {
            return new CloseTracked(input.ReadInt32());
        }
    }

    private static final class SingletonReader extends ContentTypeReaderOfT<CloseTracked> {
        private final CloseTracked singleton;

        private SingletonReader(CloseTracked singleton) {
            this.singleton = singleton;
        }

        @Override
        protected CloseTracked ReadTyped(ContentReader input, CloseTracked existingInstance) {
            return singleton;
        }
    }

    private static final class ExternalOwner {
        private final String value;

        private ExternalOwner(String value) {
            this.value = value;
        }
    }

    private static final class ExternalOwnerReader extends ContentTypeReaderOfT<ExternalOwner> {
        @Override
        protected ExternalOwner ReadTyped(ContentReader input, ExternalOwner existingInstance) {
            return new ExternalOwner(input.ReadExternalReference(String.class));
        }
    }

    private static final class MutableValue {
        private int value;
    }

    private static final class ExistingOwner {
        private final MutableValue value;
        private final boolean identityPreserved;

        private ExistingOwner(MutableValue value, boolean identityPreserved) {
            this.value = value;
            this.identityPreserved = identityPreserved;
        }
    }

    private static final class ExistingOwnerReader extends ContentTypeReaderOfT<ExistingOwner> {
        private ContentTypeReader mutableReader;

        @Override
        protected void Initialize(ContentTypeReaderManager manager) {
            mutableReader = manager.GetTypeReader(MutableValue.class);
        }

        @Override
        protected ExistingOwner ReadTyped(ContentReader input, ExistingOwner existingInstance) {
            MutableValue value = new MutableValue();
            MutableValue returned = input.ReadRawObject(
                    MutableValue.class, mutableReader, value);
            return new ExistingOwner(value, value == returned);
        }
    }

    private static class MutableReader extends ContentTypeReaderOfT<MutableValue> {
        @Override
        public boolean getCanDeserializeIntoExistingObject() {
            return true;
        }

        @Override
        protected MutableValue ReadTyped(ContentReader input, MutableValue existingInstance) {
            MutableValue value = existingInstance == null ? new MutableValue() : existingInstance;
            value.value = input.ReadInt32();
            return value;
        }
    }

    private static final class BadMutableReader extends MutableReader {
        @Override
        protected MutableValue ReadTyped(ContentReader input, MutableValue existingInstance) {
            MutableValue replacement = new MutableValue();
            replacement.value = input.ReadInt32();
            return replacement;
        }
    }

    private static final class PartialFailureReader extends ContentTypeReaderOfT<TestAsset> {
        private ContentTypeReader disposableReader;

        @Override
        protected void Initialize(ContentTypeReaderManager manager) {
            disposableReader = manager.GetTypeReader(CloseTracked.class);
        }

        @Override
        protected TestAsset ReadTyped(ContentReader input, TestAsset existingInstance) {
            input.ReadRawObject(CloseTracked.class, disposableReader);
            throw new IllegalStateException("intentional partial failure");
        }
    }

    private static final class ExposedContentManager extends ContentManager {
        private ExposedContentManager(Path root) {
            super(type -> null, root.toString());
        }

        private <T> T readWithRecorder(
                Class<T> type, String name, Action<AutoCloseable> recorder) {
            return ReadAsset(type, name, recorder);
        }
    }
}
