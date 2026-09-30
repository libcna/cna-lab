// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.RenderTargetCube` projection.
    ///
    /// `RenderTarget2D` is to `Texture2D` what this is to `TextureCube`, and
    /// the two are deliberately the same shape: one inherited handle with one
    /// owner and one destruction path, the three read-back description
    /// properties, `IsContentLost`, the `ContentLost` event over CNA's own
    /// subscription, and a `Dispose` that releases the subscription before the
    /// base releases the handle.
    ///
    /// **Its description comes from the render-target route.** Under CNA 0.21
    /// `cna_texturecube_get_info` answered a render-target-cube handle with
    /// `CNA_RESULT_SUCCESS` and **size 0, level count 0 and format 0** (SW-07),
    /// so this type reads `cna_render_target_get_info`, which answered the real
    /// size, one level and `Color`. CNA 0.35 answers the cube route correctly
    /// too (re-measured on HEADLESS and OPENGLES3); the render-target route
    /// stays because it also carries the usage, sample count and depth format.
    open class RenderTargetCube: TextureCube {
        /// `RenderTargetCube.RenderTargetUsage`, read once at construction.
        public let RenderTargetUsage: Microsoft.Xna.Framework.Graphics.RenderTargetUsage

        /// `RenderTargetCube.MultiSampleCount`, read once at construction.
        public let MultiSampleCount: Int32

        /// `RenderTargetCube.DepthStencilFormat`, read once at construction.
        public let DepthStencilFormat: DepthFormat

        private let contentLostSource = CNAEventSource<CNAEventArgs>()
        private var contentLost: Bool
        private var contentLostSubscription: RenderTargetContentLostSubscription?

        /// The native subscription handle, `0` when there is none. Internal so
        /// a test can assert that disposal released it.
        internal var contentLostRegistration: UInt64 {
            contentLostSubscription?.registration ?? 0
        }

        /// `RenderTargetCube..ctor(GraphicsDevice, Int32, Boolean, SurfaceFormat, DepthFormat)`.
        ///
        /// The IL pushes two zeros: `preferredMultiSampleCount` 0 and `usage`
        /// `RenderTargetUsage.DiscardContents`.
        public convenience init(
            graphicsDevice: GraphicsDevice, size: Int32, mipMap: Bool,
            preferredFormat: SurfaceFormat, preferredDepthFormat: DepthFormat
        ) throws {
            try self.init(
                graphicsDevice: graphicsDevice, size: size, mipMap: mipMap,
                preferredFormat: preferredFormat,
                preferredDepthFormat: preferredDepthFormat,
                preferredMultiSampleCount: 0, usage: .DiscardContents)
        }

        /// `RenderTargetCube..ctor(GraphicsDevice, Int32, Boolean, SurfaceFormat,
        /// DepthFormat, Int32, RenderTargetUsage)`.
        ///
        /// The designated constructor. `CreateRenderTarget` runs
        /// `TextureCube.ValidateCreationParameters` against the device's own
        /// profile — the same five tests `TextureCube` runs, on the same
        /// extracted table — and only then creates the surfaces, so the profile
        /// refusals happen here too and happen first.
        ///
        /// Everything the target actually got is read back from
        /// `cna_render_target_get_info` rather than echoed from the request,
        /// because CNA is free to grant less than was preferred — which is what
        /// "preferred" means in the XNA parameter names.
        public init(
            graphicsDevice: GraphicsDevice, size: Int32, mipMap: Bool,
            preferredFormat: SurfaceFormat, preferredDepthFormat: DepthFormat,
            preferredMultiSampleCount: Int32,
            usage: Microsoft.Xna.Framework.Graphics.RenderTargetUsage
        ) throws {
            try graphicsDevice.profileCapabilities.validateCubeCreation(
                size: size, format: preferredFormat)
            let deviceHandle = try graphicsDevice.validatedHandle("RenderTargetCube.init")
            let runtime = graphicsDevice.runtimeState

            var create = CNASwift_RenderTargetCubeCreateInfo()
            create.struct_size = UInt32(MemoryLayout<CNASwift_RenderTargetCubeCreateInfo>.size)
            create.struct_version = 1
            create.size = UInt32(bitPattern: size)
            create.mip_map = mipMap ? 1 : 0
            create.format = UInt32(bitPattern: preferredFormat.rawValue)
            create.depth_format = UInt32(bitPattern: preferredDepthFormat.rawValue)
            create.multi_sample_count = preferredMultiSampleCount
            create.usage = UInt32(bitPattern: usage.rawValue)

            var handle: UInt64 = 0
            try runtime.functions.check(
                withUnsafePointer(to: &create) {
                    runtime.functions.renderTargetCubeCreate(deviceHandle, $0, &handle)
                },
                operation: "cna_render_target_cube_create")

            var info = CNASwift_RenderTargetInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_RenderTargetInfo>.size)
            info.struct_version = 1
            do {
                try runtime.functions.check(
                    runtime.functions.renderTargetGetInfo(handle, &info),
                    operation: "cna_render_target_get_info")
                guard info.width <= UInt32(Int32.max),
                      info.level_count <= UInt32(Int32.max) else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTargetCube dimensions", result: 10,
                        message: "size or level count exceeds the XNA Int32 range")
                }
                // A cube's faces are square; CNA reports both extents and they
                // must agree, or `Size` would be one of two different numbers.
                guard info.width == info.height else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTargetCube.Size", result: 1,
                        message: "native cube face is \(info.width)x\(info.height), not square")
                }
                guard let grantedFormat = SurfaceFormat(
                    rawValue: Int32(bitPattern: info.format)) else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTargetCube.Format", result: 1,
                        message: "native surface format \(info.format) is not an XNA SurfaceFormat")
                }
                guard let grantedDepth = DepthFormat(
                    rawValue: Int32(bitPattern: info.depth_format)) else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTargetCube.DepthStencilFormat", result: 1,
                        message: "native depth format \(info.depth_format) is not an XNA DepthFormat")
                }
                guard let grantedUsage = Microsoft.Xna.Framework.Graphics.RenderTargetUsage(
                    rawValue: Int32(bitPattern: info.usage)) else {
                    throw CNAError.nativeFailure(
                        operation: "RenderTargetCube.RenderTargetUsage", result: 1,
                        message: "native usage \(info.usage) is not an XNA RenderTargetUsage")
                }
                RenderTargetUsage = grantedUsage
                MultiSampleCount = info.multi_sample_count
                DepthStencilFormat = grantedDepth
                contentLost = info.is_content_lost != 0
                super.init(
                    handle: handle, runtime: runtime, device: graphicsDevice,
                    typeName: "RenderTargetCube",
                    destroy: runtime.functions.renderTargetDestroy,
                    size: Int32(info.width), levelCount: Int32(info.level_count),
                    format: grantedFormat)
            } catch {
                _ = runtime.functions.renderTargetDestroy(handle)
                throw error
            }
            try subscribeToNativeContentLost()
        }

        /// `RenderTargetCube.IsContentLost`.
        ///
        /// `virtual final` in the metadata — a sealed `IDynamicGraphicsResource`
        /// implementation, not an override point — so `final`, and the CLR
        /// getter is infallible so it does not throw.
        ///
        /// The same measured divergence `RenderTarget2D` records: XNA latches
        /// from `GraphicsDevice.IsDeviceLost` on every read, this latches on
        /// CNA's ContentLost notification and on what the target reported at
        /// creation, and the qualified HEADLESS renderer cannot lose a device.
        public final var IsContentLost: Bool { contentLost }

        /// `RenderTargetCube.ContentLost`.
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

extension Microsoft.Xna.Framework.Graphics.RenderTargetCube: NativeContentLostReceiver {
    internal func nativeContentWasLost() {
        contentWasLost()
    }
}
