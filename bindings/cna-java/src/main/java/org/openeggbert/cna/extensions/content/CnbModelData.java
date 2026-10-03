package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * A model as {@code .cnb} carries it: a bone hierarchy, renderable parts, the meshes that group
 * them, an optional skinning skeleton and any baked lights.
 *
 * <p><strong>This is content, not a scene.</strong> Nothing here touches a graphics device, so a
 * build step or an inspector can read, rewrite and write a model with no window open. That is
 * also the boundary: there is no {@code toModel(GraphicsDevice)}, because XNA's {@code Model} has
 * no public constructor and CNA offers no route that builds one from this description. A game
 * loads a model through the content manager as it always did; this is for the tooling on the
 * other side of that file.
 *
 * <p>{@link CnaModel} is the other direction and a different thing: it wraps an XNA {@code Model}
 * a content manager already produced, to answer questions {@code Model} has nowhere to put.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op.
 */
public final class CnbModelData implements AutoCloseable {

    /** CNA's own selector for the skeleton's three matrix sets. */
    private static final int SKELETON_BIND_POSE = 0;
    private static final int SKELETON_INVERSE_BIND_POSE = 1;
    private static final int SKELETON_ROOT_PREFIX = 2;

    /** Sixteen floats to a matrix, which is what every skeleton matrix route counts in. */
    private static final int MATRIX_FLOATS = 16;

    private final long handle;
    private boolean closed;

    CnbModelData(long handle) {
        this.handle = handle;
    }

    /**
     * Creates an empty model.
     *
     * @return the model data, which the caller closes
     */
    public static CnbModelData create() {
        CnbExtension.requireAvailable();
        long[] model = new long[1];
        CnbExtension.check("CnbModelData.create", NativeCnbRoutes.cnbModelCreate(model));
        return new CnbModelData(model[0]);
    }

    /** Returns what the model contains. */
    public CnbModelInfo getInfo() {
        long[] values = new long[9];
        CnbExtension.check("CnbModelData.getInfo",
                NativeCnbRoutes.cnbModelGetInfo(open(), values));
        return new CnbModelInfo((int) values[0], (int) values[1], (int) values[2],
                (int) values[3], (int) values[4], values[5] != 0, values[6] != 0,
                values[7] != 0);
    }

    /**
     * Sets the two facts about the model as a whole.
     *
     * @param appliesGltfLightingPolicy whether the source asset's glTF lighting rules apply
     * @param hasBoneHierarchy whether the bones form a hierarchy rather than a flat list
     */
    public void setFlags(boolean appliesGltfLightingPolicy, boolean hasBoneHierarchy) {
        CnbExtension.check("CnbModelData.setFlags", NativeCnbRoutes
                .cnbModelSetFlags(open(), appliesGltfLightingPolicy, hasBoneHierarchy));
    }

    /**
     * Appends one bone.
     *
     * @param name the bone's name, which may be empty
     * @param parent the parent bone's index, or -1 for a root
     * @param transform the bone's transform relative to its parent
     * @return the new bone's index
     */
    public int addBone(String name, int parent, Matrix transform) {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(transform, "transform");
        long[] index = new long[1];
        CnbExtension.check("CnbModelData.addBone", NativeCnbRoutes.cnbModelAddBone(
                open(), CnbExtension.utf8(name), parent, floats(transform), index));
        return (int) index[0];
    }

    /**
     * Returns one bone.
     *
     * @param index the zero-based bone index
     * @return its name, parent and transform
     */
    public CnbModelBone getBone(int index) {
        long model = open();
        long[] integral = new long[2];
        float[] floating = new float[MATRIX_FLOATS];
        CnbExtension.check("CnbModelData.getBone",
                NativeCnbRoutes.cnbModelGetBone(model, index, integral, floating));
        return new CnbModelBone(
                CnbExtension.text("CnbModelData.getBone",
                        bytes -> NativeCnbRoutes.cnbModelGetBoneNameSize(model, index, bytes),
                        (destination, bytes) -> NativeCnbRoutes
                                .cnbModelCopyBoneName(model, index, destination, bytes)),
                (int) integral[0], matrix(floating, 0));
    }

    /** Returns every bone in index order. */
    public List<CnbModelBone> getBones() {
        return read(getInfo().BoneCount(), this::getBone);
    }

