// SPDX-License-Identifier: MIT

import Foundation

/// The demand-driven projection of .NET Framework 4.0
/// `System.IO.BinaryReader` inherited by XNA's `ContentReader`.
///
/// The default CLR constructor uses UTF-8 internally. That implementation
/// dependency deliberately does not project `System.Text.Encoding`: no
/// selected constructor or XNA signature exposes it.
open class CNABinaryReader: CNADisposable {
    private var stream: InputStream?
    private var buffer = [UInt8](repeating: 0, count: 16)
    private var pendingBytes: [UInt8] = []
    private var pendingUTF16: [UInt16] = []

    /// `BinaryReader(Stream input)`.
    public init(_ input: InputStream) throws {
        stream = input
        input.open()
        if input.streamStatus == .error {
            stream = nil
            throw CNAArgumentException(message: CNABinaryReader.streamNotReadable)
        }
    }

    /// `BinaryReader.BaseStream`. CLR disposal nulls the backing field, so a
    /// disposed reader truthfully returns `nil` despite pre-nullability CLR
    /// metadata declaring a bare reference type.
    open var BaseStream: InputStream? { stream }

    open func Close() throws { try Dispose(true) }

    /// The sealed `IDisposable.Dispose()` implementation.
    public final func Dispose() throws { try Dispose(true) }

    /// Widened from protected because Swift has no protected access level and
    /// an XNA-derived class must be able to override it.
    open func Dispose(_ disposing: Bool) throws {
        guard let current = stream else { return }
        stream = nil
        pendingBytes.removeAll(keepingCapacity: false)
        pendingUTF16.removeAll(keepingCapacity: false)
        buffer.removeAll(keepingCapacity: false)
        if disposing { current.close() }
    }

