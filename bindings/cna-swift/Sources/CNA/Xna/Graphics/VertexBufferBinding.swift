// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.VertexBufferBinding` projection.
    ///
    /// A `sealed` CLR value type over three fields: the buffer, the offset in
    /// vertices, and the instance frequency. Every one of its four members is
    /// pure managed — the three constructors validate and assign, the
    /// conversion operator forwards to one of them, and the three getters read
    /// a field — so nothing here reaches CNA. `cna_vertex_buffer_binding_init`
    /// exists and is deliberately **unbound**: it fills a native structure that
    /// only `GraphicsDevice.SetVertexBuffers` will need, and a route with no
    /// consuming member is not a route this binding has.
    public struct VertexBufferBinding {
        // `_vertexBuffer`, `_vertexOffset`, `_instanceFrequency`.
        private let storedVertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer
        private let storedVertexOffset: Int32
        private let storedInstanceFrequency: Int32

        /// `VertexBufferBinding(VertexBuffer vertexBuffer, Int32 vertexOffset,
        /// Int32 instanceFrequency)`.
        ///
        /// ```text
        /// if (vertexBuffer == null)
        ///     throw new ArgumentNullException("vertexBuffer", NullNotAllowed);
        /// if (vertexOffset < 0 || (uint)vertexOffset >= vertexBuffer._vertexCount)
        ///     throw new ArgumentOutOfRangeException("vertexOffset");
        /// if (instanceFrequency < 0)
        ///     throw new ArgumentOutOfRangeException("instanceFrequency");
        /// ```
        ///
        /// Two things about that order are worth stating. The offset is checked
        /// **before** the frequency, so a binding that is wrong in both blames
        /// the offset. And the comparison against the vertex count is
        /// `bge.un` — **unsigned** — which is why a negative offset is caught
        /// by the first half of the test rather than by wrapping into a huge
        /// value; both halves are written here because both are in the IL.
        ///
        /// Both `ArgumentOutOfRangeException`s use the one-argument overload,
        /// which takes a **paramName** and substitutes
        /// `Arg_ArgumentOutOfRangeException` as the message.
        ///
        /// The null check is unreachable through a non-Optional Swift class
        /// parameter and is recorded rather than written.
        /// Every external label is `_`: a CLR **value type**'s constructor
        /// parameters carry no Swift labels under the pinned mapping rule, the
        /// same rule `Rectangle` and `Viewport` already follow.
        public init(
            _ vertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer,
            _ vertexOffset: Int32,
            _ instanceFrequency: Int32
        ) throws {
            guard vertexOffset >= 0,
                  UInt32(bitPattern: vertexOffset)
                    < UInt32(bitPattern: vertexBuffer.VertexCount) else {
                throw CNAArgumentOutOfRangeException(paramName: "vertexOffset")
            }
            guard instanceFrequency >= 0 else {
                throw CNAArgumentOutOfRangeException(paramName: "instanceFrequency")
            }
            storedVertexBuffer = vertexBuffer
            storedVertexOffset = vertexOffset
            storedInstanceFrequency = instanceFrequency
        }

        /// `VertexBufferBinding(VertexBuffer vertexBuffer, Int32 vertexOffset)`.
        ///
        /// The same body with `_instanceFrequency` assigned `ldc.i4.0`, and
        /// **not** a forward to the three-argument constructor: the frequency
        /// check is absent from this one because the value it would check is a
        /// literal zero.
        public init(
            _ vertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer,
            _ vertexOffset: Int32
        ) throws {
            try self.init(vertexBuffer, vertexOffset, 0)
        }

        /// `VertexBufferBinding(VertexBuffer vertexBuffer)`.
        public init(_ vertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer) throws {
            try self.init(vertexBuffer, 0, 0)
        }

        /// `VertexBufferBinding.VertexBuffer`.
        public var VertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer {
            storedVertexBuffer
        }

        /// `VertexBufferBinding.VertexOffset`.
        public var VertexOffset: Int32 { storedVertexOffset }

        /// `VertexBufferBinding.InstanceFrequency`.
        public var InstanceFrequency: Int32 { storedInstanceFrequency }

        /// `op_Implicit(VertexBuffer) : VertexBufferBinding`.
        ///
        /// The whole body is `newobj VertexBufferBinding::.ctor(VertexBuffer)`,
        /// so it throws exactly where that constructor does. A CLR implicit
        /// conversion has no Swift counterpart that can throw, so it is the
        /// named static the operator mapping already uses elsewhere.
        public static func op_Implicit(
            _ vertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer
        ) throws -> Microsoft.Xna.Framework.Graphics.VertexBufferBinding {
            try VertexBufferBinding(vertexBuffer)
        }
    }
}
