// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// `VideoPlayer`, the nineteenth and last Media type.
///
/// Nothing is played: this repository ships no video and the content pipeline
/// that would load one is not projected. What is asserted is the shape, the
/// state of a player with nothing loaded, and the two refusals a caller meets
/// first.
final class Foundation97VideoPlayerTests: XCTestCase {

    func testAFreshPlayerHasNothingLoaded() throws {
        let game = try VideoProbeGame()
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else {
            throw XCTSkip("no video player here: \(game.createFailure!)")
        }
        XCTAssertEqual(game.disposedDuringLoop, false)
        XCTAssertNil(game.videoAtRest ?? nil,
                     "an infallible getter over an availability flag answers nil")
        XCTAssertEqual(game.state, .Stopped)
        XCTAssertEqual(game.isLooped, false)
        XCTAssertEqual(game.isMuted, false)
        XCTAssertGreaterThanOrEqual(game.volume ?? -1, 0)
    }

    /// **The two refusals.** A null video is refused by name; and a `Video`
    /// built from values carries no native handle, so it cannot be played --
    /// which is said rather than passed to the ABI as a zero.
    func testPlayRefusesNullAndAValueBuiltVideo() throws {
        let game = try VideoProbeGame(exerciseRefusals: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no video player") }
        XCTAssertTrue(game.nullVideoFailure is CNAArgumentNullException)
        let message = try XCTUnwrap(game.valueVideoMessage)
        XCTAssertTrue(message.contains("no native handle"),
                      "the refusal says why: \(message)")
    }

    /// With nothing playing there is no frame, and CNA reports that with an
    /// availability flag -- so `GetTexture` refuses rather than answering a
    /// texture built from an out-parameter nobody wrote.
    func testGetTextureRefusesWhenNothingIsPlaying() throws {
        let game = try VideoProbeGame(readTexture: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no video player") }
        let message = try XCTUnwrap(game.textureMessage)
        XCTAssertTrue(message.contains("no frame"),
                      "the refusal says what is missing: \(message)")
    }

    /// Every writer round-trips, and a disposed player refuses them all.
    func testTheWritersRoundTripAndDisposalRefusesThem() throws {
        let game = try VideoProbeGame(exerciseWriters: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no video player") }
        XCTAssertEqual(game.loopedAfter, true)
        XCTAssertEqual(game.mutedAfter, true)
        XCTAssertEqual(game.volumeAfter, 0.5)
        XCTAssertEqual(game.refusedAfterDispose, 4,
                       "all four writers refuse a released player")
        XCTAssertTrue(game.secondDisposeSucceeded)
    }
}

private final class VideoProbeGame: Microsoft.Xna.Framework.Game {
    let exerciseRefusals: Bool
    let readTexture: Bool
    let exerciseWriters: Bool

    var failure: Error?
    var createFailure: Error?
    var disposedDuringLoop: Bool?
    var videoAtRest: Microsoft.Xna.Framework.Media.Video??
    var state: Microsoft.Xna.Framework.Media.MediaState?
    var isLooped: Bool?
    var isMuted: Bool?
    var volume: Float?
    var nullVideoFailure: Error?
    var valueVideoMessage: String?
    var textureMessage: String?
    var loopedAfter: Bool?
    var mutedAfter: Bool?
    var volumeAfter: Float?
    var refusedAfterDispose = 0
    var secondDisposeSucceeded = false

    init(exerciseRefusals: Bool = false, readTexture: Bool = false,
         exerciseWriters: Bool = false) throws {
        self.exerciseRefusals = exerciseRefusals
        self.readTexture = readTexture
        self.exerciseWriters = exerciseWriters
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        typealias M = Microsoft.Xna.Framework.Media
        do {
            let player: M.VideoPlayer
            do { player = try M.VideoPlayer() }
            catch { createFailure = error; return }

            if exerciseRefusals {
                do { try player.Play(nil) } catch { nullVideoFailure = error }
                let built = M.Video(
                    durationMilliseconds: 1000, width: 4, height: 4,
                    framesPerSecond: 30, soundtrackType: .Music)
                do { try player.Play(built) }
                catch { valueVideoMessage = "\(error)" }
                try player.Dispose()
                return
            }

            if readTexture {
                do { _ = try player.GetTexture() }
                catch { textureMessage = "\(error)" }
                try player.Dispose()
                return
            }

            if exerciseWriters {
                try player.SetIsLooped(true)
                try player.SetIsMuted(true)
                try player.SetVolume(0.5)
                loopedAfter = player.IsLooped
                mutedAfter = player.IsMuted
                volumeAfter = player.Volume
                try player.Dispose()
                for write in [{ try player.SetIsLooped(false) },
                              { try player.SetIsMuted(false) },
                              { try player.SetVolume(1) },
                              { try player.Pause() }] as [() throws -> Void] {
                    do { try write() } catch { refusedAfterDispose += 1 }
                }
                try player.Dispose()
                secondDisposeSucceeded = true
                return
            }

            disposedDuringLoop = player.IsDisposed
            videoAtRest = player.Video
            state = try player.State
            isLooped = player.IsLooped
            isMuted = player.IsMuted
            volume = player.Volume
            try player.Dispose()
        } catch {
            failure = error
        }
    }
}
