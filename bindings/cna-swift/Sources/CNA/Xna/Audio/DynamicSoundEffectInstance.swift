// SPDX-License-Identifier: MIT

import CNAShim

/// The rooted context a `BufferNeeded` subscription carries.
///
/// The native callback is a plain C function pointer with a `void*`, and this
/// box is what it addresses: retained across the subscription, released with
/// it, and holding the instance **weakly** so a subscription cannot keep a
/// sound alive. The same shape `RenderTargetContentLostBox` uses, for the same
/// reasons.
internal final class DynamicBufferNeededBox {
    weak var target: Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance?
    init(_ target: Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance) {
        self.target = target
    }
}

internal let dynamicBufferNeededCallback: CNASwift_AudioEventCallback = { context in
    guard let context else { return }
    let box = Unmanaged<DynamicBufferNeededBox>.fromOpaque(context)
        .takeUnretainedValue()
    box.target?.nativeBufferNeeded()
}

extension Microsoft.Xna.Framework.Audio {

    /// `Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance`.
    ///
    /// Built from a sample rate and a channel count rather than from an asset,
    /// which is what makes it reachable here while the XACT family is not.
    ///
    /// XNA declares `IsLooped` and `Play` again on this sealed subclass. Swift
    /// can express `Play` and the fallible writer as overrides. It cannot
    /// override the base's infallible property getter with a fallible getter,
    /// so the accessor is projected as `GetIsLooped()` beside
    /// `SetIsLooped(_:)`; the strict verifier recombines those two compiler
    /// symbols into XNA's one property identity.
    public final class DynamicSoundEffectInstance: SoundEffectInstance {

        private let dynamicRuntime: RuntimeState
        private var dynamicHandle: UInt64
        private let bufferNeededSource = CNAEventSource<CNAEventArgs>()
        private var registration: UInt64 = 0
        private var box: Unmanaged<DynamicBufferNeededBox>?

        /// `DynamicSoundEffectInstance(Int32 sampleRate, AudioChannels channels)`.
        public init(
            sampleRate: Int32,
            channels: Microsoft.Xna.Framework.Audio.AudioChannels
        ) throws {
            let rt = try RuntimeRegistry.current()
            var created: UInt64 = 0
            try rt.functions.check(
                rt.functions.dynamicSoundEffectInstanceCreate(
                    rt.gameHandle, sampleRate,
                    UInt32(bitPattern: channels.rawValue), &created),
                operation: "cna_dynamic_sound_effect_instance_create")
            dynamicRuntime = rt
            dynamicHandle = created
            super.init(handle: created, runtime: rt)
            try subscribeToBufferNeeded()
        }

        /// `DynamicSoundEffectInstance.PendingBufferCount`.
        ///
        /// `IL_DIRECT_THROW`, so a throwing getter -- unlike the four the base
        /// declares, which cannot fail.
        public var PendingBufferCount: Int32 {
            get throws {
                let live = try validatedDynamicHandle()
                var count: Int32 = 0
                try dynamicRuntime.functions.check(
                    dynamicRuntime.functions
                        .dynamicSoundEffectInstanceGetPendingBufferCount(live, &count),
                    operation: "cna_dynamic_sound_effect_instance_get_pending_buffer_count")
                return count
            }
        }

        /// `DynamicSoundEffectInstance.BufferNeeded`.
        ///
        /// Raised from CNA's own subscription with this instance as sender and
        /// `EventArgs.Empty`, the shape every XNA `EventHandler<EventArgs>`
        /// raise site uses.
        public var BufferNeeded: CNAEvent<CNAEventArgs> { bufferNeededSource.Event }

        /// The getter of `DynamicSoundEffectInstance.IsLooped`.
        ///
        /// Unlike the base getter, XNA first refuses a disposed instance and
        /// otherwise always returns false. The named method is the required
        /// Swift spelling because a throwing getter cannot override the
        /// inherited nonthrowing property.
        public func GetIsLooped() throws -> Bool {
            _ = try validatedDynamicHandle()
            return false
        }

        /// The setter of `DynamicSoundEffectInstance.IsLooped`.
        ///
        /// False is accepted as a no-op; true is never supported by a dynamic
        /// instance. Disposal is checked first, matching the pinned IL.
        public override func SetIsLooped(_ value: Bool) throws {
            _ = try validatedDynamicHandle()
            guard !value else {
                throw CNAInvalidOperationException(
                    message: DynamicSoundEffectInstance.dynamicInvalidIsLoopedCallMessage)
            }
        }

        /// `DynamicSoundEffectInstance.Play()` is a distinct virtual member in
        /// XNA. CNA dispatches the shared route by handle kind, while this
        /// override preserves the declaring-type identity in Swift.
        public override func Play() throws {
            try super.Play()
        }

        /// `SubmitBuffer(Byte[] buffer)`.
        public func SubmitBuffer(_ buffer: [UInt8]?) throws {
            guard let buffer else {
                throw CNAArgumentNullException(paramName: "buffer")
            }
            try SubmitBuffer(buffer, offset: 0, count: Int32(buffer.count))
        }

