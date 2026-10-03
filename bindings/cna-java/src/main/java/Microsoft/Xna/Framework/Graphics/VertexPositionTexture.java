package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;

import java.util.Objects;

/** Built-in position-and-texture-coordinate vertex value. */
public final class VertexPositionTexture implements IVertexType {

    public Vector3 Position;
    public Vector2 TextureCoordinate;
    public static final VertexDeclaration VertexDeclaration = createDeclaration();

    public VertexPositionTexture() {
        Position = new Vector3();
        TextureCoordinate = new Vector2();
    }

    public VertexPositionTexture(VertexPositionTexture value) {
        VertexPositionTexture snapshot = Objects.requireNonNull(value, "value");
        Position = new Vector3(Objects.requireNonNull(snapshot.Position, "value.Position"));
        TextureCoordinate = new Vector2(
                Objects.requireNonNull(snapshot.TextureCoordinate, "value.TextureCoordinate"));
    }

    public VertexPositionTexture(Vector3 position, Vector2 textureCoordinate) {
        Position = new Vector3(Objects.requireNonNull(position, "position"));
        TextureCoordinate = new Vector2(
                Objects.requireNonNull(textureCoordinate, "textureCoordinate"));
    }

    @Override
    public VertexDeclaration getVertexDeclaration() {
        return VertexDeclaration;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof VertexPositionTexture other
                && Objects.equals(Position, other.Position)
                && Objects.equals(TextureCoordinate, other.TextureCoordinate);
    }

    @Override
    public int hashCode() {
        int hash = VertexPositionColor.vector3Bits(Position)
                ^ VertexPositionColor.vector2Bits(TextureCoordinate);
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        return "{Position:" + Position + " TextureCoordinate:" + TextureCoordinate + '}';
    }

    private static VertexDeclaration createDeclaration() {
        VertexDeclaration result = new VertexDeclaration(new VertexElement[]{
                new VertexElement(0, VertexElementFormat.Vector3, VertexElementUsage.Position, 0),
                new VertexElement(
                        12, VertexElementFormat.Vector2,
                        VertexElementUsage.TextureCoordinate, 0)
        });
        result.setName("VertexPositionTexture.VertexDeclaration");
        return result;
    }
}
