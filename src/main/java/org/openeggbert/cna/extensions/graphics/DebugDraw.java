package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.BoundingBox;
import Microsoft.Xna.Framework.BoundingFrustum;
import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * Draws wireframe geometry over a scene, for looking at what a game thinks is true.
 *
 * <p>A CNA extension. XNA has no debug renderer, so every game that wanted to see a bounding box
 * or a frustum built a {@code BasicEffect} and a vertex buffer for it. This queues lines and draws
 * them in one call, and it knows the shapes worth drawing: boxes, spheres, frusta and crosses,
 * each of which is a fiddly handful of edges to get right by hand.
 *
 * <p><strong>Two lists, not one.</strong> Lines added while {@link #setDepthTested} is true are
 * hidden by geometry in front of them, which is what you want for a collision volume; lines added
 * while it is false are drawn on top, which is what you want for a marker you must not lose behind
 * a wall. {@link #begin} restores depth testing, so a frame always starts the same way.
 *
 * <p>The queue is readable with {@link #getLineCount()} and {@link #readVertices}, which is what
 * makes this testable without looking at pixels -- a box really is twelve edges, and the vertices
 * come back in submission order.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op.
 */
public final class DebugDraw implements AutoCloseable {

    /** CNA's own result for a buffer that could not hold the answer. */
    private static final int RESULT_BUFFER_TOO_SMALL = 14;

    /** Four integral leaves to a vertex -- the colour's four channels -- and three floats. */
    private static final int VERTEX_INTEGRAL_LEAVES = 4;
    private static final int VERTEX_FLOAT_LEAVES = 3;

    private final long handle;
    private boolean closed;

    private DebugDraw(long handle) {
        this.handle = handle;
    }

    /**
     * Creates a debug renderer on a device.
     *
     * @param graphicsDevice the device to draw with
     * @return the renderer, which the caller closes
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static DebugDraw create(GraphicsDevice graphicsDevice) {
        GraphicsExtension.requireBackend();
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        long[] debug = new long[1];
        GraphicsExtension.check("DebugDraw.create", NativeGraphicsExtensionRoutes.debugDrawCreate(
                NativeBindings.nativeGraphicsDeviceValue(graphicsDevice), debug));
        return new DebugDraw(debug[0]);
    }

    /**
     * Opens a frame, clearing both line lists and restoring depth testing.
     *
     * @param view the camera's view matrix
     * @param projection the camera's projection matrix
     */
    public void begin(Matrix view, Matrix projection) {
        Objects.requireNonNull(view, "view");
        Objects.requireNonNull(projection, "projection");
        GraphicsExtension.check("DebugDraw.begin", NativeGraphicsExtensionRoutes
                .debugDrawBegin(open(), floats(view), floats(projection)));
    }

    /** Draws both line lists and closes the frame. */
    public void end() {
        GraphicsExtension.check("DebugDraw.end", NativeGraphicsExtensionRoutes.debugDrawEnd(open()));
    }

    /** Discards both line lists without drawing them. */
    public void clear() {
        GraphicsExtension.check("DebugDraw.clear",
                NativeGraphicsExtensionRoutes.debugDrawClear(open()));
    }

    /**
     * Queues one line.
     *
     * @param from where the line starts
     * @param to where it ends
     * @param color the colour to draw it in
     */
    public void addLine(Vector3 from, Vector3 to, Color color) {
        GraphicsExtension.check("DebugDraw.addLine", NativeGraphicsExtensionRoutes.debugDrawAddLine(
                open(), floats(from, "from"), floats(to, "to"), channels(color)));
    }

    /**
     * Queues the twelve edges of a box.
     *
     * @param bounds the box to outline
     * @param color the colour to draw it in
     */
    public void addBox(BoundingBox bounds, Color color) {
        Objects.requireNonNull(bounds, "bounds");
        GraphicsExtension.check("DebugDraw.addBox", NativeGraphicsExtensionRoutes.debugDrawAddBox(
                open(),
                new float[] {bounds.Min.X, bounds.Min.Y, bounds.Min.Z,
                    bounds.Max.X, bounds.Max.Y, bounds.Max.Z},
                channels(color)));
    }

    /**
     * Queues three rings approximating a sphere.
     *
     * @param center the sphere's centre
     * @param radius its radius
     * @param color the colour to draw it in
     * @param segments how many segments each ring is drawn with
     */
    public void addSphere(Vector3 center, float radius, Color color, int segments) {
        GraphicsExtension.check("DebugDraw.addSphere", NativeGraphicsExtensionRoutes
                .debugDrawAddSphere(open(), floats(center, "center"), radius,
                        channels(color), segments));
    }

    /**
     * Queues three rings approximating a bounding sphere.
     *
     * @param sphere the sphere to outline
     * @param color the colour to draw it in
     * @param segments how many segments each ring is drawn with
     */
    public void addBoundingSphere(BoundingSphere sphere, Color color, int segments) {
        Objects.requireNonNull(sphere, "sphere");
        GraphicsExtension.check("DebugDraw.addBoundingSphere", NativeGraphicsExtensionRoutes
                .debugDrawAddBoundingSphere(open(),
                        new float[] {sphere.Center.X, sphere.Center.Y, sphere.Center.Z,
                            sphere.Radius},
                        channels(color), segments));
    }

    /**
     * Queues the twelve edges of a frustum.
     *
     * @param frustum the frustum to outline
     * @param color the colour to draw it in
     */
    public void addFrustum(BoundingFrustum frustum, Color color) {
        Objects.requireNonNull(frustum, "frustum");
        GraphicsExtension.check("DebugDraw.addFrustum", NativeGraphicsExtensionRoutes
                .debugDrawAddFrustum(open(), floats(frustum.getMatrix()), channels(color)));
    }

    /**
     * Queues three axis-aligned segments crossing at a point.
     *
     * @param position where the cross sits
     * @param size how far each arm extends
     * @param color the colour to draw it in
     */
    public void addCross(Vector3 position, float size, Color color) {
        GraphicsExtension.check("DebugDraw.addCross", NativeGraphicsExtensionRoutes
                .debugDrawAddCross(open(), floats(position, "position"), size,
                        channels(color)));
    }

    /** Reports which list the following lines go into. */
    public boolean isDepthTested() {
        boolean[] tested = new boolean[1];
        GraphicsExtension.check("DebugDraw.isDepthTested",
                NativeGraphicsExtensionRoutes.debugDrawIsDepthTested(open(), tested));
        return tested[0];
    }

    /**
     * Chooses which list the following lines go into.
     *
     * @param depthTested true to let geometry hide them, false to draw them on top
     */
    public void setDepthTested(boolean depthTested) {
        GraphicsExtension.check("DebugDraw.setDepthTested",
                NativeGraphicsExtensionRoutes.debugDrawSetDepthTested(open(), depthTested));
    }

    /** Returns how many lines are queued, across both lists. */
    public int getLineCount() {
        int[] count = new int[1];
        GraphicsExtension.check("DebugDraw.getLineCount",
                NativeGraphicsExtensionRoutes.debugDrawGetLineCount(open(), count));
        return count[0];
    }

    /**
     * Returns one of the two vertex lists, in submission order.
     *
     * @param depthTested which list to read
     * @return two vertices per line, each with its position and colour
     */
    public List<DebugVertex> readVertices(boolean depthTested) {
        long debug = open();
        long[] count = new long[1];
        // A zero-capacity probe reports the count and writes nothing, so BUFFER_TOO_SMALL is the
        // expected answer to the first call rather than a failure.
        int probe = NativeGraphicsExtensionRoutes.debugDrawCopyVertices(
                debug, depthTested, new long[0], new float[0], count);
        if (probe != RESULT_BUFFER_TOO_SMALL) {
            GraphicsExtension.check("DebugDraw.readVertices", probe);
        }
        int vertices = Math.toIntExact(count[0]);
        long[] integral = new long[Math.multiplyExact(vertices, VERTEX_INTEGRAL_LEAVES)];
        float[] floating = new float[Math.multiplyExact(vertices, VERTEX_FLOAT_LEAVES)];
        GraphicsExtension.check("DebugDraw.readVertices", NativeGraphicsExtensionRoutes
                .debugDrawCopyVertices(debug, depthTested, integral, floating, count));
        List<DebugVertex> read = new ArrayList<>(vertices);
        for (int vertex = 0; vertex < count[0]; vertex++) {
            int colour = vertex * VERTEX_INTEGRAL_LEAVES;
            int position = vertex * VERTEX_FLOAT_LEAVES;
            read.add(new DebugVertex(
                    new Vector3(floating[position], floating[position + 1],
                            floating[position + 2]),
                    new Color((int) integral[colour], (int) integral[colour + 1],
                            (int) integral[colour + 2], (int) integral[colour + 3])));
        }
        return List.copyOf(read);
    }

    /** Releases the renderer. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        GraphicsExtension.check("DebugDraw.close",
                NativeGraphicsExtensionRoutes.debugDrawDestroy(handle));
    }

    private static long[] channels(Color color) {
        return EngineValues.channels(color, "color");
    }

    private static float[] floats(Vector3 value, String name) {
        return EngineValues.floats(value, name);
    }

    private static float[] floats(Matrix matrix) {
        return EngineValues.floats(matrix, "matrix");
    }

    private static float[] floats(Matrix matrix, String name) {
        return EngineValues.floats(matrix, name);
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This DebugDraw is closed");
            }
        }
        return handle;
    }
}
