// SPDX-License-Identifier: MIT

import XCTest
import Foundation
import CNAShim
@testable import CNA

/// `SoundEffect` and `SoundEffectInstance`.
///
/// **No audio asset is involved.** `cna_sound_effect_create_pcm16` takes raw
/// PCM16LE bytes, so the fixture is generated here -- a tenth of a second of
/// silence -- and the suite tests the binding rather than a file. That is the
/// property that made this family reachable before `Model`, which needs a
/// compiled `.xnb` this repository does not have.
final class Foundation89SoundEffectTests: XCTestCase {

    private typealias Audio = Microsoft.Xna.Framework.Audio

    /// 0.1 s of silence, mono, 8 kHz: 800 frames of one Int16 each.
    static let silence = [UInt8](repeating: 0, count: 1600)

    func testConstructionEitherSucceedsOrRefusesByName() throws {
        let game = try AudioProbeGame()
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }

        if let refusal = game.createFailure {
            XCTAssertTrue("\(refusal)".contains("cna_sound_effect_create_pcm16"),
                          "the refusal names the route that refused")
            return
        }
        XCTAssertEqual(game.disposedDuringLoop, false)
        XCTAssertTrue(game.durationWasPositive,
                      "800 frames at 8 kHz is a tenth of a second, not zero")
    }

    /// The name is nullable, infallible to read, and its writer refuses null.
    func testTheNameRoundTripsAndRefusesNull() throws {
        let game = try AudioProbeGame(exerciseName: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        XCTAssertNil(game.nameBefore, "no constructor assigns the name")
        XCTAssertEqual(game.nameAfter, "probe effect")
        XCTAssertTrue(game.nullNameFailure is CNAArgumentNullException)
    }

    /// The seven-argument constructor takes a **different CNA route** from
    /// the three-argument one, and its range has to reach it: half the buffer
    /// is half the duration.
    func testTheRangeConstructorUsesTheRangeItIsGiven() throws {
        let game = try AudioProbeGame(exerciseRange: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        XCTAssertEqual(game.halfBufferTicks * 2, game.wholeBufferTicks,
                       "800 bytes of an 1600-byte buffer is half the sound")
    }

    /// The two conversions are pure arithmetic in XNA, and CNA agrees: a
    /// duration converted to bytes and back is the duration again.
    func testTheSampleConversionsAreEachOthersInverse() throws {
        let game = try AudioProbeGame(exerciseConversions: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.conversionFailure == nil else {
            throw XCTSkip("conversions unavailable: \(game.conversionFailure!)")
        }
        XCTAssertEqual(game.bytesForATenthOfASecond, 1600,
                       "8000 frames/s * 0.1 s * 2 bytes, mono")
        XCTAssertEqual(game.roundTrippedTicks, 1_000_000,
                       "a tenth of a second is 1,000,000 ticks")
    }

    /// An instance's four controllable properties are `IL_NO_FAILURE_PATH`
    /// getters over one info route, and each writer's range check is the
    /// binding's own: CNA passes volume through unclamped and clamps pitch and
    /// pan, so without these checks an illegal value would be accepted twice
    /// over in two different ways.
    func testTheInstanceWritersRefuseWhatXnaRefuses() throws {
        let game = try AudioProbeGame(exerciseInstance: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }

        XCTAssertEqual(game.volumeAfter, 0.5)
        XCTAssertEqual(game.pitchAfter, -0.25)
        XCTAssertEqual(game.panAfter, 1)
        XCTAssertEqual(game.loopedAfter, true)

        XCTAssertTrue(game.volumeRefusal is CNAArgumentOutOfRangeException,
                      "volume above 1 is refused, not passed through unclamped")
        XCTAssertTrue(game.pitchRefusal is CNAArgumentOutOfRangeException,
                      "pitch outside [-1, 1] is refused, not silently clamped")
        XCTAssertTrue(game.panRefusal is CNAArgumentOutOfRangeException)
    }

    func testTheInstanceStateIsAThrowingGetter() throws {
        let game = try AudioProbeGame(exerciseInstance: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        XCTAssertEqual(game.stateAfterStop, .Stopped)
        XCTAssertTrue(game.stateAfterDispose is CNAObjectDisposedException,
                      "a disposed instance refuses the state it cannot read")
    }

    /// `Apply3D` in both shapes, and the one refusal the array overload has.
    ///
    /// The listener and emitter cross as mirrored structures rather than
    /// handles, so this is also what proves those two mirrors are right: a
    /// wrong field order would be a native failure here, not a wrong sound.
    func testApply3DAcceptsBothShapesAndRefusesANullArray() throws {
        let game = try AudioProbeGame(exercise3D: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        if let refusal = game.apply3DFailure { throw refusal }
        XCTAssertTrue(game.singleListenerAccepted)
        XCTAssertTrue(game.listenerArrayAccepted)
        XCTAssertTrue(game.nullListenersFailure is CNAArgumentNullException,
                      "a null array is refused; an empty one is not the same thing")
    }

    /// `FromStream` refuses null, and the refusal is reachable **because** the
    /// parameter is Optional -- which is the whole reason the mapping rule
    /// names it.
    func testFromStreamRefusesNull() throws {
        let game = try AudioProbeGame(exerciseStream: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.nullStreamFailure is CNAArgumentNullException)
        XCTAssertNotNil(game.emptyStreamFailure,
                        "an empty stream is refused before it reaches CNA")
    }

    /// The four constructor refusals XNA makes and CNA does not: it takes the
    /// byte count it is given and decodes what fits, so an odd-length buffer
    /// or a loop region outside it would be accepted as a shorter sound.
    func testTheConstructorRefusesWhatCnaWouldAccept() throws {
        let game = try AudioProbeGame(exerciseValidation: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.messages["oddLength"],
                       "Ensure that the buffer length is non-zero and meets "
                       + "the block alignment requirements for the audio format.")
        XCTAssertEqual(game.messages["oddOffset"],
                       "Offset must be within the buffer boundaries and meet "
                       + "the block alignment requirements for the audio format.")
        XCTAssertEqual(game.messages["countPastEnd"],
                       "Ensure that count is valid and meets the block "
                       + "alignment requirements for the audio format. Offset "
                       + "and count must define a valid region within the "
                       + "buffer boundaries.")
        XCTAssertEqual(game.messages["loopPastEnd"],
                       "Ensure that the loop region is defined in samples and "
                       + "within the buffer boundaries.")
        XCTAssertEqual(game.messages["negativeSize"], "Buffer size cannot be negative.")
    }

    /// The three stateful rules. Each one closes at the first `Play`, which is
    /// the moment XNA fixes the sound's shape, and CNA enforces none of them.
    func testTheInstanceRulesCloseAtTheFirstPlay() throws {
        let game = try AudioProbeGame(exerciseStateRules: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        XCTAssertEqual(game.messages["loopAfterPlay"],
                       "Loop must be set before the first Play call.")
        XCTAssertEqual(game.messages["apply3DAfterPlay"],
                       "The sound is not a 3D sound. Call Apply3D before the "
                       + "first Play call to configure it to be a 3D sound.")
        XCTAssertEqual(game.messages["panOn3D"],
                       "Pan cannot be set on a 3D sound. To ensure a 2D sound "
                       + "avoid calling Apply3D and ensure Pan is set before "
                       + "the first Play call.")
        // `ObjectDisposedException.get_Message` appends the object name to
        // whatever text it was given, which this binding reproduces -- so the
        // XNA string is the PREFIX, not the whole message.
        let disposed = try XCTUnwrap(game.messages["disposed"])
        XCTAssertTrue(disposed.hasPrefix("This object has already been disposed."),
                      "XNA passes its own text, not the BCL's generic one")
        XCTAssertTrue(disposed.contains("Object name: 'SoundEffectInstance'."),
                      "and the CLR appends the object name after it")

        // The effect carries the same XNA text, and it is asserted separately:
        // one message constant shared by two types is still two refusals, and
        // a mutation on either has to fail something.
        let effectDisposed = try XCTUnwrap(game.messages["effectDisposed"])
        XCTAssertTrue(effectDisposed.hasPrefix("This object has already been disposed."))
        XCTAssertTrue(effectDisposed.contains("Object name: 'SoundEffect'."))
    }

    /// Both types join the runtime's child registry, so a caller who never
    /// disposes one does not make the game's own teardown fail.
    func testTheParentReleasesWhatTheCallerForgot() throws {
        let game = try AudioProbeGame(leakOnPurpose: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        let effect = try XCTUnwrap(game.effect)
        let instance = try XCTUnwrap(game.instance)
        XCTAssertTrue(effect.IsDisposed, "the parent released the effect")
        XCTAssertTrue(instance.IsDisposed, "and the instance it made")
    }
}

private final class AudioProbeGame: Microsoft.Xna.Framework.Game {
    let exerciseName: Bool
    let exerciseConversions: Bool
    let exerciseInstance: Bool
    let leakOnPurpose: Bool
    let exercise3D: Bool
    let exerciseStream: Bool
    let exerciseRange: Bool
    let exerciseValidation: Bool
    let exerciseStateRules: Bool

    var failure: Error?
    var createFailure: Error?
    var conversionFailure: Error?
    var disposedDuringLoop: Bool?
    var durationWasPositive = false
    var nameBefore: String?
    var nameAfter: String?
    var nullNameFailure: Error?
    var bytesForATenthOfASecond: Int32 = -1
    var roundTrippedTicks: Int64 = -1
    var volumeAfter: Float = -1
    var pitchAfter: Float = -1
    var panAfter: Float = -1
    var loopedAfter: Bool?
    var volumeRefusal: Error?
    var pitchRefusal: Error?
    var panRefusal: Error?
    var stateAfterStop: Microsoft.Xna.Framework.Audio.SoundState?
    var stateAfterDispose: Error?
    var effect: Microsoft.Xna.Framework.Audio.SoundEffect?
    var instance: Microsoft.Xna.Framework.Audio.SoundEffectInstance?
    var singleListenerAccepted = false
    var listenerArrayAccepted = false
    var nullListenersFailure: Error?
    var apply3DFailure: Error?
    var nullStreamFailure: Error?
    var emptyStreamFailure: Error?
    var wholeBufferTicks: Int64 = -1
    var halfBufferTicks: Int64 = -2
    var messages: [String: String] = [:]

    init(exerciseName: Bool = false, exerciseConversions: Bool = false,
         exerciseInstance: Bool = false, leakOnPurpose: Bool = false,
         exercise3D: Bool = false, exerciseStream: Bool = false,
         exerciseRange: Bool = false, exerciseValidation: Bool = false,
         exerciseStateRules: Bool = false) throws {
        self.exerciseName = exerciseName
        self.exerciseConversions = exerciseConversions
        self.exerciseInstance = exerciseInstance
        self.leakOnPurpose = leakOnPurpose
        self.exercise3D = exercise3D
        self.exerciseStream = exerciseStream
        self.exerciseRange = exerciseRange
        self.exerciseValidation = exerciseValidation
        self.exerciseStateRules = exerciseStateRules
        try super.init()
    }

    static func ticks(_ duration: Swift.Duration) -> Int64 {
        let parts = duration.components
        return parts.seconds * 10_000_000 + parts.attoseconds / 100_000_000_000
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        do {
            func record(_ key: String, _ body: () throws -> Void) {
                do { try body() }
                catch let error as CNAException { messages[key] = error.Message ?? "" }
                catch { messages[key] = "\(error)" }
            }

            if exerciseValidation {
                typealias SE = Microsoft.Xna.Framework.Audio.SoundEffect
                record("oddLength") {
                    _ = try SE(buffer: [0, 0, 0], sampleRate: 8000, channels: .Mono)
                }
                record("oddOffset") {
                    _ = try SE(buffer: Foundation89SoundEffectTests.silence,
                               offset: 1, count: 8, sampleRate: 8000,
                               channels: .Mono, loopStart: 0, loopLength: 0)
                }
                record("countPastEnd") {
                    _ = try SE(buffer: Foundation89SoundEffectTests.silence,
                               offset: 1598, count: 8, sampleRate: 8000,
                               channels: .Mono, loopStart: 0, loopLength: 0)
                }
                record("loopPastEnd") {
                    _ = try SE(buffer: Foundation89SoundEffectTests.silence,
                               offset: 0, count: 16, sampleRate: 8000,
                               channels: .Mono, loopStart: 0, loopLength: 99)
                }
                record("negativeSize") {
                    _ = try SE.GetSampleDuration(-1, sampleRate: 8000, channels: .Mono)
                }
                return
            }

            if exerciseStream {
                do {
                    _ = try Microsoft.Xna.Framework.Audio.SoundEffect.FromStream(nil)
                } catch { nullStreamFailure = error }
                do {
                    let empty = Foundation.InputStream(data: Data())
                    _ = try Microsoft.Xna.Framework.Audio.SoundEffect.FromStream(empty)
                } catch { emptyStreamFailure = error }
                return
            }
            if exerciseConversions {
                do {
                    bytesForATenthOfASecond =
                        try Microsoft.Xna.Framework.Audio.SoundEffect
                            .GetSampleSizeInBytes(
                                .milliseconds(100), sampleRate: 8000, channels: .Mono)
                    let back = try Microsoft.Xna.Framework.Audio.SoundEffect
                        .GetSampleDuration(
                            bytesForATenthOfASecond, sampleRate: 8000, channels: .Mono)
                    let parts = back.components
                    roundTrippedTicks = parts.seconds * 10_000_000
                        + parts.attoseconds / 100_000_000_000
                } catch { conversionFailure = error }
                return
            }

            let made: Microsoft.Xna.Framework.Audio.SoundEffect
            do {
                made = try Microsoft.Xna.Framework.Audio.SoundEffect(
                    buffer: Foundation89SoundEffectTests.silence,
                    sampleRate: 8000, channels: .Mono)
            } catch {
                createFailure = error
                return
            }
            effect = made
            disposedDuringLoop = made.IsDisposed
            durationWasPositive = made.Duration > .zero

            if exerciseStateRules {
                let live = try made.CreateInstance()
                instance = live
                try live.Play()
                record("loopAfterPlay") { try live.SetIsLooped(true) }
                record("apply3DAfterPlay") {
                    try live.Apply3D(
                        Microsoft.Xna.Framework.Audio.AudioListener(),
                        emitter: Microsoft.Xna.Framework.Audio.AudioEmitter())
                }
                let fresh = try made.CreateInstance()
                try fresh.Apply3D(
                    Microsoft.Xna.Framework.Audio.AudioListener(),
                    emitter: Microsoft.Xna.Framework.Audio.AudioEmitter())
                record("panOn3D") { try fresh.SetPan(0.5) }
                try fresh.Dispose()
                record("disposed") { try fresh.SetVolume(0.5) }
                try live.Dispose()
                try made.Dispose()
                record("effectDisposed") { try made.SetName("anything") }
                return
            }

            if exerciseRange {
                wholeBufferTicks = AudioProbeGame.ticks(made.Duration)
                let half = try Microsoft.Xna.Framework.Audio.SoundEffect(
                    buffer: Foundation89SoundEffectTests.silence,
                    offset: 0, count: 800, sampleRate: 8000, channels: .Mono,
                    loopStart: 0, loopLength: 0)
                halfBufferTicks = AudioProbeGame.ticks(half.Duration)
                try half.Dispose()
                try made.Dispose()
                return
            }
            if exerciseName {
                nameBefore = made.Name
                try made.SetName("probe effect")
                nameAfter = made.Name
                do { try made.SetName(nil) } catch { nullNameFailure = error }
                try made.Dispose()
                return
            }

            if exerciseInstance {
                let live = try made.CreateInstance()
                instance = live
                try live.SetVolume(0.5)
                try live.SetPitch(-0.25)
                try live.SetPan(1)
                try live.SetIsLooped(true)
                volumeAfter = live.Volume
                pitchAfter = live.Pitch
                panAfter = live.Pan
                loopedAfter = live.IsLooped

                do { try live.SetVolume(1.5) } catch { volumeRefusal = error }
                do { try live.SetPitch(2) } catch { pitchRefusal = error }
                do { try live.SetPan(-4) } catch { panRefusal = error }

                try live.Play()
                try live.Stop()
                stateAfterStop = try live.State
                try live.Dispose()
                do { _ = try live.State } catch { stateAfterDispose = error }
                try made.Dispose()
                return
            }

            if exercise3D {
                let live = try made.CreateInstance()
                instance = live
                let listener = Microsoft.Xna.Framework.Audio.AudioListener()
                let emitter = Microsoft.Xna.Framework.Audio.AudioEmitter()
                do {
                    try live.Apply3D(listener, emitter: emitter)
                    singleListenerAccepted = true
                    try live.Apply3D([listener, listener], emitter: emitter)
                    listenerArrayAccepted = true
                } catch { apply3DFailure = error }
                do { try live.Apply3D(nil, emitter: emitter) }
                catch { nullListenersFailure = error }
                try live.Dispose()
                try made.Dispose()
                return
            }
            if leakOnPurpose {
                instance = try made.CreateInstance()
                return  // neither is disposed: the parent must do it
            }
            try made.Dispose()
        } catch {
            failure = error
        }
    }
}
