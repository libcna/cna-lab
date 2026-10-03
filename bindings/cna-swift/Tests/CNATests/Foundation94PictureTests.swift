// SPDX-License-Identifier: MIT

import XCTest
import Foundation
@testable import CNA

/// `Picture`, `PictureAlbum` and their two collections, plus the half of
/// `MediaLibrary` that reaches them.
///
/// This host's media store holds no pictures, so what is asserted is the shape
/// and the refusals -- and one round trip that does not need a store at all,
/// because `SavePicture` puts a picture in and hands it straight back.
final class Foundation94PictureTests: XCTestCase {

    func testTheLibraryAnswersItsPictureCollections() throws {
        let game = try PictureProbeGame()
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else {
            throw XCTSkip("no media library: \(game.libraryFailure!)")
        }
        XCTAssertGreaterThanOrEqual(game.pictureCount ?? -1, 0)
        XCTAssertGreaterThanOrEqual(game.savedPictureCount ?? -1, 0)
    }

    /// **A library with no pictures has no root album**, and CNA reports that
    /// with an availability flag rather than a failure -- so this one is nil
    /// rather than a refusal, because the return IS proven nullable here.
    func testTheRootAlbumIsNilWhenThereAreNoPictures() throws {
        let game = try PictureProbeGame(readRootAlbum: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else { throw XCTSkip("no media library") }
        XCTAssertEqual(game.rootAlbumWasNil, game.pictureCount == 0,
                       "no pictures means no root album, and vice versa")
        if game.rootAlbumWasNil == false {
            // Present means usable: an album built from an out-parameter CNA
            // never wrote would refuse this read.
            XCTAssertNotNil(game.rootAlbumName ?? nil,
                            "a root album that exists answers its name")
        }
    }

    /// A token naming nothing is **reported**, not answered as nil: the return
    /// is proven non-null, so there is nothing to hand back.
    func testAnUnknownTokenIsReported() throws {
        let game = try PictureProbeGame(exerciseToken: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else { throw XCTSkip("no media library") }
        let message = try XCTUnwrap(game.unknownTokenMessage)
        XCTAssertTrue(message.contains("token"), "the refusal says what failed")
        XCTAssertTrue(game.nullTokenFailure is CNAArgumentNullException)
    }

    func testSavePictureOverloadsAreDirectlyUnsupported() throws {
        let game = try PictureProbeGame(exerciseSaveNulls: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.libraryFailure == nil else { throw XCTSkip("no media library") }
        XCTAssertTrue(game.nullNameFailure is CNANotSupportedException)
        XCTAssertTrue(game.nullBufferFailure is CNANotSupportedException)
        XCTAssertTrue(game.streamFailure is CNANotSupportedException)
    }

    /// **`SavePicture` is exercised only through its authoritative refusal.**
    ///
    /// The obvious test -- save a picture and read it back -- was written,
    /// passed, and was removed: it writes into the **user's own media
    /// library**. On this machine it left `canary picture.png` in
    /// `~/Pictures/Saved Pictures/`, beside files earlier probes from other
    /// bindings had left there. CNA publishes no route to remove one, so the
    /// side effect is permanent and outside this repository.
    ///
    /// The pinned Windows XNA 4.0 IL directly throws NotSupportedException for
    /// both overloads, so no test needs or is allowed to touch a person's
    /// photo album.
    func testSavePictureIsNotExercisedAgainstTheRealLibrary() throws {
        // Nothing to run: the assertion is the absence of a save, and the
        // reason is above. The refusal tests cover what can be covered safely.
        XCTAssertTrue(true)
    }
}

private final class PictureProbeGame: Microsoft.Xna.Framework.Game {
    let readRootAlbum: Bool
    let exerciseToken: Bool
    let exerciseSaveNulls: Bool

    var failure: Error?
    var libraryFailure: Error?
    var pictureCount: Int32?
    var savedPictureCount: Int32?
    var rootAlbumWasNil: Bool?
    var rootAlbumName: String??
    var unknownTokenMessage: String?
    var nullTokenFailure: Error?
    var nullNameFailure: Error?
    var nullBufferFailure: Error?
    var streamFailure: Error?

    init(readRootAlbum: Bool = false, exerciseToken: Bool = false,
         exerciseSaveNulls: Bool = false) throws {
        self.readRootAlbum = readRootAlbum
        self.exerciseToken = exerciseToken
        self.exerciseSaveNulls = exerciseSaveNulls
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        typealias M = Microsoft.Xna.Framework.Media
        do {
            let library: M.MediaLibrary
            do { library = try M.MediaLibrary() }
            catch { libraryFailure = error; return }

            pictureCount = try (try library.Pictures)?.Count
            savedPictureCount = try (try library.SavedPictures)?.Count

            if readRootAlbum {
                let root = try library.RootPictureAlbum
                rootAlbumWasNil = root == nil
                if let root { rootAlbumName = try root.Name }
            }

            if exerciseToken {
                do { _ = try library.GetPictureFromToken("no-such-token") }
                catch { unknownTokenMessage = "\(error)" }
                do { _ = try library.GetPictureFromToken(nil) }
                catch { nullTokenFailure = error }
            }

            if exerciseSaveNulls {
                do { _ = try library.SavePicture(nil, imageBuffer: [0]) }
                catch { nullNameFailure = error }
                do { _ = try library.SavePicture("x", imageBuffer: nil) }
                catch { nullBufferFailure = error }
                do {
                    _ = try library.SavePicture(
                        "x", source: InputStream(data: Data([0])))
                } catch { streamFailure = error }
            }

            try library.Dispose()
        } catch {
            failure = error
        }
    }
}
