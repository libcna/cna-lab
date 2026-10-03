// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.VertexPositionColor` projection.
    ///
    /// `public sequential ansi serializable sealed beforefieldinit`, extending
    /// `System.ValueType` and implementing `IVertexType`: a Swift `struct`,
    /// with two public fields and one `public static initonly` declaration.
    ///
    /// `initonly` is why the static declaration is a `let` and not a `var`,
    /// which the contract could not say until Foundation 41 recorded the
    /// attribute.
    public struct VertexPositionColor: IVertexType {
        public var Position: Microsoft.Xna.Framework.Vector3
        public var Color: Microsoft.Xna.Framework.Color

        /// `VertexPositionColor..ctor(Vector3 position, Color color)` — two
        /// field assignments and nothing else.
        public init(
            _ position: Microsoft.Xna.Framework.Vector3,
            _ color: Microsoft.Xna.Framework.Color
        ) {
            Position = position
            Color = color
        }

        /// `public static initonly VertexDeclaration VertexDeclaration`.
        ///
        /// The class constructor builds two elements — `(0, Vector3, Position,
        /// 0)` and `(12, Color, Color, 0)` — hands them to the **one-argument**
        /// `VertexDeclaration` constructor, so the stride is computed rather
        /// than supplied, and names the result.
        ///
        /// That constructor throws, and a CLR class constructor cannot; XNA's
        /// layout is valid, so the failure is unreachable and
        /// `try!` states that rather than hiding it behind a fallback
        /// declaration that XNA would never produce. A test asserts the
        /// computed stride is 16, which is the same fact from the outside.
        public static let VertexDeclaration = try! Microsoft.Xna.Framework.Graphics
            .VertexDeclaration(elements: [
                VertexElement(0, .Vector3, .Position, 0),
                VertexElement(12, .Color, .Color, 0),
            ])
            .named("VertexPositionColor.VertexDeclaration")

        /// `IVertexType.get_VertexDeclaration`, an explicit interface
        /// implementation that returns the static field.
        public var VertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration {
            Self.VertexDeclaration
        }

        /// `GetHashCode` boxes the value and calls
        /// `Helpers.SmartGetHashCode`, which XORs `Marshal.SizeOf(obj) / 4`
        /// consecutive `int32` words and substitutes `0x7FFFFFFF` when the XOR
        /// is zero. This sequential layout is `Vector3` (12) + `Color` (4) =
        /// 16 bytes, exactly four words, and XOR is order independent.
        public func GetHashCode() -> Int32 {
            let hash = Position.X.bitPattern
                ^ Position.Y.bitPattern
                ^ Position.Z.bitPattern
                ^ Color.PackedValue
            return hash == 0 ? Int32.max : Int32(bitPattern: hash)
        }

        /// `String.Format(CurrentCulture, "{{Position:{0} Color:{1}}}", …)`.
        /// The doubled braces are composite-format escapes, so one brace pair
        /// is emitted.
        public func ToString() -> String {
            "{Position:\(Position.ToString()) Color:\(Color.ToString())}"
        }

        /// `op_Equality` compares `Color` first and `Position` second — the
        /// IL's own order, which decides only what short-circuits. Each field
        /// compares through its own `op_Equality`, never bitwise, so a NaN
        /// component makes a vertex unequal to itself exactly as it does in
        /// the CLR while `GetHashCode`, which *is* bitwise, still matches.
        public static func == (
            lhs: VertexPositionColor, rhs: VertexPositionColor
        ) -> Bool {
            lhs.Color == rhs.Color && lhs.Position == rhs.Position
        }

        public static func != (
            lhs: VertexPositionColor, rhs: VertexPositionColor
        ) -> Bool {
            !(lhs == rhs)
        }

        /// `Equals(object)` answers false for null, false when the boxed
        /// runtime types differ, and otherwise defers to `op_Equality`.
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? VertexPositionColor else { return false }
            return self == other
        }
    }
}

extension Microsoft.Xna.Framework.Graphics.VertexDeclaration {
    /// Names a declaration and hands it back, so a `static let` can reproduce
    /// the class constructor's `decl.Name = "…"; field = decl` in one
    /// expression. `Name` is `GraphicsResource`'s own settable property; this
    /// adds no XNA member and is internal.
    internal func named(_ name: String) -> Self {
        Name = name
        return self
    }
}
