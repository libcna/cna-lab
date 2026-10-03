// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

final class Foundation104BinaryReaderTests: XCTestCase {
    private func reader(_ bytes: [UInt8]) throws -> CNABinaryReader {
        try CNABinaryReader(InputStream(data: Data(bytes)))
    }

    func testLittleEndianPrimitiveReadsAndSignedness() throws {
        let input = try reader([
            2,
            0xff,
            0x80,
            0x34, 0x12,
            0xdc, 0xfe,
            0xfe, 0xff, 0xff, 0xff,
            0xef, 0xcd, 0xab, 0x89,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
            0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01,
            0x00, 0x00, 0xc0, 0x3f,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xc0,
        ])

        XCTAssertTrue(try input.ReadBoolean())
        XCTAssertEqual(try input.ReadByte(), 0xff)
        XCTAssertEqual(try input.ReadSByte(), -128)
        XCTAssertEqual(try input.ReadInt16(), 0x1234)
        XCTAssertEqual(try input.ReadUInt16(), 0xfedc)
        XCTAssertEqual(try input.ReadInt32(), -2)
        XCTAssertEqual(try input.ReadUInt32(), 0x89ab_cdef)
        XCTAssertEqual(try input.ReadInt64(), Int64.min)
        XCTAssertEqual(try input.ReadUInt64(), 0x0123_4567_89ab_cdef)
        XCTAssertEqual(try input.ReadSingle(), 1.5)
        XCTAssertEqual(try input.ReadDouble(), -2.25)
    }

    func testReadStringUsesSevenBitByteLengthAndReplacementUTF8() throws {
        let long = [UInt8](repeating: 0x61, count: 130)
        XCTAssertEqual(try reader([0x82, 0x01] + long).ReadString(), String(repeating: "a", count: 130))
        XCTAssertEqual(try reader([2, 0xff, 0x61]).ReadString(), "\u{fffd}a")
        XCTAssertEqual(try reader([0]).ReadString(), "")
    }

    func testCharactersAreUTF16CodeUnits() throws {
        let input = try reader(Array("Aé😀".utf8))
        XCTAssertEqual(try input.ReadChar(), 0x0041)
        XCTAssertEqual(try input.Read(), 0x00e9)
        XCTAssertEqual(try input.ReadChars(2), [0xd83d, 0xde00])
        XCTAssertEqual(try input.Read(), -1)
    }

    func testByteAndCharacterArrayReadsCanEndShort() throws {
        let bytes = try reader([10, 11, 12])
        var destination = [UInt8](repeating: 99, count: 6)
        XCTAssertEqual(try bytes.Read(&destination, 2, 4), 3)
        XCTAssertEqual(destination, [99, 99, 10, 11, 12, 99])

        let characters = try reader(Array("xy".utf8))
        var characterDestination = [UInt16](repeating: 0, count: 4)
        XCTAssertEqual(try characters.Read(&characterDestination, 1, 3), 2)
        XCTAssertEqual(characterDestination, [0, 0x78, 0x79, 0])
        XCTAssertEqual(try reader([1, 2]).ReadBytes(5), [1, 2])

        let short = try CNABinaryReader(ShortReadInputStream([1, 2, 3, 4]))
        var shortDestination = [UInt8](repeating: 0, count: 4)
        XCTAssertEqual(try short.Read(&shortDestination, 0, 4), 2)
        XCTAssertEqual(shortDestination, [1, 2, 0, 0])
        XCTAssertEqual(try short.ReadBytes(2), [3, 4])
    }

