// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

extension Microsoft.Xna.Framework.Content {

    /// `Microsoft.Xna.Framework.Content.ContentManager`.
    ///
    /// Ten members, and the interesting half is managed: the disposal test, the
    /// empty-name test, the path normalisation, and the cache — including what
    /// happens when a cached asset is the wrong type.
    ///
    /// **`Load` reuses `TitleContainer.GetCleanPath`.** XNA's does, in the
    /// instruction right after its argument checks, so an asset named
    /// `"a/../b"` and one named `"b"` are the same cache entry. Foundation 73
    /// built that routine from the same IL, and this calls it rather than
    /// reimplementing the agreement.
    open class ContentManager: RuntimeOwnedChild, CNADisposable {

        private let runtime: RuntimeState?
        private var handle: UInt64
        /// The runtime generation this manager's handle belongs to.
        ///
        /// Only `deinit` reads it, and only to refuse destroying a handle that
        /// a LATER runtime may have reissued. `NativeHandleStorage` keeps the
        /// same field for the same reason; this type does not use that storage
        /// (it has no `GraphicsResource` base and its `Dispose` is XNA's, not
        /// the storage's), so it keeps its own.
        private let generation: UInt64
        private let serviceProvider: (any CNAServiceProvider)?
        private var storedRootDirectory: String?

        /// XNA's `loadedAssets`, and **null is what "disposed" means here**:
        /// `Load` tests it before anything else and raises
        /// `ObjectDisposedException` when it is gone. Optional for the same
        /// reason.
        private var loadedAssets: [String: Any]?
        private var disposableAssets: [any CNADisposable]?
        private var openedStreamLengths: [ObjectIdentifier: StreamLengthRecord] = [:]

        private final class StreamLengthRecord {
            weak var stream: InputStream?
            let length: Int32

            init(stream: InputStream, length: Int32) {
                self.stream = stream
                self.length = length
            }
        }

        /// `ContentManager(IServiceProvider serviceProvider)`.
        public convenience init(
            serviceProvider: any CNAServiceProvider
        ) throws {
            try self.init(serviceProvider: serviceProvider, rootDirectory: "")
        }

        /// `ContentManager(IServiceProvider serviceProvider, String rootDirectory)`.
        ///
        /// The two-argument constructor sets `RootDirectory` **through the
        /// property**, so its null check runs — which is why a null root
        /// directory is refused at construction and not merely at assignment.
        public init(
            serviceProvider: any CNAServiceProvider,
            rootDirectory: String
        ) throws {
            let rt = RuntimeRegistry.currentIfAvailable()
            self.runtime = rt
            self.serviceProvider = serviceProvider
            self.storedRootDirectory = rootDirectory
            self.loadedAssets = [:]
            self.disposableAssets = []
            self.handle = 0
            self.generation = rt?.generation ?? 0

            if let rt {
                var device: UInt64 = 0
                try rt.functions.check(
                    rt.functions.gameGetGraphicsDevice(rt.gameHandle, &device),
                    operation: "cna_game_get_graphics_device")

                var utf8 = Array(rootDirectory.utf8)
                var created: UInt64 = 0
                try rt.functions.check(
                    ContentManager.withStringView(&utf8) { view in
                        var info = CNASwift_ContentManagerCreateInfo()
                        info.struct_size = UInt32(
                            MemoryLayout<CNASwift_ContentManagerCreateInfo>.size)
                        info.struct_version = 1
                        info.root_directory = view
                        return rt.functions.contentManagerCreate(
                            device, &info, &created)
                    },
                    operation: "cna_content_manager_create")
                handle = created

                // CNA requires native managers to be destroyed before their
                // game. A managed-only manager has no runtime child to join.
                rt.register(self)
            }
        }

        /// `ContentManager.ServiceProvider`.
        ///
        /// Infallible and a bare field read, so it is a stored value here too.
        public var ServiceProvider: (any CNAServiceProvider)? {
            serviceProvider
        }

        /// `ContentManager.RootDirectory`.
        ///
        /// The getter is infallible; **the setter raises
        /// `ArgumentNullException("value")`**, so it is a writer method taking
        /// an Optional — without which that branch could not be reached from
        /// Swift at all.
        public var RootDirectory: String? { storedRootDirectory }

