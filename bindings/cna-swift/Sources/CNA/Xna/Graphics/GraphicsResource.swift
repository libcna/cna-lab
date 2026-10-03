// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.GraphicsResource` projection.
    ///
    /// The root of the graphics resource hierarchy, and the reason
    /// `Texture2D` and `SpriteBatch` each had `BASE_MAPPING_MISMATCH` against
    /// them: XNA gives both this base, and a projection that dropped it left
    /// `IsDisposed`, `Name`, `Tag`, `GraphicsDevice`, `Dispose` and `Disposing`
    /// nowhere to live.
    ///
    /// The CLR class is `public abstract` with no accessible constructor, so
    /// this is an `open class` with `internal` construction: a consumer may
    /// derive from a concrete resource but cannot allocate a bare
    /// `GraphicsResource`, exactly as in XNA.
    ///
    /// ## What is native and what is managed
    ///
    /// The native handle, its ownership and its destruction are native. The
    /// four value properties are **not**, and the reason is the pinned
    /// fallibility verdicts: `get_IsDisposed`, `get_Name`, `set_Name`,
    /// `get_Tag`, `set_Tag` and `get_GraphicsDevice` are every one of them
    /// `IL_NO_FAILURE_PATH`, so their Swift projections must not throw — and a
    /// native round trip can always fail on the owner thread or a stale
    /// generation. `Name` and `Tag` are therefore managed fields, which is
    /// exactly what they are in the CLR (`_localName` and `_localTag`), and
    /// `GraphicsDevice` is the device facade the resource was created from.
    ///
    /// CNA's `cna_graphics_resource_get_tag`/`_set_tag` pair is deliberately
    /// **not** used for `Tag`: its parameter is a `CNA_GraphicsResourceTag`,
    /// documented as a "C-owned opaque tag token" and typed `uint64_t`. XNA's
    /// `Tag` is a `System.Object` reference, and squeezing a Swift object
    /// identity into an integer token would be a fabrication rather than a
    /// projection.
    open class GraphicsResource: RuntimeOwnedChild, CNADisposable {
        /// The native object this resource owns, when it has one.
        ///
        /// **Not every `GraphicsResource` is native.** XNA's own state objects
        /// — `BlendState`, `DepthStencilState`, `RasterizerState`,
        /// `SamplerState` — derive from this class and are plain settings a
        /// caller allocates before any device exists; CNA models them as POD
        /// descriptors and gives them no handle at all. A resource with no
        /// storage is that case, and it is disposed by a managed flag rather
        /// than by releasing something.
        internal let storage: NativeHandleStorage?
        private var managedDisposed = false
        private let device: GraphicsDevice?
        private let disposingSource = CNAEventSource<CNAEventArgs>()
        private var storedName: String?
        private var storedTag: Any?

        internal init(storage: NativeHandleStorage?, device: GraphicsDevice?) {
            self.storage = storage
            self.device = device
        }

        /// `GraphicsResource.IsDisposed`.
        ///
        /// `get_IsDisposed` is `ldarg.0; ldfld isDisposed; ret` — a field read
        /// with no failure path, so the Swift reader does not throw.
        public var IsDisposed: Bool { storage?.isDisposed ?? managedDisposed }

        /// `GraphicsResource.Name`.
        ///
        /// Nullable and infallible in both directions; no constructor assigns
        /// the field, so an unnamed resource reports nil rather than "".
        public var Name: String? {
            get { storedName }
            set { storedName = newValue }
        }

        /// `GraphicsResource.Tag`.
        ///
        /// `System.Object` projects to `Any?`. Nullable for the same reason as
        /// `Name`: no constructor assigns it.
        public var Tag: Any? {
            get { storedTag }
            set { storedTag = newValue }
        }

        /// `GraphicsResource.GraphicsDevice`.
        ///
        /// Proven nullable — no constructor assigns `_parent` — and
        /// infallible. The stored facade is the device the resource was
        /// created from; it validates its own callback scope when used, which
        /// is where a stale device is reported rather than here.
        public var GraphicsDevice: GraphicsDevice? { device }

        /// `GraphicsResource.Disposing`.
        ///
        /// Raised with `EventArgs.Empty` and the resource as sender, **after**
        /// the native release: `~GraphicsResource()` calls
        /// `!GraphicsResource()` first and only then invokes the handler, so a
        /// handler observes `IsDisposed == true`.
        public var Disposing: CNAEvent<CNAEventArgs> { disposingSource.Event }

        /// `GraphicsResource.Dispose()`.
        ///
        /// `virtual final` in the metadata — a sealed interface implementation
        /// and not an override point — so this is `final`. The body is
        /// `Dispose(true)` followed by `GC.SuppressFinalize(this)`; Swift has
        /// no finalizer to suppress, and `deinit` is the finalizer's language
        /// projection, so the suppression is the `storage.isDisposed` check
        /// `deinit` already performs.
        public final func Dispose() throws {
            try Dispose(true)
        }

        /// `protected virtual void Dispose(bool)`.
        ///
        /// The one override point of the family, and the flag decides what
        /// happens. The CLR body is two different paths, not one:
        ///
        /// ```text
        /// if (disposing) {
        ///     ~GraphicsResource();          // if (!isDisposed) {
        ///                                   //     isDisposed = true;
        ///                                   //     Disposing?.Invoke(this, EventArgs.Empty);
        ///                                   // }
        /// } else {
        ///     try     { !GraphicsResource(); }   // isDisposed = true, unconditionally
        ///     finally { Object.Finalize(); }
        /// }
        /// ```
        ///
        /// **`Disposing` is raised on the disposing path only.** The finalizer
        /// path sets the flag and raises nothing, because a finalizer must not
        /// reach other managed objects. Until Foundation 59 this projection
        /// raised the event on both paths, so `Dispose(false)` — which Swift's
        /// lack of `protected` puts within any consumer's reach — announced a
        /// disposal XNA announces to nobody. Reading the IL is what found it;
        /// no test failed.
        ///
        /// `Object.Finalize()` is empty and `!GraphicsResource()`'s unguarded
        /// assignment is unobservable once the flag is already set, so the
        /// early return covers both paths exactly.
        ///
        /// The native release lives here rather than in each subclass because
        /// this projection's storage does: in the CLR the COM pointer belongs
        /// to `Texture2D` and `SpriteBatch`, whose own overrides release it
        /// *before* calling this base. The order a handler can observe is the
        /// same either way — released first, then announced.
        ///
        /// Swift has no `protected`, so this is public — the same single
        /// widening `CNACollection.Items` makes.
        open func Dispose(_ disposing: Bool) throws {
            guard !IsDisposed else { return }
            if let storage {
                try storage.dispose(operation: "\(storage.typeName).Dispose")
            } else {
                managedDisposed = true
            }
            guard disposing else { return }
            try disposingSource.Raise(self, args: CNAEventArgs.Empty)
        }

        /// `GraphicsResource.ToString()`.
        ///
        /// Returns the name when it is neither null nor empty, and otherwise
        /// `Object.ToString()` — which the CLR answers with the object's own
        /// full type name. The Swift namespace enums mirror the CLR
        /// namespaces, so the reflected Swift name with this module's
        /// qualifier removed is that name.
        /// Non-Optional under the recorded deferral rule: the CIL proves this
        /// return neither nullable nor non-null — one site returns the name
        /// field and the other returns `Object.ToString()` — so the projection
        /// keeps the non-Optional shape until it is proven, rather than
        /// inventing a null XNA may never produce.
        public func ToString() -> String {
            if let storedName, !storedName.isEmpty { return storedName }
            return Microsoft.Xna.Framework.Graphics.GraphicsResource.clrTypeName(of: self)
        }

        internal static func clrTypeName(of value: Any) -> String {
            clrTypeName(ofType: type(of: value))
        }

        /// The same name for a metatype rather than a value.
        ///
        /// `System.Type` maps to the Swift metatype, and the three vertex-type
        /// messages format one into a `{0}`; the CLR formats a `Type` with its
        /// `ToString()`, which is the full name.
        internal static func clrTypeName(ofType type: Any.Type) -> String {
            let reflected = String(reflecting: type)
            let qualifier = moduleQualifier
            return reflected.hasPrefix(qualifier)
                ? String(reflected.dropFirst(qualifier.count))
                : reflected
        }

        private static let moduleQualifier: String = {
            let reflected = String(reflecting: GraphicsResource.self)
            guard let separator = reflected.firstIndex(of: ".") else { return "" }
            return String(reflected[...separator])
        }()

        internal func validatedHandle(_ operation: String) throws -> UInt64 {
            guard let storage else {
                // A state object has no native member that could reach here.
                // If one ever does, that is a defect in this binding and not a
                // consumer error, so it stays on the producer-invariant
                // channel rather than becoming a projected CLR failure.
                throw CNAError.producerInvariant(
                    "\(GraphicsResource.clrTypeName(of: self)) has no native "
                    + "object, so \(operation) has nothing to validate")
            }
            // Where XNA guards a graphics resource it does so with
            // `Helpers.CheckDisposed(this, pComPtr)`, whose whole body is
            //
            //     if (pComPtr == IntPtr.Zero)
            //         throw new ObjectDisposedException(obj.GetType().Name);
            //
            // `GetType()` — the **dynamic** type, unlike `ThrowIfBound` and the
            // four `Apply` methods, which use `ldtoken` on the declaring class.
            // `storage.typeName` is the derived name the subclass recorded, so
            // a disposed `RenderTarget2D` reports `RenderTarget2D`.
            //
            // XNA does NOT guard uniformly — `SpriteBatch` has no disposal
            // guard at all and `Texture2D` has one, inside `CopyData<T>`. This
            // binding guards every native handle before it crosses into CNA,
            // because handing a released handle to C is not an option here.
            // That is deliberately more guarding than XNA performs; what is
            // reproduced is the class and the payload, not the omission.
            guard !storage.isDisposed else {
                throw CNAObjectDisposedException(objectName: storage.typeName)
            }
            return try storage.validatedHandle(operation)
        }

        /// The native storage, for the subclasses that certainly have one.
        internal var nativeStorage: NativeHandleStorage {
            guard let storage else {
                preconditionFailure(
                    "\(GraphicsResource.clrTypeName(of: self)) has no native object; "
                    + "this accessor is internal and only native resources use it")
            }
            return storage
        }

        internal var runtimeState: RuntimeState { nativeStorage.runtime }
        internal var runtimeObjectIsDisposed: Bool { IsDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// The `Microsoft.Xna.Framework.Graphics.Texture` projection.
    ///
    /// The direct base of `Texture2D`, and the whole of its own declared
    /// surface is `LevelCount` and `Format`. Both CLR getters are
    /// `IL_NO_FAILURE_PATH`, so both are read **once** from
    /// `cna_texture_get_info` while the resource is being constructed and
    /// stored, exactly as `Texture2D.Width` and `.Height` already are: a
    /// native round trip on every read could fail, and an infallible getter
    /// cannot report that.
    open class Texture: GraphicsResource {
        /// `Texture.LevelCount`.
        public let LevelCount: Int32

        /// `Texture.Format`.
        public let Format: SurfaceFormat

        /// `Texture.isActiveRenderTarget` — an assembly-visible field, not a
        /// public member, so `internal`.
        ///
        /// XNA's private `SetRenderTargets` clears it on every target it
        /// unbinds and sets it on every target it binds, and **three** places
        /// read it: `TextureCollection.set_Item` and the `CopyData` of both
        /// `Texture2D` and `TextureCube`, each raising
        /// `InvalidOperationException(MustResolveRenderTarget)`. A render
        /// target cannot be sampled from, or written to, while it is the
        /// device's own target.
        ///
        /// CNA enforces the sampling half itself and says so:
        /// `cna_graphics_device_set_texture` on an active target answers
        /// `CNA_RESULT_INVALID_STATE` with *"A texture that is currently bound
        /// as a render target cannot be bound for sampling"*
        /// (`build-probe/f66_slots.c`). The two agree; the managed check runs
        /// first so the message is XNA's.
        internal var isActiveRenderTarget = false

        internal init(
            storage: NativeHandleStorage,
            device: GraphicsDevice?,
            levelCount: Int32,
            format: SurfaceFormat
        ) {
            LevelCount = levelCount
            Format = format
            super.init(storage: storage, device: device)
        }

        /// Reads the common texture facts a `Texture` carries, for a handle
        /// that is already owned by its caller.
        /// Drops this texture from every sampler slot the managed collections
        /// think it occupies, before the handle is released.
        ///
        /// CNA does the native half itself: `cna_texture2d_destroy` on a bound
        /// texture succeeds and the slot reads back empty afterwards
        /// (`build-probe/f66_slots.c`). Without this the managed cache would go
        /// on naming a disposed object the device no longer holds, and
        /// `Textures[i]` would answer it.
        open override func Dispose(_ disposing: Bool) throws {
            guard !IsDisposed else { return }
            if let storage {
                for collection in storage.runtime.liveTextureCollections {
                    collection.forget(self)
                }
            }
            try super.Dispose(disposing)
        }

        internal static func readCommonInfo(
            handle: UInt64, runtime: RuntimeState
        ) throws -> (levelCount: Int32, format: SurfaceFormat) {
            var info = CNASwift_TextureInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_TextureInfo>.size)
            info.struct_version = 1
            try runtime.functions.check(
                runtime.functions.textureCommonGetInfo(handle, &info),
                operation: "cna_texture_get_info"
            )
            guard info.level_count <= UInt32(Int32.max) else {
                throw CNAError.nativeFailure(
                    operation: "Texture.LevelCount", result: 10,
                    message: "level count exceeds the XNA Int32 range")
            }
            guard let format = SurfaceFormat(rawValue: Int32(bitPattern: info.format)) else {
                throw CNAError.nativeFailure(
                    operation: "Texture.Format", result: 1,
                    message: "native surface format \(info.format) is not an XNA SurfaceFormat")
            }
            return (Int32(info.level_count), format)
        }
    }
}
