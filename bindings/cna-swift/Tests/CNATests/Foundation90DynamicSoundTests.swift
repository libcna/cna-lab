// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// `Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance`.
///
/// Reachable where the XACT family is not: it is built from a sample rate and
/// a channel count, so nothing has to be loaded from disk.
final class Foundation90DynamicSoundTests: XCTestCase {

    func testItIsBuiltFromParametersAndSubmitsBuffers() throws {
        let game = try DynamicProbeGame()
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        if let refusal = game.createFailure {
            XCTAssertTrue("\(refusal)".contains("cna_dynamic_sound_effect_instance_create"))
            return
        }
        XCTAssertTrue(game.submitted, "a whole number of PCM16 frames is accepted")
        XCTAssertEqual(game.pendingAfterSubmit, 1, "the buffer is queued, not consumed")
        XCTAssertEqual(game.bytesForATenthOfASecond, 1600,
                       "the instance's own rate and channels drive the conversion")
        XCTAssertEqual(game.roundTrippedMilliseconds, 100)
    }

    /// The same block-alignment rules the `SoundEffect` constructor enforces,
    /// because CNA takes the count it is given here too.
    func testSubmitBufferRefusesWhatCnaWouldAccept() throws {
        let game = try DynamicProbeGame(exerciseRefusals: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        XCTAssertTrue(game.nullBufferFailure is CNAArgumentNullException)
        XCTAssertEqual(game.messages["oddLength"],
                       "Ensure that the buffer length is non-zero and meets the "
                       + "block alignment requirements for the audio format.")
        XCTAssertEqual(game.messages["countPastEnd"],
                       "Ensure that count is valid and meets the block alignment "
                       + "requirements for the audio format. Offset and count "
                       + "must define a valid region within the buffer boundaries.")
        XCTAssertEqual(game.messages["negativeSize"], "Buffer size cannot be negative.")
    }

    /// **Measured, and reproduced because CNA does not enforce it:**
    /// submitting 101 buffers to this runtime leaves 101 pending, where XNA
    /// refuses past 64. A queue that grows without limit fails later and
    /// somewhere else, which is worse than being told the limit.
    func testSubmitBufferRefusesPastTheInstancePacketLimit() throws {
        let game = try DynamicProbeGame(fillTheQueue: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        XCTAssertEqual(game.acceptedBeforeRefusal, 64,
                       "sixty-four are accepted and the sixty-fifth is not")
        XCTAssertEqual(game.messages["overLimit"],
                       "Please ensure that there are less than 64 buffers "
                       + "pending on this instance.")
    }

    /// The native subscription is released with the instance. A callback that
    /// outlived its box would address freed memory, which is the one ordering
    /// mistake this type can make.
    func testDisposalReleasesTheBufferNeededSubscription() throws {
        let game = try DynamicProbeGame(keepInstance: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        let instance = try XCTUnwrap(game.instance)
        XCTAssertEqual(instance.bufferNeededRegistration, 0,
                       "the registration was released with the instance")
        XCTAssertTrue(instance.IsDisposed)
    }

    /// The redeclared property keeps its own XNA behavior even though Swift
    /// needs a named getter to express its different throwing effect.
    func testRedeclaredIsLoopedAccessorsPreserveDynamicBehavior() throws {
        let game = try DynamicProbeGame(readIsLooped: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        guard game.createFailure == nil else { throw XCTSkip("no audio backend") }
        XCTAssertEqual(game.inheritedIsLooped, false,
                       "the statically inherited base getter remains callable")
        XCTAssertEqual(game.dynamicIsLooped, false)
        XCTAssertEqual(game.messages["dynamicTrue"],
                       "IsLooped property is not supported for "
                       + "DynamicSoundEffectInstance.")
        XCTAssertTrue(game.dynamicReadAfterDispose is CNAObjectDisposedException)
    }
}

private final class DynamicProbeGame: Microsoft.Xna.Framework.Game {
    let exerciseRefusals: Bool
    let keepInstance: Bool
    let readIsLooped: Bool
    let fillTheQueue: Bool

    var failure: Error?
    var createFailure: Error?
    var instance: Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance?
    var submitted = false
    var pendingAfterSubmit: Int32 = -1
    var acceptedBeforeRefusal = -1
    var bytesForATenthOfASecond: Int32 = -1
    var roundTrippedMilliseconds = -1
    var nullBufferFailure: Error?
    var messages: [String: String] = [:]
    var inheritedIsLooped: Bool?
    var dynamicIsLooped: Bool?
    var dynamicReadAfterDispose: Error?

    init(exerciseRefusals: Bool = false, keepInstance: Bool = false,
         readIsLooped: Bool = false, fillTheQueue: Bool = false) throws {
        self.exerciseRefusals = exerciseRefusals
        self.keepInstance = keepInstance
        self.readIsLooped = readIsLooped
        self.fillTheQueue = fillTheQueue
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        do {
            let live: Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance
            do {
                live = try Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance(
                    sampleRate: 8000, channels: .Mono)
            } catch {
                createFailure = error
                return
            }
            instance = live

            func record(_ key: String, _ body: () throws -> Void) {
                do { try body() }
                catch let error as CNAException { messages[key] = error.Message ?? "" }
                catch { messages[key] = "\(error)" }
            }

            if readIsLooped {
                inheritedIsLooped = live.IsLooped
                dynamicIsLooped = try live.GetIsLooped()
                try live.SetIsLooped(false)
                record("dynamicTrue") { try live.SetIsLooped(true) }
                try live.Dispose()
                do { _ = try live.GetIsLooped() }
                catch { dynamicReadAfterDispose = error }
                return
            }

            if fillTheQueue {
                let frames = [UInt8](repeating: 0, count: 64)
                var accepted = 0
                for _ in 0..<200 {
                    do { try live.SubmitBuffer(frames); accepted += 1 }
                    catch let error as CNAException {
                        messages["overLimit"] = error.Message ?? ""
                        break
                    }
                }
                acceptedBeforeRefusal = accepted
                try live.Dispose()
                return
            }
            if exerciseRefusals {
                do { try live.SubmitBuffer(nil) } catch { nullBufferFailure = error }
                record("oddLength") { try live.SubmitBuffer([0, 0, 0]) }
                record("countPastEnd") {
                    try live.SubmitBuffer([UInt8](repeating: 0, count: 16),
                                          offset: 14, count: 8)
                }
                record("negativeSize") { _ = try live.GetSampleDuration(-1) }
                try live.Dispose()
                return
            }

            let frames = [UInt8](repeating: 0, count: 1600)
            try live.SubmitBuffer(frames)
            submitted = true
            pendingAfterSubmit = try live.PendingBufferCount
            bytesForATenthOfASecond = try live.GetSampleSizeInBytes(.milliseconds(100))
            let back = try live.GetSampleDuration(bytesForATenthOfASecond)
            let parts = back.components
            roundTrippedMilliseconds = Int(parts.seconds) * 1000
                + Int(parts.attoseconds / 1_000_000_000_000_000)
            if !keepInstance { try live.Dispose() }
        } catch {
            failure = error
        }
    }
}
