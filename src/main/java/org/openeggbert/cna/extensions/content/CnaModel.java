package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Content.ContentManager;
import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.Model;
import Microsoft.Xna.Framework.Graphics.ModelBone;
import Microsoft.Xna.Framework.Graphics.ModelMesh;
import Microsoft.Xna.Framework.Graphics.ModelMeshPart;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeModelExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * One model as CNA's own runtime sees it.
 *
 * <p>A CNA extension. It is built {@linkplain #From(Model) from} an XNA {@link Model} the content
 * pipeline already produced, so the strict load keeps every guarantee it has: XNB records the
 * effect's type and the managed reader turns it into the {@code BasicEffect} a game casts to.
 * What this adds is what CNA can do with the same graph -- draw it in one call, compute its
 * transforms, and answer the questions XNA's {@code Model} has nowhere to put.
 *
 * <p><strong>Ownership.</strong> CNA <em>retains</em> the buffers and effects the XNA model
 * already owns; it does not take them. This object owns the bone, part, mesh and model handles it
 * created and releases exactly those, so the Java resources stay Java's and closing this leaves
 * the XNA model untouched. Nothing here hands a native handle to a caller, so there is no second
 * owner to get wrong.
 *
 * <p><strong>Or loaded by CNA itself.</strong> {@link #Load} opens a {@code .xnb}, {@code .cnb}
 * or {@code .gltf}/{@code .glb} model through CNA's own content manager, and {@link #getContentTag}
 * reads back the {@code Dictionary<string, object>} a content processor wrote as its
 * {@code Tag}. That route once produced a model that segfaulted during teardown
 * ({@code JAVA-UPSTREAM-004}); CNA fixed it, and
 * {@code tools/native-abi/probes/content_manager_model_teardown.c} measures the teardown.
 *
 * <p>A model belongs to the game that owns its graphics device, and is built, read, drawn and
 * closed on that game's thread.
 */
public final class CnaModel implements AutoCloseable {

    private static final int RESULT_SUCCESS = 0;
    private static final int RESULT_INVALID_STATE = 3;
    private static final int RESULT_NOT_SUPPORTED = 6;

    /** CNA reports "no bone" and "no material variant" as an invalid index of -1. */
    private static final int NONE = -1;

    private final long handle;
    private final List<Long> owned;
    private boolean closed;

    private CnaModel(long handle, List<Long> owned) {
        this.handle = handle;
        this.owned = List.copyOf(owned);
    }

    /**
     * Loads a model through CNA's own content manager.
     *
     * <p>The model owns everything the load published -- its bones, meshes, parts, effects and
     * buffers -- and the asset stays cached by name in the content manager, so a second load
     * republishes the same model rather than reading the file again.
     *
     * @param contentManager the manager whose root directory and cache to use
     * @param assetName the asset name, without extension
     * @return the model, which the caller closes
     */
    public static CnaModel Load(ContentManager contentManager, String assetName) {
        Objects.requireNonNull(contentManager, "contentManager");
        Objects.requireNonNull(assetName, "assetName");
        long[] loaded = new long[1];
        check("Load", NativeModelExtensionRoutes.contentManagerLoadModel(
                NativeBindings.nativeContentManagerHandle(contentManager), utf8(assetName),
                loaded));
        return new CnaModel(loaded[0], List.of());
    }

    /**
     * Returns the {@code Dictionary<string, object>} a content processor wrote as this model's
     * {@code Tag}, or {@code null} when it has none or a tag of another shape.
     *
     * <p>Only a model {@linkplain #Load loaded} by CNA carries one; a model built {@link #From} an
     * XNA one has none. The dictionary is owned by the caller and outlives this model.
     *
     * @return the tag, which the caller closes, or {@code null}
     */
    public CnaObjectDictionary getContentTag() {
        boolean[] hasTag = new boolean[1];
        long[] dictionary = new long[1];
        check("getContentTag", NativeModelExtensionRoutes.modelGetContentTagDictionaryExt(
                open(), hasTag, dictionary));
        return hasTag[0] ? CnaObjectDictionary.adopt(dictionary[0]) : null;
    }

    /**
     * Builds CNA's own model over the resources an XNA model already owns.
     *
     * <p>Every bone, mesh and part is recreated as a CNA object; the vertex buffers, index
     * buffers and effects are <em>retained</em>, not copied, so nothing is uploaded twice and the
     * XNA model keeps owning them.
     *
     * @param model the loaded XNA model to mirror
     * @return the CNA model, which the caller closes
     */
    public static CnaModel From(Model model) {
        Objects.requireNonNull(model, "model");
        if (model.getMeshes().isEmpty() && model.getBones().isEmpty()) {
            throw new IllegalArgumentException("model has no bones and no meshes");
        }
        GraphicsDevice graphicsDevice = deviceOf(model);
        long device = NativeBindings.nativeGraphicsDeviceValue(graphicsDevice);
        List<Long> owned = new ArrayList<>();
        try {
            long[] bones = new long[model.getBones().size()];
            for (int index = 0; index < bones.length; index++) {
                ModelBone bone = model.getBones().get(index);
                long[] created = new long[1];
                check("boneCreate", NativeModelExtensionRoutes.modelBoneCreate(
                        bone.getIndex(), utf8(bone.getName()), created));
                bones[index] = created[0];
                owned.add(created[0]);
                check("boneTransform", NativeModelExtensionRoutes
                        .modelBoneSetTransform(created[0], floats(bone.getTransform(), "bone")));
            }
            for (int index = 0; index < bones.length; index++) {
                for (ModelBone child : model.getBones().get(index).getChildren()) {
                    check("boneAddChild", NativeModelExtensionRoutes
                            .modelBoneAddChild(bones[index], bones[child.getIndex()]));
                }
            }

            long[] meshes = new long[model.getMeshes().size()];
            long[] meshParents = new long[meshes.length];
            for (int index = 0; index < meshes.length; index++) {
                ModelMesh mesh = model.getMeshes().get(index);
                long[] parts = new long[mesh.getMeshParts().size()];
                for (int part = 0; part < parts.length; part++) {
                    ModelMeshPart source = mesh.getMeshParts().get(part);
                    long[] created = new long[1];
                    check("partCreate", NativeModelExtensionRoutes.modelMeshPartCreate(
                            handleOf(source.getVertexBuffer()), handleOf(source.getIndexBuffer()),
                            source.getNumVertices(), source.getPrimitiveCount(),
                            source.getStartIndex(), source.getVertexOffset(), created));
                    parts[part] = created[0];
                    owned.add(created[0]);
                    Effect effect = source.getEffect();
                    if (effect != null) {
                        check("partSetEffect", NativeModelExtensionRoutes.modelMeshPartSetEffect(
                                created[0], NativeBindings.nativeResourceHandle(effect)));
                    }
                }
                long[] created = new long[1];
                check("meshCreate", NativeModelExtensionRoutes.modelMeshCreateNamed(
                        device, utf8(mesh.getName()), parts, created));
                meshes[index] = created[0];
                owned.add(created[0]);
                check("meshBounds", NativeModelExtensionRoutes.modelMeshSetBoundingSphere(
                        created[0], sphereFloats(mesh.getBoundingSphere())));
                ModelBone parent = mesh.getParentBone();
                meshParents[index] = parent == null ? 0L : bones[parent.getIndex()];
                if (parent != null) {
                    check("meshParent", NativeModelExtensionRoutes
                            .modelMeshSetParentBone(created[0], bones[parent.getIndex()]));
                }
            }

            long root = model.getRoot() == null ? 0L : model.getRoot().getIndex();
            long[] created = new long[1];
            check("modelCreate", NativeModelExtensionRoutes.modelCreateWithParents(
                    device, bones, meshes, meshParents, root, created));
            return new CnaModel(created[0], owned);
        } catch (RuntimeException failure) {
            releaseAll(owned, failure);
            throw failure;
        }
    }

    /**
     * Reads the whole structure into one immutable snapshot.
     *
     * <p>Every bone, mesh, part and collection CNA publishes is an owned view; this walk destroys
     * each one as it copies it, so the value that comes back carries no handle and no lifetime.
     * Bones name their parent and children by index rather than by reference, which is what
     * survives a snapshot.
     *
     * @return the model's bones, meshes and root
     */
    public CnaModelGraph getGraph() {
        long model = open();
        List<CnaModelBone> bones = new ArrayList<>();
        long[] view = new long[1];
        check("getBones", NativeModelExtensionRoutes.modelGetBones(model, view));
        long boneCollection = view[0];
        try {
            long[] count = new long[1];
            check("boneCount", NativeModelExtensionRoutes
                    .modelBoneCollectionGetCount(boneCollection, count));
            for (long index = 0; index < count[0]; index++) {
                check("boneAt", NativeModelExtensionRoutes
                        .modelBoneCollectionGetAt(boneCollection, index, view));
                long bone = view[0];
                try {
                    bones.add(readBone(bone));
                } finally {
                    check("boneRelease", NativeModelExtensionRoutes.modelBoneDestroy(bone));
                }
            }
        } finally {
            check("boneCollectionRelease", NativeModelExtensionRoutes
                    .modelBoneCollectionDestroy(boneCollection));
        }

        List<CnaModelMesh> meshes = new ArrayList<>();
        check("getMeshes", NativeModelExtensionRoutes.modelGetMeshes(model, view));
        long meshCollection = view[0];
        try {
            long[] count = new long[1];
            check("meshCount", NativeModelExtensionRoutes
                    .modelMeshCollectionGetCount(meshCollection, count));
            for (long index = 0; index < count[0]; index++) {
                check("meshAt", NativeModelExtensionRoutes
                        .modelMeshCollectionGetAt(meshCollection, index, view));
                long mesh = view[0];
                try {
                    meshes.add(readMesh(mesh));
                } finally {
                    check("meshRelease", NativeModelExtensionRoutes.modelMeshDestroy(mesh));
                }
            }
        } finally {
            check("meshCollectionRelease", NativeModelExtensionRoutes
                    .modelMeshCollectionDestroy(meshCollection));
        }

        return new CnaModelGraph(bones, meshes, rootBoneIndex(model));
    }

    /** Returns how many bone transforms the model carries. */
    public int getBoneCount() {
        long[] count = new long[1];
        check("getBoneCount",
                NativeModelExtensionRoutes.modelGetBoneTransformCount(open(), count));
        return (int) count[0];
    }

    /** Returns each bone's transform relative to its parent, as CNA holds it. */
    public List<Matrix> getBoneTransforms() {
        int count = getBoneCount();
        float[] values = new float[count * 16];
        long[] copied = new long[1];
        check("getBoneTransforms",
                NativeModelExtensionRoutes.modelCopyBoneTransforms(open(), values, copied));
        return matrices(values, (int) copied[0]);
    }

    /**
     * Returns each bone's transform in model space, computed by CNA.
     *
     * <p>This is the same arithmetic XNA's {@code CopyAbsoluteBoneTransformsTo} performs, done by
     * the runtime that owns the hierarchy rather than restated here.
     *
     * @return one matrix per bone, in bone-index order
     */
    public List<Matrix> getAbsoluteBoneTransforms() {
        int count = getBoneCount();
        float[] values = new float[count * 16];
        long[] copied = new long[1];
        check("getAbsoluteBoneTransforms", NativeModelExtensionRoutes
                .modelCopyAbsoluteBoneTransforms(open(), values, copied));
        return matrices(values, (int) copied[0]);
    }

    /**
     * Replaces every bone's transform.
     *
     * @param transforms one matrix per bone, in bone-index order
     */
    public void setBoneTransforms(List<Matrix> transforms) {
        Objects.requireNonNull(transforms, "transforms");
        float[] values = new float[transforms.size() * 16];
        for (int index = 0; index < transforms.size(); index++) {
            writeMatrix(values, index * 16,
                    Objects.requireNonNull(transforms.get(index), "transform"));
        }
        check("setBoneTransforms",
                NativeModelExtensionRoutes.modelSetBoneTransforms(open(), values));
    }

    /**
     * Draws the whole model.
     *
     * <p>CNA sets each effect's matrices and issues the draws itself, which is why this needs no
     * Java facade for the model's effects.
     *
     * @param world where the model sits
     * @param view the camera
     * @param projection the projection
     */
    public void Draw(Matrix world, Matrix view, Matrix projection) {
        check("Draw", NativeModelExtensionRoutes.modelDraw(open(),
                floats(world, "world"), floats(view, "view"),
                floats(projection, "projection")));
    }

    /**
     * Returns the sphere that encloses the whole model.
     *
     * @return the sphere, or {@code null} when the asset recorded none; absent is not a sphere of
     *     radius zero, which would be a model with no extent
     */
    public BoundingSphere getBoundingSphere() {
        boolean[] present = new boolean[1];
        float[] sphere = new float[4];
        check("getBoundingSphere", NativeModelExtensionRoutes
                .modelGetBoundingSphereExt(open(), present, sphere));
        return present[0] ? sphere(sphere) : null;
    }

    /**
     * Poses the model's bones from one clip at a point in time.
     *
     * <p>The result lands in {@link #getBoneTransforms()}, so a game animates an ordinary XNA
     * {@code Model} without a skinning runtime of its own -- which is what XNA left to its
     * {@code SkinnedModel} sample and every game that used it reimplemented.
     *
     * <p><strong>The clip must be a scene-node clip.</strong> Its bone indices select
     * {@code Model.Bones} entries directly; a joint-palette clip's indices select a skinning
     * skeleton's joints, and applying one as the other would pose the wrong bones without
     * failing. CNA refuses it instead, which is the reason the target space is stated rather
     * than inferred.
     *
     * @param animations the set holding the clip
     * @param clipIndex which clip to evaluate
     * @param timeSeconds when to evaluate it, clamped to the clip's duration
     */
    public void applyClipToBones(CnaModelAnimations animations, int clipIndex,
            double timeSeconds) {
        Objects.requireNonNull(animations, "animations");
        check("applyClipToBones", NativeModelExtensionRoutes.modelApplyClipToBonesExt(
                open(), animations.handle(), clipIndex, timeSeconds));
    }

    /**
     * Adds one camera to the model's scene.
     *
     * <p>The write half of {@link #getCameras()}. CNA copies the descriptor, so the value stays
     * the caller's.
     *
     * @param camera the camera to add
     */
    public void addCamera(CnaModelCamera camera) {
        Objects.requireNonNull(camera, "camera");
        check("addCamera", NativeModelExtensionRoutes.modelAddCameraExt(open(),
                camera.toIntegralLeaves(), camera.toFloatingLeaves(),
                CnbExtension.utf8(camera.Name())));
    }

    /** Removes every camera from the model's scene. */
    public void clearCameras() {
        check("clearCameras", NativeModelExtensionRoutes.modelClearCamerasExt(open()));
    }

    /**
     * Adds one skin: a skeleton, and which of the model's meshes it drives.
     *
     * <p>The model <strong>retains the skinning data</strong>, so close the model before the
     * data it was given.
     *
     * @param name the skin's name
     * @param data the skeleton and clips that pose it
     * @param meshIndices which meshes, by index in the model's mesh list, the skin drives
     */
    public void addSkin(String name, CnaSkinningData data, List<Long> meshIndices) {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(data, "data");
        Objects.requireNonNull(meshIndices, "meshIndices");
        long[] indices = new long[meshIndices.size()];
        for (int index = 0; index < indices.length; index++) {
            indices[index] = Objects.requireNonNull(meshIndices.get(index), "meshIndex");
        }
        check("addSkin", NativeModelExtensionRoutes.modelAddSkinExt(
                open(), CnbExtension.utf8(name), data.handle(), indices));
    }

    /** Removes every skin and releases the skinning data they retained. */
    public void clearSkins() {
        check("clearSkins", NativeModelExtensionRoutes.modelClearSkinsExt(open()));
    }

    /**
     * Returns one skin's skeleton, as its own skinning data.
     *
     * @param index the zero-based skin index
     * @return the skeleton and clips, which the caller closes
     */
    public CnaSkinningData getSkinSkeleton(int index) {
        long[] created = new long[1];
        check("getSkinSkeleton", NativeModelExtensionRoutes
                .modelCreateSkinSkeletonHandleExt(open(), index, created));
        return CnaSkinningData.adopt(created[0]);
    }

    /**
     * Poses every skinned effect on the model with a skeleton's bind pose.
     *
     * <p>What a model straight off disk needs before it is drawn: without it the skinned effects
     * carry whatever the last pose left, which for a model that has never been posed is nothing.
     *
     * @param data the skeleton to take the bind pose from
     * @return how many effects were posed
     */
    public int applyBindPoseBoneTransforms(CnaSkinningData data) {
        Objects.requireNonNull(data, "data");
        long[] posed = new long[1];
        check("applyBindPoseBoneTransforms", NativeModelExtensionRoutes
                .modelApplyBindPoseBoneTransformsExt(open(), data.handle(), posed));
        return Math.toIntExact(posed[0]);
    }

    /** Returns the cameras the asset's scene carried. */
    public List<CnaModelCamera> getCameras() {
        long model = open();
        long[] count = new long[1];
        check("getCameraCount",
                NativeModelExtensionRoutes.modelGetCameraCountExt(model, count));
        List<CnaModelCamera> cameras = new ArrayList<>((int) count[0]);
        for (long index = 0; index < count[0]; index++) {
            long[] integral = new long[4];
            float[] floating = new float[36];
            check("getCamera", NativeModelExtensionRoutes
                    .modelGetCameraExt(model, index, integral, floating));
            cameras.add(new CnaModelCamera(
                    text(model, index, TextKind.CameraName),
                    integral[0],
                    matrix(floating, 0),
                    matrix(floating, 16),
                    integral[1] != 0L,
                    floating[32], floating[33], floating[34], floating[35],
                    integral[2] != 0L,
                    integral[3] != 0L));
        }
        return List.copyOf(cameras);
    }

    /** Returns the skins the asset carried. */
    public List<CnaModelSkin> getSkins() {
        long model = open();
        long[] count = new long[1];
        check("getSkinCount", NativeModelExtensionRoutes.modelGetSkinCountExt(model, count));
        List<CnaModelSkin> skins = new ArrayList<>((int) count[0]);
        for (long index = 0; index < count[0]; index++) {
            boolean[] hasData = new boolean[1];
            long[] meshCount = new long[1];
            check("getSkin", NativeModelExtensionRoutes
                    .modelGetSkinExt(model, index, hasData, meshCount));
            List<Long> meshes = new ArrayList<>((int) meshCount[0]);
            for (long mesh = 0; mesh < meshCount[0]; mesh++) {
                long[] modelMesh = new long[1];
                check("getSkinMesh", NativeModelExtensionRoutes
                        .modelGetSkinMeshIndexExt(model, index, mesh, modelMesh));
                meshes.add(modelMesh[0]);
            }
            skins.add(new CnaModelSkin(
                    text(model, index, TextKind.SkinName), hasData[0], meshes));
        }
        return List.copyOf(skins);
    }

    /** Returns the material variants the asset offers, in CNA's order. */
    public List<String> getMaterialVariants() {
        long model = open();
        long[] count = new long[1];
        check("getMaterialVariantCount",
                NativeModelExtensionRoutes.modelGetMaterialVariantCountExt(model, count));
        List<String> variants = new ArrayList<>((int) count[0]);
        for (long index = 0; index < count[0]; index++) {
            variants.add(text(model, index, TextKind.MaterialVariantName));
        }
        return List.copyOf(variants);
    }

    /**
     * Returns which material variant is active.
     *
     * @return the index into {@link #getMaterialVariants()}, or -1 when the asset's own materials
     *     are in use
     */
    public int getMaterialVariant() {
        int[] variant = new int[1];
        check("getMaterialVariant",
                NativeModelExtensionRoutes.modelGetMaterialVariantExt(open(), variant));
        return variant[0];
    }

    /**
     * Chooses a material variant.
     *
     * @param variant an index into {@link #getMaterialVariants()}, or -1 for the asset's own
     *     materials
     */
    public void setMaterialVariant(int variant) {
        check("setMaterialVariant",
                NativeModelExtensionRoutes.modelSetMaterialVariantExt(open(), variant));
    }

    /**
     * Returns what the importer read and what it did to it.
     *
     * <p>A model that came from another content path, or from a document written before CNA
     * recorded this, reads back {@linkplain GltfImportSourceCounts#EMPTY empty} counts and no
     * diagnostics -- not an error, and not a missing report.
     *
     * @return the report; never null
     */
    public GltfImportReport getGltfImportReport() {
        long[] leaves = new long[GltfImportReport.LEAVES];
        check("getGltfImportReport",
                NativeModelExtensionRoutes.modelGetGltfImportReportExt(open(), leaves));
        return GltfImportReport.of(leaves);
    }

    /**
     * Records the shape of the scene this model was imported from, and drops its diagnostics.
     *
     * <p>This is the writing half of provenance, and it takes only the counts CNA stores: the
     * five derived values are recomputed from the diagnostics, and CNA refuses a report that
     * carries them rather than dropping them silently. Build the diagnostics afterwards with
     * {@link #addGltfImportDiagnostic}.
     *
     * @param counts the shape of the source scene
     */
    public void setGltfImportSourceCounts(GltfImportSourceCounts counts) {
        Objects.requireNonNull(counts, "counts");
        check("setGltfImportSourceCounts", NativeModelExtensionRoutes
                .modelSetGltfImportReportExt(open(), GltfImportReport.toLeaves(counts)));
    }

    /**
     * Appends one thing the importer noticed.
     *
     * <p>The diagnostic's four strings are borrowed for the call and copied by CNA, so nothing
     * here has to outlive the call.
     *
     * @param diagnostic what to record
     */
    public void addGltfImportDiagnostic(GltfImportDiagnostic diagnostic) {
        Objects.requireNonNull(diagnostic, "diagnostic");
        List<String> details = diagnostic.Details();
        byte[][] detailBytes = new byte[details.size()][];
        for (int index = 0; index < detailBytes.length; index++) {
            detailBytes[index] = utf8(details.get(index));
        }
        check("addGltfImportDiagnostic", NativeBindings.modelAddGltfImportDiagnostic(open(),
                utf8(diagnostic.Code()), diagnostic.Severity().ordinal(),
                diagnostic.Kind().ordinal(), utf8(diagnostic.Subject()), diagnostic.Count(),
                diagnostic.WorstMagnitude(), detailBytes, utf8(diagnostic.Message())));
    }

    /**
     * Returns the import diagnostics in discovery order.
     *
     * @return the diagnostics, empty when the import recorded none
     */
    public List<GltfImportDiagnostic> getGltfImportDiagnostics() {
        long model = open();
        long count = getGltfImportReport().DiagnosticCount();
        List<GltfImportDiagnostic> diagnostics = new ArrayList<>((int) count);
        for (long index = 0; index < count; index++) {
            long[] integral = new long[DIAGNOSTIC_LEAVES];
            double[] floating = new double[1];
            check("getGltfImportDiagnostic", NativeModelExtensionRoutes
                    .modelGetGltfImportDiagnosticExt(model, index, integral, floating));
            long detailCount = integral[3];
            List<String> details = new ArrayList<>((int) detailCount);
            for (long detail = 0; detail < detailCount; detail++) {
                details.add(diagnosticDetail(model, index, detail));
            }
            diagnostics.add(new GltfImportDiagnostic(
                    diagnosticText(model, index, DiagnosticText.Code),
                    GltfImportSeverity.of((int) integral[0]),
                    GltfImportKind.of((int) integral[1]),
                    diagnosticText(model, index, DiagnosticText.Subject),
                    integral[2],
                    floating[0],
                    details,
                    diagnosticText(model, index, DiagnosticText.Message)));
        }
        return List.copyOf(diagnostics);
    }

    /** How many integral leaves CNA_GltfImportDiagnosticEXT declares after its headers. */
    private static final int DIAGNOSTIC_LEAVES = 4;

    private enum DiagnosticText {
        Code, Subject, Message
    }

    private String diagnosticText(long model, long index, DiagnosticText kind) {
        long[] bytes = new long[1];
        check("diagnosticTextSize", switch (kind) {
            case Code -> NativeModelExtensionRoutes
                    .modelGetGltfImportDiagnosticCodeByteCountExt(model, index, bytes);
            case Subject -> NativeModelExtensionRoutes
                    .modelGetGltfImportDiagnosticSubjectByteCountExt(model, index, bytes);
            case Message -> NativeModelExtensionRoutes
                    .modelGetGltfImportDiagnosticMessageByteCountExt(model, index, bytes);
        });
        byte[] destination = new byte[(int) bytes[0]];
        check("diagnosticText", switch (kind) {
            case Code -> NativeModelExtensionRoutes
                    .modelCopyGltfImportDiagnosticCodeExt(model, index, destination, bytes);
            case Subject -> NativeModelExtensionRoutes
                    .modelCopyGltfImportDiagnosticSubjectExt(model, index, destination, bytes);
            case Message -> NativeModelExtensionRoutes
                    .modelCopyGltfImportDiagnosticMessageExt(model, index, destination, bytes);
        });
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private String diagnosticDetail(long model, long index, long detail) {
        long[] bytes = new long[1];
        check("diagnosticDetailSize", NativeModelExtensionRoutes
                .modelGetGltfImportDiagnosticDetailByteCountExt(model, index, detail, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        check("diagnosticDetail", NativeModelExtensionRoutes
                .modelCopyGltfImportDiagnosticDetailExt(model, index, detail, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    /**
     * Releases the model and the bone, part and mesh handles it created.
     *
     * <p>The vertex buffers, index buffers and effects behind it are the XNA model's and are only
     * released by it: CNA retained them, so releasing here would be a second owner freeing
     * something Java still uses. Closing twice is a no-op.
     */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        RuntimeException failure = null;
        try {
            check("close", NativeModelExtensionRoutes.modelDestroy(handle));
        } catch (RuntimeException exception) {
            failure = exception;
        }
        // Release the children after the model, in the order CNA's own teardown expects: the
        // model held the last reference, so these are the caller's handles going away.
        for (int index = owned.size() - 1; index >= 0; index--) {
            try {
                releaseOwned(owned.get(index));
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        if (failure != null) {
            throw failure;
        }
    }

    /**
     * Releases one created handle.
     *
     * <p>The three kinds are told apart by trying them in turn, because CNA gives each its own
     * release route and the caller recorded only the handle. A route that does not own the handle
     * answers {@code INVALID_HANDLE}, which is how the right one is found rather than guessed.
     */
    private static void releaseOwned(long handle) {
        if (NativeModelExtensionRoutes.modelMeshDestroy(handle) == RESULT_SUCCESS) {
            return;
        }
        if (NativeModelExtensionRoutes.modelMeshPartDestroy(handle) == RESULT_SUCCESS) {
            return;
        }
        check("release", NativeModelExtensionRoutes.modelBoneDestroy(handle));
    }

    private static void releaseAll(List<Long> owned, RuntimeException failure) {
        for (int index = owned.size() - 1; index >= 0; index--) {
            try {
                releaseOwned(owned.get(index));
            } catch (RuntimeException exception) {
                failure.addSuppressed(exception);
            }
        }
    }

    private static GraphicsDevice deviceOf(Model model) {
        for (ModelMesh mesh : model.getMeshes()) {
            for (ModelMeshPart part : mesh.getMeshParts()) {
                if (part.getVertexBuffer() != null) {
                    return part.getVertexBuffer().getGraphicsDevice();
                }
                if (part.getEffect() != null) {
                    return part.getEffect().getGraphicsDevice();
                }
            }
        }
        throw new IllegalArgumentException(
                "model has no vertex buffer or effect to name a graphics device");
    }

    private static long handleOf(Microsoft.Xna.Framework.Graphics.GraphicsResource resource) {
        return resource == null ? 0L : NativeBindings.nativeResourceHandle(resource);
    }

    private static byte[] utf8(String value) {
        return (value == null ? "" : value).getBytes(StandardCharsets.UTF_8);
    }

    private static float[] sphereFloats(BoundingSphere sphere) {
        return new float[] {sphere.Center.X, sphere.Center.Y, sphere.Center.Z, sphere.Radius};
    }

    private CnaModelBone readBone(long bone) {
        int[] index = new int[1];
        check("boneIndex", NativeModelExtensionRoutes.modelBoneGetIndex(bone, index));
        float[] transform = new float[16];
        check("boneTransform",
                NativeModelExtensionRoutes.modelBoneGetTransform(bone, transform));

        int parentIndex = NONE;
        boolean[] present = new boolean[1];
        long[] view = new long[1];
        check("boneParent",
                NativeModelExtensionRoutes.modelBoneGetParent(bone, present, view));
        if (present[0]) {
            long parent = view[0];
            try {
                int[] value = new int[1];
                check("parentIndex", NativeModelExtensionRoutes.modelBoneGetIndex(parent, value));
                parentIndex = value[0];
            } finally {
                check("parentRelease", NativeModelExtensionRoutes.modelBoneDestroy(parent));
            }
        }

        List<Integer> children = new ArrayList<>();
        check("boneChildren", NativeModelExtensionRoutes.modelBoneGetChildren(bone, view));
        long collection = view[0];
        try {
            long[] count = new long[1];
            check("childCount",
                    NativeModelExtensionRoutes.modelBoneCollectionGetCount(collection, count));
            for (long child = 0; child < count[0]; child++) {
                check("childAt", NativeModelExtensionRoutes
                        .modelBoneCollectionGetAt(collection, child, view));
                long childBone = view[0];
                try {
                    int[] value = new int[1];
                    check("childIndex",
                            NativeModelExtensionRoutes.modelBoneGetIndex(childBone, value));
                    children.add(value[0]);
                } finally {
                    check("childRelease",
                            NativeModelExtensionRoutes.modelBoneDestroy(childBone));
                }
            }
        } finally {
            check("childCollectionRelease", NativeModelExtensionRoutes
                    .modelBoneCollectionDestroy(collection));
        }

        return new CnaModelBone(index[0], boneName(bone), matrix(transform, 0),
                parentIndex, children);
    }

    private CnaModelMesh readMesh(long mesh) {
        float[] sphere = new float[4];
        check("meshBounds",
                NativeModelExtensionRoutes.modelMeshGetBoundingSphere(mesh, sphere));

        int parentIndex = NONE;
        boolean[] present = new boolean[1];
        long[] view = new long[1];
        check("meshParent",
                NativeModelExtensionRoutes.modelMeshGetParentBone(mesh, present, view));
        if (present[0]) {
            long parent = view[0];
            try {
                int[] value = new int[1];
                check("meshParentIndex",
                        NativeModelExtensionRoutes.modelBoneGetIndex(parent, value));
                parentIndex = value[0];
            } finally {
                check("meshParentRelease", NativeModelExtensionRoutes.modelBoneDestroy(parent));
            }
        }

        List<CnaModelMeshPart> parts = new ArrayList<>();
        check("meshParts", NativeModelExtensionRoutes.modelMeshGetMeshParts(mesh, view));
        long partCollection = view[0];
        try {
            long[] count = new long[1];
            check("partCount", NativeModelExtensionRoutes
                    .modelMeshPartCollectionGetCount(partCollection, count));
            for (long index = 0; index < count[0]; index++) {
                check("partAt", NativeModelExtensionRoutes
                        .modelMeshPartCollectionGetAt(partCollection, index, view));
                long part = view[0];
                try {
                    parts.add(readPart(part));
                } finally {
                    check("partRelease",
                            NativeModelExtensionRoutes.modelMeshPartDestroy(part));
                }
            }
        } finally {
            check("partCollectionRelease", NativeModelExtensionRoutes
                    .modelMeshPartCollectionDestroy(partCollection));
        }

        List<String> effects = new ArrayList<>();
        check("meshEffects", NativeModelExtensionRoutes.modelMeshGetEffects(mesh, view));
        long effectCollection = view[0];
        try {
            long[] count = new long[1];
            check("effectCount", NativeModelExtensionRoutes
                    .modelEffectCollectionGetCount(effectCollection, count));
            for (long index = 0; index < count[0]; index++) {
                long[] effect = new long[1];
                check("effectAt", NativeModelExtensionRoutes
                        .modelEffectCollectionGetAt(effectCollection, index, effect));
                effects.add(effectTypeName(effect[0]));
            }
        } finally {
            check("effectCollectionRelease", NativeModelExtensionRoutes
                    .modelEffectCollectionDestroy(effectCollection));
        }

        return new CnaModelMesh(meshName(mesh), parentIndex, sphere(sphere), parts, effects);
    }

    private CnaModelMeshPart readPart(long part) {
        int[] numVertices = new int[1];
        int[] primitiveCount = new int[1];
        int[] startIndex = new int[1];
        int[] vertexOffset = new int[1];
        check("partVertices",
                NativeModelExtensionRoutes.modelMeshPartGetNumVertices(part, numVertices));
        check("partPrimitives",
                NativeModelExtensionRoutes.modelMeshPartGetPrimitiveCount(part, primitiveCount));
        check("partStart",
                NativeModelExtensionRoutes.modelMeshPartGetStartIndex(part, startIndex));
        check("partOffset",
                NativeModelExtensionRoutes.modelMeshPartGetVertexOffset(part, vertexOffset));

        boolean[] present = new boolean[1];
        long[] value = new long[1];
        check("partEffect",
                NativeModelExtensionRoutes.modelMeshPartGetEffect(part, present, value));
        String effectTypeName = present[0] ? effectTypeName(value[0]) : "";

        boolean[] hasVertexBuffer = new boolean[1];
        check("partVertexBuffer", NativeModelExtensionRoutes
                .modelMeshPartGetVertexBuffer(part, hasVertexBuffer, value));
        boolean[] hasIndexBuffer = new boolean[1];
        check("partIndexBuffer", NativeModelExtensionRoutes
                .modelMeshPartGetIndexBuffer(part, hasIndexBuffer, value));

        return new CnaModelMeshPart(numVertices[0], primitiveCount[0], startIndex[0],
                vertexOffset[0], effectTypeName, hasVertexBuffer[0], hasIndexBuffer[0]);
    }

    private int rootBoneIndex(long model) {
        boolean[] present = new boolean[1];
        long[] view = new long[1];
        check("getRoot", NativeModelExtensionRoutes.modelGetRoot(model, present, view));
        if (!present[0]) {
            return NONE;
        }
        long root = view[0];
        try {
            int[] index = new int[1];
            check("rootIndex", NativeModelExtensionRoutes.modelBoneGetIndex(root, index));
            return index[0];
        } finally {
            check("rootRelease", NativeModelExtensionRoutes.modelBoneDestroy(root));
        }
    }

    private String boneName(long bone) {
        long[] bytes = new long[1];
        check("boneNameSize",
                NativeModelExtensionRoutes.modelBoneGetNameByteCount(bone, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        check("boneName",
                NativeModelExtensionRoutes.modelBoneCopyName(bone, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private String meshName(long mesh) {
        long[] bytes = new long[1];
        check("meshNameSize",
                NativeModelExtensionRoutes.modelMeshGetNameByteCount(mesh, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        check("meshName",
                NativeModelExtensionRoutes.modelMeshCopyName(mesh, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private String effectTypeName(long effect) {
        long[] bytes = new long[1];
        check("effectTypeNameSize",
                NativeModelExtensionRoutes.effectGetTypeNameByteCount(effect, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        check("effectTypeName",
                NativeModelExtensionRoutes.effectCopyTypeName(effect, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private enum TextKind {
        CameraName, SkinName, MaterialVariantName
    }

    private String text(long model, long index, TextKind kind) {
        long[] bytes = new long[1];
        check("textSize", switch (kind) {
            case CameraName -> NativeModelExtensionRoutes
                    .modelGetCameraNameByteCountExt(model, index, bytes);
            case SkinName -> NativeModelExtensionRoutes
                    .modelGetSkinNameByteCountExt(model, index, bytes);
            case MaterialVariantName -> NativeModelExtensionRoutes
                    .modelGetMaterialVariantNameByteCountExt(model, index, bytes);
        });
        byte[] destination = new byte[(int) bytes[0]];
        check("text", switch (kind) {
            case CameraName -> NativeModelExtensionRoutes
                    .modelCopyCameraNameExt(model, index, destination, bytes);
            case SkinName -> NativeModelExtensionRoutes
                    .modelCopySkinNameExt(model, index, destination, bytes);
            case MaterialVariantName -> NativeModelExtensionRoutes
                    .modelCopyMaterialVariantNameExt(model, index, destination, bytes);
        });
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private static List<Matrix> matrices(float[] values, int count) {
        List<Matrix> transforms = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            transforms.add(matrix(values, index * 16));
        }
        return List.copyOf(transforms);
    }

    private static Matrix matrix(float[] values, int offset) {
        return new Matrix(
                values[offset], values[offset + 1], values[offset + 2], values[offset + 3],
                values[offset + 4], values[offset + 5], values[offset + 6], values[offset + 7],
                values[offset + 8], values[offset + 9], values[offset + 10], values[offset + 11],
                values[offset + 12], values[offset + 13], values[offset + 14],
                values[offset + 15]);
    }

    private static BoundingSphere sphere(float[] values) {
        return new BoundingSphere(
                new Vector3(values[0], values[1], values[2]), values[3]);
    }

    private static float[] floats(Matrix matrix, String name) {
        Objects.requireNonNull(matrix, name);
        float[] values = new float[16];
        writeMatrix(values, 0, matrix);
        return values;
    }

    private static void writeMatrix(float[] values, int offset, Matrix matrix) {
        values[offset] = matrix.M11;
        values[offset + 1] = matrix.M12;
        values[offset + 2] = matrix.M13;
        values[offset + 3] = matrix.M14;
        values[offset + 4] = matrix.M21;
        values[offset + 5] = matrix.M22;
        values[offset + 6] = matrix.M23;
        values[offset + 7] = matrix.M24;
        values[offset + 8] = matrix.M31;
        values[offset + 9] = matrix.M32;
        values[offset + 10] = matrix.M33;
        values[offset + 11] = matrix.M34;
        values[offset + 12] = matrix.M41;
        values[offset + 13] = matrix.M42;
        values[offset + 14] = matrix.M43;
        values[offset + 15] = matrix.M44;
    }

    /** The native handle, package-private so a test can measure the raw route's refusals. */
    long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnaModel is closed");
            }
        }
        return handle;
    }

    private static void check(String operation, int result) {
        if (result == RESULT_SUCCESS) {
            return;
        }
        if (result == RESULT_NOT_SUPPORTED) {
            throw new ContentNotSupportedException("CnaModel." + operation
                    + " is not supported by this CNA build");
        }
        if (result == RESULT_INVALID_STATE) {
            throw new IllegalStateException("CnaModel." + operation + " was refused");
        }
        throw NativeBindings.failure("CnaModel." + operation, result);
    }
}