        public func SetRootDirectory(_ value: String?) throws {
            guard let value else {
                throw CNAArgumentNullException(paramName: "value")
            }
            // **The half Foundation 87 shipped without.** The accessor table
            // records TWO exceptions for this setter, and only the null one was
            // reproduced; `tools/api_compat/message_coverage.py` named the
            // other by its resource key. Once anything has been loaded the root
            // is frozen, because every cached asset was resolved against it and
            // moving it would leave the cache describing paths that no longer
            // exist.
            if !(loadedAssets?.isEmpty ?? true) {
                throw CNAInvalidOperationException(
                    message: ContentManager.cannotChangeRootDirectoryMessage)
            }
            storedRootDirectory = value
        }

        /// `FrameworkResources.ContentManagerCannotChangeRootDirectory`, read
        /// out of the registered `Microsoft.Xna.Framework.dll`.
        internal static let cannotChangeRootDirectoryMessage =
            "This property cannot be changed after content has been loaded "
            + "into the ContentManager." 

        /// `ContentManager.Load<T>(String assetName)`.
        ///
        /// XNA's 199 bytes, in order:
        ///
        /// ```text
        /// if (loadedAssets == null) throw ObjectDisposedException(ToString());
        /// if (String.IsNullOrEmpty(assetName)) throw ArgumentNullException("assetName");
        /// assetName = TitleContainer.GetCleanPath(assetName);
        /// if (loadedAssets.TryGetValue(assetName, out cached)) {
        ///     if (!(cached is T)) throw ContentLoadException(BadXnbWrongType, …);
        ///     return (T)cached;
        /// }
        /// … ReadAsset<T> and store
        /// ```
        ///
        /// **The disposal test comes first**, before the name is even looked
        /// at, so `Load(null)` on a disposed manager reports the disposal.
        ///
        /// **A cache hit of the wrong type throws rather than reloading.** The
        /// message names the asset, the type found and the type asked for, in
        /// that order.
        public func Load<T>(_ assetName: String?) throws -> T {
            guard var assets = loadedAssets else {
                throw CNAObjectDisposedException(objectName: "\(type(of: self))")
            }
            guard let assetName, !assetName.isEmpty else {
                throw CNAArgumentNullException(paramName: "assetName")
            }
            let cleanName = Microsoft.Xna.Framework.TitleContainer.GetCleanPath(assetName)
            let key = cleanName.lowercased()

            // One cast site, reached by both paths. A freshly loaded asset
            // always matches, because `readAsset` picks its route from `T`;
            // a cached one need not, and that is the case `BadXnbWrongType`
            // was written for -- the same name loaded twice as two types.
            let asset: Any
            if let cached = assets[key] {
                asset = cached
            } else {
                let loaded: T = try ReadAsset(
                    cleanName, recordDisposableObject: nil)
                asset = loaded
                assets[key] = asset
                loadedAssets = assets
            }
            guard let typed = asset as? T else {
                throw Microsoft.Xna.Framework.Content.ContentLoadException(
                    message: ContentManager.badXnbWrongTypeMessage(
                        key, String(describing: type(of: asset)),
                        String(describing: T.self)))
            }
            return typed
        }

