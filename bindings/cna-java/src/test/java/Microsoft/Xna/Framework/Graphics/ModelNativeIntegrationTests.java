package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class ModelNativeIntegrationTests {

    @Test
    void modelDrawUsesRealEffectsBuffersPassesAndStableSharedIdentity() {
        try (ModelGame game = new ModelGame()) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    private static final class ModelGame extends Game {
        private boolean completed;

        @Override
        protected void Update(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();
            VertexBuffer vertices = new VertexBuffer(
                    device, VertexPositionColor.class, 3, BufferUsage.None);
            IndexBuffer indices = new IndexBuffer(
                    device, IndexElementSize.SixteenBits, 3, BufferUsage.None);
            BasicEffect firstEffect = new BasicEffect(device);
            BasicEffect secondEffect = new BasicEffect(device);
            try {
                vertices.SetData(new VertexPositionColor[]{
                        new VertexPositionColor(new Vector3(-0.5f, -0.5f, 0.0f), Color.Red),
                        new VertexPositionColor(new Vector3(0.0f, 0.5f, 0.0f), Color.Green),
                        new VertexPositionColor(new Vector3(0.5f, -0.5f, 0.0f), Color.Blue)
                });
                indices.SetData(new Short[]{0, 1, 2});

                ModelBone root = new ModelBone("Root", Matrix.getIdentity(), 0);
                root.setParentAndChildren(null, new ModelBone[0]);
                ModelMeshPart firstPart = new ModelMeshPart(0, 3, 0, 1, null);
                ModelMeshPart secondPart = new ModelMeshPart(0, 3, 0, 1, null);
                firstPart.setBuffers(vertices, indices);
                secondPart.setBuffers(vertices, indices);
                ModelMesh mesh = new ModelMesh(
                        "Triangle", root, new BoundingSphere(Vector3.getZero(), 1.0f),
                        new ModelMeshPart[]{firstPart, secondPart}, null);
                firstPart.setEffect(firstEffect);
                secondPart.setEffect(firstEffect);
                Model model = new Model(
                        new ModelBone[]{root}, new ModelMesh[]{mesh}, root, null);

                assertSame(vertices, firstPart.getVertexBuffer());
                assertSame(vertices, secondPart.getVertexBuffer());
                assertSame(indices, firstPart.getIndexBuffer());
                assertSame(indices, secondPart.getIndexBuffer());
                assertEquals(1, mesh.getEffects().size());
                assertSame(firstEffect, mesh.getEffects().get(0));

                secondPart.setEffect(secondEffect);
                assertEquals(2, mesh.getEffects().size());
                firstPart.setEffect(secondEffect);
                assertEquals(1, mesh.getEffects().size());
                assertSame(secondEffect, mesh.getEffects().get(0));
                assertDoesNotThrow(() -> model.Draw(
                        Matrix.CreateTranslation(1.0f, 2.0f, 3.0f),
                        Matrix.getIdentity(), Matrix.getIdentity()));
                assertEquals(1.0f, secondEffect.getWorld().M41);
                assertEquals(2.0f, secondEffect.getWorld().M42);
                assertEquals(3.0f, secondEffect.getWorld().M43);

                secondEffect.close();
                assertThrows(IllegalStateException.class, mesh::Draw);
                device.SetVertexBuffer(null);
                device.setIndices(null);
            } finally {
                device.SetVertexBuffer(null);
                device.setIndices(null);
                firstEffect.close();
                secondEffect.close();
                indices.close();
                vertices.close();
            }
            completed = true;
        }
    }
}
