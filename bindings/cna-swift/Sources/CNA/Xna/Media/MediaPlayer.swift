// SPDX-License-Identifier: MIT

import CNAShim

/// The rooted context a media-player subscription carries.
internal final class MediaPlayerEventBox {
    let raise: () -> Void
    init(_ raise: @escaping () -> Void) { self.raise = raise }
}

internal let mediaPlayerEventCallback: CNASwift_MediaPlayerEventCallback = { context in
    guard let context else { return }
    Unmanaged<MediaPlayerEventBox>.fromOpaque(context).takeUnretainedValue().raise()
}

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.MediaPlayer`.
    ///
    /// Entirely static, as XNA's is, and the last type the music half of this
    /// namespace was waiting on.
    ///
    /// **A `final class`, not an enum**, even though every member is static
    /// and nothing is ever instantiated: the CLR seals the type and declares it
    /// a class, and the strict comparison says so by name. The constructor
    /// stays internal, because XNA declares none.
    ///
    /// **Three getters cannot fail and the rest can**, which is not a house
    /// style but the table: `IsShuffled`, `IsRepeating`, `Queue` and
    /// `GameHasControl` read fields XNA has already loaded, while `State`,
    /// `PlayPosition`, `Volume`, `IsMuted` and `IsVisualizationEnabled` reach
    /// the platform and are `IL_REACHABLE_THROW`. The infallible ones answer a
    /// default when the route fails, because there is nothing else they may do.
    public final class MediaPlayer {

        internal init() {}

        // MARK: - Playback

        /// `MediaPlayer.Play(Song song)`.
        public static func Play(_ song: Song?) throws {
            guard let song else {
                throw CNAArgumentNullException(paramName: "song")
            }
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerPlaySong(rt.gameHandle, song.nativeHandle),
                operation: "cna_media_player_play_song")
        }

        /// `MediaPlayer.Play(SongCollection songs)`.
        public static func Play(_ songs: SongCollection?) throws {
            guard let songs else {
                throw CNAArgumentNullException(paramName: "songs")
            }
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerPlaySongs(rt.gameHandle, songs.nativeHandle),
                operation: "cna_media_player_play_songs")
        }

        /// `MediaPlayer.Play(SongCollection songs, Int32 index)`.
        public static func Play(_ songs: SongCollection?, index: Int32) throws {
            guard let songs else {
                throw CNAArgumentNullException(paramName: "songs")
            }
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerPlaySongsFrom(
                    rt.gameHandle, songs.nativeHandle, index),
                operation: "cna_media_player_play_songs_from")
        }

        /// `MediaPlayer.Pause()`.
        public static func Pause() throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerPause(rt.gameHandle),
                operation: "cna_media_player_pause")
        }

        /// `MediaPlayer.Resume()`.
        public static func Resume() throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerResume(rt.gameHandle),
                operation: "cna_media_player_resume")
        }

        /// `MediaPlayer.Stop()`.
        public static func Stop() throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerStop(rt.gameHandle),
                operation: "cna_media_player_stop")
        }

        /// `MediaPlayer.MoveNext()`.
        public static func MoveNext() throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerMoveNext(rt.gameHandle),
                operation: "cna_media_player_move_next")
        }

        /// `MediaPlayer.MovePrevious()`.
        public static func MovePrevious() throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerMovePrevious(rt.gameHandle),
                operation: "cna_media_player_move_previous")
        }

        /// `MediaPlayer.GetVisualizationData(VisualizationData visualizationData)`.
        ///
        /// XNA fills the caller's object rather than answering a new one, and
        /// so does this -- which is what `VisualizationData`'s internal write
        /// path was built for. Both buffers are fixed at 256 values on either
        /// side, so the copy is a plain one.
        public static func GetVisualizationData(
            _ visualizationData: VisualizationData?
        ) throws {
            guard let visualizationData else {
                throw CNAArgumentNullException(paramName: "visualizationData")
            }
            let rt = try RuntimeRegistry.current()
            var native = CNASwift_VisualizationData()
            native.struct_size = UInt32(MemoryLayout<CNASwift_VisualizationData>.size)
            native.struct_version = 1
            try rt.functions.check(
                withUnsafeMutablePointer(to: &native) {
                    rt.functions.mediaPlayerGetVisualizationData(rt.gameHandle, $0)
                },
                operation: "cna_media_player_get_visualization_data")
            let frequencies = withUnsafeBytes(of: native.frequencies) {
                Array($0.bindMemory(to: Float.self))
            }
            let samples = withUnsafeBytes(of: native.samples) {
                Array($0.bindMemory(to: Float.self))
            }
            try visualizationData.store(frequencies: frequencies, samples: samples)
        }

        // MARK: - The four infallible reads

        /// `MediaPlayer.IsShuffled`. `IL_NO_FAILURE_PATH`, so a failed route
        /// answers false rather than refusing.
        ///
        /// **The three flag reads are spelled out, and the third time is the
        /// charm.** A helper taking `{ $0.someRoute }` crashes SILGen in this
        /// toolchain -- a `@convention(c)` pointer behind a Swift closure
        /// parameter produces a broken re-abstraction thunk. It crashed
        /// `SoundEffect`, then segfaulted the media collections at run time,
        /// and then crashed here. Write the call out.
        public static var IsShuffled: Bool {
            guard let rt = try? RuntimeRegistry.current() else { return false }
            var value: UInt8 = 0
            guard rt.functions.mediaPlayerGetIsShuffled(
                rt.gameHandle, &value) == 0 else { return false }
            return value != 0
        }

        /// `MediaPlayer.IsRepeating`.
        public static var IsRepeating: Bool {
            guard let rt = try? RuntimeRegistry.current() else { return false }
            var value: UInt8 = 0
            guard rt.functions.mediaPlayerGetIsRepeating(
                rt.gameHandle, &value) == 0 else { return false }
            return value != 0
        }

        /// `MediaPlayer.GameHasControl`.
        ///
        /// On a host where nothing else is playing this is true, which is what
        /// the qualified renderer reports.
        public static var GameHasControl: Bool {
            guard let rt = try? RuntimeRegistry.current() else { return false }
            var value: UInt8 = 0
            guard rt.functions.mediaPlayerGetGameHasControl(
                rt.gameHandle, &value) == 0 else { return false }
            return value != 0
        }

        /// `MediaPlayer.Queue`, also `IL_NO_FAILURE_PATH`: a borrowed view of
        /// the process-wide queue.
        /// **Non-Optional, and it traps when there is no runtime.** The
        /// return is proven non-null and the getter cannot refuse, which is the
        /// same pair of facts `GraphicsAdapter.DefaultAdapter` and
        /// `Game.Content` were decided by. XNA's queue exists from the moment
        /// the player does; here it needs a game.
        public static var Queue: MediaQueue {
            guard let rt = try? RuntimeRegistry.current() else {
                preconditionFailure(
                    "MediaPlayer.Queue was read before the runtime existed. "
                    + "XNA's media queue exists as soon as the player does; "
                    + "this one needs a game, because cna_media_player_get_queue "
                    + "takes one. Read it from inside a Game callback.")
            }
            var produced: UInt64 = 0
            guard rt.functions.mediaPlayerGetQueue(rt.gameHandle, &produced) == 0
            else {
                preconditionFailure(
                    "cna_media_player_get_queue refused, and an infallible "
                    + "getter with a proven non-null return has nothing else "
                    + "to say.")
            }
            return MediaQueue(handle: produced, runtime: rt)
        }

        // MARK: - The fallible reads and their writers

        /// `MediaPlayer.State`.
        public static var State: MediaState {
            get throws {
                let rt = try RuntimeRegistry.current()
                var raw: UInt32 = 0
                try rt.functions.check(
                    rt.functions.mediaPlayerGetState(rt.gameHandle, &raw),
                    operation: "cna_media_player_get_state")
                guard let state = MediaState(rawValue: Int32(bitPattern: raw)) else {
                    throw CNAError.nativeFailure(
                        operation: "MediaPlayer.State", result: 1,
                        message: "native media state \(raw) is not an XNA MediaState")
                }
                return state
            }
        }

        /// `MediaPlayer.PlayPosition`.
        public static var PlayPosition: Swift.Duration {
            get throws {
                let rt = try RuntimeRegistry.current()
                var ticks: Int64 = 0
                try rt.functions.check(
                    rt.functions.mediaPlayerGetPlayPositionTicks(rt.gameHandle, &ticks),
                    operation: "cna_media_player_get_play_position_ticks")
                return Microsoft.Xna.Framework.Audio.SoundEffect
                    .duration(fromTicks: ticks)
            }
        }

        /// `MediaPlayer.Volume`.
        public static var Volume: Float {
            get throws {
                let rt = try RuntimeRegistry.current()
                var value: Float = 0
                try rt.functions.check(
                    rt.functions.mediaPlayerGetVolume(rt.gameHandle, &value),
                    operation: "cna_media_player_get_volume")
                return value
            }
        }

        public static func SetVolume(_ value: Float) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerSetVolume(rt.gameHandle, value),
                operation: "cna_media_player_set_volume")
        }

        /// `MediaPlayer.IsMuted`.
        public static var IsMuted: Bool {
            get throws {
                let rt = try RuntimeRegistry.current()
                var value: UInt8 = 0
                try rt.functions.check(
                    rt.functions.mediaPlayerGetIsMuted(rt.gameHandle, &value),
                    operation: "cna_media_player_get_is_muted")
                return value != 0
            }
        }

        public static func SetIsMuted(_ value: Bool) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerSetIsMuted(rt.gameHandle, value ? 1 : 0),
                operation: "cna_media_player_set_is_muted")
        }

        /// `MediaPlayer.IsVisualizationEnabled`.
        public static var IsVisualizationEnabled: Bool {
            get throws {
                let rt = try RuntimeRegistry.current()
                var value: UInt8 = 0
                try rt.functions.check(
                    rt.functions.mediaPlayerGetIsVisualizationEnabled(
                        rt.gameHandle, &value),
                    operation: "cna_media_player_get_is_visualization_enabled")
                return value != 0
            }
        }

        public static func SetIsVisualizationEnabled(_ value: Bool) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerSetIsVisualizationEnabled(
                    rt.gameHandle, value ? 1 : 0),
                operation: "cna_media_player_set_is_visualization_enabled")
        }

        /// `set_IsShuffled` and `set_IsRepeating`, both `IL_REACHABLE_THROW`
        /// even though their getters cannot fail.
        public static func SetIsShuffled(_ value: Bool) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerSetIsShuffled(rt.gameHandle, value ? 1 : 0),
                operation: "cna_media_player_set_is_shuffled")
        }

        public static func SetIsRepeating(_ value: Bool) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.mediaPlayerSetIsRepeating(rt.gameHandle, value ? 1 : 0),
                operation: "cna_media_player_set_is_repeating")
        }

        // MARK: - The two events

        /// `MediaPlayer.ActiveSongChanged`.
        public static var ActiveSongChanged: CNAEvent<CNAEventArgs> {
            activeSongChangedSource.Event
        }

        /// `MediaPlayer.MediaStateChanged`.
        public static var MediaStateChanged: CNAEvent<CNAEventArgs> {
            mediaStateChangedSource.Event
        }

        private static let activeSongChangedSource = CNAEventSource<CNAEventArgs>()
        private static let mediaStateChangedSource = CNAEventSource<CNAEventArgs>()
        private static var subscriptions: [UInt64] = []
        private static var boxes: [Unmanaged<MediaPlayerEventBox>] = []

        /// Both native subscriptions, armed once per runtime.
        ///
        /// XNA's events are static and live as long as the process; CNA's
        /// registrations belong to the runtime, so they are armed when a
        /// handler is first attached and released when the runtime goes.
        internal static func armSubscriptions(_ rt: RuntimeState) throws {
            guard subscriptions.isEmpty else { return }
            let active = Unmanaged.passRetained(MediaPlayerEventBox {
                try? activeSongChangedSource.Raise(
                    MediaPlayer.self, args: CNAEventArgs.Empty)
            })
            var registration: UInt64 = 0
            var result = rt.functions.mediaPlayerSubscribeActiveSongChanged(
                mediaPlayerEventCallback, active.toOpaque(), &registration)
            guard result == 0 else {
                active.release()
                try rt.functions.check(
                    result,
                    operation: "cna_media_player_subscribe_active_song_changed_ext")
                return
            }
            subscriptions.append(registration)
            boxes.append(active)

            let state = Unmanaged.passRetained(MediaPlayerEventBox {
                try? mediaStateChangedSource.Raise(
                    MediaPlayer.self, args: CNAEventArgs.Empty)
            })
            registration = 0
            result = rt.functions.mediaPlayerSubscribeMediaStateChanged(
                mediaPlayerEventCallback, state.toOpaque(), &registration)
            guard result == 0 else {
                state.release()
                try rt.functions.check(
                    result,
                    operation: "cna_media_player_subscribe_media_state_changed_ext")
                return
            }
            subscriptions.append(registration)
            boxes.append(state)
        }

        /// Released when the runtime that armed them goes away.
        internal static func releaseSubscriptions(_ rt: RuntimeState) {
            for registration in subscriptions {
                _ = rt.functions.mediaPlayerUnsubscribe(registration)
            }
            subscriptions.removeAll()
            for box in boxes { box.release() }
            boxes.removeAll()
        }

        /// Internal, so a test can assert the registrations exist and go.
        internal static var subscriptionCount: Int { subscriptions.count }

    }
}