    /**
     * Appends one renderable part with empty geometry.
     *
     * @param part the part's numeric state
     * @param name the part's name, which may be empty
     * @param externalEffect the effect asset name, used only by
     *        {@link CnbEffectKind#External} and empty otherwise
     * @return the new part's index
     */
    public int addPart(CnbModelPart part, String name, String externalEffect) {
        Objects.requireNonNull(part, "part");
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(externalEffect, "externalEffect");
        long[] index = new long[1];
        CnbExtension.check("CnbModelData.addPart", NativeCnbRoutes.cnbModelAddPart(
                open(), new byte[2], partLeaves(part), CnbExtension.utf8(name),
                CnbExtension.utf8(externalEffect), index));
        return (int) index[0];
    }

    /**
     * Returns one part's numeric state.
     *
     * @param index the zero-based part index
     * @return that part's description, without its bytes
     */
    public CnbModelPart getPart(int index) {
        long[] values = new long[9];
        CnbExtension.check("CnbModelData.getPart",
                NativeCnbRoutes.cnbModelGetPart(open(), index, new byte[2], values));
        return new CnbModelPart((int) values[0], (int) values[1], (int) values[2],
                (int) values[3], (int) values[4], (int) values[5],
                CnbEffectKind.fromValue(values[6]), values[7] != 0, values[8] != 0);
    }

    /**
     * Replaces one part's numeric state, leaving its bytes alone.
     *
     * @param index the zero-based part index
     * @param part the replacement description
     */
    public void setPart(int index, CnbModelPart part) {
        Objects.requireNonNull(part, "part");
        CnbExtension.check("CnbModelData.setPart", NativeCnbRoutes
                .cnbModelSetPart(open(), index, new byte[2], partLeaves(part)));
    }

    /**
     * Returns one part's name.
     *
     * @param index the zero-based part index
     * @return its name, empty when it has none
     */
    public String getPartName(int index) {
        long model = open();
        return CnbExtension.text("CnbModelData.getPartName",
                bytes -> NativeCnbRoutes.cnbModelGetPartNameSize(model, index, bytes),
                (destination, bytes) -> NativeCnbRoutes
                        .cnbModelCopyPartName(model, index, destination, bytes));
    }

    /**
     * Returns the effect asset one part names for itself.
     *
     * @param index the zero-based part index
     * @return the asset name, empty unless the part's kind is {@link CnbEffectKind#External}
     */
    public String getPartExternalEffect(int index) {
        long model = open();
        return CnbExtension.text("CnbModelData.getPartExternalEffect",
                bytes -> NativeCnbRoutes
                        .cnbModelGetPartExternalEffectSize(model, index, bytes),
                (destination, bytes) -> NativeCnbRoutes
                        .cnbModelCopyPartExternalEffect(model, index, destination, bytes));
    }

    /**
     * Replaces one part's vertex payload.
     *
     * @param index the zero-based part index
     * @param bytes the vertex bytes; CNA copies them
     */
    public void setPartVertexBytes(int index, byte[] bytes) {
        Objects.requireNonNull(bytes, "bytes");
        CnbExtension.check("CnbModelData.setPartVertexBytes",
                NativeCnbRoutes.cnbModelSetPartVertexBytes(open(), index, bytes));
    }

    /**
     * Returns one part's vertex payload.
     *
     * @param index the zero-based part index
     * @return a fresh copy of the bytes
     */
    public byte[] readPartVertexBytes(int index) {
        long model = open();
        return CnbExtension.bytes("CnbModelData.readPartVertexBytes",
                (destination, count) -> NativeCnbRoutes
                        .cnbModelCopyPartVertexBytes(model, index, destination, count));
    }

    /**
     * Replaces one part's index payload.
     *
     * @param index the zero-based part index
     * @param bytes the index bytes; CNA copies them
     */
    public void setPartIndexBytes(int index, byte[] bytes) {
        Objects.requireNonNull(bytes, "bytes");
        CnbExtension.check("CnbModelData.setPartIndexBytes",
                NativeCnbRoutes.cnbModelSetPartIndexBytes(open(), index, bytes));
    }

    /**
     * Returns one part's index payload.
     *
     * @param index the zero-based part index
     * @return a fresh copy of the bytes
     */
    public byte[] readPartIndexBytes(int index) {
        long model = open();
        return CnbExtension.bytes("CnbModelData.readPartIndexBytes",
                (destination, count) -> NativeCnbRoutes
                        .cnbModelCopyPartIndexBytes(model, index, destination, count));
    }

