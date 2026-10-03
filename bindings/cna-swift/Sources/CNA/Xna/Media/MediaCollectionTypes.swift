// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.SongCollection`.
    ///
    /// One of the four Media collections; the behaviour is in
    /// `MediaCollectionStorage`, because XNA declares four identical types and
    /// CNA publishes four identical route families for them.
    public final class SongCollection: RuntimeOwnedChild {

        private let storage: MediaCollectionStorage
        private let runtime: RuntimeState

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = MediaCollectionStorage(
                handle: handle, runtime: runtime, family: .song)
            runtime.register(self)
        }

        /// The handle `MediaPlayer.Play` needs. Internal, as the storage's is.
        internal var nativeHandle: UInt64 { storage.nativeHandle }

        /// `SongCollection.IsDisposed`.
        public var IsDisposed: Bool { storage.isDisposed }

        /// `SongCollection.Count`.
        public var Count: Int32 { get throws { try storage.count() } }

        /// `SongCollection.Item[Int32 index]`.
        ///
        /// A read-only indexed property stays a Swift **subscript**, and it
        /// gains `get throws` because the CLR getter is fallible -- the
        /// indexed half of the same accessor rule every other property here
        /// follows.
        public subscript(index: Int32) -> Song {
            get throws {
                Song(handle: try storage.handle(at: index), runtime: runtime)
            }
        }

        /// `SongCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<Song> {
            CNAEnumerator(expectedVersion: 0) { [self] index, _ in
                let total = try storage.count()
                guard index < Int(total) else { return nil }
                return try self[Int32(index)]
            }
        }

        /// `SongCollection.Dispose()`.
        public func Dispose() throws { try storage.dispose() }

        internal var runtimeObjectIsDisposed: Bool { storage.isDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// `Microsoft.Xna.Framework.Media.ArtistCollection`.
    ///
    /// One of the four Media collections; the behaviour is in
    /// `MediaCollectionStorage`, because XNA declares four identical types and
    /// CNA publishes four identical route families for them.
    public final class ArtistCollection: RuntimeOwnedChild {

        private let storage: MediaCollectionStorage
        private let runtime: RuntimeState

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = MediaCollectionStorage(
                handle: handle, runtime: runtime, family: .artist)
            runtime.register(self)
        }

        /// `ArtistCollection.IsDisposed`.
        public var IsDisposed: Bool { storage.isDisposed }

        /// `ArtistCollection.Count`.
        public var Count: Int32 { get throws { try storage.count() } }

        /// `ArtistCollection.Item[Int32 index]`.
        ///
        /// A read-only indexed property stays a Swift **subscript**, and it
        /// gains `get throws` because the CLR getter is fallible -- the
        /// indexed half of the same accessor rule every other property here
        /// follows.
        public subscript(index: Int32) -> Artist {
            get throws {
                Artist(handle: try storage.handle(at: index), runtime: runtime)
            }
        }

        /// `ArtistCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<Artist> {
            CNAEnumerator(expectedVersion: 0) { [self] index, _ in
                let total = try storage.count()
                guard index < Int(total) else { return nil }
                return try self[Int32(index)]
            }
        }

        /// `ArtistCollection.Dispose()`.
        public func Dispose() throws { try storage.dispose() }

        internal var runtimeObjectIsDisposed: Bool { storage.isDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// `Microsoft.Xna.Framework.Media.AlbumCollection`.
    ///
    /// One of the four Media collections; the behaviour is in
    /// `MediaCollectionStorage`, because XNA declares four identical types and
    /// CNA publishes four identical route families for them.
    public final class AlbumCollection: RuntimeOwnedChild {

        private let storage: MediaCollectionStorage
        private let runtime: RuntimeState

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = MediaCollectionStorage(
                handle: handle, runtime: runtime, family: .album)
            runtime.register(self)
        }

        /// `AlbumCollection.IsDisposed`.
        public var IsDisposed: Bool { storage.isDisposed }

        /// `AlbumCollection.Count`.
        public var Count: Int32 { get throws { try storage.count() } }

        /// `AlbumCollection.Item[Int32 index]`.
        ///
        /// A read-only indexed property stays a Swift **subscript**, and it
        /// gains `get throws` because the CLR getter is fallible -- the
        /// indexed half of the same accessor rule every other property here
        /// follows.
        public subscript(index: Int32) -> Album {
            get throws {
                Album(handle: try storage.handle(at: index), runtime: runtime)
            }
        }

        /// `AlbumCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<Album> {
            CNAEnumerator(expectedVersion: 0) { [self] index, _ in
                let total = try storage.count()
                guard index < Int(total) else { return nil }
                return try self[Int32(index)]
            }
        }

        /// `AlbumCollection.Dispose()`.
        public func Dispose() throws { try storage.dispose() }

        internal var runtimeObjectIsDisposed: Bool { storage.isDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// `Microsoft.Xna.Framework.Media.GenreCollection`.
    ///
    /// One of the four Media collections; the behaviour is in
    /// `MediaCollectionStorage`, because XNA declares four identical types and
    /// CNA publishes four identical route families for them.
    public final class GenreCollection: RuntimeOwnedChild {

        private let storage: MediaCollectionStorage
        private let runtime: RuntimeState

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = MediaCollectionStorage(
                handle: handle, runtime: runtime, family: .genre)
            runtime.register(self)
        }

        /// `GenreCollection.IsDisposed`.
        public var IsDisposed: Bool { storage.isDisposed }

        /// `GenreCollection.Count`.
        public var Count: Int32 { get throws { try storage.count() } }

        /// `GenreCollection.Item[Int32 index]`.
        ///
        /// A read-only indexed property stays a Swift **subscript**, and it
        /// gains `get throws` because the CLR getter is fallible -- the
        /// indexed half of the same accessor rule every other property here
        /// follows.
        public subscript(index: Int32) -> Genre {
            get throws {
                Genre(handle: try storage.handle(at: index), runtime: runtime)
            }
        }

        /// `GenreCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<Genre> {
            CNAEnumerator(expectedVersion: 0) { [self] index, _ in
                let total = try storage.count()
                guard index < Int(total) else { return nil }
                return try self[Int32(index)]
            }
        }

        /// `GenreCollection.Dispose()`.
        public func Dispose() throws { try storage.dispose() }

        internal var runtimeObjectIsDisposed: Bool { storage.isDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
