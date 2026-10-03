// SPDX-License-Identifier: MIT

import Foundation

extension Microsoft.Xna.Framework.Content {
    /// `Microsoft.Xna.Framework.Content.ContentReader`.
    ///
    /// Bytes remain owned by a real `InputStream` and are consumed through the
    /// projected `System.IO.BinaryReader` superclass. The managed XNB path does
    /// not ask CNA to deserialize an asset before constructing this reader.
    public final class ContentReader: CNABinaryReader {
        private let contentManager: ContentManager
        private let assetName: String
        private let recordDisposableObject: ((any CNADisposable) throws -> Void)?
        private var typeReaders: [ContentTypeReader] = []
        private var sharedResourceFixups: [[(Any?) throws -> Void]] = []

        internal let graphicsProfile: Int32

        internal init(
            contentManager: ContentManager,
            input: InputStream,
            assetName: String,
            recordDisposableObject: ((any CNADisposable) throws -> Void)?,
            graphicsProfile: Int32
        ) throws {
            self.contentManager = contentManager
            self.assetName = assetName
            self.recordDisposableObject = recordDisposableObject
            self.graphicsProfile = graphicsProfile
            try super.init(input)
        }

        public var ContentManager: ContentManager { contentManager }
        public var AssetName: String { assetName }

        internal static func Create(
            _ contentManager: ContentManager,
            input: InputStream,
            assetName: String,
            recordDisposableObject: ((any CNADisposable) throws -> Void)?,
            availableLength: Int32?
        ) throws -> ContentReader {
            let header = try prepareStream(
                input, assetName: assetName, availableLength: availableLength)
            return try ContentReader(
                contentManager: contentManager,
                input: input,
                assetName: assetName,
                recordDisposableObject: recordDisposableObject,
                graphicsProfile: header.graphicsProfile)
        }

        internal func readAsset<T>() throws -> T? {
            do {
                let sharedResourceCount = try readHeader()
                let asset: T? = try ReadObject()
                try readSharedResources(sharedResourceCount)
                return asset
            } catch let error as ContentLoadException {
                throw error
            } catch let error as CNAIOException {
                throw ContentLoadException(
                    message: contentLoadMessage(Self.badXnb),
                    innerException: error)
            }
        }

        private static func prepareStream(
            _ input: InputStream, assetName: String, availableLength: Int32?
        ) throws -> (graphicsProfile: Int32, declaredSize: Int32) {
            let header: CNABinaryReader
            do {
                header = try CNABinaryReader(input)
                guard try header.ReadByte() == 88,
                      try header.ReadByte() == 78,
                      try header.ReadByte() == 66 else {
                    throw ContentLoadException(message: format(badXnbMagic, assetName))
                }
                guard try header.ReadByte() == 119 else {
                    throw ContentLoadException(message: format(badXnbPlatform, assetName))
                }
                let rawVersion = try header.ReadUInt16()
                let profile = Int32((rawVersion & 0x7f00) >> 8)
                let version = rawVersion & 0x80ff
                guard version == 5 || version == 0x8005 else {
                    throw ContentLoadException(message: format(badXnbVersion, assetName))
                }
                let declaredSize = try header.ReadInt32()
                if let availableLength, declaredSize > availableLength {
                    throw ContentLoadException(message: format(badXnbSize, assetName))
                }
                guard version == 5 else {
                    // XNA 4.0's compressed Windows form is LZX. No decompressor
                    // is admitted by this selected closure, so this is an
                    // explicit unsupported-format refusal rather than a claim
                    // that the compressed bytes are an ordinary XNB body.
                    throw ContentLoadException(message: decompressionError)
                }
                return (profile, declaredSize)
            } catch let error as ContentLoadException {
                throw error
            } catch let error as CNAIOException {
                throw ContentLoadException(
                    message: format(badXnb, assetName), innerException: error)
            }
        }

        private func readHeader() throws -> Int32 {
            let count = try Read7BitEncodedInt()
            guard count >= 0 else { throw contentLoadException(Self.badXnb) }
            typeReaders = try ContentTypeReaderManager.readTypeManifest(
                count: count, input: self)
            let sharedCount = try Read7BitEncodedInt()
            guard sharedCount >= 0 else { throw contentLoadException(Self.badXnb) }
            sharedResourceFixups = Array(
                repeating: [], count: Int(sharedCount))
            return sharedCount
        }

        public func ReadObject<T>() throws -> T? {
            try readObjectInternal(existingInstance: nil)
        }

        public func ReadObject<T>(_ existingInstance: T?) throws -> T? {
            try readObjectInternal(existingInstance: existingInstance)
        }

