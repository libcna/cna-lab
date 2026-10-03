// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The internal `VertexElementValidator`, which is where every
    /// `VertexDeclaration` argument failure comes from.
    ///
    /// `assembly` in the metadata, so it is not a contract type and is not
    /// projected as one; it is reproduced here because the two public
    /// constructors call it and its behaviour is theirs.
    internal enum VertexElementValidator {
        /// The four `FrameworkResources` templates this validator reproduces,
        /// read out of `Microsoft.Xna.Framework.dll`'s own embedded string
        /// table and pinned in
        /// `tools/api_compat/reference/xna40-selected-resource-strings.json`.
        ///
        /// A fifth, `VertexElementBadUsage`, is deliberately absent: see
        /// `validate(vertexStride:elements:)`.
        internal static let offsetNotMultipleFour =
            "Invalid VertexDeclaration. Vertex stride and VertexElement.Offset "
            + "must be multiples of four."
        internal static let outsideStride =
            "Invalid VertexDeclaration. Element {0}{1} does not fit within the "
            + "specified vertex stride."
        internal static let elementsOverlap =
            "Invalid VertexDeclaration. Elements {0}{1} and {2}{3} are "
            + "overlapping."
        internal static let duplicateElement =
            "Invalid VertexDeclaration. Duplicate element {0}{1}."

        /// `String.Format(CurrentCulture, template, args)` over the positional
        /// placeholders these four templates use. The arguments are boxed
        /// enum and `Int32` values in the CLR, so each is formatted with its
        /// own `ToString()` — the enum's case name and the integer's digits.
        internal static func format(
            _ template: String, _ arguments: String...
        ) -> String {
            var result = template
            for (index, argument) in arguments.enumerated() {
                result = result.replacingOccurrences(
                    of: "{\(index)}", with: argument)
            }
            return result
        }
        /// `GetTypeSize(VertexElementFormat)` — a switch with a default of 0.
        ///
        /// The default is unreachable through the Swift enum, which has no
        /// case outside the twelve, but it is the CLR's answer and is written
        /// down rather than assumed away.
        internal static func typeSize(
            _ format: Microsoft.Xna.Framework.Graphics.VertexElementFormat
        ) -> Int32 {
            switch format {
            case .Single: return 4
            case .Vector2: return 8
            case .Vector3: return 12
            case .Vector4: return 16
            case .Color: return 4
            case .Byte4: return 4
            case .Short2: return 4
            case .Short4: return 8
            case .NormalizedShort2: return 4
            case .NormalizedShort4: return 8
            case .HalfVector2: return 4
            case .HalfVector4: return 8
            }
        }

        /// `GetVertexStride(VertexElement[])`.
        ///
        /// The **maximum end offset**, not a sum and not the last element's
        /// end: elements may be given in any order and may overlap, and an
        /// empty array answers zero.
        internal static func vertexStride(
            _ elements: [Microsoft.Xna.Framework.Graphics.VertexElement]
        ) -> Int32 {
            var maximum: Int32 = 0
            for element in elements {
                let end = element.Offset + typeSize(element.VertexElementFormat)
                if maximum < end { maximum = end }
            }
            return maximum
        }

        /// `Validate(int32 vertexStride, VertexElement[] elements)`.
        ///
        /// The order of the checks is observable and is reproduced exactly: an
        /// element that is both misaligned and overlapping reports the
        /// alignment message, and one that is both outside the stride and
        /// misaligned reports the stride message. Overlap is decided with a
        /// map carrying one slot per byte of the vertex, each tagged with the
        /// index of the element that owns it — which is why that message can
        /// name **both** offending elements.
        ///
        /// XNA's first per-element check, `VertexElementBadUsage`, is
        /// **unreachable here**: `VertexElementUsage` is a Swift enum with
        /// thirteen cases and no way to hold a value outside `0...12`, so the
        /// type system enforces what XNA enforces at run time. Its message is
        /// therefore not reproduced and is deliberately not pinned.
        internal static func validate(
            vertexStride: Int32,
            elements: [Microsoft.Xna.Framework.Graphics.VertexElement]
        ) throws {
            guard vertexStride > 0 else {
                throw CNAArgumentOutOfRangeException(paramName: "vertexStride")
            }
            guard vertexStride & 3 == 0 else {
                throw CNAArgumentException(
                    message: VertexElementValidator.offsetNotMultipleFour)
            }
            var owner = [Int](repeating: -1, count: Int(vertexStride))
            for index in elements.indices {
                let element = elements[index]
                let size = typeSize(element.VertexElementFormat)
                guard element.Offset >= 0,
                      element.Offset + size <= vertexStride else {
                    throw CNAArgumentException(
                        message: format(
                            outsideStride,
                            String(describing: element.VertexElementUsage),
                            String(element.UsageIndex)))
                }
                guard element.Offset & 3 == 0 else {
                    throw CNAArgumentException(
                        message: VertexElementValidator.offsetNotMultipleFour)
                }
                // Only the elements BEFORE this one are compared, so the pair
                // is always reported with the later element's own identity.
                for earlier in elements[..<index] where
                    earlier.VertexElementUsage == element.VertexElementUsage
                    && earlier.UsageIndex == element.UsageIndex {
                    throw CNAArgumentException(
                        message: format(
                            duplicateElement,
                            String(describing: element.VertexElementUsage),
                            String(element.UsageIndex)))
                }
                for byte in Int(element.Offset)..<Int(element.Offset + size) {
                    guard owner[byte] < 0 else {
                        let held = elements[owner[byte]]
                        throw CNAArgumentException(
                            message: format(
                                elementsOverlap,
                                String(describing: held.VertexElementUsage),
                                String(held.UsageIndex),
                                String(describing: element.VertexElementUsage),
                                String(element.UsageIndex)))
                    }
                    owner[byte] = index
                }
            }
        }
    }

    /// The `Microsoft.Xna.Framework.Graphics.VertexDeclaration` projection.
    ///
    /// A `GraphicsResource` with **no native object**. CNA does publish
    /// `cna_vertex_declaration_create` and its neighbours, and none of them
    /// takes a device — but nothing in this type's own public surface needs a
    /// native handle, and binding a route with no member behind it is what
    /// `docs/native-abi.md` forbids. The routes are bound when a consumer of a
    /// declaration exists.
    open class VertexDeclaration: GraphicsResource {
        // `_elements`. Optional because XNA's constructors leave the field
        // NULL for a null or empty array, which is a state `GetVertexElements`
        // then dereferences.
        private let storedElements: [VertexElement]?
        private let storedStride: Int32

        /// `VertexDeclaration..ctor(Int32 vertexStride, VertexElement[] elements)`.
        ///
        /// An **empty** array is accepted in silence and leaves the
        /// declaration with no elements and a stride of zero — the IL's two
        /// `leave` instructions, not an oversight. Otherwise the array is
        /// copied, the stride is stored, and the pair is validated.
        ///
        /// XNA's null case is not reachable through a Swift `[VertexElement]`,
        /// and the empty case reproduces it exactly: both leave by the same
        /// branch, before any field is written.
        ///
        /// XNA wraps the body in a `fault` handler that calls `Dispose(true)`
        /// when `Validate` throws. This validates after `super.init`, so a
        /// failed construction leaves a fully initialised object that Swift
        /// deinits instead. The difference is not observable: `Disposing`
        /// cannot have a subscriber, because the initializer has not returned
        /// and no reference has escaped; there is no native storage to
        /// release; and `GraphicsResource.Dispose(Boolean)` does nothing else.
        public init(
            vertexStride: Int32,
            elements: [VertexElement]
        ) throws {
            guard !elements.isEmpty else {
                storedElements = nil
                storedStride = 0
                super.init(storage: nil, device: nil)
                return
            }
            storedElements = elements
            storedStride = vertexStride
            super.init(storage: nil, device: nil)
            try VertexElementValidator.validate(
                vertexStride: vertexStride, elements: elements)
        }

        /// `VertexDeclaration..ctor(VertexElement[] elements)`.
        ///
        /// The stride is computed from the elements and then validated, so an
        /// element list whose maximum end offset is not a multiple of four is
        /// refused by the same alignment message even though the caller never
        /// supplied a stride.
        public convenience init(elements: [VertexElement]) throws {
            try self.init(
                vertexStride: VertexElementValidator.vertexStride(elements),
                elements: elements)
        }

        /// `VertexDeclaration.VertexStride`.
        ///
        /// `ldfld _vertexStride; ret` — a bare field read with no failure
        /// path, so the Swift reader does not throw.
        public var VertexStride: Int32 { storedStride }

        /// `VertexDeclaration.GetVertexElements()`.
        ///
        /// Returns a **copy**, so a caller mutating the result does not change
        /// the declaration — Swift's value-typed `Array` gives that for free
        /// where the CLR needs `Array.Clone()`.
        ///
        /// It throws where the CLR's own `callvirt` would: the whole body is
        /// `ldfld _elements; callvirt Array::Clone()`, and `_elements` is null
        /// exactly when the constructor accepted an empty array, so
        /// `NullReferenceException` is reachable. The pinned fallibility record
        /// says `IL_NO_FAILURE_PATH` because the analyser does not treat a
        /// `callvirt` on a possibly-null field as a failure path; that record
        /// is wrong and nothing depends on it, so the IL is followed instead.
        public func GetVertexElements() throws -> [VertexElement] {
            guard let storedElements else { throw CNANullReferenceException() }
            return storedElements
        }

        /// `Dispose(Boolean)`.
        ///
        /// XNA's override runs `~VertexDeclaration()` or `!VertexDeclaration()`
        /// and then `base.Dispose(disposing)`; `~VertexDeclaration()` calls
        /// `!VertexDeclaration()`, whose whole body is `Unbind()` — internal
        /// device-binding bookkeeping with no Swift counterpart. So this
        /// reduces to the base call, and that is shown rather than assumed.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }

        // ------------------------------------------------------------------
        // `VertexDeclaration.FromType(Type)`, and the native declaration a
        // buffer is created from. Both are internal: XNA's `FromType` is
        // `assembly` and CNA's declaration handle never leaves this file.

        /// One vertex type's declaration and its compiler-measured size.
        private struct RegisteredVertexType {
            let declaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration
            /// `Marshal.SizeOf(vertexType)`, measured by the Swift compiler at
            /// registration rather than asked of a metatype at run time.
            let size: Int
        }

        /// The four `IVertexType` structs this binding projects, by identity.
        ///
        /// XNA's `FromType` calls `Activator.CreateInstance(vertexType)`, casts
        /// to `IVertexType` and reads the instance property, then caches the
        /// result per type. **Swift has no `Activator`**: a metatype cannot be
        /// instantiated, and `IVertexType.VertexDeclaration` is an instance
        /// member, so there is no way to reach a consumer's own declaration
        /// from its metatype alone. Adding an `init()` requirement to the
        /// projected `IVertexType` protocol would put a member on it that XNA's
        /// interface does not have.
        ///
        /// So the four types XNA ships are registered here with the static
        /// declaration each of them returns — the same object their own
        /// `VertexDeclaration` property answers, which is the object XNA's
        /// cache would hold — and a consumer's own vertex type reaches the
        /// `VertexDeclaration`-taking constructor beside the `Type` one, which
        /// has no such limit. Recorded as a language-mapping limitation.
        private static let registeredVertexTypes:
            [ObjectIdentifier: RegisteredVertexType] = [
                ObjectIdentifier(VertexPositionColor.self): RegisteredVertexType(
                    declaration: VertexPositionColor.VertexDeclaration,
                    size: MemoryLayout<VertexPositionColor>.size),
                ObjectIdentifier(VertexPositionColorTexture.self): RegisteredVertexType(
                    declaration: VertexPositionColorTexture.VertexDeclaration,
                    size: MemoryLayout<VertexPositionColorTexture>.size),
                ObjectIdentifier(VertexPositionNormalTexture.self): RegisteredVertexType(
                    declaration: VertexPositionNormalTexture.VertexDeclaration,
                    size: MemoryLayout<VertexPositionNormalTexture>.size),
                ObjectIdentifier(VertexPositionTexture.self): RegisteredVertexType(
                    declaration: VertexPositionTexture.VertexDeclaration,
                    size: MemoryLayout<VertexPositionTexture>.size),
            ]

        /// `VertexDeclaration.FromType(Type vertexType)`.
        ///
        /// ```text
        /// if (vertexType == null)
        ///     throw new ArgumentNullException("vertexType", NullNotAllowed);
        /// if (!vertexType.IsValueType)
        ///     throw new ArgumentException(Format(VertexTypeNotValueType, vertexType));
        /// IVertexType instance = Activator.CreateInstance(vertexType) as IVertexType;
        /// if (instance == null)
        ///     throw new ArgumentException(Format(VertexTypeNotIVertexType, vertexType));
        /// VertexDeclaration declaration = instance.VertexDeclaration;
        /// if (declaration == null)
        ///     throw new InvalidOperationException(
        ///         Format(VertexTypeNullDeclaration, vertexType));
        /// if (Marshal.SizeOf(vertexType) != declaration._vertexStride)
        ///     throw new InvalidOperationException(
        ///         Format(VertexTypeWrongSize, vertexType));
        /// return declaration;
        /// ```
        ///
        /// Four of the five tests are reproduced. The null type cannot be
        /// reached through a Swift metatype parameter, and the null declaration
        /// cannot be reached through a non-Optional Swift property; both are
        /// recorded. The size test is real and is the one that catches a vertex
        /// struct whose Swift layout has drifted from its declared stride.
        internal static func fromVertexType(
            _ vertexType: Any.Type
        ) throws -> Microsoft.Xna.Framework.Graphics.VertexDeclaration {
            guard !(vertexType is AnyClass) else {
                throw CNAArgumentException(
                    message: Microsoft.Xna.Framework.Graphics.BufferResources
                        .formatted(
                            Microsoft.Xna.Framework.Graphics.BufferResources
                                .vertexTypeNotValueType, vertexType))
            }
            guard let registered = registeredVertexTypes[ObjectIdentifier(vertexType)] else {
                // A type that is not an IVertexType at all gets XNA's own
                // message. One that IS an IVertexType but is not registered
                // gets the language limitation, on the runtime channel, because
                // saying it does not implement the interface would be false.
                if vertexType is any Microsoft.Xna.Framework.Graphics.IVertexType.Type {
                    throw CNAError.producerInvariant(
                        "\(GraphicsResource.clrTypeName(ofType: vertexType)) implements "
                        + "IVertexType, but Swift cannot construct a value from a "
                        + "metatype the way Activator.CreateInstance does, and "
                        + "IVertexType.VertexDeclaration is an instance member. "
                        + "Use the VertexDeclaration-taking constructor instead")
                }
                throw CNAArgumentException(
                    message: Microsoft.Xna.Framework.Graphics.BufferResources
                        .formatted(
                            Microsoft.Xna.Framework.Graphics.BufferResources
                                .vertexTypeNotIVertexType, vertexType))
            }
            guard Int32(registered.size) == registered.declaration.VertexStride else {
                throw CNAInvalidOperationException(
                    message: Microsoft.Xna.Framework.Graphics.BufferResources
                        .formatted(
                            Microsoft.Xna.Framework.Graphics.BufferResources
                                .vertexTypeWrongSize, vertexType))
            }
            return registered.declaration
        }

        /// A native declaration matching this one, for the caller to destroy.
        ///
        /// `cna_vertex_buffer_create` takes a declaration handle and **copies**
        /// the declaration into the buffer, so the handle is created for the
        /// call and released immediately after. That is why `VertexDeclaration`
        /// still holds no native storage of its own: Foundation 43 left it
        /// managed because nothing in its public surface needed a handle, and
        /// nothing here changes that.
        ///
        /// The explicit-stride route is the one used, because XNA's declaration
        /// carries a stride that a caller may have supplied rather than one
        /// computed from the elements, and the two can differ.
        internal static func nativeDeclaration(
            _ declaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration,
            runtime: RuntimeState
        ) throws -> UInt64 {
            let elements = try declaration.GetVertexElements()
            let native = elements.map {
                CNASwift_VertexElement(
                    offset: $0.Offset,
                    format: UInt32(bitPattern: $0.VertexElementFormat.rawValue),
                    usage: UInt32(bitPattern: $0.VertexElementUsage.rawValue),
                    usage_index: $0.UsageIndex)
            }
            var handle: UInt64 = 0
            try runtime.functions.check(
                native.withUnsafeBufferPointer { buffer in
                    runtime.functions.vertexDeclarationCreateWithStride(
                        declaration.VertexStride, buffer.baseAddress,
                        UInt64(buffer.count), &handle)
                },
                operation: "cna_vertex_declaration_create_with_stride")
            return handle
        }
    }
}
