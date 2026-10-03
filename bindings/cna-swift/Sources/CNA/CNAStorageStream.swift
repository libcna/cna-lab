// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

/// `System.IO.SeekOrigin` for the demand-driven storage-stream projection.
public enum CNASeekOrigin: Int32 {
    case Begin = 0
    case Current = 1
    case End = 2
}

/// The duplex `System.IO.Stream` returned by XNA storage containers.
///
/// Most XNA stream positions only read, and continue to use the established
/// `System.IO.Stream -> Foundation.InputStream` mapping. Storage file methods
/// are the exceptional return position: their IL returns streams that can be
/// read, written, sought, flushed and resized. This concrete `InputStream`
/// subclass retains that read identity while making the writable contract
/// statically available instead of returning a read-only facade.
public final class CNAStorageStream: InputStream {
    private let functions: NativeFunctions
    private let container: Microsoft.Xna.Framework.Storage.StorageContainer
    private let lock = NSLock()
    private var handle: UInt64
    private var status: Stream.Status = .open
    private var failure: Error?

    internal init(
        handle: UInt64,
        functions: NativeFunctions,
        container: Microsoft.Xna.Framework.Storage.StorageContainer
    ) {
        self.handle = handle
        self.functions = functions
        self.container = container
        super.init(data: Data())
    }

    deinit {
        lock.lock()
        let live = handle
        if live != 0 {
            _ = functions.storageStreamClose(live)
            handle = 0
        }
        status = .closed
        lock.unlock()
    }

    public override func open() {
        // The native create/open call has already opened the stream. Like a
        // CLR FileStream, an explicit open after Close cannot resurrect it.
    }

    public override func close() {
        try? Close()
    }

    public override var streamStatus: Stream.Status {
        lock.withLock { status }
    }

    public override var streamError: Error? {
        lock.withLock { failure }
    }

    public override var hasBytesAvailable: Bool {
        lock.withLock {
            guard handle != 0 else { return false }
            var canRead: UInt8 = 0
            guard functions.storageStreamGetCanRead(handle, &canRead) == 0,
                  canRead != 0 else { return false }
            var position: Int64 = 0
            var length: Int64 = 0
            guard functions.storageStreamGetPosition(handle, &position) == 0,
                  functions.storageStreamGetLength(handle, &length) == 0 else {
                return false
            }
            return position < length
        }
    }

    public override func read(
        _ buffer: UnsafeMutablePointer<UInt8>, maxLength len: Int
    ) -> Int {
        guard len >= 0 else { return recordFailure(
            operation: "CNAStorageStream.read", result: 1) }
        return lock.withLock {
            guard handle != 0 else {
                status = .closed
                return -1
            }
            var copied: UInt64 = 0
            let result = functions.storageStreamRead(
                handle, len == 0 ? nil : buffer, UInt64(len), &copied)
            guard result == 0, copied <= UInt64(Int.max) else {
                failure = CNAError.nativeFailure(
                    operation: "cna_storage_stream_read", result: result,
                    message: "The native storage stream read failed.")
                status = .error
                return -1
            }
            if copied == 0 { status = .atEnd }
            return Int(copied)
        }
    }

    /// `Stream.CanRead`.
    public var CanRead: Bool { get throws { try boolean(
        functions.storageStreamGetCanRead,
        operation: "cna_storage_stream_get_can_read") } }

    /// `Stream.CanWrite`.
    public var CanWrite: Bool { get throws { try boolean(
        functions.storageStreamGetCanWrite,
        operation: "cna_storage_stream_get_can_write") } }

    /// `Stream.CanSeek`.
    public var CanSeek: Bool { get throws { try boolean(
        functions.storageStreamGetCanSeek,
        operation: "cna_storage_stream_get_can_seek") } }

    /// `Stream.Length`.
    public var Length: Int64 { get throws { try integer(
        functions.storageStreamGetLength,
        operation: "cna_storage_stream_get_length") } }

    /// `Stream.Position` getter. Its fallible setter is projected separately
    /// as `SetPosition`, following the package's accessor rule.
    public var Position: Int64 { get throws { try integer(
        functions.storageStreamGetPosition,
        operation: "cna_storage_stream_get_position") } }

    public func SetPosition(_ value: Int64) throws {
        _ = try Seek(value, origin: .Begin)
    }