        /// `SubmitBuffer(Byte[] buffer, Int32 offset, Int32 count)`.
        ///
        /// The same block-alignment rules the `SoundEffect` constructor
        /// enforces, and for the same reason: CNA takes the count it is given.
        public func SubmitBuffer(
            _ buffer: [UInt8]?, offset: Int32, count: Int32
        ) throws {
            guard let buffer else {
                throw CNAArgumentNullException(paramName: "buffer")
            }
            let blockAlign: Int32 = 2
            let length = Int32(buffer.count)
            guard length > 0, length % blockAlign == 0 else {
                throw CNAArgumentException(
                    message: Microsoft.Xna.Framework.Audio.SoundEffect
                        .invalidAudioBufferMessage)
            }
            guard offset >= 0, offset < length, offset % blockAlign == 0 else {
                throw CNAArgumentException(
                    message: Microsoft.Xna.Framework.Audio.SoundEffect
                        .invalidAudioBufferOffsetMessage)
            }
            guard count > 0, count % blockAlign == 0, offset <= length - count else {
                throw CNAArgumentException(
                    message: Microsoft.Xna.Framework.Audio.SoundEffect
                        .invalidOffsetCountLengthMessage)
            }
            let live = try validatedDynamicHandle()
            // **Measured: CNA accepts more than XNA does.** Submitting 101
            // buffers to this runtime leaves 101 pending; XNA refuses past 64,
            // because its own mixer cannot hold more. Without this the queue
            // grows until something else runs out, which is a worse failure
            // than being told the limit.
            guard try PendingBufferCount < DynamicSoundEffectInstance.pendingBufferLimit else {
                throw CNAInvalidOperationException(
                    message: DynamicSoundEffectInstance.overTheInstancePacketLimitMessage)
            }
            try dynamicRuntime.functions.check(
                buffer.withUnsafeBufferPointer { bytes in
                    dynamicRuntime.functions.dynamicSoundEffectInstanceSubmitBuffer(
                        live, bytes.baseAddress, UInt64(bytes.count), offset, count)
                },
                operation: "cna_dynamic_sound_effect_instance_submit_buffer")
        }

        /// The number XNA's message names, and the one it enforces.
        internal static let pendingBufferLimit: Int32 = 64

        /// `FrameworkResources.OverTheInstancePacketLimit`, read out of the
        /// registered `Microsoft.Xna.Framework.dll`.
        internal static let overTheInstancePacketLimitMessage =
            "Please ensure that there are less than 64 buffers pending on this "
            + "instance."

        /// `FrameworkResources.InvalidDynamicIsLoopedCall`, pinned from the
        /// registered XNA Framework resource table.
        internal static let dynamicInvalidIsLoopedCallMessage =
            "IsLooped property is not supported for DynamicSoundEffectInstance."

        /// `GetSampleDuration(Int32 sizeInBytes)`.
        ///
        /// The instance answers rather than a static: its own sample rate and
        /// channel count are what the conversion needs.
        public func GetSampleDuration(_ sizeInBytes: Int32) throws -> Swift.Duration {
            guard sizeInBytes >= 0 else {
                throw CNAArgumentException(
                    message: Microsoft.Xna.Framework.Audio.SoundEffect
                        .invalidBufferSizeMessage)
            }
            let live = try validatedDynamicHandle()
            var ticks: Int64 = 0
            try dynamicRuntime.functions.check(
                dynamicRuntime.functions
                    .dynamicSoundEffectInstanceGetSampleDurationTicks(
                        live, sizeInBytes, &ticks),
                operation: "cna_dynamic_sound_effect_instance_get_sample_duration_ticks")
            return Microsoft.Xna.Framework.Audio.SoundEffect.duration(fromTicks: ticks)
        }

        /// `GetSampleSizeInBytes(TimeSpan duration)`.
        public func GetSampleSizeInBytes(_ duration: Swift.Duration) throws -> Int32 {
            let live = try validatedDynamicHandle()
            var bytes: Int32 = 0
            try dynamicRuntime.functions.check(
                dynamicRuntime.functions
                    .dynamicSoundEffectInstanceGetSampleSizeInBytes(
                        live,
                        Microsoft.Xna.Framework.Audio.SoundEffect.ticks(from: duration),
                        &bytes),
                operation: "cna_dynamic_sound_effect_instance_get_sample_size_in_bytes")
            return bytes
        }

        /// `protected override void Dispose(Boolean disposing)`.
        ///
        /// The subscription goes first: a native callback addressing a released
        /// box is the one ordering mistake this type can make.
        public override func Dispose(_ disposing: Bool) throws {
            releaseSubscription()
            dynamicHandle = 0
            try super.Dispose(disposing)
        }

        internal func nativeBufferNeeded() {
            try? bufferNeededSource.Raise(self, args: CNAEventArgs.Empty)
        }

        /// Internal so a test can assert disposal released it.
        internal var bufferNeededRegistration: UInt64 { registration }

        private func subscribeToBufferNeeded() throws {
            let retained = Unmanaged.passRetained(DynamicBufferNeededBox(self))
            var out: UInt64 = 0
            let result = dynamicRuntime.functions
                .dynamicSoundEffectInstanceSubscribeBufferNeeded(
                    dynamicHandle, dynamicBufferNeededCallback,
                    retained.toOpaque(), &out)
            guard result == 0 else {
                retained.release()
                try dynamicRuntime.functions.check(
                    result,
                    operation: "cna_dynamic_sound_effect_instance_subscribe_buffer_needed")
                return
            }
            registration = out
            box = retained
        }

        private func releaseSubscription() {
            guard registration != 0 else { return }
            _ = dynamicRuntime.functions.audioUnsubscribe(registration)
            registration = 0
            box?.release()
            box = nil
        }

        private func validatedDynamicHandle() throws -> UInt64 {
            guard dynamicHandle != 0, !IsDisposed else {
                throw CNAObjectDisposedException(
                    objectName: "DynamicSoundEffectInstance",
                    message: Microsoft.Xna.Framework.Audio.objectDisposedMessage)
            }
            return dynamicHandle
        }

        deinit { releaseSubscription() }
    }
}
