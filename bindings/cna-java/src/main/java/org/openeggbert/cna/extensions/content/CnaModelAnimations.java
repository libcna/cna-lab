package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeModelExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * A named set of animation clips a model can be posed from.
 *
 * <p>A CNA extension: XNA 4.0 ships skinning only as a sample-level content processor, so it has
 * no animation type at all -- a game that wanted one wrote the runtime itself, from the
 * {@code SkinnedModel} sample.
 *
 * <p><strong>This is what turns a {@link CnbClip} into something that poses a model.</strong>
 * {@link CnaModel#applyClipToBones} evaluates one clip at a time and writes the result straight
 * into the model's bone transforms, so a game gets skeletal animation over an ordinary XNA
 * {@code Model} without a skinning runtime of its own.
 *
 * <p><strong>The two target spaces must never be interchanged, and CNA refuses rather than
 * guessing.</strong> A {@linkplain CnbClipTargetSpace#SceneNode scene-node} clip's bone indices
 * select {@code Model.Bones} entries directly, which is what posing a model needs; a
 * {@linkplain CnbClipTargetSpace#JointPalette joint-palette} clip's select a skinning skeleton's
 * joints. Applying one as the other would pose the wrong bones without failing, so
 * {@code applyClipToBones} refuses a joint-palette clip.
 *
 * <p>The handle is owned; {@link #close()} releases it. CNA copies every clip, so the values
 * handed in stay the caller's.
 */
public final class CnaModelAnimations implements AutoCloseable {

    private final long handle;
    private boolean closed;

    private CnaModelAnimations(long handle) {
        this.handle = handle;
    }

    /**
     * Creates a set from named clips, in the order given.
     *
     * <p>CNA takes this as a pointer graph -- named clips to clips to tracks to keyframes -- which
     * the JNI generator refuses. The lifetimes are stated rather than unknown, though: every array
     * is borrowed for the call and CNA copies what it keeps, so the graph is flattened here,
     * checked in the adapter and rebuilt for the duration of one call.
     *
     * <p><strong>The set orders its clips by name, not by the order given here.</strong> That is
     * measured rather than assumed -- CNA's own container is an ordered map keyed by the clip
     * name -- and it is why {@link #indexOf(String)} exists: a caller that remembered the
     * position it passed a clip in would read the wrong one back.
     *
     * @param clips each clip's name and its poses
     * @return the set, which the caller closes
     */
    public static CnaModelAnimations of(Map<String, CnbClip> clips) {
        CnbExtension.requireAvailable();
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
        CnbExtension.check("CnaModelAnimations.of", NativeBindings.modelAnimationsCreate(
                names, durations, clipTrackCounts, flat.boneIndices(), flat.keyframeCounts(),
                flat.times(), flat.values(), created));
        return new CnaModelAnimations(created[0]);
    }

    /**
     * Returns how many clips the set holds.
     *
     * @return the clip count
     */
    public int size() {
        long[] count = new long[1];
        check("size", NativeModelExtensionRoutes.modelAnimationsGetClipCount(open(), count));
        return Math.toIntExact(count[0]);
    }

    /**
     * Returns the index of the clip with this name.
     *
     * <p>The way to reach a clip, because the set orders by name rather than by the order the
     * clips were given in.
     *
     * @param name the clip's name
     * @return its zero-based index
     * @throws IllegalArgumentException when the set holds no clip with that name
     */
    public int indexOf(String name) {
        Objects.requireNonNull(name, "name");
        int count = size();
        for (int index = 0; index < count; index++) {
            if (name.equals(getClipName(index))) {
                return index;
            }
        }
        throw new IllegalArgumentException("no animation clip named " + name);
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
                .modelAnimationsGetClipNameByteCountAt(open(), index, bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        check("getClipName", NativeModelExtensionRoutes
                .modelAnimationsCopyClipNameAt(open(), index, destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /**
     * Returns one clip's duration, track count and target space.
     *
     * @param index the zero-based clip index
     * @return the clip's description
     */
    public CnbAnimation getClip(int index) {
        double[] duration = new double[1];
        long[] tracks = new long[1];
        int[] space = new int[1];
        check("getClip", NativeModelExtensionRoutes
                .modelAnimationsGetClipInfoAt(open(), index, duration, tracks, space));
        return new CnbAnimation(getClipName(index), duration[0], Math.toIntExact(tracks[0]),
                CnbClipTargetSpace.fromValue(space[0]));
    }

    /**
     * States which index space one clip's bone indices are in.
     *
     * <p>Separate from creation because a clip does not carry it: a clip is a list of bone
     * indices and poses, and whether those indices name model bones or skeleton joints is a fact
     * about how it was authored. Saying it wrongly poses the wrong bones, so CNA makes it an
     * explicit statement rather than a default.
     *
     * @param index the zero-based clip index
     * @param targetSpace the space its bone indices live in
     */
    public void setClipTargetSpace(int index, CnbClipTargetSpace targetSpace) {
        Objects.requireNonNull(targetSpace, "targetSpace");
        check("setClipTargetSpace", NativeModelExtensionRoutes
                .modelAnimationsSetClipTargetSpaceAt(open(), index, targetSpace.ordinal()));
    }

    /**
     * Returns CNA's own name for this type.
     *
     * @return the type name
     */
    public String getTypeName() {
        long[] bytes = new long[1];
        check("getTypeName", NativeModelExtensionRoutes
                .modelAnimationsGetTypeNameByteCount(open(), bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        check("getTypeName", NativeModelExtensionRoutes
                .modelAnimationsCopyTypeName(open(), destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /** Releases the set. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        check("close", NativeModelExtensionRoutes.modelAnimationsDestroy(handle));
    }

    /** The native handle, for the model that poses itself from this set. */
    long handle() {
        return open();
    }

    private static void check(String operation, int result) {
        CnbExtension.check("CnaModelAnimations." + operation, result);
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this CnaModelAnimations is closed");
        }
        return handle;
    }
}
