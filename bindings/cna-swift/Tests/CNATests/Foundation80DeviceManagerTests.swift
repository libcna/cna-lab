import XCTest
@testable import CNA

/// `GraphicsDeviceManager`'s `CanResetDevice`, `OnPreparingDeviceSettings` and
/// the `PreparingDeviceSettings` event.
final class Foundation80DeviceManagerTests: XCTestCase {

    /// **The whole rule is the profile.** Twenty-three bytes of IL compare the
    /// device's `GraphicsProfile` to the candidate's and nothing else — not the
    /// back-buffer size, not the format, not full screen. A device can be reset
    /// into a different resolution but not into a different profile.
    func testCanResetDeviceComparesOnlyTheProfile() throws {
        let game = try ManagerProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.sameProfile, true,
                       "a candidate with the device's profile can reset it")
        XCTAssertEqual(game.differentProfile, false,
                       "a candidate with another profile cannot")
        XCTAssertEqual(game.differentResolution, true,
                       "and a different back buffer changes nothing")
    }

    /// The raiser forwards the caller's sender, like every other raiser on this
    /// type, and the handler sees the very object the manager would use.
    func testTheEventCarriesTheInformationAHandlerCanChange() throws {
        let manager = try ManagerProbeGame.detachedManager()
        let info = Microsoft.Xna.Framework.GraphicsDeviceInformation()
        info.PresentationParameters.BackBufferWidth = 640

        var seenSender: String?
        var changed = false
        manager.PreparingDeviceSettings.Add { sender, args in
            seenSender = sender as? String
            args.GraphicsDeviceInformation.PresentationParameters
                .BackBufferWidth = 1280
            changed = true
        }
        try manager.OnPreparingDeviceSettings(
            "the caller",
            args: Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs(
                graphicsDeviceInformation: info))

        XCTAssertTrue(changed)
        XCTAssertEqual(seenSender, "the caller",
                       "the caller's sender is forwarded, not the manager")
        XCTAssertEqual(info.PresentationParameters.BackBufferWidth, 1280,
                       "the handler's change reaches the manager's own object")
    }
}

private final class ManagerProbeGame: Microsoft.Xna.Framework.Game {
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var failure: Error?
    var sameProfile: Bool?
    var differentProfile: Bool?
    var differentResolution: Bool?

    override init() throws {
        try super.init()
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    /// A manager outside any callback, for the purely managed members.
    static func detachedManager() throws
        -> Microsoft.Xna.Framework.GraphicsDeviceManager {
        let game = try ManagerProbeGame()
        return try XCTUnwrap(game.manager)
    }

    override func LoadContent() throws {
        defer { try? Exit() }
        do {
            guard let manager, let device = try GraphicsDevice else { return }
            let profile = device.GraphicsProfile

            let matching = Microsoft.Xna.Framework.GraphicsDeviceInformation()
            matching.GraphicsProfile = profile
            sameProfile = try manager.CanResetDevice(matching)

            let other = Microsoft.Xna.Framework.GraphicsDeviceInformation()
            other.GraphicsProfile = profile == .Reach ? .HiDef : .Reach
            differentProfile = try manager.CanResetDevice(other)

            let resized = Microsoft.Xna.Framework.GraphicsDeviceInformation()
            resized.GraphicsProfile = profile
            resized.PresentationParameters.BackBufferWidth = 12345
            resized.PresentationParameters.IsFullScreen = false
            differentResolution = try manager.CanResetDevice(resized)
        } catch {
            failure = error
        }
    }
}
