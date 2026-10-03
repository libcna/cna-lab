package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeModelExtensionRoutes;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * Blend shapes: a base pose and the per-vertex deltas that move it towards other shapes.
 *
 * <p>A CNA extension, and glTF's morph targets. XNA 4.0's {@code Model} has no morph data at all,
 * so a face that smiles or a cloth that billows had to be a second mesh in XNA and is one mesh
 * plus a weight here.
 *
 * <p><strong>Blending is what the data is for and is where the arithmetic is.</strong>
 * {@link #blend} takes one weight per target and writes the base pose with every delta applied,
 * in the same vertex layout the base pose came in -- so the result goes straight into a vertex
 * buffer. A weight of zero contributes nothing and a weight of one contributes the whole delta.
 *
 * <p>The handle is owned; {@link #close()} releases it. CNA copies every array, so the values
 * handed in stay the caller's.
 */
public final class CnaMorphTargetData implements AutoCloseable {

    /** How many floats one delta occupies where CNA takes an array of them. */
    private static final int DELTA_FLOATS = 3;

    private final long handle;

    /** Whether closing this frees the data, which a view a mesh part lent back does not. */
    private final boolean owned;
    private boolean closed;

    private CnaMorphTargetData(long handle, boolean owned) {
        this.handle = handle;
        this.owned = owned;
    }

    /**
     * Creates morph-target data from a base pose and one delta set per target.
     *
     * @param baseVertexBytes the base pose's vertices, in the layout the blend result keeps
     * @param stride one base vertex's byte stride
     * @param targets each target's position and normal deltas
     * @param weights the starting weight per target
     * @param weightTrack how the weights change over time, or {@link CnaMorphWeightTrack#empty()}
     * @return the data, which the caller closes
     */
    public static CnaMorphTargetData of(byte[] baseVertexBytes, int stride,
            List<CnaMorphTarget> targets, float[] weights, CnaMorphWeightTrack weightTrack) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(baseVertexBytes, "baseVertexBytes");
        Objects.requireNonNull(targets, "targets");
        Objects.requireNonNull(weights, "weights");
        Objects.requireNonNull(weightTrack, "weightTrack");
        int[] positionCounts = new int[targets.size()];
        int[] normalCounts = new int[targets.size()];
        int positionTotal = 0;
        int normalTotal = 0;
        for (int index = 0; index < targets.size(); index++) {
            CnaMorphTarget target = Objects.requireNonNull(targets.get(index), "target");
            positionCounts[index] = target.PositionDeltas().size();
            normalCounts[index] = target.NormalDeltas().size();
            positionTotal += positionCounts[index];
            normalTotal += normalCounts[index];
        }
        float[] positionDeltas = new float[positionTotal * DELTA_FLOATS];
        float[] normalDeltas = new float[normalTotal * DELTA_FLOATS];
        int positionAt = 0;
        int normalAt = 0;
        for (CnaMorphTarget target : targets) {
            for (Vector3 delta : target.PositionDeltas()) {
                positionDeltas[positionAt++] = delta.X;
                positionDeltas[positionAt++] = delta.Y;
                positionDeltas[positionAt++] = delta.Z;
            }
            for (Vector3 delta : target.NormalDeltas()) {
                normalDeltas[normalAt++] = delta.X;
                normalDeltas[normalAt++] = delta.Y;
                normalDeltas[normalAt++] = delta.Z;
            }
        }
        CnaMorphWeightTrack.Flattened track = weightTrack.flatten();
        long[] created = new long[1];
        CnbExtension.check("CnaMorphTargetData.of", NativeBindings.morphTargetDataCreate(
                baseVertexBytes.clone(), stride, positionCounts, positionDeltas, normalCounts,
                normalDeltas, weights.clone(), track.times(), track.weightCounts(),
                track.weights(), track.inCounts(), track.inTangents(), track.outCounts(),
                track.outTangents(), weightTrack.StepInterpolation(), weightTrack.CubicSpline(),
                created));
        return new CnaMorphTargetData(created[0], true);
    }

    /**
     * Wraps a handle a mesh part owns, so closing this view takes nothing away.
     *
     * <p>Readable like any other, and its {@link #close()} is a no-op: the part is what holds the
     * data, and freeing it from here would take a part's morph data out from under it.
     */
    static CnaMorphTargetData borrow(long handle) {
        return new CnaMorphTargetData(handle, false);
    }

    /** The native handle, for the mesh part that retains it. */
    long handle() {
        return open();
    }

    /**
     * Returns how many morph targets the data holds.
     *
     * @return the target count
     */
    public int getTargetCount() {
        long[] count = new long[1];
        check("getTargetCount",
                NativeModelExtensionRoutes.morphTargetDataGetTargetCount(open(), count));
        return Math.toIntExact(count[0]);
    }

    /**
     * Returns one base vertex's byte stride.
     *
     * @return the stride
     */
    public int getStride() {
        int[] stride = new int[1];
        check("getStride", NativeModelExtensionRoutes.morphTargetDataGetStride(open(), stride));
        return stride[0];
    }

    /**
     * Returns the base pose's vertex bytes.
     *
     * @return the bytes
     */
    public byte[] getBaseVertexBytes() {
        long[] bytes = new long[1];
        check("getBaseVertexBytes",
                NativeModelExtensionRoutes.morphTargetDataGetBaseVertexByteCount(open(), bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        check("getBaseVertexBytes", NativeModelExtensionRoutes
                .morphTargetDataCopyBaseVertexBytes(open(), destination, bytes));
        return destination;
    }

    /**
     * Returns one target's position deltas.
     *
     * @param targetIndex the zero-based target index
     * @return the deltas
     */
    public List<Vector3> getPositionDeltas(int targetIndex) {
        return deltas("getPositionDeltas", targetIndex,
                NativeModelExtensionRoutes::morphTargetDataCopyPositionDeltas);
    }

    /**
     * Returns one target's normal deltas, empty when it has none.
     *
     * @param targetIndex the zero-based target index
     * @return the deltas
     */
    public List<Vector3> getNormalDeltas(int targetIndex) {
        return deltas("getNormalDeltas", targetIndex,
                NativeModelExtensionRoutes::morphTargetDataCopyNormalDeltas);
    }

    /**
     * Returns one target's tangent deltas, empty when it has none.
     *
     * @param targetIndex the zero-based target index
     * @return the deltas
     */
    public List<Vector3> getTangentDeltas(int targetIndex) {
        return deltas("getTangentDeltas", targetIndex,
                NativeModelExtensionRoutes::morphTargetDataCopyTangentDeltas);
    }

    /**
     * Sets one target's tangent deltas.
     *
     * <p>Separate from creation because glTF's tangent deltas are optional and are often derived
     * rather than authored: a pipeline that computes them fills them in afterwards.
     *
     * @param targetIndex the zero-based target index
     * @param deltas the deltas; an empty list clears them
     */
    public void setTangentDeltas(int targetIndex, List<Vector3> deltas) {
        Objects.requireNonNull(deltas, "deltas");
        float[] leaves = new float[deltas.size() * DELTA_FLOATS];
        int at = 0;
        for (Vector3 delta : deltas) {
            leaves[at++] = delta.X;
            leaves[at++] = delta.Y;
            leaves[at++] = delta.Z;
        }
        check("setTangentDeltas", NativeModelExtensionRoutes
                .morphTargetDataSetTangentDeltas(open(), targetIndex, leaves));
    }

    /**
     * Returns the triangle indices a flat-normal recomputation walks.
     *
     * @return the indices
     */
    public int[] getTriangleIndices() {
        long[] written = new long[1];
        int probe = NativeModelExtensionRoutes.morphTargetDataCopyTriangleIndicesExt(
                open(), new int[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check("getTriangleIndices", probe);
        }
        int[] destination = new int[Math.toIntExact(written[0])];
        check("getTriangleIndices", NativeModelExtensionRoutes
                .morphTargetDataCopyTriangleIndicesExt(open(), destination, written));
        return destination;
    }

    /**
     * Sets the triangle indices a flat-normal recomputation walks.
     *
     * @param indices three per triangle
     */
    public void setTriangleIndices(int[] indices) {
        Objects.requireNonNull(indices, "indices");
        check("setTriangleIndices", NativeModelExtensionRoutes
                .morphTargetDataSetTriangleIndicesExt(open(), indices.clone()));
    }

    /**
     * Reports whether a blend recomputes flat normals from the triangles.
     *
     * @return whether it does
     */
    public boolean getRecomputeFlatNormals() {
        boolean[] recompute = new boolean[1];
        check("getRecomputeFlatNormals", NativeModelExtensionRoutes
                .morphTargetDataGetRecomputeFlatNormalsExt(open(), recompute));
        return recompute[0];
    }

    /**
     * Sets whether a blend recomputes flat normals from the triangles.
     *
     * @param recompute whether it should
     */
    public void setRecomputeFlatNormals(boolean recompute) {
        check("setRecomputeFlatNormals", NativeModelExtensionRoutes
                .morphTargetDataSetRecomputeFlatNormalsExt(open(), recompute));
    }

    /**
     * Returns the current weight per target.
     *
     * @return the weights
     */
    public float[] getWeights() {
        long[] written = new long[1];
        int probe = NativeModelExtensionRoutes.morphTargetDataCopyWeights(
                open(), new float[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check("getWeights", probe);
        }
        float[] destination = new float[Math.toIntExact(written[0])];
        check("getWeights", NativeModelExtensionRoutes
                .morphTargetDataCopyWeights(open(), destination, written));
        return destination;
    }

    /**
     * Sets the current weight per target.
     *
     * @param weights one weight per target
     */
    public void setWeights(float[] weights) {
        Objects.requireNonNull(weights, "weights");
        check("setWeights",
                NativeModelExtensionRoutes.morphTargetDataSetWeights(open(), weights.clone()));
    }

    /**
     * Returns the weight track's shape: how many keyframes it has and how it interpolates.
     *
     * @return the keyframe count, and the two interpolation flags
     */
    public TrackInfo getWeightTrackInfo() {
        long[] keyframes = new long[1];
        boolean[] step = new boolean[1];
        boolean[] cubic = new boolean[1];
        check("getWeightTrackInfo", NativeModelExtensionRoutes
                .morphTargetDataGetWeightTrackInfo(open(), keyframes, step, cubic));
        return new TrackInfo(Math.toIntExact(keyframes[0]), step[0], cubic[0]);
    }

    /**
     * Returns one weight keyframe.
     *
     * @param index the zero-based keyframe index
     * @return the keyframe
     */
    public CnaMorphWeightKeyframe getWeightKeyframe(int index) {
        double[] time = new double[1];
        long[] weights = new long[1];
        long[] inTangents = new long[1];
        long[] outTangents = new long[1];
        int probe = NativeModelExtensionRoutes.morphTargetDataCopyWeightKeyframe(
                open(), index, time, new float[0], weights, new float[0], inTangents,
                new float[0], outTangents);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check("getWeightKeyframe", probe);
        }
        float[] weightValues = new float[Math.toIntExact(weights[0])];
        float[] inValues = new float[Math.toIntExact(inTangents[0])];
        float[] outValues = new float[Math.toIntExact(outTangents[0])];
        check("getWeightKeyframe", NativeModelExtensionRoutes
                .morphTargetDataCopyWeightKeyframe(open(), index, time, weightValues, weights,
                        inValues, inTangents, outValues, outTangents));
        return new CnaMorphWeightKeyframe(time[0], weightValues, inValues, outValues);
    }

    /**
     * Replaces the weight track.
     *
     * @param track the new track
     */
    public void setWeightTrack(CnaMorphWeightTrack track) {
        Objects.requireNonNull(track, "track");
        CnaMorphWeightTrack.Flattened flat = track.flatten();
        check("setWeightTrack", NativeBindings.morphTargetDataSetWeightTrack(open(),
                flat.times(), flat.weightCounts(), flat.weights(), flat.inCounts(),
                flat.inTangents(), flat.outCounts(), flat.outTangents(),
                track.StepInterpolation(), track.CubicSpline()));
    }

    /**
     * Blends the base pose with the deltas at the weights given, into vertex bytes.
     *
     * <p>The result is in the base pose's own layout, so it goes straight into a vertex buffer.
     *
     * @param weights one weight per target
     * @return the blended vertices
     */
    public byte[] blend(float[] weights) {
        Objects.requireNonNull(weights, "weights");
        long[] written = new long[1];
        int probe = NativeModelExtensionRoutes.morphTargetDataBlend(
                open(), weights.clone(), new byte[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check("blend", probe);
        }
        byte[] destination = new byte[Math.toIntExact(written[0])];
        check("blend", NativeModelExtensionRoutes.morphTargetDataBlend(
                open(), weights.clone(), destination, written));
        return destination;
    }

    /**
     * Releases the morph data, when this is the owning handle rather than a part's view.
     *
     * <p>Closing twice is a no-op, and closing a borrowed view does nothing at all.
     */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        if (owned) {
            check("close", NativeModelExtensionRoutes.morphTargetDataDestroy(handle));
        }
    }

    private interface DeltaReader {
        int read(long data, long targetIndex, float[] destination, long[] outCount);
    }

    private List<Vector3> deltas(String operation, int targetIndex, DeltaReader reader) {
        long[] written = new long[1];
        int probe = reader.read(open(), targetIndex, new float[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check(operation, probe);
        }
        int count = Math.toIntExact(written[0]);
        if (count == 0) {
            return List.of();
        }
        float[] leaves = new float[count * DELTA_FLOATS];
        check(operation, reader.read(open(), targetIndex, leaves, written));
        List<Vector3> values = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            values.add(new Vector3(leaves[index * DELTA_FLOATS],
                    leaves[index * DELTA_FLOATS + 1], leaves[index * DELTA_FLOATS + 2]));
        }
        return Collections.unmodifiableList(values);
    }

    private static void check(String operation, int result) {
        CnbExtension.check("CnaMorphTargetData." + operation, result);
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this CnaMorphTargetData is closed");
        }
        return handle;
    }

    /**
     * A weight track's shape, without its keyframes.
     *
     * @param KeyframeCount how many keyframes the track has
     * @param StepInterpolation whether evaluation holds the lower keyframe's value
     * @param CubicSpline whether evaluation uses the keyframes' tangents
     */
    public record TrackInfo(int KeyframeCount, boolean StepInterpolation, boolean CubicSpline) {
    }
}
