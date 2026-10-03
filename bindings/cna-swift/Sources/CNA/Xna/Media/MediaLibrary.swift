// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.MediaLibrary`.
    ///
    /// **The door into the Media graph**, and the reason it is projected now
    /// rather than later: `Artist`, `Album`, `Genre` and the four collections
    /// are correct and ABI-verified, and without a library nothing on this host
    /// can obtain one. A song created from a file path has no library context
    /// at all -- CNA says so and `Song.Artist` reports it.
    ///
    /// Both `SavePicture` overloads preserve the pinned Windows XNA runtime's
    /// direct `NotSupportedException` after its disposal check. CNA's host
    /// media-saving routes are deliberately not bound to them: those routes
    /// implement a broader host policy that the selected XNA assembly does not.
    public final class MediaLibrary: RuntimeOwnedChild {

        private var runtime: RuntimeState!
        private var handle: UInt64 = 0
        private var released = false

        /// `MediaLibrary()`, the parameterless constructor.
        public init() throws {
            let rt = try RuntimeRegistry.current()
            var created: UInt64 = 0
            try rt.functions.check(
                rt.functions.mediaLibraryCreate(rt.gameHandle, &created),
                operation: "cna_media_library_create")
            runtime = rt
            handle = created
            rt.register(self)
        }

        /// `MediaLibrary.IsDisposed`.
        public var IsDisposed: Bool {
            guard !released, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.mediaLibraryGetIsDisposed(
                handle, &value) == 0 else { return released }
            return value != 0
        }

        /// `MediaLibrary.Songs`.
        ///
        /// **Optional, and so are the other three.** Each return is proven
        /// nullable, which is XNA reporting that a library without that kind
        /// of media has no collection to hand back rather than an empty one.
        /// CNA always answers a collection here, so the nil case is a state
        /// this host does not produce -- but the projection carries it, because
        /// the pinned metadata says a caller can meet it.
        public var Songs: SongCollection? {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.mediaLibraryGetSongs(live, &produced),
                    operation: "cna_media_library_get_songs")
                return SongCollection(handle: produced, runtime: runtime)
            }
        }

        /// `MediaLibrary.Artists`.
        public var Artists: ArtistCollection? {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.mediaLibraryGetArtists(live, &produced),
                    operation: "cna_media_library_get_artists")
                return ArtistCollection(handle: produced, runtime: runtime)
            }
        }

        /// `MediaLibrary.Albums`.
        public var Albums: AlbumCollection? {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.mediaLibraryGetAlbums(live, &produced),
                    operation: "cna_media_library_get_albums")
                return AlbumCollection(handle: produced, runtime: runtime)
            }
        }

        /// `MediaLibrary.Genres`.
        public var Genres: GenreCollection? {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.mediaLibraryGetGenres(live, &produced),
                    operation: "cna_media_library_get_genres")
                return GenreCollection(handle: produced, runtime: runtime)
            }
        }

        /// `MediaLibrary(MediaSource mediaSource)`.
        ///
        /// The source carries its index in the runtime's enumeration, which is
        /// what CNA's route takes -- there is no media-source object to pass.
        public convenience init(mediaSource: MediaSource?) throws {
            guard let mediaSource else {
                throw CNAArgumentNullException(paramName: "mediaSource")
            }
            try self.init(sourceIndex: mediaSource.enumerationIndex)
        }

        private init(sourceIndex: UInt32) throws {
            let rt = try RuntimeRegistry.current()
            var created: UInt64 = 0
            try rt.functions.check(
                rt.functions.mediaLibraryCreateFromSource(
                    rt.gameHandle, sourceIndex, &created),
                operation: "cna_media_library_create_from_source")
            runtime = rt
            handle = created
            rt.register(self)
        }

        /// `MediaLibrary.MediaSource`.
        ///
        /// **Nil, always, and that is CNA's shape rather than a gap.** The
        /// runtime publishes a library's source only as a *name*, through
        /// `cna_media_library_copy_media_source_name`, and a name is not a
        /// `MediaSource`: the type carries an enumeration index this binding
        /// would have to guess at. The return is proven nullable, so nil is a
        /// state XNA itself produces, and inventing a source from a matching
        /// name would be a different object that merely looked right.
        public var MediaSource: Microsoft.Xna.Framework.Media.MediaSource? {
            get throws {
                _ = try validated()
                return nil
            }
        }

        /// `MediaLibrary.Playlists`.
        public var Playlists: PlaylistCollection? {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.mediaLibraryGetPlaylists(live, &produced),
                    operation: "cna_media_library_get_playlists")
                return PlaylistCollection(handle: produced, runtime: runtime)
            }
        }

        /// `MediaLibrary.Pictures`.
        public var Pictures: PictureCollection? {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.mediaLibraryGetPictures(live, &produced),
                    operation: "cna_media_library_get_pictures")
                return PictureCollection(handle: produced, runtime: runtime)
            }
        }

        /// `MediaLibrary.SavedPictures`.
        public var SavedPictures: PictureCollection? {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.mediaLibraryGetSavedPictures(live, &produced),
                    operation: "cna_media_library_get_saved_pictures")
                return PictureCollection(handle: produced, runtime: runtime)
            }
        }

        /// `MediaLibrary.RootPictureAlbum`.
        ///
        /// A library with no pictures has no root album, and CNA reports that
        /// with an availability flag rather than a failure.
        public var RootPictureAlbum: PictureAlbum? {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.mediaLibraryGetRootPictureAlbum(
                        live, &produced, &available),
                    operation: "cna_media_library_get_root_picture_album")
                guard available != 0 else { return nil }
                return PictureAlbum(handle: produced, runtime: runtime, borrowed: true)
            }
        }

        /// `MediaLibrary.SavePicture(String name, Byte[] imageBuffer)`.
        public func SavePicture(
            _ name: String?, imageBuffer: [UInt8]?
        ) throws -> Picture {
            _ = name
            _ = imageBuffer
            _ = try validated()
            throw CNANotSupportedException()
        }

        /// `MediaLibrary.SavePicture(String name, Stream source)`.
        public func SavePicture(
            _ name: String?, source: Foundation.InputStream
        ) throws -> Picture {
            _ = name
            _ = source
            _ = try validated()
            throw CNANotSupportedException()
        }

        /// `MediaLibrary.GetPictureFromToken(String token)`.
        ///
        /// A token naming nothing is reported rather than answered as nil: the
        /// return is proven non-null, so there is nothing to hand back.
        public func GetPictureFromToken(_ token: String?) throws -> Picture {
            guard let token else {
                throw CNAArgumentNullException(paramName: "token")
            }
            let live = try validated()
            var utf8 = Array(token.utf8)
            var produced: UInt64 = 0
            var available: UInt8 = 0
            try runtime.functions.check(
                MediaLibrary.withStringView(&utf8) { view in
                    runtime.functions.mediaLibraryGetPictureFromToken(
                        live, view, &produced, &available)
                },
                operation: "cna_media_library_get_picture_from_token")
            guard available != 0 else {
                throw CNAError.nativeFailure(
                    operation: "MediaLibrary.GetPictureFromToken", result: 1,
                    message: "no picture in this library carries that token")
            }
            return Picture(handle: produced, runtime: runtime)
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

        /// `MediaLibrary.Dispose()`.
        public func Dispose() throws {
            guard !released else { return }
            released = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.mediaLibraryDispose(live),
                operation: "cna_media_library_dispose")
            try runtime.functions.check(
                runtime.functions.mediaLibraryDestroy(live),
                operation: "cna_media_library_destroy")
        }

        private func validated() throws -> UInt64 {
            guard !released, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "MediaLibrary")
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { released }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
