package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

final class VertexValueTests {

    @Test
    void VertexAndIndexEnumsPreserveXnaNumericIdentitiesAndFlags() {
        assertEquals(0, IndexElementSize.SixteenBits.ordinal());
        assertEquals(1, IndexElementSize.ThirtyTwoBits.ordinal());
        assertEquals(0, PrimitiveType.TriangleList.ordinal());
        assertEquals(3, PrimitiveType.LineStrip.ordinal());
        assertEquals(0, VertexElementFormat.Single.ordinal());
        assertEquals(11, VertexElementFormat.HalfVector4.ordinal());
        assertEquals(0, VertexElementUsage.Position.ordinal());
        assertEquals(12, VertexElementUsage.TessellateFactor.ordinal());

        assertEquals(0, BufferUsage.None.getValue());
        assertEquals(1, BufferUsage.WriteOnly.getValue());
        assertTrue(BufferUsage.WriteOnly.Contains(BufferUsage.WriteOnly));
        assertEquals(3, SetDataOptions.Discard.Or(SetDataOptions.NoOverwrite).getValue());
        assertTrue(SetDataOptions.FromValue(3).Contains(SetDataOptions.NoOverwrite));
    }

    @Test
    void VertexDeclarationsComputeStrideSnapshotElementsAndRejectInvalidLayouts() {
        VertexElement position = new VertexElement(
                0, VertexElementFormat.Vector3, VertexElementUsage.Position, 0);
        VertexElement color = new VertexElement(
                12, VertexElementFormat.Color, VertexElementUsage.Color, 0);
        VertexDeclaration declaration = new VertexDeclaration(
                new VertexElement[]{position, color});

        assertEquals(16, declaration.getVertexStride());
        assertArrayEquals(new VertexElement[]{position, color}, declaration.GetVertexElements());
        position.setOffset(4);
        color.setUsageIndex(7);
        assertEquals(0, declaration.GetVertexElements()[0].getOffset());
        assertEquals(0, declaration.GetVertexElements()[1].getUsageIndex());
        VertexElement[] returned = declaration.GetVertexElements();
        returned[0].setOffset(8);
        assertEquals(0, declaration.GetVertexElements()[0].getOffset());

        assertEquals(16, VertexPositionColor.VertexDeclaration.getVertexStride());
        assertEquals(20, VertexPositionTexture.VertexDeclaration.getVertexStride());
        assertEquals(24, VertexPositionColorTexture.VertexDeclaration.getVertexStride());
        assertEquals(32, VertexPositionNormalTexture.VertexDeclaration.getVertexStride());
        assertEquals(VertexPositionColor.VertexDeclaration,
                new VertexPositionColor().getVertexDeclaration());

        assertThrows(IllegalArgumentException.class,
                () -> new VertexDeclaration(new VertexElement[0]));
        assertThrows(IllegalArgumentException.class,
                () -> new VertexDeclaration(15, new VertexElement[]{
                        new VertexElement(
                                0, VertexElementFormat.Vector3,
                                VertexElementUsage.Position, 0)}));
        assertThrows(IllegalArgumentException.class,
                () -> new VertexDeclaration(16, new VertexElement[]{
                        new VertexElement(
                                2, VertexElementFormat.Vector2,
                                VertexElementUsage.Position, 0)}));
        assertThrows(IllegalArgumentException.class,
                () -> new VertexDeclaration(16, new VertexElement[]{
                        new VertexElement(
                                0, VertexElementFormat.Vector3,
                                VertexElementUsage.Position, 0),
                        new VertexElement(
                                8, VertexElementFormat.Vector2,
                                VertexElementUsage.TextureCoordinate, 0)}));
        assertThrows(IllegalArgumentException.class,
                () -> new VertexDeclaration(24, new VertexElement[]{
                        new VertexElement(
                                0, VertexElementFormat.Vector3,
                                VertexElementUsage.Position, 0),
                        new VertexElement(
                                12, VertexElementFormat.Vector3,
                                VertexElementUsage.Position, 0)}));
    }

    @Test
    void BuiltInVertexValuesSnapshotMutableInputsAndRoundTripExactBinaryLayouts() {
        Vector3 position = new Vector3(1.25f, -2.5f, 3.75f);
        Vector3 normal = new Vector3(-1.0f, 0.5f, 0.25f);
        Vector2 texture = new Vector2(0.125f, 0.875f);
        Color color = new Color(1, 2, 3, 4);

        VertexPositionColor positionColor = new VertexPositionColor(position, color);
        VertexPositionTexture positionTexture = new VertexPositionTexture(position, texture);
        VertexPositionColorTexture positionColorTexture =
                new VertexPositionColorTexture(position, color, texture);
        VertexPositionNormalTexture positionNormalTexture =
                new VertexPositionNormalTexture(position, normal, texture);
        position.X = 99.0f;
        normal.Y = 99.0f;
        texture.X = 99.0f;
        color.setR(99);

        assertEquals(new Vector3(1.25f, -2.5f, 3.75f), positionColor.Position);
        assertEquals(new Color(1, 2, 3, 4), positionColor.Color);
        assertEquals(new Vector2(0.125f, 0.875f), positionTexture.TextureCoordinate);
        assertEquals(new Vector3(-1.0f, 0.5f, 0.25f), positionNormalTexture.Normal);
        assertEquals(Integer.MAX_VALUE, new VertexPositionColor().hashCode());
        assertEquals(
                "{Position:{X:1.25 Y:-2.5 Z:3.75} Color:{R:1 G:2 B:3 A:4}}",
                positionColor.toString());

        assertRoundTrip(new VertexPositionColor[]{positionColor});
        assertRoundTrip(new VertexPositionTexture[]{positionTexture});
        assertRoundTrip(new VertexPositionColorTexture[]{positionColorTexture});
        assertRoundTrip(new VertexPositionNormalTexture[]{positionNormalTexture});
    }

    private static <T> void assertRoundTrip(T[] values) {
        VertexDataCodec codec = VertexDataCodec.select(values);
        byte[] encoded = codec.encode(values, 0, values.length);
        assertEquals(values.length * codec.stride(), encoded.length);
        assertArrayEquals(values, codec.<T>decode(encoded, values.length));
    }
}
