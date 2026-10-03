// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.Song`.
    ///
    /// **The leaf of the Media graph, and the only member of it with a public
    /// factory.** `Song.FromUri` is what a consumer can call without a media
    /// library, and `cna_song_create_from_uri` is the route behind it -- which
    /// is what makes this family reachable where XACT and `Model` are not.
    ///
    /// Every getter but `IsDisposed` is `IL_REACHABLE_THROW` with
    /// `ObjectDisposedException`, so every one of them is a **throwing
    /// getter**. That is unusually uniform, and it is the table's answer
    /// rather than a house style: a song is a handle onto something the media
    /// store owns, and reading any part of a released one is a mistake XNA
    /// reports rather than absorbs.
    ///
    /// `Artist`, `Album` and `Genre` are **not projected yet**. They answer
    /// three types that are themselves missing and that carry collections back
    /// to songs, so the whole cycle lands together or not at all; this type
    /// stays partial by exactly those three members until it does.
    public final class Song: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var disposed = false

        /// A song the queue or a collection lends out is **borrowed**: the
        /// owner releases it, and a facade that destroyed the handle would
        /// release something it never took.
        private let borrowed: Bool

        internal init(handle: UInt64, runtime: RuntimeState, borrowed: Bool = false) {
            self.runtime = runtime
            self.handle = handle
            self.borrowed = borrowed
            runtime.register(self)
        }

        /// `Song.FromUri(String name, Uri uri)`.
        ///
        /// **`System.Uri` maps to `Foundation.URL`**, the platform type that
        /// already stands in for a BCL one wherever `System.IO.Stream`
        /// appears. A caller holding a URL is holding what XNA's `Uri` carries,
        /// and the alternative -- admitting the whole `Uri` family for one
        /// parameter -- would earn authority nothing else consumes.
        public static func FromUri(
            _ name: String?, uri: Foundation.URL?
        ) throws -> Song {
            guard let name else {
                throw CNAArgumentNullException(paramName: "name")
            }
            guard let uri else {
                throw CNAArgumentNullException(paramName: "uri")
            }
            let rt = try RuntimeRegistry.current()
            var nameBytes = Array(name.utf8)
            var uriBytes = Array(uri.absoluteString.utf8)
            var created: UInt64 = 0
            try rt.functions.check(
                Song.withStringView(&nameBytes) { nameView in
                    Song.withStringView(&uriBytes) { uriView in
                        rt.functions.songCreateFromUri(
                            rt.gameHandle, nameView, uriView, &created)
                    }
                },
                operation: "cna_song_create_from_uri")
            return Song(handle: created, runtime: rt)
        }

        /// The handle `MediaPlayer.Play` hands to the runtime. Internal:
        /// no XNA member publishes it.
        internal var nativeHandle: UInt64 { handle }

        /// `Song.IsDisposed`, the one accessor with no failure path.
        ///
        /// **Read from CNA rather than from a local flag**, so the answer is
        /// the song's own state and not this facade's opinion of it.
        ///
        /// Measured, because the obvious guess was wrong: two `FromUri` calls
        /// on one file are **equal** -- `cna_song_equals` says so -- and yet
        /// they are *separate objects*, each with its own disposal. Disposing
        /// one leaves the other reporting alive. So "equal" here means the same
        /// track, not the same instance, which is worth knowing before writing
        /// anything that caches songs by equality.
        ///
        /// The getter cannot fail, so a failed read falls back to this
        /// facade's own state, which is the only thing left to say.
        public var IsDisposed: Bool {
            guard !disposed, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.songGetIsDisposed(handle, &value) == 0 else {
                return disposed
            }
            return value != 0
        }

        /// `Song.Name`.
        public var Name: String? {
            get throws {
                let live = try validatedHandle()
                var size: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.songGetNameSize(live, &size),
                    operation: "cna_song_get_name_size")
                if size == 0 { return "" }
                var bytes = [CChar](repeating: 0, count: Int(size))
                var written: UInt64 = 0
                try runtime.functions.check(
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.songCopyName(
                            live, buffer.baseAddress, size, &written)
                    },
                    operation: "cna_song_copy_name")
                return String(
                    decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                    as: UTF8.self)
            }
        }

        /// `Song.Duration`.
        public var Duration: Swift.Duration {
            get throws {
                var ticks: Int64 = 0
                try read("cna_song_get_duration") {
                    runtime.functions.songGetDuration($0, &ticks)
                }
                return Microsoft.Xna.Framework.Audio.SoundEffect
                    .duration(fromTicks: ticks)
            }
        }

        /// `Song.IsRated`.
        public var IsRated: Bool {
            get throws {
                var value: UInt8 = 0
                try read("cna_song_get_is_rated") {
                    runtime.functions.songGetIsRated($0, &value)
                }
                return value != 0
            }
        }

        /// `Song.Rating`.
        public var Rating: Int32 {
            get throws {
                var value: Int32 = 0
                try read("cna_song_get_rating") {
                    runtime.functions.songGetRating($0, &value)
                }
                return value
            }
        }

        /// `Song.PlayCount`.
        public var PlayCount: Int32 {
            get throws {
                var value: Int32 = 0
                try read("cna_song_get_play_count") {
                    runtime.functions.songGetPlayCount($0, &value)
                }
                return value
            }
        }

        /// `Song.TrackNumber`.
        public var TrackNumber: Int32 {
            get throws {
                var value: Int32 = 0
                try read("cna_song_get_track_number") {
                    runtime.functions.songGetTrackNumber($0, &value)
                }
                return value
            }
        }

        /// `Song.IsProtected`.
        public var IsProtected: Bool {
            get throws {
                var value: UInt8 = 0
                try read("cna_song_get_is_protected") {
                    runtime.functions.songGetIsProtected($0, &value)
                }
                return value != 0
            }
        }

        /// `Song.Artist`, `Song.Album` and `Song.Genre`.
        ///
        /// The three members that kept this type partial through Foundation 92:
        /// each answers a type that carries collections back to songs, so the
        /// whole cycle had to land at once.
        ///
        /// **A divergence, and it is the file factory's fault rather than
        /// XNA's.** All three returns are `PROVEN_NONNULL_SUCCESS`, because in
        /// XNA a song comes from a media library and always has a library
        /// context. CNA says plainly that one created from a file path does
        /// not -- `out_available` reports false, "an ordinary answer, not a
        /// failure" -- and a non-Optional getter has nothing to return then.
        /// So the absence is reported on the runtime channel, naming the route
        /// and the reason, rather than fabricated as an empty artist.
        ///
        /// **Each call is written out rather than routed through a helper.**
        /// Passing a `@convention(c)` function pointer as a Swift closure
        /// argument produces a broken re-abstraction thunk in this toolchain --
        /// it crashed SILGen outright in `SoundEffect`, and the collection
        /// storage segfaulted on the same shape.
        public var Artist: Artist {
            get throws {
                let live = try validatedHandle()
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.songGetArtist(live, &produced, &available),
                    operation: "cna_song_get_artist")
                try Song.requireLibraryContext(available, "Artist")
                return Microsoft.Xna.Framework.Media.Artist(
                    handle: produced, runtime: runtime, borrowed: true)
            }
        }

        public var Album: Album {
            get throws {
                let live = try validatedHandle()
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.songGetAlbum(live, &produced, &available),
                    operation: "cna_song_get_album")
                try Song.requireLibraryContext(available, "Album")
                return Microsoft.Xna.Framework.Media.Album(
                    handle: produced, runtime: runtime, borrowed: true)
            }
        }

        public var Genre: Genre {
            get throws {
                let live = try validatedHandle()
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.songGetGenre(live, &produced, &available),
                    operation: "cna_song_get_genre")
                try Song.requireLibraryContext(available, "Genre")
                return Microsoft.Xna.Framework.Media.Genre(
                    handle: produced, runtime: runtime, borrowed: true)
            }
        }

        private static func requireLibraryContext(
            _ available: UInt8, _ member: String
        ) throws {
            guard available == 0 else { return }
            throw CNAError.nativeFailure(
                operation: "Song.\(member)", result: 1,
                message: "this song was created from a file path and has no "
                    + "media-library context, so it has no \(member.lowercased()); "
                    + "XNA reaches one only through a library")
        }

        /// `Song.Dispose()`.
        ///
        /// **Two native routes, and the difference matters.** `cna_song_dispose`
        /// is the managed disposal XNA's `Dispose` performs -- the song stops
        /// answering -- while `cna_song_destroy` releases the handle itself.
        /// Both run here, in that order, because a Swift facade owns the handle
        /// as well as the disposal.
        public func Dispose() throws {
            guard !disposed else { return }
            disposed = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.songDispose(live),
                operation: "cna_song_dispose")
            guard !borrowed else { return }
            try runtime.functions.check(
                runtime.functions.songDestroy(live),
                operation: "cna_song_destroy")
        }

        /// `Song.Equals(Song other)`.
        ///
        /// The comparison is CNA's, not a handle test: two facades over the
        /// same song answer equal even though their handles differ, which is
        /// the identity a media store owns rather than one this binding can
        /// invent.
        public func Equals(_ other: Song?) -> Bool {
            guard let other, !disposed, !other.disposed else {
                return other == nil ? false : self === other
            }
            var equal: UInt8 = 0
            guard runtime.functions.songEquals(
                handle, other.handle, &equal) == 0 else { return self === other }
            return equal != 0
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Song else { return false }
            return Equals(other)
        }

        /// `Song.ToString()`, which XNA answers with the song's name.
        public func ToString() -> String? {
            (try? Name) ?? ""
        }

        /// `Song.GetHashCode()`, taken from CNA so that two facades over one
        /// song hash alike -- the same reason `Equals` does not compare
        /// handles.
        public func GetHashCode() -> Int32 {
            guard !disposed else { return 0 }
            var value: Int32 = 0
            guard runtime.functions.songGetHashCode(handle, &value) == 0 else {
                return 0
            }
            return value
        }

        public static func == (lhs: Song, rhs: Song) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Song, rhs: Song) -> Bool { !lhs.Equals(rhs) }

        private func read(
            _ operation: String, _ body: (UInt64) -> UInt32
        ) throws {
            let live = try validatedHandle()
            try runtime.functions.check(body(live), operation: operation)
        }

        private func validatedHandle() throws -> UInt64 {
            guard !disposed, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "Song")
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
