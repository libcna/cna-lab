// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// Callback-scoped facade over the Game-owned graphics device.
    ///
    /// `open`, not `final`: XNA leaves the class derivable. Construction stays
    /// `private`, exactly as XNA's own accessible constructor is not yet
    /// projected, so the class is formally derivable and not yet constructible
    /// from outside — which is the honest state of a runtime-partial type
    /// rather than a strengthened one.
    open class GraphicsDevice {
        private let handle: UInt64
        private let runtime: RuntimeState
        private let generation: UInt64
        /// True only for a device this binding created, which is the one
        /// kind CNA lets a caller destroy.
        private let callerCreated: Bool
        private var callerCreatedDisposed = false
        private let disposingSource = CNAEventSource<CNAEventArgs>()
        private let deviceLostSource = CNAEventSource<CNAEventArgs>()
        private let deviceResetSource = CNAEventSource<CNAEventArgs>()
        private let deviceResettingSource = CNAEventSource<CNAEventArgs>()
        private let resourceCreatedSource = CNAEventSource<
            Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs>()
        private let resourceDestroyedSource = CNAEventSource<
            Microsoft.Xna.Framework.Graphics.ResourceDestroyedEventArgs>()
        private let callbackEpoch: UInt64

        /// Wraps a device handle the graphics device manager handed out.
        ///
        /// Both this and `borrow(from:)` produce the same callback-scoped
        /// facade; they differ only in which route supplied the handle.
        internal convenience init(borrowedHandle: UInt64, runtime: RuntimeState) throws {
            try GraphicsDevice.cacheProfile(runtime, handle: borrowedHandle)
            GraphicsAdapter.populateIfNeeded(runtime, device: borrowedHandle)
            try GraphicsDevice.cacheDeviceSnapshot(runtime, handle: borrowedHandle)
            self.init(handle: borrowedHandle, runtime: runtime)
        }

        private init(handle: UInt64, runtime: RuntimeState,
                     callerCreated: Bool = false) {
            self.handle = handle
            self.runtime = runtime
            self.callerCreated = callerCreated
            generation = runtime.generation
            callbackEpoch = runtime.callbackEpoch
        }

        /// `GraphicsDevice(GraphicsAdapter adapter, GraphicsProfile
        /// graphicsProfile, PresentationParameters presentationParameters)`.
        ///
        /// **A caller-created device is a different thing from the game's**,
        /// and CNA says so in both directions: `cna_graphics_device_create`
        /// hands back an OWNED handle, and `cna_graphics_device_destroy`
        /// accepts only such a handle while refusing a game's borrowed one.
        /// Resources remember which device made them, and mixing one device's
        /// resource into another's call is refused whichever pair it is.
        ///
        /// So this constructor exists and works, where `Dispose` on the game's
        /// device does not — the two are not in tension, they are the two
        /// halves of one ownership rule.
        public convenience init(
            adapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter,
            graphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile,
            presentationParameters:
                Microsoft.Xna.Framework.Graphics.PresentationParameters
        ) throws {
            let runtime = try RuntimeRegistry.current()
            var native = presentationParameters.nativeDescriptor()
            var created: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.graphicsDeviceCreate(
                    adapter.adapterIndex, UInt32(graphicsProfile.rawValue),
                    &native, &created),
                operation: "cna_graphics_device_create")
            self.init(handle: created, runtime: runtime, callerCreated: true)
        }

        /// **Neither of XNA's two pre-driver refusals is enforced here, and
        /// both reasons were measured rather than reasoned.**
        ///
        /// `NullWindowHandleNotAllowed`: the qualified HEADLESS renderer
        /// reports `DeviceWindowHandle = 0` for every device it makes -- the
        /// consumer canary prints `window=handle=0` -- so the check refused
        /// every creation and reset on this binding's own qualified boundary.
        ///
        /// `ProfileInvalidDevice`: reproducing it over
        /// `GraphicsAdapter.IsProfileSupported` refused a reset that CNA
        /// accepts, which is proof the two conditions are not the same one.
        /// Whether a profile can be served is CNA's decision, and
        /// `cna_graphics_device_create` reports it on the runtime channel
        /// naming the route -- which says what happened, where "Could not find
        /// a graphics device that supports the XNA Framework {0} profile."
        /// describes an enumeration this binding never performs.
        ///
        /// Both are recorded in `recorded-message-absences.json`, with the
        /// evidence, rather than left as code that refuses the wrong things.

        /// Reads the device's own `GraphicsProfile` once and caches it.
        ///
        /// Every facade this runtime hands out is the same device, so this runs
        /// on the first one and never again. It is called from the two places
        /// that build a facade rather than from the getter, because
        /// `get_GraphicsProfile` is `IL_NO_FAILURE_PATH` and a getter that
        /// cannot report a failure must not make a call that can have one.
        private static func cacheProfile(_ runtime: RuntimeState, handle: UInt64) throws {
            guard runtime.cachedGraphicsProfile == nil else { return }
            var raw: UInt32 = 0
            try runtime.functions.check(
                runtime.functions.graphicsDeviceGetGraphicsProfile(handle, &raw),
                operation: "cna_graphics_device_get_graphics_profile")
            guard let profile = Microsoft.Xna.Framework.Graphics.GraphicsProfile(
                rawValue: Int32(bitPattern: raw)) else {
                throw CNAError.nativeFailure(
                    operation: "GraphicsDevice.GraphicsProfile", result: 1,
                    message: "native graphics profile \(raw) is not an XNA GraphicsProfile")
            }
            runtime.cachedGraphicsProfile = profile
        }

        /// The device's presentation parameters and adapter, cached at the same
        /// moment as the profile and for the same reason.
        ///
        /// `get_PresentationParameters` and `get_Adapter` are both seven bytes
        /// -- `ldarg.0; ldfld; ret` -- and both pinned `IL_NO_FAILURE_PATH`, so
        /// neither Swift accessor may throw while the only source for either
        /// value is a fallible route. XNA caches them when the device is
        /// created; this caches them at the first facade, which is the moment
        /// Foundation 77 established as this binding's device-creation
        /// equivalent.
        private static func cacheDeviceSnapshot(
            _ runtime: RuntimeState, handle: UInt64
        ) throws {
            if runtime.cachedPresentationParameters == nil {
                var native = CNASwift_PresentationParameters()
                native.struct_size =
                    UInt32(MemoryLayout<CNASwift_PresentationParameters>.size)
                native.struct_version = 1
                try runtime.functions.check(
                    runtime.functions.graphicsDeviceGetPresentationParameters(
                        handle, &native),
                    operation: "cna_graphics_device_get_presentation_parameters")
                runtime.cachedPresentationParameters =
                    try Microsoft.Xna.Framework.Graphics.PresentationParameters(
                        native: native)
            }
            if runtime.cachedAdapter == nil {
                runtime.cachedAdapter = Microsoft.Xna.Framework.Graphics
                    .GraphicsAdapter.Adapters.flatMap { try? $0.Item(0) }
            }
        }

        /// `GraphicsDevice.GraphicsProfile`.
        ///
        /// `ldarg.0; ldfld _profileCapabilities; ldfld Profile; ret` — two field
        /// reads with no failure path, which is why this does not throw. The
        /// value is read from the device once, when the first facade of this
        /// runtime is built, exactly as XNA reads it once at device creation.
        ///
        /// It is non-Optional and always answered: the cache is filled before
        /// any facade exists, so there is no state in which a caller holds a
        /// `GraphicsDevice` whose profile is unknown. `Reach` is not a fallback
        /// — it is what the qualified artifact reports
        /// (`build-probe/f60_devicecaps.c`).
        public var GraphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile {
            runtime.cachedGraphicsProfile ?? .Reach
        }

        /// The limits the device's profile imposes.
        internal var profileCapabilities: Microsoft.Xna.Framework.Graphics.ProfileCapabilities {
            .table(for: GraphicsProfile)
        }

        internal static func borrow(from runtime: RuntimeState) throws -> GraphicsDevice {
            try runtime.validateBorrowed(epoch: runtime.callbackEpoch, operation: "Game.GraphicsDevice")
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.gameGetGraphicsDevice(runtime.gameHandle, &handle),
                operation: "cna_game_get_graphics_device"
            )
            try cacheProfile(runtime, handle: handle)
            // XNA enumerates adapters in GraphicsAdapter's class constructor.
            // This is the first moment a device exists, which is the closest
            // this binding can get to that.
            GraphicsAdapter.populateIfNeeded(runtime, device: handle)
            try cacheDeviceSnapshot(runtime, handle: handle)
            return GraphicsDevice(handle: handle, runtime: runtime)
        }

        public var Viewport: Microsoft.Xna.Framework.Graphics.Viewport {
            get throws {
                let handle = try validatedHandle("GraphicsDevice.Viewport")
                var native = CNASwift_Viewport()
                try runtime.functions.check(
                    runtime.functions.graphicsDeviceGetViewport(handle, &native),
                    operation: "cna_graphics_device_get_viewport"
                )
                return Microsoft.Xna.Framework.Graphics.Viewport(native: native)
            }
        }

        /// `GraphicsDevice.set_Viewport(Viewport value)`.
        ///
        /// A writer method, not a Swift `set`: the CLR setter is fallible and
        /// Swift has no throwing setter. This is the projection of the CLR
        /// setter accessor and not a new XNA member.
        ///
        /// The 387-byte body is mostly validation, and Foundation 47 shipped
        /// none of it — the projection packed six fields and pushed. Five
        /// separate branches raise the same exception:
        ///
        /// ```text
        /// Helpers.CheckDisposed(this, pComPtr)
        /// if (X < 0 || Y < 0 || Width <= 0 || Height <= 0)      throw   // IL_0172
        /// (targetW, targetH) = currentRenderTargetCount > 0
        ///     ? (currentRenderTargets[0].width, .height)
        ///     : (pInternalCachedParams.BackBufferWidth, .BackBufferHeight)
        /// if (X + Width > targetW || Y + Height > targetH)      throw   // IL_0162
        /// if (MinDepth < 0f || MinDepth > 1f)                   throw   // IL_0152
        /// if (MaxDepth < 0f || MaxDepth > 1f)                   throw   // IL_0142
        /// if (!(MaxDepth >= MinDepth))                          throw   // IL_0104
        /// ```
        ///
        /// Two details the IL settles and prose would not. The origin
        /// comparisons are `blt` against zero while the extent comparisons are
        /// `ble`, so `X = 0` is legal and `Width = 0` is not. And the last
        /// comparison is `bge.un` on `float64`, an *unordered* branch: it
        /// throws when `MaxDepth < MinDepth` **and** when either is NaN, which
        /// `MaxDepth < MinDepth` alone would not.
        ///
        /// `X + Width` is CIL `add` — unchecked — so it is `&+` here. A
        /// checked `+` would trap where XNA wraps and then rejects.
        public func SetViewport(
            _ value: Microsoft.Xna.Framework.Graphics.Viewport
        ) throws {
            let handle = try validatedHandle("GraphicsDevice.Viewport")
            guard value.X >= 0, value.Y >= 0, value.Width > 0, value.Height > 0 else {
                throw CNAArgumentException(
                    message: viewportInvalidMessage, paramName: "value")
            }
            let bounds = try currentTargetBounds()
            guard value.X &+ value.Width <= bounds.width,
                  value.Y &+ value.Height <= bounds.height else {
                throw CNAArgumentException(
                    message: viewportInvalidMessage, paramName: "value")
            }
            // Written as three rejections rather than one `guard` chain of
            // requirements, because the IL's branch *shapes* differ and a
            // chain of requirements silently unifies them. The range tests are
            // `blt`/`bgt` — "throw if below zero", "throw if above one" — and
            // neither is true of NaN, so a NaN depth passes both and is
            // rejected by the ordering test alone. Requiring `MinDepth >= 0`
            // instead would reject NaN one branch early, reach the same
            // outcome, and make the ordering test unreachable for the one
            // input that distinguishes `bge.un` from an ordered comparison.
            // The mutation harness found exactly that: the ordered rewrite
            // survived until this was restructured.
            if value.MinDepth < 0 || value.MinDepth > 1 {
                throw CNAArgumentException(
                    message: viewportInvalidMessage, paramName: "value")
            }
            if value.MaxDepth < 0 || value.MaxDepth > 1 {
                throw CNAArgumentException(
                    message: viewportInvalidMessage, paramName: "value")
            }
            if !(Double(value.MaxDepth) >= Double(value.MinDepth)) {
                throw CNAArgumentException(
                    message: viewportInvalidMessage, paramName: "value")
            }
            var native = CNASwift_Viewport()
            native.x = value.X
            native.y = value.Y
            native.width = value.Width
            native.height = value.Height
            native.min_depth = value.MinDepth
            native.max_depth = value.MaxDepth
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetViewport(handle, native),
                operation: "cna_graphics_device_set_viewport")
        }

        /// `GraphicsDevice.ScissorRectangle`.
        ///
        /// Both accessors are `IL_DIRECT_THROW`, so the reader throws and the
        /// writer is a throwing `SetScissorRectangle` — the same shape
        /// `Viewport` has, for the same reason.
        public var ScissorRectangle: Microsoft.Xna.Framework.Rectangle {
            get throws {
                let handle = try validatedHandle("GraphicsDevice.ScissorRectangle")
                var native = CNASwift_Rectangle()
                try runtime.functions.check(
                    runtime.functions.graphicsDeviceGetScissorRectangle(handle, &native),
                    operation: "cna_graphics_device_get_scissor_rectangle")
                return Microsoft.Xna.Framework.Rectangle(
                    native.x, native.y, native.width, native.height)
            }
        }

        /// `GraphicsDevice.set_ScissorRectangle(Rectangle value)`.
        ///
        /// The same shape as `set_Viewport` and, until Foundation 49, the same
        /// omission. The 300-byte body first rewrites its own copy of the
        /// rectangle into a D3DRECT — `Width` becomes the right edge and
        /// `Height` the bottom — and every comparison after that is on edges,
        /// not extents:
        ///
        /// ```text
        /// Helpers.CheckDisposed(this, pComPtr)
        /// if (X < 0 || Width < 0 || Y < 0 || Height < 0)        throw   // IL_011b
        /// value.Width  = value.X + value.Width;    // right
        /// value.Height = value.Y + value.Height;   // bottom
        /// (targetW, targetH) = the same render-target-or-backbuffer pair
        /// if (X > targetW || right > targetW ||
        ///     Y > targetH || bottom > targetH)                  throw   // IL_010b
        /// if (right - X > targetW || bottom - Y > targetH)      throw   // IL_00fb
        /// ```
        ///
        /// The last pair looks redundant — with `X >= 0` and
        /// `X + Width <= targetW`, `Width <= targetW` follows. It is not:
        /// `add` and `sub` are unchecked, so `X = 2, Width = Int32.max` wraps
        /// the right edge negative, slips past the edge test, and wraps back
        /// to a width that this test catches. That is why the arithmetic here
        /// is `&+` and `&-`, and why the pair is transcribed rather than
        /// dropped as unreachable.
        ///
        /// A negative extent is rejected here where the viewport rejects a
        /// zero one: `blt` against zero, not `ble`. An empty scissor rectangle
        /// is legal.
        public func SetScissorRectangle(
            _ value: Microsoft.Xna.Framework.Rectangle
        ) throws {
            let handle = try validatedHandle("GraphicsDevice.ScissorRectangle")
            guard value.X >= 0, value.Width >= 0,
                  value.Y >= 0, value.Height >= 0 else {
                throw CNAArgumentException(
                    message: scissorInvalidMessage, paramName: "value")
            }
            let right = value.X &+ value.Width
            let bottom = value.Y &+ value.Height
            let bounds = try currentTargetBounds()
            // `X <= targetW` and `Y <= targetH` are XNA's own comparisons and
            // are kept, but no mutation covers them: with `X >= 0` and
            // `Width >= 0` already established, `right <= targetW` implies
            // `X <= targetW`, and the one input that breaks that implication
            // — an overflowing width — is rejected by the pair below anyway.
            // Removing them changes no observable behaviour, so the mutation
            // that removed them was withdrawn rather than left in the harness
            // claiming coverage it cannot have.
            guard value.X <= bounds.width, right <= bounds.width,
                  value.Y <= bounds.height, bottom <= bounds.height else {
                throw CNAArgumentException(
                    message: scissorInvalidMessage, paramName: "value")
            }
            guard right &- value.X <= bounds.width,
                  bottom &- value.Y <= bounds.height else {
                throw CNAArgumentException(
                    message: scissorInvalidMessage, paramName: "value")
            }
            var native = CNASwift_Rectangle()
            native.x = value.X
            native.y = value.Y
            native.width = value.Width
            native.height = value.Height
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetScissorRectangle(handle, native),
                operation: "cna_graphics_device_set_scissor_rectangle")
        }

        /// `GraphicsDevice.GraphicsDeviceStatus`.
        ///
        /// `IL_DIRECT_THROW` and no setter, so a throwing reader only. CNA and
        /// XNA agree on all three values — `Normal` 0, `Lost` 1, `NotReset` 2 —
        /// and the conversion is still an explicit map: eight of the nine state
        /// enums agreed in Foundation 45 too, and the ninth did not.
        public var GraphicsDeviceStatus: Microsoft.Xna.Framework.Graphics.GraphicsDeviceStatus {
            get throws {
                let handle = try validatedHandle("GraphicsDevice.GraphicsDeviceStatus")
                var native: UInt32 = 0
                try runtime.functions.check(
                    runtime.functions.graphicsDeviceGetStatus(handle, &native),
                    operation: "cna_graphics_device_get_status")
                switch native {
                case 0: return .Normal
                case 1: return .Lost
                case 2: return .NotReset
                default:
                    throw CNAError.producerInvariant(
                        "cna_graphics_device_get_status answered \(native), which is "
                        + "outside the three values GraphicsDeviceStatus declares")
                }
            }
        }

        /// `GraphicsDevice.Clear(Color color)`.
        ///
        /// Twenty bytes of IL, and every one of them forwards:
        ///
        ///     this.Clear(this.DefaultClearOptions, color, 1.0f, 0)
        ///
        /// The `1.0f` and the `0` are pinned literals — `ldc.r4 1` and
        /// `ldc.i4.0` — and `DefaultClearOptions` decides whether the depth
        /// and stencil buffers are cleared along with the colour. Until
        /// Foundation 48 this projection called CNA's colour-only route
        /// directly, which silently dropped both. That was a real divergence
        /// wherever a depth buffer existed, and this is its repair.
        public func Clear(_ color: Microsoft.Xna.Framework.Color) throws {
            try Clear(try defaultClearOptions, color: color, depth: 1.0, stencil: 0)
        }

        /// `GraphicsDevice.Clear(ClearOptions options, Vector4 color, Single depth, Int32 stencil)`.
        ///
        /// Also pure forwarding: `new Color(vector4)` and then the Color
        /// overload. The conversion is XNA's own `Color(Vector4)`
        /// constructor, not a second rounding rule invented here.
        public func Clear(
            _ options: Microsoft.Xna.Framework.Graphics.ClearOptions,
            color: Microsoft.Xna.Framework.Vector4,
            depth: Float,
            stencil: Int32
        ) throws {
            try Clear(options, color: Microsoft.Xna.Framework.Color(color),
                      depth: depth, stencil: stencil)
        }

        /// `GraphicsDevice.Clear(ClearOptions options, Color color, Single depth, Int32 stencil)`.
        ///
        /// The 543-byte overload the other two reach. What survives
        /// projection is its order of events, which is the part that is
        /// observable: check disposed, clear, and only then — if the clear
        /// *failed* — decide which exception that failure deserves.
        ///
        ///     hr = pComPtr->Clear(0, null, options, argb, depth, stencil);
        ///     …
        ///     if (hr < 0) {
        ///         ClearOptions requested = options & 6;   // DepthBuffer|Stencil
        ///         if ((this.DefaultClearOptions & requested) != requested)
        ///             throw new InvalidOperationException(
        ///                 FrameworkResources.CannotClearNullDepth);
        ///         throw GraphicsHelpers.GetExceptionFromResult(hr);
        ///     }
        ///
        /// `CannotClearNullDepth` is therefore a *diagnosis of a failure*, not
        /// a pre-validation: XNA asks the device to clear buffers it may not
        /// have, and only explains itself once the device has refused. A
        /// projection that checked the mask up front would reject clears XNA
        /// performs, which is why this one does not.
        ///
        /// XNA hands D3D9 the `ClearOptions` word unchanged — the three
        /// declared bits share D3DCLEAR_TARGET/ZBUFFER/STENCIL's values — so
        /// an undeclared bit reaches the driver and comes back as a failure.
        /// The three declared bits are mapped explicitly here, per the
        /// Foundation 45 rule, and any remaining bits are passed through
        /// unchanged so that a caller who sets one still gets XNA's answer
        /// rather than a silently narrowed clear.
        ///
        /// What does not survive: the D3D9 scissor-state save/restore around
        /// the clear, the temporary full-target viewport, and the
        /// `lazyClearFlags` bookkeeping. All three are below CNA's
        /// abstraction — `cna_graphics_device_clear_options` is one call, not
        /// a device-state sequence — and none is observable through any
        /// projected member. `SetContentLost(false)` on each bound render
        /// target is CNA's to decide for the same reason: it owns the
        /// content-lost state that `RenderTarget2D.IsContentLost` reports.
        public func Clear(
            _ options: Microsoft.Xna.Framework.Graphics.ClearOptions,
            color: Microsoft.Xna.Framework.Color,
            depth: Float,
            stencil: Int32
        ) throws {
            let handle = try validatedHandle("GraphicsDevice.Clear")
            let result = runtime.functions.graphicsDeviceClearOptions(
                handle, NativeStateCodes.clearOptions(options), color.native,
                depth, stencil)
            guard result != 0 else { return }
            let requested = options.intersection(
                [.DepthBuffer, .Stencil])
            guard try defaultClearOptions.intersection(requested) == requested else {
                throw CNAInvalidOperationException(message: cannotClearNullDepthMessage)
            }
            try runtime.functions.check(
                result, operation: "cna_graphics_device_clear_options")
        }

        /// The width and height `set_Viewport` and `set_ScissorRectangle`
        /// validate against, and the same `currentRenderTargetCount > 0`
        /// branch `get_DefaultClearOptions` takes for the depth format.
        ///
        /// XNA writes the branch out three times; it is written once here
        /// because all three read the same two fields of the same two
        /// sources, and a single reading is easier to keep honest than three
        /// copies of it.
        internal func currentTargetBounds() throws -> (width: Int32, height: Int32) {
            if let slotZero = boundRenderTargetSlotZero {
                return (slotZero.renderTargetWidth, slotZero.renderTargetHeight)
            }
            let native = try nativePresentationParameters()
            return (native.back_buffer_width, native.back_buffer_height)
        }

        /// Slot zero's `DepthStencilFormat`, from either kind of target.
        private var boundRenderTargetDepthFormat: DepthFormat? {
            guard let first = runtime.cachedRenderTargetBindings.first,
                  !first.RenderTarget.IsDisposed else { return nil }
            if let target = first.RenderTarget as? RenderTarget2D {
                return target.DepthStencilFormat
            }
            if let target = first.RenderTarget as? RenderTargetCube {
                return target.DepthStencilFormat
            }
            return nil
        }

        /// `currentRenderTargetCount > 0 ? currentRenderTargets[0] : null`,
        /// which is the branch three XNA members take.
        ///
        /// A **disposed** target is treated as no target. XNA cannot reach that
        /// state — `GraphicsDevice.Dispose` releases its bindings and CNA
        /// refuses `cna_render_target_destroy` on a bound target with
        /// `CNA_RESULT_INVALID_STATE` — so this is the honest answer for an
        /// input XNA does not define rather than a reproduction of one.
        private var boundRenderTargetSlotZero:
            (any Microsoft.Xna.Framework.Graphics.RenderTargetDescription)? {
            guard let first = runtime.cachedRenderTargetBindings.first else { return nil }
            let target = first.RenderTarget
            guard !target.IsDisposed else { return nil }
            return target as? any Microsoft.Xna.Framework.Graphics.RenderTargetDescription
        }

        /// `GraphicsDevice.SetRenderTarget(RenderTarget2D renderTarget)`.
        ///
        /// Thirty-one bytes of IL, and every one of them is the same decision:
        ///
        /// ```text
        /// if (renderTarget != null) {
        ///     RenderTargetBinding b = new RenderTargetBinding(renderTarget);
        ///     SetRenderTargets(&b, 1);
        /// } else {
        ///     SetRenderTargets(null, 0);
        /// }
        /// ```
        ///
        /// So a nil target restores the backbuffer, and every validation the
        /// array path performs happens here too — in particular the disposal
        /// check and the device-identity check.
        ///
        /// The native call is `cna_graphics_device_set_render_target2d` rather
        /// than the array route, and that is a measured equivalence rather than
        /// an assumption: after the dedicated route,
        /// `cna_graphics_device_get_render_target_count` answers 1 and
        /// `cna_graphics_device_copy_render_targets` hands back the same handle
        /// with face `PositiveX` — the identical state the array route leaves
        /// (`build-probe/f65_rtcube.c`). Binding the array route for a call the
        /// dedicated one serves would leave the dedicated one with no consuming
        /// member, which `docs/native-abi.md` forbids.
        public func SetRenderTarget(
            _ renderTarget: RenderTarget2D?
        ) throws {
            let deviceHandle = try validatedHandle("GraphicsDevice.SetRenderTarget")
            guard let renderTarget else {
                try unbindRenderTargets(deviceHandle)
                return
            }
            let binding = RenderTargetBinding(renderTarget)
            try validateRenderTargets([binding])
            let targetHandle = try renderTarget.validatedHandle(
                "GraphicsDevice.SetRenderTarget")
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetRenderTarget2D(deviceHandle, targetHandle),
                operation: "cna_graphics_device_set_render_target2d"
            )
            recordActiveRenderTargets([binding])
        }

        /// `GraphicsDevice.SetRenderTarget(RenderTargetCube renderTarget, CubeMapFace cubeMapFace)`.
        ///
        /// The same thirty-two bytes with the face carried into the binding,
        /// and the same nil-restores-the-backbuffer branch. The face is
        /// **ignored** when the target is nil, because XNA's null branch calls
        /// `SetRenderTargets(null, 0)` without ever building a binding.
        public func SetRenderTarget(
            _ renderTarget: RenderTargetCube?, cubeMapFace: CubeMapFace
        ) throws {
            let deviceHandle = try validatedHandle("GraphicsDevice.SetRenderTarget")
            guard let renderTarget else {
                try unbindRenderTargets(deviceHandle)
                return
            }
            let binding = RenderTargetBinding(renderTarget, cubeMapFace)
            try validateRenderTargets([binding])
            let targetHandle = try renderTarget.validatedHandle(
                "GraphicsDevice.SetRenderTarget")
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetRenderTargetCube(
                    deviceHandle, targetHandle, UInt32(bitPattern: cubeMapFace.rawValue)),
                operation: "cna_graphics_device_set_render_target_cube"
            )
            recordActiveRenderTargets([binding])
        }

        /// `GraphicsDevice.SetRenderTargets(params RenderTargetBinding[] renderTargets)`.
        ///
        /// ```text
        /// if (renderTargets != null && renderTargets.Length > 0)
        ///     fixed (RenderTargetBinding* p = &renderTargets[0])
        ///         SetRenderTargets(p, renderTargets.Length);
        /// else
        ///     SetRenderTargets(null, 0);
        /// ```
        ///
        /// A null array and an **empty** array take the same branch, so both
        /// restore the backbuffer. Optional for the null half, which is XNA's
        /// own `params` reaching a normal `ret`.
        public func SetRenderTargets(
            _ renderTargets: [RenderTargetBinding]?
        ) throws {
            let deviceHandle = try validatedHandle("GraphicsDevice.SetRenderTargets")
            guard let renderTargets, !renderTargets.isEmpty else {
                try unbindRenderTargets(deviceHandle)
                return
            }
            try validateRenderTargets(renderTargets)

            var native = try renderTargets.map { binding -> CNASwift_RenderTargetBinding in
                var element = CNASwift_RenderTargetBinding()
                element.struct_size = UInt32(
                    MemoryLayout<CNASwift_RenderTargetBinding>.size)
                element.struct_version = 1
                element.render_target = try binding.RenderTarget.validatedHandle(
                    "GraphicsDevice.SetRenderTargets")
                // `array_slice` is zero for both kinds: CNA refuses a nonzero
                // slice for a 2D target outright, and a cube's subresource is
                // the face. XNA has no counterpart field at all.
                element.array_slice = 0
                element.cube_map_face = UInt32(bitPattern: binding.CubeMapFace.rawValue)
                return element
            }
            try runtime.functions.check(
                native.withUnsafeMutableBufferPointer { buffer in
                    runtime.functions.graphicsDeviceSetRenderTargets(
                        deviceHandle, buffer.baseAddress, UInt64(buffer.count))
                },
                operation: "cna_graphics_device_set_render_targets")
            recordActiveRenderTargets(renderTargets)
        }

        /// `GraphicsDevice.GetRenderTargets()`.
        ///
        /// `new RenderTargetBinding[currentRenderTargetCount]` filled by
        /// `Array.Copy` — a **copy** of the managed bindings, which Swift's
        /// value-typed `Array` gives for free. The objects are the ones that
        /// were bound, which is the one thing a handle-keyed lookup through
        /// `cna_graphics_device_copy_render_targets` could not give: CNA
        /// answers handles and publishes no route back to an object.
        public func GetRenderTargets() -> [RenderTargetBinding] {
            runtime.cachedRenderTargetBindings
        }

        /// `GraphicsDevice.GetBackBufferData<T>(T[])`.
        public func GetBackBufferData<T>(_ data: inout [T]) throws {
            try GetBackBufferData(
                nil, data: &data, startIndex: 0,
                elementCount: Int32(data.count))
        }

        /// `GraphicsDevice.GetBackBufferData<T>(T[], Int32, Int32)`.
        public func GetBackBufferData<T>(
            _ data: inout [T], startIndex: Int32, elementCount: Int32
        ) throws {
            try GetBackBufferData(
                nil, data: &data, startIndex: startIndex,
                elementCount: elementCount)
        }

        /// `GraphicsDevice.GetBackBufferData<T>(Rectangle?, T[], Int32, Int32)`.
        ///
        /// XNA validates the device/profile, array window, active render
        /// targets, element size, rectangle and total byte size in that order.
        /// CNA returns RGBA8 pixels; the bytes are copied into the caller's T
        /// window only after the complete native read succeeds.
        public func GetBackBufferData<T>(
            _ rect: Microsoft.Xna.Framework.Rectangle?,
            data: inout [T], startIndex: Int32, elementCount: Int32
        ) throws {
            let live = try validatedHandle("GraphicsDevice.GetBackBufferData")
            guard profileCapabilities.getBackBufferData else {
                try profileCapabilities.throwNotSupported(
                    ProfileCapabilities.profileFeatureNotSupported,
                    "GetBackBufferData")
            }
            guard !data.isEmpty else {
                throw CNAArgumentNullException(
                    paramName: "data", message: GraphicsDevice.nullNotAllowedMessage)
            }
            try Microsoft.Xna.Framework.Graphics.validateCopyParameters(
                dataLength: data.count, dataIndex: startIndex,
                elementCount: elementCount)
            guard runtime.cachedRenderTargetBindings.isEmpty else {
                throw CNAInvalidOperationException(
                    message: GraphicsDevice.cannotReadActiveRenderTargetMessage)
            }
            guard let parameters = runtime.cachedPresentationParameters,
                  let formatSize = Microsoft.Xna.Framework.Graphics
                    .expectedByteSize(of: parameters.BackBufferFormat) else {
                throw CNANotSupportedException(
                    message: "The active back buffer format cannot be read.")
            }
            let elementSize = Int32(MemoryLayout<T>.size)
            if elementSize != formatSize {
                guard formatSize > elementSize,
                      formatSize % elementSize == 0 else {
                    throw CNAArgumentException(message: invalidDataSizeMessage)
                }
            }

            var width = parameters.BackBufferWidth
            var height = parameters.BackBufferHeight
            if let rect {
                guard rect.X >= 0, rect.Width > 0,
                      rect.Y >= 0, rect.Height > 0 else {
                    throw CNAArgumentException(
                        message: invalidRectangleMessage, paramName: "rect")
                }
                guard UInt32(bitPattern: rect.X &+ rect.Width)
                        <= UInt32(bitPattern: parameters.BackBufferWidth),
                      UInt32(bitPattern: rect.Y &+ rect.Height)
                        <= UInt32(bitPattern: parameters.BackBufferHeight) else {
                    throw CNAArgumentException(
                        message: invalidRectangleMessage, paramName: "rect")
                }
                width = rect.Width
                height = rect.Height
            }
            let byteCount = Int64(width) * Int64(height) * Int64(formatSize)
            let destinationByteCount = Int64(elementSize) * Int64(elementCount)
            guard byteCount == destinationByteCount else {
                throw CNAArgumentException(message: invalidTotalSizeMessage)
            }
            guard formatSize == 4 else {
                throw CNANotSupportedException(
                    message: "The active back buffer format cannot be read as RGBA8.")
            }

            let pixelCount = Int64(width) * Int64(height)
            guard pixelCount >= 0, pixelCount <= Int64(Int.max) else {
                throw CNAArgumentException(message: invalidTotalSizeMessage)
            }
            var readback = CNASwift_BackBufferReadback()
            readback.struct_size = UInt32(
                MemoryLayout<CNASwift_BackBufferReadback>.size)
            readback.struct_version = 1
            readback.has_source_rectangle = rect == nil ? 0 : 1
            if let rect {
                readback.source_rectangle = CNASwift_Rectangle(
                    x: rect.X, y: rect.Y, width: rect.Width,
                    height: rect.Height)
            }
            readback.start_index = 0
            readback.element_count = UInt64(pixelCount)
            var pixels = [CNASwift_Color](
                repeating: CNASwift_Color(), count: Int(pixelCount))
            try runtime.functions.check(
                withUnsafePointer(to: &readback) { descriptor in
                    pixels.withUnsafeMutableBufferPointer { output in
                        runtime.functions.graphicsDeviceGetBackBufferData(
                            live, descriptor, output.baseAddress,
                            UInt64(output.count))
                    }
                }, operation: "cna_graphics_device_get_backbuffer_data_window")

            let destinationOffset = Int(startIndex) * Int(elementSize)
            pixels.withUnsafeBytes { source in
                data.withUnsafeMutableBytes { destination in
                    guard let sourceBase = source.baseAddress,
                          let destinationBase = destination.baseAddress else { return }
                    destinationBase.advanced(by: destinationOffset).copyMemory(
                        from: sourceBase, byteCount: Int(byteCount))
                }
            }
        }

        /// The backbuffer branch both single-target overloads and the array
        /// overload share: `SetRenderTargets(null, 0)`.
        private func unbindRenderTargets(_ deviceHandle: UInt64) throws {
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetRenderTargets(deviceHandle, nil, 0),
                operation: "cna_graphics_device_set_render_targets")
            recordActiveRenderTargets([])
        }

        /// `Texture.isActiveRenderTarget`, cleared on what leaves the device
        /// and set on what arrives, which is what XNA's private
        /// `SetRenderTargets` does around its own binding loop.
        ///
        /// Three members read the flag — `TextureCollection.SetItem` and the
        /// `CopyData` of `Texture2D` and `TextureCube` — so it is the single
        /// place all three get their answer from, and the cache and the flag
        /// are written together so neither can be updated without the other.
        private func recordActiveRenderTargets(_ bindings: [RenderTargetBinding]) {
            for previous in runtime.cachedRenderTargetBindings {
                previous.RenderTarget.isActiveRenderTarget = false
            }
            for binding in bindings {
                binding.RenderTarget.isActiveRenderTarget = true
            }
            runtime.cachedRenderTargetBindings = bindings
        }

        /// The private `SetRenderTargets(RenderTargetBinding*, Int32)`, up to
        /// the point where XNA touches the device.
        ///
        /// ```text
        /// if (count == currentRenderTargetCount
        ///     && every binding's target AND face match the current ones) return;
        /// if (count > MaxRenderTargets)
        ///     Throw(ProfileMaxRenderTargets, MaxRenderTargets);
        /// for (i = 0; i < count; i++) {
        ///     Texture t = bindings[i]._renderTarget;
        ///     if (t == null) throw new ArgumentException(NullNotAllowed);
        ///     Helpers.CheckDisposed(t, t.GetComPtr());
        ///     if (t.GraphicsDevice != this)
        ///         throw new InvalidOperationException(InvalidDevice);
        ///     if (i > 0) {
        ///         for (j = 0; j < i; j++)
        ///             if (t == bindings[j]._renderTarget)
        ///                 throw new ArgumentException(CannotSetAlreadyUsedRenderTarget);
        ///         if (!RenderTargetHelper.IsSameSize(t, bindings[0]._renderTarget))
        ///             throw new ArgumentException(RenderTargetsMustMatch);
        ///     }
        /// }
        /// ```
        ///
        /// The profile message names the **limit**, not the requested count —
        /// `V_24` is `MaxRenderTargets` and it is what is boxed — which is the
        /// same shape `ProfileMaxVertexStreams` has.
        ///
        /// The duplicate and same-size tests are `i > 0` only, so a
        /// single-element array reaches neither; that is why both single-target
        /// overloads can share this without a special case.
        ///
        /// The null-target test cannot be reached through a Swift
        /// `RenderTargetBinding`, whose stored target is non-Optional and whose
        /// both constructors require one; it is recorded in
        /// `recorded-message-absences.json` rather than written as dead code.
        ///
        /// The device-identity test compares the **`RuntimeState`**, not the
        /// facade, for the reason Foundation 63 recorded: a facade is a
        /// per-callback capability token, so comparing facades would refuse
        /// every target created in one callback and bound in another.
        private func validateRenderTargets(
            _ bindings: [RenderTargetBinding]
        ) throws {
            let capabilities = profileCapabilities
            guard bindings.count <= Int(capabilities.maxRenderTargets) else {
                try capabilities.throwNotSupported(
                    Microsoft.Xna.Framework.Graphics.ProfileCapabilities
                        .profileMaxRenderTargets,
                    "\(capabilities.maxRenderTargets)")
            }
            for (index, binding) in bindings.enumerated() {
                let target = binding.RenderTarget
                _ = try target.validatedHandle("GraphicsDevice.SetRenderTargets")
                guard target.nativeStorage.runtime === runtime else {
                    throw CNAInvalidOperationException(
                        message: GraphicsDevice.invalidDeviceMessage)
                }
                guard index > 0 else { continue }
                for earlier in bindings[0..<index] where earlier.RenderTarget === target {
                    throw CNAArgumentException(
                        message: GraphicsDevice.cannotSetAlreadyUsedRenderTargetMessage)
                }
                guard Microsoft.Xna.Framework.Graphics.renderTargetsAreSameSize(
                    target, bindings[0].RenderTarget) else {
                    throw CNAArgumentException(
                        message: GraphicsDevice.renderTargetsMustMatchMessage)
                }
            }
        }

        /// `GraphicsDevice.PresentationParameters`.
        ///
        /// Seven bytes of IL -- `ldarg.0; ldfld pPublicCachedParams; ret` --
        /// and pinned `IL_NO_FAILURE_PATH`, so this may not throw.
        ///
        /// **This property was absent until Foundation 78, and the reason it
        /// was absent stopped being true.** The note here used to read: "this
        /// binding has exactly one source for those values -- a fallible CNA
        /// route -- and no device-creation moment it observes at which to cache
        /// them infallibly". Foundation 77 created that moment, because
        /// `GraphicsAdapter` needed one for the same reason, and the value is
        /// read there beside the profile.
        public var PresentationParameters:
            Microsoft.Xna.Framework.Graphics.PresentationParameters? {
            runtime.cachedPresentationParameters
        }

        /// `GraphicsDevice.Adapter`, also seven bytes and also infallible.
        ///
        /// The adapter the device was created against. CNA has no per-device
        /// adapter route, so this is the enumerated default -- which on this
        /// host is the only one there is.
        public var Adapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter? {
            runtime.cachedAdapter
        }

        /// `GraphicsDevice.DisplayMode`.
        ///
        /// **The one accessor of this group that may throw**, and the table
        /// says why: `IL_DIRECT_THROW`. Its first instructions are
        /// `Helpers.CheckDisposed(this, pComPtr)`, so a disposed device reports
        /// `ObjectDisposedException` before anything is read.
        ///
        /// XNA then asks D3D for the adapter's current mode. This asks the
        /// adapter this device recorded, which is the same question routed
        /// through the type that owns the answer.
        public var DisplayMode: Microsoft.Xna.Framework.Graphics.DisplayMode? {
            get throws {
                _ = try validatedHandle("GraphicsDevice.DisplayMode")
                return runtime.cachedAdapter?.CurrentDisplayMode
            }
        }

