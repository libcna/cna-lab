// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {

    /// `Microsoft.Xna.Framework.Graphics.GraphicsAdapter`.
    ///
    /// **A snapshot, and the pinned fallibility table decides that.** All
    /// fifteen accessors are `IL_NO_FAILURE_PATH`, so not one Swift getter here
    /// may throw, while every `cna_graphics_adapter_*` route needs a graphics
    /// device handle that is only real inside a lifecycle callback. The values
    /// are therefore read once and every property afterwards reads a field.
    ///
    /// ## When the list is filled, and why there is no `Refresh`
    ///
    /// XNA fills `pAdapterList` in its **class constructor**, which calls
    /// `InitalizeGraphics` and then `InitializeAdapterList` — so the machine is
    /// enumerated the first time anything touches the type, with no game in
    /// sight.
    ///
    /// This projection cannot enumerate without a device, so the closest
    /// faithful moment is the first one where a device exists:
    /// `GraphicsDevice.borrow` fills the list once per runtime generation. That
    /// is deliberately **not** a public member — XNA has no `Refresh` and this
    /// binding may not invent one.
    ///
    /// The consequence is stated rather than hidden: **before a game has a
    /// device, `Adapters` is nil**, where XNA's is already populated.
    public final class GraphicsAdapter {

        // MARK: - The static half

        private static var storedAdapters:
            CNAReadOnlyCollection<GraphicsAdapter>?
        private static var populatedGeneration: UInt64?
        private static var storedUseNullDevice = false
        private static var storedUseReferenceDevice = false
        private static var devicePreferencesNeedPush = false

        /// `GraphicsAdapter.Adapters`, XNA's `ldsfld pAdapterList`.
        ///
        /// Optional because the CIL proves the field nullable: it is a static
        /// reference that reads null until the class constructor has run. Here
        /// it reads nil until a device has existed.
        public static var Adapters: CNAReadOnlyCollection<GraphicsAdapter>? {
            storedAdapters
        }

        /// `GraphicsAdapter.DefaultAdapter`, XNA's `pAdapterList[0]`.
        ///
        /// **Non-Optional, and it traps when there is nothing to return.**
        /// Both halves of that are forced. The return's nullability is not
        /// proven from the registered CIL, so the projection stays
        /// non-Optional; and the accessor is infallible, so it cannot report
        /// the empty case as an error.
        ///
        /// XNA does exactly the same thing. `get_DefaultAdapter` is twelve
        /// bytes — `ldsfld pAdapterList; ldc.i4.0; get_Item` — with no test of
        /// its own, so a null list raises `NullReferenceException` and an empty
        /// one `ArgumentOutOfRangeException`, both from the runtime rather than
        /// from any instruction in the method. That is precisely why the
        /// fallibility table calls it `IL_NO_FAILURE_PATH`.
        ///
        /// A trap is the honest Swift analogue of an unhandled CLR exception,
        /// and the message names the cause a consumer can actually act on.
        public static var DefaultAdapter: GraphicsAdapter {
            guard let list = storedAdapters, list.Count > 0,
                  let first = try? list.Item(0) else {
                preconditionFailure(
                    "GraphicsAdapter.DefaultAdapter was read before any "
                    + "graphics device existed, so no adapter has been "
                    + "enumerated. XNA fills its adapter list in a class "
                    + "constructor; this binding can only fill it once a "
                    + "device is available, because every adapter route needs "
                    + "one. Read it from inside a Game callback.")
            }
            return first
        }

        /// `GraphicsAdapter.UseNullDevice` and `UseReferenceDevice`.
        ///
        /// Both accessors of both are infallible — they read and write a static
        /// backing field, and `QueryBackBufferFormat` reads them to pick a D3D
        /// device type. Stored values here, pushed to
        /// `cna_graphics_adapter_set_device_preferences` at the next throwing
        /// moment, which is the enumeration itself. Same deferred-write
        /// architecture as `Mouse.WindowHandle`.
        public static var UseNullDevice: Bool {
            get { storedUseNullDevice }
            set {
                storedUseNullDevice = newValue
                devicePreferencesNeedPush = true
            }
        }

        public static var UseReferenceDevice: Bool {
            get { storedUseReferenceDevice }
            set {
                storedUseReferenceDevice = newValue
                devicePreferencesNeedPush = true
            }
        }

        // MARK: - The snapshot

        internal let adapterIndex: UInt32
        private let storedDeviceName: String
        private let storedDescription: String
        private let storedVendorId: Int32
        private let storedDeviceId: Int32
        private let storedRevision: Int32
        private let storedSubSystemId: Int32
        private let storedIsDefaultAdapter: Bool
        private let storedIsWideScreen: Bool
        private let storedCurrentDisplayMode:
            Microsoft.Xna.Framework.Graphics.DisplayMode?
        private let storedSupportedDisplayModes:
            Microsoft.Xna.Framework.Graphics.DisplayModeCollection?
        private let storedProfileSupport: [Int32: Bool]

        internal init(
            adapterIndex: UInt32, deviceName: String, description: String,
            vendorId: Int32, deviceId: Int32, revision: Int32,
            subSystemId: Int32, isDefaultAdapter: Bool, isWideScreen: Bool,
            currentDisplayMode: Microsoft.Xna.Framework.Graphics.DisplayMode?,
            supportedDisplayModes:
                Microsoft.Xna.Framework.Graphics.DisplayModeCollection?,
            profileSupport: [Int32: Bool]
        ) {
            self.adapterIndex = adapterIndex
            storedDeviceName = deviceName
            storedDescription = description
            storedVendorId = vendorId
            storedDeviceId = deviceId
            storedRevision = revision
            storedSubSystemId = subSystemId
            storedIsDefaultAdapter = isDefaultAdapter
            storedIsWideScreen = isWideScreen
            storedCurrentDisplayMode = currentDisplayMode
            storedSupportedDisplayModes = supportedDisplayModes
            storedProfileSupport = profileSupport
        }

        public var DeviceName: String { storedDeviceName }
        public var Description: String { storedDescription }
        public var VendorId: Int32 { storedVendorId }
        public var DeviceId: Int32 { storedDeviceId }

        /// CNA documents both of these as **always zero** on this runtime, and
        /// they are passed through as measured rather than treated as missing.
        public var Revision: Int32 { storedRevision }
        public var SubSystemId: Int32 { storedSubSystemId }

        public var IsDefaultAdapter: Bool { storedIsDefaultAdapter }
        public var IsWideScreen: Bool { storedIsWideScreen }

        public var CurrentDisplayMode:
            Microsoft.Xna.Framework.Graphics.DisplayMode? {
            storedCurrentDisplayMode
        }

        public var SupportedDisplayModes:
            Microsoft.Xna.Framework.Graphics.DisplayModeCollection? {
            storedSupportedDisplayModes
        }

        /// `GraphicsAdapter.MonitorHandle`.
        ///
        /// **Always zero, and that is a stated absence.** CNA publishes
        /// `cna_graphics_adapter_get_native_monitor_handle`, and it is not
        /// bound: a monitor handle means nothing on a HEADLESS renderer, and
        /// answering a number a caller could hand to a platform API that is not
        /// there is worse than answering none. XNA's own value is null until a
        /// device is created against the adapter.
        public var MonitorHandle: Int { 0 }

        /// `GraphicsAdapter.IsProfileSupported(GraphicsProfile)`.
        ///
        /// Infallible in the CLR, so the answer is part of the same snapshot:
        /// the enumeration asks the route once per profile.
        // MARK: - The two methods, which may throw because they are methods

        /// `QueryBackBufferFormat` and `QueryRenderTargetFormat`.
        ///
        /// Methods rather than accessors, so unlike everything else on this
        /// type they may ask the route at the call site. Each answers whether
        /// the request was met exactly and writes three chosen values back
        /// through `inout` parameters — XNA's three `out`s, in this project's
        /// `ref`/`out` shape.
        ///
        /// XNA's own body first picks a `D3DDEVTYPE` from the two static flags:
        /// `UseNullDevice` selects NULLREF, otherwise `UseReferenceDevice`
        /// selects REF or HAL. **CNA's route takes no device type**, so that
        /// branch has no expression here beyond the preferences already pushed
        /// at enumeration. Recorded because the managed branch is real and this
        /// binding cannot carry it.
        public func QueryBackBufferFormat(
            _ graphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile,
            format: Microsoft.Xna.Framework.Graphics.SurfaceFormat,
            depthFormat: Microsoft.Xna.Framework.Graphics.DepthFormat,
            multiSampleCount: Int32,
            selectedFormat: inout Microsoft.Xna.Framework.Graphics.SurfaceFormat,
            selectedDepthFormat: inout Microsoft.Xna.Framework.Graphics.DepthFormat,
            selectedMultiSampleCount: inout Int32
        ) throws -> Bool {
            try query(graphicsProfile, format, depthFormat, multiSampleCount,
                      &selectedFormat, &selectedDepthFormat,
                      &selectedMultiSampleCount, backBuffer: true)
        }

        public func QueryRenderTargetFormat(
            _ graphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile,
            format: Microsoft.Xna.Framework.Graphics.SurfaceFormat,
            depthFormat: Microsoft.Xna.Framework.Graphics.DepthFormat,
            multiSampleCount: Int32,
            selectedFormat: inout Microsoft.Xna.Framework.Graphics.SurfaceFormat,
            selectedDepthFormat: inout Microsoft.Xna.Framework.Graphics.DepthFormat,
            selectedMultiSampleCount: inout Int32
        ) throws -> Bool {
            try query(graphicsProfile, format, depthFormat, multiSampleCount,
                      &selectedFormat, &selectedDepthFormat,
                      &selectedMultiSampleCount, backBuffer: false)
        }

        private func query(
            _ profile: Microsoft.Xna.Framework.Graphics.GraphicsProfile,
            _ format: Microsoft.Xna.Framework.Graphics.SurfaceFormat,
            _ depthFormat: Microsoft.Xna.Framework.Graphics.DepthFormat,
            _ multiSampleCount: Int32,
            _ selectedFormat: inout Microsoft.Xna.Framework.Graphics.SurfaceFormat,
            _ selectedDepthFormat: inout Microsoft.Xna.Framework.Graphics.DepthFormat,
            _ selectedMultiSampleCount: inout Int32,
            backBuffer: Bool
        ) throws -> Bool {
            let runtime = try RuntimeRegistry.current()
            let device = try GraphicsAdapter.deviceHandle(runtime)
            var selection = CNASwift_GraphicsFormatSelection()
            selection.struct_size =
                UInt32(MemoryLayout<CNASwift_GraphicsFormatSelection>.size)
            selection.struct_version = 1
            let route = backBuffer
                ? runtime.functions.graphicsAdapterQueryBackbufferFormat
                : runtime.functions.graphicsAdapterQueryRenderTargetFormat
            try runtime.functions.check(
                route(device, adapterIndex, UInt32(profile.rawValue),
                      UInt32(format.rawValue), UInt32(depthFormat.rawValue),
                      multiSampleCount, &selection),
                operation: backBuffer
                    ? "cna_graphics_adapter_query_backbuffer_format"
                    : "cna_graphics_adapter_query_render_target_format")
            guard let chosen = Microsoft.Xna.Framework.Graphics
                    .SurfaceFormat(rawValue: Int32(selection.format)),
                  let chosenDepth = Microsoft.Xna.Framework.Graphics
                    .DepthFormat(rawValue: Int32(selection.depth_format)) else {
                throw CNAError.producerInvariant(
                    "the format negotiation answered a surface or depth format "
                    + "outside the pinned enumerations")
            }
            selectedFormat = chosen
            selectedDepthFormat = chosenDepth
            selectedMultiSampleCount = selection.multi_sample_count
            return selection.exact_match != 0
        }

        // MARK: - Filling the list

        private static func deviceHandle(_ runtime: RuntimeState) throws -> UInt64 {
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.gameGetGraphicsDevice(runtime.gameHandle, &handle),
                operation: "cna_game_get_graphics_device")
            return handle
        }

        /// XNA's `InitializeAdapterList`, moved to the first moment a device
        /// exists.
        ///
        /// Called from `GraphicsDevice.borrow`, once per runtime generation.
        /// A failure here is swallowed on purpose: XNA's enumeration happens in
        /// a class constructor, where a throw would become a
        /// `TypeInitializationException` on an unrelated member, and this
        /// binding will not turn "the adapter list could not be built" into a
        /// failure of whatever the caller was actually doing. The list simply
        /// stays nil, which is the state its Optional exists to express.
        internal static func populateIfNeeded(
            _ runtime: RuntimeState, device: UInt64
        ) {
            guard populatedGeneration != runtime.generation else { return }
            populatedGeneration = runtime.generation
            do {
                storedAdapters = try enumerate(runtime, device)
                lastEnumerationFailure = nil
            } catch {
                storedAdapters = nil
                lastEnumerationFailure = error
            }
        }

        /// Why the last enumeration produced nothing.
        ///
        /// The failure is not propagated -- XNA enumerates in a class
        /// constructor, where a throw becomes a `TypeInitializationException`
        /// on whatever unrelated member happened to touch the type first, and
        /// this binding will not turn "the adapter list could not be built"
        /// into a failure of the caller's actual work. But it is not discarded
        /// either: a nil list with no reason is exactly the kind of silence
        /// this project keeps finding in other people's code.
        internal private(set) static var lastEnumerationFailure: Error?


        private static func enumerate(
            _ runtime: RuntimeState, _ device: UInt64
        ) throws -> CNAReadOnlyCollection<GraphicsAdapter> {
            var count: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.graphicsAdapterGetCount(device, &count),
                operation: "cna_graphics_adapter_get_count")

            if devicePreferencesNeedPush {
                for index in 0 ..< UInt32(clamping: count) {
                    try runtime.functions.check(
                        runtime.functions.graphicsAdapterSetDevicePreferences(
                            device, index, storedUseNullDevice ? 1 : 0,
                            storedUseReferenceDevice ? 1 : 0),
                        operation: "cna_graphics_adapter_set_device_preferences")
                }
                devicePreferencesNeedPush = false
            }
            // `cna_graphics_adapters_refresh` is deliberately NOT called: it
            // refuses while a device exists -- "The active C GraphicsDevice
            // retains its adapter; refreshing the global native adapter cache
            // would invalidate that reference" -- and a device is exactly what
            // this needs. It rebuilds a global cache, which reading the current
            // adapters does not require.

            let list = CNAList<GraphicsAdapter>()
            for index in 0 ..< UInt32(clamping: count) {
                try list.Add(read(runtime, device, index))
            }
            return CNAReadOnlyCollection(list: list)
        }

        private static func read(
            _ runtime: RuntimeState, _ device: UInt64, _ index: UInt32
        ) throws -> GraphicsAdapter {
            var info = CNASwift_GraphicsAdapterInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_GraphicsAdapterInfo>.size)
            info.struct_version = 1
            try runtime.functions.check(
                runtime.functions.graphicsAdapterGetInfo(device, index, &info),
                operation: "cna_graphics_adapter_get_info")

            var current = CNASwift_DisplayMode()
            current.struct_size = UInt32(MemoryLayout<CNASwift_DisplayMode>.size)
            current.struct_version = 1
            try runtime.functions.check(
                runtime.functions.graphicsAdapterGetCurrentDisplayMode(
                    device, index, &current),
                operation: "cna_graphics_adapter_get_current_display_mode")

            var modeCount: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.graphicsAdapterGetDisplayModeCount(
                    device, index, 0, 0, &modeCount),
                operation: "cna_graphics_adapter_get_display_mode_count")
            var raw = [CNASwift_DisplayMode](
                repeating: CNASwift_DisplayMode(), count: Int(modeCount))
            for position in raw.indices {
                raw[position].struct_size =
                    UInt32(MemoryLayout<CNASwift_DisplayMode>.size)
                raw[position].struct_version = 1
            }
            if modeCount > 0 {
                var written: UInt64 = 0
                try runtime.functions.check(
                    raw.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.graphicsAdapterCopyDisplayModes(
                            device, index, 0, 0, buffer.baseAddress, modeCount,
                            &written)
                    },
                    operation: "cna_graphics_adapter_copy_display_modes")
                raw = Array(raw.prefix(Int(written)))
            }

            var support: [Int32: Bool] = [:]
            for profile in [Microsoft.Xna.Framework.Graphics.GraphicsProfile.Reach,
                            Microsoft.Xna.Framework.Graphics.GraphicsProfile.HiDef] {
                var supported: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.graphicsAdapterIsProfileSupported(
                        device, index, UInt32(profile.rawValue), &supported),
                    operation: "cna_graphics_adapter_is_profile_supported")
                support[profile.rawValue] = supported != 0
            }

            return GraphicsAdapter(
                adapterIndex: index,
                deviceName: try copyString(
                    runtime, device, index, info.device_name_byte_length,
                    runtime.functions.graphicsAdapterCopyDeviceName,
                    "cna_graphics_adapter_copy_device_name"),
                description: try copyString(
                    runtime, device, index, info.description_byte_length,
                    runtime.functions.graphicsAdapterCopyDescription,
                    "cna_graphics_adapter_copy_description"),
                vendorId: info.vendor_id, deviceId: info.device_id,
                revision: info.revision, subSystemId: info.subsystem_id,
                isDefaultAdapter: info.is_default_adapter != 0,
                isWideScreen: info.is_wide_screen != 0,
                currentDisplayMode: try mode(current),
                supportedDisplayModes: Microsoft.Xna.Framework.Graphics
                    .DisplayModeCollection(displayModes: try raw.map(mode)),
                profileSupport: support)
        }

        private static func mode(
            _ raw: CNASwift_DisplayMode
        ) throws -> Microsoft.Xna.Framework.Graphics.DisplayMode {
            guard let format = Microsoft.Xna.Framework.Graphics
                    .SurfaceFormat(rawValue: Int32(raw.format)) else {
                throw CNAError.producerInvariant(
                    "an adapter reported a display mode whose surface format is "
                    + "outside the pinned enumeration")
            }
            return Microsoft.Xna.Framework.Graphics.DisplayMode(
                width: raw.width, height: raw.height, format: format)
        }

        private static func copyString(
            _ runtime: RuntimeState, _ device: UInt64, _ index: UInt32,
            _ byteLength: UInt64,
            _ route: NativeFunctions.GraphicsAdapterCopyDescriptionRoute,
            _ operation: String
        ) throws -> String {
            guard byteLength > 0 else { return "" }
            var bytes = [CChar](repeating: 0, count: Int(byteLength) + 1)
            var written: UInt64 = 0
            try runtime.functions.check(
                bytes.withUnsafeMutableBufferPointer { buffer in
                    route(device, index, buffer.baseAddress, byteLength, &written)
                },
                operation: operation)
            return String(decoding: bytes.prefix(Int(written))
                            .map { UInt8(bitPattern: $0) }, as: UTF8.self)
        }

        public func IsProfileSupported(
            _ graphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile
        ) -> Bool {
            storedProfileSupport[graphicsProfile.rawValue] ?? false
        }
    }
}
