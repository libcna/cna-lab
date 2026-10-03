// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

extension Microsoft.Xna.Framework.Audio {

    /// `Microsoft.Xna.Framework.Audio.SoundEffect`.
    ///
    /// **Testable here without a single asset**, which is why this family came
    /// before `Model`: `cna_sound_effect_create_pcm16` takes raw PCM16LE bytes,
    /// and a test can write those. The two other creation routes -- an encoded
    /// file in memory, and a file on disk -- are `_ext`; only the first is
    /// bound, because `FromStream` consumes it and nothing consumes the other.
    ///
    /// Every getter is `IL_NO_FAILURE_PATH` and every setter is
    /// `IL_DIRECT_THROW`, so the shape is the one the accessor rule dictates:
    /// non-throwing properties, and writers named `Set<Name>` that throw.
    public final class SoundEffect: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var disposed = false

        /// The name is cached because `get_Name` cannot fail and the route can.
        private var storedName: String?

        /// `SoundEffect(Byte[] buffer, Int32 sampleRate, AudioChannels channels)`.
        public convenience init(
            buffer: [UInt8], sampleRate: Int32,
            channels: Microsoft.Xna.Framework.Audio.AudioChannels
        ) throws {
            try self.init(
                buffer: buffer, offset: 0, count: Int32(buffer.count),
                sampleRate: sampleRate, channels: channels,
                loopStart: 0, loopLength: 0, useRange: false)
        }

        /// `SoundEffect(Byte[], Int32 offset, Int32 count, Int32 sampleRate,
        /// AudioChannels, Int32 loopStart, Int32 loopLength)`.
        public convenience init(
            buffer: [UInt8], offset: Int32, count: Int32, sampleRate: Int32,
            channels: Microsoft.Xna.Framework.Audio.AudioChannels,
            loopStart: Int32, loopLength: Int32
        ) throws {
            try self.init(
                buffer: buffer, offset: offset, count: count,
                sampleRate: sampleRate, channels: channels,
                loopStart: loopStart, loopLength: loopLength, useRange: true)
        }

        /// The designated constructor both public ones reach.
        ///
        /// **The range overload is an `_ext` route**, and that is not a detail
        /// to hide: CNA publishes the seven-argument shape separately, so the
        /// two XNA constructors are two different calls rather than one call
        /// with defaults. `useRange` is what selects between them, and the
        /// three-argument constructor takes the canonical one.
        private init(
            buffer: [UInt8], offset: Int32, count: Int32, sampleRate: Int32,
            channels: Microsoft.Xna.Framework.Audio.AudioChannels,
            loopStart: Int32, loopLength: Int32, useRange: Bool
        ) throws {
            let rt = try RuntimeRegistry.current()
            self.runtime = rt
            try SoundEffect.validate(
                buffer: buffer, offset: offset, count: count,
                loopStart: loopStart, loopLength: loopLength, useRange: useRange)

            var info = CNASwift_SoundEffectCreateInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_SoundEffectCreateInfo>.size)
            info.struct_version = 1
            info.sample_rate = UInt32(bitPattern: sampleRate)
            info.channels = UInt32(bitPattern: channels.rawValue)

