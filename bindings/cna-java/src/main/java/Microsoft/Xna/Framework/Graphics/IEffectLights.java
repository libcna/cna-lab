package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Vector3;

/** Lighting contract shared by XNA stock effects. */
public interface IEffectLights {
    DirectionalLight getDirectionalLight0();
    DirectionalLight getDirectionalLight1();
    DirectionalLight getDirectionalLight2();
    Vector3 getAmbientLightColor();
    void setAmbientLightColor(Vector3 value);
    boolean getLightingEnabled();
    void setLightingEnabled(boolean value);
    void EnableDefaultLighting();
}
