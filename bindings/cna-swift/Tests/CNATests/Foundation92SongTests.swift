// SPDX-License-Identifier: MIT

import XCTest
import Foundation
@testable import CNA

/// `Microsoft.Xna.Framework.Media.Song`.
///
/// The only Media type with a public factory, and therefore the only one a
/// consumer can reach without a media library. Nothing is played: what is
/// asserted is the shape XNA gives a song and the identity CNA owns.
final class Foundation92SongTests: XCTestCase {

    /// A real file on disk, because `cna_song_create_from_uri` checks that
    /// the file exists before anything else -- measured: a URL naming nothing
    /// comes back `Could not find file`. The fixture is a minimal 8 kHz mono
    /// PCM16 WAV written here and removed afterwards; no asset ships with this
    /// repository and none needs to.
    static func writeFixture() throws -> Foundation.URL {
        let samples = 800
        let dataBytes = samples * 2
        var wav = [UInt8]()
        func u32(_ value: Int) { for shift in [0, 8, 16, 24] { wav.append(UInt8((value >> shift) & 0xFF)) } }
        func u16(_ value: Int) { for shift in [0, 8] { wav.append(UInt8((value >> shift) & 0xFF)) } }
        wav.append(contentsOf: Array("RIFF".utf8)); u32(36 + dataBytes)
        wav.append(contentsOf: Array("WAVE".utf8))
        wav.append(contentsOf: Array("fmt ".utf8)); u32(16)
        u16(1); u16(1); u32(8000); u32(16000); u16(2); u16(16)
        wav.append(contentsOf: Array("data".utf8)); u32(dataBytes)
        wav.append(contentsOf: [UInt8](repeating: 0, count: dataBytes))

        let directory = FileManager.default.temporaryDirectory
            .appendingPathComponent("cna-swift-song-fixture", isDirectory: true)
        try FileManager.default.createDirectory(
            at: directory, withIntermediateDirectories: true)
        let url = directory.appendingPathComponent("probe.wav")
        try Data(wav).write(to: url)
        // A second file, so the identity test has two genuinely different
        // songs rather than one file opened twice.
        try Data(wav).write(to: directory.appendingPathComponent("other.wav"))
        return url
    }

    /// The sibling `writeFixture` leaves beside the first.
    static func secondFixture(_ first: Foundation.URL) -> Foundation.URL {
        first.deletingLastPathComponent().appendingPathComponent("other.wav")
    }

    static func removeFixture(_ url: Foundation.URL) {
        try? FileManager.default.removeItem(at: url.deletingLastPathComponent())
    }

    func testFromUriRefusesBothNulls() throws {
        let fixture = try Foundation92SongTests.writeFixture()
        defer { Foundation92SongTests.removeFixture(fixture) }
        let game = try SongProbeGame(exerciseNulls: true, fixture: fixture)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        let noName = try XCTUnwrap(game.nullNameFailure as? CNAArgumentNullException)
        XCTAssertEqual(noName.ParamName, "name")
        let noUri = try XCTUnwrap(game.nullUriFailure as? CNAArgumentNullException)
        XCTAssertEqual(noUri.ParamName, "uri")
    }

    func testASongCarriesTheNameItWasGiven() throws {
        let fixture = try Foundation92SongTests.writeFixture()
        defer { Foundation92SongTests.removeFixture(fixture) }
        let game = try SongProbeGame(fixture: fixture)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        if let refusal = game.createFailure {
            throw XCTSkip("this build cannot open the fixture: \(refusal)")
        }
        XCTAssertEqual(game.name, "probe song")
        XCTAssertEqual(game.toString, "probe song",
                       "ToString is the name, not a braced field list")
        XCTAssertEqual(game.trackNumber, 0)
        XCTAssertFalse(game.isProtected ?? true)
    }

    /// **Identity is CNA's, not the handle's.** Two facades over the same song
    /// must compare and hash alike, which a handle test could not give -- the
    /// media store owns that identity and this binding cannot invent it.
    func testEqualityAndHashComeFromTheRuntime() throws {
        let fixture = try Foundation92SongTests.writeFixture()
        defer { Foundation92SongTests.removeFixture(fixture) }
        let game = try SongProbeGame(
            exerciseIdentity: true, fixture: fixture,
            secondFixture: Foundation92SongTests.secondFixture(fixture))
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no media backend") }
        XCTAssertTrue(game.sameUriEqual,
                      "two FromUri calls on one file answer the same song")
        XCTAssertTrue(game.sameUriHashEqual, "and it hashes alike")
        XCTAssertFalse(game.differentUriEqual,
                       "two different songs are not the same song")
    }

