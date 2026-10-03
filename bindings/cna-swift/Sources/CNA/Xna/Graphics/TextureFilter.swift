// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    public enum TextureFilter: Int32 {
        case Linear = 0
        case Point = 1
        case Anisotropic = 2
        case LinearMipPoint = 3
        case PointMipLinear = 4
        case MinLinearMagPointMipLinear = 5
        case MinLinearMagPointMipPoint = 6
        case MinPointMagLinearMipLinear = 7
        case MinPointMagLinearMipPoint = 8
    }
}
