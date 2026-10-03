// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.VertexPositionColorTexture` projection.
    ///
    /// `public sequential ansi serializable sealed beforefieldinit`, extending
    /// `System.ValueType` and implementing `IVertexType`. See
    /// `VertexPositionColor` for the reasoning every one of these four shares;
    /// what differs here is the field list, the element table its class
    /// constructor builds, and the word count `SmartGetHashCode` folds.
    public struct VertexPositionColorTexture: IVertexType {
        public var Position: Microsoft.Xna.Framework.Vector3
        public var Color: Microsoft.Xna.Framework.Color
        public var TextureCoordinate: Microsoft.Xna.Framework.Vector2

        /// `VertexPositionColorTexture..ctor(Vector3, Color, Vector2)` — plain field
        /// assignments and nothing else.
        public init(
            _ position: Microsoft.Xna.Framework.Vector3,
            _ color: Microsoft.Xna.Framework.Color,
            _ textureCoordinate: Microsoft.Xna.Framework.Vector2
        ) {
            Position = position
            Color = color
            TextureCoordinate = textureCoordinate
        }

        /// `public static initonly VertexDeclaration VertexDeclaration`, built
        /// by the class constructor from
        /// `(0, Vector3, Position, 0)`,
        /// `(12, Color, Color, 0)`,
        /// `(16, Vector2, TextureCoordinate, 0)`
        /// through the **one-argument** `VertexDeclaration` constructor, so
        /// the stride is computed rather than supplied. A test asserts it
        /// comes out 24.
        public static let VertexDeclaration = try! Microsoft.Xna.Framework.Graphics
            .VertexDeclaration(elements: [
                VertexElement(0, .Vector3, .Position, 0),
                VertexElement(12, .Color, .Color, 0),
                VertexElement(16, .Vector2, .TextureCoordinate, 0),
            ])
            .named("VertexPositionColorTexture.VertexDeclaration")

        /// `IVertexType.get_VertexDeclaration`, an explicit interface
        /// implementation that returns the static field.
        public var VertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration {
            Self.VertexDeclaration
        }

        /// `Helpers.SmartGetHashCode` over 24 bytes, i.e. 6 `int32`
        /// words, substituting `0x7FFFFFFF` for a zero XOR.
        public func GetHashCode() -> Int32 {
            let hash = Position.X.bitPattern
                ^ Position.Y.bitPattern
                ^ Position.Z.bitPattern
                ^ Color.PackedValue
                ^ TextureCoordinate.X.bitPattern
                ^ TextureCoordinate.Y.bitPattern
            return hash == 0 ? Int32.max : Int32(bitPattern: hash)
        }

        /// `String.Format(CurrentCulture, "{{Position:{0} Color:{1} TextureCoordinate:{2}}}", …)`.
        public func ToString() -> String {
            "{Position:\(Position.ToString()) Color:\(Color.ToString()) TextureCoordinate:\(TextureCoordinate.ToString())}"
        }

        /// `op_Equality`, field by field through each field's own
        /// `op_Equality` and never bitwise.
        public static func == (lhs: VertexPositionColorTexture, rhs: VertexPositionColorTexture) -> Bool {
            lhs.Position == rhs.Position
                && lhs.Color == rhs.Color
                && lhs.TextureCoordinate == rhs.TextureCoordinate
        }

        public static func != (lhs: VertexPositionColorTexture, rhs: VertexPositionColorTexture) -> Bool {
            !(lhs == rhs)
        }

        /// `Equals(object)`: false for null, false for a different boxed
        /// runtime type, otherwise `op_Equality`.
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? VertexPositionColorTexture else { return false }
            return self == other
        }
    }
}
