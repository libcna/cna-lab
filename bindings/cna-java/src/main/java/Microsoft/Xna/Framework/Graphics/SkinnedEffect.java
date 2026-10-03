package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** XNA SkinnedEffect backed by CNA's executable stock-effect implementation. */
@SuppressWarnings("this-escape")
public class SkinnedEffect extends Effect implements IEffectFog, IEffectLights, IEffectMatrices {

    public static final int MaxBones = 72;

    private final DirectionalLight[] directionalLights = new DirectionalLight[3];
    private Texture2D texture;

    public SkinnedEffect(GraphicsDevice device) {
        super(Objects.requireNonNull(device, "device"), "SkinnedEffect");
    }

    protected SkinnedEffect(SkinnedEffect cloneSource) {
        super(Objects.requireNonNull(cloneSource, "cloneSource"));
        texture = cloneSource.texture;
    }

    @Override
    public Effect Clone() {
        requireEffectAlive();
        return new SkinnedEffect(this);
    }

    @Override
    public final void EnableDefaultLighting() {
        requireEffectAlive();
        NativeBindings.enableDefaultLighting(this);
    }

    @Override
    protected void OnApply() {
        super.OnApply();
    }

    public final Matrix[] GetBoneTransforms(int count) {
        if (count < 1 || count > MaxBones) {
            throw new IllegalArgumentException("count must be between 1 and " + MaxBones);
        }
        return NativeBindings.getSkinnedEffectBoneTransforms(this, count);
    }

    public final void SetBoneTransforms(Matrix[] boneTransforms) {
        Objects.requireNonNull(boneTransforms, "boneTransforms");
        if (boneTransforms.length < 1 || boneTransforms.length > MaxBones) {
            throw new IllegalArgumentException(
                    "boneTransforms length must be between 1 and " + MaxBones);
        }
        NativeBindings.setSkinnedEffectBoneTransforms(this, boneTransforms);
    }

    public final float getAlpha() { return NativeBindings.getStockEffectFloat(this, 3, 1); }
    public final void setAlpha(float value) { NativeBindings.setStockEffectFloat(this, 3, 1, value); }

    @Override
    public final Vector3 getAmbientLightColor() { return NativeBindings.getBasicEffectVector(this, 3); }
    @Override
    public final void setAmbientLightColor(Vector3 value) {
        NativeBindings.setBasicEffectVector(this, 3, value);
    }

    public final Vector3 getDiffuseColor() { return NativeBindings.getStockEffectVector(this, 3, 0); }
    public final void setDiffuseColor(Vector3 value) {
        NativeBindings.setStockEffectVector(this, 3, 0, value);
    }

    @Override
    public final DirectionalLight getDirectionalLight0() { return directionalLight(0); }
    @Override
    public final DirectionalLight getDirectionalLight1() { return directionalLight(1); }
    @Override
    public final DirectionalLight getDirectionalLight2() { return directionalLight(2); }

    public final Vector3 getEmissiveColor() { return NativeBindings.getStockEffectVector(this, 3, 1); }
    public final void setEmissiveColor(Vector3 value) {
        NativeBindings.setStockEffectVector(this, 3, 1, value);
    }

    @Override
    public final Vector3 getFogColor() { return NativeBindings.getBasicEffectVector(this, 4); }
    @Override
    public final void setFogColor(Vector3 value) { NativeBindings.setBasicEffectVector(this, 4, value); }
    @Override
    public final boolean getFogEnabled() { return NativeBindings.getBasicEffectBoolean(this, 4); }
    @Override
    public final void setFogEnabled(boolean value) { NativeBindings.setBasicEffectBoolean(this, 4, value); }
    @Override
    public final float getFogEnd() { return NativeBindings.getBasicEffectFloat(this, 3); }
    @Override
    public final void setFogEnd(float value) { NativeBindings.setBasicEffectFloat(this, 3, value); }
    @Override
    public final float getFogStart() { return NativeBindings.getBasicEffectFloat(this, 2); }
    @Override
    public final void setFogStart(float value) { NativeBindings.setBasicEffectFloat(this, 2, value); }

    @Override
    public final boolean getLightingEnabled() { return NativeBindings.getBasicEffectBoolean(this, 3); }
    @Override
    public final void setLightingEnabled(boolean value) {
        if (!value) {
            throw new UnsupportedOperationException("SkinnedEffect does not support disabling lighting");
        }
        NativeBindings.setBasicEffectBoolean(this, 3, true);
    }

    public final boolean getPreferPerPixelLighting() {
        return NativeBindings.getStockEffectBoolean(this, 3, 0);
    }
    public final void setPreferPerPixelLighting(boolean value) {
        NativeBindings.setStockEffectBoolean(this, 3, 0, value);
    }

    @Override
    public final Matrix getProjection() { return NativeBindings.getBasicEffectMatrix(this, 2); }
    @Override
    public final void setProjection(Matrix value) { NativeBindings.setBasicEffectMatrix(this, 2, value); }

    public final Vector3 getSpecularColor() { return NativeBindings.getStockEffectVector(this, 3, 2); }
    public final void setSpecularColor(Vector3 value) {
        NativeBindings.setStockEffectVector(this, 3, 2, value);
    }
    public final float getSpecularPower() { return NativeBindings.getStockEffectFloat(this, 3, 0); }
    public final void setSpecularPower(float value) {
        NativeBindings.setStockEffectFloat(this, 3, 0, value);
    }

    public final Texture2D getTexture() {
        requireEffectAlive();
        return texture;
    }
    public final void setTexture(Texture2D value) {
        requireEffectAlive();
        NativeBindings.setStockEffectTexture(this, 3, 0, value);
        texture = value;
    }

    @Override
    public final Matrix getView() { return NativeBindings.getBasicEffectMatrix(this, 1); }
    @Override
    public final void setView(Matrix value) { NativeBindings.setBasicEffectMatrix(this, 1, value); }

    public final int getWeightsPerVertex() { return NativeBindings.getStockEffectInt(this, 3, 0); }
    public final void setWeightsPerVertex(int value) {
        if (value != 1 && value != 2 && value != 4) {
            throw new IllegalArgumentException("WeightsPerVertex must be 1, 2, or 4");
        }
        NativeBindings.setStockEffectInt(this, 3, 0, value);
    }

    @Override
    public final Matrix getWorld() { return NativeBindings.getBasicEffectMatrix(this, 0); }
    @Override
    public final void setWorld(Matrix value) { NativeBindings.setBasicEffectMatrix(this, 0, value); }

    private DirectionalLight directionalLight(int index) {
        requireEffectAlive();
        DirectionalLight result = directionalLights[index];
        if (result == null) {
            result = new DirectionalLight(
                    this, NativeBindings.getBasicEffectDirectionalLightHandle(this, index));
            directionalLights[index] = result;
        }
        return result;
    }
}
