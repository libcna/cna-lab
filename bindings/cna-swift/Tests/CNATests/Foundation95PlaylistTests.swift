// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// `Playlist`, `PlaylistCollection`, `MediaSource`, and the members that
/// complete `MediaLibrary`.
final class Foundation95PlaylistTests: XCTestCase {

    /// The seventh collection answers like the other six.
    func testTheLibraryAnswersItsPlaylists() throws {
        let game = try PlaylistProbeGame()
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else {
            throw XCTSkip("no media library: \(game.libraryFailure!)")
        }
        XCTAssertGreaterThanOrEqual(game.playlistCount ?? -1, 0)
        // **Measured: disposing the library does NOT cascade.** A collection
        // it handed out is a separate runtime child, alive until the GAME tears
        // down -- which is XNA's shape too, where a MediaLibrary's Dispose
        // disposes the library and not the objects it published.
        XCTAssertEqual(game.playlistsDisposedAfterLibrary, false,
                       "the collection outlives the library that answered it")
    }

    /// `MediaSource` carries an index rather than a handle, so a source is its
    /// position in the runtime's enumeration -- and the enumeration is walked
    /// once, when the list is built.
    func testTheAvailableSourcesAreEnumeratedOnce() throws {
        let game = try PlaylistProbeGame(readSources: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        if let refusal = game.sourceFailure { throw refusal }
        let count = try XCTUnwrap(game.sourceCount)
        XCTAssertGreaterThanOrEqual(count, 0)
        // The list must hold EVERY source the runtime counts. Comparing the
        // two is what catches a walk that stops short -- the length alone
        // would agree with itself.
        XCTAssertEqual(count, game.reportedSourceCount.map(Int32.init),
                       "the enumeration is walked to its end")
        if count > 0 {
            XCTAssertNotNil(game.firstSourceName)
            XCTAssertEqual(game.firstSourceToString, game.firstSourceName,
                           "ToString is the name")
        }
    }

    /// A library opened from a source is a library like any other. If this host
    /// publishes no source, that is recorded rather than skipped silently.
    func testALibraryCanBeOpenedFromASource() throws {
        let game = try PlaylistProbeGame(openFromSource: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.sourceCount ?? 0 > 0 else {
            throw XCTSkip("this host publishes no media source")
        }
        if let refusal = game.fromSourceFailure { throw refusal }
        XCTAssertEqual(game.fromSourceDisposed, false)
        XCTAssertTrue(game.nullSourceFailure is CNAArgumentNullException)
    }

    /// **`MediaLibrary.MediaSource` is always nil, and that is CNA's shape.**
    /// The runtime publishes a library's source only as a name, and a name is
    /// not a `MediaSource` -- the type carries an enumeration index this
    /// binding would have to guess at. The return is proven nullable, so nil is
    /// a state XNA itself produces.
    func testTheLibrarysOwnSourceIsNil() throws {
        let game = try PlaylistProbeGame(readOwnSource: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else { throw XCTSkip("no media library") }
        XCTAssertEqual(game.ownSourceWasNil, true)
        XCTAssertTrue(game.ownSourceAfterDisposeFailure is CNAObjectDisposedException,
                      "it still refuses once the library is gone, because the "
                      + "CLR getter is fallible")
    }
}

private final class PlaylistProbeGame: Microsoft.Xna.Framework.Game {
    let readSources: Bool
    let openFromSource: Bool
    let readOwnSource: Bool

    var failure: Error?
    var libraryFailure: Error?
    var sourceFailure: Error?
    var playlistCount: Int32?
    var playlistsDisposedAfterLibrary: Bool?
    var sourceCount: Int32?
    var firstSourceName: String?
    var reportedSourceCount: UInt32?
    var firstSourceToString: String?
    var fromSourceFailure: Error?
    var fromSourceDisposed: Bool?
    var nullSourceFailure: Error?
    var ownSourceWasNil: Bool?
    var ownSourceAfterDisposeFailure: Error?

    init(readSources: Bool = false, openFromSource: Bool = false,
         readOwnSource: Bool = false) throws {
        self.readSources = readSources
        self.openFromSource = openFromSource
        self.readOwnSource = readOwnSource
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        typealias M = Microsoft.Xna.Framework.Media
        do {
            if readSources || openFromSource {
                let sources: CNAList<M.MediaSource>
                do { sources = try M.MediaSource.GetAvailableMediaSources() }
                catch { sourceFailure = error; return }
                sourceCount = sources.Count
                // Read straight from the route, so the assertion compares the
                // list against the runtime rather than against itself.
                var reported: UInt32 = 0
                if let rt = try? RuntimeRegistry.current(),
                   rt.functions.mediaSourceGetAvailableCount(
                       rt.gameHandle, &reported) == 0 {
                    reportedSourceCount = reported
                }
                if sources.Count > 0 {
                    let first = try sources.Item(0)
                    firstSourceName = first.Name
                    firstSourceToString = first.ToString()
                    if openFromSource {
                        do {
                            let library = try M.MediaLibrary(mediaSource: first)
                            fromSourceDisposed = library.IsDisposed
                            try library.Dispose()
                        } catch { fromSourceFailure = error }
                        do { _ = try M.MediaLibrary(mediaSource: nil) }
                        catch { nullSourceFailure = error }
                    }
                }
                return
            }

            let library: M.MediaLibrary
            do { library = try M.MediaLibrary() }
            catch { libraryFailure = error; return }

            if readOwnSource {
                ownSourceWasNil = try library.MediaSource == nil
                try library.Dispose()
                do { _ = try library.MediaSource }
                catch { ownSourceAfterDisposeFailure = error }
                return
            }

            let playlists = try library.Playlists
            playlistCount = try playlists?.Count
            try library.Dispose()
            playlistsDisposedAfterLibrary = playlists?.IsDisposed
            try playlists?.Dispose()
        } catch {
            failure = error
        }
    }
}
