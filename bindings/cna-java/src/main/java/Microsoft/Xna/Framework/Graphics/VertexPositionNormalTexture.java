package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;

import java.util.Objects;

/** Built-in position, normal, and texture-coordinate vertex value. */
public final class VertexPositionNormalTexture implements IVertexType {

    public Vector3 Position;
    public Vector3 Normal;
    public Vector2 TextureCoordinate;
    public static final VertexDeclaration VertexDeclaration = createDeclaration();

    public VertexPositionNormalTexture() {
        Position = new Vector3();
        Normal = new Vector3();
        TextureCoordinate = new Vector2();
    }

    public VertexPositionNormalTexture(VertexPositionNormalTexture value) {
        VertexPositionNormalTexture snapshot = Objects.requireNonNull(value, "value");
        Position = new Vector3(Objects.requireNonNull(snapshot.Position, "value.Position"));
        Normal = new Vector3(Objects.requireNonNull(snapshot.Normal, "value.Normal"));
        TextureCoordinate = new Vector2(
                Objects.requireNonNull(snapshot.TextureCoordinate, "value.TextureCoordinate"));
    }

    public VertexPositionNormalTexture(
            Vector3 position,
            Vector3 normal,
            Vector2 textureCoordinate) {
        Position = new Vector3(Objects.requireNonNull(position, "position"));
        Normal = new Vector3(Objects.requireNonNull(normal, "normal"));
        TextureCoordinate = new Vector2(
                Objects.requireNonNull(textureCoordinate, "textureCoordinate"));
    }

    @Override
    public VertexDeclaration getVertexDeclaration() {
        return VertexDeclaration;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof VertexPositionNormalTexture other
                && Objects.equals(Position, other.Position)
                && Objects.equals(Normal, other.Normal)
                && Objects.equals(TextureCoordinate, other.TextureCoordinate);
    }

    @Override
    public int hashCode() {
        int hash = VertexPositionColor.vector3Bits(Position)
                ^ VertexPositionColor.vector3Bits(Normal)
                ^ VertexPositionColor.vector2Bits(TextureCoordinate);
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        return "{Position:" + Position + " Normal:" + Normal
                + " TextureCoordinate:" + TextureCoordinate + '}';
    }

    private static VertexDeclaration createDeclaration() {
        VertexDeclaration result = new VertexDeclaration(new VertexElement[]{
                new VertexElement(0, VertexElementFormat.Vector3, VertexElementUsage.Position, 0),
                new VertexElement(12, VertexElementFormat.Vector3, VertexElementUsage.Normal, 0),
                new VertexElement(
                        24, VertexElementFormat.Vector2,
                        VertexElementUsage.TextureCoordinate, 0)
        });
        result.setName("VertexPositionNormalTexture.VertexDeclaration");
        return result;
    }
}
