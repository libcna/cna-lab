package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;

import java.util.Objects;

/** Built-in position, color, and texture-coordinate vertex value. */
public final class VertexPositionColorTexture implements IVertexType {

    public Vector3 Position;
    public Color Color;
    public Vector2 TextureCoordinate;
    public static final VertexDeclaration VertexDeclaration = createDeclaration();

    public VertexPositionColorTexture() {
        Position = new Vector3();
        Color = new Color();
        TextureCoordinate = new Vector2();
    }

    public VertexPositionColorTexture(VertexPositionColorTexture value) {
        VertexPositionColorTexture snapshot = Objects.requireNonNull(value, "value");
        Position = new Vector3(Objects.requireNonNull(snapshot.Position, "value.Position"));
        Color = new Color(Objects.requireNonNull(snapshot.Color, "value.Color"));
        TextureCoordinate = new Vector2(
                Objects.requireNonNull(snapshot.TextureCoordinate, "value.TextureCoordinate"));
    }

    public VertexPositionColorTexture(
            Vector3 position,
            Color color,
            Vector2 textureCoordinate) {
        Position = new Vector3(Objects.requireNonNull(position, "position"));
        Color = new Color(Objects.requireNonNull(color, "color"));
        TextureCoordinate = new Vector2(
                Objects.requireNonNull(textureCoordinate, "textureCoordinate"));
    }

    @Override
    public VertexDeclaration getVertexDeclaration() {
        return VertexDeclaration;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof VertexPositionColorTexture other
                && Objects.equals(Position, other.Position)
                && Objects.equals(Color, other.Color)
                && Objects.equals(TextureCoordinate, other.TextureCoordinate);
    }

    @Override
    public int hashCode() {
        int hash = VertexPositionColor.vector3Bits(Position)
                ^ VertexPositionColor.colorBits(Color)
                ^ VertexPositionColor.vector2Bits(TextureCoordinate);
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        return "{Position:" + Position + " Color:" + Color
                + " TextureCoordinate:" + TextureCoordinate + '}';
    }

    private static VertexDeclaration createDeclaration() {
        VertexDeclaration result = new VertexDeclaration(new VertexElement[]{
                new VertexElement(0, VertexElementFormat.Vector3, VertexElementUsage.Position, 0),
                new VertexElement(12, VertexElementFormat.Color, VertexElementUsage.Color, 0),
                new VertexElement(
                        16, VertexElementFormat.Vector2,
                        VertexElementUsage.TextureCoordinate, 0)
        });
        result.setName("VertexPositionColorTexture.VertexDeclaration");
        return result;
    }
}