/// `GraphicsDevice.IsDisposed`.
        ///
        /// Seven bytes of `ldfld isDisposed`, pinned infallible — so this may
        /// not throw and may not call a route.
        ///
        /// **The facade's generation is the answer.** This binding's device is
        /// a per-callback facade over a handle the runtime owns; the device is
        /// disposed exactly when the game that owned it is gone, which is what
        /// a stale generation already means here. Asking
        /// `cna_graphics_device_get_is_disposed` would need a handle that is by
        /// then invalid, and the getter could not report the failure anyway.
        public var IsDisposed: Bool {
            callerCreated
                ? callerCreatedDisposed
                : runtime.generation != generation
        }

        /// `GraphicsDevice.Dispose()` and `Dispose(Boolean)`.
        ///
        /// **They refuse, and that is CNA's design rather than a gap.** The
        /// route exists precisely to say so: *"Reports that C cannot dispose
        /// the graphics device owned by the active game"*, answering
        /// `CNA_RESULT_NOT_SUPPORTED` for a valid handle, because *"disposing
        /// it through a borrowed handle would leave that game drawing into a
        /// destroyed device"*. Canonical disposal is `cna_game_destroy`, which
        /// is `Game.Dispose`'s business and not this type's.
        ///
        /// XNA's own `Dispose()` is `Dispose(true); GC.SuppressFinalize(this)`
        /// and does not throw. This one does, and the refusal is deliberate: a
        /// `Dispose` that silently did nothing would leave a caller believing
        /// the device was released. An honest failure is the better half of
        /// that trade, and it is the only half this runtime allows.
        public func Dispose() throws {
            try Dispose(true)
        }

        /// `protected virtual void Dispose(Boolean disposing)`.
        ///
        /// XNA branches on `disposing` to pick the destructor or the finalizer;
        /// both end at the same native teardown, and neither is reachable here,
        /// so the argument changes nothing and the route is asked either way.
        open func Dispose(_ disposing: Bool) throws {
            guard callerCreated else {
                // The game's device: the route exists to refuse, and it does.
                let handle = try validatedHandle("GraphicsDevice.Dispose")
                try runtime.functions.check(
                    runtime.functions.graphicsDeviceDispose(handle),
                    operation: "cna_graphics_device_dispose")
                return
            }
            // A device this binding created is ours to release, and releasing
            // it twice is not an error -- XNA's Dispose is idempotent and so
            // is this.
            guard !callerCreatedDisposed else { return }
            let handle = try validatedHandle("GraphicsDevice.Dispose")
            try runtime.functions.check(
                runtime.functions.graphicsDeviceDestroy(handle),
                operation: "cna_graphics_device_destroy")
            callerCreatedDisposed = true
        }

        /// The six events `GraphicsDevice` declares.
        ///
        /// Four carry `EventArgs`; `ResourceCreated` and `ResourceDestroyed`
        /// carry their own payloads, which is why they need their own sources.
        ///
        /// **Nothing on this host raises them.** XNA's are raised by the
        /// platform when a device is lost, reset or torn down, and by every
        /// resource as it is created or destroyed. CNA publishes no
        /// subscription for any of that, so a handler added here fires only if
        /// a consumer raises it. That is a stated absence rather than a
        /// pretence: the events exist because the surface has them, and the
        /// raisers are the honest way to reach them.
        public var Disposing: CNAEvent<CNAEventArgs> { disposingSource.Event }
        public var DeviceLost: CNAEvent<CNAEventArgs> { deviceLostSource.Event }
        public var DeviceReset: CNAEvent<CNAEventArgs> { deviceResetSource.Event }
        public var DeviceResetting: CNAEvent<CNAEventArgs> {
            deviceResettingSource.Event
        }
        public var ResourceCreated:
            CNAEvent<Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs> {
            resourceCreatedSource.Event
        }
        public var ResourceDestroyed:
            CNAEvent<Microsoft.Xna.Framework.Graphics.ResourceDestroyedEventArgs> {
            resourceDestroyedSource.Event
        }

                /// `GraphicsDevice.Present()`.
        ///
        /// Foundation 60 measured this route as accepted and nobody built the
        /// member for fourteen milestones.
        ///
        /// **What a returning `Present` means here is narrow**: the device took
        /// the request. Nothing observes a frame reaching a display, because
        /// this host has no display -- the same bound Foundation 68's draws
        /// live inside.
        public func Present() throws {
            let handle = try validatedHandle("GraphicsDevice.Present")
            // XNA refuses to present while a render target is bound, and the
            // binding already knows which ones are: `GetRenderTargets` answers
            // the same cached bindings. CNA makes no such check, so without
            // this a caller would present the backbuffer they were not drawing
            // into and see the frame they expected to have replaced.
            guard runtime.cachedRenderTargetBindings.isEmpty else {
                throw CNAInvalidOperationException(
                    message: GraphicsDevice.cannotPresentActiveRenderTargetsMessage)
            }
            try runtime.functions.check(
                runtime.functions.graphicsDevicePresent(handle),
                operation: "cna_graphics_device_present")
        }

        /// `FrameworkResources.CannotPresentActiveRenderTargets`, read out of
        /// the registered `Microsoft.Xna.Framework.dll`.
        internal static let cannotPresentActiveRenderTargetsMessage =
            "Cannot call Present when a render target is active." 

