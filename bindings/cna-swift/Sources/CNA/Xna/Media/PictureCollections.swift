// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.PictureCollection`.
    ///
    /// One of the six Media collections; the behaviour is in
    /// `MediaCollectionStorage`.
    public final class PictureCollection: RuntimeOwnedChild {

        private let storage: MediaCollectionStorage
        private let runtime: RuntimeState

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = MediaCollectionStorage(
                handle: handle, runtime: runtime, family: .picture)
            runtime.register(self)
        }

        /// `PictureCollection.IsDisposed`.
        public var IsDisposed: Bool { storage.isDisposed }

        /// `PictureCollection.Count`.
        public var Count: Int32 { get throws { try storage.count() } }

        /// `PictureCollection.Item[Int32 index]`, a read-only indexed property and
        /// therefore a subscript, with `get throws` because the CLR getter is
        /// fallible.
        public subscript(index: Int32) -> Picture {
            get throws {
                Picture(handle: try storage.handle(at: index), runtime: runtime)
            }
        }

        /// `PictureCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<Picture> {
            CNAEnumerator(expectedVersion: 0) { [self] index, _ in
                let total = try storage.count()
                guard index < Int(total) else { return nil }
                return try self[Int32(index)]
            }
        }

        /// `PictureCollection.Dispose()`.
        public func Dispose() throws { try storage.dispose() }

        internal var runtimeObjectIsDisposed: Bool { storage.isDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// `Microsoft.Xna.Framework.Media.PictureAlbumCollection`.
    ///
    /// One of the six Media collections; the behaviour is in
    /// `MediaCollectionStorage`.
    public final class PictureAlbumCollection: RuntimeOwnedChild {

        private let storage: MediaCollectionStorage
        private let runtime: RuntimeState

        internal init(handle: UInt64, runtime: RuntimeState) {
            self.runtime = runtime
            self.storage = MediaCollectionStorage(
                handle: handle, runtime: runtime, family: .pictureAlbum)
            runtime.register(self)
        }

        /// `PictureAlbumCollection.IsDisposed`.
        public var IsDisposed: Bool { storage.isDisposed }

        /// `PictureAlbumCollection.Count`.
        public var Count: Int32 { get throws { try storage.count() } }

        /// `PictureAlbumCollection.Item[Int32 index]`, a read-only indexed property and
        /// therefore a subscript, with `get throws` because the CLR getter is
        /// fallible.
        public subscript(index: Int32) -> PictureAlbum {
            get throws {
                PictureAlbum(handle: try storage.handle(at: index), runtime: runtime)
            }
        }

        /// `PictureAlbumCollection.GetEnumerator()`.
        public func GetEnumerator() -> CNAEnumerator<PictureAlbum> {
            CNAEnumerator(expectedVersion: 0) { [self] index, _ in
                let total = try storage.count()
                guard index < Int(total) else { return nil }
                return try self[Int32(index)]
            }
        }

        /// `PictureAlbumCollection.Dispose()`.
        public func Dispose() throws { try storage.dispose() }

        internal var runtimeObjectIsDisposed: Bool { storage.isDisposed }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
