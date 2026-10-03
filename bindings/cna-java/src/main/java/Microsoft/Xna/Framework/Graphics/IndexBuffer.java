package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;

import java.lang.reflect.Array;
import java.util.Objects;

/** Device-owned 16- or 32-bit index buffer backed by CNA's stable resource ABI. */
@SuppressWarnings("this-escape")
public class IndexBuffer extends GraphicsResource {

    private final BufferUsage bufferUsage;
    private final int indexCount;
    private final IndexElementSize indexElementSize;

    public IndexBuffer(
            GraphicsDevice graphicsDevice,
            IndexElementSize indexElementSize,
            int indexCount,
            BufferUsage usage) {
        this(graphicsDevice, indexElementSize, indexCount, usage, false);
    }

    IndexBuffer(
            GraphicsDevice graphicsDevice,
            IndexElementSize indexElementSize,
            int indexCount,
            BufferUsage usage,
            boolean dynamic) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
        IndexElementSize selectedSize = Objects.requireNonNull(
                indexElementSize, "indexElementSize");
        if (indexCount <= 0) {
            throw new IllegalArgumentException("indexCount must be positive");
        }
        BufferUsage selectedUsage = Objects.requireNonNull(usage, "usage");
        int[] info = NativeBindings.createIndexBuffer(
                this, graphicsDevice, selectedSize.ordinal(), indexCount,
                selectedUsage.getValue(), dynamic);
        if (info.length != 6 || info[0] != indexCount
                || info[1] != selectedSize.ordinal()
                || info[2] != selectedUsage.getValue()
                || (info[3] != 0) != dynamic) {
            NativeBindings.closeGraphicsResource(this);
            throw new IllegalStateException("CNA returned inconsistent IndexBuffer metadata");
        }
        this.indexElementSize = selectedSize;
        this.indexCount = indexCount;
        bufferUsage = selectedUsage;
    }

    public IndexBuffer(
            GraphicsDevice graphicsDevice,
            Class<?> indexType,
            int indexCount,
            BufferUsage usage) {
        this(graphicsDevice, sizeForType(indexType), indexCount, usage);
    }

    public final <T> void SetData(T[] data) {
        Objects.requireNonNull(data, "data");
        SetData(data, 0, data.length);
    }

    public final <T> void SetData(T[] data, int startIndex, int elementCount) {
        setData(-1, data, startIndex, elementCount, SetDataOptions.None);
    }

    public final <T> void SetData(
            int offsetInBytes,
            T[] data,
            int startIndex,
            int elementCount) {
        setData(offsetInBytes, data, startIndex, elementCount, SetDataOptions.None);
    }

    public final <T> void GetData(T[] data) {
        Objects.requireNonNull(data, "data");
        GetData(data, 0, data.length);
    }

    public final <T> void GetData(T[] data, int startIndex, int elementCount) {
        GetData(0, data, startIndex, elementCount);
    }

    public final <T> void GetData(
            int offsetInBytes,
            T[] data,
            int startIndex,
            int elementCount) {
        ensureNotDisposed();
        Objects.requireNonNull(data, "data");
        validateArrayWindow(data.length, startIndex, elementCount);
        validateComponentType(data.getClass().getComponentType());
        if (offsetInBytes != 0) {
            throw new UnsupportedOperationException(
                    "CNA 0.7.0 has no index-buffer readback route with a buffer byte offset");
        }
        if (elementCount > indexCount) {
            throw new IndexOutOfBoundsException("Index readback exceeds the buffer capacity");
        }
        int[] snapshot = NativeBindings.getIndexBufferData(
                this, indexElementSize.ordinal(), elementCount);
        Object[] decoded = decode(snapshot, data.getClass().getComponentType());
        System.arraycopy(decoded, 0, data, startIndex, elementCount);
    }

    public final BufferUsage getBufferUsage() {
        ensureNotDisposed();
        return bufferUsage;
    }

    public final int getIndexCount() {
        ensureNotDisposed();
        return indexCount;
    }

    public final IndexElementSize getIndexElementSize() {
        ensureNotDisposed();
        return indexElementSize;
    }

    @Override
    protected void Dispose(boolean arg0) {
        if (arg0 && !getIsDisposed()) {
            releaseDynamicSubscription();
            NativeBindings.closeGraphicsResource(this);
        }
        super.Dispose(arg0);
    }

    void releaseDynamicSubscription() {
    }

    final <T> void setData(
            int offsetInBytes,
            T[] data,
            int startIndex,
            int elementCount,
            SetDataOptions options) {
        ensureNotDisposed();
        Objects.requireNonNull(data, "data");
        validateArrayWindow(data.length, startIndex, elementCount);
        Class<?> componentType = data.getClass().getComponentType();
        validateComponentType(componentType);
        int elementBytes = indexElementSize == IndexElementSize.SixteenBits ? 2 : 4;
        if (offsetInBytes >= 0 && (offsetInBytes % elementBytes != 0
                || (long)offsetInBytes + (long)elementCount * elementBytes
                > (long)indexCount * elementBytes)) {
            throw new IndexOutOfBoundsException("Index upload window is outside the buffer");
        }
        if (offsetInBytes < -1) {
            throw new IllegalArgumentException("offsetInBytes must not be negative");
        }
        if (offsetInBytes >= 0
                && !SetDataOptions.None.equals(Objects.requireNonNull(options, "options"))) {
            throw new UnsupportedOperationException(
                    "CNA's windowed index-buffer ABI cannot carry SetDataOptions");
        }
        if (offsetInBytes < 0 && elementCount > indexCount) {
            throw new IndexOutOfBoundsException("Index upload exceeds the buffer capacity");
        }
        int[] snapshot = encode(data, startIndex, elementCount);
        NativeBindings.setIndexBufferData(
                this, offsetInBytes, indexElementSize.ordinal(), snapshot,
                Objects.requireNonNull(options, "options").getValue());
    }

    private int[] encode(Object[] data, int startIndex, int elementCount) {
        int[] result = new int[elementCount];
        for (int index = 0; index < elementCount; index++) {
            Object item = Objects.requireNonNull(
                    data[startIndex + index], "data[" + (startIndex + index) + "]");
            long value;
            if (item instanceof Short shortValue) {
                value = Short.toUnsignedLong(shortValue);
            } else if (item instanceof Integer integerValue) {
                value = indexElementSize == IndexElementSize.SixteenBits
                        ? integerValue.longValue()
                        : Integer.toUnsignedLong(integerValue);
            } else if (item instanceof Long longValue) {
                value = longValue;
            } else {
                throw new UnsupportedOperationException("Unsupported Java index element type");
            }
            long maximum = indexElementSize == IndexElementSize.SixteenBits
                    ? 0xffffL : 0xffff_ffffL;
            if (value < 0 || value > maximum) {
                throw new IllegalArgumentException("Index value is outside the buffer element width");
            }
            result[index] = (int)value;
        }
        return result;
    }

    private Object[] decode(int[] values, Class<?> componentType) {
        Object[] result = (Object[])Array.newInstance(componentType, values.length);
        for (int index = 0; index < values.length; index++) {
            if (componentType == Short.class) {
                result[index] = (short)values[index];
            } else if (componentType == Integer.class) {
                result[index] = indexElementSize == IndexElementSize.SixteenBits
                        ? values[index] & 0xffff : values[index];
            } else {
                result[index] = Integer.toUnsignedLong(values[index]);
            }
        }
        return result;
    }

    private void validateComponentType(Class<?> componentType) {
        boolean valid = indexElementSize == IndexElementSize.SixteenBits
                ? componentType == Short.class || componentType == Integer.class
                : componentType == Integer.class || componentType == Long.class;
        if (!valid) {
            throw new UnsupportedOperationException(
                    "Index array component type does not match the buffer element width");
        }
    }

    static IndexElementSize sizeForType(Class<?> type) {
        Class<?> selected = Objects.requireNonNull(type, "indexType");
        if (selected == short.class || selected == Short.class) {
            return IndexElementSize.SixteenBits;
        }
        if (selected == int.class || selected == Integer.class
                || selected == long.class || selected == Long.class) {
            return IndexElementSize.ThirtyTwoBits;
        }
        throw new IllegalArgumentException("Unsupported Java index element type " + selected.getName());
    }

    private static void validateArrayWindow(int length, int startIndex, int elementCount) {
        if (startIndex < 0 || startIndex > length) {
            throw new IndexOutOfBoundsException("Index data start index is outside the array");
        }
        if (elementCount <= 0 || elementCount > length - startIndex) {
            throw new IndexOutOfBoundsException("Index data window is outside the array");
        }
    }
}
