// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics.GraphicsDevice {
    /// `FrameworkResources.MustDrawSomething`.
    internal static let mustDrawSomethingMessage =
        "When drawing, at least one primitive must be drawn."
    /// `FrameworkResources.NumberVerticesMustBeGreaterZero`.
    internal static let numberVerticesMustBeGreaterZeroMessage =
        "When drawing indexed primitives, the number of vertices passed in "
        + "must be greater than zero."
    /// `FrameworkResources.NonZeroInstanceFrequency`.
    internal static let nonZeroInstanceFrequencyMessage =
        "Non-instanced draw calls are not valid when a vertex buffer is bound "
        + "with a non-zero instance frequency."
    /// `FrameworkResources.OffsetNotValid`.
    internal static let offsetNotValidMessage =
        "The offset must be within the valid range for this resource."
    /// `FrameworkResources.InvalidInstanceStreams`.
    internal static let invalidInstanceStreamsMessage =
        "DrawInstancedPrimitives requires at least one vertex buffer to be "
        + "bound with a non-zero instance frequency, and also at least one "
        + "with a zero instance frequency."

    // ------------------------------------------------------------------
    // The shared validation. Every draw begins with some prefix of it, in
    // this order, and the order is observable.

    /// `Helpers.CheckDisposed`, then the two primitive-count tests.
    ///
    /// ```text
    /// Helpers.CheckDisposed(this, pComPtr);
    /// if (primitiveCount <= 0)
    ///     throw new ArgumentOutOfRangeException(
    ///         "primitiveCount", MustDrawSomething);
    /// if (primitiveCount > MaxPrimitiveCount)
    ///     Throw(ProfileMaxPrimitiveCount, MaxPrimitiveCount);
    /// ```
    ///
    /// Reach's `MaxPrimitiveCount` is 65,535 and HiDef's is 1,048,575, from the
    /// table extracted in Foundation 62. The profile message names the
    /// **limit**, not the request, like every other profile message here.
    private func validatePrimitiveCount(
        _ primitiveCount: Int32, parameter: String = "primitiveCount"
    ) throws {
        guard primitiveCount > 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: parameter,
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .mustDrawSomethingMessage)
        }
        let capabilities = profileCapabilities
        guard primitiveCount <= capabilities.maxPrimitiveCount else {
            try capabilities.throwNotSupported(
                Microsoft.Xna.Framework.Graphics.ProfileCapabilities
                    .profileMaxPrimitiveCount,
                "\(capabilities.maxPrimitiveCount)")
        }
    }

    /// `if (instanceStreamMask != 0) throw
    /// new InvalidOperationException(NonZeroInstanceFrequency);`
    ///
    /// XNA keeps `instanceStreamMask` as a bitmask that `SetVertexBuffers`
    /// fills from each binding's `InstanceFrequency`. This binding already
    /// holds those bindings — `GetVertexBuffers()` reads the same cache — so
    /// the mask is derived rather than tracked separately, which is one fewer
    /// thing that can fall out of step.
    ///
    /// **This is the test that separates the instanced draw from the other
    /// three**: they refuse a non-zero frequency, and `DrawInstancedPrimitives`
    /// is the one that needs it.
    private func refuseInstancedStreams() throws {
        guard boundInstanceStreamMask == 0 else {
            throw CNAInvalidOperationException(
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .nonZeroInstanceFrequencyMessage)
        }
    }

    /// `GraphicsDevice.instanceStreamMask`, derived from the bound bindings.
    internal var boundInstanceStreamMask: Int32 {
        var mask: Int32 = 0
        for (index, binding) in runtimeState.cachedVertexBufferBindings.enumerated()
        where binding.InstanceFrequency != 0 {
            mask |= Int32(truncatingIfNeeded: 1 << min(index, 30))
        }
        return mask
    }

    /// `GraphicsDevice.GetElementCountFromPrimitiveType(PrimitiveType, Int32)`
    /// — how many vertices a primitive count needs.
    ///
    /// XNA computes it inline per topology; CNA publishes
    /// `cna_primitive_type_get_vertex_count`, which answers the same numbers
    /// and is the route this consumes. A `TriangleList` of one needs 3, which
    /// `build-probe/f67_effect.c` measured.
    internal func elementCount(
        for primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        primitiveCount: Int32
    ) throws -> Int32 {
        var vertices: Int32 = 0
        try runtimeState.functions.check(
            runtimeState.functions.primitiveTypeGetVertexCount(
                UInt32(bitPattern: primitiveType.rawValue), primitiveCount, &vertices),
            operation: "cna_primitive_type_get_vertex_count")
        return vertices
    }

    // ------------------------------------------------------------------
    // The three draws from bound buffers.

    /// `GraphicsDevice.DrawPrimitives(PrimitiveType, Int32 startVertex,
    /// Int32 primitiveCount)`.
    ///
    /// **What this call can and cannot be said to do.** It reaches
    /// `cna_graphics_device_draw_primitives` and reports what that answers.
    /// Nothing here asserts that a pixel arrived anywhere, because nothing on
    /// this host can read one back — `cna_graphics_device_get_backbuffer_data_window`
    /// answers `NOT_SUPPORTED` and has since Foundation 53. Geometry, blending,
    /// sampling and sort order remain unverifiable.
    ///
    /// Two refusals a caller will meet that XNA never produces, both from CNA
    /// and both on the runtime channel:
    ///
    /// * **no effect applied** — `CNA_RESULT_INTERNAL`, *"no effect has been
    ///   applied"*. XNA raises `InvalidOperationException(CannotDrawNoShader)`
    ///   from `VerifyCanDraw` for the same state, but that conclusion is drawn
    ///   from a D3D state tracker this binding cannot see, so the native
    ///   verdict is forwarded rather than a managed one invented.
    /// * **a vertex buffer with no data uploaded** — `CNA_RESULT_INVALID_ARGUMENT`,
    ///   *"The requested primitive range exceeds the bound vertex buffer"*.
    ///   **XNA has no such rule**: an un-`SetData`'d `VertexBuffer` draws
    ///   undefined contents there rather than raising. CNA counts vertices
    ///   *written*, not capacity allocated, measured in
    ///   `build-probe/f67_effect.c`. It is forwarded for the same reason — a
    ///   managed pre-check would have to track every byte ever written to every
    ///   buffer, which is state this binding has no business keeping and which
    ///   XNA does not keep either.
    public func DrawPrimitives(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        startVertex: Int32,
        primitiveCount: Int32
    ) throws {
        let handle = try validatedHandle("GraphicsDevice.DrawPrimitives")
        try validatePrimitiveCount(primitiveCount)
        try refuseInstancedStreams()
        try runtimeState.functions.check(
            runtimeState.functions.graphicsDeviceDrawPrimitives(
                handle, UInt32(bitPattern: primitiveType.rawValue),
                startVertex, primitiveCount),
            operation: "cna_graphics_device_draw_primitives")
    }

    /// `GraphicsDevice.DrawIndexedPrimitives(PrimitiveType, Int32 baseVertex,
    /// Int32 minVertexIndex, Int32 numVertices, Int32 startIndex,
    /// Int32 primitiveCount)`.
    ///
    /// The vertex-count test comes **first**, before the primitive count:
    /// `numVertices <= 0` raises `ArgumentOutOfRangeException("numVertices",
    /// NumberVerticesMustBeGreaterZero)` at `IL_0018`, and `primitiveCount`'s
    /// own test is at `IL_002d`.
    public func DrawIndexedPrimitives(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        baseVertex: Int32,
        minVertexIndex: Int32,
        numVertices: Int32,
        startIndex: Int32,
        primitiveCount: Int32
    ) throws {
        let handle = try validatedHandle("GraphicsDevice.DrawIndexedPrimitives")
        guard numVertices > 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "numVertices",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .numberVerticesMustBeGreaterZeroMessage)
        }
        try validatePrimitiveCount(primitiveCount)
        try refuseInstancedStreams()
        try runtimeState.functions.check(
            runtimeState.functions.graphicsDeviceDrawIndexedPrimitives(
                handle, UInt32(bitPattern: primitiveType.rawValue),
                baseVertex, minVertexIndex, numVertices, startIndex,
                primitiveCount),
            operation: "cna_graphics_device_draw_indexed_primitives")
    }

    /// `GraphicsDevice.DrawInstancedPrimitives(PrimitiveType, Int32 baseVertex,
    /// Int32 minVertexIndex, Int32 numVertices, Int32 startIndex,
    /// Int32 primitiveCount, Int32 instanceCount)`.
    ///
    /// The same tests, plus `instanceCount` measured against
    /// **`MustDrawSomething`** and `MaxPrimitiveCount` — the same two messages
    /// the primitive count uses, with `"instanceCount"` as the parameter name.
    /// XNA reuses both rather than inventing an instance-specific pair.
    ///
    /// It does **not** refuse a non-zero instance stream: this is the draw that
    /// needs one. It requires a **mixture**, which is a different test:
    ///
    /// ```text
    /// mask    = instanceStreamMask;
    /// allZero = (mask == 0);
    /// allOne  = (mask == (1 << currentVertexBufferCount) - 1);
    /// if (allZero || allOne)
    ///     throw new InvalidOperationException(InvalidInstanceStreams);
    /// ```
    ///
    /// So *every* stream instanced is as wrong as *none* — an instanced draw
    /// needs per-instance data and per-vertex data, and a set of bindings that
    /// is all one or all the other supplies only half of it.
    public func DrawInstancedPrimitives(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        baseVertex: Int32,
        minVertexIndex: Int32,
        numVertices: Int32,
        startIndex: Int32,
        primitiveCount: Int32,
        instanceCount: Int32
    ) throws {
        let handle = try validatedHandle("GraphicsDevice.DrawInstancedPrimitives")
        guard numVertices > 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "numVertices",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .numberVerticesMustBeGreaterZeroMessage)
        }
        try validatePrimitiveCount(primitiveCount)
        try validatePrimitiveCount(instanceCount, parameter: "instanceCount")
        let mask = boundInstanceStreamMask
        let streams = runtimeState.cachedVertexBufferBindings.count
        let allOne = streams > 0
            && mask == Int32(truncatingIfNeeded: (1 << min(streams, 30)) - 1)
        guard mask != 0, !allOne else {
            throw CNAInvalidOperationException(
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .invalidInstanceStreamsMessage)
        }
        try runtimeState.functions.check(
            runtimeState.functions.graphicsDeviceDrawInstancedPrimitives(
                handle, UInt32(bitPattern: primitiveType.rawValue),
                baseVertex, minVertexIndex, numVertices, startIndex,
                primitiveCount, instanceCount),
            operation: "cna_graphics_device_draw_instanced_primitives")
    }

    // ------------------------------------------------------------------
    // The user-primitive draws, whose validation is entirely managed.

    /// `DrawUserPrimitives<T>(PrimitiveType, T[] vertexData, Int32 vertexOffset,
    /// Int32 primitiveCount, VertexDeclaration vertexDeclaration)`.
    ///
    /// ```text
    /// Helpers.CheckDisposed(this, pComPtr);
    /// if (vertexData == null || vertexData.Length == 0)
    ///     throw new ArgumentNullException("vertexData", NullNotAllowed);
    /// if (vertexDeclaration == null)
    ///     throw new ArgumentNullException("vertexDeclaration", NullNotAllowed);
    /// if (primitiveCount <= 0)
    ///     throw new ArgumentOutOfRangeException("primitiveCount", MustDrawSomething);
    /// if (primitiveCount > MaxPrimitiveCount)
    ///     Throw(ProfileMaxPrimitiveCount, MaxPrimitiveCount);
    /// if (vertexOffset < 0 || vertexOffset >= vertexData.Length)
    ///     throw new ArgumentOutOfRangeException("vertexOffset", OffsetNotValid);
    /// if (GetElementCountFromPrimitiveType(primitiveType, primitiveCount)
    ///         + vertexOffset > vertexData.Length)
    ///     throw new ArgumentOutOfRangeException("primitiveCount", MustBeValidIndex);
    /// ```
    ///
    /// Seven tests, all managed, all reachable — which makes this the one draw
    /// family whose refusals are fully this binding's own rather than
    /// forwarded. The last comparison is `ble.un`, **unsigned**, so a
    /// vertex offset plus element count that overflows wraps to a huge value
    /// and is caught there rather than slipping through.
    ///
    /// `T` is blitted as a raw stream at the declaration's stride, which is
    /// what `CNA_USER_VERTEX_SOURCE_RAW_STREAM` means and what XNA's own
    /// `sizeof(T)` blit does.
    public func DrawUserPrimitives<T>(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        vertexData: [T],
        vertexOffset: Int32,
        primitiveCount: Int32,
        vertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration
    ) throws {
        let handle = try validatedHandle("GraphicsDevice.DrawUserPrimitives")
        try validateUserVertices(
            primitiveType, count: vertexData.count, vertexOffset: vertexOffset,
            primitiveCount: primitiveCount)
        // A `VertexDeclaration` is managed-only: Foundation 44 left it with no
        // native object because nothing consumed one. `VertexBuffer`'s
        // constructor was the first to need one and builds a native
        // declaration for the length of the call; a user-primitive draw is the
        // second, and takes the same path for the same reason -- the native
        // declaration describes this call's stream and has no life beyond it.
        let declaration = try Microsoft.Xna.Framework.Graphics.VertexDeclaration
            .nativeDeclaration(vertexDeclaration, runtime: runtimeState)
        defer { _ = runtimeState.functions.vertexDeclarationDestroy(declaration) }
        var primitives = userPrimitives(
            primitiveType, declaration: declaration, vertexOffset: vertexOffset,
            numVertices: 0, primitiveCount: primitiveCount)
        try runtimeState.functions.check(
            vertexData.withUnsafeBytes { bytes in
                primitives.vertex_data = bytes.baseAddress
                return withUnsafePointer(to: &primitives) {
                    runtimeState.functions.graphicsDeviceDrawUserPrimitives(handle, $0)
                }
            },
            operation: "cna_graphics_device_draw_user_primitives")
    }

    /// `DrawUserPrimitives<T>(PrimitiveType, T[] vertexData, Int32 vertexOffset,
    /// Int32 primitiveCount)` — the overload without a declaration.
    ///
    /// XNA reads `T`'s own declaration through `VertexDeclaration.FromType`,
    /// which is reflection and fails at run time for a `T` that is not an
    /// `IVertexType`. The generic parameter is therefore **unconstrained**,
    /// exactly as the contract declares it, and the failure stays where XNA
    /// puts it — `VertexBuffer`'s typed constructor takes the same path through
    /// the same helper.
    public func DrawUserPrimitives<T>(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        vertexData: [T],
        vertexOffset: Int32,
        primitiveCount: Int32
    ) throws {
        try DrawUserPrimitives(
            primitiveType, vertexData: vertexData, vertexOffset: vertexOffset,
            primitiveCount: primitiveCount,
            vertexDeclaration: try Microsoft.Xna.Framework.Graphics
                .VertexDeclaration.fromVertexType(T.self))
    }

    /// `DrawUserIndexedPrimitives<T>(PrimitiveType, T[] vertexData,
    /// Int32 vertexOffset, Int32 numVertices, Int16[] indexData,
    /// Int32 indexOffset, Int32 primitiveCount, VertexDeclaration)`.
    public func DrawUserIndexedPrimitives<T>(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        vertexData: [T],
        vertexOffset: Int32,
        numVertices: Int32,
        indexData: [Int16],
        indexOffset: Int32,
        primitiveCount: Int32,
        vertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration
    ) throws {
        try drawUserIndexed(
            primitiveType, vertexData: vertexData, vertexOffset: vertexOffset,
            numVertices: numVertices, indexData: indexData,
            indexOffset: indexOffset, primitiveCount: primitiveCount,
            vertexDeclaration: vertexDeclaration, sixteenBit: true)
    }

    /// The 32-bit index overload.
    ///
    /// **Refused on a Reach device**, and that is XNA's rule rather than a host
    /// limit: `IndexElementSize32` is false in Reach's extracted table, which
    /// `IndexBuffer` already reproduces. The same refusal applies here because
    /// the same profile decides it.
    public func DrawUserIndexedPrimitives<T>(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        vertexData: [T],
        vertexOffset: Int32,
        numVertices: Int32,
        indexData: [Int32],
        indexOffset: Int32,
        primitiveCount: Int32,
        vertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration
    ) throws {
        try drawUserIndexed(
            primitiveType, vertexData: vertexData, vertexOffset: vertexOffset,
            numVertices: numVertices, indexData: indexData,
            indexOffset: indexOffset, primitiveCount: primitiveCount,
            vertexDeclaration: vertexDeclaration, sixteenBit: false)
    }

    /// The 16-bit index overload without a declaration.
    public func DrawUserIndexedPrimitives<T>(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        vertexData: [T],
        vertexOffset: Int32,
        numVertices: Int32,
        indexData: [Int16],
        indexOffset: Int32,
        primitiveCount: Int32
    ) throws {
        try DrawUserIndexedPrimitives(
            primitiveType, vertexData: vertexData, vertexOffset: vertexOffset,
            numVertices: numVertices, indexData: indexData,
            indexOffset: indexOffset, primitiveCount: primitiveCount,
            vertexDeclaration: try Microsoft.Xna.Framework.Graphics
                .VertexDeclaration.fromVertexType(T.self))
    }

    /// The 32-bit index overload without a declaration.
    public func DrawUserIndexedPrimitives<T>(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        vertexData: [T],
        vertexOffset: Int32,
        numVertices: Int32,
        indexData: [Int32],
        indexOffset: Int32,
        primitiveCount: Int32
    ) throws {
        try DrawUserIndexedPrimitives(
            primitiveType, vertexData: vertexData, vertexOffset: vertexOffset,
            numVertices: numVertices, indexData: indexData,
            indexOffset: indexOffset, primitiveCount: primitiveCount,
            vertexDeclaration: try Microsoft.Xna.Framework.Graphics
                .VertexDeclaration.fromVertexType(T.self))
    }

    // ------------------------------------------------------------------

    private func validateUserVertices(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        count: Int, vertexOffset: Int32, primitiveCount: Int32
    ) throws {
        guard count > 0 else {
            throw CNAArgumentNullException(
                paramName: "vertexData",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .nullNotAllowedMessage)
        }
        try validatePrimitiveCount(primitiveCount)
        guard vertexOffset >= 0, Int(vertexOffset) < count else {
            throw CNAArgumentOutOfRangeException(
                paramName: "vertexOffset",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .offsetNotValidMessage)
        }
        let needed = try elementCount(
            for: primitiveType, primitiveCount: primitiveCount)
        guard Microsoft.Xna.Framework.Graphics.GraphicsDevice.arrayHolds(
            offset: vertexOffset, elements: needed, count: count) else {
            throw CNAArgumentOutOfRangeException(
                paramName: "primitiveCount",
                message: Microsoft.Xna.Framework.Graphics.BufferResources
                    .mustBeValidIndex)
        }
    }

    /// `if (GetElementCountFromPrimitiveType(...) + offset > array.Length)` —
    /// XNA's own comparison, and it is `ble.un`, **unsigned**.
    ///
    /// Split out from its two call sites because it **cannot be exercised
    /// through them on this profile**. The sum can only overflow when the
    /// element count is enormous, and `primitiveCount` is capped at Reach's
    /// `MaxPrimitiveCount` of 65,535 several tests earlier, while the offset is
    /// capped below the array's own length. So the unsigned spelling is
    /// unfalsifiable through a draw, and a mutation making it signed survived
    /// until the arithmetic was testable on its own — the same remedy
    /// `sameRenderTargetShape` needed in Foundation 65.
    internal static func arrayHolds(
        offset: Int32, elements: Int32, count: Int
    ) -> Bool {
        let end = UInt32(bitPattern: elements) &+ UInt32(bitPattern: offset)
        return end <= UInt32(clamping: count)
    }

    private func userPrimitives(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        declaration: UInt64, vertexOffset: Int32, numVertices: Int32,
        primitiveCount: Int32
    ) -> CNASwift_UserPrimitives {
        var primitives = CNASwift_UserPrimitives()
        primitives.struct_size = UInt32(MemoryLayout<CNASwift_UserPrimitives>.size)
        primitives.struct_version = 1
        primitives.primitive_type = UInt32(bitPattern: primitiveType.rawValue)
        // The array is bytes at the declaration's own stride, which is what a
        // raw stream is; the typed sources exist for callers who have a
        // `CNA_VertexPositionColor` array and no declaration, and this binding
        // always has one.
        primitives.vertex_source = 0
        primitives.vertex_declaration = declaration
        primitives.vertex_offset = vertexOffset
        primitives.num_vertices = numVertices
        primitives.primitive_count = primitiveCount
        return primitives
    }

    private func drawUserIndexed<T, Index>(
        _ primitiveType: Microsoft.Xna.Framework.Graphics.PrimitiveType,
        vertexData: [T], vertexOffset: Int32, numVertices: Int32,
        indexData: [Index], indexOffset: Int32, primitiveCount: Int32,
        vertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration,
        sixteenBit: Bool
    ) throws {
        let handle = try validatedHandle("GraphicsDevice.DrawUserIndexedPrimitives")
        guard vertexData.count > 0 else {
            throw CNAArgumentNullException(
                paramName: "vertexData",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .nullNotAllowedMessage)
        }
        guard indexData.count > 0 else {
            throw CNAArgumentNullException(
                paramName: "indexData",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .nullNotAllowedMessage)
        }
        guard numVertices > 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "numVertices",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .numberVerticesMustBeGreaterZeroMessage)
        }
        try validatePrimitiveCount(primitiveCount)
        if !sixteenBit {
            // The same profile test `IndexBuffer` makes, for the same reason:
            // Reach's IndexElementSize32 is false.
            let capabilities = profileCapabilities
            guard capabilities.indexElementSize32 else {
                try capabilities.throwNotSupported(
                    Microsoft.Xna.Framework.Graphics.ProfileCapabilities
                        .profileNoIndexElementSize32)
            }
        }
        guard vertexOffset >= 0, Int(vertexOffset) < vertexData.count else {
            throw CNAArgumentOutOfRangeException(
                paramName: "vertexOffset",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .offsetNotValidMessage)
        }
        guard indexOffset >= 0, Int(indexOffset) < indexData.count else {
            throw CNAArgumentOutOfRangeException(
                paramName: "indexOffset",
                message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                    .offsetNotValidMessage)
        }
        let needed = try elementCount(
            for: primitiveType, primitiveCount: primitiveCount)
        guard Microsoft.Xna.Framework.Graphics.GraphicsDevice.arrayHolds(
            offset: indexOffset, elements: needed, count: indexData.count) else {
            throw CNAArgumentOutOfRangeException(
                paramName: "primitiveCount",
                message: Microsoft.Xna.Framework.Graphics.BufferResources
                    .mustBeValidIndex)
        }

        let declaration = try Microsoft.Xna.Framework.Graphics.VertexDeclaration
            .nativeDeclaration(vertexDeclaration, runtime: runtimeState)
        defer { _ = runtimeState.functions.vertexDeclarationDestroy(declaration) }
        var primitives = userPrimitives(
            primitiveType, declaration: declaration, vertexOffset: vertexOffset,
            numVertices: numVertices, primitiveCount: primitiveCount)
        var indices = CNASwift_UserIndices()
        indices.struct_size = UInt32(MemoryLayout<CNASwift_UserIndices>.size)
        indices.struct_version = 1
        indices.index_element_size = sixteenBit ? 0 : 1
        indices.index_offset = indexOffset

        try runtimeState.functions.check(
            vertexData.withUnsafeBytes { vertexBytes in
                indexData.withUnsafeBytes { indexBytes in
                    primitives.vertex_data = vertexBytes.baseAddress
                    indices.index_data = indexBytes.baseAddress
                    return withUnsafePointer(to: &primitives) { p in
                        withUnsafePointer(to: &indices) { i in
                            runtimeState.functions
                                .graphicsDeviceDrawUserIndexedPrimitives(handle, p, i)
                        }
                    }
                }
            },
            operation: "cna_graphics_device_draw_user_indexed_primitives")
    }
}
