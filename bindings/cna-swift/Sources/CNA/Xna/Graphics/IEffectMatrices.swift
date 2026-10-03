// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    // The pinned metadata declares a public abstract interface with no base
    // interface and exactly three read/write Matrix properties, all abstract.
    public protocol IEffectMatrices {
        var World: Microsoft.Xna.Framework.Matrix { get set }

        var View: Microsoft.Xna.Framework.Matrix { get set }

        var Projection: Microsoft.Xna.Framework.Matrix { get set }
    }
}
