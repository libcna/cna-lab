// SPDX-License-Identifier: MIT

internal final class AudioCategoryStorage {
    let native: NativeHandleStorage
    let name: String
    weak var parent: Microsoft.Xna.Framework.Audio.AudioEngine?

    init(
        handle: UInt64, name: String,
        parent: Microsoft.Xna.Framework.Audio.AudioEngine,
        runtime: RuntimeState
    ) {
        self.name = name
        self.parent = parent
        native = NativeHandleStorage(
            handle: handle, typeName: "AudioCategory", ownership: .owned,
            runtime: runtime, destroy: runtime.functions.audioCategoryDestroy)
    }

    func dispose() throws {
        try native.dispose(operation: "cna_audio_category_destroy")
    }
}

extension Microsoft.Xna.Framework.Audio {
    public struct AudioCategory: Equatable {
        private let storage: AudioCategoryStorage?

        internal init(storage: AudioCategoryStorage) { self.storage = storage }

        public var Name: String { storage?.name ?? "" }

        public func SetVolume(_ volume: Float) throws {
            guard !(volume < 0) else {
                throw CNAArgumentException(
                    message: "Volume must be a positive float value.")
            }
            try call("AudioCategory.SetVolume") { functions, handle in
                functions.audioCategorySetVolume(handle, volume)
            }
        }

        public func Pause() throws {
            try call("AudioCategory.Pause") { $0.audioCategoryPause($1) }
        }

        public func Resume() throws {
            try call("AudioCategory.Resume") { $0.audioCategoryResume($1) }
        }

        public func Stop(_ options: AudioStopOptions) throws {
            try call("AudioCategory.Stop") {
                $0.audioCategoryStop($1, UInt32(bitPattern: options.rawValue))
            }
        }

        public func ToString() -> String? { Name }
        public func Equals(_ other: AudioCategory) -> Bool { self == other }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? AudioCategory else { return false }
            return self == other
        }

        public func GetHashCode() -> Int32 {
            guard let storage else { return 0 }
            var hash = Hasher()
            if let parent = storage.parent {
                hash.combine(ObjectIdentifier(parent))
            }
            hash.combine(storage.name)
            return Int32(truncatingIfNeeded: hash.finalize())
        }

        public static func == (lhs: AudioCategory, rhs: AudioCategory) -> Bool {
            guard let left = lhs.storage, let right = rhs.storage else {
                return lhs.storage == nil && rhs.storage == nil
            }
            return left.parent === right.parent && left.name == right.name
        }

        public static func != (lhs: AudioCategory, rhs: AudioCategory) -> Bool {
            !(lhs == rhs)
        }

        private func call(
            _ operation: String,
            _ body: (NativeFunctions, UInt64) -> UInt32
        ) throws {
            guard let storage else {
                throw CNAInvalidOperationException()
            }
            let handle = try storage.native.validatedHandle(operation)
            let functions = storage.native.runtime.functions
            try functions.check(body(functions, handle), operation: operation)
        }
    }
}
