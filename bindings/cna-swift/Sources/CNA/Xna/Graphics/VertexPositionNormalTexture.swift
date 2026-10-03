// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.VertexPositionNormalTexture` projection.
    ///
    /// `public sequential ansi serializable sealed beforefieldinit`, extending
    /// `System.ValueType` and implementing `IVertexType`. See
    /// `VertexPositionColor` for the reasoning every one of these four shares;
    /// what differs here is the field list, the element table its class
    /// constructor builds, and the word count `SmartGetHashCode` folds.
    public struct VertexPositionNormalTexture: IVertexType {
        public var Position: Microsoft.Xna.Framework.Vector3
        public var Normal: Microsoft.Xna.Framework.Vector3
        public var TextureCoordinate: Microsoft.Xna.Framework.Vector2

        /// `VertexPositionNormalTexture..ctor(Vector3, Vector3, Vector2)` — plain field
        /// assignments and nothing else.
        public init(
            _ position: Microsoft.Xna.Framework.Vector3,
            _ normal: Microsoft.Xna.Framework.Vector3,
            _ textureCoordinate: Microsoft.Xna.Framework.Vector2
        ) {
            Position = position
            Normal = normal
            TextureCoordinate = textureCoordinate
        }

        /// `public static initonly VertexDeclaration VertexDeclaration`, built
        /// by the class constructor from
        /// `(0, Vector3, Position, 0)`,
        /// `(12, Vector3, Normal, 0)`,
        /// `(24, Vector2, TextureCoordinate, 0)`
        /// through the **one-argument** `VertexDeclaration` constructor, so
        /// the stride is computed rather than supplied. A test asserts it
        /// comes out 32.
        public static let VertexDeclaration = try! Microsoft.Xna.Framework.Graphics
            .VertexDeclaration(elements: [
                VertexElement(0, .Vector3, .Position, 0),
                VertexElement(12, .Vector3, .Normal, 0),
                VertexElement(24, .Vector2, .TextureCoordinate, 0),
            ])
            .named("VertexPositionNormalTexture.VertexDeclaration")

        /// `IVertexType.get_VertexDeclaration`, an explicit interface
        /// implementation that returns the static field.
        public var VertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration {
            Self.VertexDeclaration
        }

        /// `Helpers.SmartGetHashCode` over 32 bytes, i.e. 8 `int32`
        /// words, substituting `0x7FFFFFFF` for a zero XOR.
        public func GetHashCode() -> Int32 {
            let hash = Position.X.bitPattern
                ^ Position.Y.bitPattern
                ^ Position.Z.bitPattern
                ^ Normal.X.bitPattern
                ^ Normal.Y.bitPattern
                ^ Normal.Z.bitPattern
                ^ TextureCoordinate.X.bitPattern
                ^ TextureCoordinate.Y.bitPattern
            return hash == 0 ? Int32.max : Int32(bitPattern: hash)
        }

        /// `String.Format(CurrentCulture, "{{Position:{0} Normal:{1} TextureCoordinate:{2}}}", …)`.
        public func ToString() -> String {
            "{Position:\(Position.ToString()) Normal:\(Normal.ToString()) TextureCoordinate:\(TextureCoordinate.ToString())}"
        }

        /// `op_Equality`, field by field through each field's own
        /// `op_Equality` and never bitwise.
        public static func == (lhs: VertexPositionNormalTexture, rhs: VertexPositionNormalTexture) -> Bool {
            lhs.Position == rhs.Position
                && lhs.Normal == rhs.Normal
                && lhs.TextureCoordinate == rhs.TextureCoordinate
        }

        public static func != (lhs: VertexPositionNormalTexture, rhs: VertexPositionNormalTexture) -> Bool {
            !(lhs == rhs)
        }

        /// `Equals(object)`: false for null, false for a different boxed
        /// runtime type, otherwise `op_Equality`.
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? VertexPositionNormalTexture else { return false }
            return self == other
        }
    }
}
