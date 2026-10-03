package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Matrix;

import java.util.Objects;

/** One identity-stable node in a loaded XNA Model bone hierarchy. */
public final class ModelBone {

    private final String name;
    private final int index;
    private Matrix transform;
    private ModelBone parent;
    private ModelBoneCollection children;

    ModelBone(String name, Matrix transform, int index) {
        this.name = name;
        this.transform = new Matrix(Objects.requireNonNull(transform, "transform"));
        this.index = index;
        children = new ModelBoneCollection(new ModelBone[0]);
    }

    public String getName() {
        return name;
    }

    public int getIndex() {
        return index;
    }

    public Matrix getTransform() {
        return new Matrix(transform);
    }

    public void setTransform(Matrix value) {
        transform = new Matrix(Objects.requireNonNull(value, "value"));
    }

    public ModelBone getParent() {
        return parent;
    }

    public ModelBoneCollection getChildren() {
        return children;
    }

    void setParentAndChildren(ModelBone newParent, ModelBone[] newChildren) {
        parent = newParent;
        children = new ModelBoneCollection(
                Objects.requireNonNull(newChildren, "newChildren"));
    }
}
