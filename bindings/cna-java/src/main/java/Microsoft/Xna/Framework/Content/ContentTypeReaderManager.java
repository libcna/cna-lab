package Microsoft.Xna.Framework.Content;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Graphics.SurfaceFormat;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.BasicEffect;
import Microsoft.Xna.Framework.Graphics.BufferUsage;
import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.IndexBuffer;
import Microsoft.Xna.Framework.Graphics.IndexElementSize;
import Microsoft.Xna.Framework.Graphics.Model;
import Microsoft.Xna.Framework.Graphics.VertexBuffer;
import Microsoft.Xna.Framework.Graphics.VertexDeclaration;
import Microsoft.Xna.Framework.Graphics.VertexElement;
import Microsoft.Xna.Framework.Graphics.VertexElementFormat;
import Microsoft.Xna.Framework.Graphics.VertexElementUsage;
import Microsoft.Xna.Framework.Media.Video;
import org.openeggbert.cna.content.ContentTypeReaderRegistry;
import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.ArrayList;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/** Per-asset reader table and deterministic custom-reader activator. */
public final class ContentTypeReaderManager {

    private final Map<Class<?>, ContentTypeReader> readersByTargetType = new LinkedHashMap<>();

    ContentTypeReaderManager() {
    }

    public ContentTypeReader GetTypeReader(Class<?> targetType) {
        return readersByTargetType.get(Objects.requireNonNull(targetType, "targetType"));
    }

    ContentTypeReader[] loadAssetReaders(ContentReader input) {
        int count = input.read7BitEncodedInt32();
        if (count < 0 || count > 4096) {
            throw input.failure("invalid type reader count " + count);
        }
        ContentTypeReader[] readers = new ContentTypeReader[count];
        int[] versions = new int[count];
        for (int index = 0; index < count; index++) {
            String serializedName = input.ReadString();
            readers[index] = createReader(serializedName, input.getAssetName());
            versions[index] = input.ReadInt32();
            if (versions[index] != readers[index].getTypeVersion()) {
                throw input.failure("reader version mismatch for '" + serializedName
                        + "': asset " + versions[index] + ", runtime "
                        + readers[index].getTypeVersion());
            }
            readersByTargetType.putIfAbsent(readers[index].getTargetType(), readers[index]);
        }
        input.setTypeReaderVersions(versions);
        for (ContentTypeReader reader : readers) {
            try {
                reader.initializeReader(this);
            } catch (RuntimeException exception) {
                throw new ContentLoadException(
                        "Failed to initialize content type reader '"
                                + reader.getClass().getName() + "'", exception);
            }
        }
        return readers;
    }

    private static ContentTypeReader createReader(String serializedName, String assetName) {
        String name = serializedName == null ? "" : serializedName.trim();
        if (name.isEmpty()) {
            throw new ContentLoadException(
                    "Content asset '" + assetName + "' declares an empty content reader name");
        }
        ContentTypeReader builtIn = builtIn(
                ContentTypeReaderRegistry.stripAssemblyQualification(name));
        if (builtIn != null) {
            return builtIn;
        }
        try {
            ContentTypeReader custom = ContentTypeReaderRegistry.create(name);
            if (custom != null) {
                return custom;
            }
        } catch (RuntimeException exception) {
            throw new ContentLoadException(
                    "Failed to activate content type reader '" + name
                            + "' while loading '" + assetName + "'", exception);
        }
        throw new ContentLoadException(
                "Unknown content type reader '" + name + "' while loading '" + assetName + "'");
    }

