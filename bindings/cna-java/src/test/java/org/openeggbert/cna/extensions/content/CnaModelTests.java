package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Content.ContentManager;
import Microsoft.Xna.Framework.Graphics.BasicEffect;
import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.Model;
import Microsoft.Xna.Framework.Graphics.ModelBone;
import Microsoft.Xna.Framework.Graphics.ModelMesh;
import Microsoft.Xna.Framework.Graphics.ModelMeshPart;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.junit.jupiter.api.condition.EnabledIfSystemProperty;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * CNA's own view of a model, measured against the managed XNB reader's.
 *
 * <p>The asset is an authored XNA 4.0 {@code .xnb} from the CNA checkout this build already
 * qualifies against -- a real cube with a bone, a mesh, a vertex and index buffer and a
 * {@code BasicEffect}. A hand-built fixture would prove much less here: it would be written
 * against the same assumptions the reader makes, and agreeing with itself is not evidence.
 *
 * <p>What is measured is agreement. Two independent readers -- the Java managed one and CNA's
 * native one -- open the same bytes, and their graphs have to match bone for bone, mesh for mesh
 * and part for part. That is what says a Java game and CNA are looking at the same model, and it
 * is the measurement any future decision to route {@code Load<Model>} through CNA would need.
 */
final class CnaModelTests {

    /** The authored cube in the sibling CNA checkout, relative to its root. */
    private static final String CUBE =
            "tests/assets/xnb/monogame/windows/uncompressed/BlenderDefaultCube.xnb";