        /// The native acceleration half of `ReadAsset<T>`.
        ///
        /// XNA reads an `.xnb` header and picks a type reader from it. CNA
        /// publishes **one route per asset kind** instead —
        /// `load_texture2d`, `load_texture_cube`, `load_sprite_font`,
        /// `load_effect`, `load_sound_effect`, `load_model` — so the type
        /// argument selects the route. The managed reader path below now sees
        /// and validates XNB itself for every type without an admitted native
        /// loader.
        ///
        /// **Only `Texture2D` is wired here.** CNA publishes five more
        /// loaders -- cube, sprite font, effect, sound effect, model -- and
        /// none is bound, because adopting what they produce needs machinery
        /// those types own and this milestone does not build. A route with no
        /// consuming member is not bound, so they wait for the milestones that
        /// can receive them.
        ///
        /// A type selected for native acceleration but unavailable without a
        /// callback runtime is refused by name. Other types proceed through
        /// the real managed XNB path.
        private func readNativeAsset<T>(_ key: String, as: T.Type) throws -> Any {
            guard let runtime, handle != 0 else {
                throw Microsoft.Xna.Framework.Content.ContentLoadException(
                    message: ContentManager.noLoaderMessage(
                        String(describing: T.self)))
            }
            var utf8 = Array(key.utf8)
            var produced: UInt64 = 0
            let functions = runtime.functions

            func load(
                _ route: (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt64>?) -> UInt32,
                _ operation: String
            ) throws {
                try functions.check(
                    ContentManager.withStringView(&utf8) { view in
                        route(handle, view, &produced)
                    },
                    operation: operation)
            }

            switch T.self {
            case is Microsoft.Xna.Framework.Graphics.Texture2D.Type:
                try load(functions.contentManagerLoadTexture2d,
                         "cna_content_manager_load_texture2d")
                return try Microsoft.Xna.Framework.Graphics.Texture2D
                    .adoptLoaded(handle: produced, runtime: runtime)
            default:
                throw Microsoft.Xna.Framework.Content.ContentLoadException(
                    message: ContentManager.noLoaderMessage(
                        String(describing: T.self)))
            }
        }

        /// `ContentManager.ReadAsset<T>` widened from protected for Swift.
        /// Custom readers always use the managed XNB stream. Existing CNA
        /// typed loaders remain the backend for built-in asset types, so one
        /// cache entry never crosses two incompatible loading systems.
        open func ReadAsset<T>(
            _ assetName: String,
            recordDisposableObject: ((any CNADisposable) throws -> Void)?
        ) throws -> T {
            guard loadedAssets != nil else {
                throw CNAObjectDisposedException(objectName: "\(type(of: self))")
            }
            guard !assetName.isEmpty else {
                throw CNAArgumentNullException(paramName: "assetName")
            }

            if Self.isNativeBuiltIn(T.self) {
                let value = try readNativeAsset(assetName, as: T.self)
                guard let typed = value as? T else {
                    throw ContentLoadException(message: Self.badXnbWrongTypeMessage(
                        assetName, String(describing: type(of: value)),
                        String(describing: T.self)))
                }
                return typed
            }

            let stream = try OpenStream(assetName)
            let availableLength = TakeOpenedStreamLength(stream)
            let reader: ContentReader
            do {
                reader = try ContentReader.Create(
                    self, input: stream, assetName: assetName,
                    recordDisposableObject: recordDisposableObject,
                    availableLength: availableLength)
            } catch {
                stream.close()
                throw error
            }
            defer { try? reader.Close() }
            guard let result: T = try reader.readAsset() else {
                throw ContentLoadException(message: Self.badXnbWrongTypeMessage(
                    assetName, "null", String(describing: T.self)))
            }
            return result
        }

        /// `ContentManager.OpenStream`, widened from protected for Swift.
        open func OpenStream(_ assetName: String) throws -> InputStream {
            let root = storedRootDirectory ?? ""
            let combined = root.isEmpty
                ? assetName + ".xnb"
                : root + "/" + assetName + ".xnb"
            let clean = Microsoft.Xna.Framework.TitleContainer.GetCleanPath(combined)

            do {
                // The pinned runtime is the Windows XNA build. FileStream and
                // TitleContainer both reject these filename characters before
                // any missing-file decision; Linux accepts several of them,
                // so reproduce the authority's failure ordering explicitly.
                if Self.hasInvalidWindowsAssetPathCharacter(assetName) {
                    throw CNAArgumentException(
                        message: Microsoft.Xna.Framework.TitleContainer
                            .invalidTitleContainerNameMessage)
                }
                if Self.isAbsolutePath(root) || runtime == nil {
                    let hostPath = clean.replacingOccurrences(of: "\\", with: "/")
                    guard FileManager.default.fileExists(atPath: hostPath),
                          let stream = InputStream(fileAtPath: hostPath) else {
                        let inner = CNAFileNotFoundException(
                            message: Microsoft.Xna.Framework.TitleContainer
                                .openStreamNotFoundMessage(assetName))
                        throw ContentLoadException(
                            message: Microsoft.Xna.Framework.TitleContainer
                                .openStreamNotFoundMessage(assetName),
                            innerException: inner)
                    }
                    if let size = try? FileManager.default.attributesOfItem(
                        atPath: hostPath)[.size] as? NSNumber {
                        RecordOpenedStreamLength(stream, size.uint64Value)
                    }
                    return stream
                }
                return try Microsoft.Xna.Framework.TitleContainer.OpenStream(clean)
            } catch let error as ContentLoadException {
                throw error
            } catch let error as CNAFileNotFoundException {
                throw ContentLoadException(
                    message: Microsoft.Xna.Framework.TitleContainer
                        .openStreamNotFoundMessage(assetName),
                    innerException: error)
            } catch let error as CNAException {
                throw ContentLoadException(
                    message: Microsoft.Xna.Framework.TitleContainer
                        .openStreamErrorMessage(assetName),
                    innerException: error)
            }
        }

