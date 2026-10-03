package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Mutable value projection of one entry in an XNA vertex declaration. */
public final class VertexElement {

    private int offset;
    private VertexElementFormat vertexElementFormat;
    private VertexElementUsage vertexElementUsage;
    private int usageIndex;

    public VertexElement() {
        vertexElementFormat = VertexElementFormat.Single;
        vertexElementUsage = VertexElementUsage.Position;
    }

    public VertexElement(VertexElement value) {
        VertexElement snapshot = Objects.requireNonNull(value, "value");
        offset = snapshot.offset;
        vertexElementFormat = snapshot.vertexElementFormat;
        vertexElementUsage = snapshot.vertexElementUsage;
        usageIndex = snapshot.usageIndex;
    }

    public VertexElement(
            int offset,
            VertexElementFormat elementFormat,
            VertexElementUsage elementUsage,
            int usageIndex) {
        this.offset = offset;
        this.usageIndex = usageIndex;
        vertexElementFormat = Objects.requireNonNull(elementFormat, "elementFormat");
        vertexElementUsage = Objects.requireNonNull(elementUsage, "elementUsage");
    }

    public int getOffset() {
        return offset;
    }

    public void setOffset(int value) {
        offset = value;
    }

    public int getUsageIndex() {
        return usageIndex;
    }

    public void setUsageIndex(int value) {
        usageIndex = value;
    }

    public VertexElementFormat getVertexElementFormat() {
        return vertexElementFormat;
    }

    public void setVertexElementFormat(VertexElementFormat value) {
        vertexElementFormat = Objects.requireNonNull(value, "value");
    }

    public VertexElementUsage getVertexElementUsage() {
        return vertexElementUsage;
    }

    public void setVertexElementUsage(VertexElementUsage value) {
        vertexElementUsage = Objects.requireNonNull(value, "value");
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof VertexElement other
                && offset == other.offset
                && usageIndex == other.usageIndex
                && vertexElementFormat == other.vertexElementFormat
                && vertexElementUsage == other.vertexElementUsage;
    }

    @Override
    public int hashCode() {
        int hash = offset ^ usageIndex ^ vertexElementFormat.ordinal()
                ^ vertexElementUsage.ordinal();
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        return "{Offset:" + offset
                + " Format:" + vertexElementFormat
                + " Usage:" + vertexElementUsage
                + " UsageIndex:" + usageIndex + '}';
    }
}
