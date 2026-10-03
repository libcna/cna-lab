// SPDX-License-Identifier: MIT

import Foundation

extension Microsoft.Xna.Framework.Content {
    /// `Microsoft.Xna.Framework.Content.ResourceContentManager` implemented
    /// over the projected ResourceManager virtual lookup, not CNA's similarly
    /// named placeholder route.
    open class ResourceContentManager: ContentManager {
        private var resourceManager: CNAResourceManager?

        public init(
            serviceProvider: any CNAServiceProvider,
            resourceManager: CNAResourceManager?
        ) throws {
            self.resourceManager = nil
            try super.init(serviceProvider: serviceProvider, rootDirectory: "")
            guard let resourceManager else {
                throw CNAArgumentNullException(paramName: "resourceManager")
            }
            self.resourceManager = resourceManager
        }

        open override func OpenStream(_ assetName: String) throws -> InputStream {
            guard let resourceManager else {
                throw CNAObjectDisposedException(objectName: "\(type(of: self))")
            }
            guard let value = try resourceManager.GetObject(assetName) else {
                throw ContentLoadException(message: Self.format(
                    Self.openResourceNotFound, assetName))
            }
            guard let bytes = value as? [UInt8] else {
                throw ContentLoadException(message: Self.format(
                    Self.openResourceNotBinary, assetName))
            }
            let stream = InputStream(data: Data(bytes))
            RecordOpenedStreamLength(stream, UInt64(bytes.count))
            return stream
        }

        private static func format(_ template: String, _ value: String) -> String {
            template.replacingOccurrences(of: "{0}", with: value)
        }

        internal static let openResourceNotFound =
            "Error loading \"{0}\". Resource not found."
        internal static let openResourceNotBinary =
            "Error loading \"{0}\". Not a binary resource."
    }
}