/// `Present(Nullable<Rectangle> sourceRectangle, Nullable<Rectangle>
        /// destinationRectangle, IntPtr overrideWindowHandle)`.
        ///
        /// XNA's 136 bytes turn the two optional rectangles into native `RECT`
        /// pointers and hand them, with the window handle, to D3D's `Present`.
        ///
        /// **CNA's route takes none of the three.**
        /// `cna_graphics_device_present` is `(CNA_Handle game)` and there is no
        /// other presentation route — the whole family is
        /// `get_presentation_parameters`, `set_presentation_parameters` and
        /// this one.
        ///
        /// So this member **refuses rather than silently presenting the whole
        /// surface**. A caller asking for a sub-rectangle, a stretched
        /// destination or a different window is asking for something this
        /// runtime cannot do, and quietly giving them a full-surface present
        /// would be a wrong frame reported as a right one. XNA does not refuse
        /// — this is a divergence, and it is the one that fails loudly.
        ///
        /// With all three at their defaults the request is exactly what the
        /// no-argument overload performs, and it is forwarded.
        public func Present(
            _ sourceRectangle: Microsoft.Xna.Framework.Rectangle?,
            destinationRectangle: Microsoft.Xna.Framework.Rectangle?,
            overrideWindowHandle: Int
        ) throws {
            guard sourceRectangle == nil, destinationRectangle == nil,
                  overrideWindowHandle == 0 else {
                throw CNANotSupportedException(
                    message: GraphicsDevice.presentArgumentsNotSupportedMessage)
            }
            try Present()
        }

        /// Not a Microsoft string: no XNA resource covers this, because XNA
        /// never refuses here. Written to say exactly which argument cannot be
        /// carried and why.
        internal static let presentArgumentsNotSupportedMessage =
            "This runtime presents the whole surface to the game's own window. "
            + "cna_graphics_device_present takes no source rectangle, no "
            + "destination rectangle and no window handle, so a Present naming "
            + "any of them is refused rather than silently widened to the full "
            + "surface."

                /// `GraphicsDevice.Reset()`.
        ///
        /// Resets with the parameters the device already has.
        public func Reset() throws {
            let handle = try validatedHandle("GraphicsDevice.Reset")
            try runtime.functions.check(
                runtime.functions.graphicsDeviceReset(handle),
                operation: "cna_graphics_device_reset")
        }

        /// `GraphicsDevice.Reset(PresentationParameters)`.
        ///
        /// The new parameters replace the cached ones **only if the reset is
        /// accepted**, which is the order XNA uses: its own body applies them
        /// to the device first and updates `pPublicCachedParams` afterwards, so
        /// a refused reset leaves the property reading what the device still
        /// actually has.
        public func Reset(
            _ presentationParameters:
                Microsoft.Xna.Framework.Graphics.PresentationParameters
        ) throws {
            let handle = try validatedHandle("GraphicsDevice.Reset")
            var native = presentationParameters.nativeDescriptor()
            try runtime.functions.check(
                runtime.functions.graphicsDeviceResetWithParameters(
                    handle, &native, nil),
                operation: "cna_graphics_device_reset_with_parameters")
            runtime.cachedPresentationParameters = presentationParameters
        }

        /// `GraphicsDevice.Reset(PresentationParameters, GraphicsAdapter)`.
        ///
        /// **The adapter is carried, not discarded.** CNA's reset route ends
        /// in `const uint32_t* adapter_index`, which the two-argument overload
        /// passes as null -- "keep the adapter you have" -- and this one fills
        /// with the adapter's own index. A first draft of this member claimed
        /// the route took no adapter and quietly dropped the argument; reading
        /// the declaration rather than the first three parameters is what
        /// caught it.
        public func Reset(
            _ presentationParameters:
                Microsoft.Xna.Framework.Graphics.PresentationParameters,
            graphicsAdapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter
        ) throws {
            let handle = try validatedHandle("GraphicsDevice.Reset")
            var native = presentationParameters.nativeDescriptor()
            var index = graphicsAdapter.adapterIndex
            try runtime.functions.check(
                runtime.functions.graphicsDeviceResetWithParameters(
                    handle, &native, &index),
                operation: "cna_graphics_device_reset_with_parameters")
            runtime.cachedPresentationParameters = presentationParameters
            runtime.cachedAdapter = graphicsAdapter
        }

        internal func nativePresentationParameters() throws -> CNASwift_PresentationParameters {
            let handle = try validatedHandle("GraphicsDevice.PresentationParameters")
            var native = CNASwift_PresentationParameters()
            native.struct_size = UInt32(MemoryLayout<CNASwift_PresentationParameters>.size)
            native.struct_version = 1
            try runtime.functions.check(
                runtime.functions.graphicsDeviceGetPresentationParameters(handle, &native),
                operation: "cna_graphics_device_get_presentation_parameters")
            return native
        }

        /// `GraphicsDevice.get_DefaultClearOptions`, which is `private` in XNA
        /// and is what the colour-only `Clear` forwards with:
        ///
        ///     ClearOptions o = Target;
        ///     DepthFormat f = currentRenderTargetCount > 0
        ///         ? currentRenderTargets[0].depthFormat
        ///         : pInternalCachedParams.DepthStencilFormat;
        ///     if (f != DepthFormat.None) {
        ///         o = Target | DepthBuffer;
        ///         if (f == DepthFormat.Depth24Stencil8)
        ///             o = Target | DepthBuffer | Stencil;
        ///     }
        ///
        /// Both branches are reachable here: a set render target reports its
        /// own `DepthStencilFormat`, and otherwise CNA's presentation
        /// parameters report the backbuffer's.
        internal var defaultClearOptions: ClearOptions {
            get throws {
                let format: DepthFormat
                if let target = boundRenderTargetDepthFormat {
                    format = target
                } else {
                    format = DepthFormat(
                        rawValue: Int32(try nativePresentationParameters().depth_stencil_format))
                        ?? .None
                }
                guard format != .None else { return .Target }
                if format == .Depth24Stencil8 {
                    return [.Target, .DepthBuffer, .Stencil]
                }
                return [.Target, .DepthBuffer]
            }
        }

        // ------------------------------------------------------------------
        // Device state.
        //
        // The cache lives on `RuntimeState`, not here: this facade is built
        // fresh on every access because CNA's device handle is a per-callback
        // capability token rather than the device's identity, measured with
        // `build-probe/f42b_identity.c`. `RuntimeState` is the object with the
        // device's lifetime.
        // ------------------------------------------------------------------

        /// `GraphicsDevice.BlendState`.
        ///
        /// `ldfld cachedBlendState; ret` — a bare field read with no failure
        /// path, so the reader does not throw, and Optional because the field
        /// is null until something assigns one.
        public var BlendState: BlendState? { runtime.cachedBlendState }

        /// `GraphicsDevice.set_BlendState(BlendState value)`.
        ///
        /// The IL, in order: refuse null; return when the value is the same
        /// instance **and** the dirty flag is clear; end an active effect pass
        /// whose state flags include bit 1; `value.Apply(this)`; cache the
        /// instance; copy `cachedBlendFactor` and `cachedMultiSampleMask` out
        /// of it; clear the flag. There is no effect pass here yet, so that
        /// step has nothing to end.
        public func SetBlendState(_ value: BlendState?) throws {
            guard let value else {
                throw CNAArgumentNullException(
                    paramName: "value", message: GraphicsDevice.nullNotAllowedMessage)
            }
            if value === runtime.cachedBlendState && !runtime.blendStateDirty { return }
            let handle = try validatedHandle("GraphicsDevice.BlendState")
            try value.attach(to: self)
            var native = value.nativeDescriptor()
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetBlendState(handle, &native),
                operation: "cna_graphics_device_set_blend_state")
            runtime.cachedBlendState = value
            runtime.cachedBlendFactor = value.BlendFactor
            runtime.cachedMultiSampleMask = value.MultiSampleMask
            runtime.blendStateDirty = false
        }

        /// `GraphicsDevice.DepthStencilState`.
        public var DepthStencilState: DepthStencilState? {
            runtime.cachedDepthStencilState
        }

        /// `GraphicsDevice.set_DepthStencilState(DepthStencilState value)`.
        ///
        /// The same shape as `set_BlendState`, with its own dirty flag, its
        /// own effect-state bit (2), and `cachedReferenceStencil` as the one
        /// value it copies out.
        public func SetDepthStencilState(_ value: DepthStencilState?) throws {
            guard let value else {
                throw CNAArgumentNullException(
                    paramName: "value", message: GraphicsDevice.nullNotAllowedMessage)
            }
            if value === runtime.cachedDepthStencilState
                && !runtime.depthStencilStateDirty { return }
            let handle = try validatedHandle("GraphicsDevice.DepthStencilState")
            try value.attach(to: self)
            var native = value.nativeDescriptor()
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetDepthStencilState(handle, &native),
                operation: "cna_graphics_device_set_depth_stencil_state")
            runtime.cachedDepthStencilState = value
            runtime.cachedReferenceStencil = value.ReferenceStencil
            runtime.depthStencilStateDirty = false
        }

        /// `GraphicsDevice.RasterizerState`.
        public var RasterizerState: RasterizerState? { runtime.cachedRasterizerState }

        /// `GraphicsDevice.set_RasterizerState(RasterizerState value)`.
        ///
        /// **Not symmetric with the other two.** Its early-out is `beq` alone —
        /// the same instance always returns, with no dirty flag to force a
        /// re-apply — and it copies nothing out of the state. Writing all three
        /// from one template would be wrong in two different ways at once.
        public func SetRasterizerState(_ value: RasterizerState?) throws {
            guard let value else {
                throw CNAArgumentNullException(
                    paramName: "value", message: GraphicsDevice.nullNotAllowedMessage)
            }
            if value === runtime.cachedRasterizerState { return }
            let handle = try validatedHandle("GraphicsDevice.RasterizerState")
            try value.attach(to: self)
            var native = value.nativeDescriptor()
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetRasterizerState(handle, &native),
                operation: "cna_graphics_device_set_rasterizer_state")
            runtime.cachedRasterizerState = value
        }

        /// `GraphicsDevice.SamplerStates`.
        ///
        /// `ldfld pSamplerState; ret` — a bare field read with no failure path
        /// and no setter, so a plain non-throwing reader.
        ///
        /// **Optional**, and that is proven rather than defensive: the pinned
        /// verdict is `PROVEN_NULLABLE_SUCCESS` on the evidence that *no
        /// constructor of `GraphicsDevice` assigns `pSamplerState`* — the
        /// collection is built when the device is created, not when the object
        /// is, so a device that has not created its D3D device yet answers
        /// null. The analogue here is a facade whose handle no longer
        /// validates: a stale generation, or a read outside the callback that
        /// produced it.
        ///
        /// XNA builds it with offset `0`; it lives on the runtime here, for
        /// the reason recorded there.
        public var SamplerStates: SamplerStateCollection? {
            guard (try? validatedHandle("GraphicsDevice.SamplerStates")) != nil else {
                return nil
            }
            return runtime.samplerStates(for: self, vertex: false)
        }

        /// `GraphicsDevice.VertexSamplerStates`, built with offset `0x101` —
        /// `D3DVERTEXTEXTURESAMPLER0`. Optional on the same proof.
        public var VertexSamplerStates: SamplerStateCollection? {
            guard (try? validatedHandle("GraphicsDevice.VertexSamplerStates")) != nil else {
                return nil
            }
            return runtime.samplerStates(for: self, vertex: true)
        }

        /// `GraphicsDevice.Textures`.
        ///
        /// `ldfld pTextureCollection; ret` — a bare field read with no failure
        /// path and no setter, so a plain non-throwing reader, and **Optional**
        /// on the same proof `SamplerStates` carries: the pinned verdict is
        /// `PROVEN_NULLABLE_SUCCESS` on the evidence that *no constructor of
        /// `GraphicsDevice` assigns `pTextureCollection`*.
        public var Textures: TextureCollection? {
            guard (try? validatedHandle("GraphicsDevice.Textures")) != nil else {
                return nil
            }
            return runtime.textures(for: self, vertex: false)
        }

        /// `GraphicsDevice.VertexTextures`, built with offset `0x101` —
        /// `D3DVERTEXTEXTURESAMPLER0`. Optional on the same proof.
        ///
        /// **Zero slots on Reach**, whose `MaxVertexSamplers` is 0 in the
        /// extracted table. The collection exists and every index is out of
        /// range, which is exactly what XNA does: `new TextureCollection(this,
        /// 0x101, _profileCapabilities.MaxVertexSamplers)`.
        public var VertexTextures: TextureCollection? {
            guard (try? validatedHandle("GraphicsDevice.VertexTextures")) != nil else {
                return nil
            }
            return runtime.textures(for: self, vertex: true)
        }

        /// `GraphicsDevice.BlendFactor`.
        ///
        /// `ldflda cachedBlendFactor; ldobj; ret` — the same field
        /// `set_BlendState` copies out of the state it accepts, read directly.
        /// No failure path, so the reader does not throw.
        public var BlendFactor: Microsoft.Xna.Framework.Color {
            runtime.cachedBlendFactor
        }

        /// `GraphicsDevice.set_BlendFactor(Color value)`.
        ///
        /// A throwing writer method: the recorded verdict is
        /// `IL_REACHABLE_THROW` with `ObjectDisposedException`, because the
        /// setter opens with `Helpers.CheckDisposed(this, pComPtr)` and then
        /// pushes to the device. Reproduced with the same guard —
        /// `validatedHandle` raises the projected class — and CNA's own
        /// `cna_graphics_device_set_blend_factor`.
        public func SetBlendFactor(_ value: Microsoft.Xna.Framework.Color) throws {
            let handle = try validatedHandle("GraphicsDevice.BlendFactor")
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetBlendFactor(
                    handle,
                    CNASwift_Color(r: value.R, g: value.G, b: value.B, a: value.A)),
                operation: "cna_graphics_device_set_blend_factor")
            runtime.cachedBlendFactor = value
        }

        /// `GraphicsDevice.MultiSampleMask`, the other value `set_BlendState`
        /// copies out.
        public var MultiSampleMask: Int32 { runtime.cachedMultiSampleMask }

        /// `GraphicsDevice.set_MultiSampleMask(Int32 value)`.
        public func SetMultiSampleMask(_ value: Int32) throws {
            let handle = try validatedHandle("GraphicsDevice.MultiSampleMask")
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetMultiSampleMask(handle, value),
                operation: "cna_graphics_device_set_multi_sample_mask")
            runtime.cachedMultiSampleMask = value
        }

        /// `GraphicsDevice.ReferenceStencil`, the value
        /// `set_DepthStencilState` copies out.
        public var ReferenceStencil: Int32 { runtime.cachedReferenceStencil }

        /// `GraphicsDevice.set_ReferenceStencil(Int32 value)`.
        public func SetReferenceStencil(_ value: Int32) throws {
            let handle = try validatedHandle("GraphicsDevice.ReferenceStencil")
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetReferenceStencil(handle, value),
                operation: "cna_graphics_device_set_reference_stencil")
            runtime.cachedReferenceStencil = value
        }

        /// The exact `NullNotAllowed` message, read out of
        /// `Microsoft.Xna.Framework.dll`'s own resource table. All three
        /// setters raise `ArgumentNullException("value", NullNotAllowed)` —
        /// paramName **first**, because that overload is
        /// `.ctor(String paramName, String message)`.
        internal static let nullNotAllowedMessage =
            "This method does not accept null for this parameter."
        internal static let cannotReadActiveRenderTargetMessage =
            "Cannot use GetBackBufferData when a render target is active."

        internal func validatedHandle(_ operation: String) throws -> UInt64 {
            try runtime.validateGeneration(generation)
            try runtime.validateBorrowed(epoch: callbackEpoch, operation: operation)
            return handle
        }

        // ------------------------------------------------------------------
        // Vertex and index buffer binding.
        //
        // XNA keeps the bound objects in fields and hands the same objects
        // back; CNA can say which native handle is in a slot but deliberately
        // publishes no route from a native object back to a handle, and its own
        // header prescribes caching what you bind. Both facts point the same
        // way, so the managed objects are held on `RuntimeState` and the native
        // routes carry the binding to the device.

        /// `GraphicsDevice.SetVertexBuffer(VertexBuffer vertexBuffer)`.
        ///
        /// Twenty-nine bytes: build a `VertexBufferBinding` and hand it to the
        /// private `SetVertexBuffers(binding*, 1)`, or — when the buffer is
        /// null — call `SetVertexBuffers(null, 0)`, which **unbinds every
        /// stream**. That null branch reaches a normal `ret`, which is why the
        /// parameter is Optional.
        public func SetVertexBuffer(
            _ vertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer?
        ) throws {
            guard let vertexBuffer else { return try setVertexBuffers([]) }
            try setVertexBuffers([
                Microsoft.Xna.Framework.Graphics.VertexBufferBinding(vertexBuffer)
            ])
        }

        /// `GraphicsDevice.SetVertexBuffer(VertexBuffer, Int32 vertexOffset)`.
        ///
        /// The same, through the two-argument `VertexBufferBinding`
        /// constructor — so the offset is validated against the buffer's vertex
        /// count by the binding, before the device sees it.
        public func SetVertexBuffer(
            _ vertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer?,
            vertexOffset: Int32
        ) throws {
            guard let vertexBuffer else { return try setVertexBuffers([]) }
            try setVertexBuffers([
                Microsoft.Xna.Framework.Graphics.VertexBufferBinding(
                    vertexBuffer, vertexOffset)
            ])
        }

        /// `GraphicsDevice.SetVertexBuffers(VertexBufferBinding[] vertexBuffers)`.
        ///
        /// A null or empty array unbinds every stream, which is the same
        /// selected operation a null single buffer performs.
        public func SetVertexBuffers(
            _ vertexBuffers: [Microsoft.Xna.Framework.Graphics.VertexBufferBinding]?
        ) throws {
            try setVertexBuffers(vertexBuffers ?? [])
        }

        /// The private `SetVertexBuffers(VertexBufferBinding*, Int32)`.
        ///
        /// ```text
        /// if (count > _profileCapabilities.MaxVertexStreams)
        ///     ThrowNotSupportedException(ProfileMaxVertexStreams, MaxVertexStreams);
        /// for each binding:
        ///     if (binding._vertexBuffer == null)
        ///         throw new ArgumentException(NullNotAllowed);
        ///     if (binding._vertexBuffer.GraphicsDevice != this)
        ///         throw new InvalidOperationException(InvalidDevice);
        /// ```
        ///
        /// The null-buffer test cannot be reached through a Swift
        /// `VertexBufferBinding`, whose stored buffer is non-Optional and whose
        /// every constructor requires one; it is recorded rather than written.
        ///
        /// **The device-identity test is not comparing facades.** A facade is a
        /// per-callback token (Foundation 42) and two facades of the same device
        /// are different objects, so comparing them would refuse every binding
        /// made in a different callback from the one that created the buffer.
        /// The identity that survives a callback boundary is the `RuntimeState`,
        /// and that is what is compared — the same runtime is the same device.
        private func setVertexBuffers(
            _ bindings: [Microsoft.Xna.Framework.Graphics.VertexBufferBinding]
        ) throws {
            let handle = try validatedHandle("GraphicsDevice.SetVertexBuffers")
            let capabilities = profileCapabilities
            guard bindings.count <= Int(capabilities.maxVertexStreams) else {
                try capabilities.throwNotSupported(
                    Microsoft.Xna.Framework.Graphics.ProfileCapabilities
                        .profileMaxVertexStreams,
                    "\(capabilities.maxVertexStreams)")
            }
            for binding in bindings {
                guard binding.VertexBuffer.nativeStorage.runtime === runtime else {
                    throw CNAInvalidOperationException(
                        message: GraphicsDevice.invalidDeviceMessage)
                }
            }

            var native = try bindings.map { binding in
                CNASwift_VertexBufferBinding(
                    vertex_buffer: try binding.VertexBuffer.validatedHandle(
                        "GraphicsDevice.SetVertexBuffers"),
                    vertex_offset: binding.VertexOffset,
                    instance_frequency: binding.InstanceFrequency)
            }
            try runtime.functions.check(
                native.withUnsafeMutableBufferPointer { buffer in
                    runtime.functions.graphicsDeviceSetVertexBuffers(
                        handle, buffer.baseAddress, UInt64(buffer.count))
                },
                operation: "cna_graphics_device_set_vertex_buffers")
            runtime.cachedVertexBufferBindings = bindings
        }

        /// `GraphicsDevice.GetVertexBuffers()`.
        ///
        /// `new VertexBufferBinding[currentVertexBufferCount]` filled by
        /// `Array.Copy` from the device's own array — a **copy** of the managed
        /// bindings, so a caller mutating the result changes no binding. Swift's
        /// value-typed `Array` gives that for free.
        public func GetVertexBuffers()
            -> [Microsoft.Xna.Framework.Graphics.VertexBufferBinding] {
            runtime.cachedVertexBufferBindings
        }

        /// `GraphicsDevice.Indices`.
        ///
        /// `ldarg.0; ldfld _currentIB; ret` — a bare field read, so the getter
        /// neither throws nor is non-Optional.
        public var Indices: Microsoft.Xna.Framework.Graphics.IndexBuffer? {
            runtime.cachedIndexBuffer
        }

        /// `set_Indices`, whose IL has a direct throw and is therefore a
        /// throwing writer method rather than a Swift setter.
        ///
        /// ```text
        /// Helpers.CheckDisposed(this, pComPtr);
        /// if (value != null) Helpers.CheckDisposed(value, value.pComPtr);
        /// if (value == _currentIB) return;
        /// ...bind...
        /// ```
        ///
        /// The identity short-circuit is reproduced: binding the buffer that is
        /// already bound reaches no native route at all.
        public func SetIndices(
            _ value: Microsoft.Xna.Framework.Graphics.IndexBuffer?
        ) throws {
            let handle = try validatedHandle("GraphicsDevice.Indices")
            let bufferHandle = try value?.validatedHandle("GraphicsDevice.Indices") ?? 0
            guard value !== runtime.cachedIndexBuffer else { return }
            try runtime.functions.check(
                runtime.functions.graphicsDeviceSetIndexBuffer(handle, bufferHandle),
                operation: "cna_graphics_device_set_index_buffer")
            runtime.cachedIndexBuffer = value
        }

        /// `FrameworkResources.InvalidDevice`.
        /// `FrameworkResources.CannotSetAlreadyUsedRenderTarget`.
        internal static let cannotSetAlreadyUsedRenderTargetMessage =
            "The render target has already been set on another index. Each "
            + "render target may only be set on a single index at a time."
        /// `FrameworkResources.RenderTargetsMustMatch`.
        internal static let renderTargetsMustMatchMessage =
            "All active render targets must be the same size with the same "
            + "multisample type and bit depth."
        internal static let invalidDeviceMessage =
            "Resources can only be used on the GraphicsDevice that they were "
            + "created on. This resource was not created on this GraphicsDevice."

        internal var runtimeState: RuntimeState { runtime }
    }
}
