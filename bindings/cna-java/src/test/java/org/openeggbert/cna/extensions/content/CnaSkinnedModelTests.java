package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Graphics.BufferUsage;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.IndexBuffer;
import Microsoft.Xna.Framework.Graphics.IndexElementSize;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.VertexBuffer;
import Microsoft.Xna.Framework.Graphics.VertexPositionColor;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * A skinned model: the skeleton, the clips and the parts, and what it owns.
 *
 * <p>The container half of the family {@code JAVA-EXT-007} recorded as having no door in. The door
 * turned out to be the descriptor graph, which is marshalled by hand now; what is qualified here
 * is what the container does with what it is given -- including the ownership, which is the part
 * that crashes when it is wrong.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnaSkinnedModelTests {

    private static CnaSkeleton skeleton() {
        return new CnaSkeleton(
                List.of(-1, 0),
                List.of(Matrix.CreateTranslation(new Vector3(1f, 0f, 0f)),
                        Matrix.CreateTranslation(new Vector3(0f, 2f, 0f))),
                List.of(Matrix.CreateTranslation(new Vector3(-1f, 0f, 0f)),
                        Matrix.CreateTranslation(new Vector3(0f, -2f, 0f))),
                List.of());
    }

    private static CnbClip slide() {
        return new CnbClip(2.0, List.of(new CnbBoneTrack(1, List.of(
                new CnbKeyframe(0d, new Vector3(0f, 0f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f)),
                new CnbKeyframe(2.0, new Vector3(10f, 0f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f))))));
    }

    private static Map<String, CnbClip> clips() {
        Map<String, CnbClip> clips = new LinkedHashMap<>();
        clips.put("slide", slide());
        return clips;
    }

    @Test
    void aModelKeepsItsSkeletonAndItsClipsAndCanRemoveOne() {
        CnbExtensionProbe.run(() -> {
            try (CnaSkinnedModel model = CnaSkinnedModel.of(skeleton(), clips())) {
                assertEquals(2, model.getBoneCount());
                assertEquals(1, model.getClipCount());
                assertEquals(List.of("slide"), model.getClipNames());

                CnaSkeleton read = model.getSkeleton();
                assertEquals(List.of(-1, 0), read.ParentBoneIndices());
                assertEquals(1f, read.BindPoseLocal().get(0).M41, 1e-5f);
                assertEquals(2f, read.BindPoseLocal().get(1).M42, 1e-5f);
                assertEquals(-1f, read.InverseBindPoseGlobal().get(0).M41, 1e-5f);

                CnbAnimation clip = model.getClip("slide");
                assertNotNull(clip);
                assertEquals(2.0, clip.DurationSeconds(), 0d);
                assertEquals(1, clip.TrackCount());
                assertNull(model.getClip("absent"));

                CnbBoneTrack track = model.getClipTrack("slide", 0);
                assertEquals(1, track.BoneIndex());
                assertEquals(2, track.Keyframes().size());
                assertEquals(10f, track.Keyframes().get(1).Translation().X, 1e-5f);

                // A second clip, then remove the first, so the removal is visible as a name
                // going away rather than only as a count changing.
                model.setClip("lift", new CnbClip(1.0, List.of(new CnbBoneTrack(0, List.of(
                        new CnbKeyframe(0d, new Vector3(0f, 5f, 0f),
                                new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f)))))));
                assertEquals(2, model.getClipCount());
                model.removeClip("slide");
                assertEquals(1, model.getClipCount());
                assertEquals(List.of("lift"), model.getClipNames());
                assertNull(model.getClip("slide"), "the removed clip is gone, not emptied");
            }
        });
    }

    @Test
    void aClipEvaluatesToTheSamePoseTheStandalonePlayerProduces() {
        CnbExtensionProbe.run(() -> {
            try (CnaSkinnedModel model = CnaSkinnedModel.of(skeleton(), clips())) {
                List<Matrix> atStart = model.computeBoneTransforms("slide", 0d, false);
                List<Matrix> atEnd = model.computeBoneTransforms("slide", 2.0, false);
                assertEquals(2, atStart.size(), "one matrix per bone");
                assertEquals(atStart.get(1).M41 + 10f, atEnd.get(1).M41, 1e-4f,
                        "the clip slides bone one ten units along X");
                assertNotEquals(atStart.get(1).M41, atEnd.get(1).M41);

                // Halfway is halfway, and past the end clamps unless it is told to loop.
                List<Matrix> halfway = model.computeBoneTransforms("slide", 1.0, false);
                assertEquals(atStart.get(1).M41 + 5f, halfway.get(1).M41, 1e-4f);
                assertEquals(atEnd.get(1).M41,
                        model.computeBoneTransforms("slide", 99d, false).get(1).M41, 1e-4f);
                assertEquals(halfway.get(1).M41,
                        model.computeBoneTransforms("slide", 3.0, true).get(1).M41, 1e-4f,
                        "three seconds into a two-second loop is one second in");

                // The same clip through the standalone runtime, which is a second implementation
                // of the same question -- so agreeing is evidence rather than a tautology.
                try (CnaSkinningData data = CnaSkinningData.of(skeleton(), clips());
                        CnaAnimationPlayer player = CnaAnimationPlayer.of(data)) {
                    player.startClip("slide");
                    player.update(2.0, false, false);
                    assertEquals(atEnd.get(1).M41, player.getSkinTransforms().get(1).M41, 1e-3f,
                            "the model's own evaluation and the player's agree at the same time");
                }

                assertThrows(RuntimeException.class,
                        () -> model.computeBoneTransforms("absent", 0d, false));
            }
        });
    }

    @Test
    void aPartIsRetainedByTheModelAndReleasedWithIt() {
        CnaSkinnedModelProbe.run(device -> {
            // The declaration order is the ownership chain reversed, and it is not incidental.
            // A ModelMeshPart RETAINS its vertex and index buffers, and a model RETAINS the part
            // rather than taking it -- so a model that has released everything it owns still
            // leaves the part alive, still holding the buffers. CNA refuses to destroy a buffer
            // a part holds and names the part when it does, which is how this rule was found:
            // the first version of this test closed them the other way round.
            try (VertexBuffer vertices = new VertexBuffer(device,
                            VertexPositionColor.VertexDeclaration, 3, BufferUsage.None);
                    IndexBuffer indices = new IndexBuffer(device, IndexElementSize.SixteenBits,
                            3, BufferUsage.None);
                    Texture2D texture = new Texture2D(device, 2, 2);
                    CnaModelMeshPartHandle part = CnaModelMeshPartHandle.create(
                            vertices, indices, 3, 1, 0, 0);
                    CnaModelMeshPartHandle bare = CnaModelMeshPartHandle.create(
                            vertices, indices, 3, 1, 0, 0);
                    CnaSkinnedModel model = CnaSkinnedModel.of(skeleton(), clips())) {
                assertEquals(0, model.getPartCount());
                int[] before = model.getOwnedResourceCounts();
                assertEquals(0, before[2], "no parts owned before one is added");

                model.addPart("body", vertices, indices, part, texture);
                assertEquals(1, model.getPartCount());
                assertEquals(List.of("body"), model.getPartNames());

                int[] owned = model.getOwnedResourceCounts();
                assertEquals(1, owned[0], "the vertex buffer is retained");
                assertEquals(1, owned[1], "and the index buffer");
                assertEquals(1, owned[2], "and the part");
                assertEquals(1, owned[3], "and the texture");

                // Both handles getPart answers with are the caller's, which is measured rather
                // than assumed: CNA calls the part "an owned part alias" and the texture
                // "retained", and dropping either leaves the part holding its buffers forever --
                // visible only much later, as CNA refusing to destroy a vertex buffer.
                try (CnaSkinnedModel.Part read = model.getPart(0)) {
                    assertTrue(read.HasTexture(), "the part carries the texture it was given");
                    assertNotNull(read.MeshPart());
                }

                // A part with no texture is a different answer, not a missing one.
                model.addPart("hair", vertices, indices, bare, null);
                assertEquals(2, model.getPartCount());
                try (CnaSkinnedModel.Part bareRead = model.getPart(1)) {
                    assertFalse(bareRead.HasTexture());
                }
                assertEquals(1, model.getOwnedResourceCounts()[3],
                        "and it retained no second texture");

                model.removePart("body");
                assertEquals(1, model.getPartCount());
                assertEquals(List.of("hair"), model.getPartNames());
                assertEquals(1, model.getOwnedResourceCounts()[2],
                        "removing a part releases what it retained");
            }
        });
    }

    @Test
    void attachingPartsMovesThemAndLeavesTheSourceEmpty() {
        CnaSkinnedModelProbe.run(device -> {
            // Same chain as above: the part outlives both models and is closed before the
            // buffers it holds.
            try (VertexBuffer vertices = new VertexBuffer(device,
                            VertexPositionColor.VertexDeclaration, 3, BufferUsage.None);
                    IndexBuffer indices = new IndexBuffer(device, IndexElementSize.SixteenBits,
                            3, BufferUsage.None);
                    CnaModelMeshPartHandle arm = CnaModelMeshPartHandle.create(
                            vertices, indices, 3, 1, 0, 0);
                    CnaSkinnedModel first = CnaSkinnedModel.of(skeleton(), clips());
                    CnaSkinnedModel second = CnaSkinnedModel.of(skeleton(), clips())) {
                second.addPart("arm", vertices, indices, arm, null);
                assertEquals(1, second.getPartCount());

                first.attachPartsFrom(second);
                assertEquals(1, first.getPartCount(), "the part moved");
                assertEquals(List.of("arm"), first.getPartNames());
                assertEquals(0, second.getPartCount(),
                        "and the source is left valid and empty, which is a move rather than a "
                                + "copy");
            }
        });
    }

    @Test
    void whatIsRefusedIsRefused() {
        CnbExtensionProbe.run(() -> {
            try (CnaSkinnedModel model = CnaSkinnedModel.create()) {
                assertEquals(0, model.getBoneCount(), "an empty model is a model");
                assertEquals(0, model.getClipCount());
                assertEquals(0, model.getPartCount());
                assertTrue(model.getSkeleton().ParentBoneIndices().isEmpty());

                assertThrows(NullPointerException.class, () -> model.setSkeleton(null));
                assertThrows(NullPointerException.class, () -> model.setClip(null, slide()));
                assertThrows(NullPointerException.class, () -> model.setClip("x", null));
                assertThrows(NullPointerException.class, () -> model.removeClip(null));
                assertThrows(NullPointerException.class, () -> model.attachPartsFrom(null));
                assertThrows(NullPointerException.class, () -> CnaModelMeshPartHandle.of(null));
            }
            CnaSkinnedModel closed = CnaSkinnedModel.create();
            closed.close();
            closed.close();
            assertThrows(IllegalStateException.class, closed::getBoneCount);
            assertThrows(NullPointerException.class,
                    () -> CnaSkinnedModel.of(null, Map.of()));
        });
    }

    @Test
    void aMeshPartCarriesItsOwnTopologySamplerAndMorphData() {
        CnaSkinnedModelProbe.run(device -> {
            try (VertexBuffer vertices = new VertexBuffer(device,
                            VertexPositionColor.VertexDeclaration, 3, BufferUsage.None);
                    IndexBuffer indices = new IndexBuffer(device, IndexElementSize.SixteenBits,
                            3, BufferUsage.None);
                    CnaModelMeshPartHandle part = CnaModelMeshPartHandle.create(
                            vertices, indices, 3, 1, 0, 0)) {

                // Topology: XNA's ModelMeshPart always draws a triangle list and glTF has five
                // more, so this is a per-part fact XNA has no member for.
                assertEquals(Microsoft.Xna.Framework.Graphics.PrimitiveType.TriangleList,
                        part.getPrimitiveType());
                part.setPrimitiveType(
                        Microsoft.Xna.Framework.Graphics.PrimitiveType.LineStrip);
                assertEquals(Microsoft.Xna.Framework.Graphics.PrimitiveType.LineStrip,
                        part.getPrimitiveType());

                // A sampler per texture slot, which XNA has only per device. The two slots keep
                // different states, so a projection that ignored the slot is caught.
                Microsoft.Xna.Framework.Graphics.SamplerState mirrored =
                        new Microsoft.Xna.Framework.Graphics.SamplerState();
                mirrored.setAddressU(
                        Microsoft.Xna.Framework.Graphics.TextureAddressMode.Mirror);
                mirrored.setFilter(Microsoft.Xna.Framework.Graphics.TextureFilter.Point);
                mirrored.setMaxAnisotropy(8);
                mirrored.setMipMapLevelOfDetailBias(0.5f);
                part.setSamplerState(
                        org.openeggbert.cna.extensions.graphics.PbrTextureSlot.BaseColor,
                        mirrored);

                Microsoft.Xna.Framework.Graphics.SamplerState read = part.getSamplerState(
                        org.openeggbert.cna.extensions.graphics.PbrTextureSlot.BaseColor);
                assertEquals(Microsoft.Xna.Framework.Graphics.TextureAddressMode.Mirror,
                        read.getAddressU());
                assertEquals(Microsoft.Xna.Framework.Graphics.TextureFilter.Point,
                        read.getFilter());
                assertEquals(8, read.getMaxAnisotropy());
                assertEquals(0.5f, read.getMipMapLevelOfDetailBias(), 1e-6f);
                assertNotEquals(Microsoft.Xna.Framework.Graphics.TextureAddressMode.Mirror,
                        part.getSamplerState(
                                org.openeggbert.cna.extensions.graphics.PbrTextureSlot.Normal)
                                .getAddressU(),
                        "another slot keeps its own state");

                // And the morph data a part blends with, which is what makes a face smile
                // without a second mesh.
                assertNull(part.getMorphTargetData(), "a part starts with none");
                assertThrows(NullPointerException.class, () -> part.setMorphTargetData(null));

                // An infinite-far-plane projection, which every XNA factory refuses to build:
                // they all take a far plane, and a glTF camera may declare none. The third row
                // and fourth column are what carry the difference, so those are what is
                // asserted -- against the finite form of the same camera.
                Matrix infinite = CnaModelCamera.CreateInfinitePerspectiveFieldOfView(
                        (float) (Math.PI / 4.0), 16f / 9f, 0.1f);
                Matrix finite = Matrix.CreatePerspectiveFieldOfView(
                        (float) (Math.PI / 4.0), 16f / 9f, 0.1f, 1000f);
                assertEquals(finite.M11, infinite.M11, 1e-5f, "the x scale is the same camera");
                assertEquals(finite.M22, infinite.M22, 1e-5f, "and so is the y scale");
                assertEquals(-1f, infinite.M33, 1e-5f,
                        "an infinite far plane drives the depth scale to minus one");
                assertNotEquals(finite.M33, infinite.M33,
                        "which a thousand-unit far plane does not");
            }
        });
    }
}
