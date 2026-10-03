// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Audio {
    /// `Microsoft.Xna.Framework.Audio.InstancePlayLimitException`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.dll` as a public **sealed** class whose direct base
    /// is `System.Runtime.InteropServices.ExternalException`, and it declares **nothing but constructors**: every
    /// usable member — `Message`, `InnerException`, `HResult` and `ErrorCode` — is
    /// inherited, which is why the base is a real Swift superclass rather than
    /// a dropped one.
    ///
    /// XNA raises it when a `SoundEffect` has already reached its
    /// instance play limit. Nothing in this binding can produce that
    /// condition yet — `SoundEffect` is not implemented — so the type is
    /// complete while its runtime throw site is not.
    ///
    /// Every constructor body in the IL is a bare forward to the base:
    /// `ldarg.0`, the arguments, `call base..ctor`, `ret`. No message is
    /// synthesized here, so whatever `CNAExternalException` produces for a null message
    /// is what this type reports.
    ///
    /// CLR `sealed` maps to Swift `final`.
    public final class InstancePlayLimitException: CNAExternalException {
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

        /// `.ctor(String message, Exception inner)`.
        ///
        /// The CLR names this parameter `inner`, and a class constructor's
        /// Swift labels are its CLR metadata names, so this is a NEW
        /// designated initializer rather than an override of the base's
        /// `innerException:` one. Declaring any designated initializer stops
        /// Swift inheriting the base's, which is why the two above are
        /// explicit overrides: the projected surface is then exactly the three
        /// constructors this type declares in the IL, with no inherited fourth.
        public init(message: String?, inner: CNAException?) {
            super.init(message: message, innerException: inner)
        }
    }
}
