package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.BoundingBox;
import Microsoft.Xna.Framework.BoundingFrustum;
import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The debug renderer, against the live runtime.
 *
 * <p><strong>What this can and cannot say.</strong> The renderer needs a real graphics device, so
 * it runs inside a game -- the probe in {@code tools/native-abi/probes} found it refuses an
 * invalid one. Nothing here claims a line was drawn. What it does check is the queue, which is the
 * part a game's own logic
 * depends on and which is fully readable: a box really is twelve edges, the two lists really are
 * separate, and the vertices come back in submission order with the colours they were queued
 * with.
 *
 * <p>The debug renderer belongs to CNA's extended graphics layer ({@code CNA_CNAEXT}). On a build
 * without it every test asserts the refusal instead -- {@link ExtensionNotSupportedException}, not
 * an object that silently queues nothing -- which is what {@link #withDebugDraw} decides.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class DebugDrawTests {

    @Test
    void theQueueHoldsTheEdgesEachShapeIsMadeOf() {
        withDebugDraw(debug -> {
            {
                debug.begin(Matrix.getIdentity(), Matrix.getIdentity());
                assertEquals(0, debug.getLineCount(), "begin clears both lists");
                assertTrue(debug.isDepthTested(), "begin restores depth testing");

                debug.addLine(new Vector3(0f, 0f, 0f), new Vector3(1f, 2f, 3f), Color.Red);
                assertEquals(1, debug.getLineCount());

                List<DebugVertex> vertices = debug.readVertices(true);
                assertEquals(2, vertices.size(), "a line is two vertices");
                assertEquals(new Vector3(0f, 0f, 0f), vertices.get(0).Position());
                assertEquals(new Vector3(1f, 2f, 3f), vertices.get(1).Position());
                assertEquals(Color.Red, vertices.get(0).Color());

                // A box is twelve edges, which is the fiddly count this exists to get right.
                debug.clear();
                debug.addBox(new BoundingBox(new Vector3(-1f, -1f, -1f), new Vector3(1f, 1f, 1f)),
                        Color.Lime);
                assertEquals(12, debug.getLineCount());
                assertEquals(24, debug.readVertices(true).size());

                // A cross is three segments, one per axis.
                debug.clear();
                debug.addCross(new Vector3(5f, 0f, 0f), 1.0f, Color.White);
                assertEquals(3, debug.getLineCount());

                // A sphere is three rings, so its edge count follows the segment count.
                debug.clear();
                debug.addSphere(new Vector3(0f, 0f, 0f), 2.0f, Color.Blue, 8);
                int coarse = debug.getLineCount();
                debug.clear();
                debug.addSphere(new Vector3(0f, 0f, 0f), 2.0f, Color.Blue, 16);
                assertTrue(debug.getLineCount() > coarse,
                        "more segments must mean more edges: " + coarse
                        + " then " + debug.getLineCount());

                debug.clear();
                debug.addBoundingSphere(new BoundingSphere(new Vector3(0f, 0f, 0f), 2.0f),
                        Color.Blue, 8);
                assertEquals(coarse, debug.getLineCount(),
                        "a bounding sphere is the same three rings");

                // A frustum is twelve edges too, and takes XNA's own type.
                debug.clear();
                debug.addFrustum(new BoundingFrustum(Matrix.CreatePerspectiveFieldOfView(
                        1.0f, 1.0f, 1.0f, 100.0f)), Color.Yellow);
                assertEquals(12, debug.getLineCount());

                assertThrows(NullPointerException.class,
                        () -> debug.addLine(null, new Vector3(), Color.Red));
                assertThrows(NullPointerException.class,
                        () -> debug.addBox(new BoundingBox(), null));
            }
        });
    }

    @Test
    void theTwoListsAreSeparate() {
        withDebugDraw(debug -> {
            {
                debug.begin(Matrix.getIdentity(), Matrix.getIdentity());

                // A collision volume you want hidden behind geometry, and a marker you must not
                // lose behind a wall: two different lists, and the count spans both.
                debug.addLine(new Vector3(0f, 0f, 0f), new Vector3(1f, 0f, 0f), Color.Red);
                debug.setDepthTested(false);
                assertFalse(debug.isDepthTested());
                debug.addLine(new Vector3(0f, 0f, 0f), new Vector3(0f, 1f, 0f), Color.Lime);
                debug.addLine(new Vector3(0f, 0f, 0f), new Vector3(0f, 0f, 1f), Color.Blue);

                assertEquals(3, debug.getLineCount(), "the count spans both lists");
                assertEquals(2, debug.readVertices(true).size(), "one depth-tested line");
                assertEquals(4, debug.readVertices(false).size(), "two overlay lines");
                assertEquals(Color.Lime, debug.readVertices(false).get(0).Color());

                debug.begin(Matrix.getIdentity(), Matrix.getIdentity());
                assertEquals(0, debug.getLineCount(), "begin clears both");
                assertTrue(debug.isDepthTested(), "and restores depth testing");
            }
        });
    }

    @Test
    void aClosedRendererRefusesEveryOperation() {
        withDebugDraw(debug -> {
            debug.close();
            debug.close();
            assertThrows(IllegalStateException.class, debug::getLineCount);
        });
        assertThrows(NullPointerException.class, () -> DebugDraw.create(null));
    }

    /** Runs a body over a debug renderer, or asserts the refusal on a build without the layer. */
    private static void withDebugDraw(java.util.function.Consumer<DebugDraw> body) {
        GameProbe.run(probe -> {
            if (!GraphicsExtension.isAvailable()) {
                assertThrows(ExtensionNotSupportedException.class,
                        () -> DebugDraw.create(probe.device()));
                return;
            }
            try (DebugDraw debug = DebugDraw.create(probe.device())) {
                body.accept(debug);
            }
        });
    }
}
