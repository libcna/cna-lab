// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.MediaQueue`.
    ///
    /// What `MediaPlayer.Queue` answers: the songs queued for playback and
    /// which of them is active. Four members, all fallible getters, and the
    /// active index is the one a caller can move.
    ///
    /// **The handle is a borrowed view of a process-wide queue**, not an object
    /// this facade owns -- CNA says so plainly -- so it is not destroyed here.
    public final class MediaQueue: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private let handle: UInt64

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.handle = handle
            runtime.register(self)
        }

        /// `MediaQueue.Count`.
        public var Count: Int32 {
            get throws {
                var value: Int32 = 0
                try runtime.functions.check(
                    runtime.functions.mediaQueueGetCount(handle, &value),
                    operation: "cna_media_queue_get_count")
                return value
            }
        }

        /// `MediaQueue.ActiveSongIndex`.
        public var ActiveSongIndex: Int32 {
            get throws {
                var value: Int32 = 0
                try runtime.functions.check(
                    runtime.functions.mediaQueueGetActiveSongIndex(handle, &value),
                    operation: "cna_media_queue_get_active_song_index")
                return value
            }
        }

        /// `set_ActiveSongIndex`, which is `IL_REACHABLE_THROW` and therefore a
        /// writer method.
        public func SetActiveSongIndex(_ value: Int32) throws {
            try runtime.functions.check(
                runtime.functions.mediaQueueSetActiveSongIndex(handle, value),
                operation: "cna_media_queue_set_active_song_index")
        }

        /// `MediaQueue.ActiveSong`.
        ///
        /// An empty queue has none, and CNA reports that with an availability
        /// flag rather than a failure -- so this answers nil, because the
        /// return is proven nullable.
        public var ActiveSong: Song? {
            get throws {
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.mediaQueueGetActiveSong(
                        handle, &produced, &available),
                    operation: "cna_media_queue_get_active_song")
                guard available != 0 else { return nil }
                return Song(handle: produced, runtime: runtime, borrowed: true)
            }
        }

        /// `MediaQueue.Item[Int32 index]`.
        ///
        /// `IL_DIRECT_THROW`, and the bounds test is the binding's: CNA answers
        /// its own failure for an index it does not have, which is not the
        /// `ArgumentOutOfRangeException` XNA's indexer declares.
        public subscript(index: Int32) -> Song {
            get throws {
                let total = try Count
                guard index >= 0, index < total else {
                    throw CNAArgumentOutOfRangeException(paramName: "index")
                }
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.mediaQueueGetAt(handle, index, &produced),
                    operation: "cna_media_queue_get_at")
                return Song(handle: produced, runtime: runtime, borrowed: true)
            }
        }

        /// The queue is a borrowed view, so there is nothing of its own to
        /// release -- but the runtime registry expects every child to answer.
        internal var runtimeObjectIsDisposed: Bool { false }
        internal func disposeFromParent() throws {}
    }
}