    /**
     * Appends one mesh over parts that already exist.
     *
     * @param name the mesh's name, which may be empty
     * @param parentBone the bone index the mesh follows, or -1 for none
     * @param partIndices the parts the mesh draws, in order
     * @return the new mesh's index
     */
    public int addMesh(String name, int parentBone, int... partIndices) {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(partIndices, "partIndices");
        long[] index = new long[1];
        CnbExtension.check("CnbModelData.addMesh", NativeCnbRoutes.cnbModelAddMesh(
                open(), CnbExtension.utf8(name), parentBone, partIndices.clone(), index));
        return (int) index[0];
    }

    /**
     * Returns one mesh.
     *
     * @param index the zero-based mesh index
     * @return its name, parent bone and parts
     */
    public CnbModelMesh getMesh(int index) {
        long model = open();
        long[] values = new long[3];
        CnbExtension.check("CnbModelData.getMesh",
                NativeCnbRoutes.cnbModelGetMesh(model, index, values));
        int[] parts = new int[Math.toIntExact(values[2])];
        long[] count = new long[1];
        CnbExtension.check("CnbModelData.getMesh", NativeCnbRoutes
                .cnbModelCopyMeshPartIndices(model, index, parts, count));
        List<Integer> indices = new ArrayList<>(parts.length);
        for (int part = 0; part < count[0]; part++) {
            indices.add(parts[part]);
        }
        return new CnbModelMesh(
                CnbExtension.text("CnbModelData.getMesh",
                        bytes -> NativeCnbRoutes.cnbModelGetMeshNameSize(model, index, bytes),
                        (destination, bytes) -> NativeCnbRoutes
                                .cnbModelCopyMeshName(model, index, destination, bytes)),
                (int) values[0], indices);
    }

    /** Returns every mesh in index order. */
    public List<CnbModelMesh> getMeshes() {
        return read(getInfo().MeshCount(), this::getMesh);
    }

    /**
     * Gives the model a skinning skeleton, replacing any it had.
     *
     * @param skeleton the joints, their hierarchy and their bind poses
     */
    public void setSkeleton(CnbSkeleton skeleton) {
        Objects.requireNonNull(skeleton, "skeleton");
        int joints = skeleton.getJointCount();
        int[] hierarchy = new int[joints];
        for (int joint = 0; joint < joints; joint++) {
            hierarchy[joint] = skeleton.Hierarchy().get(joint);
        }
        CnbExtension.check("CnbModelData.setSkeleton", NativeCnbRoutes.cnbModelSetSkeleton(
                open(), hierarchy, floats(skeleton.BindPose()),
                floats(skeleton.InverseBindPose()),
                skeleton.RootPrefix() == null ? null : floats(skeleton.RootPrefix())));
    }

    /** Removes the model's skinning skeleton, leaving its bones and parts. */
    public void clearSkeleton() {
        CnbExtension.check("CnbModelData.clearSkeleton",
                NativeCnbRoutes.cnbModelClearSkeleton(open()));
    }

    /**
     * Returns the model's skinning skeleton.
     *
     * @return the skeleton, or null when the model has none
     */
    public CnbSkeleton getSkeleton() {
        if (!getInfo().HasSkeleton()) {
            return null;
        }
        long model = open();
        long[] values = new long[2];
        CnbExtension.check("CnbModelData.getSkeleton",
                NativeCnbRoutes.cnbModelGetSkeleton(model, new byte[7], values));
        int joints = Math.toIntExact(values[0]);
        boolean hasRootPrefix = values[1] != 0;
        int[] hierarchy = new int[joints];
        long[] count = new long[1];
        CnbExtension.check("CnbModelData.getSkeleton",
                NativeCnbRoutes.cnbModelCopySkeletonHierarchy(model, hierarchy, count));
        List<Integer> parents = new ArrayList<>(joints);
        for (int joint = 0; joint < joints; joint++) {
            parents.add(hierarchy[joint]);
        }
        return new CnbSkeleton(parents,
                matrices(model, SKELETON_BIND_POSE, joints),
                matrices(model, SKELETON_INVERSE_BIND_POSE, joints),
                hasRootPrefix ? matrices(model, SKELETON_ROOT_PREFIX, joints) : null);
    }

    /**
     * Appends one baked directional light.
     *
     * @param light its direction and diffuse colour
     * @return the new light's index
     */
    public int addLight(CnbModelLight light) {
        Objects.requireNonNull(light, "light");
        Vector3 direction = Objects.requireNonNull(light.Direction(), "light.Direction");
        Vector3 diffuse = Objects.requireNonNull(light.DiffuseColor(), "light.DiffuseColor");
        long[] index = new long[1];
        CnbExtension.check("CnbModelData.addLight", NativeCnbRoutes.cnbModelAddLight(
                open(),
                new float[] {direction.X, direction.Y, direction.Z,
                    diffuse.X, diffuse.Y, diffuse.Z},
                index));
        return (int) index[0];
    }

