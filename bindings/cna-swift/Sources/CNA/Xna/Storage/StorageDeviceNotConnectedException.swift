// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Storage {
    /// `Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.Storage.dll` as a public **unsealed** class whose direct base
    /// is `System.Runtime.InteropServices.ExternalException`, and it declares **nothing but constructors**: every
    /// usable member — `Message`, `InnerException`, `HResult` and `ErrorCode` — is
    /// inherited, which is why the base is a real Swift superclass rather than
    /// a dropped one.
    ///
    /// The other XNA exception type the CLR leaves **unsealed**, so it is
    /// `open`. Like `ContentLoadException` it declares a fourth,
    /// `protected`, `(SerializationInfo, StreamingContext)` constructor. Its
    /// body is only a base forward, now reproduced over the admitted carriers;
    /// Swift widens it to public because the language has no `protected`.
    ///
    /// `StorageDevice` is not implemented, so nothing throws it yet.
    ///
    /// Every constructor body in the IL is a bare forward to the base:
    /// `ldarg.0`, the arguments, `call base..ctor`, `ret`. No message is
    /// synthesized here, so whatever `CNAExternalException` produces for a null message
    /// is what this type reports.
    ///
    /// CLR non-sealed maps to Swift `open`.
    open class StorageDeviceNotConnectedException: CNAExternalException {
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
