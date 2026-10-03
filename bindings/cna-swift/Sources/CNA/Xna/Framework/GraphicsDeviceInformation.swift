// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {

    /// `Microsoft.Xna.Framework.GraphicsDeviceInformation`.
    ///
    /// Three fields and no native route at all: it is the carrier
    /// `GraphicsDeviceManager` ranks candidate devices with, and
    /// `PreparingDeviceSettingsEventArgs` hands to a consumer that wants to
    /// change one before the device is created.
    public class GraphicsDeviceInformation {

        private var storedAdapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?
        private var storedProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile = .Reach
        private var storedParameters =
            Microsoft.Xna.Framework.Graphics.PresentationParameters()

        public init() {}

        /// `GraphicsDeviceInformation.Adapter`.
        ///
        /// The getter is seven bytes of `ldfld`. **The setter is thirty-two**,
        /// and the difference is a null check: `set_Adapter` raises
        /// `ArgumentNullException` before storing, which is the only validation
        /// anywhere on this type.
        ///
        /// The setter is therefore **fallible**, and a Swift `set` cannot
        /// throw — so it is projected as a writer method, which is this
        /// project's rule for exactly this shape. The getter stays a property
        /// because `get_Adapter` has no failure path.
        public var Adapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter? {
            storedAdapter
        }

        /// `GraphicsDeviceInformation.set_Adapter(GraphicsAdapter value)`.
        ///
        /// Thirty-two bytes: a null test, `ArgumentNullException`, then the
        /// store. The parameter is Optional here so the refusal is reachable
        /// from Swift at all -- a non-Optional parameter would make the branch
        /// the IL spends most of its length on impossible to exercise.
        public func SetAdapter(
            _ value: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?
        ) throws {
            guard let value else {
                // The IL selects the (paramName, message) overload and names
                // the way out, which is the useful half of the refusal: this
                // property has no null state, and DefaultAdapter is what a
                // caller who has no adapter of their own should pass.
                throw CNAArgumentNullException(
                    paramName: "value",
                    message: GraphicsDeviceInformation.noNullUseDefaultAdapterMessage)
            }
            storedAdapter = value
        }

        /// `FrameworkResources.NoNullUseDefaultAdapter`, read out of the
        /// registered `Microsoft.Xna.Framework.Game.dll`. Two spaces after the
        /// first sentence, as the resource has them.
        internal static let noNullUseDefaultAdapterMessage =
            "Adapter cannot be null.  Try using GraphicsAdapter.DefaultAdapter "
            + "instead."

        public var GraphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile {
            get { storedProfile }
            set { storedProfile = newValue }
        }

        public var PresentationParameters:
            Microsoft.Xna.Framework.Graphics.PresentationParameters {
            get { storedParameters }
            set { storedParameters = newValue }
        }

        /// `GraphicsDeviceInformation.Equals(Object)`.
        ///
        /// Three hundred and twenty-five bytes, and every one of them is a
        /// comparison: the adapter, the profile, and **ten** presentation
        /// properties — `BackBufferWidth`, `BackBufferHeight`,
        /// `BackBufferFormat`, `DepthStencilFormat`, `MultiSampleCount`,
        /// `DisplayOrientation`, `PresentationInterval`, `RenderTargetUsage`,
        /// `DeviceWindowHandle` and `IsFullScreen`.
        ///
        /// It compares those ten **through the properties**, not by comparing
        /// the two `PresentationParameters` objects, so two informations whose
        /// parameters differ in some other field still compare equal. That is
        /// XNA's rule and it is reproduced rather than tidied.
        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? GraphicsDeviceInformation else { return false }
            guard other.storedAdapter === storedAdapter else { return false }
            guard other.storedProfile == storedProfile else { return false }
            let mine = storedParameters
            let theirs = other.storedParameters
            return theirs.BackBufferWidth == mine.BackBufferWidth
                && theirs.BackBufferHeight == mine.BackBufferHeight
                && theirs.BackBufferFormat == mine.BackBufferFormat
                && theirs.DepthStencilFormat == mine.DepthStencilFormat
                && theirs.MultiSampleCount == mine.MultiSampleCount
                && theirs.DisplayOrientation == mine.DisplayOrientation
                && theirs.PresentationInterval == mine.PresentationInterval
                && theirs.RenderTargetUsage == mine.RenderTargetUsage
                && theirs.DeviceWindowHandle == mine.DeviceWindowHandle
                && theirs.IsFullScreen == mine.IsFullScreen
        }

        /// `GraphicsDeviceInformation.GetHashCode()`.
        ///
        /// **Eleven `xor` instructions and nothing else** — the same ten
        /// properties' hash codes folded together, with no multiplier and no
        /// seed. Reproduced exactly, because a hash that agreed with `Equals`
        /// but not with XNA's would still be a different observable value.
        public func GetHashCode() -> Int32 {
            let p = storedParameters
            var hash = Int32(truncatingIfNeeded: p.BackBufferWidth.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.BackBufferHeight.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.BackBufferFormat.rawValue.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.DepthStencilFormat.rawValue.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.MultiSampleCount.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.DisplayOrientation.rawValue.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.PresentationInterval.rawValue.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.RenderTargetUsage.rawValue.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.DeviceWindowHandle.hashValue)
            hash ^= Int32(truncatingIfNeeded: p.IsFullScreen.hashValue)
            return hash
        }

        /// `GraphicsDeviceInformation.Clone()`.
        ///
        /// **Deep in one field and shallow in the other two.** The presentation
        /// parameters are `Clone()`d, so a change to the copy's parameters does
        /// not reach the original; the adapter is copied by reference, because
        /// an adapter is an identity rather than a value; the profile is a
        /// value. That asymmetry is XNA's and it is what makes the manager's
        /// ranking able to try variations without disturbing what it started
        /// from.
        public func Clone() -> GraphicsDeviceInformation {
            let copy = GraphicsDeviceInformation()
            copy.storedParameters = storedParameters.Clone()
            copy.storedAdapter = storedAdapter
            copy.storedProfile = storedProfile
            return copy
        }
    }

    /// `Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs`.
    ///
    /// The payload of the one event a consumer uses to change a device before
    /// it is created. Pure managed: a constructor and a read-only property.
    public class PreparingDeviceSettingsEventArgs: CNAEventArgs {
        private let stored: Microsoft.Xna.Framework.GraphicsDeviceInformation

        public init(
            graphicsDeviceInformation:
                Microsoft.Xna.Framework.GraphicsDeviceInformation
        ) {
            stored = graphicsDeviceInformation
            super.init()
        }

        /// The information the manager is about to create a device from.
        ///
        /// The **same object**, not a copy: changing it is the entire purpose
        /// of the event, and a copy would make the handler's work invisible.
        public var GraphicsDeviceInformation:
            Microsoft.Xna.Framework.GraphicsDeviceInformation {
            stored
        }
    }
}