        internal func RecordDisposableObject(
            _ disposable: any CNADisposable
        ) throws {
            guard disposableAssets != nil else {
                throw CNAObjectDisposedException(objectName: "\(type(of: self))")
            }
            disposableAssets?.append(disposable)
        }

        internal func RecordOpenedStreamLength(
            _ stream: InputStream, _ length: UInt64
        ) {
            let bounded = length > UInt64(Int32.max) ? Int32.max : Int32(length)
            openedStreamLengths[ObjectIdentifier(stream)] = StreamLengthRecord(
                stream: stream, length: bounded)
        }

        private func TakeOpenedStreamLength(_ stream: InputStream) -> Int32? {
            guard let record = openedStreamLengths.removeValue(
                forKey: ObjectIdentifier(stream)),
                  record.stream === stream else { return nil }
            return record.length
        }

        /// `ContentManager.Unload()`.
        ///
        /// Releases every asset the manager loaded and empties the cache. The
        /// manager stays usable afterwards, which is what separates `Unload`
        /// from `Dispose`.
        public func Unload() throws {
            guard loadedAssets != nil else {
                throw CNAObjectDisposedException(objectName: "\(type(of: self))")
            }
            let disposables = disposableAssets ?? []
            defer {
                loadedAssets = [:]
                disposableAssets = []
                openedStreamLengths.removeAll(keepingCapacity: false)
            }
            for disposable in disposables { try disposable.Dispose() }
            if let runtime, handle != 0 {
                try runtime.functions.check(
                    runtime.functions.contentManagerUnload(handle),
                    operation: "cna_content_manager_unload")
            }
        }

        /// `ContentManager.Dispose()` and `Dispose(Boolean)`.
        ///
        /// **Nulling the cache is what makes the manager disposed**, because
        /// that is the field `Load` tests. Disposing twice is a no-op, as
        /// XNA's is.
        public func Dispose() throws {
            try Dispose(true)
        }

        open func Dispose(_ disposing: Bool) throws {
            guard loadedAssets != nil else { return }
            if disposing { try Unload() }
            loadedAssets = nil
            disposableAssets = nil
            if let runtime, handle != 0 {
                try runtime.functions.check(
                    runtime.functions.contentManagerDestroy(handle),
                    operation: "cna_content_manager_destroy")
                handle = 0
            }
        }

        /// `FrameworkResources.BadXnbWrongType`, a three-argument format.
        internal static let badXnbWrongType =
            "Error loading \"{0}\". File contains {1} but trying to load as {2}."

        internal static func badXnbWrongTypeMessage(
            _ asset: String, _ found: String, _ wanted: String
        ) -> String {
            badXnbWrongType
                .replacingOccurrences(of: "{0}", with: asset)
                .replacingOccurrences(of: "{1}", with: found)
                .replacingOccurrences(of: "{2}", with: wanted)
        }

        /// Not a Microsoft string: XNA has no such case, because it reads the
        /// type out of the `.xnb` rather than being told one.
        internal static func noLoaderMessage(_ wanted: String) -> String {
            "This runtime keeps projected built-in content on one route per "
            + "asset kind, and has none for \(wanted). That built-in type "
            + "cannot safely fall through to the custom managed .xnb reader."
        }