    private static ContentTypeReader builtIn(String name) {
        if (name.startsWith("Microsoft.Xna.Framework.Content.ListReader`1")) {
            if (name.contains("Microsoft.Xna.Framework.Rectangle")) {
                return new ListReader(Rectangle.class);
            }
            if (name.contains("Microsoft.Xna.Framework.Vector3")) {
                return new ListReader(Vector3.class);
            }
            if (name.contains("System.Char")) {
                return new ListReader(Character.class);
            }
        }
        return switch (name) {
            case "Microsoft.Xna.Framework.Content.BooleanReader" -> new BooleanReader();
            case "Microsoft.Xna.Framework.Content.ByteReader" -> new ByteReader();
            case "Microsoft.Xna.Framework.Content.CharReader" -> new CharReader();
            case "Microsoft.Xna.Framework.Content.DoubleReader" -> new DoubleReader();
            case "Microsoft.Xna.Framework.Content.Int16Reader" -> new Int16Reader();
            case "Microsoft.Xna.Framework.Content.Int32Reader" -> new Int32Reader();
            case "Microsoft.Xna.Framework.Content.Int64Reader" -> new Int64Reader();
            case "Microsoft.Xna.Framework.Content.SByteReader" -> new SByteReader();
            case "Microsoft.Xna.Framework.Content.SingleReader" -> new SingleReader();
            case "Microsoft.Xna.Framework.Content.StringReader" -> new StringReader();
            case "Microsoft.Xna.Framework.Content.UInt16Reader" -> new UInt16Reader();
            case "Microsoft.Xna.Framework.Content.UInt32Reader" -> new UInt32Reader();
            case "Microsoft.Xna.Framework.Content.UInt64Reader" -> new UInt64Reader();
            case "Microsoft.Xna.Framework.Content.Vector2Reader" -> new Vector2Reader();
            case "Microsoft.Xna.Framework.Content.Vector3Reader" -> new Vector3Reader();
            case "Microsoft.Xna.Framework.Content.Vector4Reader" -> new Vector4Reader();
            case "Microsoft.Xna.Framework.Content.MatrixReader" -> new MatrixReader();
            case "Microsoft.Xna.Framework.Content.QuaternionReader" -> new QuaternionReader();
            case "Microsoft.Xna.Framework.Content.ColorReader" -> new ColorReader();
            case "Microsoft.Xna.Framework.Content.RectangleReader" -> new RectangleReader();
            case "Microsoft.Xna.Framework.Content.Texture2DReader" -> new Texture2DReader();
            case "Microsoft.Xna.Framework.Content.SpriteFontReader" -> new SpriteFontReader();
            case "Microsoft.Xna.Framework.Content.VertexDeclarationReader" ->
                    new VertexDeclarationReader();
            case "Microsoft.Xna.Framework.Content.VertexBufferReader" -> new VertexBufferReader();
            case "Microsoft.Xna.Framework.Content.IndexBufferReader" -> new IndexBufferReader();
            case "Microsoft.Xna.Framework.Content.EffectReader" -> new EffectReader();
            case "Microsoft.Xna.Framework.Content.BasicEffectReader" -> new BasicEffectReader();
            case "Microsoft.Xna.Framework.Content.ModelReader" -> new ModelReader();
            case "Microsoft.Xna.Framework.Content.VideoReader" -> new VideoReader();
            default -> null;
        };
    }

