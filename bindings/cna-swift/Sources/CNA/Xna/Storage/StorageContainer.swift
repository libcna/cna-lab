// SPDX-License-Identifier: MIT

import CNAShim

internal final class StorageContainerDisposingBox {
    weak var target: Microsoft.Xna.Framework.Storage.StorageContainer?
    init(_ target: Microsoft.Xna.Framework.Storage.StorageContainer) {
        self.target = target
    }
}

internal let storageContainerDisposingCallback: CNASwift_StorageCompletionCallback = {
    context in
    guard let context else { return }
    let box = Unmanaged<StorageContainerDisposingBox>.fromOpaque(context)
        .takeUnretainedValue()
    box.target?.nativeDisposing()
}

extension Microsoft.Xna.Framework.Storage {

    /// `Microsoft.Xna.Framework.Storage.StorageContainer`.
    ///
    /// Opened through `StorageDevice.BeginOpenContainer` and released by
    /// `Dispose`, which is what CNA's own `cna_storage_container_dispose`
    /// means -- distinct from `destroy`, which releases the handle. XNA's type
    /// declares a finalizer as well as `Dispose`, and `deinit` is that
    /// finalizer's language projection, so a container a consumer drops does
    /// not leak its handle.
    ///
    /// Storage file returns are projected as `CNAStorageStream`, the concrete
    /// `Foundation.InputStream` subclass that also exposes XNA's write, seek,
    /// flush and resize behavior. Read-only stream positions elsewhere keep
    /// the established global `InputStream` mapping.
    public final class StorageContainer {

        private let functions: NativeFunctions
        private let device: StorageDevice
        private var handle: UInt64
        private var disposed = false
        private let disposingSource = CNAEventSource<CNAEventArgs>()
        private var disposingRegistration: UInt64 = 0
        private var disposingBox: Unmanaged<StorageContainerDisposingBox>?
        private var suppressNativeDisposing = false

        internal init(handle: UInt64, functions: NativeFunctions,
                      device: StorageDevice) throws {
            self.handle = handle
            self.functions = functions
            self.device = device
            try subscribeDisposing()
        }

        deinit {
            guard handle != 0 else { return }
            releaseDisposingSubscription()
            _ = functions.storageContainerDestroy(handle)
        }

        /// `StorageContainer.DisplayName`.
        public var DisplayName: String {
            get throws {
                let live = try validated()
                var byteCount: UInt64 = 0
                try functions.check(
                    functions.storageContainerGetDisplayNameSize(live, &byteCount),
                    operation: "cna_storage_container_get_display_name_size")
                guard byteCount > 0 else { return "" }
                var bytes = [CChar](repeating: 0, count: Int(byteCount) + 1)
                var written: UInt64 = 0
                try functions.check(
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        functions.storageContainerCopyDisplayName(
                            live, buffer.baseAddress, byteCount, &written)
                    },
                    operation: "cna_storage_container_copy_display_name")
                return String(decoding: bytes.prefix(Int(written))
                                .map { UInt8(bitPattern: $0) }, as: UTF8.self)
            }
        }

        /// `StorageContainer.StorageDevice`.
        ///
        /// The device this container was opened from, held strongly: CNA's
        /// container is a child of the device handle, so a device released
        /// while a container is alive would leave the container pointing at
        /// nothing.
        public var StorageDevice: Microsoft.Xna.Framework.Storage.StorageDevice {
            get throws {
                // `get throws` because the CLR getter is fallible, which the
                // accessor table reads out of the IL rather than this file
                // guessing: reading it on a disposed container raises, and a
                // Swift reader that answered anyway would be inventing a
                // success XNA does not have.
                _ = try validated()
                return device
            }
        }

        /// `StorageContainer.IsDisposed`.
        public var IsDisposed: Bool {
            guard !disposed, handle != 0 else { return true }
            var value: UInt8 = 0
            guard functions.storageContainerGetIsDisposed(handle, &value) == 0
            else { return disposed }
            return value != 0
        }

        /// `StorageContainer.Disposing`.
        public var Disposing: CNAEvent<CNAEventArgs> { disposingSource.Event }

