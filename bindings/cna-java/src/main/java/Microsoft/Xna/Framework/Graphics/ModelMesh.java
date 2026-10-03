package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.BoundingSphere;

import java.util.Objects;

/** One identity-stable mesh in an XNA Model graph. */
public final class ModelMesh {

    private final String name;
    private final ModelBone parentBone;
    private final BoundingSphere boundingSphere;
    private final ModelMeshPartCollection meshParts;
    private final ModelEffectCollection effects = new ModelEffectCollection();
    private Object tag;

    ModelMesh(
            String name,
            ModelBone parentBone,
            BoundingSphere boundingSphere,
            ModelMeshPart[] meshParts,
            Object tag) {
        this.name = name;
        this.parentBone = parentBone;
        this.boundingSphere = new BoundingSphere(
                Objects.requireNonNull(boundingSphere, "boundingSphere"));
        this.meshParts = new ModelMeshPartCollection(meshParts);
        this.tag = tag;
        for (ModelMeshPart part : this.meshParts) {
            Effect initialEffect = part.getEffect();
            part.attach(this);
            if (initialEffect != null && !effects.contains(initialEffect)) {
                effects.addEffect(initialEffect);
            }
        }
    }

    public String getName() {
        return name;
    }

    public ModelBone getParentBone() {
        return parentBone;
    }

    public BoundingSphere getBoundingSphere() {
        return new BoundingSphere(boundingSphere);
    }

    public Object getTag() {
        return tag;
    }

    public void setTag(Object value) {
        tag = value;
    }

    public ModelMeshPartCollection getMeshParts() {
        return meshParts;
    }

    public ModelEffectCollection getEffects() {
        return effects;
    }

    public void Draw() {
        for (ModelMeshPart part : meshParts) {
            Effect effect = part.getEffect();
            if (effect == null) {
                throw new IllegalStateException("ModelMeshPart has no Effect");
            }
            EffectPassCollection passes = effect.getCurrentTechnique().getPasses();
            for (int index = 0; index < passes.getCount(); index++) {
                passes.get(index).Apply();
                part.draw();
            }
        }
    }
}
