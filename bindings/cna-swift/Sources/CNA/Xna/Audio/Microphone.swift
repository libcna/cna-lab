// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

internal final class MicrophoneBufferReadyBox {
    weak var target: Microsoft.Xna.Framework.Audio.Microphone?
    init(_ target: Microsoft.Xna.Framework.Audio.Microphone) { self.target = target }
}

internal let microphoneBufferReadyCallback: CNASwift_AudioEventCallback = { context in
    guard let context else { return }
    let box = Unmanaged<MicrophoneBufferReadyBox>.fromOpaque(context)
        .takeUnretainedValue()
    box.target?.nativeBufferReady()
}

extension Microsoft.Xna.Framework.Audio {
    public final class Microphone: RuntimeOwnedChild {
        private static let cacheLock = NSLock()
        private static var cacheGeneration: UInt64?
        private static var cachedAll: CNAReadOnlyCollection<Microphone>?
        private static var cachedByIndex: [UInt64: Microphone] = [:]

        private let runtime: RuntimeState
        private let generation: UInt64
        private let index: UInt64
        private let sampleRate: Int32
        private let isHeadset: Bool
        private let stateLock = NSLock()
        private var bufferDuration: Swift.Duration
        private let bufferReadySource = CNAEventSource<CNAEventArgs>()
        private var registration: UInt64 = 0
        private var callbackBox: Unmanaged<MicrophoneBufferReadyBox>?
        private var released = false

        public let Name: String

        private init(runtime: RuntimeState, index: UInt64) throws {
            self.runtime = runtime
            self.generation = runtime.generation
            self.index = index
            Name = try XactSupport.copiedString(
                functions: runtime.functions,
                sizeOperation: "cna_microphone_get_name_size_at",
                copyOperation: "cna_microphone_copy_name_at",
                size: { runtime.functions.microphoneGetNameSize(
                    runtime.gameHandle, index, $0) },
                copy: { runtime.functions.microphoneCopyName(
                    runtime.gameHandle, index, $0, $1, $2) })
            var rate: Int32 = 0
            try runtime.functions.check(
                runtime.functions.microphoneGetSampleRate(
                    runtime.gameHandle, index, &rate),
                operation: "cna_microphone_get_sample_rate_at")
            sampleRate = rate
            var headset: UInt8 = 0
            try runtime.functions.check(
                runtime.functions.microphoneGetIsHeadset(
                    runtime.gameHandle, index, &headset),
                operation: "cna_microphone_get_is_headset_at")
            isHeadset = headset != 0
            var ticks: Int64 = 0
            try runtime.functions.check(
                runtime.functions.microphoneGetBufferDuration(
                    runtime.gameHandle, index, &ticks),
                operation: "cna_microphone_get_buffer_duration_ticks_at")
            bufferDuration = SoundEffect.duration(fromTicks: ticks)
            runtime.register(self)
            try subscribe()
        }

        public static var All: CNAReadOnlyCollection<Microphone> {
            get throws {
                let runtime = try RuntimeRegistry.current()
                return try cacheLock.withLock {
                    if cacheGeneration == runtime.generation, let cachedAll {
                        return cachedAll
                    }
                    cachedAll = nil
                    cachedByIndex = [:]
                    cacheGeneration = nil
                    var count: UInt64 = 0
                    try runtime.functions.check(
                        runtime.functions.microphoneGetCount(
                            runtime.gameHandle, &count),
                        operation: "cna_microphone_get_count")
                    let list = CNAList<Microphone>()
                    for index in 0..<count {
                        let microphone = try Microphone(
                            runtime: runtime, index: index)
                        cachedByIndex[index] = microphone
                        list.Add(microphone)
                    }
                    let collection = CNAReadOnlyCollection(list: list)
                    cachedAll = collection
                    cacheGeneration = runtime.generation
                    return collection
                }
            }
        }

        public static var Default: Microphone? {
            get throws {
                let runtime = try RuntimeRegistry.current()
                _ = try All
                var index: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.microphoneGetDefaultIndex(
                        runtime.gameHandle, &index, &available),
                    operation: "cna_microphone_get_default_index_ext")
                guard available != 0 else { return nil }
                return cacheLock.withLock { cachedByIndex[index] }
            }
        }

        public var State: MicrophoneState {
            get throws {
                try validate("Microphone.State")
                var raw: UInt32 = 0
                try runtime.functions.check(
                    runtime.functions.microphoneGetState(
                        runtime.gameHandle, index, &raw),
                    operation: "cna_microphone_get_state_at")
                guard let state = MicrophoneState(
                    rawValue: Int32(bitPattern: raw)) else {
                    throw CNAError.nativeFailure(
                        operation: "Microphone.State", result: 1,
                        message: "native microphone state \(raw) is not an XNA value")
                }
                return state
            }
        }

        public var BufferDuration: Swift.Duration {
            stateLock.lock()
            defer { stateLock.unlock() }
            return bufferDuration
        }

