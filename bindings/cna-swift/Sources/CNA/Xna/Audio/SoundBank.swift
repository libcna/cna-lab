// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Audio {
    open class SoundBank: XactEngineChild, CNADisposable {
        private let runtime: RuntimeState
        private let storage: NativeHandleStorage
        private let engine: AudioEngine
        private let disposingSource = CNAEventSource<CNAEventArgs>()
        private var cues: [WeakRuntimeChild] = []

        public init(audioEngine: AudioEngine?, filename: String?) throws {
            guard let audioEngine else {
                throw CNAArgumentNullException(
                    paramName: "audioEngine", message: XactSupport.nullNotAllowed)
            }
            let filename = try XactSupport.requiredString(
                filename, parameter: "filename")
            let rt = audioEngine.runtimeState
            let engineHandle = try audioEngine.validatedHandle("SoundBank.init")
            var created: UInt64 = 0
            let fullPath = XactSupport.fullPath(filename)
            try rt.functions.check(
                XactSupport.withStringView(fullPath) {
                    rt.functions.soundBankCreate(engineHandle, $0, &created)
                }, operation: "cna_sound_bank_create")
            runtime = rt
            engine = audioEngine
            storage = NativeHandleStorage(
                handle: created, typeName: "SoundBank", ownership: .owned,
                runtime: rt, destroy: rt.functions.soundBankDestroy)
            audioEngine.register(self)
            rt.register(self)
        }

        public var IsInUse: Bool { liveFlag(runtime.functions.soundBankGetIsInUse) }
        public var IsDisposed: Bool { storage.isDisposed }
        public var Disposing: CNAEvent<CNAEventArgs> { disposingSource.Event }

        public func GetCue(_ name: String?) throws -> Cue {
            let name = try XactSupport.requiredString(name, parameter: "name")
            let live = try storage.validatedHandle("SoundBank.GetCue")
            var created: UInt64 = 0
            try runtime.functions.check(
                XactSupport.withStringView(name) {
                    runtime.functions.soundBankGetCue(live, $0, &created)
                }, operation: "cna_sound_bank_get_cue")
            let cue = try Cue(handle: created, runtime: runtime, parent: self)
            cues.append(WeakRuntimeChild(cue))
            return cue
        }

        public func PlayCue(_ name: String?) throws {
            let name = try XactSupport.requiredString(name, parameter: "name")
            let live = try storage.validatedHandle("SoundBank.PlayCue")
            try runtime.functions.check(
                XactSupport.withStringView(name) {
                    runtime.functions.soundBankPlayCue(live, $0)
                }, operation: "cna_sound_bank_play_cue")
        }

        public func PlayCue(
            _ name: String?,
            listener: Microsoft.Xna.Framework.Audio.AudioListener?,
            emitter: Microsoft.Xna.Framework.Audio.AudioEmitter?
        ) throws {
            let name = try XactSupport.requiredString(name, parameter: "name")
            guard let listener else {
                throw CNAArgumentNullException(
                    paramName: "listener", message: XactSupport.nullNotAllowed)
            }
            guard let emitter else {
                throw CNAArgumentNullException(
                    paramName: "emitter", message: XactSupport.nullNotAllowed)
            }
            let live = try storage.validatedHandle("SoundBank.PlayCue")
            var nativeListener = listener.nativeDescriptor()
            var nativeEmitter = emitter.nativeDescriptor()
            try runtime.functions.check(
                withUnsafePointer(to: &nativeListener) { listenerPointer in
                    withUnsafePointer(to: &nativeEmitter) { emitterPointer in
                        XactSupport.withStringView(name) {
                            runtime.functions.soundBankPlayCue3D(
                                live, $0, listenerPointer, emitterPointer)
                        }
                    }
                }, operation: "cna_sound_bank_play_cue_3d")
        }

        public final func Dispose() throws { try Dispose(true) }

        open func Dispose(_ disposing: Bool) throws {
            guard !storage.isDisposed else { return }
            var firstError: Error?
            if disposing {
                do { try disposingSource.Raise(self, args: CNAEventArgs.Empty) }
                catch { firstError = error }
            }
            for child in cues.reversed() {
                guard let value = child.value, !value.runtimeObjectIsDisposed else { continue }
                do { try value.disposeFromParent() }
                catch { if firstError == nil { firstError = error } }
            }
            do { try storage.dispose(operation: "cna_sound_bank_destroy") }
            catch { if firstError == nil { firstError = error } }
            if let firstError { throw firstError }
        }

        private func liveFlag(
            _ route: (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
        ) -> Bool {
            guard let live = try? storage.validatedHandle("SoundBank.state") else {
                return false
            }
            var value: UInt8 = 0
            guard route(live, &value) == 0 else { return false }
            return value != 0
        }

        internal var runtimeObjectIsDisposed: Bool { IsDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
