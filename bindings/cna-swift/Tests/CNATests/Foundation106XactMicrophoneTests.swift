// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias XactAudio = Microsoft.Xna.Framework.Audio

private enum XactFixture {
    static func appendU8(_ value: UInt8, to bytes: inout [UInt8]) {
        bytes.append(value)
    }

    static func appendU16(_ value: UInt16, to bytes: inout [UInt8]) {
        bytes.append(UInt8(truncatingIfNeeded: value))
        bytes.append(UInt8(truncatingIfNeeded: value >> 8))
    }

    static func appendU32(_ value: UInt32, to bytes: inout [UInt8]) {
        for shift in stride(from: 0, through: 24, by: 8) {
            bytes.append(UInt8(truncatingIfNeeded: value >> UInt32(shift)))
        }
    }

    static func appendI32(_ value: Int32, to bytes: inout [UInt8]) {
        appendU32(UInt32(bitPattern: value), to: &bytes)
    }

    static func appendFloat(_ value: Float, to bytes: inout [UInt8]) {
        appendU32(value.bitPattern, to: &bytes)
    }

    static func appendPadded(
        _ value: String, size: Int, to bytes: inout [UInt8]
    ) {
        let encoded = Array(value.utf8)
        precondition(encoded.count <= size)
        bytes.append(contentsOf: encoded)
        bytes.append(contentsOf: repeatElement(0, count: size - encoded.count))
    }

    static func appendCString(_ value: String, to bytes: inout [UInt8]) {
        bytes.append(contentsOf: value.utf8)
        bytes.append(0)
    }

    /// Project-authored minimal XGS with one category and one public mutable
    /// global variable. Every multi-byte value is written explicitly LE.
    static func xgs() -> [UInt8] {
        let headerSize: UInt32 = 65
        let categoryOffset = headerSize
        let variableOffset = categoryOffset + 10
        let categoryNameOffset = variableOffset + 13
        let variableNameOffset = categoryNameOffset + 8
        var bytes = Array("XGSF".utf8)
        appendU16(46, to: &bytes)
        appendU16(0, to: &bytes)
        appendU16(0, to: &bytes)
        bytes.append(contentsOf: repeatElement(0, count: 8))
        appendU8(3, to: &bytes)
        appendU16(1, to: &bytes)
        appendU16(1, to: &bytes)
        for _ in 0..<5 { appendU16(0, to: &bytes) }
        appendU32(categoryOffset, to: &bytes)
        appendU32(variableOffset, to: &bytes)
        appendU32(0, to: &bytes)
        appendU32(0, to: &bytes)
        appendU32(0, to: &bytes)
        appendU32(0, to: &bytes)
        appendU32(categoryNameOffset, to: &bytes)
        appendU32(variableNameOffset, to: &bytes)
        appendU8(0xff, to: &bytes)
        appendU16(0, to: &bytes)
        appendU16(0, to: &bytes)
        appendU8(0, to: &bytes)
        appendU16(0xffff, to: &bytes)
        appendU8(0xff, to: &bytes)
        appendU8(0, to: &bytes)
        appendU8(0x01, to: &bytes)
        appendFloat(0.5, to: &bytes)
        appendFloat(0, to: &bytes)
        appendFloat(1, to: &bytes)
        appendCString("Default", to: &bytes)
        appendCString("Volume", to: &bytes)
        return bytes
    }

    /// Compact mono PCM wave bank with one 200-byte silent wave.
    static func xwb() -> [UInt8] {
        let headerSize: UInt32 = 48
        let bankDataSize: UInt32 = 96
        let metadataSize: UInt32 = 4
        let waveLength: UInt32 = 200
        let waveOffset = headerSize + bankDataSize + metadataSize
        let offsets = [headerSize, headerSize + bankDataSize,
                       waveOffset, waveOffset, waveOffset]
        let lengths = [bankDataSize, metadataSize, 0, 0, waveLength]
        var bytes = Array("WBND".utf8)
        appendU32(1, to: &bytes)
        for index in offsets.indices {
            appendU32(offsets[index], to: &bytes)
            appendU32(lengths[index], to: &bytes)
        }
        appendU32(0x0002_0000, to: &bytes)
        appendU32(1, to: &bytes)
        appendPadded("SwiftWaveBank", size: 64, to: &bytes)
        appendU32(metadataSize, to: &bytes)
        appendU32(0, to: &bytes)
        appendU32(4, to: &bytes)
        let compactFormat = UInt32(1 << 2) | UInt32(44_100 << 5)
            | UInt32(2 << 23) | UInt32(1 << 31)
        appendU32(compactFormat, to: &bytes)
        bytes.append(contentsOf: repeatElement(0, count: 8))
        appendU32(0, to: &bytes)
        bytes.append(contentsOf: repeatElement(0, count: Int(waveLength)))
        return bytes
    }