        /// The size this binding declares to CNA, exposed so a test can
        /// assert the value the *binding* sees rather than the one a test
        /// module computes from its own copy of the header.
        internal static let createInfoSize =
            MemoryLayout<CNASwift_ContentManagerCreateInfo>.size

        /// The cache entry a successful `Load` would have made.
        ///
        /// The frozen-root rule needs a manager that has loaded something, and
        /// this host has no `.xnb` to load -- so without a hook the rule is
        /// unprovable and a mutation that removes it survives. Internal, and
        /// named for what it is, exactly as `GraphicsDeviceManager`'s own test
        /// hook is.
        internal func testOnlyRecordLoadedAsset(_ key: String) {
            loadedAssets?[key.lowercased()] = key
        }

        internal var runtimeObjectIsDisposed: Bool { loadedAssets == nil }
        internal func disposeFromParent() throws { try Dispose() }

        /// Releases a handle the consumer dropped without disposing.
        ///
        /// Every other owned type in this binding routes its handle through
        /// `NativeHandleStorage`, whose `deinit` does exactly this. This type
        /// did not, and was therefore the ONE owned type whose handle survived
        /// its Swift object -- which `cna_game_destroy` then refuses to
        /// destroy the game around:
        ///
        ///     All owned C child resources must be destroyed before the game.
        ///
        /// The template's canary hit it on every run and had been failing at
        /// teardown, in exactly the pattern a consumer writes: read
        /// `Game.Content`, install one of your own with `SetContent`, and let
        /// the first one go. The registry's weak reference is nil by the time
        /// `Game.Dispose` walks it.
        ///
        /// This is not XNA's finalizer -- the CLR `ContentManager` declares
        /// none, and calls `GC.SuppressFinalize` over nothing. It is the
        /// opposite of a divergence: in .NET a dropped manager does not stop
        /// the game being disposed, and without this it does here.
        ///
        /// The guards are `NativeHandleStorage.deinit`'s, for its reasons: a
        /// dead runtime has already released everything, a newer generation
        /// may have reissued this handle number, and destroying from a foreign
        /// thread is not allowed.
        deinit {
            guard let runtime,
                  handle != 0,
                  runtime.isActive,
                  runtime.generation == generation,
                  runtime.owner.isCurrent else { return }
            if runtime.functions.contentManagerDestroy(handle) == 0 { handle = 0 }
        }

        private static func isAbsolutePath(_ path: String) -> Bool {
            path.hasPrefix("/")
                || path.hasPrefix("\\")
                || (path.count >= 3
                    && path[path.index(path.startIndex, offsetBy: 1)] == ":")
        }

        private static func hasInvalidWindowsAssetPathCharacter(
            _ path: String
        ) -> Bool {
            path.unicodeScalars.contains { scalar in
                scalar.value == 0
                    || scalar.value < 0x20
                    || "\"<>|*?".unicodeScalars.contains(scalar)
            }
        }

        private static func isNativeBuiltIn<T>(_ type: T.Type) -> Bool {
            type is Microsoft.Xna.Framework.Graphics.Texture2D.Type
                || type is Microsoft.Xna.Framework.Graphics.TextureCube.Type
                || type is Microsoft.Xna.Framework.Graphics.SpriteFont.Type
                || type is Microsoft.Xna.Framework.Graphics.Effect.Type
                || type is Microsoft.Xna.Framework.Audio.SoundEffect.Type
                || type is Microsoft.Xna.Framework.Graphics.Model.Type
        }

        private static func withStringView(
            _ utf8: inout [UInt8], _ body: (CNASwift_StringView) -> UInt32
        ) -> UInt32 {
            utf8.withUnsafeMutableBufferPointer { buffer -> UInt32 in
                var view = CNASwift_StringView()
                view.byte_length = UInt64(buffer.count)
                guard let base = buffer.baseAddress else { return body(view) }
                return base.withMemoryRebound(to: CChar.self, capacity: buffer.count) {
                    view.data = UnsafePointer($0)
                    return body(view)
                }
            }
        }
    }
}
