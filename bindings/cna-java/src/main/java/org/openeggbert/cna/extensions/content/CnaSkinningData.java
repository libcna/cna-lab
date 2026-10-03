package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeModelExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * A skeleton and the clips that animate it: everything a skinned mesh is posed from.
 *
 * <p>A CNA extension, and the piece XNA never shipped. XNA 4.0's skinning is a sample: the
 * {@code SkinnedModel} sample defined a {@code SkinningData} type, a {@code Keyframe}, an
 * {@code AnimationClip} and an {@code AnimationPlayer}, and every game that wanted a skinned
 * character carried a copy of all four. This is that runtime, in CNA, reachable from Java.
 *
 * <p><strong>It was recorded as having no door in, and it had one.</strong> CNA builds skinning
 * data from a descriptor whose fields are pointers to arrays of descriptors -- skeleton matrices,
 * and named clips holding tracks holding keyframes -- and the JNI generator refuses a shape it
 * cannot read off the declaration. That refusal is right and stands. What was wrong was the
 * conclusion drawn from it: the lifetimes are not unknown, only underivable. CNA documents every
 * array as borrowed for the call and the whole descriptor as deeply copied, so the graph is built
 * for the duration of one call and freed after it, with every count checked against every array
 * before anything is allocated.
 *
 * <p>The handle is owned; {@link #close()} releases it. An {@link CnaAnimationPlayer} created
 * from it retains it, so close the player first.
 */
public final class CnaSkinningData implements AutoCloseable {

    private final long handle;
    private boolean closed;

    private CnaSkinningData(long handle) {
        this.handle = handle;
    }

    /**
     * Creates skinning data from a skeleton and a set of named clips.
     *
     * <p>CNA copies everything, so both arguments stay the caller's values.
     *
     * @param skeleton the bone hierarchy and its bind pose
     * @param clips each clip's name and its poses
     * @return the skinning data, which the caller closes
     */
    public static CnaSkinningData of(CnaSkeleton skeleton, Map<String, CnbClip> clips) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(skeleton, "skeleton");
        Objects.requireNonNull(clips, "clips");
        int clipCount = clips.size();
        byte[][] names = new byte[clipCount][];
        double[] durations = new double[clipCount];
        int[] clipTrackCounts = new int[clipCount];
        List<CnbBoneTrack> tracks = new ArrayList<>();
        int index = 0;
        for (Map.Entry<String, CnbClip> entry : clips.entrySet()) {
            CnbClip clip = Objects.requireNonNull(entry.getValue(), "clip");
            names[index] = Objects.requireNonNull(entry.getKey(), "clip name")
                    .getBytes(StandardCharsets.UTF_8);
            durations[index] = clip.DurationSeconds();
            clipTrackCounts[index] = clip.Tracks().size();
            tracks.addAll(clip.Tracks());
            index++;
        }
        CnbClip.Flattened flat = new CnbClip(0d, tracks).flatten();
        long[] created = new long[1];
        CnbExtension.check("CnaSkinningData.of", NativeBindings.skinningDataCreate(
                skeleton.parents(), CnaSkeleton.matrices(skeleton.BindPoseLocal()),
                CnaSkeleton.matrices(skeleton.InverseBindPoseGlobal()),
                CnaSkeleton.matrices(skeleton.RootPrefix()), names, durations, clipTrackCounts,
                flat.boneIndices(), flat.keyframeCounts(), flat.times(), flat.values(), created));
        return new CnaSkinningData(created[0]);
    }

    /** Adopts a handle CNA created, which the caller then owns and closes. */
    static CnaSkinningData adopt(long handle) {
        return new CnaSkinningData(handle);
    }

    /**
     * Returns how many bones the skeleton has.
     *
     * @return the bone count
     */
    public int getBoneCount() {
        long[] count = new long[1];
        check("getBoneCount", NativeModelExtensionRoutes.skinningDataGetBoneCount(open(), count));
        return Math.toIntExact(count[0]);
    }

    /**
     * Returns how many clips the data holds.
     *
     * @return the clip count
     */
    public int getClipCount() {
        long[] count = new long[1];
        check("getClipCount", NativeModelExtensionRoutes.skinningDataGetClipCount(open(), count));
        return Math.toIntExact(count[0]);
    }

    /**
     * Returns one clip's name.
     *
     * @param index the zero-based clip index
     * @return the name
     */
    public String getClipName(int index) {
        long[] bytes = new long[1];
        check("getClipName", NativeModelExtensionRoutes
                .skinningDataGetClipNameByteCountAt(open(), index, bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        check("getClipName", NativeModelExtensionRoutes
                .skinningDataCopyClipNameAt(open(), index, destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /**
     * Returns every clip's name, in the order the data holds them.
     *
     * @return the names
     */
    public List<String> getClipNames() {
        int count = getClipCount();
        List<String> names = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            names.add(getClipName(index));
        }
        return Collections.unmodifiableList(names);
    }

    /**
     * Returns one clip's duration and track count, by name.
     *
     * @param name the clip's name
     * @return the clip, or {@code null} when the data holds no clip with that name
     */
    public CnbAnimation getClip(String name) {
        Objects.requireNonNull(name, "name");
        boolean[] found = new boolean[1];
        double[] duration = new double[1];
        long[] tracks = new long[1];
        check("getClip", NativeModelExtensionRoutes.skinningDataGetClipInfo(
                open(), name.getBytes(StandardCharsets.UTF_8), found, duration, tracks));
        if (!found[0]) {
            return null;
        }
        return new CnbAnimation(name, duration[0], Math.toIntExact(tracks[0]),
                getClipTargetSpace(getClipNames().indexOf(name)));
    }

    /**
     * Returns one clip's whole track, keyframe for keyframe.
     *
     * @param name the clip's name
     * @param trackIndex the zero-based track index
     * @return the track
     */
    public CnbBoneTrack getClipTrack(String name, int trackIndex) {
        Objects.requireNonNull(name, "name");
        CnbAnimation clip = getClip(name);
        if (clip == null) {
            throw new IllegalArgumentException("no clip named " + name);
        }
        byte[] utf8 = name.getBytes(StandardCharsets.UTF_8);
        int[] bone = new int[1];
        long[] written = new long[1];
        // Zero capacity first, so the read is sized once and done once.
        int probe = NativeModelExtensionRoutes.skinningDataCopyClipTrack(
                open(), utf8, trackIndex, bone, new float[0], new double[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check("getClipTrack", probe);
        }
        int count = Math.toIntExact(written[0]);
        float[] floating = new float[count * CnbKeyframes.FLOATS];
        double[] doubles = new double[count * CnbKeyframes.DOUBLES];
        check("getClipTrack", NativeModelExtensionRoutes.skinningDataCopyClipTrack(
                open(), utf8, trackIndex, bone, floating, doubles, written));
        List<CnbKeyframe> keyframes = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            keyframes.add(CnbKeyframes.read(floating, doubles, index));
        }
        return new CnbBoneTrack(bone[0], keyframes);
    }

    /**
     * Returns which index space one clip's bone indices live in.
     *
     * @param index the zero-based clip index
     * @return the target space
     */
    public CnbClipTargetSpace getClipTargetSpace(int index) {
        int[] space = new int[1];
        check("getClipTargetSpace", NativeModelExtensionRoutes
                .skinningDataGetClipTargetSpaceExt(open(), index, space));
        return CnbClipTargetSpace.fromValue(space[0]);
    }

    /**
     * States which index space one clip's bone indices live in.
     *
     * @param index the zero-based clip index
     * @param targetSpace the space
     */
    public void setClipTargetSpace(int index, CnbClipTargetSpace targetSpace) {
        Objects.requireNonNull(targetSpace, "targetSpace");
        check("setClipTargetSpace", NativeModelExtensionRoutes
                .skinningDataSetClipTargetSpaceExt(open(), index, targetSpace.ordinal()));
    }

    /**
     * Returns the skeleton the data was built from.
     *
     * @return the skeleton, read back out of CNA's own copy
     */
    public CnaSkeleton getSkeleton() {
        int bones = getBoneCount();
        int[] parents = new int[bones];
        long[] written = new long[1];
        check("getSkeleton", NativeModelExtensionRoutes
                .skinningDataCopySkeletonHierarchy(open(), parents, written));
        List<Integer> parentList = new ArrayList<>(bones);
        for (int parent : parents) {
            parentList.add(parent);
        }
        return new CnaSkeleton(parentList,
                readMatrices("getSkeleton", bones, NativeModelExtensionRoutes
                        ::skinningDataCopyBindPose),
                readMatrices("getSkeleton", bones, NativeModelExtensionRoutes
                        ::skinningDataCopyInverseBindPose),
                readMatrices("getSkeleton", bones, NativeModelExtensionRoutes
                        ::skinningDataCopySkeletonRootPrefix));
    }

    /**
     * Returns the name of the node the skeleton is rooted at, empty when it has none.
     *
     * @return the root name
     */
    public String getSkeletonRootName() {
        long[] bytes = new long[1];
        check("getSkeletonRootName", NativeModelExtensionRoutes
                .skinningDataGetSkeletonRootNameByteCountExt(open(), bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        check("getSkeletonRootName", NativeModelExtensionRoutes
                .skinningDataCopySkeletonRootNameExt(open(), destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /**
     * Records the name of the node the skeleton is rooted at.
     *
     * @param name the name; empty clears it
     */
    public void setSkeletonRootName(String name) {
        Objects.requireNonNull(name, "name");
        check("setSkeletonRootName", NativeModelExtensionRoutes
                .skinningDataSetSkeletonRootNameExt(open(),
                        name.getBytes(StandardCharsets.UTF_8)));
    }

    /**
     * Returns the scene-node index the skeleton is rooted at, or -1 when it has none.
     *
     * @return the node index
     */
    public int getSkeletonRootNodeIndex() {
        int[] value = new int[1];
        check("getSkeletonRootNodeIndex", NativeModelExtensionRoutes
                .skinningDataGetSkeletonRootNodeIndexExt(open(), value));
        return value[0];
    }

    /**
     * Records the scene-node index the skeleton is rooted at.
     *
     * @param index the node index, or -1 for none
     */
    public void setSkeletonRootNodeIndex(int index) {
        check("setSkeletonRootNodeIndex", NativeModelExtensionRoutes
                .skinningDataSetSkeletonRootNodeIndexExt(open(), index));
    }

    /** Releases the skinning data. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        check("close", NativeModelExtensionRoutes.skinningDataDestroy(handle));
    }

    /** The native handle, for the player that retains it. */
    long handle() {
        return open();
    }

    private interface MatrixReader {
        int read(long data, float[] destination, long[] outCount);
    }

    private List<Matrix> readMatrices(String operation, int bones, MatrixReader reader) {
        // Ask with no buffer: CNA reports the count it has and refuses the write, which is the
        // format's own two-call protocol rather than a failure -- so BUFFER_TOO_SMALL is the
        // expected answer here and anything else is not.
        long[] written = new long[1];
        int probe = reader.read(open(), new float[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check(operation, probe);
        }
        int count = Math.toIntExact(written[0]);
        if (count == 0) {
            return List.of();
        }
        float[] leaves = new float[count * CnaSkeleton.MATRIX_FLOATS];
        check(operation, reader.read(open(), leaves, written));
        return CnaSkeleton.matricesOf(leaves, count);
    }

    private static void check(String operation, int result) {
        CnbExtension.check("CnaSkinningData." + operation, result);
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this CnaSkinningData is closed");
        }
        return handle;
    }
}