    /**
     * Returns one baked light.
     *
     * @param index the zero-based light index
     * @return its direction and diffuse colour
     */
    public CnbModelLight getLight(int index) {
        float[] values = new float[6];
        CnbExtension.check("CnbModelData.getLight",
                NativeCnbRoutes.cnbModelGetLight(open(), index, values));
        return new CnbModelLight(new Vector3(values[0], values[1], values[2]),
                new Vector3(values[3], values[4], values[5]));
    }

    /**
     * Returns one part's material factors.
     *
     * @param part the zero-based part index
     * @return its factors, without its textures
     */
    public CnbMaterial getMaterial(int part) {
        long[] integral = new long[2];
        float[] floating = new float[17];
        CnbExtension.check("CnbModelData.getMaterial", NativeCnbRoutes
                .cnbModelGetMaterial(open(), part, new byte[3], integral, floating));
        return new CnbMaterial(
                new Vector4(floating[0], floating[1], floating[2], floating[3]),
                new Vector3(floating[4], floating[5], floating[6]),
                new Vector3(floating[7], floating[8], floating[9]),
                floating[10], floating[11], floating[12], floating[13], floating[14],
                floating[15], floating[16],
                CnbAlphaMode.fromValue(integral[0]), integral[1] != 0);
    }

    /**
     * Replaces one part's material factors, leaving its textures alone.
     *
     * @param part the zero-based part index
     * @param material the replacement factors
     */
    public void setMaterial(int part, CnbMaterial material) {
        Objects.requireNonNull(material, "material");
        Vector4 base = Objects.requireNonNull(
                material.BaseColorFactor(), "material.BaseColorFactor");
        Vector3 emissive = Objects.requireNonNull(
                material.EmissiveFactor(), "material.EmissiveFactor");
        Vector3 specular = Objects.requireNonNull(
                material.SpecularColorFactor(), "material.SpecularColorFactor");
        float[] floating = {
            base.X, base.Y, base.Z, base.W,
            emissive.X, emissive.Y, emissive.Z,
            specular.X, specular.Y, specular.Z,
            material.MetallicFactor(), material.RoughnessFactor(), material.Ior(),
            material.SpecularFactor(), material.NormalScale(), material.OcclusionStrength(),
            material.AlphaCutoff(),
        };
        long[] integral = {
            Objects.requireNonNull(material.AlphaMode(), "material.AlphaMode").ordinal(),
            material.DoubleSided() ? 1 : 0,
        };
        CnbExtension.check("CnbModelData.setMaterial", NativeCnbRoutes
                .cnbModelSetMaterial(open(), part, new byte[3], integral, floating));
    }

    /**
     * Returns the texture asset one material slot names.
     *
     * @param part the zero-based part index
     * @param slot the slot to read
     * @return the asset name, empty when the slot carries no texture
     */
    public String getMaterialTexture(int part, CnbMaterialTextureSlot slot) {
        long model = open();
        Objects.requireNonNull(slot, "slot");
        return CnbExtension.text("CnbModelData.getMaterialTexture",
                bytes -> NativeCnbRoutes.cnbModelGetMaterialTextureSize(
                        model, part, slot.ordinal(), bytes),
                (destination, bytes) -> NativeCnbRoutes.cnbModelCopyMaterialTexture(
                        model, part, slot.ordinal(), destination, bytes));
    }

    /**
     * Names the texture asset one material slot uses.
     *
     * @param part the zero-based part index
     * @param slot the slot to fill
     * @param assetName the content name of the texture, or empty to clear the slot
     */
    public void setMaterialTexture(int part, CnbMaterialTextureSlot slot, String assetName) {
        Objects.requireNonNull(slot, "slot");
        Objects.requireNonNull(assetName, "assetName");
        CnbExtension.check("CnbModelData.setMaterialTexture", NativeCnbRoutes
                .cnbModelSetMaterialTexture(open(), part, slot.ordinal(),
                        CnbExtension.utf8(assetName)));
    }

