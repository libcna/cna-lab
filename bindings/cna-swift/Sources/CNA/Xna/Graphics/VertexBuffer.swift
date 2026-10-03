// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.VertexBuffer` projection.
    ///
    /// `open`, not `final`: XNA leaves it derivable and `DynamicVertexBuffer`
    /// is the derived class that proves it.
    ///
    /// ## What CNA's raw transfer can express
    ///
    /// XNA's `CopyData<T>` writes `elementCount` elements of `sizeof(T)` bytes
    /// **spaced `vertexStride` apart**, reading them packed from the caller's
    /// array, and requires only `vertexStride >= sizeof(T)`; the buffer's own
    /// declaration stride is never compared against it. CNA's raw upload takes
    /// a byte count, a vertex count and a stride, and
    /// `build-probe/f60_stride.c` measures which triples it accepts: the
    /// stride must equal the buffer's declaration stride, the byte count must
    /// be exactly `count * stride`, and a windowed offset must be a multiple
    /// of the stride.
    ///
    /// So a transfer whose destination is a contiguous run of whole vertices —
    /// which is every transfer with `vertexStride` 0 or equal to `sizeof(T)`,
    /// and therefore every ordinary use of the six overloads — is expressible
    /// exactly. One that leaves a gap between elements is not, and is refused
    /// on the runtime channel rather than written wrongly. Every XNA
    /// validation runs **first** either way, so a call XNA would have rejected
    /// is rejected here with XNA's own exception rather than with this
    /// binding's limit.
    open class VertexBuffer: GraphicsResource {
        /// `VertexBuffer.VertexDeclaration`.
        ///
        /// The **same object** the constructor was given. The `Type` overload's
        /// declaration comes from `VertexDeclaration.FromType`, which returns
        /// the vertex struct's own static declaration and caches it, so two
        /// buffers of one vertex type share one declaration — as in XNA, where
        /// `FromType` caches the object `Activator.CreateInstance` produced.
        ///
        /// **Optional because the pinned nullability record proves it can be
        /// null**: `_vertexDeclaration` is assigned inside `CreateBuffer`, which
        /// the constructor calls under a `fault` handler, so a partially
        /// constructed buffer can be observed with the field unset. This
        /// projection cannot reach that state — a failing Swift initializer
        /// returns no object — but the shape follows the proof rather than the
        /// implementation, which is the same rule
        /// `GraphicsResource.GraphicsDevice` follows.
        public var VertexDeclaration:
            Microsoft.Xna.Framework.Graphics.VertexDeclaration? { storedVertexDeclaration }

        /// The declaration every transfer measures itself against.
        internal let storedVertexDeclaration:
            Microsoft.Xna.Framework.Graphics.VertexDeclaration

        /// `VertexBuffer.VertexCount`.
        public let VertexCount: Int32

        /// `VertexBuffer.BufferUsage`.
        public let BufferUsage: Microsoft.Xna.Framework.Graphics.BufferUsage

        /// `_size`: the byte length every transfer is compared against.
        ///
        /// XNA computes it in `CreateBuffer` as `vertexCount * stride` and
        /// stores it. Reading CNA's granted `vertex_stride` back instead would
        /// make the comparison depend on the renderer where XNA's depends only
        /// on the declaration, so the declaration is what it is computed from —
        /// and a test asserts the two agree.
        internal let sizeInBytes: Int

        internal init(
            handle: UInt64,
            runtime: RuntimeState,
            device: GraphicsDevice,
            typeName: String,
            declaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration,
            vertexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage
        ) {
            storedVertexDeclaration = declaration
            VertexCount = vertexCount
            BufferUsage = usage
            sizeInBytes = Int(vertexCount) * Int(declaration.VertexStride)
            super.init(
                storage: NativeHandleStorage(
                    handle: handle,
                    typeName: typeName,
                    ownership: .owned,
                    runtime: runtime,
                    destroy: runtime.functions.vertexBufferDestroy),
                device: device)
            runtime.register(self)
        }

        /// `VertexBuffer(GraphicsDevice, VertexDeclaration, Int32, BufferUsage)`.
        ///
        /// ```text
        /// if (vertexDeclaration == null)
        ///     throw new ArgumentNullException("vertexDeclaration", NullNotAllowed);
        /// if (vertexCount <= 0)
        ///     throw new ArgumentOutOfRangeException(
        ///         "vertexCount", ResourcesMustBeGreaterThanZeroSize);
        /// _parent = graphicsDevice;
        /// CreateBuffer(vertexDeclaration, vertexCount,
        ///              ConvertXnaBufferUsageToDx(usage), D3DPOOL_MANAGED);
        /// ```
        ///
        /// There is **no null check on `graphicsDevice`**: XNA stores it and
        /// lets `CreateBuffer` dereference it. Both null branches are
        /// unreachable through non-Optional Swift parameters and are recorded
        /// rather than written.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            vertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration,
            vertexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage
        ) throws {
            try self.init(
                graphicsDevice: graphicsDevice, declaration: vertexDeclaration,
                vertexCount: vertexCount, usage: usage, dynamic: false,
                typeName: "VertexBuffer")
        }

        /// `VertexBuffer(GraphicsDevice, Type, Int32, BufferUsage)`.
        ///
        /// `VertexDeclaration.FromType(vertexType)` runs **first** — the IL
        /// calls it at `IL_0007` and tests the count at `IL_000d` — so a bad
        /// vertex type is reported even when the count is also wrong.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            vertexType: Any.Type,
            vertexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage
        ) throws {
            let declaration = try Microsoft.Xna.Framework.Graphics.VertexDeclaration
                .fromVertexType(vertexType)
            try self.init(
                graphicsDevice: graphicsDevice, declaration: declaration,
                vertexCount: vertexCount, usage: usage, dynamic: false,
                typeName: "VertexBuffer")
        }

        /// The one creation path, shared with `DynamicVertexBuffer`.
        internal convenience init(
            graphicsDevice: GraphicsDevice,
            declaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration,
            vertexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage,
            dynamic: Bool,
            typeName: String
        ) throws {
            guard vertexCount > 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "vertexCount",
                    message: Microsoft.Xna.Framework.Graphics.Texture2D
                        .resourcesMustBeGreaterThanZeroSizeMessage)
            }
            // `CreateBuffer`'s own profile check, on the byte size the
            // declaration and the count imply.
            try graphicsDevice.profileCapabilities.validateVertexBufferSize(
                Int(vertexCount) * Int(declaration.VertexStride))
            let deviceHandle = try graphicsDevice.validatedHandle("VertexBuffer.init")
            let runtime = graphicsDevice.runtimeState
            let native = try Microsoft.Xna.Framework.Graphics.VertexDeclaration
                .nativeDeclaration(declaration, runtime: runtime)
            defer { _ = runtime.functions.vertexDeclarationDestroy(native) }

            var create = CNASwift_VertexBufferCreateInfo()
            create.struct_size = UInt32(MemoryLayout<CNASwift_VertexBufferCreateInfo>.size)
            create.struct_version = 1
            create.vertex_declaration = native
            create.vertex_count = vertexCount
            create.buffer_usage = UInt32(bitPattern: usage.rawValue)
            create.dynamic = dynamic ? 1 : 0

            var handle: UInt64 = 0
            try runtime.functions.check(
                withUnsafePointer(to: &create) {
                    runtime.functions.vertexBufferCreate(deviceHandle, $0, &handle)
                },
                operation: "cna_vertex_buffer_create")
            self.init(
                handle: handle, runtime: runtime, device: graphicsDevice,
                typeName: typeName, declaration: declaration,
                vertexCount: vertexCount, usage: usage)
        }

        // ------------------------------------------------------------------
        // SetData and GetData.

        /// `SetData<T>(T[] data)` — `SetData(0, data, 0, data.Length, 0)`.
        public func SetData<T>(_ data: [T]) throws {
            try SetData(0, data: data, startIndex: 0,
                        elementCount: Int32(data.count), vertexStride: 0)
        }

        /// `SetData<T>(T[] data, Int32 startIndex, Int32 elementCount)`.
        public func SetData<T>(
            _ data: [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try SetData(0, data: data, startIndex: startIndex,
                        elementCount: elementCount, vertexStride: 0)
        }

        /// `SetData<T>(Int32 offsetInBytes, T[] data, Int32 startIndex,
        /// Int32 elementCount, Int32 vertexStride)`.
        ///
        /// `cna_vertex_buffer_set_data_raw_at` is the route, and not the
        /// `_with_options` one beside it: `build-probe/f60_options.c` measures
        /// the option-taking upload answering `CNA_RESULT_NOT_SUPPORTED` on a
        /// static buffer for **every** option value, `CNA_SET_DATA_NONE`
        /// included, and succeeding on a dynamic one. `SetDataOptions` reaches
        /// this family only through `DynamicVertexBuffer`, which is where that
        /// route belongs.
        public func SetData<T>(
            _ offsetInBytes: Int32, data: [T], startIndex: Int32,
            elementCount: Int32, vertexStride: Int32
        ) throws {
            try writeData(offsetInBytes, data: data, startIndex: startIndex,
                          elementCount: elementCount, vertexStride: vertexStride,
                          options: .None)
        }

        /// The one upload path, shared with `DynamicVertexBuffer`.
        ///
        /// Which route carries it is decided by the option, and that is
        /// measured rather than chosen: `build-probe/f60_options.c` has the
        /// option-taking upload answering `CNA_RESULT_NOT_SUPPORTED` on a
        /// static buffer for **every** option value, `CNA_SET_DATA_NONE`
        /// included, and succeeding for all three on a dynamic one. So `None`
        /// goes through the option-free route, which both kinds accept, and a
        /// real option goes through the one only a dynamic buffer has.
        internal func writeData<T>(
            _ offsetInBytes: Int32, data: [T], startIndex: Int32,
            elementCount: Int32, vertexStride: Int32,
            options: Microsoft.Xna.Framework.Graphics.SetDataOptions
        ) throws {
            let plan = try copyPlan(
                T.self, offsetInBytes: offsetInBytes, arrayCount: data.count,
                startIndex: startIndex, elementCount: elementCount,
                vertexStride: vertexStride, isSetting: true,
                checksBinding: options == .None)
            let functions = nativeStorage.runtime.functions
            try data.withUnsafeBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                let source = base + plan.byteOffset
                if options == .None {
                    try functions.check(
                        functions.vertexBufferSetDataRawAt(
                            plan.handle, UInt64(plan.offsetInBytes), source,
                            UInt64(plan.byteCount), UInt64(plan.vertexCount),
                            UInt32(plan.stride)),
                        operation: "cna_vertex_buffer_set_data_raw_at")
                } else {
                    try functions.check(
                        functions.vertexBufferSetDataRawAtWithOptions(
                            plan.handle, UInt64(plan.offsetInBytes), source,
                            UInt64(plan.byteCount), UInt64(plan.vertexCount),
                            UInt32(plan.stride),
                            UInt32(bitPattern: options.rawValue)),
                        operation: "cna_vertex_buffer_set_data_raw_at_with_options")
                }
            }
        }

        /// `GetData<T>(T[] data)`.
        public func GetData<T>(_ data: inout [T]) throws {
            try GetData(0, data: &data, startIndex: 0,
                        elementCount: Int32(data.count), vertexStride: 0)
        }

        /// `GetData<T>(T[] data, Int32 startIndex, Int32 elementCount)`.
        public func GetData<T>(
            _ data: inout [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try GetData(0, data: &data, startIndex: startIndex,
                        elementCount: elementCount, vertexStride: 0)
        }

        /// `GetData<T>(Int32 offsetInBytes, T[] data, Int32 startIndex,
        /// Int32 elementCount, Int32 vertexStride)`.
        public func GetData<T>(
            _ offsetInBytes: Int32, data: inout [T], startIndex: Int32,
            elementCount: Int32, vertexStride: Int32
        ) throws {
            let plan = try copyPlan(
                T.self, offsetInBytes: offsetInBytes, arrayCount: data.count,
                startIndex: startIndex, elementCount: elementCount,
                vertexStride: vertexStride, isSetting: false)
            let functions = nativeStorage.runtime.functions
            try data.withUnsafeMutableBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                try functions.check(
                    functions.vertexBufferGetDataRaw(
                        plan.handle, UInt64(plan.offsetInBytes),
                        base + plan.byteOffset, UInt64(plan.byteCount),
                        UInt64(plan.vertexCount), UInt32(plan.stride)),
                    operation: "cna_vertex_buffer_get_data_raw")
            }
        }

        internal struct CopyPlan {
            var handle: UInt64
            var offsetInBytes: Int
            var byteOffset: Int
            var byteCount: Int
            var vertexCount: Int
            var stride: Int
        }

        /// `VertexBuffer.CopyData<T>` up to the point where XNA locks the
        /// buffer, then this binding's own expressibility limit.
        ///
        /// ```text
        /// Helpers.CheckDisposed(this, pComPtr);
        /// if (data == null || data.Length == 0)
        ///     throw new ArgumentNullException("data", NullNotAllowed);
        /// if ((options & (Discard | NoOverwrite)) == 0)
        ///     for (i = 0; i < device.currentVertexBufferCount; i++)
        ///         if (device.currentVertexBuffers[i]._vertexBuffer == this)
        ///             throw GetExceptionFromResult(E_ABORT);   // ResourceInUse
        /// if (!isSetting && (usage & WriteOnly) == WriteOnly)
        ///     throw new NotSupportedException(WriteOnlyGetNotSupported);
        /// Helpers.ValidateCopyParameters(data.Length, startIndex, elementCount);
        /// int elementSize = sizeof(T);
        /// int bytes = elementSize * elementCount;
        /// int slack = 0;
        /// if (vertexStride != 0) {
        ///     slack = vertexStride - elementSize;
        ///     if (slack < 0)
        ///         throw new ArgumentOutOfRangeException(
        ///             "vertexStride", VertexStrideTooSmall);
        ///     if (elementCount > 1) bytes += (elementCount - 1) * slack;
        /// }
        /// if (bytes + offsetInBytes > _size)
        ///     throw new InvalidOperationException(ResourceDataMustBeCorrectSize);
        /// ```
        ///
        /// **An empty array is an `ArgumentNullException`.** `IL_001c: ldlen;
        /// IL_001d: brfalse IL_02be` branches to the same throw the null test
        /// uses, so a zero-length array reports `"data"` as null. That is XNA's
        /// own behaviour, reproduced rather than tidied.
        ///
        /// **The bound-buffer test is the managed cache's.** `options` is the
        /// D3D lock flag set: `GetData` passes `READONLY` and a dynamic buffer's
        /// `Discard`/`NoOverwrite` pass theirs, so only a plain `SetData` scans
        /// the bound streams. CNA refuses the same upload natively since ABI
        /// 0.35, on the runtime channel; this raises XNA's exception first.
        internal func copyPlan<T>(
            _ element: T.Type, offsetInBytes: Int32, arrayCount: Int,
            startIndex: Int32, elementCount: Int32, vertexStride: Int32,
            isSetting: Bool, checksBinding: Bool = false
        ) throws -> CopyPlan {
            let handle = try validatedHandle(
                isSetting ? "VertexBuffer.SetData" : "VertexBuffer.GetData")
            guard arrayCount > 0 else {
                throw CNAArgumentNullException(
                    paramName: "data",
                    message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                        .nullNotAllowedMessage)
            }
            if checksBinding, nativeStorage.runtime.cachedVertexBufferBindings
                .contains(where: { $0.VertexBuffer === self }) {
                throw CNAInvalidOperationException(
                    message: Microsoft.Xna.Framework.Graphics.Texture.resourceInUseMessage)
            }
            if !isSetting, BufferUsage.contains(.WriteOnly) {
                throw CNANotSupportedException(
                    message: Microsoft.Xna.Framework.Graphics.BufferResources
                        .writeOnlyGetNotSupported)
            }
            try Microsoft.Xna.Framework.Graphics.validateCopyParameters(
                dataLength: arrayCount, dataIndex: startIndex,
                elementCount: elementCount)

            let elementSize = MemoryLayout<T>.size
            var bytes = elementSize * Int(elementCount)
            var slack = 0
            if vertexStride != 0 {
                slack = Int(vertexStride) - elementSize
                guard slack >= 0 else {
                    throw CNAArgumentOutOfRangeException(
                        paramName: "vertexStride",
                        message: Microsoft.Xna.Framework.Graphics.BufferResources
                            .vertexStrideTooSmall)
                }
                if elementCount > 1 { bytes += (Int(elementCount) - 1) * slack }
            }
            guard bytes + Int(offsetInBytes) <= sizeInBytes else {
                throw CNAInvalidOperationException(
                    message: Microsoft.Xna.Framework.Graphics.BufferResources
                        .resourceDataMustBeCorrectSize)
            }

            // Everything above is XNA's. What follows is this binding's limit,
            // measured in `build-probe/f60_stride.c`.
            let declarationStride = Int(storedVertexDeclaration.VertexStride)
            guard slack == 0 else {
                throw CNAError.producerInvariant(
                    "a vertexStride of \(vertexStride) leaves \(slack) unwritten "
                    + "bytes between elements; CNA's raw vertex transfer moves "
                    + "whole vertices at the declaration's own stride and cannot "
                    + "express a strided partial write")
            }
            guard declarationStride > 0,
                  Int(offsetInBytes) % declarationStride == 0,
                  bytes % declarationStride == 0 else {
                throw CNAError.producerInvariant(
                    "a \(bytes)-byte transfer at offset \(offsetInBytes) is not a "
                    + "whole number of \(declarationStride)-byte vertices; CNA's "
                    + "raw vertex transfer cannot express a partial vertex")
            }
            return CopyPlan(
                handle: handle,
                offsetInBytes: Int(offsetInBytes),
                byteOffset: Int(startIndex) * elementSize,
                byteCount: bytes,
                vertexCount: bytes / declarationStride,
                stride: declarationStride)
        }

        /// `protected override void Dispose(bool)`.
        ///
        /// ```text
        /// if (disposing) { try { ~VertexBuffer(); }
        ///                  finally { base.Dispose(true); } }
        /// else           { try { !VertexBuffer(); }
        ///                  finally { base.Dispose(false); } }
        /// ```
        ///
        /// The same shape as `Texture2D`'s: both arms run
        /// `!VertexBuffer()` — release the native buffer if it is not already
        /// released, drop the device-lost recreation copy — and differ only in
        /// the flag they hand the base. This projection's handle is owned by
        /// `GraphicsResource`, which releases it in the same position relative
        /// to `Disposing`, and CNA keeps no recreation copy.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }
    }
}