    /// One simple cue named SwiftCue, pointing at wave zero in SwiftWaveBank.
    static func xsb() -> [UInt8] {
        let headerSize: UInt32 = 74
        let bankNameSize: UInt32 = 64
        let waveBankNameOffset = headerSize + bankNameSize
        let soundOffset = waveBankNameOffset + 64
        let cueSimpleOffset = soundOffset + 12
        let cueNameIndexOffset = cueSimpleOffset + 5
        let cueNameOffset = cueNameIndexOffset + 6
        var bytes = Array("SDBK".utf8)
        appendU16(46, to: &bytes)
        appendU16(0, to: &bytes)
        appendU16(0, to: &bytes)
        bytes.append(contentsOf: repeatElement(0, count: 8))
        appendU8(0, to: &bytes)
        appendU16(1, to: &bytes)
        appendU16(0, to: &bytes)
        appendU16(0, to: &bytes)
        appendU16(0, to: &bytes)
        appendU8(1, to: &bytes)
        appendU16(1, to: &bytes)
        appendU16(0, to: &bytes)
        appendU16(0, to: &bytes)
        appendI32(Int32(cueSimpleOffset), to: &bytes)
        appendI32(-1, to: &bytes)
        appendI32(-1, to: &bytes)
        appendI32(0, to: &bytes)
        appendI32(-1, to: &bytes)
        appendI32(0, to: &bytes)
        appendI32(Int32(waveBankNameOffset), to: &bytes)
        appendI32(0, to: &bytes)
        appendI32(Int32(cueNameIndexOffset), to: &bytes)
        appendI32(Int32(soundOffset), to: &bytes)
        appendPadded("SwiftSoundBank", size: Int(bankNameSize), to: &bytes)
        appendPadded("SwiftWaveBank", size: 64, to: &bytes)
        appendU8(0, to: &bytes)
        appendU16(0, to: &bytes)
        appendU8(0xff, to: &bytes)
        appendU16(0, to: &bytes)
        appendU8(0, to: &bytes)
        appendU16(0, to: &bytes)
        appendU16(0, to: &bytes)
        appendU8(0, to: &bytes)
        appendU8(0, to: &bytes)
        appendU32(soundOffset, to: &bytes)
        appendU32(cueNameOffset, to: &bytes)
        appendU16(0, to: &bytes)
        appendCString("SwiftCue", to: &bytes)
        return bytes
    }
}

private final class XactProbeGame: Microsoft.Xna.Framework.Game {
    let xgsPath: String
    let xwbPath: String
    let xsbPath: String
    var failure: Error?
    var engineDisposed = false
    var bankDisposed = false
    var waveDisposed = false
    var cueDisposed = false
    var disposingEvents: [String] = []
    var microphoneCount: Int32 = -1
    var microphoneCollectionStable = false
    var defaultWasNilWhenEmpty = false