    /**
     * Returns which set of texture coordinates one slot samples with.
     *
     * <p>Addressed with {@link CnbImporterTextureSlot}, not {@link CnbMaterialTextureSlot}: the
     * per-slot arrays are seven entries in the importer's order and the names are eight in CNA's
     * effect order. Confusing them in C silently reads the wrong slot; here they are two types.
     *
     * @param part the zero-based part index
     * @param slot the importer slot to read
     * @return the coordinate set index, zero for the first
     */
    public int getMaterialTextureCoordinateSet(int part, CnbImporterTextureSlot slot) {
        Objects.requireNonNull(slot, "slot");
        byte[] value = new byte[1];
        CnbExtension.check("CnbModelData.getMaterialTextureCoordinateSet", NativeCnbRoutes
                .cnbModelGetMaterialTextureCoordinateSet(open(), part, slot.ordinal(), value));
        return value[0] & 0xFF;
    }

    /**
     * Sets which set of texture coordinates one slot samples with.
     *
     * @param part the zero-based part index
     * @param slot the slot to change
     * @param coordinateSet the coordinate set index, zero for the first
     */
    public void setMaterialTextureCoordinateSet(
            int part, CnbImporterTextureSlot slot, int coordinateSet) {
        Objects.requireNonNull(slot, "slot");
        if (coordinateSet < 0 || coordinateSet > 0xFF) {
            throw new IllegalArgumentException(
                    "a texture coordinate set is one byte, not " + coordinateSet);
        }
        CnbExtension.check("CnbModelData.setMaterialTextureCoordinateSet", NativeCnbRoutes
                .cnbModelSetMaterialTextureCoordinateSet(
                        open(), part, slot.ordinal(), (byte) coordinateSet));
    }

    /**
     * Returns how one slot's texture coordinates are transformed.
     *
     * @param part the zero-based part index
     * @param slot the slot to read
     * @return the scale, rotation and offset applied before sampling
     */
    public CnbTextureTransform getMaterialTextureTransform(
            int part, CnbImporterTextureSlot slot) {
        Objects.requireNonNull(slot, "slot");
        float[] values = new float[5];
        CnbExtension.check("CnbModelData.getMaterialTextureTransform", NativeCnbRoutes
                .cnbModelGetMaterialTextureTransform(open(), part, slot.ordinal(), values));
        return new CnbTextureTransform(values[0], values[1], values[2], values[3], values[4]);
    }

    /**
     * Sets how one slot's texture coordinates are transformed.
     *
     * @param part the zero-based part index
     * @param slot the slot to change
     * @param transform the scale, rotation and offset to apply
     */
    public void setMaterialTextureTransform(
            int part, CnbImporterTextureSlot slot, CnbTextureTransform transform) {
        Objects.requireNonNull(slot, "slot");
        Objects.requireNonNull(transform, "transform");
        CnbExtension.check("CnbModelData.setMaterialTextureTransform", NativeCnbRoutes
                .cnbModelSetMaterialTextureTransform(open(), part, slot.ordinal(),
                        new float[] {transform.OffsetX(), transform.OffsetY(),
                            transform.ScaleX(), transform.ScaleY(), transform.Rotation()}));
    }

    /**
     * Returns how one slot is sampled.
     *
     * @param part the zero-based part index
     * @param slot the slot to read
     * @return its sampler, whose {@code Declared} says whether the file states one at all
     */
    public CnbSamplerState getMaterialSampler(int part, CnbImporterTextureSlot slot) {
        Objects.requireNonNull(slot, "slot");
        long[] values = new long[4];
        CnbExtension.check("CnbModelData.getMaterialSampler", NativeCnbRoutes
                .cnbModelGetMaterialSampler(open(), part, slot.ordinal(), new byte[3], values));
        return new CnbSamplerState((int) values[0], (int) values[1], (int) values[2],
                values[3] != 0);
    }

    /**
     * Sets how one slot is sampled.
     *
     * @param part the zero-based part index
     * @param slot the slot to change
     * @param sampler the sampler to record; one with {@code Declared} false states nothing and
     *        leaves the choice to the renderer
     */
    public void setMaterialSampler(
            int part, CnbImporterTextureSlot slot, CnbSamplerState sampler) {
        Objects.requireNonNull(slot, "slot");
        Objects.requireNonNull(sampler, "sampler");
        CnbExtension.check("CnbModelData.setMaterialSampler", NativeCnbRoutes
                .cnbModelSetMaterialSampler(open(), part, slot.ordinal(), new byte[3],
                        new long[] {sampler.Filter(), sampler.AddressU(), sampler.AddressV(),
                            sampler.Declared() ? 1 : 0}));
    }

    /**
     * Reports whether one part carries blend shapes.
     *
     * @param part the zero-based part index
     * @return true when the part has morph targets
     */
    public boolean hasMorph(int part) {
        boolean[] present = new boolean[1];
        CnbExtension.check("CnbModelData.hasMorph",
                NativeCnbRoutes.cnbModelHasMorph(open(), part, present));
        return present[0];
    }

