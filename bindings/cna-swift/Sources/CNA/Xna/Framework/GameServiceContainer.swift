// SPDX-License-Identifier: MIT

import Foundation

extension Microsoft.Xna.Framework {
    /// `Microsoft.Xna.Framework.GameServiceContainer`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.Game.dll` as a public **unsealed**
    /// class on `System.Object` implementing `System.IServiceProvider`, with
    /// one constructor and three methods. Its whole state is one
    /// `Dictionary<System.Type, object>`, which is why it could not be
    /// implemented until both halves of that generic argument had a decided
    /// projection.
    ///
    /// ## `System.Type` is the Swift metatype
    ///
    /// The pinned contract names `System.Type` in twenty-four positions and
    /// **calls a member on it in none of them** — every one passes it,
    /// compares it or returns it. The selected surface of the family is
    /// therefore identity and assignability, and a Swift metatype has both:
    /// `==` decides identity, and `_openExistential` decides
    /// `Type.IsAssignableFrom` exactly, including protocol conformance and
    /// class inheritance.
    ///
    /// What a Swift metatype does not have is `System.Type`'s reflection
    /// surface — `Name`, `FullName`, `Assembly`, `GetMethods`. Nothing in the
    /// contract needs one, and the one message below that does is the one
    /// thing here that cannot be exact. A wrapper class would have had to
    /// either invent those members or be an empty box; neither is better than
    /// the language type. See `clrTypeLanguageProjection`.
    ///
    /// CLR non-sealed maps to Swift `open`.
    open class GameServiceContainer: CNAServiceProvider {
        /// `Dictionary<Type, object> services`.
        ///
        /// The CLR uses the default comparer, whose behaviour for a runtime
        /// `Type` is reference identity. A metatype is not `Hashable`, so an
        /// explicit comparer supplies exactly that: `==` for equality and
        /// `ObjectIdentifier` for the hash. It is a private field, so the
        /// comparer is invisible — only the identity rule it implements is
        /// observable, and that rule is the CLR's.
        private let services = CNADictionary<Any.Type, Any>(
            comparer: MetatypeComparer())

        private struct MetatypeComparer: CNAEqualityComparer {
            func Equals(_ x: Any.Type, _ y: Any.Type) -> Bool { x == y }
            func GetHashCode(_ obj: Any.Type) -> Int32 {
                Int32(truncatingIfNeeded: ObjectIdentifier(obj).hashValue)
            }
        }

        /// `.ctor()` — allocates the dictionary and nothing else.
        public init() {}

        /// `AddService(Type type, object provider)`.
        ///
        /// The IL order is exact and every step is observable:
        ///
        /// ```text
        /// type == null      -> ArgumentNullException("type",     ServiceTypeCannotBeNull)
        /// provider == null  -> ArgumentNullException("provider", ServiceProviderCannotBeNull)
        /// services.ContainsKey(type)
        ///                   -> ArgumentException(ServiceAlreadyPresent, "type")
        /// !type.IsAssignableFrom(provider.GetType())
        ///                   -> ArgumentException(ServiceMustBeAssignable formatted)
        /// services.Add(type, provider)
        /// ```
        ///
        /// The null-type branch is unreachable through a non-Optional Swift
        /// metatype; the null-provider branch is **not**, because
        /// `System.Object` projects to `Any?`, so it is reproduced.
        ///
        /// The assignability check is the interesting one, and it is exact: a
        /// provider registered under an interface type is what every real use
        /// of this container does, and identity alone would have refused it.
        ///
        /// One divergence, in the message and not in the behaviour. XNA
        /// formats `ServiceMustBeAssignable` with `provider.GetType().FullName`
        /// and `type.GetType().FullName` — note the second: `Type.GetType()`
        /// is `Object.GetType()`, so XNA's second argument is always
        /// `"System.RuntimeType"` rather than the service type's name, which
        /// is a defect in XNA. Neither name is reconstructible here, because
        /// `FullName` is not part of the `Any.Type` projection. The template
        /// is reproduced verbatim and both arguments carry the Swift type
        /// names, which is what the sentence plainly intends; XNA's own second
        /// argument is deliberately **not** reproduced, because hardcoding a
        /// CLR internal type name into a Swift message would be meaningless.
        public func AddService(_ type: Any.Type, provider: Any?) throws {
            guard let provider else {
                // `ArgumentNullException::.ctor(string paramName, string
                // message)` -- the transposed two-argument overload, so the
                // resource string is the MESSAGE and "provider" is the
                // parameter name. Message therefore composes both around
                // Environment.NewLine, which the admitted assembly declares as
                // the IL literal "\r\n".
                throw CNAArgumentNullException(
                    paramName: "provider",
                    message: GameServiceContainer.serviceProviderCannotBeNullMessage)
            }
            guard !services.ContainsKey(type) else {
                // `ArgumentException::.ctor(string message, string paramName)`
                // -- message first, then "type".
                throw CNAArgumentException(
                    message: "Container already contains a service of this type.",
                    paramName: "type")
            }
            guard GameServiceContainer.isInstance(provider, of: type) else {
                throw CNAArgumentException(
                    message:
                    GameServiceContainer.serviceMustBeAssignableFormat
                        .replacingOccurrences(
                            of: "{0}", with: String(reflecting: Swift.type(of: provider)))
                        .replacingOccurrences(
                            of: "{1}", with: String(reflecting: type)))
            }
            try services.Add(type, value: provider)
        }

        /// `RemoveService(Type type)`.
        ///
        /// A null type is the only failure, and it is unreachable here.
        /// Removing a type the container does not hold is a no-op: the IL
        /// discards `Dictionary.Remove`'s result.
        public func RemoveService(_ type: Any.Type) {
            services.Remove(type)
        }

        /// `GetService(Type type)` — the `IServiceProvider` implementation.
        ///
        /// `ContainsKey` first, then the indexer; an absent type returns
        /// **null** rather than raising, which is why the projection is
        /// Optional. It is `virtual final` in the metadata — a sealed
        /// interface implementation — so it is `final` here.
        public final func GetService(_ type: Any.Type) -> Any? {
            var found: Any?
            _ = services.TryGetValue(type, value: &found)
            return found
        }

        /// The exact `ServiceProviderCannotBeNull` message, read out of
        /// `Microsoft.Xna.Framework.Game.dll`'s own resource table.
        internal static let serviceProviderCannotBeNullMessage =
            "The service provider instance cannot be null."

        /// The exact `ServiceMustBeAssignable` template, read out of
        /// `Microsoft.Xna.Framework.Game.dll`'s own resource table.
        private static let serviceMustBeAssignableFormat =
            "Service provider object of type {0} must be assignable to "
            + "service type {1}."

        /// `Type.IsAssignableFrom(value.GetType())`.
        ///
        /// Opening the existential metatype and asking `value is T` is the
        /// Swift mechanism for a dynamic type test against a value-carried
        /// type, and it answers exactly what the CLR asks: true for the type
        /// itself, for any base class, and for any protocol the value
        /// conforms to.
        private static func isInstance(_ value: Any, of type: Any.Type) -> Bool {
            func test<T>(_: T.Type) -> Bool { value is T }
            return _openExistential(type, do: test)
        }
    }
}