    func testPeekCharPreservesASeekableFilePosition() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent(
            "cna-swift-foundation-104-peek-\(UUID().uuidString)", isDirectory: true)
        try FileManager.default.createDirectory(
            at: root, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: root) }
        let file = root.appendingPathComponent("utf8.bin")
        try Data("éZ".utf8).write(to: file)
        let input = try XCTUnwrap(InputStream(url: file))
        let binary = try CNABinaryReader(input)

        XCTAssertEqual(try binary.PeekChar(), 0x00e9)
        XCTAssertEqual(try binary.ReadChar(), 0x00e9)
        XCTAssertEqual(try binary.ReadChar(), 0x005a)
    }

    func testPrimitiveShortReadAndMalformedSevenBitIntegerThrowExactClasses() throws {
        assertProjected(
            CNAEndOfStreamException.self,
            message: "Unable to read beyond the end of the stream.",
            hResult: Int32(bitPattern: 0x8007_0026)
        ) {
            _ = try reader([1, 2, 3]).ReadInt32()
        }
        assertProjected(
            CNAFormatException.self,
            message: "Too many bytes in what should have been a 7 bit encoded Int32.",
            hResult: Int32(bitPattern: 0x8013_1537)
        ) {
            _ = try reader([0x80, 0x80, 0x80, 0x80, 0x80]).Read7BitEncodedInt()
        }
    }

    func testBaseStreamIdentityCloseAndReadAfterClose() throws {
        let stream = InputStream(data: Data([7]))
        let input = try CNABinaryReader(stream)
        XCTAssertTrue(input.BaseStream === stream)
        try input.Close()
        XCTAssertNil(input.BaseStream)
        XCTAssertEqual(stream.streamStatus, .closed)
        try input.Close()

        assertProjected(
            CNAObjectDisposedException.self,
            message: "Cannot access a closed file.",
            hResult: Int32(bitPattern: 0x8013_1622)
        ) {
            _ = try input.ReadByte()
        }

        var emptyBytes: [UInt8] = []
        XCTAssertThrowsError(try input.Read(&emptyBytes, 0, 0)) {
            XCTAssertTrue($0 is CNAObjectDisposedException)
        }
        var emptyCharacters: [UInt16] = []
        XCTAssertThrowsError(try input.Read(&emptyCharacters, 0, 0)) {
            XCTAssertTrue($0 is CNAObjectDisposedException)
        }
        XCTAssertThrowsError(try input.FillBuffer(-1)) {
            XCTAssertTrue($0 is CNAObjectDisposedException)
        }
    }

    func testRangeValidationAndNegativeCountsAreBclFailures() throws {
        var bytes = [UInt8](repeating: 0, count: 2)
        XCTAssertThrowsError(try reader([]).Read(&bytes, -1, 1)) { error in
            XCTAssertTrue(error is CNAArgumentOutOfRangeException)
        }
        XCTAssertThrowsError(try reader([]).Read(&bytes, 1, 2)) { error in
            XCTAssertTrue(error is CNAArgumentException)
        }
        XCTAssertThrowsError(try reader([]).ReadBytes(-1)) { error in
            XCTAssertTrue(error is CNAArgumentOutOfRangeException)
        }
    }

    func testDefaultExceptionConstructionComesFromPinnedBclResources() {
        XCTAssertEqual(
            CNAEndOfStreamException().Message,
            "Attempted to read past the end of the stream.")
        XCTAssertEqual(
            CNAFormatException().Message,
            "One of the identified items was in an invalid format.")
    }
}

private final class ShortReadInputStream: InputStream {
    private let values: [UInt8]
    private var offset = 0
    private var status: Stream.Status = .notOpen

    init(_ values: [UInt8]) {
        self.values = values
        super.init(data: Data())
    }

    override func open() { status = .open }
    override func close() { status = .closed }
    override var streamStatus: Stream.Status { status }
    override var hasBytesAvailable: Bool { offset < values.count }

    override func read(
        _ buffer: UnsafeMutablePointer<UInt8>, maxLength len: Int
    ) -> Int {
        guard status == .open else { return -1 }
        guard offset < values.count else {
            status = .atEnd
            return 0
        }
        let amount = min(2, len, values.count - offset)
        for index in 0..<amount { buffer[index] = values[offset + index] }
        offset += amount
        return amount
    }
}
