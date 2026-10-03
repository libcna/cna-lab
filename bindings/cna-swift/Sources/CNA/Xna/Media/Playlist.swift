// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.PlaylistCollection`.
    ///
    /// The seventh Media collection, and the last: the behaviour is in
    /// `MediaCollectionStorage`, which now serves every one of them.
    public final class PlaylistCollection: RuntimeOwnedChild {

        private let storage: MediaCollectionStorage
        private let runtime: RuntimeState

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = MediaCollectionStorage(
                handle: handle, runtime: runtime, family: .playlist)
            runtime.register(self)
        }

        /// `PlaylistCollection.IsDisposed`.
        public var IsDisposed: Bool { storage.isDisposed }

        /// `PlaylistCollection.Count`.
        public var Count: Int32 { get throws { try storage.count() } }

        /// `PlaylistCollection.Item[Int32 index]`.
        public subscript(index: Int32) -> Playlist {
            get throws {
                Playlist(handle: try storage.handle(at: index), runtime: runtime)
            }
        }

        /// `PlaylistCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<Playlist> {
            CNAEnumerator(expectedVersion: 0) { [self] index, _ in
                let total = try storage.count()
                guard index < Int(total) else { return nil }
                return try self[Int32(index)]
            }
        }

        /// `PlaylistCollection.Dispose()`.
        public func Dispose() throws { try storage.dispose() }

        internal var runtimeObjectIsDisposed: Bool { storage.isDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// `Microsoft.Xna.Framework.Media.Playlist`.
    ///
    /// A named list of songs with a total duration -- the same shape `Artist`
    /// and `Genre` have, minus the albums.
    public final class Playlist: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var released = false

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.handle = handle
            runtime.register(self)
        }

        /// `Playlist.IsDisposed`.
        public var IsDisposed: Bool {
            guard !released, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.playlistGetIsDisposed(handle, &value) == 0 else {
                return released
            }
            return value != 0
        }

        /// `Playlist.Name`.
        public var Name: String? {
            get throws {
                let live = try validated()
                var size: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.playlistGetNameSize(live, &size),
                    operation: "cna_playlist_get_name_size")
                if size == 0 { return "" }
                var bytes = [CChar](repeating: 0, count: Int(size))
                var written: UInt64 = 0
                try runtime.functions.check(
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.playlistCopyName(
                            live, buffer.baseAddress, size, &written)
                    },
                    operation: "cna_playlist_copy_name")
                return String(
                    decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                    as: UTF8.self)
            }
        }

        /// `Playlist.Songs`.
        public var Songs: SongCollection {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.playlistGetSongs(live, &produced),
                    operation: "cna_playlist_get_songs")
                return SongCollection(handle: produced, runtime: runtime)
            }
        }

        /// `Playlist.Duration`.
        public var Duration: Swift.Duration {
            get throws {
                let live = try validated()
                var ticks: Int64 = 0
                try runtime.functions.check(
                    runtime.functions.playlistGetDuration(live, &ticks),
                    operation: "cna_playlist_get_duration")
                return Microsoft.Xna.Framework.Audio.SoundEffect
                    .duration(fromTicks: ticks)
            }
        }

        /// `Playlist.Dispose()`.
        public func Dispose() throws {
            guard !released else { return }
            released = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.playlistDispose(live),
                operation: "cna_playlist_dispose")
            try runtime.functions.check(
                runtime.functions.playlistDestroy(live),
                operation: "cna_playlist_destroy")
        }

        /// `Playlist.Equals(Playlist other)`, answered by CNA and defined for
        /// null.
        public func Equals(_ other: Playlist?) -> Bool {
            guard let other else { return false }
            guard !released, !other.released else { return self === other }
            var equal: UInt8 = 0
            guard runtime.functions.playlistEquals(
                handle, other.handle, &equal) == 0 else { return self === other }
            return equal != 0
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Playlist else { return false }
            return Equals(other)
        }

        /// `Playlist.ToString()`, which XNA answers with the name.
        public func ToString() -> String? { (try? Name) ?? "" }

        /// `Playlist.GetHashCode()`, taken from CNA so equal values hash alike.
        public func GetHashCode() -> Int32 {
            guard !released, handle != 0 else { return 0 }
            var value: Int32 = 0
            guard runtime.functions.playlistGetHashCode(handle, &value) == 0 else {
                return 0
            }
            return value
        }

        public static func == (lhs: Playlist, rhs: Playlist) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Playlist, rhs: Playlist) -> Bool { !lhs.Equals(rhs) }

        private func validated() throws -> UInt64 {
            guard !released, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "Playlist")
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { released }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
