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
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * A named set of animation clips, and posing a model's bones from one.
 *
 * <p>Nine routes sat behind "model animation clips are a CNA model extension", which explains that
 * XNA has no clip type and not why Java could not build one. The reason was the same pointer graph
 * the {@code .cnb} clip encoder was blocked on -- named clips to clips to tracks to keyframes --
 * and the same thing settled it: CNA states every array as borrowed for the call and copies what
 * it keeps, so the graph can be built by hand for one call's duration.
 *
 * <p>What this buys a game is the thing XNA left to its {@code SkinnedModel} sample and every
 * game that used it reimplemented: skeletal animation over an ordinary {@code Model}, with the
 * result landing in {@code getBoneTransforms}.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnaModelAnimationsTests {

    /** A clip that moves bone zero from the origin to (10, 0, 0) over two seconds. */
    private static CnbClip slide() {
        return new CnbClip(2.0, List.of(new CnbBoneTrack(0, List.of(
                new CnbKeyframe(0d, new Vector3(0f, 0f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f)),
                new CnbKeyframe(2.0, new Vector3(10f, 0f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f))))));
    }

    /** A clip that holds bone zero five units up Y, and nothing else. */
    private static CnbClip lift() {
        return new CnbClip(1.0, List.of(new CnbBoneTrack(0, List.of(
                new CnbKeyframe(0d, new Vector3(0f, 5f, 0f),
                        new Quaternion(0f, 0f, 0f, 1f), new Vector3(1f, 1f, 1f))))));
    }

    /**
     * Three clips, and the shapes matter.
     *
     * <p>The first two move bone zero along different axes by different amounts, so a set that
     * gave every clip the first one's tracks -- which is exactly what a flattening bug produces
     * -- poses the model identically for both and is caught. The third exists to be refused.
     */
    private static Map<String, CnbClip> threeClips() {
        Map<String, CnbClip> clips = new LinkedHashMap<>();
        clips.put("slide", slide());
        clips.put("lift", lift());
        clips.put("palette", lift());
        return clips;
    }

    @Test
    void aSetReportsEveryClipItWasBuiltFrom() {
        CnbExtensionProbe.run(() -> {
            try (CnaModelAnimations animations = CnaModelAnimations.of(threeClips())) {
                assertEquals(3, animations.size());
                // Measured, and not what the first version of this test assumed: the set orders
                // its clips BY NAME, not by the order they were given in. CNA's own container is
                // an ordered map keyed by the name, so "lift" comes before "palette" comes
                // before "slide" whatever order they arrived in -- which is why indexOf exists
                // and why a caller must not remember the position it passed a clip at.
                assertEquals(List.of("lift", "palette", "slide"),
                        List.of(animations.getClipName(0), animations.getClipName(1),
                                animations.getClipName(2)));
                assertEquals(2, animations.indexOf("slide"));
                assertEquals(0, animations.indexOf("lift"));
                assertThrows(IllegalArgumentException.class, () -> animations.indexOf("absent"));

                CnbAnimation slide = animations.getClip(animations.indexOf("slide"));
                assertEquals("slide", slide.Name());
                assertEquals(2.0, slide.DurationSeconds(), 0d);
                assertEquals(1, slide.TrackCount());

                CnbAnimation lift = animations.getClip(animations.indexOf("lift"));
                assertEquals(1.0, lift.DurationSeconds(), 0d);
                assertEquals(1, lift.TrackCount());
                // Different durations, so a set that gave both clips the first one's state
                // would be visible here rather than agreeing by accident.
                assertNotEquals(slide.DurationSeconds(), lift.DurationSeconds());

                assertNotNull(animations.getTypeName());
                assertTrue(animations.getTypeName().length() > 0);
            }
        });
    }

    @Test
    void aClipsTargetSpaceIsStatedRatherThanInferred() {
        CnbExtensionProbe.run(() -> {
            try (CnaModelAnimations animations = CnaModelAnimations.of(threeClips())) {
                int slide = animations.indexOf("slide");
                int palette = animations.indexOf("palette");
                animations.setClipTargetSpace(slide, CnbClipTargetSpace.SceneNode);
                animations.setClipTargetSpace(palette, CnbClipTargetSpace.JointPalette);
                assertEquals(CnbClipTargetSpace.SceneNode,
                        animations.getClip(slide).TargetSpace());
                assertEquals(CnbClipTargetSpace.JointPalette,
                        animations.getClip(palette).TargetSpace(),
                        "the clips keep different spaces, so neither is a default");

                assertThrows(RuntimeException.class,
                        () -> animations.setClipTargetSpace(9, CnbClipTargetSpace.SceneNode));
                assertThrows(RuntimeException.class, () -> animations.getClip(9));
                assertThrows(NullPointerException.class,
                        () -> animations.setClipTargetSpace(0, null));
            }
        });
    }

    @Test
    void aClosedSetSaysSoAndAnEmptySetIsASet() {
        CnbExtensionProbe.run(() -> {
            try (CnaModelAnimations empty = CnaModelAnimations.of(Map.of())) {
                assertEquals(0, empty.size(), "no clips is a set, not a refusal");
            }
            CnaModelAnimations animations = CnaModelAnimations.of(threeClips());
            animations.close();
            animations.close();
            assertThrows(IllegalStateException.class, animations::size);
            assertThrows(IllegalStateException.class, () -> animations.getClipName(0));
            assertThrows(NullPointerException.class, () -> CnaModelAnimations.of(null));
        });
    }

    @Test
    void aSceneNodeClipMovesTheModelsBonesAndAJointPaletteClipIsRefused() {
        CnaModelProbe.run(model -> {
            List<Matrix> before = model.getBoneTransforms();
            try (CnaModelAnimations animations = CnaModelAnimations.of(threeClips())) {
                int slide = animations.indexOf("slide");
                int lift = animations.indexOf("lift");
                int palette = animations.indexOf("palette");
                animations.setClipTargetSpace(slide, CnbClipTargetSpace.SceneNode);
                animations.setClipTargetSpace(lift, CnbClipTargetSpace.SceneNode);
                animations.setClipTargetSpace(palette, CnbClipTargetSpace.JointPalette);

                model.applyClipToBones(animations, slide, 0d);
                List<Matrix> atStart = model.getBoneTransforms();
                model.applyClipToBones(animations, slide, 2.0);
                List<Matrix> atEnd = model.getBoneTransforms();

                // The clip slides bone zero ten units along X over two seconds, so the two poses
                // differ by exactly that. A projection that dropped the keyframes, or evaluated
                // the wrong clip, produces two identical poses.
                assertEquals(atStart.get(0).M41 + 10f, atEnd.get(0).M41, 1e-4f,
                        "the pose at the end of the clip is ten units along X from the start");
                assertNotEquals(atStart.get(0).M41, atEnd.get(0).M41);

                // Halfway is halfway: the clip is interpolated rather than snapped to keyframes.
                model.applyClipToBones(animations, slide, 1.0);
                assertEquals(atStart.get(0).M41 + 5f, model.getBoneTransforms().get(0).M41, 1e-4f,
                        "one second into a two-second slide is half of it");

                // A time past the end is clamped rather than refused, which the header states.
                model.applyClipToBones(animations, slide, 99d);
                assertEquals(atEnd.get(0).M41, model.getBoneTransforms().get(0).M41, 1e-4f);

                // The second clip lifts along Y rather than sliding along X, and it is the one
                // a set that gave every clip the first's tracks would get wrong: it would slide
                // instead, and X rather than Y would move.
                model.applyClipToBones(animations, lift, 0d);
                List<Matrix> lifted = model.getBoneTransforms();
                assertEquals(atStart.get(0).M42 + 5f, lifted.get(0).M42, 1e-4f,
                        "the second clip lifts five units along Y");
                assertEquals(atStart.get(0).M41, lifted.get(0).M41, 1e-4f,
                        "and does not slide along X, which the first clip does");

                // And the refusal that keeps a joint-palette clip from posing model bones,
                // which would otherwise pose the wrong ones without saying so.
                assertThrows(RuntimeException.class, () -> model.applyClipToBones(animations, palette, 0d));
                assertThrows(RuntimeException.class, () -> model.applyClipToBones(animations, 9, 0d));
                assertThrows(NullPointerException.class, () -> model.applyClipToBones(null, 0, 0d));
            }
            assertEquals(before.size(), model.getBoneTransforms().size());
        });
    }
}
