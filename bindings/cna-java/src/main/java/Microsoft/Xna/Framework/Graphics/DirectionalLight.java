package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Stable directional-light view owned by a stock effect. */
public final class DirectionalLight {

    private final Effect owner;
    private final EffectParameter directionParameter;
    private final EffectParameter diffuseColorParameter;
    private final EffectParameter specularColorParameter;
    private Vector3 cachedDirection;
    private Vector3 cachedDiffuseColor;
    private Vector3 cachedSpecularColor;
    private boolean enabled;

    public DirectionalLight(
            EffectParameter directionParameter,
            EffectParameter diffuseColorParameter,
            EffectParameter specularColorParameter,
            DirectionalLight cloneSource) {
        owner = null;
        this.directionParameter = directionParameter;
        this.diffuseColorParameter = diffuseColorParameter;
        this.specularColorParameter = specularColorParameter;
        cachedDirection = cloneSource == null
                ? Vector3.getForward() : cloneSource.getDirection();
        cachedDiffuseColor = cloneSource == null
                ? Vector3.getZero() : cloneSource.getDiffuseColor();
        cachedSpecularColor = cloneSource == null
                ? Vector3.getZero() : cloneSource.getSpecularColor();
        enabled = cloneSource != null && cloneSource.getEnabled();
        pushAll();
    }

    DirectionalLight(Effect owner, long nativeHandle) {
        this.owner = Objects.requireNonNull(owner, "owner");
        directionParameter = null;
        diffuseColorParameter = null;
        specularColorParameter = null;
        cachedDirection = null;
        cachedDiffuseColor = null;
        cachedSpecularColor = null;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 9);
    }

    public Vector3 getDiffuseColor() {
        if (owner != null) {
            requireAlive();
            return NativeBindings.getDirectionalLightVector(this, 0);
        }
        return new Vector3(cachedDiffuseColor);
    }

    public void setDiffuseColor(Vector3 value) {
        Vector3 snapshot = new Vector3(Objects.requireNonNull(value, "value"));
        if (owner != null) {
            requireAlive();
            NativeBindings.setDirectionalLightVector(this, 0, snapshot);
            return;
        }
        cachedDiffuseColor = snapshot;
        writeParameter(diffuseColorParameter, enabled ? snapshot : Vector3.getZero());
    }

    public Vector3 getDirection() {
        if (owner != null) {
            requireAlive();
            return NativeBindings.getDirectionalLightVector(this, 1);
        }
        return new Vector3(cachedDirection);
    }

    public void setDirection(Vector3 value) {
        Vector3 snapshot = new Vector3(Objects.requireNonNull(value, "value"));
        if (owner != null) {
            requireAlive();
            NativeBindings.setDirectionalLightVector(this, 1, snapshot);
            return;
        }
        cachedDirection = snapshot;
        writeParameter(directionParameter, snapshot);
    }

    public boolean getEnabled() {
        if (owner != null) {
            requireAlive();
            return NativeBindings.getDirectionalLightEnabled(this);
        }
        return enabled;
    }

    public void setEnabled(boolean value) {
        if (owner != null) {
            requireAlive();
            NativeBindings.setDirectionalLightEnabled(this, value);
            return;
        }
        enabled = value;
        writeParameter(diffuseColorParameter, value ? cachedDiffuseColor : Vector3.getZero());
        writeParameter(specularColorParameter, value ? cachedSpecularColor : Vector3.getZero());
    }

    public Vector3 getSpecularColor() {
        if (owner != null) {
            requireAlive();
            return NativeBindings.getDirectionalLightVector(this, 2);
        }
        return new Vector3(cachedSpecularColor);
    }

    public void setSpecularColor(Vector3 value) {
        Vector3 snapshot = new Vector3(Objects.requireNonNull(value, "value"));
        if (owner != null) {
            requireAlive();
            NativeBindings.setDirectionalLightVector(this, 2, snapshot);
            return;
        }
        cachedSpecularColor = snapshot;
        writeParameter(specularColorParameter, enabled ? snapshot : Vector3.getZero());
    }

    private void requireAlive() {
        owner.requireEffectAlive();
        NativeBindings.requireEffectMember(this);
    }

    private void pushAll() {
        writeParameter(directionParameter, cachedDirection);
        writeParameter(diffuseColorParameter, enabled ? cachedDiffuseColor : Vector3.getZero());
        writeParameter(specularColorParameter, enabled ? cachedSpecularColor : Vector3.getZero());
    }

    private static void writeParameter(EffectParameter parameter, Vector3 value) {
        if (parameter != null) {
            parameter.SetValue(new Vector3(value));
        }
    }
}