    /// **Measured, and it corrected the obvious guess.** Two `FromUri` calls
    /// on one file are *equal* -- `cna_song_equals` says so -- and yet they are
    /// separate objects: disposing one leaves the other reporting alive.
    ///
    /// So equality here means "the same track", not "the same instance". That
    /// distinction is worth pinning, because anything that caches songs by
    /// equality would otherwise assume one disposal covers every reference.
    func testEqualSongsAreStillSeparateInstances() throws {
        let fixture = try Foundation92SongTests.writeFixture()
        defer { Foundation92SongTests.removeFixture(fixture) }
        let game = try SongProbeGame(exerciseSharedDisposal: true, fixture: fixture)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no media backend") }
        XCTAssertEqual(game.otherDisposedBefore, false, "both start alive")
        XCTAssertEqual(game.otherDisposedAfter, false,
                       "disposing one equal song does not dispose the other")
    }

    /// Every getter but `IsDisposed` is `IL_REACHABLE_THROW`, so a released
    /// song refuses every read rather than answering a stale value.
    func testADisposedSongRefusesEveryRead() throws {
        let fixture = try Foundation92SongTests.writeFixture()
        defer { Foundation92SongTests.removeFixture(fixture) }
        let game = try SongProbeGame(disposeThenRead: true, fixture: fixture)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no media backend") }
        XCTAssertTrue(game.disposedFlag)
        XCTAssertEqual(game.refusedReads, 6,
                       "name, duration, rating, play count, track number and "
                       + "protection all refuse")
        XCTAssertTrue(game.secondDisposeSucceeded, "disposing twice is a no-op")
        XCTAssertEqual(game.toStringAfterDispose, "",
                       "ToString cannot throw, so it answers empty")
    }
}

private final class SongProbeGame: Microsoft.Xna.Framework.Game {
    let exerciseNulls: Bool
    let exerciseIdentity: Bool
    let disposeThenRead: Bool
    let exerciseSharedDisposal: Bool
    let fixture: Foundation.URL
    let secondFixture: Foundation.URL?

    var failure: Error?
    var createFailure: Error?
    var nullNameFailure: Error?
    var nullUriFailure: Error?
    var name: String?
    var toString: String?
    var trackNumber: Int32?
    var isProtected: Bool?
    var sameUriEqual = false
    var sameUriHashEqual = false
    var differentUriEqual = true
    var disposedFlag = false
    var refusedReads = 0
    var secondDisposeSucceeded = false
    var toStringAfterDispose: String?
    var otherDisposedBefore: Bool?
    var otherDisposedAfter: Bool?

    init(exerciseNulls: Bool = false, exerciseIdentity: Bool = false,
         disposeThenRead: Bool = false, exerciseSharedDisposal: Bool = false,
         fixture: Foundation.URL,
         secondFixture: Foundation.URL? = nil) throws {
        self.fixture = fixture
        self.secondFixture = secondFixture
        self.exerciseNulls = exerciseNulls
        self.exerciseSharedDisposal = exerciseSharedDisposal
        self.exerciseIdentity = exerciseIdentity
        self.disposeThenRead = disposeThenRead
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        typealias S = Microsoft.Xna.Framework.Media.Song
        do {
            let probe = fixture

            if exerciseNulls {
                do { _ = try S.FromUri(nil, uri: probe) }
                catch { nullNameFailure = error }
                do { _ = try S.FromUri("probe song", uri: nil) }
                catch { nullUriFailure = error }
                return
            }

            let song: S
            do { song = try S.FromUri("probe song", uri: probe) }
            catch { createFailure = error; return }

            if exerciseIdentity {
                let same = try S.FromUri("probe song", uri: probe)
                sameUriEqual = song.Equals(same)
                sameUriHashEqual = song.GetHashCode() == same.GetHashCode()
                let other = try S.FromUri("another", uri: secondFixture ?? fixture)
                differentUriEqual = song.Equals(other)
                try other.Dispose()
                try same.Dispose()
                try song.Dispose()
                return
            }

            if exerciseSharedDisposal {
                let other = try S.FromUri("probe song", uri: probe)
                otherDisposedBefore = other.IsDisposed
                try song.Dispose()
                otherDisposedAfter = other.IsDisposed
                try other.Dispose()
                return
            }

            if disposeThenRead {
                try song.Dispose()
                disposedFlag = song.IsDisposed
                for read in [{ _ = try song.Name },
                             { _ = try song.Duration },
                             { _ = try song.Rating },
                             { _ = try song.PlayCount },
                             { _ = try song.TrackNumber },
                             { _ = try song.IsProtected }] as [() throws -> Void] {
                    do { try read() } catch { refusedReads += 1 }
                }
                toStringAfterDispose = song.ToString()
                try song.Dispose()
                secondDisposeSucceeded = true
                return
            }

            name = try song.Name
            toString = song.ToString()
            trackNumber = try song.TrackNumber
            isProtected = try song.IsProtected
            try song.Dispose()
        } catch {
            failure = error
        }
    }
}
