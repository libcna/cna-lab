package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** XNA DualTextureEffect backed by CNA's executable stock-effect implementation. */
@SuppressWarnings("this-escape")
public class DualTextureEffect extends Effect implements IEffectFog, IEffectMatrices {

    private Texture2D texture;
    private Texture2D texture2;

    public DualTextureEffect(GraphicsDevice device) {
        super(Objects.requireNonNull(device, "device"), "DualTextureEffect");
    }

    protected DualTextureEffect(DualTextureEffect cloneSource) {
        super(Objects.requireNonNull(cloneSource, "cloneSource"));
        texture = cloneSource.texture;
        texture2 = cloneSource.texture2;
    }

    @Override
    public Effect Clone() {
        requireEffectAlive();
        return new DualTextureEffect(this);
    }

    @Override
    protected void OnApply() {
        super.OnApply();
    }

    public final float getAlpha() { return NativeBindings.getStockEffectFloat(this, 1, 0); }
    public final void setAlpha(float value) { NativeBindings.setStockEffectFloat(this, 1, 0, value); }

    public final Vector3 getDiffuseColor() { return NativeBindings.getStockEffectVector(this, 1, 0); }
    public final void setDiffuseColor(Vector3 value) {
        NativeBindings.setStockEffectVector(this, 1, 0, value);
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
    public final Matrix getProjection() { return NativeBindings.getBasicEffectMatrix(this, 2); }
    @Override
    public final void setProjection(Matrix value) { NativeBindings.setBasicEffectMatrix(this, 2, value); }

    public final Texture2D getTexture() {
        requireEffectAlive();
        return texture;
    }
    public final void setTexture(Texture2D value) {
        requireEffectAlive();
        NativeBindings.setStockEffectTexture(this, 1, 0, value);
        texture = value;
    }

    public final Texture2D getTexture2() {
        requireEffectAlive();
        return texture2;
    }
    public final void setTexture2(Texture2D value) {
        requireEffectAlive();
        NativeBindings.setStockEffectTexture(this, 1, 1, value);
        texture2 = value;
    }

    public final boolean getVertexColorEnabled() {
        return NativeBindings.getStockEffectBoolean(this, 1, 0);
    }
    public final void setVertexColorEnabled(boolean value) {
        NativeBindings.setStockEffectBoolean(this, 1, 0, value);
    }

    @Override
    public final Matrix getView() { return NativeBindings.getBasicEffectMatrix(this, 1); }
    @Override
    public final void setView(Matrix value) { NativeBindings.setBasicEffectMatrix(this, 1, value); }
    @Override
    public final Matrix getWorld() { return NativeBindings.getBasicEffectMatrix(this, 0); }
    @Override
    public final void setWorld(Matrix value) { NativeBindings.setBasicEffectMatrix(this, 0, value); }
}
