package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Blend shapes: glTF's morph targets, which XNA's {@code Model} has no member for.
 *
 * <p>The last of the three descriptor graphs {@code JAVA-EXT-007} recorded as impassable, and the
 * widest: a base pose, one delta pair per target, a weight vector, and a weight track whose
 * keyframes each carry a weight vector and two tangent vectors. Seven parallel arrays cross the
 * boundary and every relationship between them is checked before anything is built.
 *
 * <p><strong>What is asserted is the blend.</strong> A base pose of three known vertices, two
 * targets that move them in different directions by different amounts, and weights that pick out
 * exactly one of them at a time -- so the vertices that come back are arithmetic rather than a
 * result code, and a projection that handed the second target's deltas to the first produces
 * visibly the wrong point.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnaMorphTargetDataTests {

    /**
     * Thirty-two bytes: position, normal and one texture coordinate.
     *
     * <p>Measured rather than chosen. CNA refuses any other size -- "morph target stride must be
     * 32, 52, or 56 bytes" -- because a blend has to know where the position and the normal sit
     * inside a vertex, and it recognises the layouts rather than being told. The first version of
     * this test used twelve, which is what found the rule.
     */
    private static final int STRIDE = 32;

    /** How many floats one vertex of that layout holds. */
    private static final int FLOATS = STRIDE / 4;

    /** One vertex: a position, a unit normal along Z and a texture coordinate. */
    private static float[] vertex(float x, float y, float z) {
        return new float[] {x, y, z, 0f, 0f, 1f, 0f, 0f};
    }

    private static byte[] basePose(float... values) {
        ByteBuffer buffer = ByteBuffer.allocate(values.length * 4).order(ByteOrder.LITTLE_ENDIAN);
        for (float value : values) {
            buffer.putFloat(value);
        }
        return buffer.array();
    }

    private static float[] verticesOf(byte[] bytes) {
        ByteBuffer buffer = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
        float[] values = new float[bytes.length / 4];
        for (int index = 0; index < values.length; index++) {
            values[index] = buffer.getFloat();
        }
        return values;
    }

    /** A base pose at the origin, and two targets that move it along different axes. */
    private static CnaMorphTargetData data() {
        float[] pose = new float[3 * FLOATS];
        System.arraycopy(vertex(0f, 0f, 0f), 0, pose, 0, FLOATS);
        System.arraycopy(vertex(1f, 0f, 0f), 0, pose, FLOATS, FLOATS);
        System.arraycopy(vertex(0f, 1f, 0f), 0, pose, FLOATS * 2, FLOATS);
        byte[] base = basePose(pose);
        CnaMorphTarget alongX = new CnaMorphTarget(
                List.of(new Vector3(10f, 0f, 0f), new Vector3(20f, 0f, 0f),
                        new Vector3(30f, 0f, 0f)),
                List.of());
        CnaMorphTarget alongY = new CnaMorphTarget(
                List.of(new Vector3(0f, 5f, 0f), new Vector3(0f, 6f, 0f),
                        new Vector3(0f, 7f, 0f)),
                List.of(new Vector3(0f, 0f, 1f), new Vector3(0f, 0f, 1f),
                        new Vector3(0f, 0f, 1f)));
        return CnaMorphTargetData.of(base, STRIDE, List.of(alongX, alongY),
                new float[] {0f, 0f}, CnaMorphWeightTrack.empty());
    }

    @Test
    void aBlendAppliesEachTargetInProportionToItsWeight() {
        CnbExtensionProbe.run(() -> {
            try (CnaMorphTargetData data = data()) {
                assertEquals(2, data.getTargetCount());
                assertEquals(STRIDE, data.getStride());
                float[] base = verticesOf(data.getBaseVertexBytes());
                assertEquals(3 * FLOATS, base.length);
                assertEquals(0f, base[0], 0f);
                assertEquals(1f, base[FLOATS], 0f, "vertex one is at x = 1");
                assertEquals(1f, base[FLOATS * 2 + 1], 0f, "and vertex two at y = 1");

                // Zero weights are the base pose exactly.
                assertArrayEquals(verticesOf(data.getBaseVertexBytes()),
                        verticesOf(data.blend(new float[] {0f, 0f})), 1e-5f);

                // The first target alone, at full weight: each vertex moves by its own delta,
                // and the three deltas differ, so a blend that applied one of them to all three
                // is caught.
                float[] first = verticesOf(data.blend(new float[] {1f, 0f}));
                assertEquals(10f, first[0], 1e-4f);
                assertEquals(21f, first[FLOATS], 1e-4f,
                        "vertex one was at x = 1 and moved twenty");
                assertEquals(30f, first[FLOATS * 2], 1e-4f);
                assertEquals(1f, first[FLOATS * 2 + 1], 1e-4f,
                        "and its y is untouched by an x-only target");

                // The second target alone moves along Y instead, so a projection that handed the
                // first target's deltas to the second produces an X displacement here.
                float[] second = verticesOf(data.blend(new float[] {0f, 1f}));
                assertEquals(0f, second[0], 1e-4f);
                assertEquals(5f, second[1], 1e-4f);
                assertEquals(8f, second[FLOATS * 2 + 1], 1e-4f,
                        "vertex two was at y = 1 and moved seven");
                assertNotEquals(first[0], second[0]);
                // The second target carries normal deltas and the first does not, so the
                // normals move for one and not the other. The moved one is RENORMALISED, which
                // is measured rather than assumed: a unit normal along Z given a delta along Z
                // stays a unit normal along Z, and only a delta that turns it shows the
                // difference. That is why the third target below exists.
                assertEquals(1f, first[5], 1e-4f, "an x-only target leaves the normal alone");

                // Half of each is half of each, which is what proportional means.
                float[] half = verticesOf(data.blend(new float[] {0.5f, 0.5f}));
                assertEquals(5f, half[0], 1e-4f);
                assertEquals(2.5f, half[1], 1e-4f);
            }
        });
    }

    @Test
    void aBlendedNormalIsRenormalisedRatherThanJustAdded() {
        CnbExtensionProbe.run(() -> {
            // One vertex, a unit normal along Z, and a target that turns it towards X. Adding
            // the delta gives (1, 0, 1), whose length is the square root of two; what CNA writes
            // is that vector normalised. A projection or a blend that only added would put 1.0
            // in the X slot, and half a delta would put 0.5 -- neither of which these are.
            float[] pose = vertex(0f, 0f, 0f);
            CnaMorphTarget turn = new CnaMorphTarget(List.of(new Vector3(10f, 0f, 0f)),
                    List.of(new Vector3(1f, 0f, 0f)));
            try (CnaMorphTargetData data = CnaMorphTargetData.of(basePose(pose), STRIDE,
                    List.of(turn), new float[] {0f}, CnaMorphWeightTrack.empty())) {
                float[] none = verticesOf(data.blend(new float[] {0f}));
                assertEquals(0f, none[3], 1e-5f);
                assertEquals(1f, none[5], 1e-5f, "no weight leaves the normal alone");

                float[] whole = verticesOf(data.blend(new float[] {1f}));
                assertEquals(10f, whole[0], 1e-4f, "and the position is not normalised");
                assertEquals((float) (1.0 / Math.sqrt(2.0)), whole[3], 1e-5f);
                assertEquals((float) (1.0 / Math.sqrt(2.0)), whole[5], 1e-5f);

                // Half the delta is (0.5, 0, 1) normalised, which is a different pair of
                // numbers from half of the whole one -- so the normalisation happens after the
                // weighting rather than before.
                float[] half = verticesOf(data.blend(new float[] {0.5f}));
                assertEquals(0.4472136f, half[3], 1e-5f);
                assertEquals(0.8944272f, half[5], 1e-5f);
            }
        });
    }

    @Test
    void eachTargetsDeltasComeBackAsItsOwn() {
        CnbExtensionProbe.run(() -> {
            try (CnaMorphTargetData data = data()) {
                List<Vector3> alongX = data.getPositionDeltas(0);
                assertEquals(3, alongX.size());
                assertEquals(10f, alongX.get(0).X, 1e-5f);
                assertEquals(30f, alongX.get(2).X, 1e-5f);
                assertEquals(0f, alongX.get(0).Y, 1e-5f);

                List<Vector3> alongY = data.getPositionDeltas(1);
                assertEquals(5f, alongY.get(0).Y, 1e-5f);
                assertEquals(7f, alongY.get(2).Y, 1e-5f);
                assertEquals(0f, alongY.get(0).X, 1e-5f,
                        "the second target's deltas are its own, not the first's");

                // Normal deltas are optional, and the two targets differ, so an empty list is a
                // real answer rather than a missing one.
                assertTrue(data.getNormalDeltas(0).isEmpty());
                assertEquals(3, data.getNormalDeltas(1).size());
                assertEquals(1f, data.getNormalDeltas(1).get(0).Z, 1e-5f);

                // Tangents are set afterwards, which is how a pipeline that derives them works.
                // Reading them before any are set is an empty answer for an in-range target: CNA
                // once bounded the index by its tangent array instead (JAVA-UPSTREAM-023) and
                // fixed it (BINDFIX-009).
                assertTrue(data.getTangentDeltas(0).isEmpty());

                data.setTangentDeltas(0, List.of(new Vector3(1f, 2f, 3f),
                        new Vector3(4f, 5f, 6f), new Vector3(7f, 8f, 9f)));
                assertEquals(3, data.getTangentDeltas(0).size());
                assertEquals(5f, data.getTangentDeltas(0).get(1).Y, 1e-5f);
                assertTrue(data.getTangentDeltas(1).isEmpty(),
                        "and setting one target's tangents does not set another's -- which is "
                                + "also what says the array is now sized per target");
            }
        });
    }

    @Test
    void theWeightsAndTheTriangleIndicesAreTheDataOwn() {
        CnbExtensionProbe.run(() -> {
            try (CnaMorphTargetData data = data()) {
                assertArrayEquals(new float[] {0f, 0f}, data.getWeights(), 0f);
                data.setWeights(new float[] {0.25f, 0.75f});
                assertArrayEquals(new float[] {0.25f, 0.75f}, data.getWeights(), 1e-6f);

                assertEquals(0, data.getTriangleIndices().length);
                data.setTriangleIndices(new int[] {0, 1, 2});
                assertArrayEquals(new int[] {0, 1, 2}, data.getTriangleIndices());

                assertFalse(data.getRecomputeFlatNormals());
                data.setRecomputeFlatNormals(true);
                assertTrue(data.getRecomputeFlatNormals());

                assertThrows(NullPointerException.class, () -> data.setWeights(null));
                assertThrows(NullPointerException.class, () -> data.setTriangleIndices(null));
            }
        });
    }

    private static void assertFalse(boolean value) {
        org.junit.jupiter.api.Assertions.assertFalse(value);
    }

    @Test
    void aWeightTrackAnimatesTheWeightsAndEvaluatesOnItsOwn() {
        CnbExtensionProbe.run(() -> {
            CnaMorphWeightTrack track = new CnaMorphWeightTrack(List.of(
                    new CnaMorphWeightKeyframe(0d, new float[] {0f, 1f},
                            new float[0], new float[0]),
                    new CnaMorphWeightKeyframe(2d, new float[] {1f, 0f},
                            new float[0], new float[0])), false, false);

            // A standalone evaluation, with no morph data attached: the two keyframes cross over,
            // so halfway is halfway in both weights and a track read in the wrong order would
            // produce the mirror image.
            assertArrayEquals(new float[] {0f, 1f}, track.evaluate(0d), 1e-5f);
            assertArrayEquals(new float[] {1f, 0f}, track.evaluate(2d), 1e-5f);
            assertArrayEquals(new float[] {0.5f, 0.5f}, track.evaluate(1d), 1e-5f);

            // A step track holds the lower keyframe's value instead, which is a different answer
            // at the same time -- and is what says the flag reached CNA.
            CnaMorphWeightTrack stepped = new CnaMorphWeightTrack(track.Keyframes(), true, false);
            assertArrayEquals(new float[] {0f, 1f}, stepped.evaluate(1d), 1e-5f);

            try (CnaMorphTargetData data = data()) {
                assertEquals(0, data.getWeightTrackInfo().KeyframeCount());
                data.setWeightTrack(track);
                CnaMorphTargetData.TrackInfo info = data.getWeightTrackInfo();
                assertEquals(2, info.KeyframeCount());
                assertFalse(info.StepInterpolation());
                assertFalse(info.CubicSpline());

                CnaMorphWeightKeyframe first = data.getWeightKeyframe(0);
                assertEquals(0d, first.TimeSeconds(), 0d);
                assertArrayEquals(new float[] {0f, 1f}, first.Weights(), 0f);
                CnaMorphWeightKeyframe second = data.getWeightKeyframe(1);
                assertEquals(2d, second.TimeSeconds(), 0d);
                assertArrayEquals(new float[] {1f, 0f}, second.Weights(), 0f,
                        "the two keyframes keep their own weights");

                data.setWeightTrack(stepped);
                assertTrue(data.getWeightTrackInfo().StepInterpolation());
            }
        });
    }

    @Test
    void aGraphWhoseCountsDoNotAddUpIsRefusedBeforeAnythingIsBuilt() {
        CnbExtensionProbe.run(() -> {
            long[] created = new long[1];
            // Two targets promised three deltas each and given three between them. Refused in
            // the adapter, before a single descriptor is allocated, because a sum that does not
            // add up would make CNA read past an array's end from inside C.
            assertEquals(1, org.openeggbert.cna.internal.NativeBindings.morphTargetDataCreate(
                    new byte[12], STRIDE, new int[] {3, 3}, new float[9], new int[] {0, 0},
                    new float[0], new float[] {0f, 0f}, new double[0], new int[0], new float[0],
                    new int[0], new float[0], new int[0], new float[0], false, false, created));
            // A delta array that is not a whole number of vectors.
            assertEquals(1, org.openeggbert.cna.internal.NativeBindings.morphTargetDataCreate(
                    new byte[12], STRIDE, new int[] {1}, new float[2], new int[] {0},
                    new float[0], new float[] {0f}, new double[0], new int[0], new float[0],
                    new int[0], new float[0], new int[0], new float[0], false, false, created));
            // A weight track whose keyframe counts disagree with its weight array.
            assertEquals(1, org.openeggbert.cna.internal.NativeBindings.morphTargetDataCreate(
                    new byte[12], STRIDE, new int[] {1}, new float[3], new int[] {0},
                    new float[0], new float[] {0f}, new double[] {0d}, new int[] {2},
                    new float[1], new int[] {0}, new float[0], new int[] {0}, new float[0],
                    false, false, created));

            assertThrows(NullPointerException.class, () -> CnaMorphTargetData.of(
                    null, STRIDE, List.of(), new float[0], CnaMorphWeightTrack.empty()));
            assertThrows(NullPointerException.class,
                    () -> new CnaMorphTarget(null, List.of()));
            assertThrows(NullPointerException.class,
                    () -> new CnaMorphWeightKeyframe(0d, null, new float[0], new float[0]));

            CnaMorphTargetData closed = data();
            closed.close();
            closed.close();
            assertThrows(IllegalStateException.class, closed::getTargetCount);
        });
    }
}
