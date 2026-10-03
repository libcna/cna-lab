// SPDX-License-Identifier: MIT

import XCTest
import Foundation
@testable import CNA

/// `Artist`, `Album`, `Genre`, the four Media collections, and the half of
/// `MediaLibrary` that reaches them.
///
/// These types are one cycle -- each entity reaches songs and albums, and every
/// collection answers an entity -- so they land together. The library is the
/// only door: a song created from a file path has no library context at all,
/// which is measured here rather than assumed.
final class Foundation93MediaGraphTests: XCTestCase {

    /// **Measured, and it is the file factory's limit rather than a defect.**
    /// CNA reports "not available" for the artist of a song built from a path,
    /// and the return is proven non-null, so the absence has to be reported.
    func testAFileSongHasNoLibraryContext() throws {
        let fixture = try Foundation92SongTests.writeFixture()
        defer { Foundation92SongTests.removeFixture(fixture) }
        let game = try MediaGraphProbeGame(fixture: fixture)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no media backend") }
        for member in ["Artist", "Album", "Genre"] {
            let message = try XCTUnwrap(game.messages[member])
            XCTAssertTrue(message.contains("media-library context"),
                          "\(member) says why it cannot answer: \(message)")
        }
    }

    /// The library opens, and every collection it publishes answers a count.
    /// This host's media store is empty, which is the honest expectation: what
    /// is asserted is that the collections **work**, not that they hold
    /// anything.
    func testTheLibraryOpensAndItsCollectionsAnswer() throws {
        let game = try MediaGraphProbeGame(
            fixture: try Foundation92SongTests.writeFixture(), openLibrary: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else {
            throw XCTSkip("no media library here: \(game.libraryFailure!)")
        }
        XCTAssertEqual(game.libraryDisposed, false)
        XCTAssertGreaterThanOrEqual(game.songCount ?? -1, 0)
        XCTAssertGreaterThanOrEqual(game.artistCount ?? -1, 0)
        XCTAssertGreaterThanOrEqual(game.albumCount ?? -1, 0)
        XCTAssertGreaterThanOrEqual(game.genreCount ?? -1, 0)
    }

    /// The indexer refuses an index it does not have, and **the refusal is the
    /// binding's**: CNA answers its own failure for a bad index, which is not
    /// the `ArgumentOutOfRangeException` XNA's indexer declares.
    func testTheIndexerRefusesAnIndexOutOfRange() throws {
        let game = try MediaGraphProbeGame(
            fixture: try Foundation92SongTests.writeFixture(), exerciseBounds: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else { throw XCTSkip("no media library") }
        let past = try XCTUnwrap(
            game.outOfRangeFailure as? CNAArgumentOutOfRangeException)
        XCTAssertEqual(past.ParamName, "index")
        let negative = try XCTUnwrap(
            game.negativeIndexFailure as? CNAArgumentOutOfRangeException)
        XCTAssertEqual(negative.ParamName, "index")
    }

    /// A disposed collection refuses its count, and disposing twice is a
    /// no-op -- the shape every disposable in this binding has.
    func testADisposedCollectionRefusesItsCount() throws {
        let game = try MediaGraphProbeGame(
            fixture: try Foundation92SongTests.writeFixture(), disposeCollection: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else { throw XCTSkip("no media library") }
        XCTAssertTrue(game.disposedCountFailure is CNAObjectDisposedException)
        XCTAssertTrue(game.secondDisposeSucceeded)

        // **Measured: disposal is shared, not per-handle.** Two facades over
        // the library's songs name one collection, and releasing either one is
        // visible from the other -- the opposite of `Song`, where two equal
        // songs are separate objects with independent disposal.
        //
        // What is NOT established is which native half does it: removing
        // `cna_song_collection_dispose` and keeping only `destroy` leaves this
        // assertion passing, so `destroy` alone is enough to make a collection
        // report disposed here.
        XCTAssertEqual(game.secondFacadeDisposed, true,
                       "disposing one facade disposes the collection")
    }

    /// Everything the library hands out joins the runtime's child registry, so
    /// a caller who disposes none of it does not make the game's own teardown
    /// fail.
    func testEverythingJoinsTheParentRegistry() throws {
        let game = try MediaGraphProbeGame(
            fixture: try Foundation92SongTests.writeFixture(), leakOnPurpose: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else { throw XCTSkip("no media library") }
        let library = try XCTUnwrap(game.escapedLibrary)
        let songs = try XCTUnwrap(game.escapedSongs)
        XCTAssertTrue(library.IsDisposed, "the parent released the library")
        XCTAssertTrue(songs.IsDisposed, "and the collection it handed out")
    }
}

private final class MediaGraphProbeGame: Microsoft.Xna.Framework.Game {
    let fixture: Foundation.URL
    let openLibrary: Bool
    let exerciseBounds: Bool
    let disposeCollection: Bool
    let leakOnPurpose: Bool

    var failure: Error?
    var createFailure: Error?
    var libraryFailure: Error?
    var messages: [String: String] = [:]
    var libraryDisposed: Bool?
    var songCount: Int32?
    var artistCount: Int32?
    var albumCount: Int32?
    var genreCount: Int32?
    var outOfRangeFailure: Error?
    var negativeIndexFailure: Error?
    var disposedCountFailure: Error?
    var secondDisposeSucceeded = false
    var secondFacadeDisposed: Bool?
    var escapedLibrary: Microsoft.Xna.Framework.Media.MediaLibrary?
    var escapedSongs: Microsoft.Xna.Framework.Media.SongCollection?

    init(fixture: Foundation.URL, openLibrary: Bool = false,
         exerciseBounds: Bool = false, disposeCollection: Bool = false,
         leakOnPurpose: Bool = false) throws {
        self.fixture = fixture
        self.openLibrary = openLibrary
        self.exerciseBounds = exerciseBounds
        self.disposeCollection = disposeCollection
        self.leakOnPurpose = leakOnPurpose
        try super.init()
    }

    private func XCTUnwrapInGame<T>(_ value: T?) throws -> T {
        guard let value else {
            throw CNAError.streamFailure("the library answered no collection")
        }
        return value
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer {
            Foundation92SongTests.removeFixture(fixture)
            try? Exit()
        }
        typealias M = Microsoft.Xna.Framework.Media
        do {
            if !openLibrary && !exerciseBounds && !disposeCollection && !leakOnPurpose {
                let song: M.Song
                do { song = try M.Song.FromUri("probe song", uri: fixture) }
                catch { createFailure = error; return }
                for (name, read) in [
                    ("Artist", { _ = try song.Artist }),
                    ("Album", { _ = try song.Album }),
                    ("Genre", { _ = try song.Genre }),
                ] as [(String, () throws -> Void)] {
                    do { try read() } catch { messages[name] = "\(error)" }
                }
                try song.Dispose()
                return
            }

            let library: M.MediaLibrary
            do { library = try M.MediaLibrary() }
            catch { libraryFailure = error; return }
            libraryDisposed = library.IsDisposed

            // The four collections are Optional because their returns are
            // proven nullable; CNA always answers one, which is the state this
            // host produces.
            let songs = try XCTUnwrapInGame(try library.Songs)
            songCount = try songs.Count
            artistCount = try (try library.Artists)?.Count
            albumCount = try (try library.Albums)?.Count
            genreCount = try (try library.Genres)?.Count

            if exerciseBounds {
                let total = try songs.Count
                do { _ = try songs[total] } catch { outOfRangeFailure = error }
                do { _ = try songs[-1] } catch { negativeIndexFailure = error }
            }

            if disposeCollection {
                // A second facade over the library's songs, to see whether
                // CNA's disposal MARK is shared or per-handle.
                let second = try XCTUnwrapInGame(try library.Songs)
                try songs.Dispose()
                secondFacadeDisposed = second.IsDisposed
                do { _ = try songs.Count } catch { disposedCountFailure = error }
                try songs.Dispose()
                secondDisposeSucceeded = true
            }

            if leakOnPurpose {
                escapedLibrary = library
                escapedSongs = songs
                return  // nothing is disposed: the parent must do it
            }
            try library.Dispose()
        } catch {
            failure = error
        }
    }
}