    private static final class BooleanReader extends ContentTypeReaderOfT<Boolean> {
        @Override protected Boolean ReadTyped(ContentReader input, Boolean existing) { return input.ReadBoolean(); }
    }
    private static final class ByteReader extends ContentTypeReaderOfT<Integer> {
        @Override protected Integer ReadTyped(ContentReader input, Integer existing) { return input.ReadByte(); }
    }
    private static final class CharReader extends ContentTypeReaderOfT<Character> {
        @Override protected Character ReadTyped(ContentReader input, Character existing) { return input.ReadChar(); }
    }
    private static final class DoubleReader extends ContentTypeReaderOfT<Double> {
        @Override protected Double ReadTyped(ContentReader input, Double existing) { return input.ReadDouble(); }
    }
    private static final class Int16Reader extends ContentTypeReaderOfT<Short> {
        @Override protected Short ReadTyped(ContentReader input, Short existing) { return input.ReadInt16(); }
    }
    private static final class Int32Reader extends ContentTypeReaderOfT<Integer> {
        @Override protected Integer ReadTyped(ContentReader input, Integer existing) { return input.ReadInt32(); }
    }
    private static final class Int64Reader extends ContentTypeReaderOfT<Long> {
        @Override protected Long ReadTyped(ContentReader input, Long existing) { return input.ReadInt64(); }
    }
    private static final class SByteReader extends ContentTypeReaderOfT<Byte> {
        @Override protected Byte ReadTyped(ContentReader input, Byte existing) { return input.ReadSByte(); }
    }
    private static final class SingleReader extends ContentTypeReaderOfT<Float> {
        @Override protected Float ReadTyped(ContentReader input, Float existing) { return input.ReadSingle(); }
    }
    private static final class StringReader extends ContentTypeReaderOfT<String> {
        @Override protected String ReadTyped(ContentReader input, String existing) { return input.ReadString(); }
    }
    private static final class UInt16Reader extends ContentTypeReaderOfT<Integer> {
        @Override protected Integer ReadTyped(ContentReader input, Integer existing) { return input.ReadUInt16(); }
    }
    private static final class UInt32Reader extends ContentTypeReaderOfT<Long> {
        @Override protected Long ReadTyped(ContentReader input, Long existing) { return input.ReadUInt32(); }
    }
    private static final class UInt64Reader extends ContentTypeReaderOfT<Long> {
        @Override protected Long ReadTyped(ContentReader input, Long existing) { return input.ReadUInt64(); }
    }
    private static final class Vector2Reader extends ContentTypeReaderOfT<Vector2> {
        @Override protected Vector2 ReadTyped(ContentReader input, Vector2 existing) { return input.ReadVector2(); }
    }
    private static final class Vector3Reader extends ContentTypeReaderOfT<Vector3> {
        @Override protected Vector3 ReadTyped(ContentReader input, Vector3 existing) { return input.ReadVector3(); }
    }
    private static final class Vector4Reader extends ContentTypeReaderOfT<Vector4> {
        @Override protected Vector4 ReadTyped(ContentReader input, Vector4 existing) { return input.ReadVector4(); }
    }
    private static final class MatrixReader extends ContentTypeReaderOfT<Matrix> {
        @Override protected Matrix ReadTyped(ContentReader input, Matrix existing) { return input.ReadMatrix(); }
    }
    private static final class QuaternionReader extends ContentTypeReaderOfT<Quaternion> {
        @Override protected Quaternion ReadTyped(ContentReader input, Quaternion existing) { return input.ReadQuaternion(); }
    }
    private static final class ColorReader extends ContentTypeReaderOfT<Color> {
        @Override protected Color ReadTyped(ContentReader input, Color existing) { return input.ReadColor(); }
    }

    private static final class RectangleReader extends ContentTypeReaderOfT<Rectangle> {
        @Override
        protected Rectangle ReadTyped(ContentReader input, Rectangle existing) {
            return new Rectangle(
                    input.ReadInt32(), input.ReadInt32(), input.ReadInt32(), input.ReadInt32());
        }
    }

    private static final class ListReader extends ContentTypeReaderOfT<List<Object>> {
        private final Class<?> elementType;
        private ContentTypeReader elementReader;

        private ListReader(Class<?> elementType) {
            this.elementType = elementType;
        }

        @Override
        protected void Initialize(ContentTypeReaderManager manager) {
            elementReader = manager.GetTypeReader(elementType);
            if (elementReader == null) {
                throw new IllegalStateException(
                        "The XNB reader table has no element reader for " + elementType.getName());
            }
        }

