// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Content {
    /// `Microsoft.Xna.Framework.Content.ContentLoadException`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.dll` as a public **unsealed** class whose direct base
    /// is `System.Exception`, and it declares **nothing but constructors**: every
    /// usable member — `Message`, `InnerException`, `HResult` — is
    /// inherited, which is why the base is a real Swift superclass rather than
    /// a dropped one.
    ///
    /// One of the two XNA exception types the CLR leaves **unsealed**, so
    /// it is `open` rather than `final` and a consumer may derive from it.
    ///
    /// Its fourth constructor takes the admitted serialization carriers and
    /// forwards to the base exactly as the XNA IL does. Swift has no
    /// `protected`, so it is public here as a measured visibility widening.
    ///
    /// Every constructor body in the IL is a bare forward to the base:
    /// `ldarg.0`, the arguments, `call base..ctor`, `ret`. No message is
    /// synthesized here, so whatever `CNAException` produces for a null message
    /// is what this type reports.
    ///
    /// CLR non-sealed maps to Swift `open`.
    open class ContentLoadException: CNAException {
        /// `.ctor()`.
        public override init() {
            super.init()
        }

        /// `.ctor(String message)`.
        ///
        /// The CLR stores the argument without validating it, so `nil` is a
        /// normal state and selects the base's default message.
        public override init(message: String?) {
            super.init(message: message)
        }

        /// `.ctor(String message, Exception innerException)`.
        public override init(message: String?, innerException: CNAException?) {
            super.init(message: message, innerException: innerException)
        }

        /// `protected .ctor(SerializationInfo info, StreamingContext context)`.
        public override init(
            info: CNASerializationInfo, context: CNAStreamingContext
        ) {
            super.init(info: info, context: context)
        }
    }
}