    /**
     * Returns one part's morph state.
     *
     * @param part the zero-based part index
     * @return its blend-shape counts and weight-track flags
     */
    public CnbMorphInfo getMorph(int part) {
        long[] values = new long[8];
        CnbExtension.check("CnbModelData.getMorph",
                NativeCnbRoutes.cnbModelGetMorph(open(), part, new byte[5], values));
        return new CnbMorphInfo((int) values[0], (int) values[2], (int) values[3],
                (int) values[4], values[5] != 0, values[6] != 0, values[7] != 0);
    }

    /**
     * Sets one part's morph state.
     *
     * <p>This is what <em>creates</em> a part's morph block; {@link #addMorphTarget} then adds
     * shapes to it. Doing it the other way round is refused with "the part carries no
     * morph-target data".
     *
     * <p>The target, weight and key counts are CNA's own: they follow from what the part actually
     * holds, and this sends zeros for them exactly as CNA's own encoder does. What a caller sets
     * is the vertex count and the three flags.
     *
     * @param part the zero-based part index
     * @param vertexCount how many vertices each target's deltas cover
     * @param recomputeFlatNormals whether normals are recomputed after blending
     * @param stepInterpolation whether the weight track steps rather than interpolates
     * @param cubicSpline whether the weight track is a cubic spline
     */
    public void setMorph(int part, int vertexCount, boolean recomputeFlatNormals,
            boolean stepInterpolation, boolean cubicSpline) {
        long[] values = {
            vertexCount, 0, 0, 0, 0,
            recomputeFlatNormals ? 1 : 0, stepInterpolation ? 1 : 0, cubicSpline ? 1 : 0,
        };
        CnbExtension.check("CnbModelData.setMorph",
                NativeCnbRoutes.cnbModelSetMorph(open(), part, new byte[5], values));
    }

    /**
     * Removes one part's blend shapes entirely.
     *
     * @param part the zero-based part index
     */
    public void clearMorph(int part) {
        CnbExtension.check("CnbModelData.clearMorph",
                NativeCnbRoutes.cnbModelClearMorph(open(), part));
    }

    /**
     * Appends one blend shape to a part, with no deltas yet.
     *
     * @param part the zero-based part index
     * @return the new target's index
     */
    public int addMorphTarget(int part) {
        long[] index = new long[1];
        CnbExtension.check("CnbModelData.addMorphTarget",
                NativeCnbRoutes.cnbModelAddMorphTarget(open(), part, index));
        return (int) index[0];
    }

    /**
     * Sets one blend shape's per-vertex displacements for one stream.
     *
     * @param part the zero-based part index
     * @param target the zero-based target index
     * @param stream which per-vertex stream the deltas belong to
     * @param values three floats per vertex; CNA copies them
     */
    public void setMorphTargetDeltas(
            int part, int target, CnbMorphDeltaStream stream, float[] values) {
        Objects.requireNonNull(stream, "stream");
        Objects.requireNonNull(values, "values");
        CnbExtension.check("CnbModelData.setMorphTargetDeltas", NativeCnbRoutes
                .cnbModelSetMorphTargetDeltas(open(), part, target, stream.ordinal(), values));
    }

    /**
     * Returns one blend shape's per-vertex displacements for one stream.
     *
     * @param part the zero-based part index
     * @param target the zero-based target index
     * @param stream which per-vertex stream to read
     * @return a fresh copy of the deltas, three floats per vertex
     */
    public float[] readMorphTargetDeltas(int part, int target, CnbMorphDeltaStream stream) {
        Objects.requireNonNull(stream, "stream");
        long model = open();
        return CnbExtension.floats("CnbModelData.readMorphTargetDeltas",
                (destination, count) -> NativeCnbRoutes.cnbModelCopyMorphTargetDeltas(
                        model, part, target, stream.ordinal(), destination, count));
    }

    /**
     * Sets the weights a part blends its shapes with.
     *
     * @param part the zero-based part index
     * @param values one weight per blend shape being blended; CNA copies them
     */
    public void setMorphWeights(int part, float[] values) {
        Objects.requireNonNull(values, "values");
        CnbExtension.check("CnbModelData.setMorphWeights",
                NativeCnbRoutes.cnbModelSetMorphWeights(open(), part, values));
    }

    /**
     * Returns the weights a part blends its shapes with.
     *
     * @param part the zero-based part index
     * @return a fresh copy of the weights
     */
    public float[] readMorphWeights(int part) {
        long model = open();
        return CnbExtension.floats("CnbModelData.readMorphWeights",
                (destination, count) -> NativeCnbRoutes
                        .cnbModelCopyMorphWeights(model, part, destination, count));
    }

