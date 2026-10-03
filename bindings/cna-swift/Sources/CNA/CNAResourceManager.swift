// SPDX-License-Identifier: MIT

import Foundation

private final class CNAProjectedRuntimeResourceSet {}

/// The selected, self-contained projection of .NET Framework 4.0
/// `System.Resources.ResourceManager`.
///
/// Assembly, CultureInfo, ResourceSet and UnmanagedMemoryStream overloads are
/// outside the measured ContentReader closure. The protected constructor and
/// the virtual invariant-culture lookup path remain real so an ordinary Swift
/// subclass can supply resources exactly as ResourceContentManager expects.
open class CNAResourceManager {
    private let lock = NSRecursiveLock()
    private let baseName: String?
    private let resourceFactories: [String: () -> Any]
    private var loadedResources: [String: Any] = [:]
    private var ignoreCase = false

    /// `ResourceManager..ctor()`, widened from protected for Swift subclasses.
    public init() {
        baseName = nil
        resourceFactories = [:]
    }

    /// Project-owned resource fixtures enter through this non-public path.
    /// Each closure models loading one serialized resource value; releasing
    /// cached resource sets makes a later lookup invoke it again.
    internal init(
        baseName: String,
        resourceFactories: [String: () -> Any]
    ) {
        self.baseName = baseName
        self.resourceFactories = resourceFactories
    }

    open var BaseName: String? { baseName }

    open var IgnoreCase: Bool {
        get {
            lock.lock()
            defer { lock.unlock() }
            return ignoreCase
        }
        set {
            lock.lock()
            ignoreCase = newValue
            lock.unlock()
        }
    }

    open var ResourceSetType: Any.Type {
        CNAProjectedRuntimeResourceSet.self
    }

    open func ReleaseAllResources() {
        lock.lock()
        loadedResources.removeAll(keepingCapacity: false)
        lock.unlock()
    }

    open func GetString(_ name: String?) throws -> String? {
        guard let value = try GetObject(name) else { return nil }
        guard let string = value as? String else {
            throw CNAInvalidOperationException(
                message: Self.resourceNotString.replacingOccurrences(
                    of: "{0}", with: Self.resourceTypeName(type(of: value))))
        }
        return string
    }

    open func GetObject(_ name: String?) throws -> Any? {
        guard let name else {
            throw CNAArgumentNullException(paramName: "name")
        }
        lock.lock()
        defer { lock.unlock() }

        let key: String?
        if resourceFactories[name] != nil {
            key = name
        } else if ignoreCase {
            key = resourceFactories.keys.sorted().first {
                $0.compare(name, options: .caseInsensitive) == .orderedSame
            }
        } else {
            key = nil
        }
        guard let key, let factory = resourceFactories[key] else { return nil }
        if let cached = loadedResources[key] { return cached }
        let value = factory()
        loadedResources[key] = value
        return value
    }

    private static func resourceTypeName(_ type: Any.Type) -> String {
        if type == [UInt8].self { return "System.Byte[]" }
        if type == String.self { return "System.String" }
        return String(reflecting: type)
    }

    internal static let resourceNotString =
        "Resource was of type '{0}' instead of String - call GetObject instead."
}
