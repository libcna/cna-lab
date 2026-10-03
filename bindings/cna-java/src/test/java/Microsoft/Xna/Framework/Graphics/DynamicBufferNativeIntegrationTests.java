package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.openeggbert.cna.internal.CnaNativeException;

import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class DynamicBufferNativeIntegrationTests {

    @Test
    void dynamicBuffersForwardOptionsPreserveWindowsAndTearDownSubscriptions() {
        DynamicBufferGame game = new DynamicBufferGame();
        try {
            game.RunOneFrame();
            assertTrue(game.completed);
            assertFalse(game.liveVertex.getIsDisposed());
            assertFalse(game.liveIndex.getIsDisposed());

            game.close();
            assertTrue(game.liveVertex.getIsDisposed());
            assertTrue(game.liveIndex.getIsDisposed());
            game.close();
        } finally {
            game.close();
        }
    }

    private static final class DynamicBufferGame extends Game {
        private DynamicVertexBuffer liveVertex;
        private DynamicIndexBuffer liveIndex;
        private boolean completed;

        /**
         * HiDef, because what this game exercises -- occlusion queries, 32-bit indices -- is
         * HiDef-only in XNA, and CNA enforces the default Reach profile's limits.
         */
        private DynamicBufferGame() {
            new Microsoft.Xna.Framework.GraphicsDeviceManager(this)
                    .setGraphicsProfile(GraphicsProfile.HiDef);
        }

        @Override
        protected void Update(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();
            VertexPositionColor[] firstVertices = vertices(Color.Red, Color.Green, Color.Blue);
            VertexPositionColor[] secondVertices = vertices(Color.Yellow, Color.Cyan, Color.Magenta);

            DynamicVertexBuffer vertex = new DynamicVertexBuffer(
                    device, VertexPositionColor.class, 3, BufferUsage.None);
                AtomicInteger vertexLost = new AtomicInteger();
                var listener = (Microsoft.Xna.Framework.EventHandler<Microsoft.Xna.Framework.EventArgs>)
                        (sender, args) -> vertexLost.incrementAndGet();
                vertex.addContentLostListener(listener);
                vertex.removeContentLostListener(listener);
                vertex.addContentLostListener(listener);
                assertFalse(vertex.getIsContentLost());

                vertex.SetData(firstVertices, 0, 3, SetDataOptions.Discard);
                VertexPositionColor[] actual = new VertexPositionColor[3];
                vertex.GetData(actual);
                assertArrayEquals(firstVertices, actual);

                vertex.SetData(secondVertices, 0, 3, SetDataOptions.NoOverwrite);
                vertex.GetData(actual);
                assertArrayEquals(secondVertices, actual);

                VertexPositionColor replacement = new VertexPositionColor(
                        new Vector3(7.0f, 8.0f, 9.0f), Color.White);
                vertex.SetData(16, new VertexPositionColor[]{replacement},
                        0, 1, 16, SetDataOptions.None);
                vertex.GetData(actual);
                assertEquals(replacement, actual[1]);
                assertThrows(UnsupportedOperationException.class,
                        () -> vertex.SetData(0, firstVertices, 0, 1, 16,
                                SetDataOptions.Discard));
                assertThrows(CnaNativeException.class,
                        () -> vertex.SetData(firstVertices, 0, 3,
                                SetDataOptions.Discard.Or(SetDataOptions.NoOverwrite)));
                vertex.SetData(firstVertices, 0, 3, SetDataOptions.None);

                device.SetVertexBuffer(vertex);
                assertEquals(1, device.GetVertexBuffers().length);
                vertex.close();
                assertEquals(0, device.GetVertexBuffers().length);
                assertEquals(0, vertexLost.get());
                vertex.close();
                assertThrows(IllegalStateException.class, vertex::getIsContentLost);

            DynamicIndexBuffer index = new DynamicIndexBuffer(
                    device, short.class, 4, BufferUsage.None);
                AtomicInteger indexLost = new AtomicInteger();
                index.addContentLostListener((sender, args) -> indexLost.incrementAndGet());
                assertFalse(index.getIsContentLost());

                Short[] first = {(short)0, (short)1, (short)2, (short)3};
                Short[] second = {(short)3, (short)2, (short)1, (short)0};
                index.SetData(first, 0, first.length, SetDataOptions.Discard);
                Short[] actualIndices = new Short[4];
                index.GetData(actualIndices);
                assertArrayEquals(first, actualIndices);

                index.SetData(second, 0, second.length, SetDataOptions.NoOverwrite);
                index.GetData(actualIndices);
                assertArrayEquals(second, actualIndices);

                index.SetData(2, new Short[]{(short)9}, 0, 1, SetDataOptions.None);
                index.GetData(actualIndices);
                assertEquals((short)9, actualIndices[1]);
                assertThrows(UnsupportedOperationException.class,
                        () -> index.SetData(0, first, 0, 1, SetDataOptions.NoOverwrite));
                assertThrows(CnaNativeException.class,
                        () -> index.SetData(first, 0, first.length,
                                SetDataOptions.Discard.Or(SetDataOptions.NoOverwrite)));
                index.SetData(first, 0, first.length, SetDataOptions.None);

                device.setIndices(index);
                assertEquals(index, device.getIndices());
                index.close();
                assertNull(device.getIndices());
                assertEquals(0, indexLost.get());
                index.close();
                assertThrows(IllegalStateException.class, index::getIsContentLost);

            liveVertex = new DynamicVertexBuffer(
                    device, VertexPositionColor.VertexDeclaration, 3, BufferUsage.WriteOnly);
            liveIndex = new DynamicIndexBuffer(
                    device, IndexElementSize.ThirtyTwoBits, 3, BufferUsage.WriteOnly);
            liveVertex.addContentLostListener((sender, args) -> { });
            liveIndex.addContentLostListener((sender, args) -> { });
            completed = true;
        }

        private static VertexPositionColor[] vertices(Color first, Color second, Color third) {
            return new VertexPositionColor[]{
                    new VertexPositionColor(new Vector3(0.0f, 0.0f, 0.0f), first),
                    new VertexPositionColor(new Vector3(1.0f, 0.0f, 0.0f), second),
                    new VertexPositionColor(new Vector3(0.0f, 1.0f, 0.0f), third)
            };
        }
    }
}
