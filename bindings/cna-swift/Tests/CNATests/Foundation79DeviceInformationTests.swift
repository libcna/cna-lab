import XCTest
@testable import CNA

/// `GraphicsDeviceInformation` and `PreparingDeviceSettingsEventArgs`.
///
/// Two managed carriers with no CNA route between them. Everything asserted
/// here comes from the IL: which fields `Equals` compares, how `GetHashCode`
/// folds them, and which of `Clone`'s three fields is deep.
final class Foundation79DeviceInformationTests: XCTestCase {
    private typealias Info = Microsoft.Xna.Framework.GraphicsDeviceInformation

    private func information(width: Int32 = 640, height: Int32 = 480) -> Info {
        let info = Info()
        info.PresentationParameters.BackBufferWidth = width
        info.PresentationParameters.BackBufferHeight = height
        return info
    }

    /// The adapter setter is the type's only validation, and it is the reason
    /// `Adapter` is a writer method rather than a Swift `set`.
    func testSettingANilAdapterIsRefused() {
        let info = information()
        XCTAssertThrowsError(try info.SetAdapter(nil)) { error in
            guard let failure = error as? CNAArgumentNullException else {
                return XCTFail("expected CNAArgumentNullException, got \(error)")
            }
            XCTAssertEqual(failure.ParamName, "value")
        }
        XCTAssertNil(info.Adapter, "a refused set stores nothing")
    }

    func testEqualsComparesTheTenPropertiesAndTheProfile() {
        let a = information()
        let b = information()
        XCTAssertTrue(a.Equals(b), "same values compare equal")

        b.PresentationParameters.BackBufferWidth = 641
        XCTAssertFalse(a.Equals(b), "one of the ten differs")

        b.PresentationParameters.BackBufferWidth = 640
        b.GraphicsProfile = .HiDef
        XCTAssertFalse(a.Equals(b), "the profile is compared too")

        XCTAssertFalse(a.Equals("not an information"))
        XCTAssertFalse(a.Equals(nil))
    }

    /// `Equals` reads the ten properties **through the parameters object**,
    /// never comparing the two objects themselves — so a difference in a field
    /// outside those ten leaves the two equal. That is XNA's rule and this
    /// asserts it rather than tidying it away.
    func testAPropertyOutsideTheTenDoesNotBreakEquality() {
        let a = information()
        let b = information()
        XCTAssertTrue(a.Equals(b))
        XCTAssertFalse(a.PresentationParameters === b.PresentationParameters,
                       "different objects, equal in the ten compared fields")
    }

    /// Eleven `xor`s and nothing else: no multiplier, no seed.
    func testTheHashIsTheXorOfTheSameProperties() {
        let a = information()
        let b = information()
        XCTAssertEqual(a.GetHashCode(), b.GetHashCode())

        // Every one of the ten must reach the fold. A mutation dropping just
        // BackBufferHeight survived a test that only ever changed the width --
        // one property proves one property.
        let changes: [(String, (Microsoft.Xna.Framework.Graphics.PresentationParameters) -> Void)] = [
            ("BackBufferWidth", { $0.BackBufferWidth = 1024 }),
            ("BackBufferHeight", { $0.BackBufferHeight = 768 }),
            ("BackBufferFormat", { $0.BackBufferFormat = .Bgra5551 }),
            ("DepthStencilFormat", { $0.DepthStencilFormat = .Depth24Stencil8 }),
            ("MultiSampleCount", { $0.MultiSampleCount = 4 }),
            ("DisplayOrientation", { $0.DisplayOrientation = .LandscapeLeft }),
            ("PresentationInterval", { $0.PresentationInterval = .Immediate }),
            ("RenderTargetUsage", { $0.RenderTargetUsage = .PreserveContents }),
            ("DeviceWindowHandle", { $0.DeviceWindowHandle = 4242 }),
            ("IsFullScreen", { $0.IsFullScreen = false })   // the default is true,
        ]
        for (name, change) in changes {
            let baseline = information()
            let changed = information()
            change(changed.PresentationParameters)
            XCTAssertNotEqual(baseline.GetHashCode(), changed.GetHashCode(),
                              "\(name) must reach the hash")
            XCTAssertFalse(baseline.Equals(changed),
                           "\(name) must reach Equals too")
        }
    }

    /// Deep in the parameters, shallow in the adapter.
    func testCloneIsDeepInTheParametersAndShallowInTheAdapter() {
        let original = information()
        original.GraphicsProfile = .HiDef
        let copy = original.Clone()

        XCTAssertTrue(original.Equals(copy))
        XCTAssertEqual(copy.GraphicsProfile, .HiDef)
        XCTAssertFalse(copy.PresentationParameters === original.PresentationParameters,
                       "the parameters are cloned, not shared")

        copy.PresentationParameters.BackBufferWidth = 1
        XCTAssertEqual(original.PresentationParameters.BackBufferWidth, 640,
                       "and a change to the copy does not reach the original")
    }

    /// The event payload carries the **same** information object, because
    /// changing it is the point of the event.
    func testTheEventArgsCarryTheSameObject() {
        let info = information()
        let args = Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs(graphicsDeviceInformation: info)
        XCTAssertTrue(args.GraphicsDeviceInformation === info)
        args.GraphicsDeviceInformation.PresentationParameters.BackBufferWidth = 800
        XCTAssertEqual(info.PresentationParameters.BackBufferWidth, 800,
                       "a handler's change is visible to the manager")
    }
}
