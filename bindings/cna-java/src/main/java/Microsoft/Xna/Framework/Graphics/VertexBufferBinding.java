package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Immutable Java value projection of one XNA vertex-buffer binding. */
public final class VertexBufferBinding {

    private final VertexBuffer vertexBuffer;
    private final int vertexOffset;
    private final int instanceFrequency;

    public VertexBufferBinding() {
        vertexBuffer = null;
        vertexOffset = 0;
        instanceFrequency = 0;
    }

    public VertexBufferBinding(VertexBufferBinding value) {
        VertexBufferBinding snapshot = Objects.requireNonNull(value, "value");
        vertexBuffer = snapshot.vertexBuffer;
        vertexOffset = snapshot.vertexOffset;
        instanceFrequency = snapshot.instanceFrequency;
    }

    public VertexBufferBinding(
            VertexBuffer vertexBuffer,
            int vertexOffset,
            int instanceFrequency) {
        VertexBuffer selected = Objects.requireNonNull(vertexBuffer, "vertexBuffer");
        if (vertexOffset < 0 || vertexOffset >= selected.getVertexCount()) {
            throw new IndexOutOfBoundsException("vertexOffset is outside the vertex buffer");
        }
        if (instanceFrequency < 0) {
            throw new IllegalArgumentException("instanceFrequency must not be negative");
        }
        this.vertexBuffer = selected;
        this.vertexOffset = vertexOffset;
        this.instanceFrequency = instanceFrequency;
    }

    public VertexBufferBinding(VertexBuffer vertexBuffer, int vertexOffset) {
        this(vertexBuffer, vertexOffset, 0);
    }

    public VertexBufferBinding(VertexBuffer vertexBuffer) {
        this(vertexBuffer, 0, 0);
    }

    public static VertexBufferBinding fromVertexBuffer(VertexBuffer vertexBuffer) {
        return new VertexBufferBinding(vertexBuffer);
    }

    public int getInstanceFrequency() {
        return instanceFrequency;
    }

    public VertexBuffer getVertexBuffer() {
        return vertexBuffer;
    }

    public int getVertexOffset() {
        return vertexOffset;
    }
}