        @Override
        @SuppressWarnings({"rawtypes", "unchecked"})
        protected List<Object> ReadTyped(ContentReader input, List<Object> existing) {
            int count = input.ReadInt32();
            if (count < 0 || count > 1_000_000) {
                throw input.failure("invalid list element count " + count);
            }
            List<Object> values = new ArrayList<>(count);
            for (int index = 0; index < count; index++) {
                values.add(input.ReadObject((Class)elementType, elementReader, null));
            }
            return values;
        }
    }

    private static final class Texture2DReader extends ContentTypeReaderOfT<Texture2D> {
        @Override
        protected Texture2D ReadTyped(ContentReader input, Texture2D existing) {
            int formatValue = input.ReadInt32();
            int width = input.ReadInt32();
            int height = input.ReadInt32();
            int mipCount = input.ReadInt32();
            SurfaceFormat[] formats = SurfaceFormat.values();
            if (formatValue < 0 || formatValue >= formats.length) {
                throw input.failure("invalid Texture2D SurfaceFormat " + formatValue);
            }
            if (width <= 0 || height <= 0) {
                throw input.failure("Texture2D dimensions must be positive");
            }
            int completeMipCount = 32 - Integer.numberOfLeadingZeros(Math.max(width, height));
            if (mipCount != 1 && mipCount != completeMipCount) {
                throw input.failure("invalid Texture2D mip count " + mipCount
                        + " for " + width + "x" + height);
            }
            SurfaceFormat format = formats[formatValue];
            if (format != SurfaceFormat.Color) {
                throw input.failure("Texture2D SurfaceFormat." + format
                        + " requires CNA surface-format support; payload fidelity is not degraded");
            }

            Texture2D texture = new Texture2D(
                    input.getContentManager().graphicsDeviceForContentReader(),
                    width, height, mipCount > 1, format);
            try {
                for (int level = 0; level < mipCount; level++) {
                    int payloadLength = input.ReadInt32();
                    long expected = (long)Math.max(1, width >> level)
                            * Math.max(1, height >> level) * 4L;
                    if (payloadLength < 0 || payloadLength != expected) {
                        throw input.failure("Texture2D mip " + level + " payload length "
                                + payloadLength + " does not match " + expected);
                    }
                    byte[] payload = input.readByteBuffer(payloadLength);
                    NativeBindings.setTexture2DRawBytes(texture, level, payload);
                }
                return texture;
            } catch (RuntimeException failure) {
                try {
                    texture.close();
                } catch (RuntimeException closeFailure) {
                    failure.addSuppressed(closeFailure);
                }
                throw failure;
            }
        }
    }

    private static final class SpriteFontReader extends ContentTypeReaderOfT<SpriteFont> {
        @Override
        @SuppressWarnings("unchecked")
        protected SpriteFont ReadTyped(ContentReader input, SpriteFont existing) {
            Texture2D texture = input.ReadObject(Texture2D.class);
            List<Rectangle> glyphs = (List<Rectangle>)(List<?>)input.ReadObject(List.class);
            List<Rectangle> cropping = (List<Rectangle>)(List<?>)input.ReadObject(List.class);
            List<Character> characters = (List<Character>)(List<?>)input.ReadObject(List.class);
            int lineSpacing = input.ReadInt32();
            float spacing = input.ReadSingle();
            List<Vector3> kerning = (List<Vector3>)(List<?>)input.ReadObject(List.class);
            Character defaultCharacter = input.ReadBoolean() ? input.ReadChar() : null;
            Objects.requireNonNull(texture, "SpriteFont texture");
            Objects.requireNonNull(glyphs, "SpriteFont glyphs");
            Objects.requireNonNull(cropping, "SpriteFont cropping");
            Objects.requireNonNull(characters, "SpriteFont characters");
            Objects.requireNonNull(kerning, "SpriteFont kerning");
            int count = characters.size();
            if (count == 0 || glyphs.size() != count || cropping.size() != count
                    || kerning.size() != count) {
                throw input.failure("SpriteFont glyph, crop, character and kerning counts differ");
            }
            if (!Float.isFinite(spacing)) {
                throw input.failure("SpriteFont spacing is not finite");
            }
            if (defaultCharacter != null && !characters.contains(defaultCharacter)) {
                throw input.failure("SpriteFont default character is absent from its character map");
            }
            SpriteFont font = NativeBindings.createSpriteFont(
                    texture, glyphs, cropping, characters,
                    lineSpacing, spacing, kerning, defaultCharacter);
            input.recordManagedNativeObject(font);
            return font;
        }
    }