    /**
     * Appends one key to a part's morph weight track.
     *
     * @param part the zero-based part index
     * @param timeSeconds when in the track this key applies
     * @param weights one weight per blend shape
     * @param inTangents the incoming tangents, or null for a track that is not a cubic spline
     * @param outTangents the outgoing tangents, or null for the same reason
     * @return the new key's index
     */
    public int addMorphWeightKey(int part, double timeSeconds, float[] weights,
            float[] inTangents, float[] outTangents) {
        Objects.requireNonNull(weights, "weights");
        long[] index = new long[1];
        CnbExtension.check("CnbModelData.addMorphWeightKey",
                NativeCnbRoutes.cnbModelAddMorphWeightKey(open(), part, timeSeconds, weights,
                        inTangents == null ? new float[0] : inTangents,
                        outTangents == null ? new float[0] : outTangents, index));
        return (int) index[0];
    }

    /**
     * Returns one morph weight key's time and array lengths.
     *
     * @param part the zero-based part index
     * @param key the zero-based key index
     * @return its time and how many weights and tangents it carries
     */
    public CnbMorphWeightKey getMorphWeightKey(int part, int key) {
        long[] integral = new long[3];
        double[] doubles = new double[1];
        CnbExtension.check("CnbModelData.getMorphWeightKey", NativeCnbRoutes
                .cnbModelGetMorphWeightKey(open(), part, key, integral, doubles));
        return new CnbMorphWeightKey(doubles[0], (int) integral[0], (int) integral[1],
                (int) integral[2]);
    }

    /**
     * Returns one morph weight key's values for one stream.
     *
     * @param part the zero-based part index
     * @param key the zero-based key index
     * @param stream which of the key's three arrays to read
     * @return a fresh copy of that array
     */
    public float[] readMorphWeightKeyValues(int part, int key, CnbMorphKeyStream stream) {
        Objects.requireNonNull(stream, "stream");
        long model = open();
        return CnbExtension.floats("CnbModelData.readMorphWeightKeyValues",
                (destination, count) -> NativeCnbRoutes.cnbModelCopyMorphWeightKeyValues(
                        model, part, key, stream.ordinal(), destination, count));
    }

    /**
     * Returns one animation clip's name, duration, track count and index space.
     *
     * @param index the zero-based animation index
     * @return that clip's description
     */
    public CnbAnimation getAnimation(int index) {
        long model = open();
        double[] duration = new double[1];
        long[] tracks = new long[1];
        int[] space = new int[1];
        CnbExtension.check("CnbModelData.getAnimation", NativeCnbRoutes
                .cnbModelGetAnimation(model, index, duration, tracks, space));
        return new CnbAnimation(
                CnbExtension.text("CnbModelData.getAnimation",
                        bytes -> NativeCnbRoutes
                                .cnbModelGetAnimationNameSize(model, index, bytes),
                        (destination, bytes) -> NativeCnbRoutes
                                .cnbModelCopyAnimationName(model, index, destination, bytes)),
                duration[0], (int) tracks[0], CnbClipTargetSpace.fromValue(space[0]));
    }

    /**
     * Adds one named animation clip to the model.
     *
     * <p>The write half of {@link #getAnimation(int)}, and it was the missing one: CNA takes a
     * clip as a pointer graph the JNI generator refuses, so until now a model could be read for
     * its animations and never given any. The graph is built in the adapter for the duration of
     * one call and the model deeply copies it, so the clip stays the caller's value.
     *
     * @param name the clip's name, which {@link CnbAnimation#Name()} reports back
     * @param clip the clip to copy in
     * @param targetSpace which index space the tracks' bone indices live in
     * @return the clip's zero-based index in the model
     */
    public int addAnimation(String name, CnbClip clip, CnbClipTargetSpace targetSpace) {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(clip, "clip");
        Objects.requireNonNull(targetSpace, "targetSpace");
        CnbClip.Flattened flat = clip.flatten();
        long[] index = new long[1];
        CnbExtension.check("CnbModelData.addAnimation", NativeBindings.cnbModelAddAnimation(
                handle(), CnbExtension.utf8(name), clip.DurationSeconds(), flat.boneIndices(),
                flat.keyframeCounts(), flat.times(), flat.values(), targetSpace.ordinal(),
                index));
        return Math.toIntExact(index[0]);
    }

