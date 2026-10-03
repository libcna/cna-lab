// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.Picture`.
    ///
    /// One picture in the media library: its album, its size, when it was taken, and the two streams that carry it.
    ///
    /// Its identity is CNA's -- `Equals` and `GetHashCode` both ask the runtime
    /// -- and its `Name` and relations are throwing getters, the same shape the
    /// music half of this namespace has.
    public final class Picture: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var released = false

        /// A handle the library owns is never destroyed here.
        private let borrowed: Bool

        internal init(handle: UInt64, runtime: RuntimeState, borrowed: Bool = false) {
            self.runtime = runtime
            self.handle = handle
            self.borrowed = borrowed
            runtime.register(self)
        }

        /// `Picture.IsDisposed`.
        public var IsDisposed: Bool {
            guard !released, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.pictureGetIsDisposed(handle, &value) == 0 else {
                return released
            }
            return value != 0
        }

        /// `Picture.Name`.
        public var Name: String? {
            get throws {
                let live = try validated()
                var size: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.pictureGetNameSize(live, &size),
                    operation: "cna_picture_get_name_size")
                if size == 0 { return "" }
                var bytes = [CChar](repeating: 0, count: Int(size))
                var written: UInt64 = 0
                try runtime.functions.check(
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.pictureCopyName(
                            live, buffer.baseAddress, size, &written)
                    },
                    operation: "cna_picture_copy_name")
                return String(
                    decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                    as: UTF8.self)
            }
        }

        /// `Picture.Album`.
        ///
        /// The same library-context divergence the music half carries: a
        /// picture outside an album names none, and the return is proven
        /// non-null, so the absence is reported rather than fabricated.
        public var Album: PictureAlbum {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.pictureGetAlbum(live, &produced, &available),
                    operation: "cna_picture_get_album")
                guard available != 0 else {
                    throw CNAError.nativeFailure(
                        operation: "Picture.Album", result: 1,
                        message: "this picture belongs to no album")
                }
                return Microsoft.Xna.Framework.Media.PictureAlbum(
                    handle: produced, runtime: runtime, borrowed: true)
            }
        }

        /// `Picture.Width`.
        public var Width: Int32 {
            get throws {
                let live = try validated()
                var value: Int32 = 0
                try runtime.functions.check(
                    runtime.functions.pictureGetWidth(live, &value),
                    operation: "cna_picture_get_width")
                return value
            }
        }

        /// `Picture.Height`.
        public var Height: Int32 {
            get throws {
                let live = try validated()
                var value: Int32 = 0
                try runtime.functions.check(
                    runtime.functions.pictureGetHeight(live, &value),
                    operation: "cna_picture_get_height")
                return value
            }
        }

        /// `Picture.Date`.
        ///
        /// **`System.DateTime` maps to `Foundation.Date`**, the same move
        /// `System.TimeSpan` makes to `Duration`. CNA answers a Unix instant,
        /// which is exactly what a `Date` carries -- XNA's `DateTime` also has
        /// a `Kind`, and there is nothing here to reproduce it from.
        public var Date: Foundation.Date {
            get throws {
                let live = try validated()
                var ticks: Int64 = 0
                try runtime.functions.check(
                    runtime.functions.pictureGetDateUnixTicks(live, &ticks),
                    operation: "cna_picture_get_date_unix_ticks")
                return Foundation.Date(timeIntervalSince1970: Double(ticks))
            }
        }

        /// `Picture.GetImage()`.
        public func GetImage() throws -> Foundation.InputStream? {
            try readBytes(
                sizeOperation: "cna_picture_get_image_size",
                copyOperation: "cna_picture_copy_image", thumbnail: false)
        }

        /// `Picture.GetThumbnail()`.
        public func GetThumbnail() throws -> Foundation.InputStream? {
            try readBytes(
                sizeOperation: "cna_picture_get_thumbnail_size",
                copyOperation: "cna_picture_copy_thumbnail", thumbnail: true)
        }

        /// The two image reads share a body, and the route is selected by a
        /// flag rather than passed in: a `@convention(c)` pointer behind a
        /// Swift closure parameter segfaults in this toolchain.
        private func readBytes(
            sizeOperation: String, copyOperation: String, thumbnail: Bool
        ) throws -> Foundation.InputStream {
            let live = try validated()
            var bytes: UInt64 = 0
            let sizeResult = thumbnail
                ? runtime.functions.pictureGetThumbnailSize(live, &bytes)
                : runtime.functions.pictureGetImageSize(live, &bytes)
            try runtime.functions.check(sizeResult, operation: sizeOperation)
            if bytes == 0 { return Foundation.InputStream(data: Data()) }
            var buffer = [UInt8](repeating: 0, count: Int(bytes))
            var written: UInt64 = 0
            let copyResult = buffer.withUnsafeMutableBufferPointer { raw -> UInt32 in
                thumbnail
                    ? runtime.functions.pictureCopyThumbnail(
                        live, raw.baseAddress, bytes, &written)
                    : runtime.functions.pictureCopyImage(
                        live, raw.baseAddress, bytes, &written)
            }
            try runtime.functions.check(copyResult, operation: copyOperation)
            return Foundation.InputStream(data: Data(buffer.prefix(Int(written))))
        }

        /// `Picture.Dispose()`.
        public func Dispose() throws {
            guard !released else { return }
            released = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.pictureDispose(live), operation: "cna_picture_dispose")
            guard !borrowed else { return }
            try runtime.functions.check(
                runtime.functions.pictureDestroy(live), operation: "cna_picture_destroy")
        }

        /// `Picture.Equals(Picture other)`, answered by CNA and defined for null.
        public func Equals(_ other: Picture?) -> Bool {
            guard let other else { return false }
            guard !released, !other.released else { return self === other }
            var equal: UInt8 = 0
            guard runtime.functions.pictureEquals(
                handle, other.handle, &equal) == 0 else { return self === other }
            return equal != 0
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? Picture else { return false }
            return Equals(other)
        }

        /// `Picture.ToString()`, which XNA answers with the name.
        public func ToString() -> String? { (try? Name) ?? "" }

        /// `Picture.GetHashCode()`, taken from CNA so equal values hash alike.
        public func GetHashCode() -> Int32 {
            guard !released, handle != 0 else { return 0 }
            var value: Int32 = 0
            guard runtime.functions.pictureGetHashCode(handle, &value) == 0 else {
                return 0
            }
            return value
        }

        public static func == (lhs: Picture, rhs: Picture) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: Picture, rhs: Picture) -> Bool { !lhs.Equals(rhs) }

        private func validated() throws -> UInt64 {
            guard !released, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "Picture")
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { released }
        internal func disposeFromParent() throws { try Dispose() }
    }

    /// `Microsoft.Xna.Framework.Media.PictureAlbum`.
    ///
    /// One album of pictures, which nests: an album holds pictures and further albums, and every album but the root has a parent.
    ///
    /// Its identity is CNA's -- `Equals` and `GetHashCode` both ask the runtime
    /// -- and its `Name` and relations are throwing getters, the same shape the
    /// music half of this namespace has.
    public final class PictureAlbum: RuntimeOwnedChild {

        private let runtime: RuntimeState
        private var handle: UInt64
        private var released = false

        /// A handle the library owns is never destroyed here.
        private let borrowed: Bool

        internal init(handle: UInt64, runtime: RuntimeState, borrowed: Bool = false) {
            self.runtime = runtime
            self.handle = handle
            self.borrowed = borrowed
            runtime.register(self)
        }

        /// `PictureAlbum.IsDisposed`.
        public var IsDisposed: Bool {
            guard !released, handle != 0 else { return true }
            var value: UInt8 = 0
            guard runtime.functions.pictureAlbumGetIsDisposed(handle, &value) == 0 else {
                return released
            }
            return value != 0
        }

        /// `PictureAlbum.Name`.
        public var Name: String? {
            get throws {
                let live = try validated()
                var size: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.pictureAlbumGetNameSize(live, &size),
                    operation: "cna_picture_album_get_name_size")
                if size == 0 { return "" }
                var bytes = [CChar](repeating: 0, count: Int(size))
                var written: UInt64 = 0
                try runtime.functions.check(
                    bytes.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.pictureAlbumCopyName(
                            live, buffer.baseAddress, size, &written)
                    },
                    operation: "cna_picture_album_copy_name")
                return String(
                    decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                    as: UTF8.self)
            }
        }

        /// `PictureAlbum.Albums`.
        public var Albums: PictureAlbumCollection {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.pictureAlbumGetAlbums(live, &produced),
                    operation: "cna_picture_album_get_albums")
                return PictureAlbumCollection(handle: produced, runtime: runtime)
            }
        }

        /// `PictureAlbum.Pictures`.
        public var Pictures: PictureCollection {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.pictureAlbumGetPictures(live, &produced),
                    operation: "cna_picture_album_get_pictures")
                return PictureCollection(handle: produced, runtime: runtime)
            }
        }

        /// `PictureAlbum.Parent`.
        ///
        /// The root album has none, and CNA says so with an availability flag
        /// rather than a failure -- so the absence is reported here, because
        /// the return is proven non-null and there is nothing to hand back.
        public var Parent: PictureAlbum {
            get throws {
                let live = try validated()
                var produced: UInt64 = 0
                var available: UInt8 = 0
                try runtime.functions.check(
                    runtime.functions.pictureAlbumGetParent(live, &produced, &available),
                    operation: "cna_picture_album_get_parent")
                guard available != 0 else {
                    throw CNAError.nativeFailure(
                        operation: "PictureAlbum.Parent", result: 1,
                        message: "this album is the root and has no parent")
                }
                return Microsoft.Xna.Framework.Media.PictureAlbum(
                    handle: produced, runtime: runtime, borrowed: true)
            }
        }

        /// `PictureAlbum.Dispose()`.
        public func Dispose() throws {
            guard !released else { return }
            released = true
            let live = handle
            handle = 0
            try runtime.functions.check(
                runtime.functions.pictureAlbumDispose(live), operation: "cna_picture_album_dispose")
            guard !borrowed else { return }
            try runtime.functions.check(
                runtime.functions.pictureAlbumDestroy(live), operation: "cna_picture_album_destroy")
        }

        /// `PictureAlbum.Equals(PictureAlbum other)`, answered by CNA and defined for null.
        public func Equals(_ other: PictureAlbum?) -> Bool {
            guard let other else { return false }
            guard !released, !other.released else { return self === other }
            var equal: UInt8 = 0
            guard runtime.functions.pictureAlbumEquals(
                handle, other.handle, &equal) == 0 else { return self === other }
            return equal != 0
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? PictureAlbum else { return false }
            return Equals(other)
        }

        /// `PictureAlbum.ToString()`, which XNA answers with the name.
        public func ToString() -> String? { (try? Name) ?? "" }

        /// `PictureAlbum.GetHashCode()`, taken from CNA so equal values hash alike.
        public func GetHashCode() -> Int32 {
            guard !released, handle != 0 else { return 0 }
            var value: Int32 = 0
            guard runtime.functions.pictureAlbumGetHashCode(handle, &value) == 0 else {
                return 0
            }
            return value
        }

        public static func == (lhs: PictureAlbum, rhs: PictureAlbum) -> Bool { lhs.Equals(rhs) }
        public static func != (lhs: PictureAlbum, rhs: PictureAlbum) -> Bool { !lhs.Equals(rhs) }

        private func validated() throws -> UInt64 {
            guard !released, handle != 0 else {
                throw CNAObjectDisposedException(objectName: "PictureAlbum")
            }
            return handle
        }

        internal var runtimeObjectIsDisposed: Bool { released }
        internal func disposeFromParent() throws { try Dispose() }
    }
}
