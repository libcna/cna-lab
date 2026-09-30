package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The physically-based effect, against the live runtime.
 *
 * <p><strong>What this can and cannot say.</strong> VERIFIED_HEADLESS_GAME. Nothing here is about a
 * shaded pixel -- this renderer shades none -- and everything is about what the effect knows: what
 * each of its two dozen values reads back as. CNA ABI 0.30 removed the whole-material routes, so
 * every value is checked against its own getter.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class PbrEffectTests {

    @Test
    void everyScalarAndFlagReadsBackAsItself() {
        GameProbe.run(probe -> {
            try (PbrEffect effect = new PbrEffect(probe.device())) {
                // Every value distinct, so a getter reading its neighbour's parameter fails
                // rather than agreeing by accident.
                effect.setAlpha(0.25f);
                effect.setAlphaCutoff(0.375f);
                effect.setMetallicFactor(0.5f);
                effect.setRoughnessFactor(0.625f);
                effect.setNormalScale(0.75f);
                effect.setOcclusionStrength(0.875f);
                effect.setIor(1.75f);
                effect.setSpecularFactor(0.125f);

                assertEquals(0.25f, effect.getAlpha(), 1.0e-6f, "alpha");
                assertEquals(0.375f, effect.getAlphaCutoff(), 1.0e-6f, "alpha cutoff");
                assertEquals(0.5f, effect.getMetallicFactor(), 1.0e-6f, "metallic");
                assertEquals(0.625f, effect.getRoughnessFactor(), 1.0e-6f, "roughness");
                assertEquals(0.75f, effect.getNormalScale(), 1.0e-6f, "normal scale");
                assertEquals(0.875f, effect.getOcclusionStrength(), 1.0e-6f, "occlusion");
                assertEquals(1.75f, effect.getIor(), 1.0e-6f, "ior");
                assertEquals(0.125f, effect.getSpecularFactor(), 1.0e-6f, "specular");

                effect.setDoubleSided(true);
                effect.setOutputEncodedToSrgb(true);
                effect.setVertexColorEnabled(true);
                assertTrue(effect.isDoubleSided());
                assertTrue(effect.isOutputEncodedToSrgb());
                assertTrue(effect.isVertexColorEnabled());
                effect.setDoubleSided(false);
                assertFalse(effect.isDoubleSided(), "and each turns back off on its own");
                assertTrue(effect.isOutputEncodedToSrgb(), "without disturbing its neighbours");
                assertTrue(effect.isVertexColorEnabled());

                for (AlphaMode mode : AlphaMode.values()) {
                    effect.setAlphaMode(mode);
                    assertEquals(mode, effect.getAlphaMode(), "alpha mode " + mode);
                }
                assertThrows(NullPointerException.class, () -> effect.setAlphaMode(null));
            }
        });
    }

    @Test
    void theThreeColourFactorsAreThreeDifferentValues() {
        GameProbe.run(probe -> {
            try (PbrEffect effect = new PbrEffect(probe.device())) {
                effect.setDiffuseColor(new Vector3(0.1f, 0.2f, 0.3f));
                effect.setEmissiveFactor(new Vector3(0.4f, 0.5f, 0.6f));
                effect.setSpecularColorFactor(new Vector3(0.7f, 0.8f, 0.9f));

                Vector3 emissive = effect.getEmissiveFactor();
                assertEquals(0.4f, emissive.X, 1.0e-5f);
                assertEquals(0.5f, emissive.Y, 1.0e-5f);
                assertEquals(0.6f, emissive.Z, 1.0e-5f);

                Vector3 specular = effect.getSpecularColorFactor();
                assertEquals(0.7f, specular.X, 1.0e-5f);
                assertEquals(0.8f, specular.Y, 1.0e-5f);
                assertEquals(0.9f, specular.Z, 1.0e-5f);

                // Base colour comes back as a Color, which is eight bits a channel -- so the
                // comparison is against what a byte can hold rather than against the float.
                Color diffuse = effect.getDiffuseColor();
                assertEquals(Math.round(0.1f * 255f), diffuse.getR(), 1);
                assertEquals(Math.round(0.2f * 255f), diffuse.getG(), 1);
                assertEquals(Math.round(0.3f * 255f), diffuse.getB(), 1);
            }
        });
    }

    @Test
    void eachTextureSlotIsItsOwnSlot() {
        GameProbe.run(probe -> {
            GraphicsDevice device = probe.device();
            Texture2D albedo = new Texture2D(device, 4, 4);
            Texture2D normal = new Texture2D(device, 8, 8);
            try (PbrEffect effect = new PbrEffect(device)) {
                for (PbrTextureSlot slot : PbrTextureSlot.values()) {
                    assertNull(effect.getTexture(slot), slot + " starts empty");
                }

                effect.setTexture(PbrTextureSlot.BaseColor, albedo);
                effect.setTexture(PbrTextureSlot.Normal, normal);
                // Two slots, two textures, and neither leaks into the other -- which a single
                // shared field would fail and a per-slot one passes.
                assertSame(albedo, effect.getTexture(PbrTextureSlot.BaseColor));
                assertSame(normal, effect.getTexture(PbrTextureSlot.Normal));
                assertNull(effect.getTexture(PbrTextureSlot.Emissive));

                // Clearing one leaves the other, and CNA's own answer is what says the slot is
                // empty rather than the Java field.
                effect.setTexture(PbrTextureSlot.BaseColor, null);
                assertNull(effect.getTexture(PbrTextureSlot.BaseColor));
                assertSame(normal, effect.getTexture(PbrTextureSlot.Normal));

                assertThrows(NullPointerException.class, () -> effect.getTexture(null));
                assertThrows(NullPointerException.class, () -> effect.setTexture(null, albedo));

                // Retained, not merely named: CNA refuses to dispose a texture an effect is
                // still holding, which is the difference between a borrow it tracks and a
                // pointer it would later dereference into freed memory.
                assertThrows(RuntimeException.class, normal::Dispose);
                effect.setTexture(PbrTextureSlot.Normal, null);
                normal.Dispose();
                albedo.Dispose();
            }
        });
    }

    @Test
    void eachSlotCarriesItsOwnCoordinateSetAndPlacement() {
        GameProbe.run(probe -> {
            try (PbrEffect effect = new PbrEffect(probe.device())) {
                for (PbrTextureSlot slot : PbrTextureSlot.values()) {
                    effect.setTextureCoordinateSet(slot, slot.ordinal() % 2);
                    effect.setTextureTransform(slot, new TextureTransform(
                            new Vector2(slot.ordinal(), slot.ordinal() * 2f),
                            new Vector2(slot.ordinal() + 1f, slot.ordinal() + 2f),
                            slot.ordinal() * 0.125f));
                }
                // Seven slots with seven different placements. A flattening that wrote every
                // transform to the same five leaves, or that started them one leaf early, gives
                // the wrong answer for at least six of these.
                for (PbrTextureSlot slot : PbrTextureSlot.values()) {
                    assertEquals(slot.ordinal() % 2, effect.getTextureCoordinateSet(slot),
                            slot + " coordinate set");
                    TextureTransform transform = effect.getTextureTransform(slot);
                    assertEquals(slot.ordinal(), transform.offset().X, 1.0e-5f, slot + " offset");
                    assertEquals(slot.ordinal() * 2f, transform.offset().Y, 1.0e-5f);
                    assertEquals(slot.ordinal() + 1f, transform.scale().X, 1.0e-5f,
                            slot + " scale");
                    assertEquals(slot.ordinal() + 2f, transform.scale().Y, 1.0e-5f);
                    assertEquals(slot.ordinal() * 0.125f, transform.rotation(), 1.0e-5f,
                            slot + " rotation");
                }
                assertThrows(NullPointerException.class,
                        () -> effect.setTextureTransform(PbrTextureSlot.Normal, null));
            }
        });
    }

    @Test
    void theSkinnedEffectCarriesBonesAsWellAsTheSurface() {
        GameProbe.run(probe -> {
            try (SkinnedPbrEffect effect = new SkinnedPbrEffect(probe.device())) {
                for (int weights : new int[] {1, 2, 4}) {
                    effect.setWeightsPerVertex(weights);
                    assertEquals(weights, effect.getWeightsPerVertex());
                }
                assertThrows(IllegalArgumentException.class, () -> effect.setWeightsPerVertex(3));

                List<Matrix> bones = new ArrayList<>();
                for (int bone = 0; bone < 8; bone++) {
                    bones.add(Matrix.CreateTranslation(new Vector3(bone, bone * 2f, bone * 3f)));
                }
                effect.setBoneTransforms(bones);
                List<Matrix> returned = effect.getBoneTransforms(8);
                assertEquals(8, returned.size());
                for (int bone = 0; bone < 8; bone++) {
                    // Each bone is its own transform: an array flattened at a fixed offset would
                    // give eight copies of the first.
                    assertEquals(bone, returned.get(bone).M41, 1.0e-5f, "bone " + bone + " x");
                    assertEquals(bone * 3f, returned.get(bone).M43, 1.0e-5f, "bone " + bone);
                }
                assertThrows(IllegalArgumentException.class,
                        () -> effect.getBoneTransforms(SkinnedPbrEffect.MAX_BONES + 1));
                assertThrows(NullPointerException.class, () -> effect.setBoneTransforms(null));

                // And it is a PBR effect too: the inherited setters reach the skinned effect.
                effect.setRoughnessFactor(0.25f);
                assertEquals(0.25f, effect.getRoughnessFactor(), 1.0e-5f);
            }
        });
    }

    @Test
    void nullsAreRefusedBeforeAnythingNativeHappens() {
        GameProbe.run(probe -> {
            try (PbrEffect effect = new PbrEffect(probe.device())) {
                assertThrows(NullPointerException.class,
                        () -> effect.getTextureCoordinateSet(null));
                assertThrows(NullPointerException.class, () -> effect.isTextureSrgb(null));
                assertThrows(NullPointerException.class, () -> new PbrEffect(null));
                assertThrows(NullPointerException.class, () -> new SkinnedPbrEffect(null));
            }
        });
    }
}