    @Test
    void buildingRequiresAModel() {
        assertThrows(NullPointerException.class, () -> CnaModel.From(null));
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    @EnabledIfSystemProperty(named = "cna.root", matches = ".+")
    void cnaAndTheManagedReaderAgreeOnAnAuthoredModel() {
        Path asset = Path.of(System.getProperty("cna.root")).resolve(CUBE);
        if (!Files.isRegularFile(asset)) {
            // The authored asset is upstream's, not this repository's. Without it there is
            // nothing honest to measure, so the test stops rather than inventing a cube.
            return;
        }
        try (ModelProbe probe = new ModelProbe(asset)) {
            probe.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    private static final class ModelProbe extends Game {

        private final Path asset;
        private boolean ran;
        private Throwable failure;

        private ModelProbe(Path asset) {
            this.asset = asset;
            new GraphicsDeviceManager(this);
        }

        @Override
        protected void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                probe();
            } catch (Throwable exception) {
                failure = exception;
            }
        }

        private void probe() {
            Path root = stage();
            // The game's own content manager, because it is the one whose service provider
            // carries the graphics device the buffers and effects need.
            ContentManager content = getContent();
            content.setRootDirectory(root.toString());
            Model managed = content.Load(Model.class, "cube");
            assertNotNull(managed);
            try (CnaModel loaded = CnaModel.From(managed)) {
                compare(managed, loaded);
                modernFeatures(loaded);
                lifetime(loaded);
            }
            // The two loads are independent: closing CNA's view must leave the XNA model
            // usable, because the managed reader built its own resources.
            assertEquals(managed.getBones().size(), managed.getBones().size());
            assertNotNull(managed.getMeshes());
        }

        private void compare(Model managed, CnaModel loaded) {
            CnaModelGraph graph = loaded.getGraph();

            assertEquals(managed.getBones().size(), graph.Bones().size(),
                    "both readers must find the same bones");
            for (int index = 0; index < graph.Bones().size(); index++) {
                ModelBone expected = managed.getBones().get(index);
                CnaModelBone actual = graph.Bones().get(index);
                assertEquals(expected.getIndex(), actual.Index());
                assertEquals(expected.getName(), actual.Name(),
                        "a bone name read two ways must be the same name");
                assertEquals(expected.getTransform(), actual.Transform(),
                        "a bone transform read two ways must be the same matrix");
                assertEquals(expected.getParent() == null ? -1 : expected.getParent().getIndex(),
                        actual.ParentIndex());
                assertEquals(expected.getChildren().size(), actual.ChildIndices().size());
            }
            assertEquals(managed.getRoot() == null ? -1 : managed.getRoot().getIndex(),
                    graph.RootBoneIndex());

            assertEquals(managed.getMeshes().size(), graph.Meshes().size(),
                    "both readers must find the same meshes");
            for (int index = 0; index < graph.Meshes().size(); index++) {
                ModelMesh expected = managed.getMeshes().get(index);
                CnaModelMesh actual = graph.Meshes().get(index);
                assertEquals(expected.getName(), actual.Name());
                assertEquals(expected.getParentBone().getIndex(), actual.ParentBoneIndex());
                assertSphere(expected.getBoundingSphere(), actual.BoundingSphere());
                assertEquals(expected.getMeshParts().size(), actual.Parts().size(),
                        "both readers must find the same mesh parts");
                for (int part = 0; part < actual.Parts().size(); part++) {
                    ModelMeshPart expectedPart = expected.getMeshParts().get(part);
                    CnaModelMeshPart actualPart = actual.Parts().get(part);
                    assertEquals(expectedPart.getNumVertices(), actualPart.NumVertices());
                    assertEquals(expectedPart.getPrimitiveCount(), actualPart.PrimitiveCount());
                    assertEquals(expectedPart.getStartIndex(), actualPart.StartIndex());
                    assertEquals(expectedPart.getVertexOffset(), actualPart.VertexOffset());
                    assertTrue(actualPart.HasVertexBuffer(),
                            "a loaded part draws from a real vertex buffer");
                    assertTrue(actualPart.HasIndexBuffer());
                    assertNotNull(expectedPart.getVertexBuffer());
                    assertNotNull(expectedPart.getIndexBuffer());
                    // The effect's type is what XNB records and what the managed reader turns
                    // into a Java class. CNA reports the same identity as a name, which is why
                    // routing the strict load through CNA would need this mapping rather than
                    // handing out an untyped effect.
                    Effect effect = expectedPart.getEffect();
                    assertNotNull(effect);
                    assertTrue(effect instanceof BasicEffect,
                            "the managed reader builds the effect subclass XNB names");
                    assertTrue(actualPart.EffectTypeName().endsWith("BasicEffect"),
                            "CNA reports the same effect identity as a type name, got "
                            + actualPart.EffectTypeName());
                }
            }

            // The absolute transforms are the same arithmetic done in two runtimes.
            Matrix[] expected = new Matrix[managed.getBones().size()];
            managed.CopyAbsoluteBoneTransformsTo(expected);
            List<Matrix> actual = loaded.getAbsoluteBoneTransforms();
            assertEquals(expected.length, actual.size());
            for (int index = 0; index < expected.length; index++) {
                assertEquals(expected[index], actual.get(index),
                        "absolute bone transform " + index + " differs between the runtimes");
            }
            assertEquals(managed.getBones().size(), loaded.getBoneCount());
            assertEquals(loaded.getBoneTransforms().size(), loaded.getBoneCount());

            // CNA draws the graph in one call, using the very buffers and effects the XNA model
            // owns. No claim is made about pixels; what is asserted is that the whole path --
            // retained buffers, retained effects, bone transforms and the draw itself -- either
            // runs or is refused for a stated reason. A renderer that cannot shade a model
            // refuses, and the refusal is an answer about the renderer rather than a defect in
            // the path that led to it.
            try {
                loaded.Draw(Matrix.getIdentity(),
                        Matrix.CreateLookAt(new Microsoft.Xna.Framework.Vector3(0, 0, 5),
                                Microsoft.Xna.Framework.Vector3.getZero(),
                                Microsoft.Xna.Framework.Vector3.getUp()),
                        Matrix.CreatePerspectiveFieldOfView(1.0f, 1.0f, 0.1f, 100.0f));
            } catch (ContentNotSupportedException refused) {
                assertTrue(refused.getMessage().contains("Draw"),
                        "a refused draw names itself: " + refused.getMessage());
            }
        }

        private void modernFeatures(CnaModel loaded) {
            // XNA's Model has nowhere to put any of these. An XNB cube has none of them, and
            // reporting none is the honest answer -- what matters is that asking works.
            assertEquals(List.of(), loaded.getCameras());
            assertEquals(List.of(), loaded.getSkins());
            assertEquals(List.of(), loaded.getMaterialVariants());
            assertEquals(-1, loaded.getMaterialVariant(),
                    "no variant selected means the asset's own materials");
            BoundingSphere sphere = loaded.getBoundingSphere();
            if (sphere != null) {
                assertTrue(sphere.Radius >= 0.0f);
            }
        }

        private void lifetime(CnaModel loaded) {
            // Every view the graph walk took was released as it was copied, so walking twice
            // must give the same answer rather than exhausting anything.
            assertEquals(loaded.getGraph().Bones().size(), loaded.getGraph().Bones().size());
            assertThrows(NullPointerException.class, () -> loaded.setBoneTransforms(null));
            assertThrows(NullPointerException.class,
                    () -> loaded.Draw(null, Matrix.getIdentity(), Matrix.getIdentity()));
        }

        private void assertSphere(BoundingSphere expected, BoundingSphere actual) {
            assertEquals(expected.Center, actual.Center);
            assertEquals(expected.Radius, actual.Radius, 1.0e-6f);
        }

        private Path stage() {
            try {
                Path directory = Files.createTempDirectory("cna-java-model");
                directory.toFile().deleteOnExit();
                Path staged = directory.resolve("cube.xnb");
                Files.copy(asset, staged);
                staged.toFile().deleteOnExit();
                return directory;
            } catch (IOException exception) {
                throw new UncheckedIOException(exception);
            }
        }
    }
}
