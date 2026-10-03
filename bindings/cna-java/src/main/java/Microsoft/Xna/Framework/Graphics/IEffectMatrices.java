package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Matrix;

/** Matrix contract shared by XNA stock effects. */
public interface IEffectMatrices {
    Matrix getWorld();
    void setWorld(Matrix value);
    Matrix getView();
    void setView(Matrix value);
    Matrix getProjection();
    void setProjection(Matrix value);
}