        /// `StorageContainer.Dispose()`.
        public func Dispose() throws {
            guard !disposed, handle != 0 else { return }
            suppressNativeDisposing = true
            defer { suppressNativeDisposing = false }
            try functions.check(functions.storageContainerDispose(handle),
                                operation: "cna_storage_container_dispose")
            disposed = true
            try disposingSource.Raise(self, args: CNAEventArgs.Empty)
        }

        // ------------------------------------------------------------------
        // Directories and files
        // ------------------------------------------------------------------

        /// `CreateDirectory(String directory)`.
        public func CreateDirectory(_ directory: String) throws {
            try withName(directory, "directory") { live, view in
                self.functions.storageContainerCreateDirectory(live, view)
            } operation: { "cna_storage_container_create_directory" }
        }

        /// `DeleteDirectory(String directory)`.
        public func DeleteDirectory(_ directory: String) throws {
            try withName(directory, "directory") { live, view in
                self.functions.storageContainerDeleteDirectory(live, view)
            } operation: { "cna_storage_container_delete_directory" }
        }

        /// `DeleteFile(String file)`.
        public func DeleteFile(_ file: String) throws {
            try withName(file, "file") { live, view in
                self.functions.storageContainerDeleteFile(live, view)
            } operation: { "cna_storage_container_delete_file" }
        }

        /// `StorageContainer.CreateFile(String)`.
        public func CreateFile(_ file: String) throws -> CNAStorageStream {
            let live = try validated()
            var utf8 = Array(file.utf8)
            var created: UInt64 = 0
            try functions.check(
                StorageContainer.withStringView(&utf8) {
                    functions.storageContainerCreateFile(live, $0, &created)
                }, operation: "cna_storage_container_create_file")
            return CNAStorageStream(
                handle: created, functions: functions, container: self)
        }

        /// `StorageContainer.OpenFile(String, FileMode)`.
        public func OpenFile(
            _ file: String, fileMode: CNAFileMode
        ) throws -> CNAStorageStream {
            let live = try validated()
            var utf8 = Array(file.utf8)
            var opened: UInt64 = 0
            try functions.check(
                StorageContainer.withStringView(&utf8) {
                    functions.storageContainerOpenFile(
                        live, $0, UInt32(bitPattern: fileMode.rawValue), &opened)
                }, operation: "cna_storage_container_open_file")
            return CNAStorageStream(
                handle: opened, functions: functions, container: self)
        }

        /// `StorageContainer.OpenFile(String, FileMode, FileAccess)`.
        public func OpenFile(
            _ file: String, fileMode: CNAFileMode, fileAccess: CNAFileAccess
        ) throws -> CNAStorageStream {
            let live = try validated()
            var utf8 = Array(file.utf8)
            var opened: UInt64 = 0
            try functions.check(
                StorageContainer.withStringView(&utf8) {
                    functions.storageContainerOpenFileAccess(
                        live, $0, UInt32(bitPattern: fileMode.rawValue),
                        UInt32(bitPattern: fileAccess.rawValue), &opened)
                }, operation: "cna_storage_container_open_file_access")
            return CNAStorageStream(
                handle: opened, functions: functions, container: self)
        }

        /// `StorageContainer.OpenFile(String, FileMode, FileAccess, FileShare)`.
        public func OpenFile(
            _ file: String, fileMode: CNAFileMode, fileAccess: CNAFileAccess,
            fileShare: CNAFileShare
        ) throws -> CNAStorageStream {
            let live = try validated()
            var utf8 = Array(file.utf8)
            var opened: UInt64 = 0
            try functions.check(
                StorageContainer.withStringView(&utf8) {
                    functions.storageContainerOpenFileShare(
                        live, $0, UInt32(bitPattern: fileMode.rawValue),
                        UInt32(bitPattern: fileAccess.rawValue),
                        UInt32(bitPattern: fileShare.rawValue), &opened)
                }, operation: "cna_storage_container_open_file_share")
            return CNAStorageStream(
                handle: opened, functions: functions, container: self)
        }

        /// `DirectoryExists(String directory)`.
        public func DirectoryExists(_ directory: String) throws -> Bool {
            try exists(directory, "directory") { live, view, out in
                self.functions.storageContainerDirectoryExists(live, view, out)
            } operation: { "cna_storage_container_directory_exists" }
        }

        /// `FileExists(String file)`.
        public func FileExists(_ file: String) throws -> Bool {
            try exists(file, "file") { live, view, out in
                self.functions.storageContainerFileExists(live, view, out)
            } operation: { "cna_storage_container_file_exists" }
        }

