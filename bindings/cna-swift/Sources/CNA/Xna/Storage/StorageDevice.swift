// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

internal final class StorageDeviceEventState {
    let source = CNAEventSource<CNAEventArgs>()
    private let lock = NSLock()
    private var armed = false
    private var registration: UInt64 = 0
    private var callbackBox: Unmanaged<StorageDeviceEventState>?

    func arm() {
        lock.lock()
        defer { lock.unlock() }
        guard !armed, let functions = try? NativeFunctions.load() else { return }
        let retained = Unmanaged.passRetained(self)
        var produced: UInt64 = 0
        let result = functions.storageDeviceSubscribeDeviceChanged(
            storageDeviceChangedCallback, retained.toOpaque(), &produced)
        guard result == 0 else {
            retained.release()
            return
        }
        registration = produced
        callbackBox = retained
        armed = true
    }

    func raise() {
        try? source.Raise(nil, args: CNAEventArgs.Empty)
    }
}

internal let storageDeviceEventState = StorageDeviceEventState()

internal let storageDeviceChangedCallback: CNASwift_StorageCompletionCallback = {
    context in
    guard let context else { return }
    Unmanaged<StorageDeviceEventState>.fromOpaque(context)
        .takeUnretainedValue().raise()
}

extension Microsoft.Xna.Framework.Storage {

    /// `Microsoft.Xna.Framework.Storage.StorageDevice`.
    ///
    /// **The first projected type that does not live in a game's runtime.**
    /// Every other owned type here reaches its routes through
    /// `RuntimeRegistry.current()`, which refuses outside a Game callback.
    /// `cna_storage_device_show_selector` takes no game handle at all, and
    /// XNA's own `StorageDevice` is usable without a `Game`, so requiring one
    /// would be this binding narrowing a contract the ABI does not narrow.
    /// It reaches the function table directly through `NativeFunctions.load()`,
    /// which is cached and needs nothing.
    ///
    /// The cost of that is lifetime: with no `RuntimeState` there is no child
    /// registry to fall back on, so this type releases its own handle in
    /// `deinit`. Foundation 101 is why that is written down rather than
    /// assumed -- `ContentManager` was the one owned type without such a
    /// release and it leaked its handle past every consumer that dropped one.
    public final class StorageDevice {

        private let functions: NativeFunctions
        private var handle: UInt64

        internal init(handle: UInt64, functions: NativeFunctions) {
            self.handle = handle
            self.functions = functions
        }

        deinit {
            guard handle != 0 else { return }
            _ = functions.storageDeviceDestroy(handle)
        }

        // ------------------------------------------------------------------
        // Selection
        // ------------------------------------------------------------------

        /// `StorageDevice.DeviceChanged`. XNA raises the static event with a
        /// null sender; the process-global CNA subscription is armed lazily
        /// when the event is first named and then lives for the process.
        public static var DeviceChanged: CNAEvent<CNAEventArgs> {
            storageDeviceEventState.arm()
            return storageDeviceEventState.source.Event
        }

        /// `BeginShowSelector(AsyncCallback callback, Object state)`.
        public static func BeginShowSelector(
            _ callback: CNAAsyncCallback, state: Any?
        ) throws -> any CNAAsyncResult {
            let functions = try NativeFunctions.load()
            var produced: UInt64 = 0
            try functions.check(
                functions.storageDeviceShowSelector(nil, nil, &produced),
                operation: "cna_storage_device_show_selector")
            return complete(produced, .device, callback, state)
        }

        /// `BeginShowSelector(Int32 sizeInBytes, Int32 directoryCount, AsyncCallback callback, Object state)`.
        public static func BeginShowSelector(
            _ sizeInBytes: Int32, directoryCount: Int32,
            callback: CNAAsyncCallback, state: Any?
        ) throws -> any CNAAsyncResult {
            let functions = try NativeFunctions.load()
            var produced: UInt64 = 0
            try functions.check(
                functions.storageDeviceShowSelectorWithSpace(
                    sizeInBytes, directoryCount, nil, nil, &produced),
                operation: "cna_storage_device_show_selector_with_space")
            return complete(produced, .device, callback, state)
        }

        /// `BeginShowSelector(PlayerIndex player, AsyncCallback callback, Object state)`.
        public static func BeginShowSelector(
            _ player: Microsoft.Xna.Framework.PlayerIndex,
            callback: CNAAsyncCallback, state: Any?
        ) throws -> any CNAAsyncResult {
            let functions = try NativeFunctions.load()
            var produced: UInt64 = 0
            try functions.check(
                functions.storageDeviceShowSelectorForPlayer(
                    UInt32(player.rawValue), nil, nil, &produced),
                operation: "cna_storage_device_show_selector_for_player")
            return complete(produced, .device, callback, state)
        }

        /// `BeginShowSelector(PlayerIndex player, Int32 sizeInBytes, Int32 directoryCount, AsyncCallback callback, Object state)`.
        public static func BeginShowSelector(
            _ player: Microsoft.Xna.Framework.PlayerIndex,
            sizeInBytes: Int32, directoryCount: Int32,
            callback: CNAAsyncCallback, state: Any?
        ) throws -> any CNAAsyncResult {
            let functions = try NativeFunctions.load()
            var produced: UInt64 = 0
            try functions.check(
                functions.storageDeviceShowSelectorForPlayerWithSpace(
                    UInt32(player.rawValue), sizeInBytes, directoryCount,
                    nil, nil, &produced),
                operation: "cna_storage_device_show_selector_for_player_with_space")
            return complete(produced, .device, callback, state)
        }

