// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Audio {

    /// `Microsoft.Xna.Framework.Audio.SoundEffectInstance`.
    ///
    /// **Unsealed in the metadata**, so unsealed here, with `Dispose(Boolean)`
    /// as its override point. There is no public constructor: XNA hands one out
    /// from `SoundEffect.CreateInstance`, and so does this.
    ///
    /// The four controllable properties are read from **one** route --
    /// `cna_sound_effect_instance_get_info` answers state, loop flag, volume,
    /// pitch and pan together -- which is what lets four `IL_NO_FAILURE_PATH`
    /// getters share a single fallible call instead of four.
    public class SoundEffectInstance: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var disposed = false

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.handle = handle
            runtime.register(self)
        }

        /// `SoundEffectInstance.IsDisposed`.
        public var IsDisposed: Bool { disposed }

        /// `SoundEffectInstance.State`.
        ///
        /// **The one throwing getter in the pair**, and the table is why:
        /// `get_State` is `IL_DIRECT_THROW`, where the other four getters have
        /// no failure path at all. It reads live rather than from a snapshot,
        /// because the state changes without anyone assigning it -- a sound
        /// that finishes moves to Stopped on its own.
        public var State: Microsoft.Xna.Framework.Audio.SoundState {
            get throws {
                let info = try readInfo("SoundEffectInstance.State")
                guard let state = Microsoft.Xna.Framework.Audio.SoundState(
                    rawValue: Int32(bitPattern: info.state)) else {
                    throw CNAError.nativeFailure(
                        operation: "SoundEffectInstance.State", result: 1,
                        message: "native sound state \(info.state) is not an XNA SoundState")
                }
                return state
            }
        }

        /// `SoundEffectInstance.Volume`, `Pitch`, `Pan` and `IsLooped`.
        ///
        /// All four getters are `IL_NO_FAILURE_PATH`, so none may refuse. They
        /// read the same info route and answer the value CNA last accepted; a
        /// failed read answers the last value this facade wrote, and the error
        /// is kept rather than dropped.
        public var Volume: Float { readFloat(\.volume, stored: storedVolume) }
        public var Pitch: Float { readFloat(\.pitch, stored: storedPitch) }
        public var Pan: Float { readFloat(\.pan, stored: storedPan) }

        public var IsLooped: Bool {
            guard let info = try? readInfo("SoundEffectInstance.IsLooped") else {
                return storedIsLooped
            }
            lastInfoFailure = nil
            return info.is_looped != 0
        }

        private var storedVolume: Float = 1
        private var storedPitch: Float = 0
        private var storedPan: Float = 0
        private var storedIsLooped = false
        internal private(set) var lastInfoFailure: Error?

        private func readFloat(
            _ field: KeyPath<CNASwift_SoundEffectInstanceInfo, Float>, stored: Float
        ) -> Float {
            guard let info = try? readInfo("SoundEffectInstance.read") else { return stored }
            lastInfoFailure = nil
            return info[keyPath: field]
        }

        private func readInfo(
            _ operation: String
        ) throws -> CNASwift_SoundEffectInstanceInfo {
            let live = try validatedHandle(operation)
            var info = CNASwift_SoundEffectInstanceInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_SoundEffectInstanceInfo>.size)
            info.struct_version = 1
            do {
                try runtime.functions.check(
                    runtime.functions.soundEffectInstanceGetInfo(live, &info),
                    operation: "cna_sound_effect_instance_get_info")
            } catch {
                lastInfoFailure = error
                throw error
            }
            return info
        }

        /// `set_Volume`. XNA refuses anything outside `[0, 1]`; CNA passes the
        /// value through **without clamping**, which its own header says, so
        /// the range check is the binding's.
        public func SetVolume(_ value: Float) throws {
            guard value >= 0, value <= 1 else {
                throw CNAArgumentOutOfRangeException(paramName: "value")
            }
            let live = try validatedHandle("SoundEffectInstance.SetVolume")
            try runtime.functions.check(
                runtime.functions.soundEffectInstanceSetVolume(live, value),
                operation: "cna_sound_effect_instance_set_volume")
            storedVolume = value
        }

        /// `set_Pitch`. XNA's range is `[-1, 1]`, and CNA clamps to the same
        /// one -- so a value outside it would be silently accepted as the
        /// nearest legal one instead of refused. The check is reproduced for
        /// that reason: a clamp and a refusal are different answers.
        public func SetPitch(_ value: Float) throws {
            guard value >= -1, value <= 1 else {
                throw CNAArgumentOutOfRangeException(paramName: "value")
            }
            let live = try validatedHandle("SoundEffectInstance.SetPitch")
            try runtime.functions.check(
                runtime.functions.soundEffectInstanceSetPitch(live, value),
                operation: "cna_sound_effect_instance_set_pitch")
            storedPitch = value
        }

        /// `set_Pan`, whose range is `[-1, 1]` as well.
        public func SetPan(_ value: Float) throws {
            guard value >= -1, value <= 1 else {
                throw CNAArgumentOutOfRangeException(paramName: "value")
            }
            // Pan is a 2D control: a sound placed in space has its position
            // from the emitter instead, and XNA refuses rather than letting
            // the two fight. The rule also closes after the first Play.
            guard !is3D, !hasPlayed else {
                throw CNAInvalidOperationException(
                    message: SoundEffectInstance.invalidPanCallMessage)
            }
            let live = try validatedHandle("SoundEffectInstance.SetPan")
            try runtime.functions.check(
                runtime.functions.soundEffectInstanceSetPan(live, value),
                operation: "cna_sound_effect_instance_set_pan")
            storedPan = value
        }

        /// `set_IsLooped`.
        open func SetIsLooped(_ value: Bool) throws {
            // The loop flag is fixed when playback starts; XNA says so and CNA
            // would accept a change nobody would hear.
            guard !hasPlayed else {
                throw CNAInvalidOperationException(
                    message: SoundEffectInstance.invalidIsLoopedCallMessage)
            }
            let live = try validatedHandle("SoundEffectInstance.SetIsLooped")
            try runtime.functions.check(
                runtime.functions.soundEffectInstanceSetIsLooped(live, value ? 1 : 0),
                operation: "cna_sound_effect_instance_set_is_looped")
            storedIsLooped = value
        }

        /// `SoundEffectInstance.Play()`.
        open func Play() throws {
            try call("Play", runtime.functions.soundEffectInstancePlay,
                     "cna_sound_effect_instance_play")
            hasPlayed = true
        }

        /// The two flags XNA's rules are written against: whether playback has
        /// ever started, and whether `Apply3D` made this a 3D sound.
        private var hasPlayed = false
        private var is3D = false

        /// `SoundEffectInstance.Pause()`.
        public func Pause() throws {
            try call("Pause", runtime.functions.soundEffectInstancePause,
                     "cna_sound_effect_instance_pause")
        }

        /// `SoundEffectInstance.Resume()`.
        public func Resume() throws {
            try call("Resume", runtime.functions.soundEffectInstanceResume,
                     "cna_sound_effect_instance_resume")
        }

        /// `SoundEffectInstance.Stop()`, which forwards `immediate: true`.
        public func Stop() throws {
            try Stop(true)
        }

        /// `SoundEffectInstance.Stop(Boolean immediate)`.
        public func Stop(_ immediate: Bool) throws {
            let live = try validatedHandle("SoundEffectInstance.Stop")
            try runtime.functions.check(
                runtime.functions.soundEffectInstanceStop(live, immediate ? 1 : 0),
                operation: "cna_sound_effect_instance_stop")
        }

        /// `SoundEffectInstance.Apply3D(AudioListener, AudioEmitter)`.
        ///
        /// The listener and emitter cross as **mirrored structures**, not
        /// handles: CNA takes them by value, which is why they are the only
        /// two managed audio types that needed a shim mirror.
        public func Apply3D(
            _ listener: Microsoft.Xna.Framework.Audio.AudioListener,
            emitter: Microsoft.Xna.Framework.Audio.AudioEmitter
        ) throws {
            let live = try validatedHandle("SoundEffectInstance.Apply3D")
            try requireNotYetPlayed()
            var nativeListener = listener.nativeDescriptor()
            var nativeEmitter = emitter.nativeDescriptor()
            try runtime.functions.check(
                withUnsafePointer(to: &nativeListener) { l in
                    withUnsafePointer(to: &nativeEmitter) { e in
                        runtime.functions.soundEffectInstanceApply3D(live, l, e)
                    }
                },
                operation: "cna_sound_effect_instance_apply_3d")
            is3D = true
        }

        /// `SoundEffectInstance.Apply3D(AudioListener[], AudioEmitter)`.
        ///
        /// **The multi-listener route is an `_ext`**, which is worth saying
        /// plainly: CNA publishes the array shape separately rather than as the
        /// canonical one, so the two XNA overloads are two different calls.
        ///
        /// XNA refuses a null array; an empty one is not the same thing and is
        /// passed through, because CNA is the authority on what zero listeners
        /// means.
        public func Apply3D(
            _ listeners: [Microsoft.Xna.Framework.Audio.AudioListener]?,
            emitter: Microsoft.Xna.Framework.Audio.AudioEmitter
        ) throws {
            guard let listeners else {
                throw CNAArgumentNullException(paramName: "listeners")
            }
            let live = try validatedHandle("SoundEffectInstance.Apply3D")
            try requireNotYetPlayed()
            let natives = listeners.map { $0.nativeDescriptor() }
            var nativeEmitter = emitter.nativeDescriptor()
            try runtime.functions.check(
                natives.withUnsafeBufferPointer { buffer in
                    withUnsafePointer(to: &nativeEmitter) { e in
                        runtime.functions.soundEffectInstanceApply3DMulti(
                            live, buffer.baseAddress, UInt64(buffer.count), e)
                    }
                },
                operation: "cna_sound_effect_instance_apply_3d_multi_ext")
            is3D = true
        }

        /// A sound becomes 3D by being told so **before** it first plays;
        /// afterwards it is too late, and XNA's message says exactly that.
        private func requireNotYetPlayed() throws {
            guard !hasPlayed else {
                throw CNAInvalidOperationException(
                    message: SoundEffectInstance.invalidApply3DCallMessage)
            }
        }

        /// `FrameworkResources.InvalidApply3DCall`.
        internal static let invalidApply3DCallMessage =
            "The sound is not a 3D sound. Call Apply3D before the first Play "
            + "call to configure it to be a 3D sound."

        /// `FrameworkResources.InvalidPanCall`.
        internal static let invalidPanCallMessage =
            "Pan cannot be set on a 3D sound. To ensure a 2D sound avoid "
            + "calling Apply3D and ensure Pan is set before the first Play call."

        /// `FrameworkResources.InvalidIsLoopedCall`.
        internal static let invalidIsLoopedCallMessage =
            "Loop must be set before the first Play call."

        /// `SoundEffectInstance.Dispose()`.
        public func Dispose() throws {
            try Dispose(true)
        }

        /// `protected virtual void Dispose(Boolean disposing)` -- the override
        /// point. Swift has no `protected`, so this is public.
        open func Dispose(_ disposing: Bool) throws {
            guard !disposed else { return }
            disposed = true
            let live = handle
            handle = 0
            guard disposing else { return }
            try runtime.functions.check(
                runtime.functions.soundEffectInstanceDestroy(live),
                operation: "cna_sound_effect_instance_destroy")
        }

        private func call(
            _ name: String, _ route: (UInt64) -> UInt32, _ operation: String
        ) throws {
            let live = try validatedHandle("SoundEffectInstance.\(name)")
            try runtime.functions.check(route(live), operation: operation)
        }

        private func validatedHandle(_ operation: String) throws -> UInt64 {
            guard !disposed, handle != 0 else {
                throw CNAObjectDisposedException(
                    objectName: "SoundEffectInstance",
                    message: Microsoft.Xna.Framework.Audio.objectDisposedMessage)
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { disposed }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
