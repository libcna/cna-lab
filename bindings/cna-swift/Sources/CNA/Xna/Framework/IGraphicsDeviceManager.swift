// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    // Pinned in Microsoft.Xna.Framework.Game.dll as a public abstract
    // interface with no base interface and exactly three abstract methods.
    // All three are XNA device/presentation failure paths — device creation,
    // frame begin and frame end — so each takes the established `throws`
    // language projection, matching the existing `Game.BeginDraw`,
    // `Game.EndDraw` and `GraphicsDeviceManager.ApplyChanges` signatures.
    //
    // Declaring the protocol claims no device capability. Nothing conforms to
    // it: `GraphicsDeviceManager` remains an untouched runtime partial.
    public protocol IGraphicsDeviceManager {
        func CreateDevice() throws

        func BeginDraw() throws -> Bool

        func EndDraw() throws
    }
}
