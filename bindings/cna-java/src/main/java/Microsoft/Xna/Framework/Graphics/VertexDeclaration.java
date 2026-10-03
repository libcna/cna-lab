package Microsoft.Xna.Framework.Graphics;

import java.lang.reflect.InvocationTargetException;
import java.util.Objects;

/** Pure managed vertex layout which is materialized through CNA only while native code uses it. */
public class VertexDeclaration extends GraphicsResource {

    private final VertexElement[] elements;
    private final int vertexStride;

    public VertexDeclaration(VertexElement[] elements) {
        this(computeStride(requireElements(elements)), elements);
    }

    public VertexDeclaration(int vertexStride, VertexElement[] elements) {
        super();
        VertexElement[] source = requireElements(elements);
        this.elements = copyElements(source);
        this.vertexStride = vertexStride;
        validate(vertexStride, this.elements);
    }

    public final VertexElement[] GetVertexElements() {
        return copyElements(elements);
    }

    public final int getVertexStride() {
        return vertexStride;
    }

    @Override
    protected void Dispose(boolean arg0) {
        super.Dispose(arg0);
    }

    final int[] descriptorForUse(GraphicsDevice device) {
        ensureNotDisposed();
        attachGraphicsDevice(Objects.requireNonNull(device, "device"));
        int[] descriptor = new int[elements.length * 4];
        for (int index = 0; index < elements.length; index++) {
            VertexElement element = elements[index];
            if (element.getUsageIndex() < 0) {
                throw new IllegalArgumentException("Vertex element usage index must not be negative");
            }
            int destination = index * 4;
            descriptor[destination] = element.getOffset();
            descriptor[destination + 1] = element.getVertexElementFormat().ordinal();
            descriptor[destination + 2] = element.getVertexElementUsage().ordinal();
            descriptor[destination + 3] = element.getUsageIndex();
        }
        return descriptor;
    }

    static VertexDeclaration fromType(Class<?> vertexType) {
        Class<?> selected = Objects.requireNonNull(vertexType, "vertexType");
        if (!IVertexType.class.isAssignableFrom(selected)) {
            throw new IllegalArgumentException(
                    selected.getName() + " does not implement IVertexType");
        }
        try {
            IVertexType value = (IVertexType)selected.getConstructor().newInstance();
            VertexDeclaration declaration = value.getVertexDeclaration();
            if (declaration == null) {
                throw new IllegalStateException(
                        selected.getName() + " returned a null VertexDeclaration");
            }
            return declaration;
        } catch (NoSuchMethodException | InstantiationException | IllegalAccessException exception) {
            throw new IllegalArgumentException(
                    selected.getName() + " must expose a public default constructor", exception);
        } catch (InvocationTargetException exception) {
            throw new IllegalArgumentException(
                    selected.getName() + " could not be instantiated", exception.getCause());
        }
    }

    static int elementSize(VertexElementFormat format) {
        return switch (Objects.requireNonNull(format, "format")) {
            case Single, Color, Byte4, Short2, NormalizedShort2, HalfVector2 -> 4;
            case Vector2, Short4, NormalizedShort4, HalfVector4 -> 8;
            case Vector3 -> 12;
            case Vector4 -> 16;
        };
    }

    private static VertexElement[] requireElements(VertexElement[] values) {
        Objects.requireNonNull(values, "elements");
        if (values.length == 0) {
            throw new IllegalArgumentException("elements must not be empty");
        }
        return values;
    }

    private static VertexElement[] copyElements(VertexElement[] values) {
        VertexElement[] result = new VertexElement[values.length];
        for (int index = 0; index < values.length; index++) {
            result[index] = new VertexElement(
                    Objects.requireNonNull(values[index], "elements[" + index + "]"));
        }
        return result;
    }

    private static int computeStride(VertexElement[] values) {
        long result = 0;
        for (int index = 0; index < values.length; index++) {
            VertexElement element = Objects.requireNonNull(
                    values[index], "elements[" + index + "]");
            result = Math.max(result,
                    (long)element.getOffset() + elementSize(element.getVertexElementFormat()));
        }
        if (result > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Computed vertex stride exceeds the Java range");
        }
        return (int)result;
    }

    private static void validate(int stride, VertexElement[] values) {
        if (stride <= 0) {
            throw new IllegalArgumentException("vertexStride must be positive");
        }
        if ((stride & 3) != 0) {
            throw new IllegalArgumentException("vertexStride must be a multiple of four");
        }
        for (int index = 0; index < values.length; index++) {
            VertexElement current = values[index];
            int offset = current.getOffset();
            int size = elementSize(current.getVertexElementFormat());
            if (offset < 0 || (offset & 3) != 0 || (long)offset + size > stride) {
                throw new IllegalArgumentException(
                        "Vertex element " + index + " is outside or misaligned within the stride");
            }
            for (int previousIndex = 0; previousIndex < index; previousIndex++) {
                VertexElement previous = values[previousIndex];
                if (previous.getVertexElementUsage() == current.getVertexElementUsage()
                        && previous.getUsageIndex() == current.getUsageIndex()) {
                    throw new IllegalArgumentException(
                            "Vertex declaration contains a duplicate usage and index");
                }
                int previousOffset = previous.getOffset();
                int previousEnd = previousOffset
                        + elementSize(previous.getVertexElementFormat());
                if (offset < previousEnd && previousOffset < offset + size) {
                    throw new IllegalArgumentException("Vertex declaration elements overlap");
                }
            }
        }
    }
}
