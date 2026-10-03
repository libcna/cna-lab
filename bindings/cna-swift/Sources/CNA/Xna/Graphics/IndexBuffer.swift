// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.IndexBuffer` projection.
    ///
    /// The same shape as `VertexBuffer` without the stride: XNA's index
    /// `CopyData` has no `vertexStride` parameter, so its byte arithmetic is
    /// `sizeof(T) * elementCount` and nothing else.
    open class IndexBuffer: GraphicsResource {
        /// `IndexBuffer.IndexCount`.
        public let IndexCount: Int32

        /// `IndexBuffer.IndexElementSize`.
        public let IndexElementSize: Microsoft.Xna.Framework.Graphics.IndexElementSize

        /// `IndexBuffer.BufferUsage`.
        public let BufferUsage: Microsoft.Xna.Framework.Graphics.BufferUsage

        /// `_bufferSize`, the byte length every transfer is compared against.
        internal let sizeInBytes: Int

        /// One index in bytes.
        ///
        /// `IL_0022: ldarg.2; ldc.i4.0; beq IL_0028` — the constructor pushes 2
        /// for the zero-valued `SixteenBits` and 4 for the other, which is the
        /// whole of the conversion.
        internal static func byteWidth(
            of size: Microsoft.Xna.Framework.Graphics.IndexElementSize
        ) -> Int {
            size == .SixteenBits ? 2 : 4
        }

        internal var elementSizeInBytes: Int {
            IndexBuffer.byteWidth(of: IndexElementSize)
        }

        internal init(
            handle: UInt64,
            runtime: RuntimeState,
            device: GraphicsDevice,
            typeName: String,
            indexCount: Int32,
            elementSize: Microsoft.Xna.Framework.Graphics.IndexElementSize,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage
        ) {
            IndexCount = indexCount
            IndexElementSize = elementSize
            BufferUsage = usage
            sizeInBytes = Int(indexCount) * IndexBuffer.byteWidth(of: elementSize)
            super.init(
                storage: NativeHandleStorage(
                    handle: handle,
                    typeName: typeName,
                    ownership: .owned,
                    runtime: runtime,
                    destroy: runtime.functions.indexBufferDestroy),
                device: device)
            runtime.register(self)
        }

        /// `IndexBuffer(GraphicsDevice, IndexElementSize, Int32, BufferUsage)`.
        ///
        /// ```text
        /// if (indexCount <= 0)
        ///     throw new ArgumentOutOfRangeException(
        ///         "indexCount", ResourcesMustBeGreaterThanZeroSize);
        /// _parent = graphicsDevice;
        /// CreateBuffer(indexElementSize == SixteenBits ? 2 : 4, indexCount, ...);
        /// ```
        public convenience init(
            graphicsDevice: GraphicsDevice,
            indexElementSize: Microsoft.Xna.Framework.Graphics.IndexElementSize,
            indexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage
        ) throws {
            try self.init(
                graphicsDevice: graphicsDevice, elementSize: indexElementSize,
                indexCount: indexCount, usage: usage, dynamic: false,
                typeName: "IndexBuffer")
        }

        /// `IndexBuffer(GraphicsDevice, Type, Int32, BufferUsage)`.
        ///
        /// The count is checked first, then `Marshal.SizeOf(indexType)` decides
        /// the width; `CreateBuffer` refuses anything that is neither 2 nor 4
        /// with `IndexBuffersMustBeSizedCorrectly`.
        ///
        /// Swift has no `Marshal.SizeOf` for a metatype — `MemoryLayout<T>` needs
        /// a static `T` — so the four CLR index widths are recognised by
        /// identity and anything else is refused. See
        /// `IndexBuffer.byteWidth(ofIndexType:)`.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            indexType: Any.Type,
            indexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage
        ) throws {
            guard indexCount > 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "indexCount",
                    message: Microsoft.Xna.Framework.Graphics.Texture2D
                        .resourcesMustBeGreaterThanZeroSizeMessage)
            }
            let width = try IndexBuffer.byteWidth(ofIndexType: indexType)
            try self.init(
                graphicsDevice: graphicsDevice,
                elementSize: width == 2 ? .SixteenBits : .ThirtyTwoBits,
                indexCount: indexCount, usage: usage, dynamic: false,
                typeName: "IndexBuffer")
        }

        /// `Marshal.SizeOf(indexType)`, then `CreateBuffer`'s width test.
        ///
        /// ```text
        /// if (size != 2 && size != 4)
        ///     throw new ArgumentException(IndexBuffersMustBeSizedCorrectly);
        /// ```
        ///
        /// A Swift metatype carries no size a caller can ask for, so the two
        /// widths XNA accepts are recognised by the CLR primitive types that
        /// have them. A consumer's own two- or four-byte struct is accepted by
        /// XNA and refused here; the `IndexElementSize` overload beside this
        /// one has no such limit and is the one to use. Recorded as a language
        /// mapping limitation rather than a native one.
        internal static func byteWidth(ofIndexType type: Any.Type) throws -> Int {
            switch ObjectIdentifier(type) {
            case ObjectIdentifier(Int16.self), ObjectIdentifier(UInt16.self):
                return 2
            case ObjectIdentifier(Int32.self), ObjectIdentifier(UInt32.self):
                return 4
            default:
                throw CNAArgumentException(
                    message: Microsoft.Xna.Framework.Graphics.BufferResources
                        .indexBuffersMustBeSizedCorrectly)
            }
        }

        /// The one creation path, shared with `DynamicIndexBuffer`.
        internal convenience init(
            graphicsDevice: GraphicsDevice,
            elementSize: Microsoft.Xna.Framework.Graphics.IndexElementSize,
            indexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage,
            dynamic: Bool,
            typeName: String
        ) throws {
            guard indexCount > 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "indexCount",
                    message: Microsoft.Xna.Framework.Graphics.Texture2D
                        .resourcesMustBeGreaterThanZeroSizeMessage)
            }
            // `CreateBuffer`'s two profile checks, in the IL's order: the
            // 32-bit index refusal comes before the size comparison.
            let width = IndexBuffer.byteWidth(of: elementSize)
            try graphicsDevice.profileCapabilities.validateIndexBuffer(
                elementSizeInBytes: width, size: Int(indexCount) * width)
            let deviceHandle = try graphicsDevice.validatedHandle("IndexBuffer.init")
            let runtime = graphicsDevice.runtimeState

            var create = CNASwift_IndexBufferCreateInfo()
            create.struct_size = UInt32(MemoryLayout<CNASwift_IndexBufferCreateInfo>.size)
            create.struct_version = 1
            create.index_count = indexCount
            create.index_element_size = UInt32(bitPattern: elementSize.rawValue)
            create.buffer_usage = UInt32(bitPattern: usage.rawValue)
            create.dynamic = dynamic ? 1 : 0

            var handle: UInt64 = 0
            try runtime.functions.check(
                withUnsafePointer(to: &create) {
                    runtime.functions.indexBufferCreate(deviceHandle, $0, &handle)
                },
                operation: "cna_index_buffer_create")
            self.init(
                handle: handle, runtime: runtime, device: graphicsDevice,
                typeName: typeName, indexCount: indexCount,
                elementSize: elementSize, usage: usage)
        }

        // ------------------------------------------------------------------
        // SetData and GetData.

        /// `SetData<T>(T[] data)` — `SetData(0, data, 0, data.Length)`.
        public func SetData<T>(_ data: [T]) throws {
            try SetData(0, data: data, startIndex: 0,
                        elementCount: Int32(data.count))
        }

        /// `SetData<T>(T[] data, Int32 startIndex, Int32 elementCount)`.
        public func SetData<T>(
            _ data: [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try SetData(0, data: data, startIndex: startIndex,
                        elementCount: elementCount)
        }

        /// `SetData<T>(Int32 offsetInBytes, T[] data, Int32 startIndex,
        /// Int32 elementCount)`.
        ///
        /// `cna_index_buffer_set_data_at` accepts `CNA_SET_DATA_NONE` on a
        /// static buffer and refuses `Discard` and `NoOverwrite`, measured in
        /// `build-probe/f60_options.c`. Options reach this family only through
        /// `DynamicIndexBuffer`.
        public func SetData<T>(
            _ offsetInBytes: Int32, data: [T], startIndex: Int32,
            elementCount: Int32
        ) throws {
            try writeData(offsetInBytes, data: data, startIndex: startIndex,
                          elementCount: elementCount, options: .None)
        }

        /// The one upload path, shared with `DynamicIndexBuffer`.
        ///
        /// `build-probe/f60_options.c`: a dynamic index buffer accepts
        /// `Discard` and `NoOverwrite` through `cna_index_buffer_set_data`,
        /// which replaces the whole buffer, and **refuses** them through the
        /// windowed `cna_index_buffer_set_data_at`. So an option rides the
        /// whole-buffer route when the write covers the whole buffer, and is
        /// dropped otherwise — which changes nothing observable, because the
        /// flag is a D3D lock hint and the one consequence XNA lets a caller
        /// see is the bound-buffer test, which is managed and reproduced above.
        internal func writeData<T>(
            _ offsetInBytes: Int32, data: [T], startIndex: Int32,
            elementCount: Int32,
            options: Microsoft.Xna.Framework.Graphics.SetDataOptions
        ) throws {
            let plan = try copyPlan(
                T.self, offsetInBytes: offsetInBytes, arrayCount: data.count,
                startIndex: startIndex, elementCount: elementCount,
                isSetting: true, checksBinding: options == .None)
            let functions = nativeStorage.runtime.functions
            let wholeBuffer = plan.offsetInBytes == 0
                && plan.nativeElementCount == Int(IndexCount)
            let forwarded = (options != .None && wholeBuffer)
                ? options
                : Microsoft.Xna.Framework.Graphics.SetDataOptions.None
            var transfer = CNASwift_IndexBufferTransfer()
            transfer.struct_size = UInt32(MemoryLayout<CNASwift_IndexBufferTransfer>.size)
            transfer.struct_version = 1
            transfer.index_element_size = UInt32(bitPattern: IndexElementSize.rawValue)
            transfer.options = UInt32(bitPattern: forwarded.rawValue)
            transfer.start_index = 0
            transfer.element_count = UInt64(plan.nativeElementCount)
            try data.withUnsafeBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                let source = base + plan.byteOffset
                if forwarded == .None {
                    try functions.check(
                        withUnsafePointer(to: transfer) { pointer in
                            functions.indexBufferSetDataAt(
                                plan.handle, UInt64(plan.offsetInBytes), pointer,
                                source, UInt64(plan.nativeElementCount))
                        },
                        operation: "cna_index_buffer_set_data_at")
                } else {
                    try functions.check(
                        withUnsafePointer(to: transfer) { pointer in
                            functions.indexBufferSetData(
                                plan.handle, pointer, source,
                                UInt64(plan.nativeElementCount))
                        },
                        operation: "cna_index_buffer_set_data")
                }
            }
        }

        /// `GetData<T>(T[] data)`.
        public func GetData<T>(_ data: inout [T]) throws {
            try GetData(0, data: &data, startIndex: 0,
                        elementCount: Int32(data.count))
        }

        /// `GetData<T>(T[] data, Int32 startIndex, Int32 elementCount)`.
        public func GetData<T>(
            _ data: inout [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try GetData(0, data: &data, startIndex: startIndex,
                        elementCount: elementCount)
        }

        /// `GetData<T>(Int32 offsetInBytes, T[] data, Int32 startIndex,
        /// Int32 elementCount)`.
        ///
        /// CNA's read begins at the buffer's own index zero — its header says
        /// so — so a non-zero `offsetInBytes` has no native counterpart on the
        /// read side and is refused rather than silently ignored.
        public func GetData<T>(
            _ offsetInBytes: Int32, data: inout [T], startIndex: Int32,
            elementCount: Int32
        ) throws {
            let plan = try copyPlan(
                T.self, offsetInBytes: offsetInBytes, arrayCount: data.count,
                startIndex: startIndex, elementCount: elementCount,
                isSetting: false)
            guard plan.offsetInBytes == 0 else {
                throw CNAError.producerInvariant(
                    "cna_index_buffer_get_data reads from the buffer's first "
                    + "index; a non-zero offsetInBytes has no native counterpart")
            }
            let functions = nativeStorage.runtime.functions
            var transfer = CNASwift_IndexBufferTransfer()
            transfer.struct_size = UInt32(MemoryLayout<CNASwift_IndexBufferTransfer>.size)
            transfer.struct_version = 1
            transfer.index_element_size = UInt32(bitPattern: IndexElementSize.rawValue)
            transfer.options = 0
            transfer.start_index = 0
            transfer.element_count = UInt64(plan.nativeElementCount)
            try data.withUnsafeMutableBytes { bytes in
                guard let base = bytes.baseAddress else { return }
                var required: UInt64 = 0
                try functions.check(
                    withUnsafePointer(to: transfer) { pointer in
                        functions.indexBufferGetData(
                            plan.handle, pointer, base + plan.byteOffset,
                            UInt64(plan.nativeElementCount), &required)
                    },
                    operation: "cna_index_buffer_get_data")
            }
        }

        internal struct CopyPlan {
            var handle: UInt64
            var offsetInBytes: Int
            var byteOffset: Int
            var nativeElementCount: Int
        }

        /// `IndexBuffer.CopyData<T>`, which is `VertexBuffer`'s without the
        /// stride:
        ///
        /// ```text
        /// Helpers.CheckDisposed(this, pComPtr);
        /// if (data == null || data.Length == 0)
        ///     throw new ArgumentNullException("data", NullNotAllowed);
        /// if ((options & (Discard | NoOverwrite)) == 0 && device.Indices == this)
        ///     throw GetExceptionFromResult(E_ABORT);   // ResourceInUse
        /// if (!isSetting && (usage & WriteOnly) == WriteOnly)
        ///     throw new NotSupportedException(WriteOnlyGetNotSupported);
        /// Helpers.ValidateCopyParameters(data.Length, startIndex, elementCount);
        /// int bytes = sizeof(T) * elementCount;
        /// if (bytes + offsetInBytes > _bufferSize)
        ///     throw new InvalidOperationException(ResourceDataMustBeCorrectSize);
        /// ```
        ///
        /// CNA counts in indices of the buffer's own width where XNA counts in
        /// bytes of `T`, so the window converts through bytes — the same
        /// reinterpretation XNA performs implicitly, and the same conversion
        /// `Texture2D`'s transfer already makes. A window that is not a whole
        /// number of indices, or that starts mid-index, is refused rather than
        /// rounded.
        ///
        /// The bound-buffer test reads the managed `Indices` cache, and only a
        /// plain `SetData` makes it (see `VertexBuffer.copyPlan`).
        internal func copyPlan<T>(
            _ element: T.Type, offsetInBytes: Int32, arrayCount: Int,
            startIndex: Int32, elementCount: Int32, isSetting: Bool,
            checksBinding: Bool = false
        ) throws -> CopyPlan {
            let handle = try validatedHandle(
                isSetting ? "IndexBuffer.SetData" : "IndexBuffer.GetData")
            guard arrayCount > 0 else {
                throw CNAArgumentNullException(
                    paramName: "data",
                    message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                        .nullNotAllowedMessage)
            }
            if checksBinding, nativeStorage.runtime.cachedIndexBuffer === self {
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
            let bytes = elementSize * Int(elementCount)
            guard bytes + Int(offsetInBytes) <= sizeInBytes else {
                throw CNAInvalidOperationException(
                    message: Microsoft.Xna.Framework.Graphics.BufferResources
                        .resourceDataMustBeCorrectSize)
            }

            let width = elementSizeInBytes
            let byteOffset = Int(startIndex) * elementSize
            guard bytes % width == 0, Int(offsetInBytes) % width == 0,
                  byteOffset % width == 0 else {
                throw CNAError.producerInvariant(
                    "a \(bytes)-byte window at offset \(offsetInBytes) is not a "
                    + "whole number of \(width)-byte indices; CNA's index "
                    + "transfer is counted in indices and cannot express a "
                    + "partial one")
            }
            return CopyPlan(
                handle: handle, offsetInBytes: Int(offsetInBytes),
                byteOffset: byteOffset, nativeElementCount: bytes / width)
        }

        /// `protected override void Dispose(bool)` — the same shape as
        /// `VertexBuffer`'s, and for the same reasons.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }
    }
}
