// SPDX-License-Identifier: MIT

import Foundation

extension Microsoft.Xna.Framework.Content {
    /// `Microsoft.Xna.Framework.Content.ContentTypeReaderManager`.
    ///
    /// The pinned Windows XNA 4.0 metadata exposes exactly one public member:
    /// `GetTypeReader(Type)`. It exposes no `AddTypeCreator`; Swift factories
    /// below are intentionally internal infrastructure replacing CLR
    /// reflection, not invented XNA API.
    public final class ContentTypeReaderManager {
        internal typealias Creator = () throws -> ContentTypeReader

        // CLR Monitor is recursive. Reader Initialize hooks can ask this
        // manager for another reader while the manifest lock is held.
        private static let lock = NSRecursiveLock()
        private static var creators: [String: Creator] = [:]
        private static var readersByName: [String: ContentTypeReader] = [:]
        private static var readersByTarget: [ObjectIdentifier: ContentTypeReader] = [:]

        private weak var contentReader: ContentReader?

        internal init(contentReader: ContentReader?) {
            self.contentReader = contentReader
        }

        public func GetTypeReader(_ targetType: Any.Type) throws -> ContentTypeReader {
            Self.lock.lock()
            defer { Self.lock.unlock() }
            if let reader = Self.readersByTarget[ObjectIdentifier(targetType)] {
                return reader
            }
            throw contentReader?.contentLoadException(
                Self.typeReaderNotRegistered,
                Self.typeName(targetType))
                ?? ContentLoadException(message: Self.format(
                    Self.typeReaderNotRegistered, "", Self.typeName(targetType)))
        }

        internal static func registerTypeCreator(
            _ readerTypeName: String, creator: @escaping Creator
        ) throws {
            lock.lock()
            defer { lock.unlock() }
            guard creators[readerTypeName] == nil,
                  readersByName[readerTypeName] == nil else {
                throw CNAArgumentException(
                    message: "A ContentTypeReader creator is already registered for \(readerTypeName).")
            }
            creators[readerTypeName] = creator
        }

        internal static func unregisterTypeCreator(_ readerTypeName: String) {
            lock.lock()
            defer { lock.unlock() }
            creators.removeValue(forKey: readerTypeName)
            if let reader = readersByName.removeValue(forKey: readerTypeName) {
                let key = ObjectIdentifier(reader.TargetType)
                if readersByTarget[key] === reader { readersByTarget.removeValue(forKey: key) }
            }
        }

        internal static func readTypeManifest(
            count: Int32, input: ContentReader
        ) throws -> [ContentTypeReader] {
            var result: [ContentTypeReader] = []
            result.reserveCapacity(Int(count))
            var newReaders: [ContentTypeReader] = []

            lock.lock()
            defer { lock.unlock() }
            do {
                for _ in 0..<count {
                    let name = try input.ReadString()
                    let version = try input.ReadInt32()
                    let reader: ContentTypeReader
                    if let cached = readersByName[name] {
                        reader = cached
                    } else {
                        guard let creator = creators[name] else {
                            throw input.contentLoadException(typeReaderNotFound, name)
                        }
                        do {
                            reader = try creator()
                        } catch let error as CNAException {
                            throw ContentLoadException(
                                message: input.contentLoadMessage(typeReaderInvalid, name),
                                innerException: error)
                        }
                        let targetKey = ObjectIdentifier(reader.TargetType)
                        if let previous = readersByTarget[targetKey], previous !== reader {
                            throw input.contentLoadException(
                                typeReaderDuplicate,
                                name, typeName(type(of: previous)), typeName(reader.TargetType))
                        }
                        readersByName[name] = reader
                        readersByTarget[targetKey] = reader
                        newReaders.append(reader)
                    }
                    guard version == reader.TypeVersion else {
                        throw input.contentLoadException(
                            badXnbTypeVersion, typeName(reader.TargetType))
                    }
                    result.append(reader)
                }
                if !newReaders.isEmpty {
                    let manager = ContentTypeReaderManager(contentReader: input)
                    for reader in newReaders { try reader.Initialize(manager) }
                }
                return result
            } catch {
                for reader in newReaders {
                    readersByName = readersByName.filter { $0.value !== reader }
                    let key = ObjectIdentifier(reader.TargetType)
                    if readersByTarget[key] === reader { readersByTarget.removeValue(forKey: key) }
                }
                throw error
            }
        }

        private static func typeName(_ type: Any.Type) -> String {
            Microsoft.Xna.Framework.Graphics.GraphicsResource.clrTypeName(ofType: type)
        }

        private static func format(_ template: String, _ values: String...) -> String {
            values.enumerated().reduce(template) {
                $0.replacingOccurrences(of: "{\($1.offset)}", with: $1.element)
            }
        }

        internal static let typeReaderNotRegistered =
            "Error loading \"{0}\". Cannot find ContentTypeReader for {1}."
        internal static let typeReaderNotFound =
            "Error loading \"{0}\". Cannot find ContentTypeReader {1}."
        internal static let typeReaderInvalid =
            "Error loading \"{0}\". Cannot instantiate ContentTypeReader {1}."
        internal static let typeReaderDuplicate =
            "Error loading \"{0}\". ContentTypeReader {1} conflicts with existing handler {2} for type {3}."
        internal static let badXnbTypeVersion =
            "Error loading \"{0}\". File contains the wrong version of type {1}."
    }
}