    /// `Stream.Read(Byte[], Int32, Int32)`.
    public func Read(
        _ buffer: inout [UInt8], offset: Int32, count: Int32
    ) throws -> Int32 {
        let start = Int(offset)
        let amount = Int(count)
        guard offset >= 0 else {
            throw CNAArgumentOutOfRangeException(paramName: "offset")
        }
        guard count >= 0 else {
            throw CNAArgumentOutOfRangeException(paramName: "count")
        }
        guard start <= buffer.count, amount <= buffer.count - start else {
            throw CNAArgumentException(
                message: "Offset and length were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.")
        }
        let live = try validated()
        var copied: UInt64 = 0
        let result = buffer.withUnsafeMutableBufferPointer { bytes in
            functions.storageStreamRead(
                live,
                amount == 0 ? nil : bytes.baseAddress?.advanced(by: start),
                UInt64(amount), &copied)
        }
        try functions.check(result, operation: "cna_storage_stream_read")
        guard copied <= UInt64(Int32.max) else {
            throw CNAIOException(message: "The stream read count is not representable.")
        }
        return Int32(copied)
    }

    /// `Stream.Write(Byte[], Int32, Int32)`.
    public func Write(
        _ buffer: [UInt8], offset: Int32, count: Int32
    ) throws {
        let start = Int(offset)
        let amount = Int(count)
        guard offset >= 0 else {
            throw CNAArgumentOutOfRangeException(paramName: "offset")
        }
        guard count >= 0 else {
            throw CNAArgumentOutOfRangeException(paramName: "count")
        }
        guard start <= buffer.count, amount <= buffer.count - start else {
            throw CNAArgumentException(
                message: "Offset and length were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.")
        }
        let live = try validated()
        let result = buffer.withUnsafeBufferPointer { bytes in
            functions.storageStreamWrite(
                live,
                amount == 0 ? nil : bytes.baseAddress?.advanced(by: start),
                UInt64(amount))
        }
        try functions.check(result, operation: "cna_storage_stream_write")
    }

    /// `Stream.Seek(Int64, SeekOrigin)`.
    @discardableResult
    public func Seek(_ offset: Int64, origin: CNASeekOrigin) throws -> Int64 {
        let live = try validated()
        var position: Int64 = 0
        try functions.check(
            functions.storageStreamSeek(
                live, offset, UInt32(bitPattern: origin.rawValue), &position),
            operation: "cna_storage_stream_seek")
        return position
    }

    /// `Stream.SetLength(Int64)`.
    public func SetLength(_ value: Int64) throws {
        let live = try validated()
        try functions.check(
            functions.storageStreamSetLength(live, value),
            operation: "cna_storage_stream_set_length")
    }

    /// `Stream.Flush()`.
    public func Flush() throws {
        let live = try validated()
        try functions.check(
            functions.storageStreamFlush(live),
            operation: "cna_storage_stream_flush")
    }

    /// `Stream.Close()`. Repeated close is silent at the managed boundary.
    public func Close() throws {
        lock.lock()
        defer { lock.unlock() }
        guard handle != 0 else { return }
        try functions.check(
            functions.storageStreamClose(handle),
            operation: "cna_storage_stream_close")
        handle = 0
        status = .closed
    }

    private func validated() throws -> UInt64 {
        lock.lock()
        defer { lock.unlock() }
        guard handle != 0 else {
            throw CNAObjectDisposedException(objectName: "CNAStorageStream")
        }
        return handle
    }

    private func boolean(
        _ route: (UInt64, UnsafeMutablePointer<UInt8>?) -> UInt32,
        operation: String
    ) throws -> Bool {
        let live = try validated()
        var value: UInt8 = 0
        try functions.check(route(live, &value), operation: operation)
        return value != 0
    }

    private func integer(
        _ route: (UInt64, UnsafeMutablePointer<Int64>?) -> UInt32,
        operation: String
    ) throws -> Int64 {
        let live = try validated()
        var value: Int64 = 0
        try functions.check(route(live, &value), operation: operation)
        return value
    }

    private func recordFailure(operation: String, result: UInt32) -> Int {
        lock.withLock {
            failure = CNAError.nativeFailure(
                operation: operation, result: result,
                message: "Invalid stream read length.")
            status = .error
        }
        return -1
    }
}

private extension NSLock {
    func withLock<T>(_ body: () throws -> T) rethrows -> T {
        lock()
        defer { unlock() }
        return try body()
    }
}
