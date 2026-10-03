package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Content.ContentLoadException;
import Microsoft.Xna.Framework.Content.ContentReader;

import java.util.Objects;

/** Identity-stable managed XNA Model graph over existing graphics resources. */
public final class Model {

    private final ModelBone root;
    private final ModelBoneCollection bones;
    private final ModelMeshCollection meshes;
    private Object tag;

    Model(ModelBone[] bones, ModelMesh[] meshes, ModelBone root, Object tag) {
        this.bones = new ModelBoneCollection(Objects.requireNonNull(bones, "bones"));
        this.meshes = new ModelMeshCollection(Objects.requireNonNull(meshes, "meshes"));
        this.root = root;
        this.tag = tag;
    }

    static Model read(ContentReader input) {
        Objects.requireNonNull(input, "input");
        int boneCount = requireContentCount(input.ReadInt32(), "bone");
        ModelBone[] boneValues = new ModelBone[boneCount];
        for (int index = 0; index < boneCount; index++) {
            String name = input.ReadObject(String.class);
            boneValues[index] = new ModelBone(name, input.ReadMatrix(), index);
        }
        for (ModelBone bone : boneValues) {
            ModelBone parent = readBoneReference(input, boneValues);
            int childCount = requireContentCount(input.ReadInt32(), "bone child");
            ModelBone[] children = new ModelBone[childCount];
            for (int index = 0; index < childCount; index++) {
                children[index] = readBoneReference(input, boneValues);
                if (children[index] == null) {
                    throw new ContentLoadException("Model child bone reference is null");
                }
            }
            bone.setParentAndChildren(parent, children);
        }

        int meshCount = requireContentCount(input.ReadInt32(), "mesh");
        ModelMesh[] meshValues = new ModelMesh[meshCount];
        for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
            String name = input.ReadObject(String.class);
            ModelBone parent = readBoneReference(input, boneValues);
            BoundingSphere bounds = new BoundingSphere(
                    new Vector3(input.ReadVector3()), input.ReadSingle());
            Object tag = input.ReadObject(Object.class);
            int partCount = requireContentCount(input.ReadInt32(), "mesh part");
            ModelMeshPart[] parts = new ModelMeshPart[partCount];
            for (int partIndex = 0; partIndex < partCount; partIndex++) {
                ModelMeshPart part = new ModelMeshPart(
                        input.ReadInt32(), input.ReadInt32(),
                        input.ReadInt32(), input.ReadInt32(),
                        input.ReadObject(Object.class));
                parts[partIndex] = part;
                input.ReadSharedResource(VertexBuffer.class,
                        value -> part.setBuffers(value, part.getIndexBuffer()));
                input.ReadSharedResource(IndexBuffer.class,
                        value -> part.setBuffers(part.getVertexBuffer(), value));
                input.ReadSharedResource(Effect.class, part::setEffect);
            }
            meshValues[meshIndex] = new ModelMesh(name, parent, bounds, parts, tag);
        }
        ModelBone root = readBoneReference(input, boneValues);
        Object tag = input.ReadObject(Object.class);
        return new Model(boneValues, meshValues, root, tag);
    }

    public void CopyBoneTransformsTo(Matrix[] destinationBoneTransforms) {
        Objects.requireNonNull(destinationBoneTransforms, "destinationBoneTransforms");
        requireCapacity(destinationBoneTransforms.length, "destinationBoneTransforms");
        for (int index = 0; index < bones.size(); index++) {
            destinationBoneTransforms[index] = bones.get(index).getTransform();
        }
    }

    public void CopyAbsoluteBoneTransformsTo(Matrix[] destinationBoneTransforms) {
        Objects.requireNonNull(destinationBoneTransforms, "destinationBoneTransforms");
        requireCapacity(destinationBoneTransforms.length, "destinationBoneTransforms");
        for (int index = 0; index < bones.size(); index++) {
            ModelBone bone = bones.get(index);
            destinationBoneTransforms[index] = bone.getParent() == null
                    ? bone.getTransform()
                    : Matrix.Multiply(
                            bone.getTransform(),
                            destinationBoneTransforms[bone.getParent().getIndex()]);
        }
    }

    public void CopyBoneTransformsFrom(Matrix[] sourceBoneTransforms) {
        Objects.requireNonNull(sourceBoneTransforms, "sourceBoneTransforms");
        requireCapacity(sourceBoneTransforms.length, "sourceBoneTransforms");
        for (int index = 0; index < bones.size(); index++) {
            bones.get(index).setTransform(Objects.requireNonNull(
                    sourceBoneTransforms[index], "sourceBoneTransforms[" + index + "]"));
        }
    }

    public void Draw(Matrix world, Matrix view, Matrix projection) {
        Objects.requireNonNull(world, "world");
        Objects.requireNonNull(view, "view");
        Objects.requireNonNull(projection, "projection");
        Matrix[] absolute = new Matrix[bones.size()];
        CopyAbsoluteBoneTransformsTo(absolute);
        for (ModelMesh mesh : meshes) {
            ModelBone parent = mesh.getParentBone();
            for (Effect effect : mesh.getEffects()) {
                if (!(effect instanceof IEffectMatrices matrices)) {
                    throw new IllegalStateException("Model Effect does not implement IEffectMatrices");
                }
                matrices.setWorld(Matrix.Multiply(absolute[parent.getIndex()], world));
                matrices.setView(view);
                matrices.setProjection(projection);
            }
            mesh.Draw();
        }
    }

    public ModelBone getRoot() {
        return root;
    }

    public ModelBoneCollection getBones() {
        return bones;
    }

    public Object getTag() {
        return tag;
    }

    public void setTag(Object value) {
        tag = value;
    }

    public ModelMeshCollection getMeshes() {
        return meshes;
    }

    private void requireCapacity(int capacity, String parameter) {
        if (capacity < bones.size()) {
            throw new IllegalArgumentException(
                    parameter + " must contain at least " + bones.size() + " matrices");
        }
    }

    private static ModelBone readBoneReference(ContentReader input, ModelBone[] bones) {
        int encoded = bones.length + 1 <= 255 ? input.ReadByte() : input.ReadInt32();
        if (encoded == 0) {
            return null;
        }
        if (encoded < 1 || encoded > bones.length) {
            throw new ContentLoadException("Model bone reference is outside the bone table");
        }
        return bones[encoded - 1];
    }

    private static int requireContentCount(int count, String name) {
        if (count < 0 || count > 1_000_000) {
            throw new ContentLoadException("Invalid Model " + name + " count " + count);
        }
        return count;
    }
}
