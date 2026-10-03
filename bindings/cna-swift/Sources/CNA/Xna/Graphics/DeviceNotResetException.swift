// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// `Microsoft.Xna.Framework.Graphics.DeviceNotResetException`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.Graphics.dll` as a public **sealed** class whose direct base
    /// is `System.Exception`, and it declares **nothing but constructors**: every
    /// usable member — `Message`, `InnerException`, `HResult` — is
    /// inherited, which is why the base is a real Swift superclass rather than
    /// a dropped one.
    ///
    /// XNA raises it when the device is available again but has not been
    /// reset. It shares its throw-site blocker with
    /// `DeviceLostException`.
    ///
    /// Every constructor body in the IL is a bare forward to the base:
    /// `ldarg.0`, the arguments, `call base..ctor`, `ret`. No message is
    /// synthesized here, so whatever `CNAException` produces for a null message
    /// is what this type reports.
    ///
    /// CLR `sealed` maps to Swift `final`.
    public final class DeviceNotResetException: CNAException {
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
