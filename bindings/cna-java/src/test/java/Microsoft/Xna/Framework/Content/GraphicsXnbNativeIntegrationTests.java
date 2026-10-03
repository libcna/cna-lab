package Microsoft.Xna.Framework.Content;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Graphics.BasicEffect;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.IndexBuffer;
import Microsoft.Xna.Framework.Graphics.Model;
import Microsoft.Xna.Framework.Graphics.ModelMeshPart;
import Microsoft.Xna.Framework.Graphics.SpriteBatch;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Graphics.SurfaceFormat;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.VertexBuffer;
import Microsoft.Xna.Framework.Graphics.VertexElementFormat;
import Microsoft.Xna.Framework.Graphics.VertexElementUsage;
import Microsoft.Xna.Framework.Media.Video;
import Microsoft.Xna.Framework.Media.VideoPlayer;
import Microsoft.Xna.Framework.Media.VideoSoundtrackType;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.junit.jupiter.api.io.TempDir;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class GraphicsXnbNativeIntegrationTests {

    private static final String TEXTURE_READER =
            "Microsoft.Xna.Framework.Content.Texture2DReader, Microsoft.Xna.Framework.Graphics";
    private static final String SPRITE_FONT_READER =
            "Microsoft.Xna.Framework.Content.SpriteFontReader, Microsoft.Xna.Framework.Graphics";
    private static final String RECTANGLE_READER =
            "Microsoft.Xna.Framework.Content.RectangleReader, Microsoft.Xna.Framework";
    private static final String CHAR_READER =
            "Microsoft.Xna.Framework.Content.CharReader, Microsoft.Xna.Framework";
    private static final String VECTOR3_READER =
            "Microsoft.Xna.Framework.Content.Vector3Reader, Microsoft.Xna.Framework";
    private static final String RECTANGLE_LIST_READER =
            "Microsoft.Xna.Framework.Content.ListReader`1[[Microsoft.Xna.Framework.Rectangle, "
                    + "Microsoft.Xna.Framework]], Microsoft.Xna.Framework";
    private static final String CHAR_LIST_READER =
            "Microsoft.Xna.Framework.Content.ListReader`1[[System.Char, mscorlib]], "
                    + "Microsoft.Xna.Framework";
    private static final String VECTOR3_LIST_READER =
            "Microsoft.Xna.Framework.Content.ListReader`1[[Microsoft.Xna.Framework.Vector3, "
                    + "Microsoft.Xna.Framework]], Microsoft.Xna.Framework";
    private static final String MODEL_READER =
            "Microsoft.Xna.Framework.Content.ModelReader, Microsoft.Xna.Framework.Graphics";
    private static final String VERTEX_BUFFER_READER =
            "Microsoft.Xna.Framework.Content.VertexBufferReader, Microsoft.Xna.Framework.Graphics";
    private static final String VERTEX_DECLARATION_READER =
            "Microsoft.Xna.Framework.Content.VertexDeclarationReader, "
                    + "Microsoft.Xna.Framework.Graphics";
    private static final String INDEX_BUFFER_READER =
            "Microsoft.Xna.Framework.Content.IndexBufferReader, Microsoft.Xna.Framework.Graphics";
    private static final String BASIC_EFFECT_READER =
            "Microsoft.Xna.Framework.Content.BasicEffectReader, Microsoft.Xna.Framework.Graphics";
    private static final String STRING_READER =
            "Microsoft.Xna.Framework.Content.StringReader, Microsoft.Xna.Framework";
    private static final String INT32_READER =
            "Microsoft.Xna.Framework.Content.Int32Reader, Microsoft.Xna.Framework";
    private static final String SINGLE_READER =
            "Microsoft.Xna.Framework.Content.SingleReader, Microsoft.Xna.Framework";
    private static final String VIDEO_READER =
            "Microsoft.Xna.Framework.Content.VideoReader, Microsoft.Xna.Framework.Video";

    @Test
    void texture2DXnbLoadsMipsCachesOwnsAndRejectsMalformedPayloads(
            @TempDir Path root) throws Exception {
        byte[] colorFixture = textureXnb(2, 2, new byte[][]{
                rgba(255, 0, 0, 255, 0, 255, 0, 255,
                        0, 0, 255, 255, 255, 255, 255, 255),
                rgba(7, 8, 9, 10)
        });
        write(root, "color", colorFixture);
        write(root, "color-compressed", compressXnb(colorFixture));
        write(root, "wrong-type", textureXnb(1, 1,
                new byte[][]{rgba(1, 2, 3, 4)}));
        write(root, "bad-dimensions", textureXnb(0, 1,
                new byte[][]{new byte[0]}));
        write(root, "bad-mips", textureXnb(4, 4,
                new byte[][]{new byte[64], new byte[16]}));
        write(root, "unsupported", textureXnb(SurfaceFormat.Dxt1.ordinal(), 4, 4,
                new byte[][]{new byte[8]}));
        write(root, "bad-payload", textureXnbWithDeclaredPayload(
                1, 1, 3, rgba(1, 2, 3)));
        write(root, "truncated", textureXnbWithDeclaredPayload(
                1, 1, 4, rgba(1, 2, 3)));

        try (TextureGame game = new TextureGame(root)) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    @Test
    void spriteFontXnbBuildsRealAtlasMeasuresDrawsAndUnloads(
            @TempDir Path root) throws Exception {
        write(root, "font", spriteFontXnb(true));
        write(root, "font-no-default", spriteFontXnb(false));
        try (FontGame game = new FontGame(root)) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    @Test
    void modelXnbBuildsSharedNativeBuffersEffectGraphAndDraws(
            @TempDir Path root) throws Exception {
        write(root, "model", modelXnb());
        try (ModelGame game = new ModelGame(root)) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    @Test
    void videoXnbUsesAuthoritativeBoxedFieldLayoutCachesAndUnloads(
            @TempDir Path root) throws Exception {
        Files.createDirectories(root.resolve("videos"));
        write(root.resolve("videos"), "clip", videoXnb(
                "missing-frame-source.mp4", 1234, 320, 180, 29.97f,
                VideoSoundtrackType.MusicAndDialog));
        try (VideoGame game = new VideoGame(root)) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    private static final class TextureGame extends Game {
        private boolean completed;

        private TextureGame(Path root) {
            new GraphicsDeviceManager(this);
            getContent().setRootDirectory(root.toString());
        }

        @Override
        protected void Update(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();
            AtomicInteger created = new AtomicInteger();
            AtomicInteger destroyed = new AtomicInteger();
            device.addResourceCreatedListener((sender, args) -> created.incrementAndGet());
            device.addResourceDestroyedListener((sender, args) -> destroyed.incrementAndGet());

            int createdBeforeFailure = created.get();
            int destroyedBeforeFailure = destroyed.get();
            assertFailure("bad-payload", "payload length");
            assertEquals(createdBeforeFailure + 1, created.get());
            assertEquals(destroyedBeforeFailure + 1, destroyed.get());
            assertFailure("truncated", "truncated byte payload");
            assertFailure("bad-dimensions", "dimensions");
            assertFailure("bad-mips", "mip count");
            assertFailure("unsupported", "requires CNA surface-format support");

            int createdBeforeWrongType = created.get();
            int destroyedBeforeWrongType = destroyed.get();
            assertThrows(ContentLoadException.class,
                    () -> getContent().Load(String.class, "wrong-type"));
            assertEquals(createdBeforeWrongType + 1, created.get());
            assertEquals(destroyedBeforeWrongType + 1, destroyed.get());

            Texture2D first;
            try {
                first = getContent().Load(Texture2D.class, "color");
            } catch (ContentLoadException failure) {
                throw new AssertionError(messages(failure), failure);
            }
            assertSame(first, getContent().Load(Texture2D.class, "COLOR"));
            assertEquals(2, first.getWidth());
            assertEquals(2, first.getHeight());
            assertEquals(2, first.getLevelCount());
            Color[] levelZero = new Color[4];
            first.GetData(0, null, levelZero, 0, levelZero.length);
            assertArrayEquals(new Color[]{
                    new Color(255, 0, 0, 255), new Color(0, 255, 0, 255),
                    new Color(0, 0, 255, 255), Color.White
            }, levelZero);
            Color[] levelOne = new Color[1];
            first.GetData(1, null, levelOne, 0, levelOne.length);
            assertArrayEquals(new Color[]{new Color(7, 8, 9, 10)}, levelOne);

            Texture2D compressed = getContent().Load(
                    Texture2D.class, "color-compressed");
            assertSame(compressed, getContent().Load(
                    Texture2D.class, "COLOR-COMPRESSED"));
            Color[] compressedLevelZero = new Color[4];
            compressed.GetData(compressedLevelZero);
            assertArrayEquals(levelZero, compressedLevelZero);

            getContent().Unload();
            assertThrows(IllegalStateException.class, first::getWidth);
            assertThrows(IllegalStateException.class, compressed::getWidth);
            Texture2D reloaded = getContent().Load(Texture2D.class, "color");
            assertNotSame(first, reloaded);
            getContent().close();
            assertThrows(IllegalStateException.class, reloaded::getWidth);
            completed = true;
        }

        private void assertFailure(String asset, String message) {
            ContentLoadException failure = assertThrows(ContentLoadException.class,
                    () -> getContent().Load(Texture2D.class, asset));
            assertTrue(messages(failure).contains(message), () -> messages(failure));
        }
    }

    private static final class FontGame extends Game {
        private boolean completed;

        private FontGame(Path root) {
            new GraphicsDeviceManager(this);
            getContent().setRootDirectory(root.toString());
        }

        @Override
        protected void Update(GameTime gameTime) {
            SpriteFont font;
            try {
                font = getContent().Load(SpriteFont.class, "font");
            } catch (ContentLoadException failure) {
                throw new AssertionError(messages(failure), failure);
            }
            assertSame(font, getContent().Load(SpriteFont.class, "FONT"));
            assertEquals(3, font.getLineSpacing());
            assertEquals(0.5f, font.getSpacing());
            assertEquals(Character.valueOf('A'), font.getDefaultCharacter());
            assertEquals(java.util.List.of('A'), font.getCharacters());
            assertEquals(new Vector2(1.0f, 3.0f), font.MeasureString("A"));
            assertEquals(new Vector2(1.0f, 3.0f), font.MeasureString("B"));

            try (SpriteBatch batch = new SpriteBatch(getGraphicsDevice())) {
                batch.Begin();
                batch.DrawString(font, "A", Vector2.getZero(), Color.White);
                batch.End();
            }

            SpriteFont noDefault = getContent().Load(
                    SpriteFont.class, "font-no-default");
            assertThrows(IllegalArgumentException.class,
                    () -> noDefault.MeasureString("B"));
            getContent().Unload();
            assertThrows(IllegalStateException.class, () -> font.MeasureString("A"));
            assertThrows(IllegalStateException.class,
                    () -> noDefault.MeasureString("A"));
            completed = true;
        }
    }

    private static final class ModelGame extends Game {
        private boolean completed;

        private ModelGame(Path root) {
            new GraphicsDeviceManager(this);
            getContent().setRootDirectory(root.toString());
        }

        @Override
        protected void Update(GameTime gameTime) {
            Model model = getContent().Load(Model.class, "model");
            assertSame(model, getContent().Load(Model.class, "MODEL"));
            assertEquals(1, model.getBones().size());
            assertEquals("Root", model.getRoot().getName());
            assertEquals(1, model.getMeshes().size());
            assertEquals("Triangle", model.getMeshes().get(0).getName());
            ModelMeshPart part = model.getMeshes().get(0).getMeshParts().get(0);
            VertexBuffer vertices = part.getVertexBuffer();
            IndexBuffer indices = part.getIndexBuffer();
            BasicEffect effect = (BasicEffect)part.getEffect();
            assertSame(effect, model.getMeshes().get(0).getEffects().get(0));
            assertEquals(3, vertices.getVertexCount());
            assertEquals(3, indices.getIndexCount());
            Integer[] indexValues = new Integer[3];
            indices.GetData(indexValues);
            assertArrayEquals(new Integer[]{0, 1, 2}, indexValues);
            assertEquals(1.0f, effect.getAlpha());
            model.Draw(Matrix.getIdentity(), Matrix.getIdentity(), Matrix.getIdentity());
            getGraphicsDevice().SetVertexBuffer(null);
            getGraphicsDevice().setIndices(null);
            getContent().Unload();
            assertThrows(IllegalStateException.class, vertices::getVertexCount);
            assertThrows(IllegalStateException.class, indices::getIndexCount);
            assertThrows(IllegalStateException.class, effect::getAlpha);
            completed = true;
        }
    }

    private static final class VideoGame extends Game {
        private boolean completed;

        private VideoGame(Path root) {
            new GraphicsDeviceManager(this);
            getContent().setRootDirectory(root.toString());
        }

        @Override
        protected void Update(GameTime gameTime) {
            Video video = getContent().Load(Video.class, "videos/clip");
            assertSame(video, getContent().Load(Video.class, "VIDEOS/CLIP"));
            assertEquals(java.time.Duration.ofMillis(1234), video.getDuration());
            assertEquals(320, video.getWidth());
            assertEquals(180, video.getHeight());
            assertEquals(29.97f, video.getFramesPerSecond());
            assertEquals(VideoSoundtrackType.MusicAndDialog,
                    video.getVideoSoundtrackType());
            try (VideoPlayer player = new VideoPlayer()) {
                assertThrows(IllegalStateException.class, player::GetTexture);
            }
            getContent().Unload();
            assertThrows(IllegalStateException.class,
                    () -> org.openeggbert.cna.internal.NativeMedia.getVideoHandle(video));
            completed = true;
        }
    }

    private static byte[] textureXnb(int width, int height, byte[][] mips) {
        return textureXnb(SurfaceFormat.Color.ordinal(), width, height, mips);
    }

    private static byte[] videoXnb(String reference, int durationMilliseconds,
            int width, int height, float framesPerSecond,
            VideoSoundtrackType soundtrackType) {
        return xnb(new String[] {
                VIDEO_READER, STRING_READER, INT32_READER, SINGLE_READER
        }, output -> {
            output.seven(1);
            output.seven(2);
            output.string(reference);
            output.seven(3);
            output.int32(durationMilliseconds);
            output.seven(3);
            output.int32(width);
            output.seven(3);
            output.int32(height);
            output.seven(4);
            output.single(framesPerSecond);
            output.seven(3);
            output.int32(soundtrackType.ordinal());
        });
    }

    private static byte[] textureXnb(
            int format, int width, int height, byte[][] mips) {
        return xnb(new String[]{TEXTURE_READER}, output -> {
            output.seven(1);
            output.int32(format);
            output.int32(width);
            output.int32(height);
            output.int32(mips.length);
            for (byte[] mip : mips) {
                output.int32(mip.length);
                output.bytes(mip);
            }
        });
    }

    private static byte[] textureXnbWithDeclaredPayload(
            int width, int height, int declaredLength, byte[] payload) {
        return xnb(new String[]{TEXTURE_READER}, output -> {
            output.seven(1);
            output.int32(SurfaceFormat.Color.ordinal());
            output.int32(width);
            output.int32(height);
            output.int32(1);
            output.int32(declaredLength);
            output.bytes(payload);
        });
    }

    private static byte[] spriteFontXnb(boolean defaultCharacter) {
        String[] readers = {
                SPRITE_FONT_READER, TEXTURE_READER,
                RECTANGLE_LIST_READER, RECTANGLE_READER,
                CHAR_LIST_READER, CHAR_READER,
                VECTOR3_LIST_READER, VECTOR3_READER
        };
        return xnb(readers, output -> {
            output.seven(1);
            output.seven(2);
            output.int32(SurfaceFormat.Color.ordinal());
            output.int32(1);
            output.int32(1);
            output.int32(1);
            output.int32(4);
            output.bytes(rgba(255, 255, 255, 255));
            output.seven(3);
            output.int32(1);
            output.rectangle(0, 0, 1, 1);
            output.seven(3);
            output.int32(1);
            output.rectangle(0, 0, 1, 1);
            output.seven(5);
            output.int32(1);
            output.character('A');
            output.int32(3);
            output.single(0.5f);
            output.seven(7);
            output.int32(1);
            output.single(0.0f);
            output.single(1.0f);
            output.single(0.0f);
            output.bool(defaultCharacter);
            if (defaultCharacter) {
                output.character('A');
            }
        });
    }

    private static byte[] modelXnb() {
        String[] readers = {
                MODEL_READER, VERTEX_BUFFER_READER, VERTEX_DECLARATION_READER,
                INDEX_BUFFER_READER, BASIC_EFFECT_READER, STRING_READER
        };
        Writer body = new Writer();
        body.seven(readers.length);
        for (String reader : readers) {
            body.string(reader);
            body.int32(0);
        }
        body.seven(3);
        body.seven(1);

        body.int32(1);
        body.seven(6);
        body.string("Root");
        body.matrixIdentity();
        body.int8(0);
        body.int32(0);

        body.int32(1);
        body.seven(6);
        body.string("Triangle");
        body.int8(1);
        body.single(0.0f);
        body.single(0.0f);
        body.single(0.0f);
        body.single(1.0f);
        body.seven(0);
        body.int32(1);
        body.int32(0);
        body.int32(3);
        body.int32(0);
        body.int32(1);
        body.seven(0);
        body.seven(1);
        body.seven(2);
        body.seven(3);
        body.int8(1);
        body.seven(0);

        body.seven(2);
        body.int32(16);
        body.int32(2);
        body.int32(0);
        body.int32(VertexElementFormat.Vector3.ordinal());
        body.int32(VertexElementUsage.Position.ordinal());
        body.int32(0);
        body.int32(12);
        body.int32(VertexElementFormat.Color.ordinal());
        body.int32(VertexElementUsage.Color.ordinal());
        body.int32(0);
        body.int32(3);
        body.vertex(new Vector3(-0.5f, -0.5f, 0.0f), new Color(255, 0, 0, 255));
        body.vertex(new Vector3(0.0f, 0.5f, 0.0f), new Color(0, 255, 0, 255));
        body.vertex(new Vector3(0.5f, -0.5f, 0.0f), new Color(0, 0, 255, 255));

        body.seven(4);
        body.bool(true);
        body.int32(6);
        body.int16(0);
        body.int16(1);
        body.int16(2);

        body.seven(5);
        body.seven(0);
        body.vector3(1.0f, 1.0f, 1.0f);
        body.vector3(0.0f, 0.0f, 0.0f);
        body.vector3(1.0f, 1.0f, 1.0f);
        body.single(16.0f);
        body.single(1.0f);
        body.bool(true);
        return frame(body.toByteArray());
    }

    private static byte[] xnb(String[] readers, Payload payload) {
        Writer body = new Writer();
        body.seven(readers.length);
        for (String reader : readers) {
            body.string(reader);
            body.int32(0);
        }
        body.seven(0);
        payload.write(body);
        return frame(body.toByteArray());
    }

    private static byte[] frame(byte[] payloadBytes) {
        byte[] framed = new byte[10 + payloadBytes.length];
        framed[0] = 'X';
        framed[1] = 'N';
        framed[2] = 'B';
        framed[3] = 'w';
        framed[4] = 5;
        int size = framed.length;
        framed[6] = (byte)size;
        framed[7] = (byte)(size >>> 8);
        framed[8] = (byte)(size >>> 16);
        framed[9] = (byte)(size >>> 24);
        System.arraycopy(payloadBytes, 0, framed, 10, payloadBytes.length);
        return framed;
    }

    private static byte[] compressXnb(byte[] uncompressed) {
        byte[] payload = java.util.Arrays.copyOfRange(uncompressed, 10, uncompressed.length);
        int headerBits = 3 << 28 | payload.length << 4;
        byte[] block = new byte[16 + payload.length];
        block[0] = (byte) (headerBits >>> 16);
        block[1] = (byte) (headerBits >>> 24);
        block[2] = (byte) headerBits;
        block[3] = (byte) (headerBits >>> 8);
        block[4] = 1;
        block[8] = 1;
        block[12] = 1;
        System.arraycopy(payload, 0, block, 16, payload.length);

        byte[] compressed = new byte[19 + block.length];
        compressed[0] = 'X';
        compressed[1] = 'N';
        compressed[2] = 'B';
        compressed[3] = 'w';
        compressed[4] = 5;
        compressed[5] = (byte) 0x80;
        int totalSize = compressed.length;
        compressed[6] = (byte) totalSize;
        compressed[7] = (byte) (totalSize >>> 8);
        compressed[8] = (byte) (totalSize >>> 16);
        compressed[9] = (byte) (totalSize >>> 24);
        compressed[10] = (byte) payload.length;
        compressed[11] = (byte) (payload.length >>> 8);
        compressed[12] = (byte) (payload.length >>> 16);
        compressed[13] = (byte) (payload.length >>> 24);
        compressed[14] = (byte) 0xff;
        compressed[15] = (byte) (payload.length >>> 8);
        compressed[16] = (byte) payload.length;
        compressed[17] = (byte) (block.length >>> 8);
        compressed[18] = (byte) block.length;
        System.arraycopy(block, 0, compressed, 19, block.length);
        return compressed;
    }

    private static byte[] rgba(int... values) {
        byte[] result = new byte[values.length];
        for (int index = 0; index < values.length; index++) {
            result[index] = (byte)values[index];
        }
        return result;
    }

    private static void write(Path root, String name, byte[] bytes) throws IOException {
        Files.write(root.resolve(name + ".xnb"), bytes);
    }

    private static String messages(Throwable failure) {
        StringBuilder result = new StringBuilder();
        for (Throwable current = failure; current != null; current = current.getCause()) {
            if (!result.isEmpty()) result.append(" -> ");
            result.append(current.getMessage());
        }
        return result.toString();
    }

    @FunctionalInterface
    private interface Payload {
        void write(Writer output);
    }

    private static final class Writer {
        private final ByteArrayOutputStream output = new ByteArrayOutputStream();

        private void bool(boolean value) {
            output.write(value ? 1 : 0);
        }

        private void bytes(byte[] values) {
            output.writeBytes(values);
        }

        private void character(char value) {
            output.write(value);
            output.write(value >>> 8);
        }

        private void int32(int value) {
            output.write(value);
            output.write(value >>> 8);
            output.write(value >>> 16);
            output.write(value >>> 24);
        }

        private void int16(int value) {
            output.write(value);
            output.write(value >>> 8);
        }

        private void int8(int value) {
            output.write(value);
        }

        private void matrixIdentity() {
            for (int row = 0; row < 4; row++) {
                for (int column = 0; column < 4; column++) {
                    single(row == column ? 1.0f : 0.0f);
                }
            }
        }

        private void rectangle(int x, int y, int width, int height) {
            int32(x);
            int32(y);
            int32(width);
            int32(height);
        }

        private void seven(int value) {
            int remaining = value;
            do {
                int next = remaining & 0x7f;
                remaining >>>= 7;
                if (remaining != 0) next |= 0x80;
                output.write(next);
            } while (remaining != 0);
        }

        private void single(float value) {
            int32(Float.floatToIntBits(value));
        }

        private void string(String value) {
            byte[] encoded = value.getBytes(StandardCharsets.UTF_8);
            seven(encoded.length);
            bytes(encoded);
        }

        private void vector3(float x, float y, float z) {
            single(x);
            single(y);
            single(z);
        }

        private void vertex(Vector3 position, Color color) {
            vector3(position.X, position.Y, position.Z);
            output.write(color.getR());
            output.write(color.getG());
            output.write(color.getB());
            output.write(color.getA());
        }

        private byte[] toByteArray() {
            return output.toByteArray();
        }
    }
}
