package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Vector3;

import java.util.Objects;

/** Built-in position-and-color vertex value. */
public final class VertexPositionColor implements IVertexType {

    public Vector3 Position;
    public Color Color;
    public static final VertexDeclaration VertexDeclaration = createDeclaration();

    public VertexPositionColor() {
        Position = new Vector3();
        Color = new Color();
    }

    public VertexPositionColor(VertexPositionColor value) {
        VertexPositionColor snapshot = Objects.requireNonNull(value, "value");
        Position = new Vector3(Objects.requireNonNull(snapshot.Position, "value.Position"));
        Color = new Color(Objects.requireNonNull(snapshot.Color, "value.Color"));
    }

    public VertexPositionColor(Vector3 position, Color color) {
        Position = new Vector3(Objects.requireNonNull(position, "position"));
        Color = new Color(Objects.requireNonNull(color, "color"));
    }

    @Override
    public VertexDeclaration getVertexDeclaration() {
        return VertexDeclaration;
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof VertexPositionColor other
                && Objects.equals(Position, other.Position)
                && Objects.equals(Color, other.Color);
    }

    @Override
    public int hashCode() {
        int hash = vector3Bits(Position) ^ colorBits(Color);
        return hash == 0 ? Integer.MAX_VALUE : hash;
    }

    @Override
    public String toString() {
        return "{Position:" + Position + " Color:" + Color + '}';
    }

    private static VertexDeclaration createDeclaration() {
        VertexDeclaration result = new VertexDeclaration(new VertexElement[]{
                new VertexElement(0, VertexElementFormat.Vector3, VertexElementUsage.Position, 0),
                new VertexElement(12, VertexElementFormat.Color, VertexElementUsage.Color, 0)
        });
        result.setName("VertexPositionColor.VertexDeclaration");
        return result;
    }

    static int vector3Bits(Vector3 value) {
        if (value == null) {
            return 0;
        }
        return Float.floatToRawIntBits(value.X)
                ^ Float.floatToRawIntBits(value.Y)
                ^ Float.floatToRawIntBits(value.Z);
    }

    static int vector2Bits(Microsoft.Xna.Framework.Vector2 value) {
        if (value == null) {
            return 0;
        }
        return Float.floatToRawIntBits(value.X) ^ Float.floatToRawIntBits(value.Y);
    }

    static int colorBits(Color value) {
        return value == null ? 0 : value.getPackedValue().intValue();
    }
}
