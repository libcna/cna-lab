package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector4;
import Microsoft.Xna.Framework.Graphics.PackedVector.Alpha8;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgr565;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgra4444;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgra5551;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfSingle;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfVector2;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfVector4;
import Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedByte2;
import Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedByte4;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rg32;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rgba1010102;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rgba64;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;

/** Internal, explicit mapping from Java array components to CNA texture-transfer representations. */
enum TextureDataCodec {
    COLOR(Color.class, 0, 4, SurfaceFormat.Color) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            Color color = (Color)requireValue(value, offset);
            buffer.put(offset, (byte)color.getR());
            buffer.put(offset + 1, (byte)color.getG());
            buffer.put(offset + 2, (byte)color.getB());
            buffer.put(offset + 3, (byte)color.getA());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            return new Color(
                    Byte.toUnsignedInt(buffer.get(offset)),
                    Byte.toUnsignedInt(buffer.get(offset + 1)),
                    Byte.toUnsignedInt(buffer.get(offset + 2)),
                    Byte.toUnsignedInt(buffer.get(offset + 3)));
        }
    },
    BGR565(Bgr565.class, 1, 2, SurfaceFormat.Bgr565) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putShort(offset, (short)((Bgr565)requireValue(value, offset))
                    .getPackedValue().intValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            Bgr565 value = new Bgr565();
            value.setPackedValue(Short.toUnsignedInt(buffer.getShort(offset)));
            return value;
        }
    },
    BYTE(Integer.class, 4, 1,
            SurfaceFormat.Dxt1, SurfaceFormat.Dxt3, SurfaceFormat.Dxt5) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            int unsigned = (Integer)requireValue(value, offset);
            if ((unsigned & ~0xff) != 0) {
                throw new IllegalArgumentException("Texture byte elements must be between 0 and 255");
            }
            buffer.put(offset, (byte)unsigned);
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            return Byte.toUnsignedInt(buffer.get(offset));
        }
    },
    BGRA5551(Bgra5551.class, 2, 2, SurfaceFormat.Bgra5551) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putShort(offset, (short)((Bgra5551)requireValue(value, offset))
                    .getPackedValue().intValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            Bgra5551 value = new Bgra5551();
            value.setPackedValue(Short.toUnsignedInt(buffer.getShort(offset)));
            return value;
        }
    },
    BGRA4444(Bgra4444.class, 3, 2, SurfaceFormat.Bgra4444) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putShort(offset, (short)((Bgra4444)requireValue(value, offset))
                    .getPackedValue().intValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            Bgra4444 value = new Bgra4444();
            value.setPackedValue(Short.toUnsignedInt(buffer.getShort(offset)));
            return value;
        }
    },
    NORMALIZED_BYTE2(NormalizedByte2.class, 5, 2, SurfaceFormat.NormalizedByte2) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putShort(offset, (short)((NormalizedByte2)requireValue(value, offset))
                    .getPackedValue().intValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            NormalizedByte2 value = new NormalizedByte2();
            value.setPackedValue(Short.toUnsignedInt(buffer.getShort(offset)));
            return value;
        }
    },
    NORMALIZED_BYTE4(NormalizedByte4.class, 6, 4, SurfaceFormat.NormalizedByte4) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putInt(offset, (int)((NormalizedByte4)requireValue(value, offset))
                    .getPackedValue().longValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            NormalizedByte4 value = new NormalizedByte4();
            value.setPackedValue(Integer.toUnsignedLong(buffer.getInt(offset)));
            return value;
        }
    },
    RGBA1010102(Rgba1010102.class, 7, 4, SurfaceFormat.Rgba1010102) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putInt(offset, (int)((Rgba1010102)requireValue(value, offset))
                    .getPackedValue().longValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            Rgba1010102 value = new Rgba1010102();
            value.setPackedValue(Integer.toUnsignedLong(buffer.getInt(offset)));
            return value;
        }
    },
    RG32(Rg32.class, 8, 4, SurfaceFormat.Rg32) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putInt(offset, (int)((Rg32)requireValue(value, offset))
                    .getPackedValue().longValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            Rg32 value = new Rg32();
            value.setPackedValue(Integer.toUnsignedLong(buffer.getInt(offset)));
            return value;
        }
    },
    RGBA64(Rgba64.class, 9, 8, SurfaceFormat.Rgba64) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putLong(offset, ((Rgba64)requireValue(value, offset)).getPackedValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            Rgba64 value = new Rgba64();
            value.setPackedValue(buffer.getLong(offset));
            return value;
        }
    },
    ALPHA8(Alpha8.class, 10, 1, SurfaceFormat.Alpha8) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.put(offset, (byte)((Alpha8)requireValue(value, offset))
                    .getPackedValue().intValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            Alpha8 value = new Alpha8();
            value.setPackedValue(Byte.toUnsignedInt(buffer.get(offset)));
            return value;
        }
    },
    SINGLE(Float.class, 11, 4, SurfaceFormat.Single) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putFloat(offset, (Float)requireValue(value, offset));
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            return buffer.getFloat(offset);
        }
    },
    VECTOR2(Vector2.class, 12, 8, SurfaceFormat.Vector2) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            Vector2 vector = (Vector2)requireValue(value, offset);
            buffer.putFloat(offset, vector.X);
            buffer.putFloat(offset + 4, vector.Y);
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            return new Vector2(buffer.getFloat(offset), buffer.getFloat(offset + 4));
        }
    },
    VECTOR4(Vector4.class, 13, 16, SurfaceFormat.Vector4) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            Vector4 vector = (Vector4)requireValue(value, offset);
            buffer.putFloat(offset, vector.X);
            buffer.putFloat(offset + 4, vector.Y);
            buffer.putFloat(offset + 8, vector.Z);
            buffer.putFloat(offset + 12, vector.W);
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            return new Vector4(
                    buffer.getFloat(offset), buffer.getFloat(offset + 4),
                    buffer.getFloat(offset + 8), buffer.getFloat(offset + 12));
        }
    },
    HALF_SINGLE(HalfSingle.class, 14, 2, SurfaceFormat.HalfSingle) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putShort(offset, (short)((HalfSingle)requireValue(value, offset))
                    .getPackedValue().intValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            HalfSingle value = new HalfSingle();
            value.setPackedValue(Short.toUnsignedInt(buffer.getShort(offset)));
            return value;
        }
    },
    HALF_VECTOR2(HalfVector2.class, 15, 4, SurfaceFormat.HalfVector2) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putInt(offset, (int)((HalfVector2)requireValue(value, offset))
                    .getPackedValue().longValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            HalfVector2 value = new HalfVector2();
            value.setPackedValue(Integer.toUnsignedLong(buffer.getInt(offset)));
            return value;
        }
    },
    HALF_VECTOR4(HalfVector4.class, 16, 8,
            SurfaceFormat.HalfVector4, SurfaceFormat.HdrBlendable) {
        @Override void put(ByteBuffer buffer, int offset, Object value) {
            buffer.putLong(offset, ((HalfVector4)requireValue(value, offset)).getPackedValue());
        }

        @Override Object get(ByteBuffer buffer, int offset) {
            HalfVector4 value = new HalfVector4();
            value.setPackedValue(buffer.getLong(offset));
            return value;
        }
    };

    private final Class<?> componentType;
    private final int dataType;
    private final int elementSize;
    private final SurfaceFormat[] formats;

    TextureDataCodec(
            Class<?> componentType,
            int dataType,
            int elementSize,
            SurfaceFormat... formats) {
        this.componentType = componentType;
        this.dataType = dataType;
        this.elementSize = elementSize;
        this.formats = formats;
    }

    abstract void put(ByteBuffer buffer, int offset, Object value);

    abstract Object get(ByteBuffer buffer, int offset);

    int dataType() {
        return dataType;
    }

    int elementSize() {
        return elementSize;
    }

    byte[] encode(Object[] data, int startIndex, int elementCount) {
        ByteBuffer buffer = nativeBuffer(data.length, elementSize);
        int end = startIndex + elementCount;
        for (int index = startIndex; index < end; index++) {
            put(buffer, Math.multiplyExact(index, elementSize), data[index]);
        }
        return buffer.array();
    }

    void decodeInto(byte[] encoded, Object[] destination, int startIndex, int elementCount) {
        if (encoded.length != Math.multiplyExact(destination.length, elementSize)) {
            throw new IllegalArgumentException("Native texture payload capacity changed unexpectedly");
        }
        ByteBuffer buffer = ByteBuffer.wrap(encoded).order(ByteOrder.nativeOrder());
        int end = startIndex + elementCount;
        for (int index = startIndex; index < end; index++) {
            destination[index] = get(buffer, Math.multiplyExact(index, elementSize));
        }
    }

    /**
     * Returns the one codec that carries a surface format, whatever element type a caller has.
     *
     * <p>{@link #select(Class, SurfaceFormat)} answers the game's question -- may these elements
     * become this format -- and this answers the content format's: what does this format's byte
     * layout actually look like.
     */
    static TextureDataCodec forFormat(SurfaceFormat format) {
        Objects.requireNonNull(format, "format");
        for (TextureDataCodec codec : values()) {
            if (codec.supports(format)) {
                return codec;
            }
        }
        throw new UnsupportedOperationException(
                "No CNA texture transfer carries SurfaceFormat." + format);
    }

    static TextureDataCodec select(Class<?> componentType, SurfaceFormat format) {
        Objects.requireNonNull(componentType, "componentType");
        Objects.requireNonNull(format, "format");
        for (TextureDataCodec codec : values()) {
            if (codec.componentType == componentType && codec.supports(format)) {
                return codec;
            }
        }
        throw new UnsupportedOperationException(
                "No safe CNA texture transfer maps Java " + componentType.getTypeName()
                        + " elements to SurfaceFormat." + format);
    }

    private boolean supports(SurfaceFormat format) {
        for (SurfaceFormat candidate : formats) {
            if (candidate == format) {
                return true;
            }
        }
        return false;
    }

    private static ByteBuffer nativeBuffer(int elementCount, int elementSize) {
        if (elementCount < 0) {
            throw new IllegalArgumentException("Texture element capacity must not be negative");
        }
        return ByteBuffer.allocate(Math.multiplyExact(elementCount, elementSize))
                .order(ByteOrder.nativeOrder());
    }

    private static Object requireValue(Object value, int byteOffset) {
        return Objects.requireNonNull(value, "Texture data element at byte offset " + byteOffset);
    }
}
