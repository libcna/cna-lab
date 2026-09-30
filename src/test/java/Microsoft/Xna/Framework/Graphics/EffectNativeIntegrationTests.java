package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.openeggbert.cna.internal.CnaNativeException;

import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class EffectNativeIntegrationTests {

    @Test
    void effectGraphUsesStableBorrowedViewsAndRealPassExecution() {
        try (EffectGame game = new EffectGame()) {
            game.RunOneFrame();
            assertTrue(game.completed);
            assertTrue(game.crossDeviceRejected);
        }
    }

    @Test
    void basicEffectUsesExecutableStockRoutesAndParentOwnedLights() {
        try (BasicEffectGame game = new BasicEffectGame()) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    @Test
    void remainingStockEffectsUseExecutableNativeStateAndSafeOwnership() {
        try (StockEffectsGame game = new StockEffectsGame()) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    private static final class EffectGame extends Game {
        private Effect effect;
        private SpriteBatch batch;
        private boolean completed;
        private boolean crossDeviceRejected;

        @Override
        protected void LoadContent() {
            effect = new Effect(getGraphicsDevice(), true);
            batch = new SpriteBatch(getGraphicsDevice());
        }

        @Override
        protected void Update(GameTime gameTime) {
            EffectParameterCollection parameters = effect.getParameters();
            assertSame(parameters, effect.getParameters());
            assertEquals(0, parameters.getCount());
            assertNull(parameters.get(-1));
            assertNull(parameters.get(0));
            assertNull(parameters.get("missing"));
            assertNull(parameters.GetParameterBySemantic("MISSING"));

            EffectTechniqueCollection techniques = effect.getTechniques();
            assertSame(techniques, effect.getTechniques());
            assertEquals(1, techniques.getCount());
            EffectTechnique technique = techniques.get(0);
            assertSame(technique, techniques.get(0));
            assertSame(technique, techniques.get("Default"));
            assertNull(techniques.get(-1));
            assertNull(techniques.get(1));
            assertNull(techniques.get("missing"));
            assertSame(technique, effect.getCurrentTechnique());
            assertSame(technique, effect.getCurrentTechnique());

            assertSame(technique.getPasses(), technique.getPasses());
            assertSame(technique.getAnnotations(), technique.getAnnotations());
            assertEquals(0, technique.getAnnotations().getCount());
            EffectPassCollection passes = technique.getPasses();
            assertEquals(1, passes.getCount());
            EffectPass pass = passes.get(0);
            assertSame(pass, passes.get(0));
            assertSame(pass, passes.get("P0"));
            assertEquals("P0", pass.getName());
            assertSame(pass.getAnnotations(), pass.getAnnotations());
            assertEquals(0, pass.getAnnotations().getCount());
            assertDoesNotThrow(pass::Apply);

            try (Effect clone = effect.Clone()) {
                assertNotSame(effect, clone);
                assertNotSame(effect.getParameters(), clone.getParameters());
                assertNotSame(effect.getTechniques(), clone.getTechniques());
                assertNotSame(effect.getCurrentTechnique(), clone.getCurrentTechnique());
                assertEquals("Default", clone.getCurrentTechnique().getName());
                assertThrows(IllegalArgumentException.class,
                        () -> effect.setCurrentTechnique(clone.getCurrentTechnique()));
                assertSame(technique, effect.getCurrentTechnique());
            }

            AtomicInteger disposing = new AtomicInteger();
            Effect parent = new Effect(getGraphicsDevice(), true);
            parent.addDisposingListener((sender, args) -> disposing.incrementAndGet());
            EffectTechnique child = parent.getCurrentTechnique();
            EffectPassCollection childPasses = child.getPasses();
            EffectPass childPass = childPasses.get(0);
            EffectAnnotationCollection childAnnotations = childPass.getAnnotations();
            parent.close();
            parent.close();
            assertEquals(1, disposing.get());
            assertThrows(IllegalStateException.class, child::getName);
            assertThrows(IllegalStateException.class, childPasses::getCount);
            assertThrows(IllegalStateException.class, childPass::Apply);
            assertThrows(IllegalStateException.class, childAnnotations::getCount);

            assertThrows(IllegalStateException.class, batch::End);
            batch.Begin(SpriteSortMode.Deferred, null, null, null, null, null);
            assertThrows(IllegalStateException.class, batch::Begin);
            batch.End();

            Matrix transform = Matrix.CreateTranslation(3.0f, 5.0f, 0.0f);
            batch.Begin(SpriteSortMode.Deferred, null, null, null, null, effect, transform);
            transform.M41 = 900.0f;
            transform.M42 = 901.0f;
            batch.End();

            // CNA ABI 0.9.0 stopped refusing a non-finite transform component and now
            // carries it into the vertex path, which is what XNA does: the reference
            // SpriteBatch validates none of its float inputs. Java therefore must not
            // reintroduce a rejection the original API never had.
            Matrix nonFinite = Matrix.getIdentity();
            nonFinite.M11 = Float.NaN;
            assertDoesNotThrow(() -> {
                batch.Begin(SpriteSortMode.Deferred, null, null, null, null, effect, nonFinite);
                batch.End();
            });
            assertDoesNotThrow(() -> {
                batch.Begin(SpriteSortMode.Deferred, null, null, null, null, effect);
                batch.End();
            });
            assertThrows(NullPointerException.class,
                    () -> batch.Begin(null, null, null, null, null, effect));

            Effect disposed = effect.Clone();
            disposed.close();
            assertThrows(IllegalStateException.class,
                    () -> batch.Begin(
                            SpriteSortMode.Deferred, null, null, null, null, disposed));
            assertDoesNotThrow(() -> {
                batch.Begin();
                batch.End();
            });

            GraphicsDevice otherDevice = new GraphicsDevice(this);
            try (Effect foreignEffect = new Effect(otherDevice, true)) {
                assertThrows(IllegalArgumentException.class,
                        () -> batch.Begin(
                                SpriteSortMode.Deferred, null, null, null, null,
                                foreignEffect));
                assertDoesNotThrow(() -> {
                    batch.Begin();
                    batch.End();
                });
                crossDeviceRejected = true;
            } finally {
                otherDevice.close();
            }
            completed = true;
        }
    }

    private static final class BasicEffectGame extends Game {
        private boolean completed;

        @Override
        protected void Update(GameTime gameTime) {
            BasicEffect effect = new BasicEffect(getGraphicsDevice());
            try (Texture2D texture = new Texture2D(getGraphicsDevice(), 1, 1)) {
                assertEquals(1.0f, effect.getAlpha());
                assertVector(new Vector3(1.0f), effect.getDiffuseColor());
                assertVector(Vector3.getZero(), effect.getEmissiveColor());
                assertVector(new Vector3(1.0f), effect.getSpecularColor());
                assertEquals(16.0f, effect.getSpecularPower());
                assertNull(effect.getTexture());
                assertTrue(!effect.getTextureEnabled());
                assertTrue(!effect.getVertexColorEnabled());
                assertTrue(!effect.getLightingEnabled());
                assertTrue(!effect.getFogEnabled());

                Matrix world = Matrix.CreateTranslation(2.0f, 3.0f, 4.0f);
                effect.setWorld(world);
                world.M41 = 99.0f;
                assertEquals(2.0f, effect.getWorld().M41);
                effect.setView(Matrix.CreateTranslation(5.0f, 6.0f, 7.0f));
                effect.setProjection(Matrix.CreateScale(2.0f));
                assertEquals(5.0f, effect.getView().M41);
                assertEquals(2.0f, effect.getProjection().M11);

                Vector3 diffuse = new Vector3(0.2f, 0.3f, 0.4f);
                effect.setDiffuseColor(diffuse);
                diffuse.X = 9.0f;
                assertVector(new Vector3(0.2f, 0.3f, 0.4f), effect.getDiffuseColor());
                effect.setEmissiveColor(new Vector3(0.1f, 0.15f, 0.2f));
                effect.setSpecularColor(new Vector3(0.6f, 0.7f, 0.8f));
                effect.setSpecularPower(24.0f);
                effect.setAlpha(0.75f);
                effect.setFogColor(new Vector3(0.4f, 0.5f, 0.6f));
                effect.setFogStart(10.0f);
                effect.setFogEnd(100.0f);
                effect.setFogEnabled(true);
                effect.setVertexColorEnabled(true);
                effect.setPreferPerPixelLighting(true);
                effect.setTexture(texture);
                effect.setTextureEnabled(true);
                assertSame(texture, effect.getTexture());

                DirectionalLight light0 = effect.getDirectionalLight0();
                assertSame(light0, effect.getDirectionalLight0());
                assertSame(effect.getDirectionalLight1(), effect.getDirectionalLight1());
                assertSame(effect.getDirectionalLight2(), effect.getDirectionalLight2());
                Vector3 direction = new Vector3(1.0f, -2.0f, 3.0f);
                light0.setDirection(direction);
                direction.Y = 20.0f;
                assertVector(new Vector3(1.0f, -2.0f, 3.0f), light0.getDirection());
                light0.setDiffuseColor(new Vector3(0.9f, 0.8f, 0.7f));
                light0.setSpecularColor(new Vector3(0.5f, 0.4f, 0.3f));
                light0.setEnabled(true);
                assertTrue(light0.getEnabled());

                effect.EnableDefaultLighting();
                assertTrue(effect.getLightingEnabled());
                assertTrue(effect.getDirectionalLight0().getEnabled());
                assertDoesNotThrow(() -> effect.getCurrentTechnique().getPasses().get(0).Apply());

                try (BasicEffect clone = (BasicEffect)effect.Clone()) {
                    assertNotSame(effect, clone);
                    assertSame(texture, clone.getTexture());
                    assertEquals(effect.getAlpha(), clone.getAlpha());
                    assertVector(effect.getDiffuseColor(), clone.getDiffuseColor());
                    assertVector(effect.getDirectionalLight0().getDirection(),
                            clone.getDirectionalLight0().getDirection());
                    assertNotSame(effect.getDirectionalLight0(), clone.getDirectionalLight0());
                }

                GraphicsDevice other = new GraphicsDevice(this);
                try (Texture2D foreign = new Texture2D(other, 1, 1)) {
                    assertThrows(IllegalArgumentException.class, () -> effect.setTexture(foreign));
                    assertSame(texture, effect.getTexture());
                } finally {
                    other.close();
                }

                effect.setTexture(null);
                assertNull(effect.getTexture());
                effect.close();
                effect.close();
                assertThrows(IllegalStateException.class, light0::getEnabled);
                assertThrows(IllegalStateException.class, effect::getWorld);
            } finally {
                effect.close();
            }
            completed = true;
        }
    }

    private static final class StockEffectsGame extends Game {
        private boolean completed;

        /**
         * HiDef, because what this game exercises -- occlusion queries, 32-bit indices -- is
         * HiDef-only in XNA, and CNA enforces the default Reach profile's limits.
         */
        private StockEffectsGame() {
            new Microsoft.Xna.Framework.GraphicsDeviceManager(this)
                    .setGraphicsProfile(GraphicsProfile.HiDef);
        }

        @Override
        protected void Update(GameTime gameTime) {
            try (Texture2D texture = new Texture2D(getGraphicsDevice(), 1, 1);
                 Texture2D texture2 = new Texture2D(getGraphicsDevice(), 1, 1);
                 TextureCube cube = new TextureCube(
                         getGraphicsDevice(), 1, false, SurfaceFormat.Color)) {
                verifyAlphaTest(texture);
                verifyDualTexture(texture, texture2);
                verifyEnvironmentMap(texture, cube);
                verifySkinned(texture);
                verifyEffectMaterial();
                verifyOcclusionNative();
                verifyWrongDeviceTextures(texture);
            }
            completed = true;
        }

        private void verifyAlphaTest(Texture2D texture) {
            AlphaTestEffect effect = new AlphaTestEffect(getGraphicsDevice());
            assertEquals(1.0f, effect.getAlpha());
            assertVector(new Vector3(1.0f), effect.getDiffuseColor());
            assertEquals(CompareFunction.Greater, effect.getAlphaFunction());
            assertEquals(0, effect.getReferenceAlpha());
            assertTrue(!effect.getVertexColorEnabled());
            assertCommonFogAndMatrices(effect);

            effect.setAlpha(0.4f);
            effect.setDiffuseColor(new Vector3(0.2f, 0.3f, 0.4f));
            effect.setAlphaFunction(CompareFunction.LessEqual);
            effect.setReferenceAlpha(123);
            effect.setVertexColorEnabled(true);
            effect.setTexture(texture);
            assertSame(texture, effect.getTexture());
            assertDoesNotThrow(() -> effect.getCurrentTechnique().getPasses().get(0).Apply());
            try (AlphaTestEffect clone = (AlphaTestEffect)effect.Clone()) {
                assertNotSame(effect, clone);
                assertSame(texture, clone.getTexture());
                assertEquals(0.4f, clone.getAlpha());
                assertEquals(CompareFunction.LessEqual, clone.getAlphaFunction());
                assertEquals(123, clone.getReferenceAlpha());
                assertTrue(clone.getVertexColorEnabled());
            }
            effect.close();
            effect.close();
            assertThrows(IllegalStateException.class, effect::getAlpha);
        }

        private void verifyDualTexture(Texture2D texture, Texture2D texture2) {
            DualTextureEffect effect = new DualTextureEffect(getGraphicsDevice());
            assertEquals(1.0f, effect.getAlpha());
            assertVector(new Vector3(1.0f), effect.getDiffuseColor());
            assertNull(effect.getTexture());
            assertNull(effect.getTexture2());
            assertTrue(!effect.getVertexColorEnabled());
            assertCommonFogAndMatrices(effect);

            effect.setTexture(texture);
            effect.setTexture2(texture2);
            effect.setAlpha(0.6f);
            effect.setDiffuseColor(new Vector3(0.8f, 0.7f, 0.6f));
            effect.setVertexColorEnabled(true);
            assertDoesNotThrow(() -> effect.getCurrentTechnique().getPasses().get(0).Apply());
            try (DualTextureEffect clone = (DualTextureEffect)effect.Clone()) {
                assertSame(texture, clone.getTexture());
                assertSame(texture2, clone.getTexture2());
                assertEquals(0.6f, clone.getAlpha());
                assertTrue(clone.getVertexColorEnabled());
            }
            effect.close();
            assertThrows(IllegalStateException.class, effect::getTexture2);
        }

        private void verifyEnvironmentMap(Texture2D texture, TextureCube cube) {
            EnvironmentMapEffect effect = new EnvironmentMapEffect(getGraphicsDevice());
            assertEquals(1.0f, effect.getAlpha());
            assertEquals(1.0f, effect.getEnvironmentMapAmount());
            assertEquals(1.0f, effect.getFresnelFactor());
            assertVector(new Vector3(1.0f), effect.getDiffuseColor());
            assertVector(Vector3.getZero(), effect.getEmissiveColor());
            assertVector(Vector3.getZero(), effect.getEnvironmentMapSpecular());
            assertTrue(effect.getLightingEnabled());
            assertThrows(UnsupportedOperationException.class,
                    () -> effect.setLightingEnabled(false));
            assertCommonFogAndMatrices(effect);

            effect.setTexture(texture);
            effect.setEnvironmentMap(cube);
            effect.setEnvironmentMapAmount(0.5f);
            effect.setEnvironmentMapSpecular(new Vector3(0.1f, 0.2f, 0.3f));
            effect.setFresnelFactor(0.75f);
            effect.setEmissiveColor(new Vector3(0.3f, 0.2f, 0.1f));
            DirectionalLight light = effect.getDirectionalLight0();
            assertSame(light, effect.getDirectionalLight0());
            assertSame(effect.getDirectionalLight1(), effect.getDirectionalLight1());
            assertSame(effect.getDirectionalLight2(), effect.getDirectionalLight2());
            effect.EnableDefaultLighting();
            assertDoesNotThrow(() -> effect.getCurrentTechnique().getPasses().get(0).Apply());
            try (EnvironmentMapEffect clone = (EnvironmentMapEffect)effect.Clone()) {
                assertSame(texture, clone.getTexture());
                assertSame(cube, clone.getEnvironmentMap());
                assertEquals(0.5f, clone.getEnvironmentMapAmount());
                assertVector(effect.getEnvironmentMapSpecular(), clone.getEnvironmentMapSpecular());
                assertNotSame(light, clone.getDirectionalLight0());
            }
            effect.close();
            assertThrows(IllegalStateException.class, light::getEnabled);
        }

        private void verifySkinned(Texture2D texture) {
            SkinnedEffect effect = new SkinnedEffect(getGraphicsDevice());
            assertEquals(72, SkinnedEffect.MaxBones);
            assertEquals(1.0f, effect.getAlpha());
            assertEquals(16.0f, effect.getSpecularPower());
            assertEquals(4, effect.getWeightsPerVertex());
            assertVector(new Vector3(1.0f), effect.getDiffuseColor());
            assertVector(Vector3.getZero(), effect.getEmissiveColor());
            assertVector(new Vector3(1.0f), effect.getSpecularColor());
            assertTrue(effect.getLightingEnabled());
            assertTrue(!effect.getPreferPerPixelLighting());
            assertThrows(UnsupportedOperationException.class,
                    () -> effect.setLightingEnabled(false));
            assertCommonFogAndMatrices(effect);

            Matrix bone = Matrix.CreateTranslation(2.0f, 3.0f, 4.0f);
            Matrix[] input = {bone};
            effect.SetBoneTransforms(input);
            bone.M41 = 99.0f;
            input[0] = Matrix.getIdentity();
            Matrix[] first = effect.GetBoneTransforms(1);
            assertEquals(2.0f, first[0].M41);
            first[0].M41 = 77.0f;
            assertEquals(2.0f, effect.GetBoneTransforms(1)[0].M41);
            assertThrows(NullPointerException.class, () -> effect.SetBoneTransforms(null));
            assertThrows(IllegalArgumentException.class,
                    () -> effect.SetBoneTransforms(new Matrix[0]));
            assertThrows(IllegalArgumentException.class,
                    () -> effect.SetBoneTransforms(new Matrix[SkinnedEffect.MaxBones + 1]));
            assertThrows(NullPointerException.class,
                    () -> effect.SetBoneTransforms(new Matrix[]{null}));
            assertThrows(IllegalArgumentException.class, () -> effect.GetBoneTransforms(0));
            assertThrows(IllegalArgumentException.class,
                    () -> effect.GetBoneTransforms(SkinnedEffect.MaxBones + 1));
            assertThrows(IllegalArgumentException.class, () -> effect.setWeightsPerVertex(3));

            effect.setWeightsPerVertex(2);
            effect.setTexture(texture);
            effect.setPreferPerPixelLighting(true);
            effect.EnableDefaultLighting();
            DirectionalLight light = effect.getDirectionalLight0();
            assertSame(light, effect.getDirectionalLight0());
            assertDoesNotThrow(() -> effect.getCurrentTechnique().getPasses().get(0).Apply());
            try (SkinnedEffect clone = (SkinnedEffect)effect.Clone()) {
                assertSame(texture, clone.getTexture());
                assertEquals(2, clone.getWeightsPerVertex());
                assertEquals(2.0f, clone.GetBoneTransforms(1)[0].M41);
                Matrix[] cloneBones = clone.GetBoneTransforms(1);
                cloneBones[0].M41 = -5.0f;
                assertEquals(2.0f, clone.GetBoneTransforms(1)[0].M41);
            }
            effect.close();
            effect.close();
            assertThrows(IllegalStateException.class, effect::getWorld);
            assertThrows(IllegalStateException.class, light::getEnabled);
        }

        private void verifyWrongDeviceTextures(Texture2D retained) {
            GraphicsDevice other = new GraphicsDevice(this);
            try (Texture2D foreign2D = new Texture2D(other, 1, 1);
                 TextureCube foreignCube = new TextureCube(other, 1, false, SurfaceFormat.Color);
                 AlphaTestEffect alpha = new AlphaTestEffect(getGraphicsDevice());
                 DualTextureEffect dual = new DualTextureEffect(getGraphicsDevice());
                 EnvironmentMapEffect environment = new EnvironmentMapEffect(getGraphicsDevice());
                 SkinnedEffect skinned = new SkinnedEffect(getGraphicsDevice())) {
                alpha.setTexture(retained);
                dual.setTexture(retained);
                dual.setTexture2(retained);
                environment.setTexture(retained);
                skinned.setTexture(retained);
                assertThrows(IllegalArgumentException.class, () -> alpha.setTexture(foreign2D));
                assertThrows(IllegalArgumentException.class, () -> dual.setTexture(foreign2D));
                assertThrows(IllegalArgumentException.class, () -> dual.setTexture2(foreign2D));
                assertThrows(IllegalArgumentException.class,
                        () -> environment.setTexture(foreign2D));
                assertThrows(IllegalArgumentException.class,
                        () -> environment.setEnvironmentMap(foreignCube));
                assertThrows(IllegalArgumentException.class, () -> skinned.setTexture(foreign2D));
                assertSame(retained, alpha.getTexture());
                assertSame(retained, dual.getTexture());
                assertSame(retained, dual.getTexture2());
                assertSame(retained, environment.getTexture());
                assertNull(environment.getEnvironmentMap());
                assertSame(retained, skinned.getTexture());
            } finally {
                other.close();
            }
        }

        private void verifyEffectMaterial() {
            BasicEffect source = new BasicEffect(getGraphicsDevice());
            source.setAlpha(0.35f);
            EffectMaterial material = new EffectMaterial(source);
            assertSame(getGraphicsDevice(), material.getGraphicsDevice());
            assertNotSame(source.getCurrentTechnique(), material.getCurrentTechnique());
            assertDoesNotThrow(() -> material.getCurrentTechnique().getPasses().get(0).Apply());
            source.close();
            assertDoesNotThrow(() -> material.getCurrentTechnique().getPasses().get(0).Apply());
            try (Effect clone = material.Clone()) {
                assertEquals(Effect.class, clone.getClass());
                assertSame(getGraphicsDevice(), clone.getGraphicsDevice());
                assertDoesNotThrow(() -> clone.getCurrentTechnique().getPasses().get(0).Apply());
            }
            material.close();
            material.close();
            assertThrows(IllegalStateException.class, material::getCurrentTechnique);
            assertThrows(NullPointerException.class, () -> new EffectMaterial(null));
        }

        private void verifyOcclusionNative() {
            OcclusionQuery query = new OcclusionQuery(getGraphicsDevice());
            assertTrue(!query.getIsComplete());
            query.Begin();
            query.End();
            // An occlusion query is answered by the GPU, not by the call that ended it. A
            // renderer with no GPU answers instantly; a real one answers when it gets to the
            // query; and a CPU rasterizer that claims the capability may never answer at all.
            // All three are qualified: the query is waited for rather than assumed, and where it
            // never arrives the contract that a pixel count needs a completed query is what is
            // asserted instead of a number nobody measured.
            long deadline = System.nanoTime() + 2_000_000_000L;
            while (!query.getIsComplete() && System.nanoTime() < deadline) {
                Thread.onSpinWait();
            }
            // Whether it completed or not, the count is never negative -- a renderer that
            // never answers reports nothing rather than rubbish, which is the only claim about
            // it worth making here.
            assertTrue(query.getPixelCount() >= 0,
                    "a pixel count is a count, complete or not: " + query.getPixelCount());
            query.close();
            query.close();
            assertThrows(IllegalStateException.class, query::getIsComplete);
        }

        private static void assertCommonFogAndMatrices(IEffectFog fog) {
            assertTrue(!fog.getFogEnabled());
            assertEquals(0.0f, fog.getFogStart());
            assertEquals(1.0f, fog.getFogEnd());
            assertVector(Vector3.getZero(), fog.getFogColor());
            IEffectMatrices matrices = (IEffectMatrices)fog;
            assertEquals(1.0f, matrices.getWorld().M11);
            assertEquals(1.0f, matrices.getView().M22);
            assertEquals(1.0f, matrices.getProjection().M33);
            Matrix world = Matrix.CreateTranslation(8.0f, 9.0f, 10.0f);
            matrices.setWorld(world);
            world.M41 = -1.0f;
            assertEquals(8.0f, matrices.getWorld().M41);
        }
    }

    private static void assertVector(Vector3 expected, Vector3 actual) {
        assertEquals(expected.X, actual.X, 0.000001f);
        assertEquals(expected.Y, actual.Y, 0.000001f);
        assertEquals(expected.Z, actual.Z, 0.000001f);
    }
}
