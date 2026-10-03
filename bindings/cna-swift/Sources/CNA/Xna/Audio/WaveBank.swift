// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Audio {
    open class WaveBank: XactEngineChild, CNADisposable {
        private let runtime: RuntimeState
        private let storage: NativeHandleStorage
        private let engine: AudioEngine
        private let disposingSource = CNAEventSource<CNAEventArgs>()

        public init(audioEngine: AudioEngine?, nonStreamingWaveBankFilename: String?) throws {
            guard let audioEngine else {
                throw CNAArgumentNullException(
                    paramName: "audioEngine", message: XactSupport.nullNotAllowed)
            }
            let filename = try XactSupport.requiredString(
                nonStreamingWaveBankFilename,
                parameter: "nonStreamingWaveBankFilename")
            let rt = audioEngine.runtimeState
            let engineHandle = try audioEngine.validatedHandle("WaveBank.init")
            var created: UInt64 = 0
            let fullPath = XactSupport.fullPath(filename)
            try rt.functions.check(
                XactSupport.withStringView(fullPath) {
                    rt.functions.waveBankCreate(engineHandle, $0, &created)
                }, operation: "cna_wave_bank_create")
            runtime = rt
            engine = audioEngine
            storage = NativeHandleStorage(
                handle: created, typeName: "WaveBank", ownership: .owned,
                runtime: rt, destroy: rt.functions.waveBankDestroy)
            audioEngine.register(self)
            rt.register(self)
        }

        public init(
            audioEngine: AudioEngine?, streamingWaveBankFilename: String?,
            offset: Int32, packetsize: Int16
        ) throws {
            guard let audioEngine else {
                throw CNAArgumentNullException(
                    paramName: "audioEngine", message: XactSupport.nullNotAllowed)
            }
            let filename = try XactSupport.requiredString(
                streamingWaveBankFilename,
                parameter: "streamingWaveBankFilename")
            let rt = audioEngine.runtimeState
            let engineHandle = try audioEngine.validatedHandle("WaveBank.init")
            var created: UInt64 = 0
            try rt.functions.check(
                XactSupport.withStringView(filename) {
                    rt.functions.waveBankCreateStreaming(
                        engineHandle, $0, offset, packetsize, &created)
                }, operation: "cna_wave_bank_create_streaming")
            runtime = rt
            engine = audioEngine
            storage = NativeHandleStorage(
                handle: created, typeName: "WaveBank", ownership: .owned,
                runtime: rt, destroy: rt.functions.waveBankDestroy)
            audioEngine.register(self)
            rt.register(self)
        }

        public var IsInUse: Bool { liveFlag(runtime.functions.waveBankGetIsInUse) }
        public var IsPrepared: Bool { liveFlag(runtime.functions.waveBankGetIsPrepared) }
        public var IsDisposed: Bool { storage.isDisposed }
        public var Disposing: CNAEvent<CNAEventArgs> { disposingSource.Event }

        public final func Dispose() throws { try Dispose(true) }

        open func Dispose(_ disposing: Bool) throws {
            guard !storage.isDisposed else { return }
            var firstError: Error?
            if disposing {
                do { try disposingSource.Raise(self, args: CNAEventArgs.Empty) }
                catch { firstError = error }
            }
            do { try storage.dispose(operation: "cna_wave_bank_destroy") }
            catch { if firstError == nil { firstError = error } }
            if let firstError { throw firstError }
        }

        private func liveFlag(
            _ route: (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32
        ) -> Bool {
            guard let live = try? storage.validatedHandle("WaveBank.state") else {
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
