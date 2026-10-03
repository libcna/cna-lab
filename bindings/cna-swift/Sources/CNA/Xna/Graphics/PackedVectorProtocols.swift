// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics.PackedVector {
    /// Converts packed vector values to and from four-component vectors.
    public protocol IPackedVector {
        func ToVector4() -> Microsoft.Xna.Framework.Vector4

        mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4)
    }

    /// Exposes the mutable packed storage used by a packed vector value.
    public protocol IPackedVectorOfT<TPacked>: IPackedVector {
        associatedtype TPacked

        var PackedValue: TPacked { get set }
    }
}