    private static final class VideoReader extends ContentTypeReaderOfT<Video> {
        @Override
        protected Video ReadTyped(ContentReader input, Video existing) {
            String fileName = input.ReadObject(String.class);
            int durationMilliseconds = input.ReadObject(Integer.class);
            int width = input.ReadObject(Integer.class);
            int height = input.ReadObject(Integer.class);
            float framesPerSecond = input.ReadObject(Float.class);
            int soundtrackType = input.ReadObject(Integer.class);
            if (fileName == null) throw input.failure("Video file reference is null");
            if (soundtrackType < 0 || soundtrackType > 2) {
                throw input.failure("Video soundtrack identity is undefined");
            }
            String parent = "";
            int slash = Math.max(input.getAssetName().lastIndexOf('/'),
                    input.getAssetName().lastIndexOf('\\'));
            if (slash >= 0) parent = input.getAssetName().substring(0, slash);
            java.nio.file.Path relative = parent.isEmpty()
                    ? java.nio.file.Path.of(fileName)
                    : java.nio.file.Path.of(parent).resolve(fileName);
            String resolved = java.nio.file.Path.of(
                    input.getContentManager().getRootDirectory()).resolve(relative)
                    .normalize().toString();
            Video video = FacadeFactory.createVideo(
                    input.getContentManager().graphicsDeviceForContentReader(), resolved,
                    durationMilliseconds, width, height, framesPerSecond, soundtrackType);
            input.recordManagedNativeObject(video);
            return video;
        }
    }

    private static final class VertexDeclarationReader
            extends ContentTypeReaderOfT<VertexDeclaration> {
        @Override
        protected VertexDeclaration ReadTyped(ContentReader input, VertexDeclaration existing) {
            int stride = input.ReadInt32();
            int count = input.ReadInt32();
            if (count <= 0 || count > 256) {
                throw input.failure("invalid vertex-element count " + count);
            }
            VertexElementFormat[] formats = VertexElementFormat.values();
            VertexElementUsage[] usages = VertexElementUsage.values();
            VertexElement[] elements = new VertexElement[count];
            for (int index = 0; index < count; index++) {
                int offset = input.ReadInt32();
                int format = input.ReadInt32();
                int usage = input.ReadInt32();
                int usageIndex = input.ReadInt32();
                if (format < 0 || format >= formats.length
                        || usage < 0 || usage >= usages.length) {
                    throw input.failure("invalid vertex declaration enum value");
                }
                elements[index] = new VertexElement(
                        offset, formats[format], usages[usage], usageIndex);
            }
            return new VertexDeclaration(stride, elements);
        }
    }

    private static final class VertexBufferReader extends ContentTypeReaderOfT<VertexBuffer> {
        @Override
        protected VertexBuffer ReadTyped(ContentReader input, VertexBuffer existing) {
            VertexDeclaration declaration = input.ReadRawObject(VertexDeclaration.class);
            int vertexCount = input.ReadInt32();
            if (vertexCount <= 0) {
                throw input.failure("vertex-buffer count must be positive");
            }
            int byteCount;
            try {
                byteCount = Math.multiplyExact(vertexCount, declaration.getVertexStride());
            } catch (ArithmeticException exception) {
                throw input.failure("vertex-buffer byte count overflows the Java range");
            }
            byte[] payload = input.readByteBuffer(byteCount);
            VertexBuffer buffer = new VertexBuffer(
                    input.getContentManager().graphicsDeviceForContentReader(),
                    declaration, vertexCount, BufferUsage.None);
            try {
                NativeBindings.setVertexBufferRawBytes(
                        buffer, payload, vertexCount, declaration.getVertexStride());
                return buffer;
            } catch (RuntimeException failure) {
                try {
                    buffer.close();
                } catch (RuntimeException closeFailure) {
                    failure.addSuppressed(closeFailure);
                }
                throw failure;
            }
        }
    }