        /// `EndShowSelector(IAsyncResult result)`.
        public static func EndShowSelector(
            _ result: any CNAAsyncResult
        ) throws -> StorageDevice {
            guard let produced = result as? StorageAsyncResult,
                  produced.kind == .device else {
                throw CNAArgumentException(
                    message: StorageDevice.wrongResultMessage,
                    paramName: "result")
            }
            return StorageDevice(handle: produced.produced,
                                 functions: try NativeFunctions.load())
        }

        // ------------------------------------------------------------------
        // Containers
        // ------------------------------------------------------------------

        /// `BeginOpenContainer(String displayName, AsyncCallback callback, Object state)`.
        public func BeginOpenContainer(
            _ displayName: String, callback: CNAAsyncCallback, state: Any?
        ) throws -> any CNAAsyncResult {
            let live = try validated()
            var utf8 = Array(displayName.utf8)
            var produced: UInt64 = 0
            try functions.check(
                StorageDevice.withStringView(&utf8) { view in
                    functions.storageContainerOpen(live, view, nil, nil, &produced)
                },
                operation: "cna_storage_container_open")
            return StorageDevice.complete(produced, .container, callback, state)
        }

        /// `EndOpenContainer(IAsyncResult result)`.
        public func EndOpenContainer(
            _ result: any CNAAsyncResult
        ) throws -> Microsoft.Xna.Framework.Storage.StorageContainer {
            guard let produced = result as? StorageAsyncResult,
                  produced.kind == .container else {
                throw CNAArgumentException(
                    message: StorageDevice.wrongResultMessage,
                    paramName: "result")
            }
            return try Microsoft.Xna.Framework.Storage.StorageContainer(
                handle: produced.produced, functions: functions, device: self)
        }

        /// `DeleteContainer(String titleName)`.
        public func DeleteContainer(_ titleName: String) throws {
            let live = try validated()
            var utf8 = Array(titleName.utf8)
            try functions.check(
                StorageDevice.withStringView(&utf8) { view in
                    functions.storageDeviceDeleteContainer(live, view)
                },
                operation: "cna_storage_device_delete_container")
        }

        // ------------------------------------------------------------------
        // State
        // ------------------------------------------------------------------

        /// `StorageDevice.FreeSpace`.
        public var FreeSpace: Int64 {
            get throws {
                let live = try validated()
                var value: Int64 = 0
                try functions.check(
                    functions.storageDeviceGetFreeSpace(live, &value),
                    operation: "cna_storage_device_get_free_space")
                return value
            }
        }

        /// `StorageDevice.TotalSpace`.
        public var TotalSpace: Int64 {
            get throws {
                let live = try validated()
                var value: Int64 = 0
                try functions.check(
                    functions.storageDeviceGetTotalSpace(live, &value),
                    operation: "cna_storage_device_get_total_space")
                return value
            }
        }

        /// `StorageDevice.IsConnected`.
        ///
        /// **Not `throws`, and the accessor table is why.** The CLR getter has
        /// no failure path, so a Swift reader that threw would be offering a
        /// refusal XNA cannot produce. A released device answers false, which
        /// is what "not connected" means, and a route failure answers the same
        /// -- there is no third thing an infallible Bool can say.
        public var IsConnected: Bool {
            guard handle != 0 else { return false }
            var value: UInt8 = 0
            guard functions.storageDeviceGetIsConnected(handle, &value) == 0
            else { return false }
            return value != 0
        }

        // ------------------------------------------------------------------
        // Internals
        // ------------------------------------------------------------------

        internal var nativeHandle: UInt64 { handle }

        /// `FrameworkResources` has no message for this; it is the binding's
        /// own, because XNA cannot reach the state: a CLR `IAsyncResult` from
        /// the wrong Begin is a cast failure inside the End method, and the
        /// projection reports it as an argument fault rather than trapping.
        internal static let wrongResultMessage =
            "The IAsyncResult was not produced by the matching Begin method."

        private func validated() throws -> UInt64 {
            guard handle != 0 else {
                throw CNAObjectDisposedException(objectName: "\(type(of: self))")
            }
            return handle
        }

        private static func complete(
            _ produced: UInt64, _ kind: StorageAsyncResult.Kind,
            _ callback: CNAAsyncCallback, _ state: Any?
        ) -> StorageAsyncResult {
            let result = StorageAsyncResult(
                produced: produced, kind: kind, state: state)
            // XNA invokes the callback when the operation completes. It has
            // completed, so it is invoked here -- before Begin returns, which
            // is what CompletedSynchronously tells the caller to expect.
            callback(result)
            return result
        }

        private static func withStringView(
            _ utf8: inout [UInt8], _ body: (CNASwift_StringView) -> UInt32
        ) -> UInt32 {
            utf8.withUnsafeMutableBufferPointer { buffer -> UInt32 in
                var view = CNASwift_StringView()
                view.byte_length = UInt64(buffer.count)
                guard let base = buffer.baseAddress else { return body(view) }
                return base.withMemoryRebound(to: CChar.self, capacity: buffer.count) {
                    view.data = UnsafePointer($0)
                    return body(view)
                }
            }
        }
    }
}
