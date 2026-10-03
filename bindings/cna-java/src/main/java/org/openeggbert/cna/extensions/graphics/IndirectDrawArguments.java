package org.openeggbert.cna.extensions.graphics;

import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;

/**
 * The arguments of an indirect draw, in the exact layout the GPU reads them in.
 *
 * <p>A CNA extension. In XNA every draw carries its counts from the CPU; an indirect draw reads
 * them out of GPU memory. CNA ABI 0.30 removed the indirect draw routes and the storage buffers
 * that fed them together with the engine layer, but {@code graphics_ext.h} still declares this
 * structure and its initializer, so it is kept as a value.
 *
 * <p><strong>Sixteen bytes, four words, little-endian.</strong> This is a wire format rather than
 * a convenience type: {@link #toBytes()} produces exactly the bytes the GPU's command processor
 * reads, and {@link #fromBytes} reads them back.
 *
 * <p><strong>{@code baseInstance} must be zero on OpenGL ES.</strong> ES 3.1 has no base-instance
 * parameter and requires the word to be zero; a non-zero value there is undefined rather than
 * diagnosed, and cannot be checked anywhere, because by the time the draw runs the value lives in
 * GPU memory. CNA says so and this repeats it rather than pretending to enforce it.
 */
public final class IndirectDrawArguments {

    /** The size of the structure the GPU reads, in bytes. */
    public static final int BYTES = 16;

    private final int vertexCount;
    private final int instanceCount;
    private final int firstVertex;
    private final int baseInstance;

    /**
     * Creates the arguments.
     *
     * @param vertexCount how many vertices to fetch
     * @param instanceCount how many instances to draw; one for an ordinary draw, zero to draw
     *        nothing
     * @param firstVertex the first vertex, in elements of the bound stream
     * @param baseInstance the first instance; must be zero on OpenGL ES
     */
    public IndirectDrawArguments(int vertexCount, int instanceCount, int firstVertex,
            int baseInstance) {
        this.vertexCount = vertexCount;
        this.instanceCount = instanceCount;
        this.firstVertex = firstVertex;
        this.baseInstance = baseInstance;
    }

    /**
     * Returns CNA's own defaults.
     *
     * <p>Asked of CNA rather than assumed here. The value is documented to draw nothing, which is
     * the safe starting point for a buffer a shader is about to fill.
     *
     * @return the default arguments
     */
    public static IndirectDrawArguments defaults() {
        GraphicsExtension.requireBackend();
        long[] fields = new long[4];
        GraphicsExtension.check("IndirectDrawArguments.defaults",
                NativeGraphicsExtensionRoutes.indirectDrawArgumentsInit(fields));
        return new IndirectDrawArguments((int) fields[0], (int) fields[1], (int) fields[2],
                (int) fields[3]);
    }

    /** @return how many vertices to fetch */
    public int getVertexCount() {
        return vertexCount;
    }

    /** @return how many instances to draw */
    public int getInstanceCount() {
        return instanceCount;
    }

    /** @return the first vertex, in elements of the bound stream */
    public int getFirstVertex() {
        return firstVertex;
    }

    /** @return the first instance */
    public int getBaseInstance() {
        return baseInstance;
    }

    /**
     * Writes the four words the GPU reads.
     *
     * @return exactly {@link #BYTES} bytes, little-endian
     */
    public byte[] toBytes() {
        ByteBuffer buffer = ByteBuffer.allocate(BYTES).order(ByteOrder.LITTLE_ENDIAN);
        buffer.putInt(vertexCount).putInt(instanceCount).putInt(firstVertex).putInt(baseInstance);
        return buffer.array();
    }

    /**
     * Reads four words back.
     *
     * @param bytes at least {@link #BYTES} bytes, little-endian
     * @param offset where the structure starts
     * @return the arguments those bytes describe
     * @throws IllegalArgumentException when the range does not fit
     */
    public static IndirectDrawArguments fromBytes(byte[] bytes, int offset) {
        Objects.requireNonNull(bytes, "bytes");
        if (offset < 0 || bytes.length - offset < BYTES) {
            throw new IllegalArgumentException(
                    "indirect draw arguments need " + BYTES + " bytes at offset " + offset);
        }
        ByteBuffer buffer = ByteBuffer.wrap(bytes, offset, BYTES).order(ByteOrder.LITTLE_ENDIAN);
        return new IndirectDrawArguments(buffer.getInt(), buffer.getInt(), buffer.getInt(),
                buffer.getInt());
    }

    @Override
    public boolean equals(Object other) {
        if (this == other) {
            return true;
        }
        if (!(other instanceof IndirectDrawArguments)) {
            return false;
        }
        IndirectDrawArguments that = (IndirectDrawArguments) other;
        return vertexCount == that.vertexCount && instanceCount == that.instanceCount
                && firstVertex == that.firstVertex && baseInstance == that.baseInstance;
    }

    @Override
    public int hashCode() {
        return Objects.hash(vertexCount, instanceCount, firstVertex, baseInstance);
    }

    @Override
    public String toString() {
        return "IndirectDrawArguments{vertexCount=" + vertexCount + ", instanceCount="
                + instanceCount + ", firstVertex=" + firstVertex + ", baseInstance=" + baseInstance
                + "}";
    }
}
