// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.RenderTarget2D` projection.
    ///
    /// The type that made `Texture2D`'s `final` a real problem rather than a
    /// formal one: XNA derives this class from `Texture2D`, so a sealed
    /// `Texture2D` made it inexpressible. It is a genuine Swift subclass, and
    /// it inherits **one** handle with one owner and one destruction path — no
    /// second storage, no second `Dispose`, no downcast.
    ///
    /// CNA agrees with that inheritance at the C boundary, which is measured
    /// rather than assumed: `cna_texture2d_get_info` and `cna_texture_get_info`
    /// both accept a render-target handle and report the same dimensions,
    /// level count and format, and `cna_sprite_batch_submit_scaled_many`
    /// accepts one as its `texture`. Substitutability is native here, not
    /// simulated by the binding.
    ///
    /// The destroy route is the render-target one. `cna_texture2d_destroy` was
    /// measured to release a live render target too, so the choice is about
    /// using the documented route rather than about avoiding a leak.
    open class RenderTarget2D: Texture2D {
        /// `RenderTarget2D.RenderTargetUsage`, read once at construction.
        public let RenderTargetUsage: Microsoft.Xna.Framework.Graphics.RenderTargetUsage

        /// `RenderTarget2D.MultiSampleCount`, read once at construction.
        public let MultiSampleCount: Int32

        /// `RenderTarget2D.DepthStencilFormat`, read once at construction.
        public let DepthStencilFormat: DepthFormat

        private let contentLostSource = CNAEventSource<CNAEventArgs>()
        private var contentLost: Bool
        private var contentLostSubscription: RenderTargetContentLostSubscription?
        /// The native subscription handle, `0` when there is none.
        ///
        /// Internal rather than private so a test can assert that disposal
        /// released it: a subscription that outlived its target would leave a
        /// native callback addressing a released Swift box.
        internal var contentLostRegistration: UInt64 {
            contentLostSubscription?.registration ?? 0
        }

        /// `RenderTarget2D..ctor(GraphicsDevice, Int32, Int32)`.
        ///
        /// The IL pushes five zeros: `mipMap` false, `preferredFormat`
        /// `SurfaceFormat.Color`, `preferredDepthFormat` `DepthFormat.None`,
        /// `preferredMultiSampleCount` 0 and `usage`
        /// `RenderTargetUsage.DiscardContents`. Those are the defaults, and
        /// they are read out of the IL rather than assumed.
        public convenience init(
            graphicsDevice: GraphicsDevice, width: Int32, height: Int32
        ) throws {
            try self.init(
                graphicsDevice: graphicsDevice, width: width, height: height,
                mipMap: false, preferredFormat: .Color, preferredDepthFormat: .None,
                preferredMultiSampleCount: 0, usage: .DiscardContents)
        }

        /// `RenderTarget2D..ctor(GraphicsDevice, Int32, Int32, Boolean, SurfaceFormat, DepthFormat)`.
        ///
        /// The IL pushes two zeros: `preferredMultiSampleCount` 0 and `usage`
        /// `RenderTargetUsage.DiscardContents`.
        public convenience init(
            graphicsDevice: GraphicsDevice, width: Int32, height: Int32,
            mipMap: Bool, preferredFormat: SurfaceFormat,
            preferredDepthFormat: DepthFormat
        ) throws {
            try self.init(
                graphicsDevice: graphicsDevice, width: width, height: height,
                mipMap: mipMap, preferredFormat: preferredFormat,
                preferredDepthFormat: preferredDepthFormat,
                preferredMultiSampleCount: 0, usage: .DiscardContents)
        }

        /// `RenderTarget2D..ctor(GraphicsDevice, Int32, Int32, Boolean, SurfaceFormat, DepthFormat, Int32, RenderTargetUsage)`.
        ///
        /// The designated constructor. Every dimension and format the target
        /// actually got is read back from `cna_render_target_get_info` rather
        /// than echoed from the request, because CNA is free to grant less
        /// than was preferred — which is exactly what "preferred" means in the
        /// XNA parameter names.
        public init(
            graphicsDevice: GraphicsDevice, width: Int32, height: Int32,
            mipMap: Bool, preferredFormat: SurfaceFormat,
            preferredDepthFormat: DepthFormat, preferredMultiSampleCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.RenderTargetUsage
        ) throws {
            let deviceHandle = try graphicsDevice.validatedHandle("RenderTarget2D.init")
            let runtime = graphicsDevice.runtimeState

            var create = CNASwift_RenderTarget2DCreateInfo()
            create.struct_size = UInt32(MemoryLayout<CNASwift_RenderTarget2DCreateInfo>.size)
            create.struct_version = 1
            create.width = UInt32(bitPattern: width)
            create.height = UInt32(bitPattern: height)
            create.mip_map = mipMap ? 1 : 0
            create.format = UInt32(bitPattern: preferredFormat.rawValue)
            create.depth_format = UInt32(bitPattern: preferredDepthFormat.rawValue)
            create.multi_sample_count = preferredMultiSampleCount
            create.usage = UInt32(bitPattern: usage.rawValue)

            var handle: UInt64 = 0
            try runtime.functions.check(
                withUnsafePointer(to: &create) {
                    runtime.functions.renderTarget2DCreate(deviceHandle, $0, &handle)
                },
                operation: "cna_render_target2d_create"
            )

            var info = CNASwift_RenderTargetInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_RenderTargetInfo>.size)
            info.struct_version = 1
            do {
                try runtime.functions.check(
                    runtime.functions.renderTargetGetInfo(handle, &info),
                    operation: "cna_render_target_get_info"
                )
                guard info.width <= UInt32(Int32.max), info.height <= UInt32(Int32.max),
                      info.level_count <= UInt32(Int32.max) else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTarget2D dimensions", result: 10,
                        message: "dimensions exceed the XNA Int32 range")
                }
                guard let grantedFormat = SurfaceFormat(
                    rawValue: Int32(bitPattern: info.format)) else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTarget2D.Format", result: 1,
                        message: "native surface format \(info.format) is not an XNA SurfaceFormat")
                }
                guard let grantedDepth = DepthFormat(
                    rawValue: Int32(bitPattern: info.depth_format)) else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTarget2D.DepthStencilFormat", result: 1,
                        message: "native depth format \(info.depth_format) is not an XNA DepthFormat")
                }
                guard let grantedUsage = Microsoft.Xna.Framework.Graphics.RenderTargetUsage(
                    rawValue: Int32(bitPattern: info.usage)) else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTarget2D.RenderTargetUsage", result: 1,
                        message: "native usage \(info.usage) is not an XNA RenderTargetUsage")
                }
                RenderTargetUsage = grantedUsage
                MultiSampleCount = info.multi_sample_count
                DepthStencilFormat = grantedDepth
                contentLost = info.is_content_lost != 0
                super.init(
                    handle: handle,
                    runtime: runtime,
                    device: graphicsDevice,
                    typeName: "RenderTarget2D",
                    destroy: runtime.functions.renderTargetDestroy,
                    width: Int32(info.width),
                    height: Int32(info.height),
                    levelCount: Int32(info.level_count),
                    format: grantedFormat
                )
            } catch {
                _ = runtime.functions.renderTargetDestroy(handle)
                throw error
            }
            try subscribeToNativeContentLost()
        }

        /// `RenderTarget2D.IsContentLost`.
        ///
        /// `virtual final` in the metadata — a sealed `IDynamicGraphicsResource`
        /// implementation, not an override point — so `final`, and the CLR
        /// getter is infallible so it does not throw.
        ///
        /// **A measured divergence.** XNA latches its `_contentLost` field
        /// from `GraphicsDevice.IsDeviceLost` on every read. There is no
        /// `IsDeviceLost` here, and CNA reports content loss by notification
        /// instead, so this latches on CNA's own ContentLost callback and on
        /// the state the target reported when it was created. On the qualified
        /// HEADLESS renderer the value is always false, because that renderer
        /// cannot lose a device.
        public final var IsContentLost: Bool { contentLost }

        /// `RenderTarget2D.ContentLost`.
        ///
        /// Raised from CNA's own subscription with the target as sender and
        /// `EventArgs.Empty`, which is the shape every XNA
        /// `EventHandler<EventArgs>` raise site uses.
        public var ContentLost: CNAEvent<CNAEventArgs> { contentLostSource.Event }

        /// Latches the flag and raises the event, from CNA's own notification.
        internal func contentWasLost() {
            contentLost = true
            do {
                try contentLostSource.Raise(self, args: CNAEventArgs.Empty)
            } catch {
                nativeStorage.runtime.storeCallbackError(error)
            }
        }

        /// Releases the native subscription before the base releases the
        /// handle, so no native callback can address freed Swift state.
        open override func Dispose(_ disposing: Bool) throws {
            guard !IsDisposed else { return }
            contentLostSubscription?.release()
            try super.Dispose(disposing)
        }

        private func subscribeToNativeContentLost() throws {
            let subscription = RenderTargetContentLostSubscription(
                runtime: nativeStorage.runtime)
            try subscription.subscribe(handle: nativeStorage.handle, receiver: self)
            contentLostSubscription = subscription
        }

        deinit { contentLostSubscription?.release() }
    }
}

extension Microsoft.Xna.Framework.Graphics.RenderTarget2D: NativeContentLostReceiver {
    internal func nativeContentWasLost() {
        contentWasLost()
    }
}