        public func SetBufferDuration(_ value: Swift.Duration) throws {
            let ticks = SoundEffect.ticks(from: value)
            let tenMilliseconds: Int64 = 100_000
            guard ticks >= 1_000_000, ticks <= 10_000_000,
                  ticks % tenMilliseconds == 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "value",
                    message: "Microphone buffer duration must be between 100ms and 1sec and  10ms aligned.")
            }
            try validate("Microphone.BufferDuration")
            try runtime.functions.check(
                runtime.functions.microphoneSetBufferDuration(
                    runtime.gameHandle, index, ticks),
                operation: "cna_microphone_set_buffer_duration_ticks_at")
            stateLock.lock()
            bufferDuration = value
            stateLock.unlock()
        }

        public var SampleRate: Int32 { sampleRate }
        public var IsHeadset: Bool { isHeadset }
        public var BufferReady: CNAEvent<CNAEventArgs> { bufferReadySource.Event }

        public func Start() throws {
            try validate("Microphone.Start")
            try runtime.functions.check(
                runtime.functions.microphoneStart(runtime.gameHandle, index),
                operation: "cna_microphone_start_at")
        }

        public func Stop() throws {
            try validate("Microphone.Stop")
            try runtime.functions.check(
                runtime.functions.microphoneStop(runtime.gameHandle, index),
                operation: "cna_microphone_stop_at")
        }

        public func GetData(_ buffer: inout [UInt8]) throws -> Int32 {
            try GetData(&buffer, offset: 0, count: Int32(buffer.count))
        }

        public func GetData(
            _ buffer: inout [UInt8], offset: Int32, count: Int32
        ) throws -> Int32 {
            let length = Int32(buffer.count)
            guard length > 0, length % 2 == 0 else {
                throw CNAArgumentException(message: SoundEffect.invalidAudioBufferMessage)
            }
            guard offset >= 0, offset < length, offset % 2 == 0 else {
                throw CNAArgumentException(
                    message: SoundEffect.invalidAudioBufferOffsetMessage)
            }
            guard count > 0, count % 2 == 0,
                  offset <= length - count else {
                throw CNAArgumentException(
                    message: SoundEffect.invalidOffsetCountLengthMessage)
            }
            guard try State == .Started else { return 0 }
            var copied: UInt64 = 0
            let result = buffer.withUnsafeMutableBufferPointer { bytes in
                runtime.functions.microphoneGetData(
                    runtime.gameHandle, index,
                    bytes.baseAddress?.advanced(by: Int(offset)),
                    UInt64(count), &copied)
            }
            try runtime.functions.check(
                result, operation: "cna_microphone_get_data_at")
            guard copied <= UInt64(Int32.max) else {
                throw CNAError.nativeFailure(
                    operation: "Microphone.GetData", result: 1,
                    message: "native microphone read count is not representable")
            }
            return Int32(copied)
        }

        public func GetSampleDuration(_ sizeInBytes: Int32) throws -> Swift.Duration {
            guard sizeInBytes >= 0 else {
                throw CNAArgumentException(message: SoundEffect.invalidBufferSizeMessage)
            }
            guard sizeInBytes != 0 else { return .zero }
            try validate("Microphone.GetSampleDuration")
            var ticks: Int64 = 0
            try runtime.functions.check(
                runtime.functions.microphoneGetSampleDuration(
                    runtime.gameHandle, index, sizeInBytes, &ticks),
                operation: "cna_microphone_get_sample_duration_ticks_at")
            return SoundEffect.duration(fromTicks: ticks)
        }

        public func GetSampleSizeInBytes(_ duration: Swift.Duration) throws -> Int32 {
            let ticks = SoundEffect.ticks(from: duration)
            guard ticks >= 0, ticks <= Int64(Int32.max) * 10_000 else {
                throw CNAArgumentOutOfRangeException(paramName: "duration")
            }
            guard ticks != 0 else { return 0 }
            try validate("Microphone.GetSampleSizeInBytes")
            var bytes: Int32 = 0
            try runtime.functions.check(
                runtime.functions.microphoneGetSampleSize(
                    runtime.gameHandle, index, ticks, &bytes),
                operation: "cna_microphone_get_sample_size_in_bytes_at")
            return bytes
        }

        internal func nativeBufferReady() {
            try? bufferReadySource.Raise(self, args: CNAEventArgs.Empty)
        }

        private func subscribe() throws {
            let retained = Unmanaged.passRetained(MicrophoneBufferReadyBox(self))
            var out: UInt64 = 0
            let result = runtime.functions.microphoneSubscribeBufferReady(
                runtime.gameHandle, index, microphoneBufferReadyCallback,
                retained.toOpaque(), &out)
            guard result == 0 else {
                retained.release()
                try runtime.functions.check(
                    result, operation: "cna_microphone_subscribe_buffer_ready_at")
                return
            }
            registration = out
            callbackBox = retained
        }

        private func validate(_ operation: String) throws {
            guard !released else {
                throw CNAObjectDisposedException(objectName: "Microphone")
            }
            try runtime.validateGeneration(generation)
            try runtime.owner.validate(operation)
        }

        private func releaseSubscription() {
            guard !released else { return }
            released = true
            if registration != 0 {
                _ = runtime.functions.audioUnsubscribe(registration)
                registration = 0
            }
            callbackBox?.release()
            callbackBox = nil
        }

        internal var runtimeObjectIsDisposed: Bool { released }

        internal func disposeFromParent() throws {
            releaseSubscription()
            Microphone.cacheLock.withLock {
                guard Microphone.cacheGeneration == generation else { return }
                Microphone.cachedAll = nil
                Microphone.cachedByIndex = [:]
                Microphone.cacheGeneration = nil
            }
        }

        deinit { releaseSubscription() }
    }
}

private extension NSLock {
    func withLock<T>(_ body: () throws -> T) rethrows -> T {
        lock()
        defer { unlock() }
        return try body()
    }
}
