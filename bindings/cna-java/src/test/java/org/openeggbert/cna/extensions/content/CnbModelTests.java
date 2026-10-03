package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The {@code .cnb} model family, against the live runtime.
 *
 * <p>The whole graph goes out through CNA's encoder and back in through its decoder, and what is
 * compared is the graph, not a result code: the bones with their transforms, the parts with their
 * bytes, the meshes with the parts they name, the skeleton's three matrix sets, and the lights.
 *
 * <p>This path was probed in C before any of it was bound. JAVA-UPSTREAM-004 found CNA's
 * <em>content manager</em> model loader segfaulting during teardown for any asset with a mesh
 * part, and the {@code .cnb} family is a different code path -- but that was measured rather than
 * assumed, with a build-encode-decode-destroy cycle in C, before a line of this existed.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnbModelTests {

    /** A triangle: three vertices of three floats, drawn by three sixteen-bit indices. */
    private static final float[] VERTICES = {0f, 0f, 0f, 1f, 0f, 0f, 0f, 1f, 0f};
    private static final short[] INDICES = {0, 1, 2};

    private static byte[] vertexBytes() {
        ByteBuffer buffer = ByteBuffer.allocate(VERTICES.length * 4)
                .order(ByteOrder.LITTLE_ENDIAN);
        for (float value : VERTICES) {
            buffer.putFloat(value);
        }
        return buffer.array();
    }

    private static byte[] indexBytes() {
        ByteBuffer buffer = ByteBuffer.allocate(INDICES.length * 2)
                .order(ByteOrder.LITTLE_ENDIAN);
        for (short value : INDICES) {
            buffer.putShort(value);
        }
        return buffer.array();
    }

    private static CnbModelPart trianglePart() {
        return new CnbModelPart(12, 3, 3, 2, 4, 1, CnbEffectKind.Pbr, true, false);
    }

    private static Matrix scaled(float value) {
        Matrix matrix = Matrix.getIdentity();
        matrix.M11 = value;
        matrix.M22 = value;
        matrix.M33 = value;
        return matrix;
    }

    private static CnbModelData buildModel() {
        CnbModelData model = CnbModelData.create();
        try {
            model.setFlags(true, true);
            assertEquals(0, model.addBone("root", -1, scaled(1.0f)));
            assertEquals(1, model.addBone("child", 0, scaled(2.0f)));
            assertEquals(0, model.addPart(trianglePart(), "triangle", ""));
            model.setPartVertexBytes(0, vertexBytes());
            model.setPartIndexBytes(0, indexBytes());
            assertEquals(0, model.addMesh("body", 1, 0));
            model.setSkeleton(new CnbSkeleton(
                    List.of(-1, 0),
                    List.of(scaled(1.0f), scaled(2.0f)),
                    List.of(scaled(3.0f), scaled(4.0f)),
                    List.of(scaled(5.0f), scaled(6.0f))));
            model.addLight(new CnbModelLight(
                    new Vector3(0f, -1f, 0f), new Vector3(0.25f, 0.5f, 0.75f)));
            return model;
        } catch (RuntimeException failure) {
            model.close();
            throw failure;
        }
    }

    private static void expectReferenceModel(CnbModelData model) {
        CnbModelInfo info = model.getInfo();
        assertEquals(2, info.BoneCount());
        assertEquals(1, info.PartCount());
        assertEquals(1, info.MeshCount());
        assertEquals(1, info.LightCount());
        assertTrue(info.HasSkeleton());
        assertTrue(info.AppliesGltfLightingPolicy());
        assertTrue(info.HasBoneHierarchy());

        List<CnbModelBone> bones = model.getBones();
        assertEquals(List.of("root", "child"), bones.stream().map(CnbModelBone::Name).toList());
        assertEquals(-1, bones.get(0).Parent());
        assertEquals(0, bones.get(1).Parent());
        // Each bone was given a different scale, so a swap or a transposed matrix shows.
        assertEquals(1.0f, bones.get(0).Transform().M11);
        assertEquals(2.0f, bones.get(1).Transform().M22);

        assertEquals(trianglePart(), model.getPart(0));
        assertEquals("triangle", model.getPartName(0));
        assertEquals("", model.getPartExternalEffect(0));
        assertArrayEquals(vertexBytes(), model.readPartVertexBytes(0));
        assertArrayEquals(indexBytes(), model.readPartIndexBytes(0));

        CnbModelMesh mesh = model.getMesh(0);
        assertEquals("body", mesh.Name());
        assertEquals(1, mesh.ParentBone());
        assertEquals(List.of(0), mesh.PartIndices());

        CnbSkeleton skeleton = model.getSkeleton();
        assertEquals(2, skeleton.getJointCount());
        assertEquals(List.of(-1, 0), skeleton.Hierarchy());
        // Three matrix sets with six distinct scales: a set read from the wrong slot cannot
        // pass, which is the point of not using identity anywhere.
        assertEquals(1.0f, skeleton.BindPose().get(0).M11);
        assertEquals(2.0f, skeleton.BindPose().get(1).M11);
        assertEquals(3.0f, skeleton.InverseBindPose().get(0).M11);
        assertEquals(4.0f, skeleton.InverseBindPose().get(1).M11);
        assertEquals(5.0f, skeleton.RootPrefix().get(0).M11);
        assertEquals(6.0f, skeleton.RootPrefix().get(1).M11);

        CnbModelLight light = model.getLight(0);
        assertEquals(new Vector3(0f, -1f, 0f), light.Direction());
        assertEquals(new Vector3(0.25f, 0.5f, 0.75f), light.DiffuseColor());
    }

    @Test
    void theWholeGraphSurvivesTheContainer() {
        byte[] file;
        try (CnbModelData model = buildModel()) {
            expectReferenceModel(model);
            file = Cnb.encodeModel(model, "models/probe");
        }
        try (CnbDocument document = CnbDocument.parse(
                     file, "probe.cnb", CnbReadLimits.standard())) {
            assertEquals(CnbAssetType.MODEL, document.getAssetType());
            try (CnbModelData decoded = document.decodeModel()) {
                expectReferenceModel(decoded);
            }
        }
    }

    @Test
    void aSkeletonIsSeparateFromTheBones() {
        try (CnbModelData model = CnbModelData.create()) {
            model.addBone("root", -1, Matrix.getIdentity());
            // Bones without a skeleton is an ordinary shape: the bones are the asset's node
            // hierarchy, the skeleton is what a skinned part is weighted against.
            assertEquals(1, model.getInfo().BoneCount());
            assertFalse(model.getInfo().HasSkeleton());
            assertNull(model.getSkeleton());

            model.setSkeleton(new CnbSkeleton(List.of(-1), List.of(scaled(2.0f)),
                    List.of(scaled(3.0f)), null));
            assertTrue(model.getInfo().HasSkeleton());
            // A source that carried no prefix is not the same as one that carried an identity,
            // so an absent prefix comes back absent.
            assertNull(model.getSkeleton().RootPrefix());

            model.clearSkeleton();
            assertFalse(model.getInfo().HasSkeleton());
            assertEquals(1, model.getInfo().BoneCount(), "clearing a skeleton keeps the bones");
        }

        // The record refuses a skeleton whose parts describe different joints, because the
        // native call would otherwise be given arrays that disagree.
        assertThrows(IllegalArgumentException.class, () -> new CnbSkeleton(
                List.of(-1, 0), List.of(Matrix.getIdentity()),
                List.of(Matrix.getIdentity(), Matrix.getIdentity()), null));
    }

    @Test
    void aDescriptionThatDoesNotMatchItsBytesIsRefusedByTheEncoder() {
        try (CnbModelData model = CnbModelData.create()) {
            model.addBone("root", -1, Matrix.getIdentity());
            model.addPart(trianglePart(), "triangle", "");
            // Three vertices at a stride of twelve is thirty-six bytes; this part is given
            // twenty-four. The handle takes it -- it stores a description and a payload -- and
            // the encoder is where the two have to agree, because that is the point at which a
            // file someone else would read gets written.
            model.setPartVertexBytes(0, new byte[24]);
            model.setPartIndexBytes(0, indexBytes());
            model.addMesh("body", 0, 0);
            assertThrows(CnbFormatException.class,
                    () -> Cnb.encodeModel(model, "models/short"));
        }
    }

    @Test
    void anExternalEffectNamesItsOwnAsset() {
        try (CnbModelData model = CnbModelData.create()) {
            CnbModelPart part = new CnbModelPart(12, 3, 3, 2, 4, 1,
                    CnbEffectKind.External, false, true);
            model.addPart(part, "custom", "effects/water");
            assertEquals(CnbEffectKind.External, model.getPart(0).EffectKind());
            assertEquals("effects/water", model.getPartExternalEffect(0));
            assertTrue(model.getPart(0).Unlit());

            // Replacing the description leaves the bytes and the names alone.
            model.setPartVertexBytes(0, vertexBytes());
            model.setPart(0, trianglePart());
            assertEquals(trianglePart(), model.getPart(0));
            assertEquals("custom", model.getPartName(0));
            assertArrayEquals(vertexBytes(), model.readPartVertexBytes(0));
        }
    }

    @Test
    void blendShapesSurviveTheContainer() {
        byte[] file;
        try (CnbModelData model = buildModel()) {
            assertFalse(model.hasMorph(0), "a plain part carries no blend shapes");

            // setMorph is what creates the morph block; adding a target to a part that has
            // none is refused rather than silently creating one.
            assertThrows(IllegalArgumentException.class, () -> model.addMorphTarget(0));
            model.setMorph(0, 3, true, false, true);
            assertTrue(model.hasMorph(0));
            assertEquals(0, model.addMorphTarget(0));
            assertEquals(1, model.addMorphTarget(0));

            // Three vertices of three floats per stream, each stream given its own values so a
            // position read out of the normal slot cannot pass.
            for (int target = 0; target < 2; target++) {
                for (CnbMorphDeltaStream stream : CnbMorphDeltaStream.values()) {
                    model.setMorphTargetDeltas(0, target, stream,
                            deltas(target * 100 + stream.ordinal() * 10));
                }
            }
            model.setMorphWeights(0, new float[] {0.25f, 0.75f});
            assertEquals(0, model.addMorphWeightKey(0, 0.0,
                    new float[] {0.0f, 0.0f},
                    new float[] {0.1f, 0.2f}, new float[] {0.3f, 0.4f}));
            assertEquals(1, model.addMorphWeightKey(0, 1.5,
                    new float[] {1.0f, 0.5f},
                    new float[] {0.5f, 0.6f}, new float[] {0.7f, 0.8f}));

            CnbMorphInfo info = model.getMorph(0);
            assertEquals(3, info.VertexCount());
            assertEquals(2, info.TargetCount());
            assertEquals(2, info.WeightCount());
            assertEquals(2, info.WeightTrackKeyCount());
            assertTrue(info.RecomputeFlatNormals());
            assertFalse(info.WeightTrackStepInterpolation());
            assertTrue(info.WeightTrackCubicSpline());

            file = Cnb.encodeModel(model, "models/morph");
        }

        try (CnbDocument document = CnbDocument.parse(
                     file, "morph.cnb", CnbReadLimits.standard());
             CnbModelData decoded = document.decodeModel()) {
            assertTrue(decoded.hasMorph(0));
            assertEquals(2, decoded.getMorph(0).TargetCount());
            for (int target = 0; target < 2; target++) {
                for (CnbMorphDeltaStream stream : CnbMorphDeltaStream.values()) {
                    assertArrayEquals(deltas(target * 100 + stream.ordinal() * 10),
                            decoded.readMorphTargetDeltas(0, target, stream),
                            "target " + target + " " + stream);
                }
            }
            assertArrayEquals(new float[] {0.25f, 0.75f}, decoded.readMorphWeights(0));

            CnbMorphWeightKey second = decoded.getMorphWeightKey(0, 1);
            assertEquals(1.5, second.TimeSeconds());
            assertEquals(2, second.WeightCount());
            assertEquals(2, second.InTangentCount());
            assertArrayEquals(new float[] {1.0f, 0.5f},
                    decoded.readMorphWeightKeyValues(0, 1, CnbMorphKeyStream.Weights));
            assertArrayEquals(new float[] {0.5f, 0.6f},
                    decoded.readMorphWeightKeyValues(0, 1, CnbMorphKeyStream.InTangent));
            assertArrayEquals(new float[] {0.7f, 0.8f},
                    decoded.readMorphWeightKeyValues(0, 1, CnbMorphKeyStream.OutTangent));
        }

        try (CnbModelData model = buildModel()) {
            model.setMorph(0, 3, false, true, false);
            model.addMorphTarget(0);
            assertTrue(model.hasMorph(0));
            model.clearMorph(0);
            assertFalse(model.hasMorph(0), "clearing removes the blend shapes entirely");
        }
    }

    /** Three vertices of three floats, offset so every stream and target is distinguishable. */
    private static float[] deltas(int offset) {
        float[] values = new float[9];
        for (int index = 0; index < values.length; index++) {
            values[index] = offset + index;
        }
        return values;
    }

    @Test
    void aMaterialAndItsEightSlotsSurviveTheContainer() {
        CnbMaterial material = new CnbMaterial(
                new Vector4(0.25f, 0.5f, 0.75f, 1.0f),
                new Vector3(0.125f, 0.0f, 0.0f),
                new Vector3(0.875f, 0.0f, 0.0f),
                0.4f, 0.6f, 1.33f, 0.9f, 1.5f, 0.8f, 0.25f,
                CnbAlphaMode.Blend, true);
        byte[] file;
        try (CnbModelData model = buildModel()) {
            model.setMaterial(0, material);
            assertEquals(material, model.getMaterial(0));

            // Eight slots, each given its own asset name, coordinate set and transform, so a
            // slot read from the wrong index cannot round trip.
            CnbMaterialTextureSlot[] slots = CnbMaterialTextureSlot.values();
            for (int index = 0; index < slots.length; index++) {
                model.setMaterialTexture(0, slots[index], "textures/slot" + index);
            }
            // The per-slot state is a different index space -- seven entries in the importer's
            // order, not eight names in CNA's effect order -- which is a trap CNA's own header
            // warns about and which two Java types make uncompilable.
            CnbImporterTextureSlot[] importer = CnbImporterTextureSlot.values();
            for (int index = 0; index < importer.length; index++) {
                // Zero or one: CNA's vertex layouts carry at most two coordinate
                // sets, and the decoder refuses a file that names a third.
                model.setMaterialTextureCoordinateSet(0, importer[index], index % 2);
                model.setMaterialTextureTransform(0, importer[index], new CnbTextureTransform(
                        index, index + 0.5f, index + 1.0f, index + 1.5f, index + 2.0f));
                model.setMaterialSampler(0, importer[index],
                        new CnbSamplerState(index, index + 1, index + 2, true));
            }
            file = Cnb.encodeModel(model, "models/material");
        }

        try (CnbDocument document = CnbDocument.parse(
                     file, "material.cnb", CnbReadLimits.standard());
             CnbModelData decoded = document.decodeModel()) {
            assertEquals(material, decoded.getMaterial(0));
            CnbMaterialTextureSlot[] slots = CnbMaterialTextureSlot.values();
            for (int index = 0; index < slots.length; index++) {
                assertEquals("textures/slot" + index,
                        decoded.getMaterialTexture(0, slots[index]));
            }
            CnbImporterTextureSlot[] importer = CnbImporterTextureSlot.values();
            for (int index = 0; index < importer.length; index++) {
                assertEquals(index % 2,
                        decoded.getMaterialTextureCoordinateSet(0, importer[index]));
                assertEquals(new CnbTextureTransform(
                                index, index + 0.5f, index + 1.0f, index + 1.5f, index + 2.0f),
                        decoded.getMaterialTextureTransform(0, importer[index]));
                assertEquals(new CnbSamplerState(index, index + 1, index + 2, true),
                        decoded.getMaterialSampler(0, importer[index]));
            }
        }
    }

    @Test
    void anUndeclaredSamplerIsNotTheSameAsADefaultOne() {
        try (CnbModelData model = CnbModelData.create()) {
            model.addPart(trianglePart(), "triangle", "");
            // A part with nothing said about its sampling states nothing, which is what lets a
            // renderer choose. That is different from a part that explicitly asks for filter 0.
            assertFalse(model.getMaterialSampler(0, CnbImporterTextureSlot.BaseColor).Declared());
            model.setMaterialSampler(0, CnbImporterTextureSlot.BaseColor,
                    new CnbSamplerState(0, 0, 0, true));
            assertTrue(model.getMaterialSampler(0, CnbImporterTextureSlot.BaseColor).Declared());
            model.setMaterialSampler(0, CnbImporterTextureSlot.BaseColor,
                    CnbSamplerState.undeclared());
            assertFalse(model.getMaterialSampler(0, CnbImporterTextureSlot.BaseColor).Declared());

            // An empty slot names no texture, and clearing one puts it back that way.
            assertEquals("", model.getMaterialTexture(0, CnbMaterialTextureSlot.Normal));
            model.setMaterialTexture(0, CnbMaterialTextureSlot.Normal, "textures/bumps");
            assertEquals("textures/bumps",
                    model.getMaterialTexture(0, CnbMaterialTextureSlot.Normal));
            model.setMaterialTexture(0, CnbMaterialTextureSlot.Normal, "");
            assertEquals("", model.getMaterialTexture(0, CnbMaterialTextureSlot.Normal));

            assertEquals(CnbTextureTransform.identity(),
                    model.getMaterialTextureTransform(0, CnbImporterTextureSlot.Emissive));
            assertThrows(IllegalArgumentException.class, () -> model
                    .setMaterialTextureCoordinateSet(0, CnbImporterTextureSlot.Normal, 256));

            // A coordinate set CNA's vertex layouts cannot carry is stored by the writer and
            // refused by the reader, which is the container's own layering: the writer records
            // what it was told, and the file is where the rule is enforced.
            model.addBone("root", -1, Matrix.getIdentity());
            model.setPartVertexBytes(0, vertexBytes());
            model.setPartIndexBytes(0, indexBytes());
            model.addMesh("body", 0, 0);
            model.setMaterialTextureCoordinateSet(0, CnbImporterTextureSlot.Normal, 2);
            byte[] file = Cnb.encodeModel(model, "models/third-set");
            assertThrows(CnbFormatException.class, () -> {
                try (CnbDocument document = CnbDocument.parse(
                             file, "third-set.cnb", CnbReadLimits.standard())) {
                    document.decodeModel().close();
                }
            });
            assertThrows(NullPointerException.class,
                    () -> model.getMaterialTexture(0, null));
        }
    }

    @Test
    void aClosedModelRefusesEveryOperation() {
        CnbModelData model = CnbModelData.create();
        model.close();
        model.close();
        assertThrows(IllegalStateException.class, model::getInfo);
        assertThrows(IllegalStateException.class,
                () -> model.addBone("late", -1, Matrix.getIdentity()));
    }
}
