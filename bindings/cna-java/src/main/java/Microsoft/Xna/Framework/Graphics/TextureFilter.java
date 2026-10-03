package Microsoft.Xna.Framework.Graphics;

/** Texture minification, magnification, and mip selection mode. */
public enum TextureFilter {
    Linear,
    Point,
    Anisotropic,
    LinearMipPoint,
    PointMipLinear,
    MinLinearMagPointMipLinear,
    MinLinearMagPointMipPoint,
    MinPointMagLinearMipLinear,
    MinPointMagLinearMipPoint
}