    /** Returns every animation clip in index order. */
    public List<CnbAnimation> getAnimations() {
        return read(getInfo().AnimationCount(), this::getAnimation);
    }

    /**
     * Returns which bone one animation track drives, and how many keyframes it has.
     *
     * @param index the zero-based animation index
     * @param track the zero-based track index
     * @return the bone index in the clip's own space, and the keyframe count
     */
    public int[] getAnimationTrack(int index, int track) {
        int[] bone = new int[1];
        long[] keyframes = new long[1];
        CnbExtension.check("CnbModelData.getAnimationTrack", NativeCnbRoutes
                .cnbModelGetAnimationTrack(open(), index, track, bone, keyframes));
        return new int[] {bone[0], Math.toIntExact(keyframes[0])};
    }

    /**
     * Returns one animation track's keyframes.
     *
     * @param index the zero-based animation index
     * @param track the zero-based track index
     * @return the poses that track holds, in time order
     */
    public List<CnbKeyframe> readAnimationKeyframes(int index, int track) {
        long model = open();
        int count = getAnimationTrack(index, track)[1];
        float[] floating = new float[Math.multiplyExact(count, 10)];
        double[] doubles = new double[count];
        long[] written = new long[1];
        CnbExtension.check("CnbModelData.readAnimationKeyframes", NativeCnbRoutes
                .cnbModelCopyAnimationKeyframes(model, index, track, floating, doubles, written));
        List<CnbKeyframe> keyframes = new ArrayList<>(count);
        for (int keyframe = 0; keyframe < written[0]; keyframe++) {
            int base = keyframe * 10;
            keyframes.add(new CnbKeyframe(doubles[keyframe],
                    new Vector3(floating[base], floating[base + 1], floating[base + 2]),
                    new Quaternion(floating[base + 3], floating[base + 4],
                            floating[base + 5], floating[base + 6]),
                    new Vector3(floating[base + 7], floating[base + 8], floating[base + 9])));
        }
        return List.copyOf(keyframes);
    }

    long handle() {
        return open();
    }

    /** Releases the model data. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbModelData.close", NativeCnbRoutes.cnbModelDestroy(handle));
    }

    private List<Matrix> matrices(long model, int set, int joints) {
        float[] values = new float[Math.multiplyExact(joints, MATRIX_FLOATS)];
        long[] count = new long[1];
        CnbExtension.check("CnbModelData.getSkeleton", NativeCnbRoutes
                .cnbModelCopySkeletonMatrices(model, set, values, count));
        List<Matrix> result = new ArrayList<>(joints);
        for (int joint = 0; joint < joints; joint++) {
            result.add(matrix(values, joint * MATRIX_FLOATS));
        }
        return result;
    }

    private static <T> List<T> read(int count, java.util.function.IntFunction<T> reader) {
        List<T> values = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            values.add(reader.apply(index));
        }
        return List.copyOf(values);
    }

    private static long[] partLeaves(CnbModelPart part) {
        return new long[] {
            part.VertexStride(), part.VertexCount(), part.IndexCount(),
            part.IndexElementSize(), part.PrimitiveTopology(), part.PrimitiveCount(),
            Objects.requireNonNull(part.EffectKind(), "part.EffectKind").ordinal(),
            part.VertexColorEnabled() ? 1 : 0, part.Unlit() ? 1 : 0,
        };
    }

    private static float[] floats(List<Matrix> matrices) {
        float[] values = new float[Math.multiplyExact(matrices.size(), MATRIX_FLOATS)];
        for (int index = 0; index < matrices.size(); index++) {
            System.arraycopy(floats(matrices.get(index)), 0, values,
                    index * MATRIX_FLOATS, MATRIX_FLOATS);
        }
        return values;
    }

    private static float[] floats(Matrix matrix) {
        return new float[] {
            matrix.M11, matrix.M12, matrix.M13, matrix.M14,
            matrix.M21, matrix.M22, matrix.M23, matrix.M24,
            matrix.M31, matrix.M32, matrix.M33, matrix.M34,
            matrix.M41, matrix.M42, matrix.M43, matrix.M44,
        };
    }

    private static Matrix matrix(float[] values, int offset) {
        return new Matrix(
                values[offset], values[offset + 1], values[offset + 2], values[offset + 3],
                values[offset + 4], values[offset + 5], values[offset + 6], values[offset + 7],
                values[offset + 8], values[offset + 9], values[offset + 10], values[offset + 11],
                values[offset + 12], values[offset + 13], values[offset + 14],
                values[offset + 15]);
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnbModelData is closed");
            }
        }
        return handle;
    }
}