    private static final class IndexBufferReader extends ContentTypeReaderOfT<IndexBuffer> {
        @Override
        protected IndexBuffer ReadTyped(ContentReader input, IndexBuffer existing) {
            boolean sixteenBits = input.ReadBoolean();
            int byteCount = input.ReadInt32();
            int elementBytes = sixteenBits ? 2 : 4;
            if (byteCount <= 0 || byteCount % elementBytes != 0) {
                throw input.failure("invalid index-buffer byte count " + byteCount);
            }
            byte[] payload = input.readByteBuffer(byteCount);
            int count = byteCount / elementBytes;
            Integer[] values = new Integer[count];
            ByteBuffer bytes = ByteBuffer.wrap(payload).order(ByteOrder.LITTLE_ENDIAN);
            for (int index = 0; index < count; index++) {
                values[index] = sixteenBits
                        ? Short.toUnsignedInt(bytes.getShort()) : bytes.getInt();
            }
            IndexBuffer buffer = new IndexBuffer(
                    input.getContentManager().graphicsDeviceForContentReader(),
                    sixteenBits ? IndexElementSize.SixteenBits : IndexElementSize.ThirtyTwoBits,
                    count, BufferUsage.None);
            try {
                buffer.SetData(values);
                return buffer;
            } catch (RuntimeException failure) {
                try {
                    buffer.close();
                } catch (RuntimeException closeFailure) {
                    failure.addSuppressed(closeFailure);
                }
                throw failure;
            }
        }
    }

    private static final class EffectReader extends ContentTypeReaderOfT<Effect> {
        @Override
        protected Effect ReadTyped(ContentReader input, Effect existing) {
            int count = input.ReadInt32();
            if (count <= 0) {
                throw input.failure("Effect bytecode must not be empty");
            }
            byte[] payload = input.readByteBuffer(count);
            int[] effectCode = new int[count];
            for (int index = 0; index < count; index++) {
                effectCode[index] = Byte.toUnsignedInt(payload[index]);
            }
            return new Effect(
                    input.getContentManager().graphicsDeviceForContentReader(), effectCode);
        }
    }

    private static final class BasicEffectReader extends ContentTypeReaderOfT<BasicEffect> {
        @Override
        protected BasicEffect ReadTyped(ContentReader input, BasicEffect existing) {
            BasicEffect effect = new BasicEffect(
                    input.getContentManager().graphicsDeviceForContentReader());
            try {
                Texture2D texture = input.ReadExternalReference(Texture2D.class);
                if (texture != null) {
                    effect.setTexture(texture);
                    effect.setTextureEnabled(true);
                }
                effect.setDiffuseColor(input.ReadVector3());
                effect.setEmissiveColor(input.ReadVector3());
                effect.setSpecularColor(input.ReadVector3());
                effect.setSpecularPower(input.ReadSingle());
                effect.setAlpha(input.ReadSingle());
                effect.setVertexColorEnabled(input.ReadBoolean());
                return effect;
            } catch (RuntimeException failure) {
                try {
                    effect.close();
                } catch (RuntimeException closeFailure) {
                    failure.addSuppressed(closeFailure);
                }
                throw failure;
            }
        }
    }

    private static final class ModelReader extends ContentTypeReaderOfT<Model> {
        @Override
        protected Model ReadTyped(ContentReader input, Model existing) {
            return FacadeFactory.readModel(input);
        }
    }
}