    open func FillBuffer(_ numBytes: Int32) throws {
        // The CLR nulls both fields during disposal. Its range guard is
        // conditional on the buffer still existing, then the missing stream
        // wins; consequently even FillBuffer(-1) reports a closed reader once
        // disposed.
        _ = try checkedStream()
        guard numBytes >= 0, numBytes <= 16 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "numBytes", message: CNABinaryReader.fillBufferOutOfRange)
        }
        if buffer.count != 16 { buffer = [UInt8](repeating: 0, count: 16) }
        let bytes = try readExactly(Int(numBytes))
        if !bytes.isEmpty { buffer.replaceSubrange(0..<bytes.count, with: bytes) }
    }

    open func PeekChar() throws -> Int32 {
        let current = try checkedStream()
        if let character = pendingUTF16.first { return Int32(character) }
        guard pendingBytes.isEmpty,
              let position = current.property(forKey: .fileCurrentOffsetKey) as? NSNumber
        else { return -1 }

        let byteSnapshot = pendingBytes
        let characterSnapshot = pendingUTF16
        let value = try Read()
        guard current.setProperty(position, forKey: .fileCurrentOffsetKey) else { return -1 }
        pendingBytes = byteSnapshot
        pendingUTF16 = characterSnapshot
        return value
    }

    /// One UTF-16 code unit, or -1 at EOF.
    open func Read() throws -> Int32 {
        if !pendingUTF16.isEmpty { return Int32(pendingUTF16.removeFirst()) }
        guard let unit = try decodeUTF16Unit() else { return -1 }
        return Int32(unit)
    }

    open func Read(
        _ target: inout [UInt8], _ index: Int32, _ count: Int32
    ) throws -> Int32 {
        let range = try validatedRange(index: index, count: count, length: target.count)
        let bytes = try readAtMostOnce(range.count)
        if !bytes.isEmpty {
            target.replaceSubrange(range.lowerBound..<(range.lowerBound + bytes.count), with: bytes)
        }
        return Int32(bytes.count)
    }

    open func Read(
        _ target: inout [UInt16], _ index: Int32, _ count: Int32
    ) throws -> Int32 {
        let range = try validatedRange(index: index, count: count, length: target.count)
        _ = try checkedStream()
        var written = 0
        while written < range.count {
            let value = try Read()
            if value < 0 { break }
            target[range.lowerBound + written] = UInt16(value)
            written += 1
        }
        return Int32(written)
    }

    /// Widened from protected-internal for the projected XNA subclass.
    public func Read7BitEncodedInt() throws -> Int32 {
        var result: UInt32 = 0
        var shift: UInt32 = 0
        while shift != 35 {
            let byte = try ReadByte()
            result |= UInt32(byte & 0x7f) << shift
            shift += 7
            if byte & 0x80 == 0 { return Int32(bitPattern: result) }
        }
        throw CNAFormatException(message: CNABinaryReader.bad7BitInt32)
    }

    open func ReadBoolean() throws -> Bool { try ReadByte() != 0 }

    open func ReadByte() throws -> UInt8 {
        guard let value = try readOneByte() else { throw endOfStream() }
        return value
    }

    open func ReadSByte() throws -> Int8 { Int8(bitPattern: try ReadByte()) }
    open func ReadInt16() throws -> Int16 { Int16(bitPattern: try ReadUInt16()) }

    open func ReadUInt16() throws -> UInt16 {
        try FillBuffer(2)
        return UInt16(buffer[0]) | (UInt16(buffer[1]) << 8)
    }

    open func ReadInt32() throws -> Int32 { Int32(bitPattern: try ReadUInt32()) }

    open func ReadUInt32() throws -> UInt32 {
        try FillBuffer(4)
        return UInt32(buffer[0])
            | (UInt32(buffer[1]) << 8)
            | (UInt32(buffer[2]) << 16)
            | (UInt32(buffer[3]) << 24)
    }

    open func ReadInt64() throws -> Int64 { Int64(bitPattern: try ReadUInt64()) }

    open func ReadUInt64() throws -> UInt64 {
        try FillBuffer(8)
        var result: UInt64 = 0
        for index in 0..<8 { result |= UInt64(buffer[index]) << UInt64(index * 8) }
        return result
    }

    open func ReadSingle() throws -> Float { Float(bitPattern: try ReadUInt32()) }
    open func ReadDouble() throws -> Double { Double(bitPattern: try ReadUInt64()) }

    open func ReadChar() throws -> UInt16 {
        let value = try Read()
        guard value >= 0 else { throw endOfStream() }
        return UInt16(value)
    }

    open func ReadChars(_ count: Int32) throws -> [UInt16] {
        guard count >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "count", message: CNABinaryReader.needNonNegative)
        }
        var result: [UInt16] = []
        result.reserveCapacity(Int(count))
        while result.count < Int(count) {
            let value = try Read()
            if value < 0 { break }
            result.append(UInt16(value))
        }
        return result
    }

    open func ReadBytes(_ count: Int32) throws -> [UInt8] {
        guard count >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "count", message: CNABinaryReader.needNonNegative)
        }
        return try readAtMost(Int(count))
    }

    open func ReadString() throws -> String {
        let length = try Read7BitEncodedInt()
        guard length >= 0 else {
            throw CNAIOException(message: CNABinaryReader.invalidStringLength(length))
        }
        guard length != 0 else { return "" }
        return String(decoding: try readExactly(Int(length)), as: UTF8.self)
    }

    private func checkedStream() throws -> InputStream {
        guard let stream else {
            throw CNAObjectDisposedException(objectName: nil, message: CNABinaryReader.fileClosed)
        }
        return stream
    }

    private func readOneByte() throws -> UInt8? {
        _ = try checkedStream()
        if !pendingBytes.isEmpty { return pendingBytes.removeFirst() }
        var byte: UInt8 = 0
        let amount = try withUnsafeMutablePointer(to: &byte) { pointer in
            try readFromStream(pointer, maximum: 1)
        }
        return amount == 0 ? nil : byte
    }

    private func readFromStream(
        _ pointer: UnsafeMutablePointer<UInt8>, maximum: Int
    ) throws -> Int {
        let current = try checkedStream()
        let amount = current.read(pointer, maxLength: maximum)
        if amount < 0 { throw CNAIOException(message: current.streamError?.localizedDescription) }
        return amount
    }

    private func readAtMost(_ count: Int) throws -> [UInt8] {
        _ = try checkedStream()
        guard count > 0 else { return [] }
        var result: [UInt8] = []
        result.reserveCapacity(count)
        while !pendingBytes.isEmpty, result.count < count {
            result.append(pendingBytes.removeFirst())
        }
        while result.count < count {
            let requested = min(4096, count - result.count)
            var chunk = [UInt8](repeating: 0, count: requested)
            let amount = try chunk.withUnsafeMutableBufferPointer { storage in
                try readFromStream(storage.baseAddress!, maximum: requested)
            }
            if amount == 0 { break }
            result.append(contentsOf: chunk.prefix(amount))
        }
        return result
    }

    /// `BinaryReader.Read(Byte[], Int32, Int32)` delegates to exactly one
    /// `Stream.Read` call. `ReadBytes`, by contrast, loops until its requested
    /// count or EOF. Keeping these paths separate preserves a stream that
    /// deliberately returns a short chunk while more data is available.
    private func readAtMostOnce(_ count: Int) throws -> [UInt8] {
        _ = try checkedStream()
        guard count > 0 else { return [] }
        var result: [UInt8] = []
        result.reserveCapacity(count)
        while !pendingBytes.isEmpty, result.count < count {
            result.append(pendingBytes.removeFirst())
        }
        let remaining = count - result.count
        guard remaining > 0 else { return result }
        var chunk = [UInt8](repeating: 0, count: remaining)
        let amount = try chunk.withUnsafeMutableBufferPointer { storage in
            try readFromStream(storage.baseAddress!, maximum: remaining)
        }
        result.append(contentsOf: chunk.prefix(amount))
        return result
    }

    private func readExactly(_ count: Int) throws -> [UInt8] {
        let result = try readAtMost(count)
        guard result.count == count else { throw endOfStream() }
        return result
    }

    private func validatedRange(
        index: Int32, count: Int32, length: Int
    ) throws -> Range<Int> {
        guard index >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNABinaryReader.needNonNegative)
        }
        guard count >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "count", message: CNABinaryReader.needNonNegative)
        }
        let lower = Int(index)
        let amount = Int(count)
        guard lower <= length, amount <= length - lower else {
            throw CNAArgumentException(message: CNABinaryReader.invalidOffsetLength)
        }
        return lower..<(lower + amount)
    }

    private func decodeUTF16Unit() throws -> UInt16? {
        guard let first = try readOneByte() else { return nil }
        if first < 0x80 { return UInt16(first) }

        let length: Int
        let initial: UInt32
        switch first {
        case 0xc2...0xdf: length = 2; initial = UInt32(first & 0x1f)
        case 0xe0...0xef: length = 3; initial = UInt32(first & 0x0f)
        case 0xf0...0xf4: length = 4; initial = UInt32(first & 0x07)
        default: return 0xfffd
        }

        var scalar = initial
        var bytes = [first]
        for _ in 1..<length {
            guard let next = try readOneByte() else { return 0xfffd }
            guard next & 0xc0 == 0x80 else {
                pendingBytes.insert(next, at: 0)
                return 0xfffd
            }
            bytes.append(next)
            scalar = (scalar << 6) | UInt32(next & 0x3f)
        }

        let valid = (length != 2 || scalar >= 0x80)
            && (length != 3 || scalar >= 0x800)
            && (length != 4 || scalar >= 0x10000)
            && !(0xd800...0xdfff).contains(scalar)
            && scalar <= 0x10ffff
        guard valid else {
            if bytes.count > 1 { pendingBytes.insert(contentsOf: bytes.dropFirst(), at: 0) }
            return 0xfffd
        }
        if scalar <= 0xffff { return UInt16(scalar) }
        let adjusted = scalar - 0x10000
        let high = UInt16(0xd800 + (adjusted >> 10))
        let low = UInt16(0xdc00 + (adjusted & 0x3ff))
        pendingUTF16.append(low)
        return high
    }

    private func endOfStream() -> CNAEndOfStreamException {
        CNAEndOfStreamException(message: CNABinaryReader.readBeyondEOF)
    }

    internal static let streamNotReadable = "Stream was not readable."
    internal static let fillBufferOutOfRange =
        "The number of bytes requested does not fit into BinaryReader's internal buffer."
    internal static let needNonNegative = "Non-negative number required."
    internal static let invalidOffsetLength =
        "Offset and length were out of bounds for the array or count is greater "
        + "than the number of elements from index to the end of the source collection."
    internal static let bad7BitInt32 =
        "Too many bytes in what should have been a 7 bit encoded Int32."
    internal static let readBeyondEOF = "Unable to read beyond the end of the stream."
    internal static let fileClosed = "Cannot access a closed file."
    internal static let invalidStringLengthFormat =
        "BinaryReader encountered an invalid string length of {0} characters."

    internal static func invalidStringLength(_ length: Int32) -> String {
        invalidStringLengthFormat.replacingOccurrences(of: "{0}", with: String(length))
    }
}
