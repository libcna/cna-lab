// SPDX-License-Identifier: MIT

/// The erased destroy route a `NativeHandleStorage` holds.
///
/// This is deliberately *not* one of the per-route types in
/// `NativeFunctions`: those exist so tools/native_abi can pair one canonical
/// symbol with one stored property, and every owned handle kind hands its own
/// destroy route to the same storage slot. Assigning a route type here is
/// checked by the compiler, because a Swift typealias is transparent and the
/// two spellings denote the same `@convention(c)` function type.
internal typealias NativeDestroyRoute = @convention(c) (UInt64) -> UInt32

internal protocol RuntimeOwnedChild: AnyObject {
    var runtimeObjectIsDisposed: Bool { get }
    func disposeFromParent() throws
}

internal final class WeakRuntimeChild {
    weak var value: RuntimeOwnedChild?
    init(_ value: RuntimeOwnedChild) { self.value = value }
}

internal final class NativeHandleStorage {
    private(set) var handle: UInt64
    let typeName: String
    let ownership: NativeOwnership
    let generation: UInt64
    let runtime: RuntimeState
    private let destroy: NativeDestroyRoute

    init(
        handle: UInt64,
        typeName: String,
        ownership: NativeOwnership,
        runtime: RuntimeState,
        destroy: @escaping NativeDestroyRoute
    ) {
        self.handle = handle
        self.typeName = typeName
        self.ownership = ownership
        generation = runtime.generation
        self.runtime = runtime
        self.destroy = destroy
    }

    var isDisposed: Bool { handle == 0 }

    func validatedHandle(_ operation: String) throws -> UInt64 {
        guard handle != 0 else { throw CNAError.disposedObject(typeName) }
        try runtime.validateGeneration(generation)
        try runtime.owner.validate(operation)
        return handle
    }

    func dispose(operation: String) throws {
        if handle == 0 { return }
        let current = try validatedHandle(operation)
        let result = destroy(current)
        try runtime.functions.check(result, operation: operation)
        handle = 0
    }

    deinit {
        guard handle != 0,
              runtime.isActive,
              runtime.generation == generation,
              runtime.owner.isCurrent else { return }
        if destroy(handle) == 0 { handle = 0 }
    }
}
