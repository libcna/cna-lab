// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Audio {
    public final class Cue: RuntimeOwnedChild, CNADisposable {
        private let runtime: RuntimeState
        private let storage: NativeHandleStorage
        private weak var parent: SoundBank?
        private let disposingSource = CNAEventSource<CNAEventArgs>()
        private let name: String
        private var fallback = CueSnapshot.created

        internal init(
            handle: UInt64, runtime: RuntimeState, parent: SoundBank
        ) throws {
            self.runtime = runtime
            self.parent = parent
            storage = NativeHandleStorage(
                handle: handle, typeName: "Cue", ownership: .owned,
                runtime: runtime, destroy: runtime.functions.cueDestroy)
            name = try XactSupport.copiedString(
                functions: runtime.functions,
                sizeOperation: "cna_cue_get_name_size",
                copyOperation: "cna_cue_copy_name",
                size: { runtime.functions.cueGetNameSize(handle, $0) },
                copy: { runtime.functions.cueCopyName(handle, $0, $1, $2) })
            runtime.register(self)
        }

        public var Name: String { name }
        public var IsCreated: Bool { snapshot().isCreated }
        public var IsDisposed: Bool { storage.isDisposed }
        public var IsPaused: Bool { snapshot().isPaused }
        public var IsPlaying: Bool { snapshot().isPlaying }
        public var IsPrepared: Bool { snapshot().isPrepared }
        public var IsPreparing: Bool { snapshot().isPreparing }
        public var IsStopped: Bool { snapshot().isStopped }
        public var IsStopping: Bool { snapshot().isStopping }
        public var Disposing: CNAEvent<CNAEventArgs> { disposingSource.Event }

        public func Play() throws {
            try call("Cue.Play", runtime.functions.cuePlay,
                     native: "cna_cue_play")
            fallback = .playing
        }

        public func Pause() throws {
            try call("Cue.Pause", runtime.functions.cuePause,
                     native: "cna_cue_pause")
            fallback.isPaused = true
            fallback.isPlaying = false
        }

        public func Resume() throws {
            try call("Cue.Resume", runtime.functions.cueResume,
                     native: "cna_cue_resume")
            fallback.isPaused = false
            fallback.isPlaying = true
        }

        public func Stop(_ options: AudioStopOptions) throws {
            let live = try storage.validatedHandle("Cue.Stop")
            try runtime.functions.check(
                runtime.functions.cueStop(
                    live, UInt32(bitPattern: options.rawValue)),
                operation: "cna_cue_stop")
            fallback = .stopped
        }

        public func GetVariable(_ name: String?) throws -> Float {
            let name = try XactSupport.requiredString(name, parameter: "name")
            let live = try storage.validatedHandle("Cue.GetVariable")
            var value: Float = 0
            try runtime.functions.check(
                XactSupport.withStringView(name) {
                    runtime.functions.cueGetVariable(live, $0, &value)
                }, operation: "cna_cue_get_variable")
            return value
        }

        public func SetVariable(_ name: String?, value: Float) throws {
            let name = try XactSupport.requiredString(name, parameter: "name")
            let live = try storage.validatedHandle("Cue.SetVariable")
            try runtime.functions.check(
                XactSupport.withStringView(name) {
                    runtime.functions.cueSetVariable(live, $0, value)
                }, operation: "cna_cue_set_variable")
        }

        public func Apply3D(
            _ listener: Microsoft.Xna.Framework.Audio.AudioListener?,
            emitter: Microsoft.Xna.Framework.Audio.AudioEmitter?
        ) throws {
            guard let listener else {
                throw CNAArgumentNullException(
                    paramName: "listener", message: XactSupport.nullNotAllowed)
            }
            guard let emitter else {
                throw CNAArgumentNullException(
                    paramName: "emitter", message: XactSupport.nullNotAllowed)
            }
            let live = try storage.validatedHandle("Cue.Apply3D")
            var nativeListener = listener.nativeDescriptor()
            var nativeEmitter = emitter.nativeDescriptor()
            try runtime.functions.check(
                withUnsafePointer(to: &nativeListener) { listenerPointer in
                    withUnsafePointer(to: &nativeEmitter) { emitterPointer in
                        runtime.functions.cueApply3D(
                            live, listenerPointer, emitterPointer)
                    }
                }, operation: "cna_cue_apply_3d")
        }

        public func Dispose() throws {
            guard !storage.isDisposed else { return }
            var firstError: Error?
            do { try disposingSource.Raise(self, args: CNAEventArgs.Empty) }
            catch { firstError = error }
            do { try storage.dispose(operation: "cna_cue_destroy") }
            catch { if firstError == nil { firstError = error } }
            fallback = .disposed
            if let firstError { throw firstError }
        }

        private func call(
            _ operation: String, _ route: (UInt64) -> UInt32,
            native: String
        ) throws {
            let live = try storage.validatedHandle(operation)
            try runtime.functions.check(route(live), operation: native)
        }

        private func snapshot() -> CueSnapshot {
            guard !storage.isDisposed,
                  let live = try? storage.validatedHandle("Cue.state") else {
                return storage.isDisposed ? .disposed : fallback
            }
            var info = CNASwift_CueInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_CueInfo>.size)
            info.struct_version = 1
            guard runtime.functions.cueGetInfo(live, &info) == 0 else {
                return fallback
            }
            let value = CueSnapshot(info)
            fallback = value
            return value
        }

        internal var runtimeObjectIsDisposed: Bool { IsDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
private struct CueSnapshot {
    var isCreated: Bool
    var isPaused: Bool
    var isPlaying: Bool
    var isPrepared: Bool
    var isPreparing: Bool
    var isStopped: Bool
    var isStopping: Bool

    static let created = CueSnapshot(
        isCreated: true, isPaused: false, isPlaying: false,
        isPrepared: false, isPreparing: false, isStopped: false,
        isStopping: false)
    static let playing = CueSnapshot(
        isCreated: false, isPaused: false, isPlaying: true,
        isPrepared: true, isPreparing: false, isStopped: false,
        isStopping: false)
    static let stopped = CueSnapshot(
        isCreated: false, isPaused: false, isPlaying: false,
        isPrepared: true, isPreparing: false, isStopped: true,
        isStopping: false)
    static let disposed = CueSnapshot(
        isCreated: false, isPaused: false, isPlaying: false,
        isPrepared: false, isPreparing: false, isStopped: true,
        isStopping: false)

    init(
        isCreated: Bool, isPaused: Bool, isPlaying: Bool,
        isPrepared: Bool, isPreparing: Bool, isStopped: Bool,
        isStopping: Bool
    ) {
        self.isCreated = isCreated
        self.isPaused = isPaused
        self.isPlaying = isPlaying
        self.isPrepared = isPrepared
        self.isPreparing = isPreparing
        self.isStopped = isStopped
        self.isStopping = isStopping
    }

    init(_ info: CNASwift_CueInfo) {
        self.init(
            isCreated: info.is_created != 0,
            isPaused: info.is_paused != 0,
            isPlaying: info.is_playing != 0,
            isPrepared: info.is_prepared != 0,
            isPreparing: info.is_preparing != 0,
            isStopped: info.is_stopped != 0,
            isStopping: info.is_stopping != 0)
    }
}
