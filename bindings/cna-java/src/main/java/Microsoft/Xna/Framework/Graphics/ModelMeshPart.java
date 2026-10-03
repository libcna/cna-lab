package Microsoft.Xna.Framework.Graphics;

/** One draw range within an identity-stable XNA ModelMesh. */
public final class ModelMeshPart {

    private final int vertexOffset;
    private final int numVertices;
    private final int startIndex;
    private final int primitiveCount;
    private VertexBuffer vertexBuffer;
    private IndexBuffer indexBuffer;
    private Effect effect;
    private Object tag;
    private ModelMesh parent;

    ModelMeshPart(
            int vertexOffset,
            int numVertices,
            int startIndex,
            int primitiveCount,
            Object tag) {
        this.vertexOffset = vertexOffset;
        this.numVertices = numVertices;
        this.startIndex = startIndex;
        this.primitiveCount = primitiveCount;
        this.tag = tag;
    }

    public int getStartIndex() {
        return startIndex;
    }

    public int getPrimitiveCount() {
        return primitiveCount;
    }

    public int getVertexOffset() {
        return vertexOffset;
    }

    public int getNumVertices() {
        return numVertices;
    }

    public IndexBuffer getIndexBuffer() {
        return indexBuffer;
    }

    public VertexBuffer getVertexBuffer() {
        return vertexBuffer;
    }

    public Effect getEffect() {
        return effect;
    }

    public void setEffect(Effect value) {
        if (value == effect) {
            return;
        }
        if (parent == null) {
            effect = value;
            return;
        }
        boolean oldUsedByAnotherPart = false;
        boolean newUsedByAnotherPart = false;
        for (ModelMeshPart part : parent.getMeshParts()) {
            if (part == this) {
                continue;
            }
            oldUsedByAnotherPart |= part.effect == effect;
            newUsedByAnotherPart |= part.effect == value;
        }
        if (!oldUsedByAnotherPart && effect != null) {
            parent.getEffects().removeEffect(effect);
        }
        if (!newUsedByAnotherPart && value != null) {
            parent.getEffects().addEffect(value);
        }
        effect = value;
    }

    public Object getTag() {
        return tag;
    }

    public void setTag(Object value) {
        tag = value;
    }

    void attach(ModelMesh newParent) {
        parent = newParent;
    }

    void setBuffers(VertexBuffer vertexBuffer, IndexBuffer indexBuffer) {
        this.vertexBuffer = vertexBuffer;
        this.indexBuffer = indexBuffer;
    }

    void draw() {
        if (numVertices <= 0) {
            return;
        }
        if (vertexBuffer == null || indexBuffer == null) {
            throw new IllegalStateException("ModelMeshPart has no vertex/index buffer");
        }
        GraphicsDevice device = vertexBuffer.getGraphicsDevice();
        device.SetVertexBuffer(vertexBuffer, vertexOffset);
        device.setIndices(indexBuffer);
        device.DrawIndexedPrimitives(
                PrimitiveType.TriangleList, 0, 0, numVertices, startIndex, primitiveCount);
    }
}
