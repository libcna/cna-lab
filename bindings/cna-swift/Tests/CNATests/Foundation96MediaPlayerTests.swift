// SPDX-License-Identifier: MIT

import XCTest
import Foundation
@testable import CNA

/// `MediaPlayer` and `MediaQueue`.
///
/// The last two types of the music half. Nothing is heard: what is asserted is
/// the shape, the refusals, and one real play of a song opened from a file.
final class Foundation96MediaPlayerTests: XCTestCase {

    /// **The queue is process-wide and outlives the game that filled it.**
    /// CNA says so -- `cna_media_player_get_queue` answers "a view of the
    /// process-wide media queue" -- and this suite measured it the hard way: a
    /// test that played a song left it queued for the next test, whose own game
    /// was a different one. So nothing here asserts an empty queue; what is
    /// asserted is that every read answers whatever the process happens to
    /// hold.
    func testThePlayerAnswersItsState() throws {
        let game = try PlayerProbeGame()
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        if let refusal = game.readFailure { throw refusal }
        XCTAssertNotNil(game.state, "the state reads")
        XCTAssertGreaterThanOrEqual(game.queueCount ?? -1, 0)
        XCTAssertGreaterThanOrEqual(game.activeSongIndexAtRest ?? -2, -1,
                                    "-1 when nothing is active, an index when "
                                    + "something is")
        XCTAssertGreaterThanOrEqual(game.volume ?? -1, 0)
    }

