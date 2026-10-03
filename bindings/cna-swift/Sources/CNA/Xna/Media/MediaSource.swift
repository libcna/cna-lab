// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Media {

    /// `Microsoft.Xna.Framework.Media.MediaSource`.
    ///
    /// One place media can come from -- the device itself, or something
    /// attached to it. XNA hands these out only through the static
    /// `GetAvailableMediaSources`, and so does this: there is no public
    /// constructor.
    ///
    /// **It carries an index rather than a handle**, because CNA publishes no
    /// media-source object. Every route is `_at(game, index)`, so a source is
    /// its position in the runtime's enumeration -- and the name and type are
    /// read once when the source is made, not on every access, since the
    /// enumeration a later call sees may not be the one this index came from.
    public final class MediaSource {

        private let storedName: String
        private let storedType: MediaSourceType

        internal init(name: String, type: MediaSourceType, index: UInt32) {
            storedName = name
            storedType = type
            enumerationIndex = index
        }

        /// `MediaSource.Name`.
        public var Name: String { storedName }

        /// `MediaSource.MediaSourceType`.
        public var MediaSourceType: Microsoft.Xna.Framework.Media.MediaSourceType {
            storedType
        }

        /// `MediaSource.ToString()`, which XNA answers with the name.
        public func ToString() -> String { storedName }

        /// `MediaSource.GetAvailableMediaSources()`.
        ///
        /// The list is built by walking the runtime's enumeration once. XNA
        /// answers `IList<MediaSource>`, which this binding projects as
        /// `CNAList`.
        public static func GetAvailableMediaSources() throws -> CNAList<MediaSource> {
            let rt = try RuntimeRegistry.current()
            var count: UInt32 = 0
            try rt.functions.check(
                rt.functions.mediaSourceGetAvailableCount(rt.gameHandle, &count),
                operation: "cna_media_source_get_available_count")

            let list = CNAList<MediaSource>()
            for index in 0..<count {
                var size: UInt64 = 0
                try rt.functions.check(
                    rt.functions.mediaSourceGetNameSizeAt(
                        rt.gameHandle, index, &size),
                    operation: "cna_media_source_get_name_size_at")
                var name = ""
                if size > 0 {
                    var bytes = [CChar](repeating: 0, count: Int(size))
                    var written: UInt64 = 0
                    try rt.functions.check(
                        bytes.withUnsafeMutableBufferPointer { buffer in
                            rt.functions.mediaSourceCopyNameAt(
                                rt.gameHandle, index, buffer.baseAddress,
                                size, &written)
                        },
                        operation: "cna_media_source_copy_name_at")
                    name = String(
                        decoding: bytes.prefix(Int(written)).map { UInt8(bitPattern: $0) },
                        as: UTF8.self)
                }
                var raw: UInt32 = 0
                try rt.functions.check(
                    rt.functions.mediaSourceGetTypeAt(rt.gameHandle, index, &raw),
                    operation: "cna_media_source_get_type_at")
                guard let type = Microsoft.Xna.Framework.Media.MediaSourceType(
                    rawValue: Int32(bitPattern: raw)) else {
                    throw CNAError.nativeFailure(
                        operation: "MediaSource.MediaSourceType", result: 1,
                        message: "native media source type \(raw) is not an XNA MediaSourceType")
                }
                list.Add(MediaSource(name: name, type: type, index: index))
            }
            return list
        }

        /// The index a source occupies in the runtime's enumeration, which is
        /// what `MediaLibrary(MediaSource)` needs and no public member exposes.
        internal let enumerationIndex: UInt32
    }
}
