package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Skeletal animation: the runtime XNA left to its {@code SkinnedModel} sample.
 *
 * <p>Eighty routes were recorded as blocked, and the reason given was that the family "is entered
 * through cna_skinning_data_create, which takes a CNA_SkinningDataDescriptor -- a pointer graph
 * the generator refuses rather than guesses at -- and no route takes a clip handle, so there is no
 * other door in".
 *
 * <p>Every clause of that is true and the conclusion did not follow. The generator does refuse the
 * shape, and it is right to: nothing in the C says which keyframes belong to which track, nor how
 * many matrices sit behind a bone count. But a shape a generator cannot derive is not a lifetime
 * nobody knows. CNA states both plainly -- every array borrowed for the call, the whole descriptor
 * deeply copied -- so the graph is built by hand for one call, with every count checked against
 * every array before anything is allocated, and freed after it. That is what a hand-written
 * marshaller is for.
 *
 * <p>What is asserted here is arithmetic and structure, not result codes: a two-bone skeleton and
 * a two-second clip go in, and the skeleton, the clips, every keyframe and the three matrix
 * palettes come back out.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnaSkinningDataTests {

    /** A two-bone skeleton: a root and a child, with distinguishable bind poses. */
    private static CnaSkeleton skeleton() {
        return new CnaSkeleton(
                List.of(-1, 0),
                List.of(Matrix.CreateTranslation(new Vector3(1f, 0f, 0f)),
                        Matrix.CreateTranslation(new Vector3(0f, 2f, 0f))),
                List.of(Matrix.CreateTranslation(new Vector3(-1f, 0f, 0f)),
                        Matrix.CreateTranslation(new Vector3(0f, -2f, 0f))),
                List.of());
    }

    /** A clip that slides bone one ten units along X across two seconds. */
    private static CnbClip slide() {
        return new CnbClip(2.0, List.of(new CnbBoneTrack(1, List.of(
                new CnbKeyframe(0d, new Vector3(0f, 0f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f)),
                new CnbKeyframe(2.0, new Vector3(10f, 0f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f))))));
    }

    /** A shorter clip on the other bone, so two clips are never confusable. */
    private static CnbClip lift() {
        return new CnbClip(1.0, List.of(new CnbBoneTrack(0, List.of(
                new CnbKeyframe(0d, new Vector3(0f, 5f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f))))));
    }

    private static Map<String, CnbClip> clips() {
        Map<String, CnbClip> clips = new LinkedHashMap<>();
        clips.put("slide", slide());
        clips.put("lift", lift());
        return clips;
    }

    @Test
    void aSkeletonAndItsClipsSurviveTheDescriptorGraph() {
        CnbExtensionProbe.run(() -> {
            try (CnaSkinningData data = CnaSkinningData.of(skeleton(), clips())) {
                assertEquals(2, data.getBoneCount());
                assertEquals(2, data.getClipCount());

                // The skeleton, read back out of CNA's own copy. The two bind poses translate
                // along different axes by different amounts, so a marshaller that handed CNA the
                // same matrix twice -- or the inverse where the bind pose belonged -- is caught
                // rather than agreeing by symmetry.
                CnaSkeleton read = data.getSkeleton();
                assertEquals(List.of(-1, 0), read.ParentBoneIndices());
                assertEquals(1f, read.BindPoseLocal().get(0).M41, 1e-5f);
                assertEquals(0f, read.BindPoseLocal().get(0).M42, 1e-5f);
                assertEquals(2f, read.BindPoseLocal().get(1).M42, 1e-5f);
                assertEquals(-1f, read.InverseBindPoseGlobal().get(0).M41, 1e-5f);
                assertEquals(-2f, read.InverseBindPoseGlobal().get(1).M42, 1e-5f);
                assertTrue(read.RootPrefix().isEmpty(),
                        "an absent root prefix comes back absent, not as identity matrices");

                // The clips, by name, with their own durations and track counts.
                assertEquals(List.of("lift", "slide"), data.getClipNames(),
                        "CNA orders its clips by name, as the model animations do");
                CnbAnimation slide = data.getClip("slide");
                assertNotNull(slide);
                assertEquals(2.0, slide.DurationSeconds(), 0d);
                assertEquals(1, slide.TrackCount());
                CnbAnimation lift = data.getClip("lift");
                assertNotNull(lift);
                assertEquals(1.0, lift.DurationSeconds(), 0d);
                assertNotEquals(slide.DurationSeconds(), lift.DurationSeconds());
                assertNull(data.getClip("absent"), "an unknown clip is absent, not a failure");

                // And every keyframe of a track, which is the deepest level of the graph.
                CnbBoneTrack track = data.getClipTrack("slide", 0);
                assertEquals(1, track.BoneIndex(),
                        "the track drives bone one, not bone zero");
                assertEquals(2, track.Keyframes().size());
                assertEquals(0d, track.Keyframes().get(0).TimeSeconds(), 0d);
                assertEquals(0f, track.Keyframes().get(0).Translation().X, 1e-5f);
                assertEquals(2.0, track.Keyframes().get(1).TimeSeconds(), 0d);
                assertEquals(10f, track.Keyframes().get(1).Translation().X, 1e-5f);

                CnbBoneTrack other = data.getClipTrack("lift", 0);
                assertEquals(0, other.BoneIndex(), "and the other clip drives the other bone");
                assertEquals(5f, other.Keyframes().get(0).Translation().Y, 1e-5f);
            }
        });
    }

    @Test
    void aRootPrefixIsOptionalAndIsOneMatrixPerBoneWhenItIsThere() {
        CnbExtensionProbe.run(() -> {
            CnaSkeleton prefixed = new CnaSkeleton(
                    List.of(-1, 0),
                    skeleton().BindPoseLocal(), skeleton().InverseBindPoseGlobal(),
                    List.of(Matrix.CreateTranslation(new Vector3(7f, 0f, 0f)),
                            Matrix.CreateTranslation(new Vector3(0f, 8f, 0f))));
            try (CnaSkinningData data = CnaSkinningData.of(prefixed, clips())) {
                CnaSkeleton read = data.getSkeleton();
                assertEquals(2, read.RootPrefix().size());
                assertEquals(7f, read.RootPrefix().get(0).M41, 1e-5f);
                assertEquals(8f, read.RootPrefix().get(1).M42, 1e-5f);
            }
            // A prefix that is neither empty nor one per bone is refused before CNA sees it.
            assertThrows(IllegalArgumentException.class, () -> new CnaSkeleton(
                    List.of(-1, 0), skeleton().BindPoseLocal(),
                    skeleton().InverseBindPoseGlobal(),
                    List.of(Matrix.getIdentity())));
        });
    }

    @Test
    void aSkeletonWhoseListsDisagreeIsRefusedBeforeItReachesCna() {
        assertThrows(IllegalArgumentException.class, () -> new CnaSkeleton(
                List.of(-1, 0), List.of(Matrix.getIdentity()),
                List.of(Matrix.getIdentity(), Matrix.getIdentity()), List.of()));
        assertThrows(IllegalArgumentException.class, () -> new CnaSkeleton(
                List.of(-1), List.of(Matrix.getIdentity()), List.of(), List.of()));
        assertThrows(NullPointerException.class,
                () -> new CnaSkeleton(null, List.of(), List.of(), List.of()));
        assertEquals(2, skeleton().boneCount());
    }

    @Test
    void aClipsTargetSpaceIsStatedAndTheRootIsNameable() {
        CnbExtensionProbe.run(() -> {
            try (CnaSkinningData data = CnaSkinningData.of(skeleton(), clips())) {
                data.setClipTargetSpace(0, CnbClipTargetSpace.JointPalette);
                data.setClipTargetSpace(1, CnbClipTargetSpace.SceneNode);
                assertEquals(CnbClipTargetSpace.JointPalette, data.getClipTargetSpace(0));
                assertEquals(CnbClipTargetSpace.SceneNode, data.getClipTargetSpace(1),
                        "the two clips keep different spaces, so neither is a default");

                assertEquals("", data.getSkeletonRootName(),
                        "a skeleton with no named root says so with an empty name");
                data.setSkeletonRootName("Armature");
                assertEquals("Armature", data.getSkeletonRootName());

                assertEquals(-1, data.getSkeletonRootNodeIndex(),
                        "and -1 for no node, which is not node zero");
                data.setSkeletonRootNodeIndex(3);
                assertEquals(3, data.getSkeletonRootNodeIndex());

                assertThrows(NullPointerException.class,
                        () -> data.setSkeletonRootName(null));
                assertThrows(NullPointerException.class,
                        () -> data.setClipTargetSpace(0, null));
            }
        });
    }

    @Test
    void aPlayerPosesTheSkeletonAndReportsThreeDifferentPalettes() {
        CnbExtensionProbe.run(() -> {
            try (CnaSkinningData data = CnaSkinningData.of(skeleton(), clips());
                    CnaAnimationPlayer player = CnaAnimationPlayer.of(data)) {
                assertNull(player.getCurrentClip(), "nothing plays before a clip is started");

                player.startClip("slide");
                CnbAnimation playing = player.getCurrentClip();
                assertNotNull(playing);
                assertEquals("slide", playing.Name());
                assertEquals(2.0, playing.DurationSeconds(), 0d);
                assertEquals(0d, player.getCurrentPosition(), 1e-9);

                player.update(0d, false, false);
                List<Matrix> atStart = player.getBoneTransforms();
                assertEquals(2, atStart.size(), "one matrix per bone");

                // Absolute rather than relative: the whole clip in one step.
                player.update(2.0, false, false);
                assertEquals(2.0, player.getCurrentPosition(), 1e-9);
                List<Matrix> atEnd = player.getBoneTransforms();
                assertEquals(atStart.get(1).M41 + 10f, atEnd.get(1).M41, 1e-4f,
                        "bone one slid ten units along X, which is what the clip says");
                assertEquals(atStart.get(0).M41, atEnd.get(0).M41, 1e-4f,
                        "and bone zero, which the clip does not animate, did not move");

                // Relative advances from where it is rather than jumping.
                player.update(0d, false, false);
                player.update(1.0, true, false);
                assertEquals(1.0, player.getCurrentPosition(), 1e-9);
                assertEquals(atStart.get(1).M41 + 5f, player.getBoneTransforms().get(1).M41,
                        1e-4f, "one second into a two-second slide is half of it");

                // The three palettes are three different things, and a projection that returned
                // the same one three times would pass a test that only checked sizes.
                player.update(2.0, false, false);
                List<Matrix> bones = player.getBoneTransforms();
                List<Matrix> world = player.getWorldTransforms();
                List<Matrix> skin = player.getSkinTransforms();
                assertEquals(2, world.size());
                assertEquals(2, skin.size());
                // Bone one's parent translates one unit along X, so its world transform carries
                // that and its bone transform does not.
                assertEquals(bones.get(1).M41 + 1f, world.get(1).M41, 1e-4f,
                        "a world transform is the local pose composed down the hierarchy");
                // And the skin transform is the world one with the inverse bind pose applied,
                // which for bone one removes two units along Y.
                assertNotEquals(world.get(1).M42, skin.get(1).M42,
                        "a skin transform is not a world transform");
            }
        });
    }

    @Test
    void aLoopingUpdateWrapsAndAClosedPlayerSaysSo() {
        CnbExtensionProbe.run(() -> {
            try (CnaSkinningData data = CnaSkinningData.of(skeleton(), clips())) {
                CnaAnimationPlayer player = CnaAnimationPlayer.of(data);
                try {
                    player.startClip("slide");
                    player.update(3.0, false, true);
                    assertTrue(player.getCurrentPosition() < 2.0,
                            "three seconds into a two-second loop wraps to one");
                    assertThrows(RuntimeException.class, () -> player.startClip("absent"));
                    assertThrows(NullPointerException.class, () -> player.startClip(null));
                } finally {
                    player.close();
                }
                player.close();
                assertThrows(IllegalStateException.class, player::getCurrentPosition);
                assertThrows(NullPointerException.class, () -> CnaAnimationPlayer.of(null));
            }
        });
    }

    @Test
    void aClosedSkinningDataSaysSo() {
        CnbExtensionProbe.run(() -> {
            CnaSkinningData data = CnaSkinningData.of(skeleton(), clips());
            data.close();
            data.close();
            assertThrows(IllegalStateException.class, data::getBoneCount);
            assertThrows(IllegalStateException.class, () -> data.getClip("slide"));
            assertThrows(NullPointerException.class,
                    () -> CnaSkinningData.of(null, Map.of()));
            assertThrows(NullPointerException.class,
                    () -> CnaSkinningData.of(skeleton(), null));
        });
    }
}