    init(xgsPath: String, xwbPath: String, xsbPath: String) throws {
        self.xgsPath = xgsPath
        self.xwbPath = xwbPath
        self.xsbPath = xsbPath
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        do {
            XCTAssertEqual(XactAudio.AudioEngine.ContentVersion, 39)
            XCTAssertThrowsError(try XactAudio.AudioEngine(settingsFile: nil)) {
                XCTAssertTrue($0 is CNAArgumentNullException)
            }
            XCTAssertThrowsError(try XactAudio.SoundBank(
                audioEngine: nil, filename: xsbPath)) {
                XCTAssertTrue($0 is CNAArgumentNullException)
            }
            XCTAssertThrowsError(try XactAudio.WaveBank(
                audioEngine: nil, nonStreamingWaveBankFilename: xwbPath)) {
                XCTAssertTrue($0 is CNAArgumentNullException)
            }

            let engine = try XactAudio.AudioEngine(settingsFile: xgsPath)
            _ = engine.Disposing.Add { [weak self] _, _ in
                self?.disposingEvents.append("engine")
            }
            let renderers = try engine.RendererDetails
            XCTAssertNotNil(renderers)
            XCTAssertGreaterThan(renderers?.Count ?? 0, 0)

            let firstCategory = try engine.GetCategory("Default")
            let secondCategory = try engine.GetCategory("Default")
            XCTAssertEqual(firstCategory.Name, "Default")
            XCTAssertEqual(firstCategory.ToString(), "Default")
            XCTAssertEqual(firstCategory, secondCategory)
            XCTAssertEqual(firstCategory.GetHashCode(), secondCategory.GetHashCode())
            XCTAssertThrowsError(try firstCategory.SetVolume(-0.01)) {
                XCTAssertTrue($0 is CNAArgumentException)
            }
            try firstCategory.SetVolume(0.75)
            XCTAssertEqual(try engine.GetGlobalVariable("Volume"), 0.5)
            try engine.SetGlobalVariable("Volume", value: 0.75)
            XCTAssertEqual(try engine.GetGlobalVariable("Volume"), 0.75)

            let wave = try XactAudio.WaveBank(
                audioEngine: engine, nonStreamingWaveBankFilename: xwbPath)
            _ = wave.Disposing.Add { [weak self] _, _ in
                self?.disposingEvents.append("wave")
            }
            XCTAssertTrue(wave.IsPrepared)

            let bank = try XactAudio.SoundBank(
                audioEngine: engine, filename: xsbPath)
            _ = bank.Disposing.Add { [weak self] _, _ in
                self?.disposingEvents.append("bank")
            }
            let cue = try bank.GetCue("SwiftCue")
            _ = cue.Disposing.Add { [weak self] _, _ in
                self?.disposingEvents.append("cue")
            }
            XCTAssertEqual(cue.Name, "SwiftCue")
            XCTAssertTrue(cue.IsCreated || cue.IsPrepared)
            try cue.Apply3D(XactAudio.AudioListener(),
                            emitter: XactAudio.AudioEmitter())
            try cue.Play()
            XCTAssertTrue(cue.IsPlaying || cue.IsPrepared)
            try cue.Pause()
            try cue.Resume()
            try cue.Stop(.Immediate)
            try bank.PlayCue("SwiftCue")
            try engine.Update()

            let microphones = try XactAudio.Microphone.All
            microphoneCount = microphones.Count
            microphoneCollectionStable = try XactAudio.Microphone.All === microphones
            let defaultMicrophone = try XactAudio.Microphone.Default
            defaultWasNilWhenEmpty = microphones.Count != 0 || defaultMicrophone == nil
            if let microphone = defaultMicrophone {
                XCTAssertGreaterThan(microphone.SampleRate, 0)
                XCTAssertEqual(try microphone.GetSampleSizeInBytes(.zero), 0)
                XCTAssertEqual(try microphone.GetSampleDuration(0), .zero)
                XCTAssertThrowsError(
                    try microphone.SetBufferDuration(.milliseconds(99))) {
                    XCTAssertTrue($0 is CNAArgumentOutOfRangeException)
                }
                var invalid = [UInt8](repeating: 0, count: 3)
                XCTAssertThrowsError(try microphone.GetData(&invalid)) {
                    XCTAssertTrue($0 is CNAArgumentException)
                }
            }

            try engine.Dispose()
            try engine.Dispose()
            cueDisposed = cue.IsDisposed
            bankDisposed = bank.IsDisposed
            waveDisposed = wave.IsDisposed
            engineDisposed = engine.IsDisposed
        } catch {
            failure = error
        }
    }
}

final class Foundation106XactMicrophoneTests: XCTestCase {
    func testProjectAuthoredXactGraphAndMicrophoneEnumeration() throws {
        guard ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] != nil else {
            XCTFail("selected XACT/Microphone tests require CNA_NATIVE_LIBRARY")
            return
        }
        let root = FileManager.default.temporaryDirectory.appendingPathComponent(
            "cna-swift-foundation106-\(UUID().uuidString)", isDirectory: true)
        try FileManager.default.createDirectory(
            at: root, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: root) }
        let xgs = root.appendingPathComponent("fixture.xgs")
        let xwb = root.appendingPathComponent("fixture.xwb")
        let xsb = root.appendingPathComponent("fixture.xsb")
        try Data(XactFixture.xgs()).write(to: xgs, options: .atomic)
        try Data(XactFixture.xwb()).write(to: xwb, options: .atomic)
        try Data(XactFixture.xsb()).write(to: xsb, options: .atomic)

        let game = try XactProbeGame(
            xgsPath: xgs.path, xwbPath: xwb.path, xsbPath: xsb.path)
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.cueDisposed)
        XCTAssertTrue(game.bankDisposed)
        XCTAssertTrue(game.waveDisposed)
        XCTAssertTrue(game.engineDisposed)
        XCTAssertEqual(game.disposingEvents, ["engine", "bank", "cue", "wave"])
        XCTAssertGreaterThanOrEqual(game.microphoneCount, 0)
        XCTAssertTrue(game.microphoneCollectionStable)
        XCTAssertTrue(game.defaultWasNilWhenEmpty)
    }
}