        public func ReadObject<T>(_ typeReader: ContentTypeReader) throws -> T? {
            try readObjectInternal(typeReader, existingInstance: nil)
        }

        public func ReadObject<T>(
            _ typeReader: ContentTypeReader, existingInstance: T?
        ) throws -> T? {
            try readObjectInternal(typeReader, existingInstance: existingInstance)
        }

        public func ReadRawObject<T>() throws -> T? {
            let manager = ContentTypeReaderManager(contentReader: self)
            return try invokeReader(
                manager.GetTypeReader(T.self), existingInstance: nil)
        }

        public func ReadRawObject<T>(_ existingInstance: T?) throws -> T? {
            let manager = ContentTypeReaderManager(contentReader: self)
            return try invokeReader(
                manager.GetTypeReader(T.self), existingInstance: existingInstance)
        }

        public func ReadRawObject<T>(
            _ typeReader: ContentTypeReader
        ) throws -> T? {
            try invokeReader(typeReader, existingInstance: nil)
        }

        public func ReadRawObject<T>(
            _ typeReader: ContentTypeReader, existingInstance: T?
        ) throws -> T? {
            try invokeReader(typeReader, existingInstance: existingInstance)
        }

        private func readObjectInternal<T>(existingInstance: T?) throws -> T? {
            let encodedIndex = try Read7BitEncodedInt()
            if encodedIndex == 0 { return nil }
            let index = encodedIndex - 1
            guard index >= 0, Int(index) < typeReaders.count else {
                throw contentLoadException(Self.badXnb)
            }
            return try invokeReader(
                typeReaders[Int(index)], existingInstance: existingInstance)
        }

        private func readObjectInternal<T>(
            _ typeReader: ContentTypeReader, existingInstance: T?
        ) throws -> T? {
            if typeReader.TargetIsValueType {
                return try invokeReader(
                    typeReader, existingInstance: existingInstance)
            }
            return try readObjectInternal(existingInstance: existingInstance)
        }

        private func invokeReader<T>(
            _ reader: ContentTypeReader, existingInstance: T?
        ) throws -> T? {
            let value: Any?
            if let typedReader = reader as? ContentTypeReaderOfT<T> {
                value = try typedReader.readTyped(
                    self, existingInstance: existingInstance)
            } else {
                value = try reader.Read(
                    self, existingInstance: existingInstance)
            }

            let typed: T?
            if let value {
                guard let converted = value as? T else {
                    throw contentLoadException(
                        Self.badXnbWrongType,
                        Self.typeName(type(of: value)), Self.typeName(T.self))
                }
                typed = converted
            } else {
                typed = nil
            }

            if let existingInstance {
                guard Self.sameReference(existingInstance, typed) else {
                    throw CNAInvalidOperationException(
                        message: Self.format(
                            Self.readerConstructedNewInstance,
                            Self.typeName(type(of: reader))))
                }
            } else if !reader.TargetIsValueType,
                      let disposable = typed as? any CNADisposable {
                if let recordDisposableObject {
                    try recordDisposableObject(disposable)
                } else {
                    try contentManager.RecordDisposableObject(disposable)
                }
            }
            return typed
        }

        public func ReadSharedResource<T>(
            _ fixup: ((T) throws -> Void)?
        ) throws {
            guard let fixup else {
                throw CNAArgumentNullException(paramName: "fixup")
            }
            let encodedIndex = try Read7BitEncodedInt()
            if encodedIndex == 0 { return }
            let index = encodedIndex - 1
            guard index >= 0, Int(index) < sharedResourceFixups.count else {
                throw contentLoadException(Self.badXnb)
            }
            sharedResourceFixups[Int(index)].append { value in
                guard let typed = value as? T else {
                    throw self.contentLoadException(Self.badXnb)
                }
                try fixup(typed)
            }
        }

        private func readSharedResources(_ count: Int32) throws {
            guard count > 0 else { return }
            var resources: [Any?] = []
            resources.reserveCapacity(Int(count))
            for _ in 0..<count {
                resources.append(try readObjectErased())
            }
            for index in resources.indices {
                for fixup in sharedResourceFixups[index] {
                    try fixup(resources[index])
                }
            }
        }

        private func readObjectErased() throws -> Any? {
            let encodedIndex = try Read7BitEncodedInt()
            if encodedIndex == 0 { return nil }
            let index = encodedIndex - 1
            guard index >= 0, Int(index) < typeReaders.count else {
                throw contentLoadException(Self.badXnb)
            }
            let reader = typeReaders[Int(index)]
            let value = try reader.Read(self, existingInstance: nil)
            if !reader.TargetIsValueType,
               let disposable = value as? any CNADisposable {
                if let recordDisposableObject {
                    try recordDisposableObject(disposable)
                } else {
                    try contentManager.RecordDisposableObject(disposable)
                }
            }
            return value
        }

