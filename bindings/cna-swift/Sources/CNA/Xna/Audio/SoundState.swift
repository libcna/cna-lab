// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Audio {
    public enum SoundState: Int32 {
        case Playing = 0
        case Paused = 1
        case Stopped = 2
    }
}

extension Microsoft.Xna.Framework.Audio {
    /// `FrameworkResources.ObjectDisposedException`, read out of the
    /// registered `Microsoft.Xna.Framework.dll`.
    ///
    /// XNA's audio types pass this text rather than the BCL's generic one, and
    /// every disposed-object refusal in this namespace uses it.
    internal static let objectDisposedMessage =
        "This object has already been disposed."
}
