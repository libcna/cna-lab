// SPDX-License-Identifier: MIT

import CNAShim

/// The four Media collections share one implementation.
///
/// XNA declares `SongCollection`, `ArtistCollection`, `AlbumCollection` and
/// `GenreCollection` as four separate sealed classes with the identical five
/// members, and CNA publishes four identical route families for them. Four
/// hand-written copies would be four places for the same defect to live, so the
/// behaviour lives here once and each public class is a thin shell over it.
///
/// It is deliberately **not** a public generic base: XNA's four types derive
/// from `System.Object`, and a shared base class would be a shape the pinned
/// contract does not have.
///
/// **The family is selected by an enum and every route is called directly.**
/// The obvious design -- a struct of five closures, one per route -- was
/// written first and segfaulted: passing a `@convention(c)` function pointer as
/// a Swift closure argument produces a broken re-abstraction thunk in this
/// toolchain. The same pattern crashed SILGen outright in `SoundEffect`. A
/// `switch` per method is longer and it works.
internal final class MediaCollectionStorage {

    enum Family: String {
        case song, artist, album, genre
        case picture
        case pictureAlbum = "picture_album"
        case playlist

        /// The Swift type name a refusal names, which is not the route prefix:
        /// `picture_album` is one word in C and two in Swift.
        var typeName: String {
            switch self {
            case .pictureAlbum: return "PictureAlbumCollection"
            default: return rawValue.capitalized + "Collection"
            }
        }
    }

    private let runtime: RuntimeState
    private var handle: UInt64
    private let family: Family
    private var released = false

    init(handle: UInt64, runtime: RuntimeState, family: Family) {
        self.handle = handle
        self.runtime = runtime
        self.family = family
    }

    /// The handle a caller outside this file needs -- `MediaPlayer.Play`
    /// takes a song collection by handle. Internal: no XNA member publishes it.
    var nativeHandle: UInt64 { handle }

    /// `IsDisposed`, which is `IL_NO_FAILURE_PATH` on every one of the four --
    /// so it reads CNA and falls back to this facade's own state.
    var isDisposed: Bool {
        guard !released, handle != 0 else { return true }
        var value: UInt8 = 0
        let functions = runtime.functions
        let result: UInt32
        switch family {
        case .song: result = functions.songCollectionGetIsDisposed(handle, &value)
        case .artist: result = functions.artistCollectionGetIsDisposed(handle, &value)
        case .album: result = functions.albumCollectionGetIsDisposed(handle, &value)
        case .genre: result = functions.genreCollectionGetIsDisposed(handle, &value)
        case .picture: result = functions.pictureCollectionGetIsDisposed(handle, &value)
        case .pictureAlbum: result = functions.pictureAlbumCollectionGetIsDisposed(handle, &value)
        case .playlist: result = functions.playlistCollectionGetIsDisposed(handle, &value)
        }
        guard result == 0 else { return released }
        return value != 0
    }

    /// `Count`, `IL_REACHABLE_THROW` with `ObjectDisposedException`.
    func count() throws -> Int32 {
        let live = try validated()
        var value: Int32 = 0
        let functions = runtime.functions
        let result: UInt32
        switch family {
        case .song: result = functions.songCollectionGetCount(live, &value)
        case .artist: result = functions.artistCollectionGetCount(live, &value)
        case .album: result = functions.albumCollectionGetCount(live, &value)
        case .genre: result = functions.genreCollectionGetCount(live, &value)
        case .picture: result = functions.pictureCollectionGetCount(live, &value)
        case .pictureAlbum: result = functions.pictureAlbumCollectionGetCount(live, &value)
        case .playlist: result = functions.playlistCollectionGetCount(live, &value)
        }
        try functions.check(
            result, operation: "cna_\(family.rawValue)_collection_get_count")
        return value
    }

    /// `Item[index]`, whose declared refusal is `ArgumentOutOfRangeException`.
    ///
    /// The bounds test is **the binding's**: CNA answers its own failure for an
    /// index it does not have, and that is not the exception XNA's indexer
    /// declares. Checking here also means the refusal names the parameter XNA
    /// names.
    func handle(at index: Int32) throws -> UInt64 {
        let live = try validated()
        let total = try count()
        guard index >= 0, index < total else {
            throw CNAArgumentOutOfRangeException(paramName: "index")
        }
        var produced: UInt64 = 0
        let functions = runtime.functions
        let result: UInt32
        switch family {
        case .song: result = functions.songCollectionGetAt(live, index, &produced)
        case .artist: result = functions.artistCollectionGetAt(live, index, &produced)
        case .album: result = functions.albumCollectionGetAt(live, index, &produced)
        case .genre: result = functions.genreCollectionGetAt(live, index, &produced)
        case .picture: result = functions.pictureCollectionGetAt(live, index, &produced)
        case .pictureAlbum: result = functions.pictureAlbumCollectionGetAt(live, index, &produced)
        case .playlist: result = functions.playlistCollectionGetAt(live, index, &produced)
        }
        try functions.check(
            result, operation: "cna_\(family.rawValue)_collection_get_at")
        return produced
    }

    /// `Dispose()`. Both native halves run, in CNA's own order: the collection
    /// is marked disposed and then the handle is released.
    func dispose() throws {
        guard !released else { return }
        released = true
        let live = handle
        handle = 0
        let functions = runtime.functions
        let disposeResult: UInt32
        let destroyResult: UInt32
        switch family {
        case .song:
            disposeResult = functions.songCollectionDispose(live)
            destroyResult = functions.songCollectionDestroy(live)
        case .artist:
            disposeResult = functions.artistCollectionDispose(live)
            destroyResult = functions.artistCollectionDestroy(live)
        case .album:
            disposeResult = functions.albumCollectionDispose(live)
            destroyResult = functions.albumCollectionDestroy(live)
        case .genre:
            disposeResult = functions.genreCollectionDispose(live)
            destroyResult = functions.genreCollectionDestroy(live)
        case .picture:
            disposeResult = functions.pictureCollectionDispose(live)
            destroyResult = functions.pictureCollectionDestroy(live)
        case .pictureAlbum:
            disposeResult = functions.pictureAlbumCollectionDispose(live)
            destroyResult = functions.pictureAlbumCollectionDestroy(live)
        case .playlist:
            disposeResult = functions.playlistCollectionDispose(live)
            destroyResult = functions.playlistCollectionDestroy(live)
        }
        try functions.check(
            disposeResult, operation: "cna_\(family.rawValue)_collection_dispose")
        try functions.check(
            destroyResult, operation: "cna_\(family.rawValue)_collection_destroy")
    }

    private func validated() throws -> UInt64 {
        guard !released, handle != 0 else {
            throw CNAObjectDisposedException(
                objectName: family.typeName)
        }
        return handle
    }
}
