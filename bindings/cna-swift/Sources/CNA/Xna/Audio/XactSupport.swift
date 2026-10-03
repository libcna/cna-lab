// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

/// Shared marshaling for the XACT facade. It is internal implementation
/// machinery, not an XNA identity.
internal enum XactSupport {
    static let nullNotAllowed = "This method does not accept null for this parameter."

    static func requiredString(_ value: String?, parameter: String) throws -> String {
        guard let value, !value.isEmpty else {
            throw CNAArgumentNullException(
                paramName: parameter, message: nullNotAllowed)
        }
        return value
    }

    static func fullPath(_ value: String) -> String {
        URL(fileURLWithPath: value).standardizedFileURL.path
    }

    static func withStringView(
        _ value: String, _ body: (CNASwift_StringView) -> UInt32
    ) -> UInt32 {
        var bytes = Array(value.utf8)
        return bytes.withUnsafeMutableBufferPointer { buffer in
            var view = CNASwift_StringView()
            view.data = UnsafeRawPointer(buffer.baseAddress)?
                .assumingMemoryBound(to: CChar.self)
            view.byte_length = UInt64(buffer.count)
            return body(view)
        }
    }

    static func copiedString(
        functions: NativeFunctions,
        sizeOperation: String,
        copyOperation: String,
        size: (UnsafeMutablePointer<UInt64>?) -> UInt32,
        copy: (UnsafeMutablePointer<CChar>?, UInt64,
               UnsafeMutablePointer<UInt64>?) -> UInt32
    ) throws -> String {
        var required: UInt64 = 0
        try functions.check(size(&required), operation: sizeOperation)
        guard required <= UInt64(Int.max) else {
            throw CNAError.nativeFailure(
                operation: copyOperation, result: 1,
                message: "native text length is not representable")
        }
        var bytes = [UInt8](repeating: 0, count: Int(required))
        var written: UInt64 = 0
        let result = bytes.withUnsafeMutableBufferPointer { buffer in
            copy(
                UnsafeMutableRawPointer(buffer.baseAddress)?
                    .assumingMemoryBound(to: CChar.self),
                UInt64(buffer.count), &written)
        }
        try functions.check(result, operation: copyOperation)
        guard written <= UInt64(bytes.count) else {
            throw CNAError.nativeFailure(
                operation: copyOperation, result: 1,
                message: "native text copy exceeded its destination")
        }
        return String(decoding: bytes.prefix(Int(written)), as: UTF8.self)
    }
}

/// The engine owns banks and category handles at the native layer. Keeping
/// weak Swift children lets it reproduce XNA's children-before-parent teardown
/// without inventing a second owner.
internal protocol XactEngineChild: RuntimeOwnedChild {}
