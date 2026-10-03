// SPDX-License-Identifier: MIT

import CNAShim

internal final class VertexBufferContentLostBox {
    weak var buffer: Microsoft.Xna.Framework.Graphics.DynamicVertexBuffer?
    init(_ buffer: Microsoft.Xna.Framework.Graphics.DynamicVertexBuffer) {
        self.buffer = buffer
    }
}

private let vertexBufferContentLostCallback:
    CNASwift_VertexBufferContentLostCallback = { _, context in
    guard let context else { return }
    let box = Unmanaged<VertexBufferContentLostBox>.fromOpaque(context)
        .takeUnretainedValue()
    box.buffer?.nativeContentWasLost()
}

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.DynamicVertexBuffer` projection.
    ///
    /// A `VertexBuffer` with two extra `SetData` overloads that take
    /// `SetDataOptions`, a latching `IsContentLost`, and a `ContentLost` event.
    /// Its constructors are `VertexBuffer`'s with `dynamic` true; XNA's set a
    /// different D3D usage and pool and are otherwise the same body, including
    /// the same `vertexCount` and `vertexDeclaration` checks in the same order.
    open class DynamicVertexBuffer: VertexBuffer {
        private let contentLostSource = CNAEventSource<CNAEventArgs>()
        private var contentLost = false
        /// The native subscription handle, `0` when there is none.
        ///
        /// Internal rather than private so a test can assert that disposal
        /// released it: a subscription that outlived its target would leave a
        /// native callback addressing a released Swift box.
        internal private(set) var contentLostRegistration: UInt64 = 0
        private var contentLostBox: Unmanaged<VertexBufferContentLostBox>?

        /// `DynamicVertexBuffer(GraphicsDevice, VertexDeclaration, Int32, BufferUsage)`.
        public convenience init(
            graphicsDevice: GraphicsDevice,
            vertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration,
            vertexCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.BufferUsage
        ) throws {
            try self.init(
                graphicsDevice: graphicsDevice, declaration: vertexDeclaration,
                vertexCount: vertexCount, usage: usage, dynamic: true,
                typeName: "DynamicVertexBuffer")
            try subscribeToNativeContentLost()
        }

        /// `DynamicVertexBuffer(GraphicsDevice, Type, Int32, BufferUsage)`.
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
                vertexCount: vertexCount, usage: usage, dynamic: true,
                typeName: "DynamicVertexBuffer")
            try subscribeToNativeContentLost()
        }

        /// `SetData<T>(T[] data, Int32 startIndex, Int32 elementCount,
        /// SetDataOptions options)` — `SetData(0, data, startIndex,
        /// elementCount, 0, options)`.
        public func SetData<T>(
            _ data: [T], startIndex: Int32, elementCount: Int32,
            options: Microsoft.Xna.Framework.Graphics.SetDataOptions
        ) throws {
            try SetData(0, data: data, startIndex: startIndex,
                        elementCount: elementCount, vertexStride: 0,
                        options: options)
        }

        /// `SetData<T>(Int32 offsetInBytes, T[] data, Int32 startIndex,
        /// Int32 elementCount, Int32 vertexStride, SetDataOptions options)`.
        ///
        /// The body is `CopyData(..., ConvertXnaSetDataOptionsToDx(options),
        /// isSetting: true)` — the same `CopyData` the base's overloads reach,
        /// with the option carried into it.
        ///
        /// **What the option does, and what it does not.** Inside `CopyData`
        /// its only *managed* consequence is that `Discard` or `NoOverwrite`
        /// skips the "is this buffer currently bound to the device" test that
        /// otherwise raises `ResourceInUse`; everything else it does is a D3D
        /// lock flag, a driver hint with no defined observable effect. The
        /// managed half is reproduced. The hint is forwarded through
        /// `cna_vertex_buffer_set_data_raw_at_with_options`, which
        /// `build-probe/f60_options.c` measures accepting every option value on
        /// a dynamic buffer and refusing all three -- `CNA_SET_DATA_NONE`
        /// included -- on a static one.
        public func SetData<T>(
            _ offsetInBytes: Int32, data: [T], startIndex: Int32,
            elementCount: Int32, vertexStride: Int32,
            options: Microsoft.Xna.Framework.Graphics.SetDataOptions
        ) throws {
            try writeData(offsetInBytes, data: data, startIndex: startIndex,
                          elementCount: elementCount, vertexStride: vertexStride,
                          options: options)
        }

        /// `DynamicVertexBuffer.IsContentLost`.
        ///
        /// ```text
        /// if (!_contentLost) _contentLost = _parent.IsDeviceLost;
        /// return _contentLost;
        /// ```
        ///
        /// The latch from the device has no counterpart:
        /// `GraphicsDevice.IsDeviceLost` is not projected, and HEADLESS is one
        /// of the renderer families CNA documents as unable to lose a device,
        /// so the native `ContentLost` callback never fires here either. The
        /// flag is therefore always false on this host — the same recorded
        /// divergence `RenderTarget2D.IsContentLost` carries.
        public var IsContentLost: Bool { contentLost }

        /// `DynamicVertexBuffer.ContentLost`.
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
            let box = Unmanaged.passRetained(VertexBufferContentLostBox(self))
            var registration: UInt64 = 0
            let result = nativeStorage.runtime.functions.vertexBufferSubscribeContentLost(
                nativeStorage.handle, vertexBufferContentLostCallback,
                box.toOpaque(), &registration)
            guard result == 0 else {
                box.release()
                try nativeStorage.runtime.functions.check(
                    result, operation: "cna_vertex_buffer_subscribe_content_lost")
                return
            }
            contentLostRegistration = registration
            contentLostBox = box
        }

        private func unsubscribeFromNativeContentLost() {
            guard contentLostRegistration != 0 else { return }
            _ = nativeStorage.runtime.functions.vertexBufferUnsubscribeContentLost(
                contentLostRegistration)
            contentLostRegistration = 0
            contentLostBox?.release()
            contentLostBox = nil
        }

        /// The subscription is released before the buffer is, so no native
        /// callback can address a released Swift box.
        open override func Dispose(_ disposing: Bool) throws {
            guard !IsDisposed else { return }
            unsubscribeFromNativeContentLost()
            try super.Dispose(disposing)
        }

        deinit { unsubscribeFromNativeContentLost() }
    }
}
