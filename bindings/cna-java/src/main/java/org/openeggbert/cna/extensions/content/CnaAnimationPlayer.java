package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;
import org.openeggbert.cna.internal.generated.NativeModelExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Objects;

/**
 * Plays one animation clip and reports the three matrix palettes a skinned draw needs.
 *
 * <p>A CNA extension, and the other half of what XNA left to its {@code SkinnedModel} sample. The
 * sample's {@code AnimationPlayer} kept three arrays -- bone, world and skin transforms -- and
 * advanced them by walking the clip's keyframes every frame; a game that wanted a skinned
 * character copied it. This is CNA's, reachable from Java.
 *
 * <p>The three palettes are different things and a draw needs the right one:
 *
 * <ul>
 *   <li><strong>bone</strong> transforms are each bone's local pose, straight from the clip;</li>
 *   <li><strong>world</strong> transforms are those composed down the hierarchy;</li>
 *   <li><strong>skin</strong> transforms are the world ones with the inverse bind pose applied,
 *       which is what a vertex shader multiplies by.</li>
 * </ul>
 *
 * <p>The player <strong>retains its skinning data</strong>: close the player before the data it
 * was made from.
 */
public final class CnaAnimationPlayer implements AutoCloseable {

    private final long handle;
    private boolean closed;

    private CnaAnimationPlayer(long handle) {
        this.handle = handle;
    }

    /**
     * Creates a player over skinning data.
     *
     * @param data the skeleton and clips to play; the player retains it
     * @return the player, which the caller closes before the data
     */
    public static CnaAnimationPlayer of(CnaSkinningData data) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(data, "data");
        long[] created = new long[1];
        check("of", NativeModelExtensionRoutes.animationPlayerCreate(data.handle(), created));
        return new CnaAnimationPlayer(created[0]);
    }

    /**
     * Starts a clip by name, from its beginning.
     *
     * @param clipName the clip to play
     */
    public void startClip(String clipName) {
        Objects.requireNonNull(clipName, "clipName");
        check("startClip", NativeModelExtensionRoutes.animationPlayerStartClip(
                open(), clipName.getBytes(StandardCharsets.UTF_8)));
    }

    /**
     * Advances the clip and recomputes the three palettes.
     *
     * @param timeSeconds how far to move, or where to move to
     * @param relativeToCurrentTime whether {@code timeSeconds} is an amount to advance by rather
     *        than an absolute position in the clip
     * @param loop whether to wrap around at the end rather than stopping there
     */
    public void update(double timeSeconds, boolean relativeToCurrentTime, boolean loop) {
        check("update", NativeModelExtensionRoutes.animationPlayerUpdate(
                open(), timeSeconds, relativeToCurrentTime, loop));
    }

    /**
     * Returns where in the clip the player currently is.
     *
     * @return the position in seconds
     */
    public double getCurrentPosition() {
        double[] position = new double[1];
        check("getCurrentPosition",
                NativeModelExtensionRoutes.animationPlayerGetCurrentPosition(open(), position));
        return position[0];
    }

    /**
     * Returns the clip the player is on.
     *
     * @return the clip, or {@code null} when none has been started
     */
    public CnbAnimation getCurrentClip() {
        boolean[] present = new boolean[1];
        double[] duration = new double[1];
        long[] tracks = new long[1];
        check("getCurrentClip", NativeModelExtensionRoutes
                .animationPlayerGetCurrentClipInfo(open(), present, duration, tracks));
        if (!present[0]) {
            return null;
        }
        long[] bytes = new long[1];
        check("getCurrentClip", NativeModelExtensionRoutes
                .animationPlayerGetCurrentClipNameByteCount(open(), bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        check("getCurrentClip", NativeModelExtensionRoutes
                .animationPlayerCopyCurrentClipName(open(), destination, bytes));
        return new CnbAnimation(
                new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8),
                duration[0], Math.toIntExact(tracks[0]), CnbClipTargetSpace.JointPalette);
    }

    /**
     * Returns each bone's local pose, as the clip states it.
     *
     * @return one matrix per bone
     */
    public List<Matrix> getBoneTransforms() {
        return palette("getBoneTransforms",
                NativeModelExtensionRoutes::animationPlayerCopyBoneTransforms);
    }

    /**
     * Returns each bone's pose composed down the hierarchy.
     *
     * @return one matrix per bone
     */
    public List<Matrix> getWorldTransforms() {
        return palette("getWorldTransforms",
                NativeModelExtensionRoutes::animationPlayerCopyWorldTransforms);
    }

    /**
     * Returns the palette a skinned vertex shader multiplies by.
     *
     * <p>The world transforms with the inverse bind pose applied, which is what makes a vertex
     * weighted to a bone follow it.
     *
     * @return one matrix per bone
     */
    public List<Matrix> getSkinTransforms() {
        return palette("getSkinTransforms",
                NativeModelExtensionRoutes::animationPlayerCopySkinTransforms);
    }

    /** Releases the player. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        check("close", NativeModelExtensionRoutes.animationPlayerDestroy(handle));
    }

    private interface PaletteReader {
        int read(long player, float[] destination, long[] outCount);
    }

    private List<Matrix> palette(String operation, PaletteReader reader) {
        long[] written = new long[1];
        // Zero capacity first: CNA reports how many matrices it has -- the bone count, which is
        // not something this side should assume -- and refuses the write, which is the two-call
        // protocol rather than a failure.
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
        CnbExtension.check("CnaAnimationPlayer." + operation, result);
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this CnaAnimationPlayer is closed");
        }
        return handle;
    }
}
