package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Graphics.IndexBuffer;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.VertexBuffer;
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
 * A skinned character: a skeleton, its clips, and the parts drawn with them.
 *
 * <p>A CNA extension. XNA 4.0's {@code Model} has no skeleton and no clips -- skinning was a
 * sample-level content processor plus a runtime the game copied -- so a skinned model has no XNA
 * type to project onto and lives here instead.
 *
 * <p><strong>Built rather than described.</strong> CNA also offers a one-call constructor over a
 * whole descriptor; this builds an empty model and then states its skeleton, its clips and its
 * parts. That is the same state and it is the only form that can be changed afterwards, which is
 * what a game adding a part at load time needs.
 *
 * <p>The model <strong>retains what it is given rather than taking it</strong>: adding a part
 * retains the vertex buffer, the index buffer, the mesh part and the texture, and removing the
 * part or closing the model releases the model's hold on them. It does not destroy them. A mesh
 * part outlives the model that drew it and still holds its own buffers, so the close order is the
 * model, then the parts, then the buffers -- CNA refuses to destroy a buffer a part holds, and
 * names the part when it does.
 *
 * <p>{@link #computeBoneTransforms} is the shortest path from a clip to a draw: it evaluates a
 * named clip at a position and hands back the skinning palette, without a separate player. Use
 * {@link CnaAnimationPlayer} when the position advances frame by frame and the intermediate
 * palettes matter.
 */
public final class CnaSkinnedModel implements AutoCloseable {

    private final long handle;
    private boolean closed;

    private CnaSkinnedModel(long handle) {
        this.handle = handle;
    }

    /**
     * Creates an empty skinned model.
     *
     * @return the model, which the caller closes
     */
    public static CnaSkinnedModel create() {
        CnbExtension.requireAvailable();
        long[] created = new long[1];
        check("create", NativeModelExtensionRoutes.skinnedModelCreateDefault(created));
        return new CnaSkinnedModel(created[0]);
    }

    /**
     * Creates a model with a skeleton and a set of named clips already in it.
     *
     * @param skeleton the bone hierarchy and its bind pose
     * @param clips each clip's name and its poses
     * @return the model, which the caller closes
     */
    public static CnaSkinnedModel of(CnaSkeleton skeleton, Map<String, CnbClip> clips) {
        Objects.requireNonNull(skeleton, "skeleton");
        Objects.requireNonNull(clips, "clips");
        CnaSkinnedModel model = create();
        try {
            model.setSkeleton(skeleton);
            for (Map.Entry<String, CnbClip> entry : clips.entrySet()) {
                model.setClip(Objects.requireNonNull(entry.getKey(), "clip name"),
                        Objects.requireNonNull(entry.getValue(), "clip"));
            }
        } catch (RuntimeException failure) {
            model.close();
            throw failure;
        }
        return model;
    }

    /**
     * States the model's skeleton.
     *
     * @param skeleton the bone hierarchy and its bind pose; a root prefix is not part of a
     *        skinned model's skeleton and is ignored
     */
    public void setSkeleton(CnaSkeleton skeleton) {
        Objects.requireNonNull(skeleton, "skeleton");
        check("setSkeleton", NativeBindings.skinnedModelSetSkeleton(open(), skeleton.parents(),
                CnaSkeleton.matrices(skeleton.BindPoseLocal()),
                CnaSkeleton.matrices(skeleton.InverseBindPoseGlobal())));
    }

    /**
     * Adds or replaces one named clip.
     *
     * @param name the clip's name
     * @param clip the clip's poses; CNA copies it
     */
    public void setClip(String name, CnbClip clip) {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(clip, "clip");
        CnbClip.Flattened flat = clip.flatten();
        check("setClip", NativeBindings.skinnedModelSetClip(open(),
                name.getBytes(StandardCharsets.UTF_8), clip.DurationSeconds(),
                flat.boneIndices(), flat.keyframeCounts(), flat.times(), flat.values()));
    }

    /**
     * Removes one named clip.
     *
     * @param name the clip's name
     */
    public void removeClip(String name) {
        Objects.requireNonNull(name, "name");
        check("removeClip", NativeModelExtensionRoutes.skinnedModelRemoveClip(
                open(), name.getBytes(StandardCharsets.UTF_8)));
    }

    /**
     * Adds one renderable part, which the model then retains.
     *
     * <p><strong>Retained, not taken.</strong> The part stays the caller's: closing the model
     * releases the model's hold and leaves the part alive, still retaining its own buffers. The
     * close order is the model, then the part, then the buffers -- measured, because CNA refuses
     * to destroy a buffer a part still holds and names the part when it does.
     *
     * @param name the part's name, which removing it takes
     * @param vertexBuffer the part's vertices, on the same device
     * @param indexBuffer the part's indices, on the same device
     * @param part the mesh part describing the draw; it stays the caller's
     * @param texture the part's texture, or {@code null} for none
     */
    public void addPart(String name, VertexBuffer vertexBuffer, IndexBuffer indexBuffer,
            CnaModelMeshPartHandle part, Texture2D texture) {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(vertexBuffer, "vertexBuffer");
        Objects.requireNonNull(indexBuffer, "indexBuffer");
        Objects.requireNonNull(part, "part");
        check("addPart", NativeModelExtensionRoutes.skinnedModelAddPart(open(),
                name.getBytes(StandardCharsets.UTF_8),
                NativeBindings.nativeResourceHandle(vertexBuffer),
                NativeBindings.nativeResourceHandle(indexBuffer),
                part.value(),
                texture == null ? 0L : NativeBindings.nativeResourceHandle(texture)));
    }

    /**
     * Removes one named part and releases what it retained.
     *
     * @param name the part's name
     */
    public void removePart(String name) {
        Objects.requireNonNull(name, "name");
        check("removePart", NativeModelExtensionRoutes.skinnedModelRemovePart(
                open(), name.getBytes(StandardCharsets.UTF_8)));
    }

    /**
     * Moves every part out of another model of the same skeleton, replacing by name.
     *
     * <p>The other model is left <strong>valid and empty</strong>, which is CNA's own move
     * semantics rather than a copy: a caller that wanted both would have to build both.
     *
     * @param other the model whose parts to take
     */
    public void attachPartsFrom(CnaSkinnedModel other) {
        Objects.requireNonNull(other, "other");
        check("attachPartsFrom", NativeModelExtensionRoutes.skinnedModelAttachParts(
                open(), other.open()));
    }

    /**
     * Returns how many bones the skeleton has.
     *
     * @return the bone count
     */
    public int getBoneCount() {
        long[] count = new long[1];
        check("getBoneCount", NativeModelExtensionRoutes.skinnedModelGetBoneCount(open(), count));
        return Math.toIntExact(count[0]);
    }

    /**
     * Returns how many clips the model holds.
     *
     * @return the clip count
     */
    public int getClipCount() {
        long[] count = new long[1];
        check("getClipCount", NativeModelExtensionRoutes.skinnedModelGetClipCount(open(), count));
        return Math.toIntExact(count[0]);
    }

    /**
     * Returns every clip's name, in the order the model holds them.
     *
     * @return the names
     */
    public List<String> getClipNames() {
        int count = getClipCount();
        List<String> names = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            names.add(text("getClipNames", index,
                    NativeModelExtensionRoutes::skinnedModelGetClipNameByteCountAt,
                    NativeModelExtensionRoutes::skinnedModelCopyClipNameAt));
        }
        return Collections.unmodifiableList(names);
    }

    /**
     * Returns one clip's duration and track count, by name.
     *
     * @param name the clip's name
     * @return the clip, or {@code null} when the model holds no clip with that name
     */
    public CnbAnimation getClip(String name) {
        Objects.requireNonNull(name, "name");
        boolean[] found = new boolean[1];
        double[] duration = new double[1];
        long[] tracks = new long[1];
        check("getClip", NativeModelExtensionRoutes.skinnedModelGetClipInfo(
                open(), name.getBytes(StandardCharsets.UTF_8), found, duration, tracks));
        return found[0] ? new CnbAnimation(name, duration[0], Math.toIntExact(tracks[0]),
                CnbClipTargetSpace.JointPalette) : null;
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
        byte[] utf8 = name.getBytes(StandardCharsets.UTF_8);
        int[] bone = new int[1];
        long[] written = new long[1];
        int probe = NativeModelExtensionRoutes.skinnedModelCopyClipTrack(
                open(), utf8, trackIndex, bone, new float[0], new double[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check("getClipTrack", probe);
        }
        int count = Math.toIntExact(written[0]);
        float[] floating = new float[count * CnbKeyframes.FLOATS];
        double[] doubles = new double[count * CnbKeyframes.DOUBLES];
        check("getClipTrack", NativeModelExtensionRoutes.skinnedModelCopyClipTrack(
                open(), utf8, trackIndex, bone, floating, doubles, written));
        List<CnbKeyframe> keyframes = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            keyframes.add(CnbKeyframes.read(floating, doubles, index));
        }
        return new CnbBoneTrack(bone[0], keyframes);
    }

    /**
     * Returns the skeleton the model was given.
     *
     * @return the skeleton, read back out of CNA's own copy; its root prefix is always empty
     */
    public CnaSkeleton getSkeleton() {
        int bones = getBoneCount();
        int[] parents = new int[bones];
        long[] written = new long[1];
        int probe = NativeModelExtensionRoutes.skinnedModelCopyParentBoneIndices(
                open(), bones == 0 ? new int[0] : parents, written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check("getSkeleton", probe);
        }
        List<Integer> parentList = new ArrayList<>(bones);
        for (int parent : parents) {
            parentList.add(parent);
        }
        return new CnaSkeleton(parentList,
                matrices("getSkeleton", NativeModelExtensionRoutes::skinnedModelCopyBindPoseLocal),
                matrices("getSkeleton",
                        NativeModelExtensionRoutes::skinnedModelCopyInverseBindPoseGlobal),
                List.of());
    }

    /**
     * Returns how many parts the model holds.
     *
     * @return the part count
     */
    public int getPartCount() {
        long[] count = new long[1];
        check("getPartCount", NativeModelExtensionRoutes.skinnedModelGetPartCount(open(), count));
        return Math.toIntExact(count[0]);
    }

    /**
     * Returns every part's name, in the order the model holds them.
     *
     * @return the names
     */
    public List<String> getPartNames() {
        int count = getPartCount();
        List<String> names = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            names.add(text("getPartNames", index,
                    NativeModelExtensionRoutes::skinnedModelGetPartNameByteCountAt,
                    NativeModelExtensionRoutes::skinnedModelCopyPartNameAt));
        }
        return Collections.unmodifiableList(names);
    }

    /**
     * Returns one part's mesh-part alias and whether it carries a texture.
     *
     * <p><strong>The two handles it answers with are not the same kind, and both readings had to
     * be measured.</strong> CNA calls the part "an <em>owned</em> part alias" and the texture
     * "<em>retained</em>", which reads like two handles to give back. Only one is. The part alias
     * really is owned -- dropping it leaves the part alive holding its own buffers, and the first
     * sign of that is CNA refusing to destroy a vertex buffer much later, naming a part rather
     * than the call that leaked it. The texture handle is the model's own: releasing it is
     * refused with "the Texture2D is retained by an active ... model", because it is not a fresh
     * name but the very handle the model holds.
     *
     * <p>So the part comes back as an {@link AutoCloseable} the caller closes, and the texture is
     * reported as a yes or no rather than handed over.
     *
     * @param index the zero-based part index
     * @return the part, whose mesh-part handle the caller closes
     */
    public Part getPart(int index) {
        long[] part = new long[1];
        boolean[] hasTexture = new boolean[1];
        long[] texture = new long[1];
        check("getPart", NativeModelExtensionRoutes.skinnedModelGetPartAt(
                open(), index, part, hasTexture, texture));
        return new Part(new CnaModelMeshPartHandle(part[0], true), hasTexture[0]);
    }

    /**
     * Returns how many of each resource the model owns.
     *
     * @return the counts, in CNA's own order: vertex buffers, index buffers, parts, textures
     */
    public int[] getOwnedResourceCounts() {
        long[] vertexBuffers = new long[1];
        long[] indexBuffers = new long[1];
        long[] parts = new long[1];
        long[] textures = new long[1];
        check("getOwnedResourceCounts",
                NativeModelExtensionRoutes.skinnedModelGetOwnedResourceCounts(
                        open(), vertexBuffers, indexBuffers, parts, textures));
        return new int[] {Math.toIntExact(vertexBuffers[0]), Math.toIntExact(indexBuffers[0]),
                Math.toIntExact(parts[0]), Math.toIntExact(textures[0])};
    }

    /**
     * Evaluates a named clip at a position and returns the skinning palette.
     *
     * <p>The shortest path from a clip to a draw. {@link CnaAnimationPlayer} is the other one, and
     * the difference is what a game needs: this is one evaluation, and a player carries a position
     * forward and keeps the intermediate palettes.
     *
     * @param clipName the clip to evaluate
     * @param positionSeconds where in it to evaluate
     * @param loop whether to wrap around rather than clamp at the end
     * @return one matrix per bone
     */
    public List<Matrix> computeBoneTransforms(String clipName, double positionSeconds,
            boolean loop) {
        Objects.requireNonNull(clipName, "clipName");
        byte[] utf8 = clipName.getBytes(StandardCharsets.UTF_8);
        long[] written = new long[1];
        int probe = NativeModelExtensionRoutes.skinnedModelComputeBoneTransforms(
                open(), utf8, positionSeconds, loop, new float[0], written);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            check("computeBoneTransforms", probe);
        }
        int count = Math.toIntExact(written[0]);
        if (count == 0) {
            return List.of();
        }
        float[] leaves = new float[count * CnaSkeleton.MATRIX_FLOATS];
        check("computeBoneTransforms", NativeModelExtensionRoutes
                .skinnedModelComputeBoneTransforms(open(), utf8, positionSeconds, loop, leaves,
                        written));
        return CnaSkeleton.matricesOf(leaves, count);
    }

    /** Releases the model and everything it still retains. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        check("close", NativeModelExtensionRoutes.skinnedModelDestroy(handle));
    }

    /** The native handle, for the avatar renderer that draws it. */
    public long handle() {
        return open();
    }

    private interface Sizer {
        int size(long model, long index, long[] outByteCount);
    }

    private interface Copier {
        int copy(long model, long index, byte[] destination, long[] outByteCount);
    }

    private interface MatrixReader {
        int read(long model, float[] destination, long[] outCount);
    }

    private String text(String operation, int index, Sizer sizer, Copier copier) {
        long[] bytes = new long[1];
        check(operation, sizer.size(open(), index, bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        check(operation, copier.copy(open(), index, destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    private List<Matrix> matrices(String operation, MatrixReader reader) {
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
        CnbExtension.check("CnaSkinnedModel." + operation, result);
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this CnaSkinnedModel is closed");
        }
        return handle;
    }

    /**
     * One of a skinned model's renderable parts, as the model lends it back.
     *
     * <p>The mesh part is <strong>the caller's to close</strong>, which is why this is an
     * {@link AutoCloseable} rather than a plain value: CNA hands back an owned alias, and a
     * caller that dropped it would leave the part holding its buffers with nothing left to
     * release it.
     *
     * @param MeshPart the mesh part describing the draw; closing this closes it
     * @param HasTexture whether the part carries a texture
     */
    public record Part(CnaModelMeshPartHandle MeshPart, boolean HasTexture)
            implements AutoCloseable {

        /** Releases the alias CNA lent back. */
        @Override
        public void close() {
            MeshPart.close();
        }
    }
}
