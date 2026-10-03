// SPDX-License-Identifier: MIT

import Foundation

/// The formal `System.EventArgs` projection.
///
/// This is a CLR *class*, and the whole XNA event-argument hierarchy derives
/// from it: `GameComponentCollectionEventArgs`, `ResourceCreatedEventArgs`,
/// `ResourceDestroyedEventArgs` and `PreparingDeviceSettingsEventArgs` all have
/// `System.EventArgs` as their direct base. Projecting it as a Swift `struct`
/// would make that hierarchy inexpressible, so it is an `open class`: base
/// fidelity is worth more than the accidental value semantics the earlier
/// support struct happened to have. The base is a measured mapping, not a
/// dropped one — see `BASE_MAPPING_MISMATCH` in the strict verifier.
///
/// It deliberately does **not** conform to `Sendable`. It is open, so a
/// subclass anywhere may add mutable stored state, and neither this type nor
/// the compiler can make a cross-actor safety promise on that subclass's
/// behalf. `@unchecked Sendable` would assert exactly the guarantee that
/// cannot be earned here.
///
/// It is deliberately outside the strict XNA namespace and is not counted as
/// an XNA type.
open class CNAEventArgs {
    /// `System.EventArgs.Empty`.
    ///
    /// The pinned XNA IL never constructs an event-argument object for an
    /// `EventHandler<EventArgs>` raise: all 46 raise sites across the
    /// registered `Microsoft.Xna.Framework.Game.dll` and
    /// `Microsoft.Xna.Framework.Graphics.dll` load the static
    /// `System.EventArgs::Empty` field, and none executes `newobj`. One shared
    /// instance therefore preserves the object identity a handler observes.
    public static let Empty = CNAEventArgs()

    public init() {}
}

/// Errors raised by CNA-Swift's own runtime, and **only** by it.
///
/// This type is intentionally outside the strict `Microsoft.Xna.Framework`
/// projection and is deliberately not in the `CNAException` hierarchy. It
/// reports a native library that will not load, an admitted-ABI refusal, a
/// missing symbol, a native call failure, an owner-thread violation, a stale
/// runtime generation, an unsupported platform, callback-lifecycle misuse, a
/// stream failure and one producer invariant — none of which corresponds to a
/// CLR or XNA exception identity. Every CLR-shaped failure the projection
/// raises is a real projected exception class instead, so `catch is
/// CNAException` cannot swallow a native runtime error and `catch is CNAError`
/// cannot swallow a projected one.
public enum CNAError: Error, Equatable, CustomStringConvertible {
    case nativeLibraryNotFound(candidates: [String])
    case nativeLibraryLoadFailed(path: String, message: String)
    case missingNativeSymbol(String)
    case unsupportedABIVersion(admitted: String, actual: String, actualEncoded: UInt32, path: String)
    case nativeFailure(operation: String, result: UInt32, message: String)
    case unsupportedPlatform(String)
    case disposedObject(String)
    case ownerThreadViolation(String)
    case staleRuntimeGeneration(expected: UInt64, actual: UInt64?)
    case callbackOutsideGameLifecycle
    case streamFailure(String)
    /// A CNA-Swift producer invariant, and deliberately not a CLR argument
    /// failure. After the exception-payload conversion, no case of this enum
    /// is a CLR-shaped identity: every projected `ArgumentException`,
    /// `ArgumentNullException`, `ArgumentOutOfRangeException`,
    /// `NotSupportedException`, `InvalidOperationException`,
    /// `KeyNotFoundException`, `NullReferenceException` and
    /// `IndexOutOfRangeException` is now the real class, so a name here that
    /// merely looked like one would invite the confusion this channel exists
    /// to prevent.
    case producerInvariant(String)

    public var description: String {
        switch self {
        case .nativeLibraryNotFound(let candidates):
            return "CNA native library was not found; tried: \(candidates.joined(separator: ", "))"
        case .nativeLibraryLoadFailed(let path, let message):
            return "Could not load CNA native library at \(path): \(message)"
        case .missingNativeSymbol(let symbol):
            return "CNA native library is missing required symbol \(symbol)"
        case .unsupportedABIVersion(let admitted, let actual, let actualEncoded, let path):
            // The diagnostic names all three facts a caller needs to act:
            // what was admitted, what the library actually reports, and which
            // file was selected — an installed soname and an absolute
            // CNA_NATIVE_LIBRARY override are otherwise indistinguishable.
            return String(
                format: "CNA native library %@ reports C ABI %@ (0x%08X); CNA-Swift admits %@",
                path, actual, actualEncoded, admitted
            )
        case .nativeFailure(let operation, let result, let message):
            return "CNA operation \(operation) failed with result \(result): \(message)"
        case .unsupportedPlatform(let platform):
            return "CNA-Swift runtime is not qualified on \(platform)"
        case .disposedObject(let type):
            return "\(type) is disposed"
        case .ownerThreadViolation(let operation):
            return "\(operation) must run on the native Game owner thread"
        case .staleRuntimeGeneration(let expected, let actual):
            return "Native object belongs to stale Game generation \(expected); active generation is \(actual.map(String.init) ?? "none")"
        case .callbackOutsideGameLifecycle:
            return "The native operation requires an active Game lifecycle callback"
        case .streamFailure(let message):
            return "InputStream failed: \(message)"
        case .producerInvariant(let message):
            return message
        }
    }
}

/// Throwing support projection for CLR `IEnumerator<T>` return values.
///
/// This type deliberately does not conform to Swift `IteratorProtocol`: XNA
/// collection enumeration can fail when the source mutates, while
/// `IteratorProtocol.next()` cannot throw.
public final class CNAEnumerator<Element> {
    private let nextElement: (Int, UInt64) throws -> Element?
    private let expectedVersion: UInt64
    private var index = 0

    internal init(
        expectedVersion: UInt64,
        nextElement: @escaping (Int, UInt64) throws -> Element?
    ) {
        self.expectedVersion = expectedVersion
        self.nextElement = nextElement
    }

    public func Next() throws -> Element? {
        let value = try nextElement(index, expectedVersion)
        if value != nil { index += 1 }
        return value
    }
}