        /// `GetDirectoryNames()`.
        public func GetDirectoryNames() throws -> [String] {
            try names("*", directories: true)
        }

        /// `GetDirectoryNames(String searchPattern)`.
        public func GetDirectoryNames(_ searchPattern: String) throws -> [String] {
            try names(searchPattern, directories: true)
        }

        /// `GetFileNames()`.
        public func GetFileNames() throws -> [String] {
            try names("*", directories: false)
        }

        /// `GetFileNames(String searchPattern)`.
        public func GetFileNames(_ searchPattern: String) throws -> [String] {
            try names(searchPattern, directories: false)
        }

        // ------------------------------------------------------------------
        // Internals
        // ------------------------------------------------------------------

        private func validated() throws -> UInt64 {
            guard !disposed, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "\(type(of: self))")
            }
            return handle
        }

        internal func nativeDisposing() {
            guard !suppressNativeDisposing else { return }
            try? disposingSource.Raise(self, args: CNAEventArgs.Empty)
        }

        private func subscribeDisposing() throws {
            let retained = Unmanaged.passRetained(StorageContainerDisposingBox(self))
            var registration: UInt64 = 0
            let result = functions.storageContainerSubscribeDisposing(
                handle, storageContainerDisposingCallback,
                retained.toOpaque(), &registration)
            guard result == 0 else {
                retained.release()
                try functions.check(
                    result, operation: "cna_storage_container_subscribe_disposing")
                return
            }
            disposingRegistration = registration
            disposingBox = retained
        }

        private func releaseDisposingSubscription() {
            if disposingRegistration != 0 {
                _ = functions.storageContainerUnsubscribeDisposing(
                    disposingRegistration)
                disposingRegistration = 0
            }
            disposingBox?.release()
            disposingBox = nil
        }

        private func withName(
            _ name: String, _ parameter: String,
            _ call: (UInt64, CNASwift_StringView) -> UInt32,
            operation: () -> String
        ) throws {
            let live = try validated()
            var utf8 = Array(name.utf8)
            try functions.check(
                StorageContainer.withStringView(&utf8) { call(live, $0) },
                operation: operation())
        }

        private func exists(
            _ name: String, _ parameter: String,
            _ call: (UInt64, CNASwift_StringView, UnsafeMutablePointer<UInt8>?) -> UInt32,
            operation: () -> String
        ) throws -> Bool {
            let live = try validated()
            var utf8 = Array(name.utf8)
            var value: UInt8 = 0
            try functions.check(
                StorageContainer.withStringView(&utf8) { call(live, $0, &value) },
                operation: operation())
            return value != 0
        }

        private func names(_ pattern: String, directories: Bool) throws -> [String] {
            let live = try validated()
            var utf8 = Array(pattern.utf8)
            var count: UInt64 = 0
            try functions.check(
                StorageContainer.withStringView(&utf8) { view in
                    directories
                        ? self.functions.storageContainerGetDirectoryNameCount(
                            live, view, &count)
                        : self.functions.storageContainerGetFileNameCount(
                            live, view, &count)
                },
                operation: directories
                    ? "cna_storage_container_get_directory_name_count"
                    : "cna_storage_container_get_file_name_count")
            guard count > 0 else { return [] }

            var produced: [String] = []
            produced.reserveCapacity(Int(count))
            for index in 0..<count {
                var bytes = [CChar](repeating: 0, count: 4096)
                var written: UInt64 = 0
                try functions.check(
                    StorageContainer.withStringView(&utf8) { view in
                        bytes.withUnsafeMutableBufferPointer { buffer in
                            directories
                                ? self.functions.storageContainerCopyDirectoryName(
                                    live, view, index, buffer.baseAddress,
                                    UInt64(buffer.count), &written)
                                : self.functions.storageContainerCopyFileName(
                                    live, view, index, buffer.baseAddress,
                                    UInt64(buffer.count), &written)
                        }
                    },
                    operation: directories
                        ? "cna_storage_container_copy_directory_name"
                        : "cna_storage_container_copy_file_name")
                produced.append(String(decoding: bytes.prefix(Int(written))
                                        .map { UInt8(bitPattern: $0) }, as: UTF8.self))
            }
            return produced
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