            var created: UInt64 = 0
            let result = buffer.withUnsafeBufferPointer { bytes -> UInt32 in
                withUnsafePointer(to: &info) { block in
                    if useRange {
                        return rt.functions.soundEffectCreatePcm16Range(
                            rt.gameHandle, block, bytes.baseAddress,
                            UInt64(bytes.count), offset, count,
                            loopStart, loopLength, &created)
                    }
                    return rt.functions.soundEffectCreatePcm16(
                        rt.gameHandle, block, bytes.baseAddress,
                        UInt64(bytes.count), &created)
                }
            }
            try rt.functions.check(
                result,
                operation: useRange
                    ? "cna_sound_effect_create_pcm16_range_ext"
                    : "cna_sound_effect_create_pcm16")
            handle = created
            rt.register(self)
        }

        /// XNA's four constructor refusals, reproduced because CNA makes
        /// none of them: it takes the byte count it is given and decodes what
        /// fits. Every message is read out of the registered
        /// `Microsoft.Xna.Framework.dll`, and every one was named by
        /// `tools/api_compat/message_coverage.py` rather than guessed at.
        ///
        /// PCM16 is two bytes a frame, which is the "block alignment" the
        /// texts refer to, and the reason an odd length is not a valid buffer.
        private static func validate(
            buffer: [UInt8], offset: Int32, count: Int32,
            loopStart: Int32, loopLength: Int32, useRange: Bool
        ) throws {
            let blockAlign: Int32 = 2
            let length = Int32(buffer.count)
            guard length > 0, length % blockAlign == 0 else {
                throw CNAArgumentException(
                    message: SoundEffect.invalidAudioBufferMessage)
            }
            guard useRange else { return }
            guard offset >= 0, offset < length, offset % blockAlign == 0 else {
                throw CNAArgumentException(
                    message: SoundEffect.invalidAudioBufferOffsetMessage)
            }
            guard count > 0, count % blockAlign == 0,
                  offset <= length - count else {
                throw CNAArgumentException(
                    message: SoundEffect.invalidOffsetCountLengthMessage)
            }
            let frames = count / blockAlign
            guard loopStart >= 0, loopLength >= 0,
                  loopStart <= frames, loopStart <= frames - loopLength else {
                throw CNAArgumentException(
                    message: SoundEffect.invalidLoopRegionMessage)
            }
        }

        /// `FrameworkResources.InvalidAudioBuffer`.
        internal static let invalidAudioBufferMessage =
            "Ensure that the buffer length is non-zero and meets the block "
            + "alignment requirements for the audio format."

        /// `FrameworkResources.InvalidAudioBufferOffset`.
        internal static let invalidAudioBufferOffsetMessage =
            "Offset must be within the buffer boundaries and meet the block "
            + "alignment requirements for the audio format."

        /// `FrameworkResources.InvalidOffsetCountLength`.
        internal static let invalidOffsetCountLengthMessage =
            "Ensure that count is valid and meets the block alignment "
            + "requirements for the audio format. Offset and count must define "
            + "a valid region within the buffer boundaries."

        /// `FrameworkResources.InvalidLoopRegion`.
        internal static let invalidLoopRegionMessage =
            "Ensure that the loop region is defined in samples and within the "
            + "buffer boundaries."

        /// `FrameworkResources.InvalidBufferSize`.
        internal static let invalidBufferSizeMessage =
            "Buffer size cannot be negative."

        /// The adoption path `FromStream` uses.
        private init(adopting created: UInt64, runtime rt: RuntimeState) {
            self.runtime = rt
            self.handle = created
            rt.register(self)
        }

        /// `SoundEffect.FromStream(Stream stream)`.
        ///
        /// XNA reads a RIFF/WAVE stream; CNA decodes whatever formats the build
        /// supports, which is a **wider** contract, not a narrower one. A build
        /// with no decoder for the bytes refuses with `NOT_SUPPORTED`, and that
        /// refusal is passed through.
        public static func FromStream(
            _ stream: Foundation.InputStream?
        ) throws -> SoundEffect {
            guard let stream else {
                throw CNAArgumentNullException(paramName: "stream")
            }
            let rt = try RuntimeRegistry.current()
            let encoded = try SoundEffect.readAll(stream)
            guard !encoded.isEmpty else {
                throw CNAError.streamFailure("encoded audio stream is empty")
            }
            var created: UInt64 = 0
            try rt.functions.check(
                encoded.withUnsafeBufferPointer { bytes in
                    rt.functions.soundEffectCreateFromEncoded(
                        rt.gameHandle, bytes.baseAddress,
                        UInt64(bytes.count), &created)
                },
                operation: "cna_sound_effect_create_from_encoded_ext")
            return SoundEffect(adopting: created, runtime: rt)
        }

        private static func readAll(_ stream: Foundation.InputStream) throws -> [UInt8] {
            if stream.streamStatus == .notOpen { stream.open() }
            var bytes: [UInt8] = []
            var chunk = [UInt8](repeating: 0, count: 8192)
            while stream.hasBytesAvailable {
                let read = chunk.withUnsafeMutableBufferPointer { buffer in
                    stream.read(buffer.baseAddress!, maxLength: buffer.count)
                }
                if read < 0 { throw CNAError.streamFailure("audio stream read failed") }
                if read == 0 { break }
                bytes.append(contentsOf: chunk[0..<read])
            }
            return bytes
        }

        /// `SoundEffect.IsDisposed`.
        public var IsDisposed: Bool { disposed }

        /// `SoundEffect.Name`, nullable and infallible in the getter.
        ///
        /// **Read from the runtime, not from the cache.** A cached-only getter
        /// would agree with a writer that never reached CNA -- and a mutation
        /// that removed the native call survived on exactly that. The cache is
        /// the fallback for a failed read, which an infallible getter needs,
        /// not the answer.
        public var Name: String? {
            guard let live = try? validatedHandle("SoundEffect.Name") else {
                return storedName
            }
            var size: UInt64 = 0
            guard runtime.functions.soundEffectGetNameSize(live, &size) == 0 else {
                return storedName
            }
            if size == 0 { return storedName == nil ? nil : "" }
            var bytes = [CChar](repeating: 0, count: Int(size))
            var written: UInt64 = 0
            let result = bytes.withUnsafeMutableBufferPointer { buffer in
                runtime.functions.soundEffectCopyName(
                    live, buffer.baseAddress, size, &written)
            }
            guard result == 0 else { return storedName }
            return String(decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                          as: UTF8.self)
        }

        /// `set_Name`, which throws `ArgumentNullException` on null and
        /// `ObjectDisposedException` on a released effect.
        public func SetName(_ value: String?) throws {
            guard let value else {
                throw CNAArgumentNullException(paramName: "value")
            }
            let live = try validatedHandle("SoundEffect.SetName")
            var utf8 = Array(value.utf8)
            try runtime.functions.check(
                SoundEffect.withStringView(&utf8) { view in
                    runtime.functions.soundEffectSetName(live, view)
                },
                operation: "cna_sound_effect_set_name")
            storedName = value
        }

        /// `SoundEffect.Duration`.
        ///
        /// Infallible, and the route is not, so a failed read answers zero --
        /// the same value XNA reports for an effect with no samples. The error
        /// is kept for a test rather than discarded.
        public var Duration: Swift.Duration {
            guard let live = try? validatedHandle("SoundEffect.Duration") else {
                return .seconds(0)
            }
            var ticks: Int64 = 0
            let result = runtime.functions.soundEffectGetDurationTicks(live, &ticks)
            guard result == 0 else {
                lastDurationFailure = CNAError.nativeFailure(
                    operation: "cna_sound_effect_get_duration_ticks",
                    result: result, message: "the effect could not report its duration")
                return .seconds(0)
            }
            lastDurationFailure = nil
            return SoundEffect.duration(fromTicks: ticks)
        }

        internal private(set) var lastDurationFailure: Error?

        /// `SoundEffect.Play()`.
        ///
        /// Returns whether a voice was available, which is the whole point of
        /// the Boolean: XNA answers false when the platform ran out.
        public func Play() throws -> Bool {
            let live = try validatedHandle("SoundEffect.Play")
            var played: UInt8 = 0
            try runtime.functions.check(
                runtime.functions.soundEffectPlay(live, &played),
                operation: "cna_sound_effect_play")
            return played != 0
        }

        /// `SoundEffect.Play(Single volume, Single pitch, Single pan)`.
        public func Play(_ volume: Float, pitch: Float, pan: Float) throws -> Bool {
            let live = try validatedHandle("SoundEffect.Play")
            var played: UInt8 = 0
            try runtime.functions.check(
                runtime.functions.soundEffectPlayWithSettings(
                    live, volume, pitch, pan, &played),
                operation: "cna_sound_effect_play_with_settings")
            return played != 0
        }

        /// `SoundEffect.CreateInstance()`.
        public func CreateInstance() throws
            -> Microsoft.Xna.Framework.Audio.SoundEffectInstance
        {
            let live = try validatedHandle("SoundEffect.CreateInstance")
            var created: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.soundEffectCreateInstance(live, &created),
                operation: "cna_sound_effect_create_instance")
            return Microsoft.Xna.Framework.Audio.SoundEffectInstance(
                handle: created, runtime: runtime)
        }

        /// `SoundEffect.Dispose()`.
        public func Dispose() throws {
            guard !disposed else { return }
            disposed = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.soundEffectDestroy(live),
                operation: "cna_sound_effect_destroy")
        }

        // MARK: - The four static settings, which live on the game

        /// `SoundEffect.MasterVolume`. Static in XNA, and CNA's routes take
        /// the **game** handle rather than an effect: the setting belongs to
        /// the audio device, not to any one sound.
        ///
        /// The four getters are spelled out rather than routed through one
        /// helper. A helper that returns a `@convention(c)` function through a
        /// Swift closure crashes SILGen in this toolchain
        /// ("bridging in re-abstraction thunk?"), and four short bodies are a
        /// better trade than a workaround nobody would recognise later.
        ///
        /// An infallible static getter with no runtime answers **zero**: not a
        /// crash, and not a plausible-looking default a caller would trust.
        public static var MasterVolume: Float {
            guard let rt = try? RuntimeRegistry.current() else { return 0 }
            var value: Float = 0
            guard rt.functions.soundEffectGetMasterVolume(
                rt.gameHandle, &value) == 0 else { return 0 }
            return value
        }

        public static func SetMasterVolume(_ value: Float) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.soundEffectSetMasterVolume(rt.gameHandle, value),
                operation: "cna_sound_effect_set_master_volume")
        }

        public static var DistanceScale: Float {
            guard let rt = try? RuntimeRegistry.current() else { return 0 }
            var value: Float = 0
            guard rt.functions.soundEffectGetDistanceScale(
                rt.gameHandle, &value) == 0 else { return 0 }
            return value
        }

        public static func SetDistanceScale(_ value: Float) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.soundEffectSetDistanceScale(rt.gameHandle, value),
                operation: "cna_sound_effect_set_distance_scale")
        }

        public static var DopplerScale: Float {
            guard let rt = try? RuntimeRegistry.current() else { return 0 }
            var value: Float = 0
            guard rt.functions.soundEffectGetDopplerScale(
                rt.gameHandle, &value) == 0 else { return 0 }
            return value
        }

        public static func SetDopplerScale(_ value: Float) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.soundEffectSetDopplerScale(rt.gameHandle, value),
                operation: "cna_sound_effect_set_doppler_scale")
        }

        public static var SpeedOfSound: Float {
            guard let rt = try? RuntimeRegistry.current() else { return 0 }
            var value: Float = 0
            guard rt.functions.soundEffectGetSpeedOfSound(
                rt.gameHandle, &value) == 0 else { return 0 }
            return value
        }

        public static func SetSpeedOfSound(_ value: Float) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.soundEffectSetSpeedOfSound(rt.gameHandle, value),
                operation: "cna_sound_effect_set_speed_of_sound")
        }

        // MARK: - The two static conversions

        /// `SoundEffect.GetSampleSizeInBytes(TimeSpan, Int32, AudioChannels)`.
        public static func GetSampleSizeInBytes(
            _ duration: Swift.Duration, sampleRate: Int32,
            channels: Microsoft.Xna.Framework.Audio.AudioChannels
        ) throws -> Int32 {
            let rt = try RuntimeRegistry.current()
            var bytes: Int32 = 0
            try rt.functions.check(
                rt.functions.soundEffectGetSampleSizeInBytes(
                    SoundEffect.ticks(from: duration), sampleRate,
                    UInt32(bitPattern: channels.rawValue), &bytes),
                operation: "cna_sound_effect_get_sample_size_in_bytes")
            return bytes
        }

        /// `SoundEffect.GetSampleDuration(Int32, Int32, AudioChannels)`.
        public static func GetSampleDuration(
            _ sizeInBytes: Int32, sampleRate: Int32,
            channels: Microsoft.Xna.Framework.Audio.AudioChannels
        ) throws -> Swift.Duration {
            guard sizeInBytes >= 0 else {
                throw CNAArgumentException(
                    message: SoundEffect.invalidBufferSizeMessage)
            }
            let rt = try RuntimeRegistry.current()
            var ticks: Int64 = 0
            try rt.functions.check(
                rt.functions.soundEffectGetSampleDurationTicks(
                    sizeInBytes, sampleRate,
                    UInt32(bitPattern: channels.rawValue), &ticks),
                operation: "cna_sound_effect_get_sample_duration_ticks")
            return SoundEffect.duration(fromTicks: ticks)
        }

        // MARK: - Support

        internal static let ticksPerSecond: Int64 = 10_000_000

        internal static func duration(fromTicks ticks: Int64) -> Swift.Duration {
            Swift.Duration(
                secondsComponent: ticks / ticksPerSecond,
                attosecondsComponent: (ticks % ticksPerSecond) * 100_000_000_000)
        }

        internal static func ticks(from duration: Swift.Duration) -> Int64 {
            let parts = duration.components
            return parts.seconds * ticksPerSecond
                + parts.attoseconds / 100_000_000_000
        }

        private func validatedHandle(_ operation: String) throws -> UInt64 {
            guard !disposed, handle != 0 else {
                // XNA passes its OWN text, not the BCL's generic one, and
                // message_coverage.py named the key. The object name is kept
                // beside it, which is what the two-argument overload is for.
                throw CNAObjectDisposedException(
                    objectName: "SoundEffect",
                    message: Microsoft.Xna.Framework.Audio
                        .objectDisposedMessage)
            }
            return handle
        }

        private static func withStringView(
            _ utf8: inout [UInt8], _ body: (CNASwift_StringView) -> UInt32
        ) -> UInt32 {
            utf8.withUnsafeMutableBufferPointer { buffer -> UInt32 in
                var view = CNASwift_StringView()
                view.data = UnsafeRawPointer(buffer.baseAddress)?
                    .assumingMemoryBound(to: CChar.self)
                view.byte_length = UInt64(buffer.count)
                return body(view)
            }
        }

        internal var runtimeObjectIsDisposed: Bool { disposed }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
