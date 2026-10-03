// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.VideoPlayer`.
    ///
    /// The last Media type, and the only one that reaches back into Graphics:
    /// `GetTexture` answers the frame currently on screen as a `Texture2D`.
    ///
    /// **Four getters cannot fail and two can**, which is the table's shape and
    /// worth noticing because it is the opposite of `MediaPlayer`'s: here
    /// `IsLooped`, `IsMuted` and `Volume` read fields XNA has already stored,
    /// while `State` and `PlayPosition` reach the platform.
    public final class VideoPlayer: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var released = false

        /// `VideoPlayer()`.
        public init() throws {
            let rt = try RuntimeRegistry.current()
            var created: UInt64 = 0
            try rt.functions.check(
                rt.functions.videoPlayerCreate(rt.gameHandle, &created),
                operation: "cna_video_player_create")
            runtime = rt
            handle = created
            rt.register(self)
        }

        /// `VideoPlayer.IsDisposed`.
        public var IsDisposed: Bool {
            guard !released, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.videoPlayerGetIsDisposed(handle, &value) == 0
            else { return released }
            return value != 0
        }

        /// `VideoPlayer.Play(Video video)`.
        ///
        /// **A video built from values cannot be played, and that is said
        /// rather than crashed on.** CNA takes a video *handle*, and the only
        /// videos that carry one are those a player answered. XNA's come from
        /// the content pipeline, which this binding cannot load -- so the
        /// refusal names the reason instead of passing a zero handle to the
        /// ABI.
        public func Play(_ video: Video?) throws {
            guard let video else {
                throw CNAArgumentNullException(paramName: "video")
            }
            let live = try validated()
            guard video.handle != 0 else {
                throw CNAError.nativeFailure(
                    operation: "VideoPlayer.Play", result: 1,
                    message: "this Video was built from values and carries no "
                        + "native handle; only a video a player answered can be "
                        + "played back")
            }
            try runtime.functions.check(
                runtime.functions.videoPlayerPlay(live, video.handle),
                operation: "cna_video_player_play")
        }

        /// `VideoPlayer.Pause()`.
        public func Pause() throws {
            let live = try validated()
            try runtime.functions.check(
                runtime.functions.videoPlayerPause(live),
                operation: "cna_video_player_pause")
        }

        /// `VideoPlayer.Resume()`.
        public func Resume() throws {
            let live = try validated()
            try runtime.functions.check(
                runtime.functions.videoPlayerResume(live),
                operation: "cna_video_player_resume")
        }

        /// `VideoPlayer.Stop()`.
        public func Stop() throws {
            let live = try validated()
            try runtime.functions.check(
                runtime.functions.videoPlayerStop(live),
                operation: "cna_video_player_stop")
        }

        /// `VideoPlayer.GetTexture()`.
        ///
        /// The frame on screen. A player with nothing playing has none, and
        /// CNA reports that with an availability flag rather than a failure --
        /// so the absence is reported here, because the return is proven
        /// non-null and there is nothing to hand back.
        public func GetTexture() throws -> Microsoft.Xna.Framework.Graphics.Texture2D {
            let live = try validated()
            var produced: UInt64 = 0
            var available: UInt8 = 0
            try runtime.functions.check(
                runtime.functions.videoPlayerGetTexture(live, &produced, &available),
                operation: "cna_video_player_get_texture")
            guard available != 0 else {
                throw CNAError.nativeFailure(
                    operation: "VideoPlayer.GetTexture", result: 1,
                    message: "nothing is playing, so there is no frame to hand back")
            }
            return try Microsoft.Xna.Framework.Graphics.Texture2D
                .adoptLoaded(handle: produced, runtime: runtime)
        }

        /// `VideoPlayer.Video`.
        ///
        /// `IL_NO_FAILURE_PATH`, so it cannot refuse -- and a player with
        /// nothing loaded answers nil, which is what an infallible getter over
        /// an availability flag has left to say.
        public var Video: Microsoft.Xna.Framework.Media.Video? {
            guard !released, handle != 0 else { return nil }
            var produced: UInt64 = 0
            var available: UInt8 = 0
            guard runtime.functions.videoPlayerGetVideo(
                handle, &produced, &available) == 0, available != 0 else {
                return nil
            }
            return try? Microsoft.Xna.Framework.Media.Video(
                adopting: produced, runtime: runtime)
        }

        /// `VideoPlayer.State`.
        public var State: MediaState {
            get throws {
                let live = try validated()
                var raw: UInt32 = 0
                try runtime.functions.check(
                    runtime.functions.videoPlayerGetState(live, &raw),
                    operation: "cna_video_player_get_state")
                guard let state = MediaState(rawValue: Int32(bitPattern: raw)) else {
                    throw CNAError.nativeFailure(
                        operation: "VideoPlayer.State", result: 1,
                        message: "native media state \(raw) is not an XNA MediaState")
                }
                return state
            }
        }

        /// `VideoPlayer.PlayPosition`.
        public var PlayPosition: Swift.Duration {
            get throws {
                let live = try validated()
                var ticks: Int64 = 0
                try runtime.functions.check(
                    runtime.functions.videoPlayerGetPlayPositionTicks(live, &ticks),
                    operation: "cna_video_player_get_play_position_ticks")
                return Microsoft.Xna.Framework.Audio.SoundEffect
                    .duration(fromTicks: ticks)
            }
        }

        /// `VideoPlayer.IsLooped`, infallible to read and fallible to write.
        public var IsLooped: Bool {
            guard !released, handle != 0 else { return false }
            var value: UInt8 = 0
            guard runtime.functions.videoPlayerGetIsLooped(handle, &value) == 0
            else { return false }
            return value != 0
        }

        public func SetIsLooped(_ value: Bool) throws {
            let live = try validated()
            try runtime.functions.check(
                runtime.functions.videoPlayerSetIsLooped(live, value ? 1 : 0),
                operation: "cna_video_player_set_is_looped")
        }

        /// `VideoPlayer.IsMuted`.
        public var IsMuted: Bool {
            guard !released, handle != 0 else { return false }
            var value: UInt8 = 0
            guard runtime.functions.videoPlayerGetIsMuted(handle, &value) == 0
            else { return false }
            return value != 0
        }

        public func SetIsMuted(_ value: Bool) throws {
            let live = try validated()
            try runtime.functions.check(
                runtime.functions.videoPlayerSetIsMuted(live, value ? 1 : 0),
                operation: "cna_video_player_set_is_muted")
        }

        /// `VideoPlayer.Volume`.
        ///
        /// The getter cannot fail and the setter is `IL_DIRECT_THROW` -- the
        /// only accessor on this type whose refusal XNA writes itself rather
        /// than inheriting from a platform call.
        public var Volume: Float {
            guard !released, handle != 0 else { return 0 }
            var value: Float = 0
            guard runtime.functions.videoPlayerGetVolume(handle, &value) == 0
            else { return 0 }
            return value
        }

        public func SetVolume(_ value: Float) throws {
            let live = try validated()
            try runtime.functions.check(
                runtime.functions.videoPlayerSetVolume(live, value),
                operation: "cna_video_player_set_volume")
        }

        /// `VideoPlayer.Dispose()`.
        public func Dispose() throws {
            guard !released else { return }
            released = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.videoPlayerDispose(live),
                operation: "cna_video_player_dispose")
            try runtime.functions.check(
                runtime.functions.videoPlayerDestroy(live),
                operation: "cna_video_player_destroy")
        }

        private func validated() throws -> UInt64 {
            guard !released, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "VideoPlayer")
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { released }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
