// SPDX-License-Identifier: MIT

import CNAShim

internal final class IndexBufferContentLostBox {
    weak var buffer: Microsoft.Xna.Framework.Graphics.DynamicIndexBuffer?
    init(_ buffer: Microsoft.Xna.Framework.Graphics.DynamicIndexBuffer) {
        self.buffer = buffer
    }
}

private let indexBufferContentLostCallback:
    CNASwift_IndexBufferContentLostCallback = { _, context in
    guard let context else { return }
    let box = Unmanaged<IndexBufferContentLostBox>.fromOpaque(context)
        .takeUnretainedValue()
    box.buffer?.nativeContentWasLost()
}

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.DynamicIndexBuffer` projection.
    ///
    /// `DynamicVertexBuffer`'s shape without the stride: two option-taking
    /// `SetData` overloads, a latching `IsContentLost` and a `ContentLost`
    /// event.
    open class DynamicIndexBuffer: IndexBuffer {
        private let contentLostSource = CNAEventSource<CNAEventArgs>()
        private var contentLost = false
        internal private(set) var contentLostRegistration: UInt64 = 0
        private var contentLostBox: Unmanaged<IndexBufferContentLostBox>?

        /// `DynamicIndexBuffer(GraphicsDevice, IndexElementSize, Int32, BufferUsage)`.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            indexElementSize: Microsoft.Xna.Framework.Graphics.IndexElementSize,
            indexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage
        ) throws {
            try self.init(
                graphicsDevice: graphicsDevice, elementSize: indexElementSize,
                indexCount: indexCount, usage: usage, dynamic: true,
                typeName: "DynamicIndexBuffer")
            try subscribeToNativeContentLost()
        }

        /// `DynamicIndexBuffer(GraphicsDevice, Type, Int32, BufferUsage)`.
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
                indexCount: indexCount, usage: usage, dynamic: true,
                typeName: "DynamicIndexBuffer")
            try subscribeToNativeContentLost()
        }

        /// `SetData<T>(T[] data, Int32 startIndex, Int32 elementCount,
        /// SetDataOptions options)`.
        public func SetData<T>(
            _ data: [T], startIndex: Int32, elementCount: Int32,
            options: Microsoft.Xna.Framework.Graphics.SetDataOptions
        ) throws {
            try SetData(0, data: data, startIndex: startIndex,
                        elementCount: elementCount, options: options)
        }

        /// `SetData<T>(Int32 offsetInBytes, T[] data, Int32 startIndex,
        /// Int32 elementCount, SetDataOptions options)`.
        ///
        /// The option's only managed consequence is skipping the bound-buffer
        /// test — see `DynamicVertexBuffer.SetData` — and that half is
        /// reproduced. The hint itself is forwarded where CNA takes it and
        /// dropped where it does not: `build-probe/f60_options.c` measures a
        /// dynamic index buffer accepting `Discard` and `NoOverwrite` through
        /// `cna_index_buffer_set_data`, which replaces the whole buffer, and
        /// **refusing** them through the windowed `cna_index_buffer_set_data_at`.
        /// A windowed write therefore carries `CNA_SET_DATA_NONE`, which changes
        /// no observable behaviour: the flag is a D3D lock hint, and the one
        /// thing XNA lets a caller observe about it is managed and already
        /// reproduced above.
        public func SetData<T>(
            _ offsetInBytes: Int32, data: [T], startIndex: Int32,
            elementCount: Int32,
            options: Microsoft.Xna.Framework.Graphics.SetDataOptions
        ) throws {
            try writeData(offsetInBytes, data: data, startIndex: startIndex,
                          elementCount: elementCount, options: options)
        }

        /// `DynamicIndexBuffer.IsContentLost`, latching exactly as
        /// `DynamicVertexBuffer`'s does and for the same recorded reason.
        public var IsContentLost: Bool { contentLost }

        /// `DynamicIndexBuffer.ContentLost`.
        public var ContentLost: CNAEvent<CNAEventArgs> { contentLostSource.Event }

        internal func nativeContentWasLost() {
            contentLost = true
            do {
                try contentLostSource.Raise(self, args: CNAEventArgs.Empty)
            } catch {
                nativeStorage.runtime.storeCallbackError(error)
            }
        }

        private func subscribeToNativeContentLost() throws {
            let box = Unmanaged.passRetained(IndexBufferContentLostBox(self))
            var registration: UInt64 = 0
            let result = nativeStorage.runtime.functions.indexBufferSubscribeContentLost(
                nativeStorage.handle, indexBufferContentLostCallback,
                box.toOpaque(), &registration)
            guard result == 0 else {
                box.release()
                try nativeStorage.runtime.functions.check(
                    result, operation: "cna_index_buffer_subscribe_content_lost")
                return
            }
            contentLostRegistration = registration
            contentLostBox = box
        }

        private func unsubscribeFromNativeContentLost() {
            guard contentLostRegistration != 0 else { return }
            _ = nativeStorage.runtime.functions.indexBufferUnsubscribeContentLost(
                contentLostRegistration)
            contentLostRegistration = 0
            contentLostBox?.release()
            contentLostBox = nil
        }

        open override func Dispose(_ disposing: Bool) throws {
            guard !IsDisposed else { return }
            unsubscribeFromNativeContentLost()
            try super.Dispose(disposing)
        }

        deinit { unsubscribeFromNativeContentLost() }
    }
}
