package org.openeggbert.cna.extensions.graphics;

import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;

/**
 * The arguments of an indexed indirect draw, in the exact layout the GPU reads them in.
 *
 * <p>The same contract as {@link IndirectDrawArguments}, one word longer: <strong>twenty bytes,
 * five words, little-endian.</strong> {@code baseVertex} is signed, as the underlying API is.
 */
public final class IndirectDrawIndexedArguments {

    /** The size of the structure the GPU reads, in bytes. */
    public static final int BYTES = 20;

    private final int indexCount;
    private final int instanceCount;
    private final int firstIndex;
    private final int baseVertex;
    private final int baseInstance;

    /**
     * Creates the arguments.
     *
     * @param indexCount how many indices to fetch
     * @param instanceCount how many instances to draw
     * @param firstIndex the first index, in index elements
     * @param baseVertex added to every decoded index, in vertex elements; signed
     * @param baseInstance the first instance; must be zero on OpenGL ES
     */
    public IndirectDrawIndexedArguments(int indexCount, int instanceCount, int firstIndex,
            int baseVertex, int baseInstance) {
        this.indexCount = indexCount;
        this.instanceCount = instanceCount;
        this.firstIndex = firstIndex;
        this.baseVertex = baseVertex;
        this.baseInstance = baseInstance;
    }

    /**
     * Returns CNA's own defaults.
     *
     * @return the default arguments, which draw nothing
     */
    public static IndirectDrawIndexedArguments defaults() {
        GraphicsExtension.requireBackend();
        long[] fields = new long[5];
        GraphicsExtension.check("IndirectDrawIndexedArguments.defaults",
                NativeGraphicsExtensionRoutes.indirectDrawIndexedArgumentsInit(fields));
        return new IndirectDrawIndexedArguments((int) fields[0], (int) fields[1], (int) fields[2],
                (int) fields[3], (int) fields[4]);
    }

    /** @return how many indices to fetch */
    public int getIndexCount() {
        return indexCount;
    }

    /** @return how many instances to draw */
    public int getInstanceCount() {
        return instanceCount;
    }

    /** @return the first index, in index elements */
    public int getFirstIndex() {
        return firstIndex;
    }

    /** @return the value added to every decoded index */
    public int getBaseVertex() {
        return baseVertex;
    }

    /** @return the first instance */
    public int getBaseInstance() {
        return baseInstance;
    }

    /**
     * Writes the five words the GPU reads.
     *
     * @return exactly {@link #BYTES} bytes, little-endian
     */
    public byte[] toBytes() {
        ByteBuffer buffer = ByteBuffer.allocate(BYTES).order(ByteOrder.LITTLE_ENDIAN);
        buffer.putInt(indexCount).putInt(instanceCount).putInt(firstIndex).putInt(baseVertex)
                .putInt(baseInstance);
        return buffer.array();
    }

    /**
     * Reads five words back.
     *
     * @param bytes at least {@link #BYTES} bytes, little-endian
     * @param offset where the structure starts
     * @return the arguments those bytes describe
     * @throws IllegalArgumentException when the range does not fit
     */
    public static IndirectDrawIndexedArguments fromBytes(byte[] bytes, int offset) {
        Objects.requireNonNull(bytes, "bytes");
        if (offset < 0 || bytes.length - offset < BYTES) {
            throw new IllegalArgumentException(
                    "indexed indirect draw arguments need " + BYTES + " bytes at offset " + offset);
        }
        ByteBuffer buffer = ByteBuffer.wrap(bytes, offset, BYTES).order(ByteOrder.LITTLE_ENDIAN);
        return new IndirectDrawIndexedArguments(buffer.getInt(), buffer.getInt(), buffer.getInt(),
                buffer.getInt(), buffer.getInt());
    }

    @Override
    public boolean equals(Object other) {
        if (this == other) {
            return true;
        }
        if (!(other instanceof IndirectDrawIndexedArguments)) {
            return false;
        }
        IndirectDrawIndexedArguments that = (IndirectDrawIndexedArguments) other;
        return indexCount == that.indexCount && instanceCount == that.instanceCount
                && firstIndex == that.firstIndex && baseVertex == that.baseVertex
                && baseInstance == that.baseInstance;
    }

    @Override
    public int hashCode() {
        return Objects.hash(indexCount, instanceCount, firstIndex, baseVertex, baseInstance);
    }

    @Override
    public String toString() {
        return "IndirectDrawIndexedArguments{indexCount=" + indexCount + ", instanceCount="
                + instanceCount + ", firstIndex=" + firstIndex + ", baseVertex=" + baseVertex
                + ", baseInstance=" + baseInstance + "}";
    }
}
