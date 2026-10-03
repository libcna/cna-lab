package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Vector3;

/** Fog contract shared by XNA stock effects. */
public interface IEffectFog {
    boolean getFogEnabled();
    void setFogEnabled(boolean value);
    float getFogStart();
    void setFogStart(float value);
    float getFogEnd();
    void setFogEnd(float value);
    Vector3 getFogColor();
    void setFogColor(Vector3 value);
}