        public func ReadExternalReference<T>() throws -> T? {
            let reference = try ReadString()
            guard !reference.isEmpty else { return nil }
            let directory = Self.assetDirectory(assetName)
            let combined = directory.isEmpty ? reference : directory + "\\" + reference
            let clean = Microsoft.Xna.Framework.TitleContainer.GetCleanPath(combined)
            return try contentManager.Load(clean)
        }

        public func ReadVector2() throws -> Microsoft.Xna.Framework.Vector2 {
            Microsoft.Xna.Framework.Vector2(try ReadSingle(), try ReadSingle())
        }

        public func ReadVector3() throws -> Microsoft.Xna.Framework.Vector3 {
            Microsoft.Xna.Framework.Vector3(
                try ReadSingle(), try ReadSingle(), try ReadSingle())
        }

        public func ReadVector4() throws -> Microsoft.Xna.Framework.Vector4 {
            Microsoft.Xna.Framework.Vector4(
                try ReadSingle(), try ReadSingle(), try ReadSingle(), try ReadSingle())
        }

        public func ReadMatrix() throws -> Microsoft.Xna.Framework.Matrix {
            Microsoft.Xna.Framework.Matrix(
                try ReadSingle(), try ReadSingle(), try ReadSingle(), try ReadSingle(),
                try ReadSingle(), try ReadSingle(), try ReadSingle(), try ReadSingle(),
                try ReadSingle(), try ReadSingle(), try ReadSingle(), try ReadSingle(),
                try ReadSingle(), try ReadSingle(), try ReadSingle(), try ReadSingle())
        }

        public func ReadQuaternion() throws -> Microsoft.Xna.Framework.Quaternion {
            Microsoft.Xna.Framework.Quaternion(
                try ReadSingle(), try ReadSingle(), try ReadSingle(), try ReadSingle())
        }

        public func ReadColor() throws -> Microsoft.Xna.Framework.Color {
            Microsoft.Xna.Framework.Color(packedValue: try ReadUInt32())
        }

        public override func ReadSingle() throws -> Float {
            Float(bitPattern: try ReadUInt32())
        }

        public override func ReadDouble() throws -> Double {
            Double(bitPattern: try ReadUInt64())
        }

        internal func wrongExistingInstance(
            expected: Any.Type, actual: Any.Type
        ) -> ContentLoadException {
            contentLoadException(
                Self.badXnbWrongType,
                Self.typeName(actual), Self.typeName(expected))
        }

        internal func contentLoadException(
            _ template: String, _ values: String...
        ) -> ContentLoadException {
            ContentLoadException(
                message: Self.format(template, [assetName] + values))
        }

        internal func contentLoadMessage(
            _ template: String, _ values: String...
        ) -> String {
            Self.format(template, [assetName] + values)
        }

        private static func format(
            _ template: String, _ values: String...
        ) -> String {
            format(template, values)
        }

        private static func format(_ template: String, _ values: [String]) -> String {
            values.enumerated().reduce(template) { result, item in
                result.replacingOccurrences(
                    of: "{\(item.offset)}", with: item.element)
            }
        }

        private static func typeName(_ type: Any.Type) -> String {
            Microsoft.Xna.Framework.Graphics.GraphicsResource.clrTypeName(ofType: type)
        }

        private static func sameReference<T>(_ lhs: T, _ rhs: T?) -> Bool {
            guard let lhsObject = lhs as AnyObject?,
                  let rhsObject = rhs as AnyObject? else { return false }
            return lhsObject === rhsObject
        }

        private static func assetDirectory(_ name: String) -> String {
            let units = Array(name)
            guard let separator = units.lastIndex(where: { $0 == "\\" || $0 == "/" })
            else { return "" }
            return String(units[..<separator])
        }

        internal static let badXnb = "Error loading \"{0}\"."
        internal static let badXnbMagic =
            "Error loading \"{0}\". This is not a compiled content file."
        internal static let badXnbPlatform =
            "Error loading \"{0}\". This file was compiled for the wrong target platform."
        internal static let badXnbSize =
            "Error loading \"{0}\". File has been truncated."
        internal static let badXnbVersion =
            "Error loading \"{0}\". This file was compiled using the wrong version of the XNA Framework."
        internal static let badXnbWrongType =
            "Error loading \"{0}\". File contains {1} but trying to load as {2}."
        internal static let readerConstructedNewInstance =
            "ContentTypeReader {0} returned a new object instance from its Read method. "
            + "This should have loaded data into the existingInstance parameter."
        internal static let decompressionError = "Error decompressing content data."
    }
}
