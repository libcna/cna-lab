// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Audio {

    /// `Microsoft.Xna.Framework.Audio.AudioEngine` backed by CNA's canonical
    /// XACT engine routes.
    open class AudioEngine: RuntimeOwnedChild, CNADisposable {
        public static let ContentVersion: Int32 = 39

        private let runtime: RuntimeState
        private let storage: NativeHandleStorage
        private let disposingSource = CNAEventSource<CNAEventArgs>()
        private var children: [WeakRuntimeChild] = []
        private var categoryStorage: [AudioCategoryStorage] = []

        public convenience init(settingsFile: String?) throws {
            try self.init(
                settingsFile: settingsFile,
                lookAheadTime: .milliseconds(250), rendererId: "")
        }

        public init(
            settingsFile: String?, lookAheadTime: Swift.Duration,
            rendererId: String
        ) throws {
            let settings = try XactSupport.requiredString(
                settingsFile, parameter: "settingsFile")
            let rt = try RuntimeRegistry.current()
            var created: UInt64 = 0
            let fullPath = XactSupport.fullPath(settings)
            let result = XactSupport.withStringView(fullPath) { settingsView in
                XactSupport.withStringView(rendererId) { rendererView in
                    rt.functions.audioEngineCreateWithRenderer(
                        rt.gameHandle, settingsView,
                        Microsoft.Xna.Framework.Audio.SoundEffect.ticks(
                            from: lookAheadTime),
                        rendererView, &created)
                }
            }
            try rt.functions.check(
                result, operation: "cna_audio_engine_create_with_renderer")
            runtime = rt
            storage = NativeHandleStorage(
                handle: created, typeName: "AudioEngine", ownership: .owned,
                runtime: rt, destroy: rt.functions.audioEngineDestroy)
            rt.register(self)
        }

        public var RendererDetails:
            CNAReadOnlyCollection<Microsoft.Xna.Framework.Audio.RendererDetail>?
        {
            get throws {
                let live = try storage.validatedHandle("AudioEngine.RendererDetails")
                var count: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.audioEngineGetRendererCount(live, &count),
                    operation: "cna_audio_engine_get_renderer_count")
                guard count > 0 else { return nil }
                let list = CNAList<Microsoft.Xna.Framework.Audio.RendererDetail>()
                for index in 0..<count {
                    let friendlyName = try XactSupport.copiedString(
                        functions: runtime.functions,
                        sizeOperation: "cna_audio_engine_get_renderer_friendly_name_size",
                        copyOperation: "cna_audio_engine_copy_renderer_friendly_name",
                        size: { runtime.functions
                            .audioEngineGetRendererFriendlyNameSize(live, index, $0) },
                        copy: { runtime.functions
                            .audioEngineCopyRendererFriendlyName(live, index, $0, $1, $2) })
                    let rendererId = try XactSupport.copiedString(
                        functions: runtime.functions,
                        sizeOperation: "cna_audio_engine_get_renderer_id_size",
                        copyOperation: "cna_audio_engine_copy_renderer_id",
                        size: { runtime.functions
                            .audioEngineGetRendererIdSize(live, index, $0) },
                        copy: { runtime.functions
                            .audioEngineCopyRendererId(live, index, $0, $1, $2) })
                    list.Add(RendererDetail(
                        friendlyName: friendlyName, rendererId: rendererId))
                }
                return CNAReadOnlyCollection(list: list)
            }
        }

        public var IsDisposed: Bool { storage.isDisposed }
        public var Disposing: CNAEvent<CNAEventArgs> { disposingSource.Event }

        public func GetCategory(_ name: String?) throws -> AudioCategory {
            let name = try XactSupport.requiredString(name, parameter: "name")
            let live = try storage.validatedHandle("AudioEngine.GetCategory")
            var created: UInt64 = 0
            try runtime.functions.check(
                XactSupport.withStringView(name) {
                    runtime.functions.audioEngineGetCategory(live, $0, &created)
                }, operation: "cna_audio_engine_get_category")
            let box = AudioCategoryStorage(
                handle: created, name: name, parent: self, runtime: runtime)
            categoryStorage.append(box)
            return AudioCategory(storage: box)
        }

        public func GetGlobalVariable(_ name: String?) throws -> Float {
            let name = try XactSupport.requiredString(name, parameter: "name")
            let live = try storage.validatedHandle("AudioEngine.GetGlobalVariable")
            var value: Float = 0
            try runtime.functions.check(
                XactSupport.withStringView(name) {
                    runtime.functions.audioEngineGetGlobalVariable(live, $0, &value)
                }, operation: "cna_audio_engine_get_global_variable")
            return value
        }

        public func SetGlobalVariable(_ name: String?, value: Float) throws {
            let name = try XactSupport.requiredString(name, parameter: "name")
            let live = try storage.validatedHandle("AudioEngine.SetGlobalVariable")
            try runtime.functions.check(
                XactSupport.withStringView(name) {
                    runtime.functions.audioEngineSetGlobalVariable(live, $0, value)
                }, operation: "cna_audio_engine_set_global_variable")
        }

        public func Update() throws {
            let live = try storage.validatedHandle("AudioEngine.Update")
            try runtime.functions.check(
                runtime.functions.audioEngineUpdate(live),
                operation: "cna_audio_engine_update")
        }

        public final func Dispose() throws { try Dispose(true) }

        /// The projection of XNA's protected virtual `Dispose(Boolean)`.
        open func Dispose(_ disposing: Bool) throws {
            guard !storage.isDisposed else { return }
            var firstError: Error?
            if disposing {
                do { try disposingSource.Raise(self, args: CNAEventArgs.Empty) }
                catch { firstError = error }
            }
            for child in children.reversed() {
                guard let value = child.value, !value.runtimeObjectIsDisposed else { continue }
                do { try value.disposeFromParent() }
                catch { if firstError == nil { firstError = error } }
            }
            for category in categoryStorage.reversed() {
                do { try category.dispose() }
                catch { if firstError == nil { firstError = error } }
            }
            categoryStorage.removeAll()
            do { try storage.dispose(operation: "cna_audio_engine_destroy") }
            catch { if firstError == nil { firstError = error } }
            if let firstError { throw firstError }
        }

        internal func register(_ child: XactEngineChild) {
            children.removeAll { $0.value == nil }
            children.append(WeakRuntimeChild(child))
        }

        internal func validatedHandle(_ operation: String) throws -> UInt64 {
            try storage.validatedHandle(operation)
        }

        internal var runtimeState: RuntimeState { runtime }
        internal var runtimeObjectIsDisposed: Bool { IsDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