    /// Every writer round-trips through its getter -- and `IsShuffled` and
    /// `IsRepeating` are the pair whose **getter cannot fail while the setter
    /// can**, which is the accessor table's shape and not a guess.
    func testTheWritersRoundTrip() throws {
        let game = try PlayerProbeGame(exerciseWriters: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        if let refusal = game.readFailure { throw refusal }
        XCTAssertEqual(game.shuffledAfter, true)
        XCTAssertEqual(game.repeatingAfter, true)
        XCTAssertEqual(game.mutedAfter, true)
        XCTAssertEqual(game.visualizationAfter, true)
        XCTAssertEqual(game.volumeAfter, 0.25)
    }

    /// A real play: a song opened from a generated file reaches the queue, and
    /// the player reports it.
    func testPlayingASongPutsItInTheQueue() throws {
        let fixture = try Foundation92SongTests.writeFixture()
        defer { Foundation92SongTests.removeFixture(fixture) }
        let game = try PlayerProbeGame(fixture: fixture, playASong: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.songFailure == nil else {
            throw XCTSkip("this host cannot open the fixture: \(game.songFailure!)")
        }
        if let refusal = game.readFailure { throw refusal }
        XCTAssertEqual(game.queueCountWhilePlaying, 1,
                       "Play replaces the queue rather than appending -- one "
                       + "song in, one song queued, whatever was there before")
        XCTAssertEqual(game.activeSongIndexWhilePlaying, 0)
        XCTAssertNotNil(game.activeSongNameWhilePlaying ?? nil)
    }

    /// Every `Play` overload refuses null, and the indexer refuses an index the
    /// queue does not have -- **the binding's refusal**, because CNA answers
    /// its own failure there and XNA declares `ArgumentOutOfRangeException`.
    func testTheRefusals() throws {
        let game = try PlayerProbeGame(exerciseRefusals: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.nullSongFailure is CNAArgumentNullException)
        XCTAssertTrue(game.nullCollectionFailure is CNAArgumentNullException)
        XCTAssertTrue(game.nullVisualizationFailure is CNAArgumentNullException)
        let outOfRange = try XCTUnwrap(
            game.queueIndexFailure as? CNAArgumentOutOfRangeException)
        XCTAssertEqual(outOfRange.ParamName, "index")
    }

    /// `GetVisualizationData` fills the caller's object rather than answering a
    /// new one, and both buffers stay 256 values long -- which is what
    /// `VisualizationData`'s internal write path guarantees.
    func testVisualizationDataIsFilledInPlace() throws {
        let game = try PlayerProbeGame(readVisualization: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        if let refusal = game.visualizationFailure { throw refusal }
        XCTAssertEqual(game.frequencyCount, 256)
        XCTAssertEqual(game.sampleCount, 256)
        XCTAssertTrue(game.sameCollectionsAfterFill,
                      "the object was filled, not replaced")
        // **Measured: this renderer fills both buffers with zeros.** That is
        // an honest answer for a host with nothing playing, and it is recorded
        // because it is also what makes one mutation unfalsifiable here --
        // swapping the two buffers cannot be seen when both are empty.
        XCTAssertEqual(game.nonzeroFrequencies, 0)
        XCTAssertEqual(game.nonzeroSamples, 0)
    }
}

private final class PlayerProbeGame: Microsoft.Xna.Framework.Game {
    let fixture: Foundation.URL?
    let exerciseWriters: Bool
    let playASong: Bool
    let exerciseRefusals: Bool
    let readVisualization: Bool

    var failure: Error?
    var readFailure: Error?
    var songFailure: Error?
    var visualizationFailure: Error?
    var state: Microsoft.Xna.Framework.Media.MediaState?
    var queueCount: Int32?
    var activeSongIndexAtRest: Int32?
    var activeSongAtRest: Microsoft.Xna.Framework.Media.Song??
    var volume: Float?
    var shuffledAfter: Bool?
    var repeatingAfter: Bool?
    var mutedAfter: Bool?
    var visualizationAfter: Bool?
    var volumeAfter: Float?
    var queueCountWhilePlaying: Int32?
    var activeSongIndexWhilePlaying: Int32?
    var activeSongNameWhilePlaying: String??
    var nullSongFailure: Error?
    var nullCollectionFailure: Error?
    var nullVisualizationFailure: Error?
    var queueIndexFailure: Error?
    var frequencyCount: Int32?
    var sampleCount: Int32?
    var sameCollectionsAfterFill = false
    var nonzeroFrequencies: Int?
    var nonzeroSamples: Int?
    var buffersIdentical: Bool?

    init(fixture: Foundation.URL? = nil, exerciseWriters: Bool = false,
         playASong: Bool = false, exerciseRefusals: Bool = false,
         readVisualization: Bool = false) throws {
        self.fixture = fixture
        self.exerciseWriters = exerciseWriters
        self.playASong = playASong
        self.exerciseRefusals = exerciseRefusals
        self.readVisualization = readVisualization
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        typealias M = Microsoft.Xna.Framework.Media
        do {
            if exerciseRefusals {
                do { try M.MediaPlayer.Play(nil as M.Song?) }
                catch { nullSongFailure = error }
                do { try M.MediaPlayer.Play(nil as M.SongCollection?) }
                catch { nullCollectionFailure = error }
                do { try M.MediaPlayer.GetVisualizationData(nil) }
                catch { nullVisualizationFailure = error }
                let queue = M.MediaPlayer.Queue
                do { _ = try queue[try queue.Count] }
                catch { queueIndexFailure = error }
                return
            }

            if readVisualization {
                let data = M.VisualizationData()
                let frequencies = data.Frequencies
                let samples = data.Samples
                do { try M.MediaPlayer.GetVisualizationData(data) }
                catch { visualizationFailure = error; return }
                frequencyCount = data.Frequencies.Count
                sampleCount = data.Samples.Count
                sameCollectionsAfterFill =
                    data.Frequencies === frequencies && data.Samples === samples
                var f: [Float] = [], m: [Float] = []
                for i in 0..<Int(data.Frequencies.Count) {
                    f.append(try data.Frequencies.Item(Int32(i)))
                    m.append(try data.Samples.Item(Int32(i)))
                }
                nonzeroFrequencies = f.filter { $0 != 0 }.count
                nonzeroSamples = m.filter { $0 != 0 }.count
                buffersIdentical = f == m
                return
            }

            if exerciseWriters {
                do {
                    try M.MediaPlayer.SetIsShuffled(true)
                    try M.MediaPlayer.SetIsRepeating(true)
                    try M.MediaPlayer.SetIsMuted(true)
                    try M.MediaPlayer.SetIsVisualizationEnabled(true)
                    try M.MediaPlayer.SetVolume(0.25)
                    shuffledAfter = M.MediaPlayer.IsShuffled
                    repeatingAfter = M.MediaPlayer.IsRepeating
                    mutedAfter = try M.MediaPlayer.IsMuted
                    visualizationAfter = try M.MediaPlayer.IsVisualizationEnabled
                    volumeAfter = try M.MediaPlayer.Volume
                } catch { readFailure = error }
                return
            }

            if playASong, let fixture {
                let song: M.Song
                do { song = try M.Song.FromUri("queued song", uri: fixture) }
                catch { songFailure = error; return }
                do {
                    try M.MediaPlayer.Play(song)
                    let queue = M.MediaPlayer.Queue
                    queueCountWhilePlaying = try queue.Count
                    activeSongIndexWhilePlaying = try queue.ActiveSongIndex
                    activeSongNameWhilePlaying = try (try queue.ActiveSong)?.Name
                    try M.MediaPlayer.Stop()
                } catch { readFailure = error }
                try song.Dispose()
                return
            }

            do {
                state = try M.MediaPlayer.State
                let queue = M.MediaPlayer.Queue
                queueCount = try queue.Count
                activeSongIndexAtRest = try queue.ActiveSongIndex
                // ActiveSong is deliberately NOT read here: it reaches the
                // file behind whatever song the process still has queued, and
                // an earlier test's fixture is already gone.
                volume = try M.MediaPlayer.Volume
            } catch { readFailure = error }
        } catch {
            failure = error
        }
    }
}
