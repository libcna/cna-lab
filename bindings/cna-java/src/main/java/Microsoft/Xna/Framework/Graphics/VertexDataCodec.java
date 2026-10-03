package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;

import java.lang.reflect.Array;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;

/** Snapshots the four XNA built-in vertex layouts exposed by CNA's user-draw ABI. */
final class VertexDataCodec {

    private final Class<?> type;
    private final int nativeType;
    private final int stride;
    private final int userSource;

    private VertexDataCodec(Class<?> type, int nativeType, int stride, int userSource) {
        this.type = type;
        this.nativeType = nativeType;
        this.stride = stride;
        this.userSource = userSource;
    }

    static VertexDataCodec select(Object[] data) {
        Objects.requireNonNull(data, "data");
        return forType(data.getClass().getComponentType());
    }

    static VertexDataCodec forType(Class<?> type) {
        if (type == VertexPositionColor.class) {
            return new VertexDataCodec(type, 0, 16, 1);
        }
        if (type == VertexPositionColorTexture.class) {
            return new VertexDataCodec(type, 1, 24, 2);
        }
        if (type == VertexPositionTexture.class) {
            return new VertexDataCodec(type, 6, 20, 3);
        }
        if (type == VertexPositionNormalTexture.class) {
            return new VertexDataCodec(type, 4, 32, 4);
        }
        throw new UnsupportedOperationException(
                "CNA-Java can marshal only the four built-in XNA vertex value types");
    }

    int stride() {
        return stride;
    }

    int nativeType() {
        return nativeType;
    }

    int userSource() {
        return userSource;
    }

    byte[] encode(Object[] data, int startIndex, int elementCount) {
        Objects.requireNonNull(data, "data");
        if (data.getClass().getComponentType() != type) {
            throw new IllegalArgumentException("Vertex array component type changed during transfer");
        }
        byte[] result = new byte[Math.multiplyExact(elementCount, stride)];
        ByteBuffer output = ByteBuffer.wrap(result).order(ByteOrder.nativeOrder());
        for (int index = 0; index < elementCount; index++) {
            Object value = Objects.requireNonNull(
                    data[startIndex + index], "data[" + (startIndex + index) + "]");
            encodeOne(output, value);
        }
        return result;
    }

    @SuppressWarnings("unchecked")
    <T> T[] decode(byte[] payload, int elementCount) {
        Objects.requireNonNull(payload, "payload");
        if (payload.length != Math.multiplyExact(elementCount, stride)) {
            throw new IllegalArgumentException("Vertex payload has the wrong byte length");
        }
        T[] result = (T[])Array.newInstance(type, elementCount);
        ByteBuffer input = ByteBuffer.wrap(payload).order(ByteOrder.nativeOrder());
        for (int index = 0; index < elementCount; index++) {
            result[index] = (T)decodeOne(input);
        }
        return result;
    }

    private void encodeOne(ByteBuffer output, Object value) {
        if (value instanceof VertexPositionColor vertex) {
            putVector3(output, vertex.Position, "Position");
            putColor(output, vertex.Color);
        } else if (value instanceof VertexPositionColorTexture vertex) {
            putVector3(output, vertex.Position, "Position");
            putColor(output, vertex.Color);
            putVector2(output, vertex.TextureCoordinate, "TextureCoordinate");
        } else if (value instanceof VertexPositionTexture vertex) {
            putVector3(output, vertex.Position, "Position");
            putVector2(output, vertex.TextureCoordinate, "TextureCoordinate");
        } else if (value instanceof VertexPositionNormalTexture vertex) {
            putVector3(output, vertex.Position, "Position");
            putVector3(output, vertex.Normal, "Normal");
            putVector2(output, vertex.TextureCoordinate, "TextureCoordinate");
        } else {
            throw new IllegalArgumentException("Vertex value does not match its array component type");
        }
    }

    private Object decodeOne(ByteBuffer input) {
        if (type == VertexPositionColor.class) {
            return new VertexPositionColor(getVector3(input), getColor(input));
        }
        if (type == VertexPositionColorTexture.class) {
            return new VertexPositionColorTexture(
                    getVector3(input), getColor(input), getVector2(input));
        }
        if (type == VertexPositionTexture.class) {
            return new VertexPositionTexture(getVector3(input), getVector2(input));
        }
        return new VertexPositionNormalTexture(
                getVector3(input), getVector3(input), getVector2(input));
    }

    private static void putVector2(ByteBuffer output, Vector2 value, String name) {
        Vector2 snapshot = new Vector2(Objects.requireNonNull(value, name));
        output.putFloat(snapshot.X).putFloat(snapshot.Y);
    }

    private static void putVector3(ByteBuffer output, Vector3 value, String name) {
        Vector3 snapshot = new Vector3(Objects.requireNonNull(value, name));
        output.putFloat(snapshot.X).putFloat(snapshot.Y).putFloat(snapshot.Z);
    }

    private static void putColor(ByteBuffer output, Color value) {
        Color snapshot = new Color(Objects.requireNonNull(value, "Color"));
        output.put((byte)snapshot.getR());
        output.put((byte)snapshot.getG());
        output.put((byte)snapshot.getB());
        output.put((byte)snapshot.getA());
    }

    private static Vector2 getVector2(ByteBuffer input) {
        return new Vector2(input.getFloat(), input.getFloat());
    }

    private static Vector3 getVector3(ByteBuffer input) {
        return new Vector3(input.getFloat(), input.getFloat(), input.getFloat());
    }

    private static Color getColor(ByteBuffer input) {
        return new Color(
                Byte.toUnsignedInt(input.get()),
                Byte.toUnsignedInt(input.get()),
                Byte.toUnsignedInt(input.get()),
                Byte.toUnsignedInt(input.get()));
    }
}
