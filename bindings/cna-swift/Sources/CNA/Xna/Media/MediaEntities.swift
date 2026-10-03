// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.Artist`.
    ///
    /// One performer in the media library: a name, the songs they made and the albums those songs are on.
    ///
    /// Its identity is CNA's: `Equals` and `GetHashCode` both ask the runtime,
    /// so two facades naming one artist agree -- the same shape `Song` uses, and
    /// for the same reason.
    public final class Artist: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var released = false

        /// **Borrowed handles are not destroyed here.** CNA hands out a
        /// borrowed entity when a song names one, and the library owns it; a
        /// facade that called destroy on such a handle would release something
        /// it never took.
        private let borrowed: Bool

        internal init(handle: UInt64, runtime: RuntimeState, borrowed: Bool = false) {
            self.runtime = runtime
            self.handle = handle
            self.borrowed = borrowed
            runtime.register(self)
        }

        /// `Artist.IsDisposed`, the one accessor with no failure path.
        public var IsDisposed: Bool {
            guard !released, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.artistGetIsDisposed(handle, &value) == 0 else {
                return released
            }
            return value != 0
        }

        /// `Artist.Name`.
        public var Name: String? {
            get throws {
                let live = try validated()
                var size: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.artistGetNameSize(live, &size),
                    operation: "cna_artist_get_name_size")
                if size == 0 { return "" }
                var bytes = [CChar](repeating: 0, count: Int(size))
                var written: UInt64 = 0
                try runtime.functions.check(
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.artistCopyName(
                            live, buffer.baseAddress, size, &written)
                    },
                    operation: "cna_artist_copy_name")
                return String(
                    decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                    as: UTF8.self)
            }
        }

        /// `Artist.Songs`.
        public var Songs: SongCollection {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.artistGetSongs(live, &produced),
                    operation: "cna_artist_get_songs")
                return SongCollection(handle: produced, runtime: runtime)
            }
        }

        /// `Artist.Albums`.
        public var Albums: AlbumCollection {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.artistGetAlbums(live, &produced),
                    operation: "cna_artist_get_albums")
                return AlbumCollection(handle: produced, runtime: runtime)
            }
        }

        /// `Artist.Dispose()`. Both native halves, in CNA's order: the artist is
        /// marked disposed and then the handle is released.
        public func Dispose() throws {
            guard !released else { return }
            released = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.artistDispose(live), operation: "cna_artist_dispose")
            guard !borrowed else { return }
            try runtime.functions.check(
                runtime.functions.artistDestroy(live), operation: "cna_artist_destroy")
        }

        /// `Artist.Equals(Artist other)`, answered by CNA rather than by a handle
        /// test -- and defined for null, which XNA answers false for.
        public func Equals(_ other: Artist?) -> Bool {
            guard let other else { return false }
            guard !released, !other.released else { return self === other }
            var equal: UInt8 = 0
            guard runtime.functions.artistEquals(
                handle, other.handle, &equal) == 0 else { return self === other }
            return equal != 0
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Artist else { return false }
            return Equals(other)
        }

        /// `Artist.ToString()`, which XNA answers with the name.
        public func ToString() -> String? { (try? Name) ?? "" }

        /// `Artist.GetHashCode()`, taken from CNA so equal values hash alike.
        public func GetHashCode() -> Int32 {
            guard !released, handle != 0 else { return 0 }
            var value: Int32 = 0
            guard runtime.functions.artistGetHashCode(handle, &value) == 0 else {
                return 0
            }
            return value
        }

        public static func == (lhs: Artist, rhs: Artist) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Artist, rhs: Artist) -> Bool { !lhs.Equals(rhs) }

        private func validated() throws -> UInt64 {
            guard !released, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "Artist")
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { released }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// `Microsoft.Xna.Framework.Media.Genre`.
    ///
    /// One genre in the media library, which reaches songs and albums the same way an artist does.
    ///
    /// Its identity is CNA's: `Equals` and `GetHashCode` both ask the runtime,
    /// so two facades naming one genre agree -- the same shape `Song` uses, and
    /// for the same reason.
    public final class Genre: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var released = false

        /// **Borrowed handles are not destroyed here.** CNA hands out a
        /// borrowed entity when a song names one, and the library owns it; a
        /// facade that called destroy on such a handle would release something
        /// it never took.
        private let borrowed: Bool

        internal init(handle: UInt64, runtime: RuntimeState, borrowed: Bool = false) {
            self.runtime = runtime
            self.handle = handle
            self.borrowed = borrowed
            runtime.register(self)
        }

        /// `Genre.IsDisposed`, the one accessor with no failure path.
        public var IsDisposed: Bool {
            guard !released, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.genreGetIsDisposed(handle, &value) == 0 else {
                return released
            }
            return value != 0
        }

        /// `Genre.Name`.
        public var Name: String? {
            get throws {
                let live = try validated()
                var size: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.genreGetNameSize(live, &size),
                    operation: "cna_genre_get_name_size")
                if size == 0 { return "" }
                var bytes = [CChar](repeating: 0, count: Int(size))
                var written: UInt64 = 0
                try runtime.functions.check(
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.genreCopyName(
                            live, buffer.baseAddress, size, &written)
                    },
                    operation: "cna_genre_copy_name")
                return String(
                    decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                    as: UTF8.self)
            }
        }

        /// `Genre.Songs`.
        public var Songs: SongCollection {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.genreGetSongs(live, &produced),
                    operation: "cna_genre_get_songs")
                return SongCollection(handle: produced, runtime: runtime)
            }
        }

        /// `Genre.Albums`.
        public var Albums: AlbumCollection {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.genreGetAlbums(live, &produced),
                    operation: "cna_genre_get_albums")
                return AlbumCollection(handle: produced, runtime: runtime)
            }
        }

        /// `Genre.Dispose()`. Both native halves, in CNA's order: the genre is
        /// marked disposed and then the handle is released.
        public func Dispose() throws {
            guard !released else { return }
            released = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.genreDispose(live), operation: "cna_genre_dispose")
            guard !borrowed else { return }
            try runtime.functions.check(
                runtime.functions.genreDestroy(live), operation: "cna_genre_destroy")
        }

        /// `Genre.Equals(Genre other)`, answered by CNA rather than by a handle
        /// test -- and defined for null, which XNA answers false for.
        public func Equals(_ other: Genre?) -> Bool {
            guard let other else { return false }
            guard !released, !other.released else { return self === other }
            var equal: UInt8 = 0
            guard runtime.functions.genreEquals(
                handle, other.handle, &equal) == 0 else { return self === other }
            return equal != 0
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Genre else { return false }
            return Equals(other)
        }

        /// `Genre.ToString()`, which XNA answers with the name.
        public func ToString() -> String? { (try? Name) ?? "" }

        /// `Genre.GetHashCode()`, taken from CNA so equal values hash alike.
        public func GetHashCode() -> Int32 {
            guard !released, handle != 0 else { return 0 }
            var value: Int32 = 0
            guard runtime.functions.genreGetHashCode(handle, &value) == 0 else {
                return 0
            }
            return value
        }

        public static func == (lhs: Genre, rhs: Genre) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Genre, rhs: Genre) -> Bool { !lhs.Equals(rhs) }

        private func validated() throws -> UInt64 {
            guard !released, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "Genre")
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { released }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// `Microsoft.Xna.Framework.Media.Album`.
    ///
    /// One album: its artist, its genre, its songs, its total duration and the cover art, which is the only place this family answers a stream.
    ///
    /// Its identity is CNA's: `Equals` and `GetHashCode` both ask the runtime,
    /// so two facades naming one album agree -- the same shape `Song` uses, and
    /// for the same reason.
    public final class Album: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var released = false

        /// **Borrowed handles are not destroyed here.** CNA hands out a
        /// borrowed entity when a song names one, and the library owns it; a
        /// facade that called destroy on such a handle would release something
        /// it never took.
        private let borrowed: Bool

        internal init(handle: UInt64, runtime: RuntimeState, borrowed: Bool = false) {
            self.runtime = runtime
            self.handle = handle
            self.borrowed = borrowed
            runtime.register(self)
        }

        /// `Album.IsDisposed`, the one accessor with no failure path.
        public var IsDisposed: Bool {
            guard !released, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.albumGetIsDisposed(handle, &value) == 0 else {
                return released
            }
            return value != 0
        }

        /// `Album.Name`.
        public var Name: String? {
            get throws {
                let live = try validated()
                var size: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.albumGetNameSize(live, &size),
                    operation: "cna_album_get_name_size")
                if size == 0 { return "" }
                var bytes = [CChar](repeating: 0, count: Int(size))
                var written: UInt64 = 0
                try runtime.functions.check(
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.albumCopyName(
                            live, buffer.baseAddress, size, &written)
                    },
                    operation: "cna_album_copy_name")
                return String(
                    decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                    as: UTF8.self)
            }
        }

        /// `Album.Songs`.
        public var Songs: SongCollection {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.albumGetSongs(live, &produced),
                    operation: "cna_album_get_songs")
                return SongCollection(handle: produced, runtime: runtime)
            }
        }

        /// `Album.Artist`.
        public var Artist: Artist {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.albumGetArtist(live, &produced, &available),
                    operation: "cna_album_get_artist")
                // The same library-context divergence Song carries: an album
                // outside a media library names no artist, and the return is
                // proven non-null, so the absence is reported rather than
                // fabricated.
                guard available != 0 else {
                    throw CNAError.nativeFailure(
                        operation: "Album.Artist", result: 1,
                        message: "this album has no media-library context, so "
                            + "it names no artist")
                }
                return Microsoft.Xna.Framework.Media.Artist(
                    handle: produced, runtime: runtime, borrowed: true)
            }
        }

        /// `Album.Genre`.
        public var Genre: Genre {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.albumGetGenre(live, &produced, &available),
                    operation: "cna_album_get_genre")
                // The same library-context divergence Song carries: an album
                // outside a media library names no genre, and the return is
                // proven non-null, so the absence is reported rather than
                // fabricated.
                guard available != 0 else {
                    throw CNAError.nativeFailure(
                        operation: "Album.Genre", result: 1,
                        message: "this album has no media-library context, so "
                            + "it names no genre")
                }
                return Microsoft.Xna.Framework.Media.Genre(
                    handle: produced, runtime: runtime, borrowed: true)
            }
        }

        /// `Album.Duration`.
        public var Duration: Swift.Duration {
            get throws {
                let live = try validated()
                var ticks: Int64 = 0
                try runtime.functions.check(
                    runtime.functions.albumGetDuration(live, &ticks),
                    operation: "cna_album_get_duration")
                return Microsoft.Xna.Framework.Audio.SoundEffect
                    .duration(fromTicks: ticks)
            }
        }

        /// `Album.HasArt`.
        public var HasArt: Bool {
            get throws {
                let live = try validated()
                var value: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.albumGetHasArt(live, &value),
                    operation: "cna_album_get_has_art")
                return value != 0
            }
        }

        /// `Album.GetAlbumArt()`.
        ///
        /// XNA answers a `Stream`, and this reads the bytes CNA holds and
        /// wraps them -- `System.IO.Stream` maps to `Foundation.InputStream`
        /// throughout this binding. An album with no art answers an **empty**
        /// stream rather than nil, because the return is not proven nullable.
        public func GetAlbumArt() throws -> Foundation.InputStream? {
            try readImage(
                size: runtime.functions.albumGetArtSize,
                copy: runtime.functions.albumCopyArt,
                sizeOperation: "cna_album_get_art_size",
                copyOperation: "cna_album_copy_art")
        }

        /// `Album.GetThumbnail()`.
        public func GetThumbnail() throws -> Foundation.InputStream? {
            try readImage(
                size: runtime.functions.albumGetThumbnailSize,
                copy: runtime.functions.albumCopyThumbnail,
                sizeOperation: "cna_album_get_thumbnail_size",
                copyOperation: "cna_album_copy_thumbnail")
        }

        private func readImage(
            size: (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32,
            copy: (UInt64, UnsafeMutablePointer<UInt8>?, UInt64,
                   UnsafeMutablePointer<UInt64>?) -> UInt32,
            sizeOperation: String, copyOperation: String
        ) throws -> Foundation.InputStream {
            let live = try validated()
            var bytes: UInt64 = 0
            try runtime.functions.check(
                size(live, &bytes), operation: sizeOperation)
            if bytes == 0 { return Foundation.InputStream(data: Data()) }
            var buffer = [UInt8](repeating: 0, count: Int(bytes))
            var written: UInt64 = 0
            try runtime.functions.check(
                buffer.withUnsafeMutableBufferPointer { raw in
                    copy(live, raw.baseAddress, bytes, &written)
                },
                operation: copyOperation)
            return Foundation.InputStream(data: Data(buffer.prefix(Int(written))))
        }

        /// `Album.Dispose()`. Both native halves, in CNA's order: the album is
        /// marked disposed and then the handle is released.
        public func Dispose() throws {
            guard !released else { return }
            released = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.albumDispose(live), operation: "cna_album_dispose")
            guard !borrowed else { return }
            try runtime.functions.check(
                runtime.functions.albumDestroy(live), operation: "cna_album_destroy")
        }

        /// `Album.Equals(Album other)`, answered by CNA rather than by a handle
        /// test -- and defined for null, which XNA answers false for.
        public func Equals(_ other: Album?) -> Bool {
            guard let other else { return false }
            guard !released, !other.released else { return self === other }
            var equal: UInt8 = 0
            guard runtime.functions.albumEquals(
                handle, other.handle, &equal) == 0 else { return self === other }
            return equal != 0
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Album else { return false }
            return Equals(other)
        }

        /// `Album.ToString()`, which XNA answers with the name.
        public func ToString() -> String? { (try? Name) ?? "" }

        /// `Album.GetHashCode()`, taken from CNA so equal values hash alike.
        public func GetHashCode() -> Int32 {
            guard !released, handle != 0 else { return 0 }
            var value: Int32 = 0
            guard runtime.functions.albumGetHashCode(handle, &value) == 0 else {
                return 0
            }
            return value
        }

        public static func == (lhs: Album, rhs: Album) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Album, rhs: Album) -> Bool { !lhs.Equals(rhs) }

        private func validated() throws -> UInt64 {
            guard !released, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "Album")
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { released }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
