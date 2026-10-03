// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Content {
    /// `Microsoft.Xna.Framework.Content.ContentTypeReader`.
    ///
    /// XNA declares this class abstract. Swift has no abstract classes, so the
    /// two protected hooks are `open`; the default `Read` body refuses use of
    /// the otherwise constructible base instead of fabricating an object.
    open class ContentTypeReader {
        private let targetType: Any.Type
        internal let TargetIsValueType: Bool

        /// Widened from protected so a Swift subclass can call it.
        public init(targetType: Any.Type) {
            self.targetType = targetType
            TargetIsValueType = !(targetType is AnyClass)
                && targetType != String.self
                && targetType != Any.self
        }

        public var TargetType: Any.Type { targetType }
        open var TypeVersion: Int32 { 0 }
        open var CanDeserializeIntoExistingObject: Bool { false }

        /// Widened from protected-internal.
        open func Initialize(_ manager: ContentTypeReaderManager) throws {}

        /// The erased abstract bridge used by `ContentReader`.
        open func Read(
            _ input: ContentReader, existingInstance: Any?
        ) throws -> Any? {
            throw CNANotSupportedException(
                message: "ContentTypeReader.Read must be implemented by a derived reader.")
        }
    }

    /// `Microsoft.Xna.Framework.Content.ContentTypeReader<T>`.
    ///
    /// The generic parameter remains part of the public Swift class. Only the
    /// dispatch bridge erases it, privately through the non-generic override.
    open class ContentTypeReaderOfT<T>: ContentTypeReader {
        /// Widened from protected, matching the base-class construction rule.
        public init() {
            super.init(targetType: T.self)
        }

        /// XNA's typed abstract hook. Optional existing state represents
        /// CLR `default(T)` for reference types.
        open func Read(
            _ input: ContentReader, existingInstance: T?
        ) throws -> T {
            throw CNANotSupportedException(
                message: "ContentTypeReader<T>.Read must be implemented by a derived reader.")
        }

        /// Disambiguates the typed virtual from the erased base overload.
        /// Swift otherwise prefers the `Any?` overload for some generic `T`
        /// instantiations and recursively re-enters the bridge below.
        internal func readTyped(
            _ input: ContentReader, existingInstance: T?
        ) throws -> T {
            let implementation: (ContentReader, T?) throws -> T = self.Read
            return try implementation(input, existingInstance)
        }

        /// The actual XNA erased bridge: nil becomes default(T), a wrong
        /// existing value reports BadXnbWrongType, and the typed hook receives
        /// the surviving value.
        open override func Read(
            _ input: ContentReader, existingInstance: Any?
        ) throws -> Any? {
            let typed: T?
            if let existingInstance {
                guard let converted = existingInstance as? T else {
                    throw input.wrongExistingInstance(
                        expected: T.self, actual: type(of: existingInstance))
                }
                typed = converted
            } else {
                typed = nil
            }
            return try readTyped(input, existingInstance: typed)
        }
    }
}
